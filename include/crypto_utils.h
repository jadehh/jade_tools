/**
# @File     : crypto_utils.h
# @Author   : jade
# @Date     : 2025/8/25 10:55
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : crypto_utils.h
*/
#pragma once
#include "jade_tools.h"
#include <vector>
#include <string>
#include <openssl/evp.h>

class CryptoUtil {
public:
    explicit CryptoUtil(jade::JadeCryptoUtil::CryptoMode mode = jade::JadeCryptoUtil::CryptoMode::AES_256_CBC);
    ~CryptoUtil();

    // 设置密钥和IV
    void setKey(const std::vector<unsigned char>& key);
    void setIV(const std::vector<unsigned char>& iv);

    // 加密接口
    jade::JadeCryptoUtil::CryptoResult encryptDataToMemory(const std::vector<unsigned char>& inputData,std::vector<unsigned char>& outputData) const;

    jade::JadeCryptoUtil::CryptoResult encryptFileToMemory(const std::string& inputFile,std::vector<unsigned char>& outputData) const;

    [[nodiscard]] jade::JadeCryptoUtil::CryptoResult encryptFileToFile(const std::string& inputFile,const std::string& outputFile) const;

    // 解密接口
    jade::JadeCryptoUtil::CryptoResult decryptData(const std::vector<unsigned char>& inputData, std::vector<unsigned char>& outputData) const;

    jade::JadeCryptoUtil::CryptoResult decryptFileToMemory(const std::string& inputFile,std::vector<unsigned char>& outputData) const;

    [[nodiscard]] jade::JadeCryptoUtil::CryptoResult decryptFileToFile(const std::string& inputFile, const std::string& outputFile) const;

    static std::vector<unsigned char> generateRandomIV(jade::JadeCryptoUtil::CryptoMode mode);

    // 获取当前模式
    [[nodiscard]] jade::JadeCryptoUtil::CryptoMode getMode() const { return mode_; }



private:
    bool initializeContext();
    void cleanupContext();
    [[nodiscard]] const EVP_CIPHER* getCipher() const;
    jade::JadeCryptoUtil::CryptoResult processData(const std::vector<unsigned char>& inputData,std::vector<unsigned char>& outputData,bool encrypt) const;
    [[nodiscard]] jade::JadeCryptoUtil::CryptoResult processFile(const std::string& inputFile,const std::string& outputFile, bool encrypt) const;

private:
    jade::JadeCryptoUtil::CryptoMode mode_;
    std::vector<unsigned char> key_;
    std::vector<unsigned char> iv_;
    EVP_CIPHER_CTX* ctx_;
};