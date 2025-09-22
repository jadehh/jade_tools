/**
# @File     : jade_tools.h
# @Author   : jade
# @Date     : 2025/8/1 13:50
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : jade_tools.h
*/
//

#pragma once

// 跨平台导出宏
#include <atomic>
#include <chrono>
#include <functional>
#include <map>
#include <set>
#include <thread>
#include <variant>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#define SqliteInt64 int64_t
#ifdef JADE_TOOLS_EXPORTS
#define JADE_API __declspec(dllexport)
#else
    #define JADE_API __declspec(dllimport)
#endif
#else
#define SqliteInt64 long long int
#define JADE_API __attribute__((visibility("default")))
#endif
#if defined(__GNUC__) && (__GNUC__ >= 7) && (__GNUC__ < 8)
#define LOW_GCC 1
#else
#define LOG_GCC 0
#endif

#if defined(__has_include)
#  if __has_include(<opencv2/opencv.hpp>)  // 标准化的头文件存在性检查
#    include <opencv2/opencv.hpp>
#    define OPENCV_ENABLED 1
#if defined(__has_include)
#if __has_include(<opencv2/cudacodec.hpp>) // 标准化的头文件存在性检查
#include <opencv2/cudacodec.hpp>
#define OPENCV_CUDA_ENABLED 1
#endif
#endif
#endif
#endif

#if defined(__has_include)
#if __has_include(<hasp_api.h>) // 标准化的头文件存在性检查
#define HASP_ENABLED 1
#endif
#endif


// 核心功能命名空间
namespace jade
{
    struct jade_time : tm
    {
        int tm_millis;
    };


    JADE_API const char* getVersion();
    /**
    * 打印水平居中表格
    * @param headers  表头
    * @param data     表格数据
    * @param widths   列宽（可选）
    */
    JADE_API void printPrettyTable(const std::vector<std::string>& headers,
                                   const std::vector<std::vector<std::string>>& data,
                                   const std::vector<int>& widths = {});
    /**
    * 输出表格
    * @param headers  表头
    * @param data     表格数据
    * @param widths   列宽（可选）
    * @return         表格字符串
    */
    JADE_API std::string getPrettyTable(const std::vector<std::string>& headers,
                                        const std::vector<std::vector<std::string>>& data,
                                        const std::vector<int>& widths = {});

    /**
     * String转 WString
     * @param str
     * @return
     */
    std::wstring string_to_wstring(const std::string& str);
    /**
     * WString转String
     * @param w_str
     * @return
     */
    std::string wstring_to_string(const std::wstring& w_str);

#ifdef _WIN32
    /**
     * 二进制Code转String
     * @param code
     * @return
     */
    std::string to_hex_string(DWORD code);

    /**
    * 将字节转换为易读格式
    * @param bytes
    * @return
    */
    std::string formatBytes(SIZE_T bytes);
    /**
     *
     * @param bytes
     * @return
     */
    double bytesToMB(SIZE_T bytes);
#endif
    // 检查文件扩展名是否为图片格式
    JADE_API bool isImageFile(const std::string& path);
    // 保留2位小数点
    std::string formatValue(const double& value, int precision = 2, bool fixed = true);
    std::string formatValue(int& value, int precision = 2, bool fixed = true);

    /**
     * 获取操作系统名称
     * @reutrn   名称
     */
    std::string getOperatingSystemName();
    /**
     * 时间相关
     * @return
     */
    jade_time getTimeStamp();
    std::string getTimeStampString(const jade_time& time, const char* fmt_arg = "[%Y-%m-%d %H:%M:%S]", bool with_milliseconds = false);
    std::string getTimeStampString(const char* fmt_arg = "[%Y-%m-%d %H:%M:%S]", bool with_milliseconds = false);
    std::string timePointToTimeString(std::chrono::time_point<std::chrono::system_clock> clock, const char* fmt_arg = "[%Y-%m-%d %H:%M:%S]",
                                      bool with_milliseconds = false);

