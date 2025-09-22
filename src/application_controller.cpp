#include "include/jade_tools.h"
#include <atomic>
#include <csignal>
#include <mutex>
#include <condition_variable>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#include <conio.h> // 用于 Windows 键盘输入
#else
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#endif
#define MODULE_NAME "ApplicationController"
using namespace jade;

// PImpl 实现类
class ApplicationController::Impl
{
public:
    explicit Impl() :  shouldExit_(false)
    {
        setupSignalHandling();
        startKeyListener(); // 启动键盘监听线程
    }

    ~ Impl()
    {
        // 平台特定的清理
        platformSpecificCleanup();

        // 确保退出标志被设置
        requestExit();

        // 停止键盘监听线程
        if (keyListenerThread_.joinable())
        {
            keyListenerThread_.join();
        }
    }

    void run()
    {
        // 主线程等待退出信号
        waitForExit();
    }

private:
    void setupSignalHandling()
    {
        // 设置信号处理
        std::signal(SIGINT, handleSignal);
        std::signal(SIGTERM, handleSignal);

        // 平台特定的设置
        platformSpecificSetup();
    }


    void requestExit()
    {
        shouldExit_ = true;
        {
            std::lock_guard lock(exitMutex_);
            exitRequested_ = true;
        }
        exitCondition_.notify_all();
    }

    void waitForExit()
    {
        std::unique_lock<std::mutex> lock(exitMutex_);
        exitCondition_.wait(lock, [this] { return exitRequested_; });
    }

    static void handleSignal(const int signal)
    {
        if (signal == SIGINT || signal == SIGTERM)
        {
            // 获取当前实例（如果有）
            if (Impl* instance = currentInstance_.load())
            {
                instance->requestExit();
            }
        }
    }

    void platformSpecificSetup()
    {
        // 保存当前实例指针
        currentInstance_ = this;

#ifdef _WIN32
        // 添加控制台事件处理程序
        if (!SetConsoleCtrlHandler(windowsCtrlHandler, TRUE)) {
            // 输出错误信息
            DLL_LOG_ERROR(MODULE_NAME)<< "设置控制台时间失败  ";
        }

        // 禁用快速编辑模式，确保 Ctrl+C 正常工作
        if ( HANDLE const hStdin = GetStdHandle(STD_INPUT_HANDLE); hStdin != INVALID_HANDLE_VALUE) {
            DWORD mode = 0;
            if (GetConsoleMode(hStdin, &mode)) {
                // 禁用快速编辑模式
                mode &= ~ENABLE_QUICK_EDIT_MODE;
                // 启用窗口输入模式
                mode |= ENABLE_WINDOW_INPUT;
                SetConsoleMode(hStdin, mode);
            }
        }

#endif
    }

    static void platformSpecificCleanup()
    {
#ifdef _WIN32
        // 清理Windows控制台事件处理
        SetConsoleCtrlHandler(windowsCtrlHandler, FALSE);
#endif
        // 清除当前实例指针
        currentInstance_ = nullptr;
    }

#ifdef _WIN32
    static BOOL WINAPI windowsCtrlHandler(const DWORD event) {
        // 处理所有可能的退出事件
        switch (event) {
            case CTRL_C_EVENT:       // Ctrl+C
            case CTRL_BREAK_EVENT:   // Ctrl+Break
            case CTRL_CLOSE_EVENT:   // 控制台关闭
                if (Impl* instance = currentInstance_.load()) {
                    instance->requestExit();
                    return TRUE;
                }
            default: break;
        }
        return FALSE;
    }
#endif
    // 启动键盘监听线程
    void startKeyListener()
    {
        keyListenerThread_ = std::thread([this]
        {
            while (!shouldExit_)
            {
                if (checkForEnterKey())
                {
                    DLL_LOG_TRACE("ApplicationController") << "检测到Enter键按下，程序将退出...";
                    requestExit();
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        });
    }

    // 检查是否按下Enter键
    static bool checkForEnterKey()
    {
#ifdef _WIN32
        // Windows实现
        if (_kbhit()) {
            if (int ch = _getch(); ch == '\r' || ch == '\n') {
                return true;
            }
        }
#else
        // Unix/Linux实现
        termios old_t{}, newt{};
        tcgetattr(STDIN_FILENO, &old_t);
        newt = old_t;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);

        const int old_f = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, old_f | O_NONBLOCK);

        int ch = getchar();

        tcsetattr(STDIN_FILENO, TCSANOW, &old_t);
        fcntl(STDIN_FILENO, F_SETFL, old_f);

        if (ch != EOF && (ch == '\r' || ch == '\n'))
        {
            return true;
        }
#endif
        return false;
    }

private:
    std::atomic<bool> shouldExit_;
    std::thread keyListenerThread_; // 键盘监听线程
    // 用于等待退出的同步变量
    std::mutex exitMutex_;
    std::condition_variable exitCondition_;
    bool exitRequested_ = false;

    // 当前实例指针（线程安全）
    static std::atomic<Impl*> currentInstance_;
};

// 初始化静态成员
std::atomic<ApplicationController::Impl*> ApplicationController::Impl::currentInstance_{nullptr};



// ApplicationController 成员函数实现
ApplicationController::ApplicationController():impl_(new Impl())
{
}

ApplicationController& ApplicationController::getInstance()
{
    static ApplicationController instance;
    return instance;
}

void ApplicationController::run() const
{
    if (impl_)
    {
        // 提示用户如何退出
        impl_->run();
    }
}

void ApplicationController::stop()
{
    if (impl_)
    {
        delete impl_;
        impl_ = nullptr;
    }
}

