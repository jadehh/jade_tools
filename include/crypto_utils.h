/**
# @File     : crypto_utils.h
# @Author   : jade
# @Date     : 2025/8/25 10:55
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : crypto_utils.h
*/
#pragma once
#include <vector>
#include <string>
#include <openssl/evp.h>

class CryptoUtil {
public:
    enum class CryptoMode {
        AES_256_CBC,
        AES_256_ECB,
        AES_128_CBC,
        AES_128_ECB,
        // 可以扩展其他加密算法
    };

    struct CryptoResult {
        bool success;
        std::string message;
        size_t dataSize;
    };

public:
    explicit CryptoUtil(CryptoMode mode = CryptoMode::AES_256_CBC);
    ~CryptoUtil();

    // 设置密钥和IV
    void setKey(const std::vector<unsigned char>& key);
    void setIV(const std::vector<unsigned char>& iv);

    // 加密接口
    CryptoResult encryptDataToMemory(const std::vector<unsigned char>& inputData,std::vector<unsigned char>& outputData) const;

    CryptoResult encryptFileToMemory(const std::string& inputFile,std::vector<unsigned char>& outputData) const;

    [[nodiscard]] CryptoResult encryptFileToFile(const std::string& inputFile,const std::string& outputFile) const;

    // 解密接口
    CryptoResult decryptData(const std::vector<unsigned char>& inputData, std::vector<unsigned char>& outputData) const;

    CryptoResult decryptFileToMemory(const std::string& inputFile,std::vector<unsigned char>& outputData) const;

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
    bool initializeContext();
    void cleanupContext();
    [[nodiscard]] const EVP_CIPHER* getCipher() const;
    CryptoResult processData(const std::vector<unsigned char>& inputData,std::vector<unsigned char>& outputData,bool encrypt) const;
    [[nodiscard]] CryptoResult processFile(const std::string& inputFile,const std::string& outputFile, bool encrypt) const;

private:
    CryptoMode mode_;
    std::vector<unsigned char> key_;
    std::vector<unsigned char> iv_;
    EVP_CIPHER_CTX* ctx_;
};