    /**
     * 资源清理函数
     */
    JADE_API void jadeToolsClean();


    /**
     * ####################ConsoleColor###########################
     **/
    class JADE_API ConsoleColor
    {
    public:
        // 预定义颜色常量（RGB）
        struct Colors
        {
            static constexpr int RED[3] = {255, 0, 0};
            static constexpr int GREEN[3] = {0, 255, 0};
            static constexpr int BLUE[3] = {0, 0, 255};
            static constexpr int YELLOW[3] = {255, 255, 0};
            static constexpr int MAGENTA[3] = {255, 0, 255};
            static constexpr int CYAN[3] = {0, 255, 255};
            static constexpr int WHITE[3] = {255, 255, 255};
            static constexpr int BLACK[3] = {0, 0, 0};
            static constexpr int ORANGE[3] = {255, 165, 0};
            static constexpr int PURPLE[3] = {128, 0, 128};
        };

        // 重置所有样式
        static void reset();
        // 前景色（文本颜色）
        static void setForegroundColor(const int rgb[3]);
        static std::string getForegroundColor(const int rgb[3]);

        // 背景色
        static void setBackgroundColor(const int rgb[3]);
        // 前景色（文本颜色）
        static void setForegroundColor(int r, int g, int b);
        // 背景色
        static void setBackgroundColor(int r, int g, int b);


        // 文本样式
        static void bold(); // 粗体
        static void italic(); // 斜体
        static void underline(); // 下划线
        static void reverse(); // 反色（交换前景和背景）
    };

    /**
     * ####################Logger###########################
     */

    class SpdLoggerIMPL;

    class JADE_API Logger
    {
    public:
        // 日志级别枚举
        enum Level
        {
            S_TRACE = 0,
            S_DEBUG = 1,
            S_INFO = 2,
            S_WARNING = 3,
            S_ERROR = 4,
            S_CRITICAL = 5,
            S_EXCEPTION = 6,
        };

        // 获取单例实例
        static Logger& getInstance();

        // 初始化日志系统
        void init(const std::string& app_name = "app",
                  const std::string& logName = "info",
                  const std::string& logDir = "Logs",
                  Level logLevel = S_INFO,
                  bool consoleOutput = true,
                  bool fileOutput = true,
                  size_t maxFileSize = 1024 * 1024 * 1, // 1MB
                  size_t maxFiles = 30) const;
        // 日志记录方法
        void log(Level level, const std::string& message, const char* file = "", int line = 0) const;
        void trace(const std::string& message, const char* file = "", int line = 0) const;
        void debug(const std::string& message, const char* file = "", int line = 0) const;
        void info(const std::string& message, const char* file = "", int line = 0) const;
        void warn(const std::string& message, const char* file = "", int line = 0) const;
        void error(const std::string& message, const char* file = "", int line = 0) const;
        void critical(const std::string& message, int exitCode, const char* file = "", int line = 0) const;
        void exception(const std::string& message, const std::string& e, int exitCode, const char* file = "", int line = 0) const;

        // 设置日志级别
        void setLevel(Level level) const;
        // 关闭日志
        void shutDown() ;
        // 设置DLLName
        static void setDllName(const std::string& dllName);

    private:
        // 立即刷新日志
        SpdLoggerIMPL* logger_;
        Logger(); // 私有构造函数
        void getError(const std::string& message, int exitCode, const char* file, int line) const;
    };

    /**
     * ###############################################Utils##################################################
     */
    class JADE_API Utils
    {
    public:
        // 获取单例实例
        static Utils& GetInstance()
        {
            static Utils instance;
            return instance;
        }

        static void setConsole();
        static void setConsoleUTF8();
        static void enableVirtualTerminal();
    };

    class JADE_API LoggerStream
    {
    public:
        class Impl;
        LoggerStream(Logger::Level level, const char* file, int line, int exitCode = 0, const std::string& e = "");
        // 重载 << 操作符，支持fmt格式化
        ~LoggerStream();
        LoggerStream& operator<<(const char* value);
        LoggerStream& operator<<(const std::string& value);
        LoggerStream& operator<<(int value);
        LoggerStream& operator<<(double value);
        LoggerStream& operator<<(float value);
        LoggerStream& operator<<(bool value);
        LoggerStream& operator<<(long value);

