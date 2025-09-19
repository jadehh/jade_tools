/**
# @File     : hasp_adapter.cpp
# @Author   : jade
# @Date     : 2025/9/16 10:42
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : hasp_adapter.cpp
*/
#include <algorithm>
#include <atomic>
#include <condition_variable>
#include "include/jade_tools.h"
using namespace jade;

#include "include/hasp_code.h"
#define MODULE_NAME "HaspAdapter"

#ifdef  HASP_ENABLED
class HaspAdapter::Impl
{
    HaspAdapter::HaspAdapterDevice device_;
    int featureId_;
    std::string lastErrorMsg_;
    hasp_status_t lastError_;
    std::vector<int> haspIdList_;
    std::atomic<bool> running_;
    std::thread haspListenerThread_; // 加密狗监听线程
    hasp_handle_t handle_;
    hasp_handle_t listen_handle_;
public:
    explicit Impl(const HaspAdapterDevice device,const std::vector<int>& haspIdList):device_(device),featureId_(0),lastError_(HASP_STATUS_OK),haspIdList_(haspIdList),running_(false),handle_(HASP_INVALID_HANDLE_VALUE),listen_handle_(HASP_INVALID_HANDLE_VALUE)
    {
        if (const bool success = login(&handle_); !success) printError();
        else DLL_LOG_TRACE(MODULE_NAME) << "加密狗登录成功,当前授权ID为:101";
    }

    bool checkHaspStatus(const hasp_status_t status)
    {
        lastError_ = status;
        switch (status)
        {
        case HASP_STATUS_OK:
            return true;
        case HASP_FEATURE_NOT_FOUND:
            lastErrorMsg_ = "找不到功能,请联系开发重新授权";
            return false;
        case HASP_ACCESS_DENIED:
            lastErrorMsg_ = "禁止访问";
            return false;
        case HASP_HASP_NOT_FOUND:
            lastErrorMsg_ = "未检测到加密狗,请检查加密狗是否插入";
            return false;
        case HASP_TOO_MANY_USERS :
            lastErrorMsg_ = "当前登录用户超过最大限制";
            return false;
        case HASP_FEATURE_EXPIRED:
            lastErrorMsg_ = "当前加密狗驱动版本过低,请点击工具中并升级加密狗驱动";
            return false;
        case HASP_MISSING_LM:
            lastErrorMsg_ = "未找到许可证管理器,请安装加密狗驱动,访问连接http://localhost:1947";
            return false;
        case HASP_SCOPE_RESULTS_EMPTY:
            lastErrorMsg_ = "找不到与范围匹配的功能,当前程序未使用网络锁功能,请检查本地是否插入加密狗";
            return false;
        case HASP_SHARING_VIOLATION:
            lastErrorMsg_ = "共享违规,如果是在Docker中启动,需要将宿主机的加密狗驱动卸载";
            return false;
        default:
            return false;
        }
    }

    /**
    * 设备上的多进程，只会占用会话，不会占用登录数，
    * 如果不在Docker上使用的话，需要对锁进行站点的限制，或者直接对会话进行限制
    */
    hasp_status_t login_featureID(hasp_handle_t* handle,const int feature_id) const
    {
        if (feature_id == 0) throw std::runtime_error("Feature ID 不能为0");
        std::string scope = "";
        switch (device_)
        {
        case DOCKER:
            // Docker 启动  Docker中 使用远程的锁,注意Docker不能使用--net启动,如果需要用到--net,请在Docker中安装加密狗的环境，设备类型需要切换到系统启动
            scope =
                "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"
                     "<haspscope>"
                      "    <license_manager hostname=\"~localhost\" />"
                "</haspscope>";
            return hasp_login_scope(feature_id,scope.c_str(), vendor_code, handle);
        case SYSTEM:
            // 系统启动
            scope =
            "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"
            "<haspscope>"
            "    <license_manager hostname=\"localhost\" />"
            "</haspscope>";
            return hasp_login_scope(feature_id,scope.c_str(), vendor_code, handle);
            default:
            return hasp_login(feature_id,vendor_code,handle);
        }

    }

    bool login(hasp_handle_t* handle, const int featureId)
    {
        hasp_status_t status = HASP_STATUS_OK;
        status = login_featureID(handle,featureId);
        return checkHaspStatus(status);
    }

    bool login(hasp_handle_t* handle)
    {
        const bool status = std::any_of(haspIdList_.begin(), haspIdList_.end(), [this, handle](const auto& id) {
            if (login(handle,id)){return true;}// 直接返回条件
            return false;
        });
        return status;
    };
    ~Impl() {shutDown();};
    Impl(const Impl&) = delete;
    Impl& operator=(const Impl&) = delete;
    void run()
    {
        if (running_) return;
        running_ = true;
        startHaspListener();
    };
    // 启动加密狗监听线程
    void startHaspListener() {
        haspListenerThread_ = std::thread([this] {
            while (running_) {
                std::this_thread::sleep_for(std::chrono::seconds(30));
                if (!login(&listen_handle_)){printError();}else
                {
                    DLL_LOG_TRACE(MODULE_NAME) << "加密狗监听成功";
                    hasp_logout(listen_handle_);
                }
            }
        });
    }
    void shutDown()
    {
        running_ = false;
        hasp_logout(handle_);
        DLL_LOG_TRACE(MODULE_NAME) << "加密狗监听关闭";
    };

    [[nodiscard]] int getLastError() const
    {
        return lastError_;
    }

    [[nodiscard]] std::string getLastErrorMsg() const
    {
        return lastErrorMsg_;
    }

    void printError()
    {
        shutDown();
        DLL_LOG_CRITICAL(MODULE_NAME,getLastError()) << "加密狗登录失败,失败原因:" << getLastErrorMsg() ;
    }
};


HaspAdapter& HaspAdapter::getInstance()
{
    static HaspAdapter instance;
    return instance;
}

HaspAdapter::HaspAdapter():impl_(nullptr){}

void HaspAdapter::init(const HaspAdapterDevice device,const std::vector<int>& haspIdList)
{
    impl_ = new Impl(device,haspIdList);
}

void HaspAdapter::run() const
{
    if (impl_)
    {
        impl_->run();
    }
}

void HaspAdapter::shutDown() const
{
    if (impl_)
    {
        impl_->shutDown();
    }
}
#endif