        [[nodiscard]] Impl* getImpl() const;

        void setStream(const char* value) const;
        void setStream(const std::string& value) const;
        void setStream(int value) const;
        void setStream(double value) const;
        void setStream(float value) const;
        void setStream(long value) const;

    private:
        Impl* impl_;
    };

    class JADE_API DLLLoggerStream : public LoggerStream
    {
    public:
        DLLLoggerStream(Logger::Level level, const char* file, int line, const char* moduleName, int exitCode = 0,
                        const std::string& e = "");
        void setModuleName(const char* moduleName) const;
    };

    /**
     * #####################################################FileTools##############################################
     */
    class JADE_API FileTools
    {
    public:
        // 获取单例实例
        static FileTools& GetInstance()
        {
            static FileTools instance;
            return instance;
        }

        // 创建单层目录（父目录必须存在）
        static bool createDirectory(const std::string& path);

        // 递归创建多级目录（自动创建父目录）
        static bool createDirectories(const std::string& path);

        // 检查路径是否存在且是目录
        static bool isExists(const std::string& path);

        // 获取最后一次错误信息
        static std::string getLastError();

        // 删除目录（可选功能）
        static bool remove(const std::string& path);

        //处理不同操作系统下的路径分隔符差异，确保路径字符串在不同平台上都能正确工作
        static std::string fixPath(const std::string& path);


        // 获取文件夹下所有的图片文件
        static std::vector<std::string> getImageFiles(const std::string& path, bool fullPath = true);

        // 写入二进制文件
        static bool writeBinaryToFile(const std::string& path, const float* data, int size);
        static bool writeBinaryToFile(const std::string& path, const char* data, int size);
        // 禁止拷贝和赋值
        FileTools(const FileTools&) = delete;
        FileTools& operator=(const FileTools&) = delete;

    private:
        // 将错误信息设置为模块私有，通过静态方法访问
        static std::string& getLastErrorRef()
        {
            static std::string lastError;
            return lastError;
        }

        FileTools() = default; // 私有构造函数
        ~FileTools() = default;
    };

    /**
     * ################################################EnhancedTimeProfiler##########################################################
     */
    class DynamicSystemMonitorImpl;

    class SystemMonitorImpl
    {
    public:
        struct ResourceMetrics
        {
            double cpuUsage{}; // CPU使用率（百分比）
            double memoryUsage{}; // 内存使用量（字节）
            uint64_t diskReads{}; // 磁盘读取次数
            uint64_t diskWrites{}; // 磁盘写入次数
            // 新增速度指标
            double readSpeed = 0.0; // 读取速度（MB/s）
            double writeSpeed = 0.0; // 写入速度（MB/s）
            double gpuUsage{}; // GPU使用率（百分比）
            double gpuMemory{}; // GPU内存使用量（MB）
        };

        // 获取系统资源指标
        static ResourceMetrics getMetrics();

    private:
        static void getMetrics(ResourceMetrics& metrics);
    };

    class JADE_API EnhancedTimeProfiler
    {
        DynamicSystemMonitorImpl* dynamic_system_monitor_impl_;
        int count_ = 1;
        [[nodiscard]] std::vector<std::vector<std::string>> getDatas() const;

    public:
        EnhancedTimeProfiler();
        ~EnhancedTimeProfiler();
        // 开始计时并监控资源
        void startStep(const std::string& stepName, int interval_ms = 10) const;
        // 结束计时并记录资源使用情况
        void endStep(const std::string& stepName, int count = 1);
        static std::vector<SystemMonitorImpl::ResourceMetrics> extractMiddleElements(
            const std::vector<SystemMonitorImpl::ResourceMetrics>& arr);
        // 输出统计报告
        void printTable() const;
        [[nodiscard]] std::string getTable() const;
        // 重置所有监控数据
        void reset() const;
    };

    /**
     * ######################################CryptoUtil###################################
     */
    class CryptoUtilImpl;

    class JADE_API JadeCryptoUtil
    {
    public:
        enum class CryptoMode
        {
            AES_256_CBC,
            AES_256_ECB,
            AES_128_CBC,
            AES_128_ECB,
            // 可以扩展其他加密算法
        };

        struct CryptoResult
        {
            bool success;
            std::string message;
            size_t dataSize;
        };

    public:
        explicit JadeCryptoUtil(CryptoMode mode = CryptoMode::AES_256_CBC);
        ~JadeCryptoUtil();
        // 设置密钥和IV
        void setKey(const std::vector<unsigned char>& key) const;
        void setIV(const std::vector<unsigned char>& iv) const;
        // 加密接口
        CryptoResult encryptDataToMemory(const std::vector<unsigned char>& inputData,
                                         std::vector<unsigned char>& outputData) const;
        CryptoResult encryptFileToMemory(const std::string& inputFile, std::vector<unsigned char>& outputData) const;
        [[nodiscard]] CryptoResult encryptFileToFile(const std::string& inputFile, const std::string& outputFile) const;
        // 解密接口
        CryptoResult decryptData(const std::vector<unsigned char>& inputData,
                                 std::vector<unsigned char>& outputData) const;
        CryptoResult decryptFileToMemory(const std::string& inputFile, std::vector<unsigned char>& outputData) const;
        [[nodiscard]] CryptoResult decryptFileToFile(const std::string& inputFile, const std::string& outputFile) const;
        // 工具方法
        static bool validateKeySize(CryptoMode mode, size_t keySize);
        static bool validateIVSize(CryptoMode mode, size_t ivSize);
        static std::string getModeName(CryptoMode mode);
        static std::vector<unsigned char> generateRandomKey(CryptoMode mode);
        static std::vector<unsigned char> generateRandomIV(CryptoMode mode);
        // 获取当前模式
        [[nodiscard]] CryptoMode getMode() const { return mode_; }

    private:
        CryptoMode mode_;
        CryptoUtilImpl* crypto_util_impl_;
    };

    /**
    * Sqlite 帮助类
    */

    class JADE_API SqliteHelper
    {
    public:
        // 定义SQLiteValue类型，可以表示SQLite中的所有数据类型
        using SQLiteValue = std::variant<
            std::monostate, // 对应SQLite的NULL
            SqliteInt64, // 对应SQLite的INTEGER
            double, // 对应SQLite的REAL
            std::string, // 对应SQLite的TEXT
            std::vector<uint8_t> // 对应SQLite的BLOB
        >;

        // 事务处理（线程安全）
        class JADE_API Transaction
        {
        public:
            explicit Transaction(SqliteHelper& db);
            ~Transaction();
            void commit();

        private:
            SqliteHelper& db_;
            bool committed;
        };

        // 删除复制构造函数和赋值运算符
        SqliteHelper(const SqliteHelper&) = delete;
        SqliteHelper& operator=(const SqliteHelper&) = delete;
        static SqliteHelper& getInstance();
        void init(const char* dbPath);
        // 创建数据库连接（使用智能指针管理）
        [[nodiscard]] std::vector<std::map<std::string, SQLiteValue>> query(const std::string& sql) const;
        // 执行SQL命令（线程安全）
        [[nodiscard]] bool execute(const std::string& sql) const;
        [[nodiscard]] bool executeWithParams(const std::string& sql, const std::vector<SQLiteValue>& params) const;
        void close() ;

    private:
        SqliteHelper();
        class Impl;
        Impl* impl_;
    };

    /** 崩溃处理类
     * ######################################CrashHandler###################################
     */
    class JADE_API CrashHandler
    {
    public:
        // 删除复制构造函数和赋值运算符
        CrashHandler(const CrashHandler&) = delete;
        CrashHandler& operator=(const CrashHandler&) = delete;
        static CrashHandler& getInstance();
        void init(const std::string& dumpPath, const std::function<void()>& func = nullptr);
        void shutDown();
        // 添加自定义信息
        void setCustomInfo(const std::string& key, const std::string& value) const;
        void clearCustomInfo() const;
        // 触发测试崩溃（仅用于调试）
        void triggerTestCrash() const;

    private:
        CrashHandler();
        class Impl;
        Impl* impl_;
    };

    /** ini文件处理类
     * ######################################INIReader###################################
     */
    class JADE_API INIReader
    {
    public:
        // Empty Constructor
        INIReader();

        // Construct INIReader and parse given filename. See ini.h for more info
        // about the parsing.
        explicit INIReader(const std::string& filename);

        // Construct INIReader and parse given file. See ini.h for more info
        // about the parsing.
        explicit INIReader(FILE* file);

        // Return the result of ini_parse(), i.e., 0 on success, line number of
        // first error on parse error, or -1 on file open error.
        [[nodiscard]] int ParseError() const;

        // Return the list of sections found in ini file
        [[nodiscard]] const std::set<std::string>& Sections() const;

        // Get a string value from INI file, returning default_value if not found.
        [[nodiscard]] std::string Get(const std::string& section, const std::string& name,
                                      const std::string& default_value) const;

        // Get an integer (long) value from INI file, returning default_value if
        // not found or not a valid integer (decimal "1234", "-1234", or hex "0x4d2").
        [[nodiscard]] long GetInteger(const std::string& section, const std::string& name, long default_value) const;

        // Get a real (floating point double) value from INI file, returning
        // default_value if not found or not a valid floating point value
        // according to strtod().
        [[nodiscard]] double GetReal(const std::string& section, const std::string& name, double default_value) const;

        // Get a single precision floating point number value from INI file, returning
        // default_value if not found or not a valid floating point value
        // according to strtof().
        [[nodiscard]] float GetFloat(const std::string& section, const std::string& name, float default_value) const;

        // Get a boolean value from INI file, returning default_value if not found or
        // if not a valid true/false value. Valid true values are "true", "yes", "on",
        // "1", and valid false values are "false", "no", "off", "0" (not case
        // sensitive).
        [[nodiscard]] bool GetBoolean(const std::string& section, const std::string& name, bool default_value) const;

    protected:
        class Impl;
        Impl* impl_;;
    };

    /** SocketServer
     * ######################################SocketServer###################################
     */

    class JADE_API SocketServer
    {
        using MessageHandler = std::function<void(int, const std::string&)>;

    public:
        // 获取单例实例的静态方法
        static SocketServer& getInstance();
        // 删除拷贝构造函数和赋值运算符
        SocketServer(const SocketServer&) = delete;
        SocketServer& operator=(const SocketServer&) = delete;
        void init(int port, const MessageHandler& handler);
        void start() const;
        void stop() ;

    private:
        SocketServer();
        class Impl;
        Impl* impl_;
    };

    /** ApplicationController
     * ######################################ApplicationController###################################
     */

    class JADE_API ApplicationController
    {
    private:
        class Impl;
        Impl* impl_;
        ApplicationController();
    public:
        static ApplicationController& getInstance();
        void run() const;
        void stop();
        // 禁止拷贝和赋值
    };

    /** HaspAdapter
    * ######################################HaspAdapter###################################
    */
#ifdef HASP_ENABLED
    class JADE_API HaspAdapter
    {
    private:
        HaspAdapter();
        class Impl;
        Impl* impl_;

    public:
        enum HaspAdapterDevice
        {
            /** Docker登录 需要有远程访问*/
            DOCKER = 0,
            /**系统登录*/
            SYSTEM = 1,
            /**未知*/
            UNKNOW = 3
        };

        static HaspAdapter& getInstance();
        void init(HaspAdapterDevice device, const std::vector<int>& haspIdList = {});
        void run() const;
        void shutDown() ;
        // 禁止拷贝和赋值
        HaspAdapter(const HaspAdapter&) = delete;
        HaspAdapter& operator=(const HaspAdapter&) = delete;
    };
#endif

    /** VideoCapture
    * ######################################VideoCapture###################################
    */
#ifdef  OPENCV_ENABLED
    class JADE_API VideoCaptureBase
    {
    private:
        class Impl;
        Impl* impl_;

    public:
        explicit VideoCaptureBase(const std::string& source, bool use_gpu, int frame_interval);
        void start();
        void stop() const;
        virtual std::string getVideoInfo() const = 0;
        virtual ~VideoCaptureBase() = default;
        virtual void process(cv::Mat& frame) = 0;
#ifdef OPENCV_CUDA_ENABLED
        virtual void process(cv::cuda::GpuMat& gpu_mat) = 0;
#endif
    };

    class RtspVideoCapture final : public VideoCaptureBase
    {
    public:
        enum class RtspDeviceType
        {
            UNKNOWN, // 未知设备
            HIKVISION, // 海康威视
            DAHUA, // 大华
            ONVIF_GENERIC // 通用ONVIF设备
        };

        // 流类型枚举（主码流、子码流等）
        enum class RtspStreamType
        {
            MAIN_STREAM, // 主码流（高清）
            SUB_STREAM, // 子码流（标清）
            THIRD_STREAM, // 第三码流
            AUDIO_STREAM // 音频流
        };

        struct RtspInfo
        {
            std::string camera_name; // 相机名称
            std::string username; // 用户名
            std::string password; // 密码
            std::string ip_address; // IP地址或域名
            int port; // 端口号，默认为554
            std::string stream_path; // 流路径
            bool use_gpu; //是否使用GPU解码
            int frame_interval; // 参数含义, 每5帧中，你只处理第1帧（或任意指定的一帧），跳过中间的4帧。
            RtspDeviceType device_type; // 设备类型
            // 构造函数
            RtspInfo();
            RtspInfo(const std::string& camera_name, const std::string& user,
                     const std::string& pwd, const std::string& ip, int port_num = 554,
                     const std::string& path = "", bool use_gpu = false, int frame_interval = 5,
                     RtspDeviceType type = RtspDeviceType::UNKNOWN);

            [[nodiscard]] std::string toRtspUrl() const;
            // 根据设备类型获取默认流路径
            [[nodiscard]] std::string getDefaultStreamPath() const;
            // 获取设备类型字符串
            [[nodiscard]] std::string getDeviceTypeString() const;
            // 设置设备类型（支持从字符串设置）
            void setDeviceTypeFromString(const std::string& type_str);
            // 检查连接信息是否有效
            [[nodiscard]] bool isValid() const;
            // 清空所有信息
            void clear();
        };

        // 流路径工具类
        class RtspPathHelper
        {
        public:
            // 获取海康设备流路径
            static std::string getHikvisionPath(RtspStreamType stream_type = RtspStreamType::MAIN_STREAM,
                                                int channel = 1);
            // 获取大华设备流路径
            static std::string getDahuaPath(RtspStreamType stream_type = RtspStreamType::MAIN_STREAM, int channel = 1);
            // 根据设备类型获取流路径
            static std::string getPathByDeviceType(RtspDeviceType device_type,
                                                   RtspStreamType stream_type = RtspStreamType::MAIN_STREAM,
                                                   int channel = 1);
        };

    private:
        RtspInfo rtsp_info_;

    public:
        //定义回调函数
        using CpuFrameCallback = std::function<void(RtspInfo, const cv::Mat&)>;
        explicit RtspVideoCapture(const RtspInfo& rtsp_info,
                                  CpuFrameCallback cpu_frame_callback);

#ifdef OPENCV_CUDA_ENABLED
        using GpuFrameCallback = std::function<void(RtspInfo, cv::cuda::GpuMat& gpu_mat)>;
        explicit RtspVideoCapture(const RtspInfo& rtsp_info,
                                  GpuFrameCallback gpu_frame_callback);
        explicit RtspVideoCapture(const RtspInfo& rtsp_info,
                                  CpuFrameCallback cpu_frame_callback,
                                  const GpuFrameCallback& gpu_frame_callback);

#endif
        void process(cv::Mat& frame) override;
#ifdef OPENCV_CUDA_ENABLED
        void process(cv::cuda::GpuMat& gpu_mat) override;
#endif
        [[nodiscard]] std::string getRtspIpAddress() const;;
        [[nodiscard]] std::string getVideoInfo() const override;;

    private:
        CpuFrameCallback cpu_frame_callback_;
#ifdef OPENCV_CUDA_ENABLED
        GpuFrameCallback gpu_frame_callback_;
#endif
    };

    class MultiRtspManager
    {
        class Impl;
        Impl* impl_;
        MultiRtspManager();

    public:
        using CpuFrameCallback = RtspVideoCapture::CpuFrameCallback;
        void init(const CpuFrameCallback& cpu_callback);
#ifdef OPENCV_CUDA_ENABLED
        using GpuFrameCallback = RtspVideoCapture::GpuFrameCallback;
        void init(const GpuFrameCallback& gpu_callback);
        void init(const CpuFrameCallback& cpu_callback, const GpuFrameCallback& gpu_callback);
#endif
        static MultiRtspManager& getInstance();
        void addStream(const RtspVideoCapture::RtspInfo& rtsp_info) const;
        void stopAll();
        // 禁止拷贝和赋值
        MultiRtspManager(const MultiRtspManager&) = delete;
        MultiRtspManager& operator=(const MultiRtspManager&) = delete;
    };
#endif
} // namespace jade


#define LOG_TRACE() jade::LoggerStream(jade::Logger::Level::S_TRACE,__FILE__,__LINE__)
#define DLL_LOG_TRACE(module) jade::DLLLoggerStream(jade::Logger::Level::S_TRACE, __FILE__, __LINE__,module)


#define LOG_DEBUG() jade::LoggerStream(jade::Logger::Level::S_DEBUG,__FILE__,__LINE__)
#define DLL_LOG_DEBUG(module) jade::DLLLoggerStream(jade::Logger::Level::S_DEBUG, __FILE__, __LINE__,module)

#define LOG_INFO() jade::LoggerStream(jade::Logger::Level::S_INFO,__FILE__,__LINE__)
#define DLL_LOG_INFO(module) jade::DLLLoggerStream(jade::Logger::Level::S_INFO, __FILE__, __LINE__,module)

#define LOG_WARN() jade::LoggerStream(jade::Logger::Level::S_WARNING,__FILE__,__LINE__)
#define DLL_LOG_WARN(module) jade::DLLLoggerStream(jade::Logger::Level::S_WARNING, __FILE__, __LINE__,module)


#define LOG_ERROR() jade::LoggerStream(jade::Logger::Level::S_ERROR,__FILE__,__LINE__)
#define DLL_LOG_ERROR(module) jade::DLLLoggerStream(jade::Logger::Level::S_ERROR, __FILE__, __LINE__,module)
#define DLL_LOG_ERROR_FL(module,file,line) jade::DLLLoggerStream(jade::Logger::Level::S_ERROR, file, line,module)


#define LOG_CRITICAL(exitCode) jade::LoggerStream(jade::Logger::Level::S_CRITICAL,__FILE__,__LINE__,exitCode)
#define DLL_LOG_CRITICAL(module,exitCode) jade::DLLLoggerStream(jade::Logger::Level::S_CRITICAL, __FILE__, __LINE__,module,exitCode)


#define LOG_EXCEPTION(exitCode,ex) jade::LoggerStream(jade::Logger::Level::S_EXCEPTION,__FILE__,__LINE__,exitCode,ex)
#define DLL_EXCEPTION(module,exitCode,ex) jade::DLLLoggerStream(jade::Logger::Level::S_EXCEPTION, __FILE__, __LINE__,module,exitCode,ex)
