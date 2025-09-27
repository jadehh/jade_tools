/**
# @File     : crypto_utils.cpp
# @Author   : jade
# @Date     : 2025/8/25 10:56
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : crypto_utils.cpp
*/
#include "include/jade_tools.h"
#include "include/crypto_utils.h"
#include <fstream>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <stdexcept>
#include <random>

CryptoUtil::CryptoUtil(const jade::JadeCryptoUtil::CryptoMode mode) :
    mode_(mode), ctx_(nullptr)
{
    initializeContext();
}

CryptoUtil::~CryptoUtil()
{
    cleanupContext();
}

void CryptoUtil::setKey(const std::vector<unsigned char>& key)
{
    if (!jade::JadeCryptoUtil::validateKeySize(mode_, key.size()))
    {
        throw std::invalid_argument("Invalid key size for selected mode");
    }
    key_ = key;
}

void CryptoUtil::setIV(const std::vector<unsigned char>& iv)
{
    if (!jade::JadeCryptoUtil::validateIVSize(mode_, iv.size()))
    {
        throw std::invalid_argument("Invalid IV size for selected mode");
    }
    iv_ = iv;
}

bool CryptoUtil::initializeContext()
{
    cleanupContext();
    ctx_ = EVP_CIPHER_CTX_new();
    return ctx_ != nullptr;
}

void CryptoUtil::cleanupContext()
{
    if (ctx_)
    {
        EVP_CIPHER_CTX_free(ctx_);
        ctx_ = nullptr;
    }
}

const EVP_CIPHER* CryptoUtil::getCipher() const
{
    switch (mode_)
    {
    case jade::JadeCryptoUtil::CryptoMode::AES_256_CBC:
        return EVP_aes_256_cbc();
    case jade::JadeCryptoUtil::CryptoMode::AES_256_ECB:
        return EVP_aes_256_ecb();
    case jade::JadeCryptoUtil::CryptoMode::AES_128_CBC:
        return EVP_aes_128_cbc();
    case jade::JadeCryptoUtil::CryptoMode::AES_128_ECB:
        return EVP_aes_128_ecb();
    default:
        return EVP_aes_256_cbc();
    }
}

jade::JadeCryptoUtil::CryptoResult CryptoUtil::processData(
    const std::vector<unsigned char>& inputData,
    std::vector<unsigned char>& outputData,
    bool encrypt) const
{
    jade::JadeCryptoUtil::CryptoResult result{false, "", 0};

    if (key_.empty())
    {
        result.message = encrypt ? "未设置加密密钥" : "未设置解密密钥";
        return result;
    }

    if (!ctx_)
    {
        result.message = "加解密上下文初始化失败";
        return result;
    }

    try
    {
        // 修正后的初始化代码
        int (*init_func)(EVP_CIPHER_CTX*, const EVP_CIPHER*,
                         ENGINE*, const unsigned char*,
                         const unsigned char*) =
            encrypt ? EVP_EncryptInit_ex : EVP_DecryptInit_ex;

        if (init_func(ctx_, getCipher(), nullptr, key_.data(), iv_.data()) != 1)
        {
            result.message = encrypt ? "加密初始化失败" : "解密初始化失败";
            return result;
        }


        // 分配缓冲区（加密时需要额外空间用于填充）
        const size_t outputSize = inputData.size() + (encrypt ? EVP_MAX_BLOCK_LENGTH : 0);
        outputData.resize(outputSize);

        int processedLength = 0;
        int finalLength = 0;

        // 处理数据
        int (*update_func)(EVP_CIPHER_CTX*, unsigned char*, int*, const unsigned char*, int) =
            encrypt ? EVP_EncryptUpdate : EVP_DecryptUpdate;
        if (update_func(ctx_, outputData.data(), &processedLength, inputData.data(),
                        static_cast<int>(inputData.size())) != 1)
        {
            result.message = encrypt ? "加密更新失败" : "解密更新失败";
            return result;
        }

        // 完成加解密
        int (*final_func)(EVP_CIPHER_CTX*, unsigned char*, int*) =
            encrypt ? EVP_EncryptFinal_ex : EVP_DecryptFinal_ex;

        if (final_func(ctx_, outputData.data() + processedLength, &finalLength) != 1)
        {
            result.message = encrypt ? "加密最终化失败" : "解密最终化失败";
            return result;
        }

        processedLength += finalLength;
        outputData.resize(processedLength);

        result.success = true;
        result.message = encrypt ? "加密成功" : "解密成功";
        result.dataSize = processedLength;
    }
    catch (const std::exception& e)
    {
        result.message = std::string(encrypt ? "加密过程异常: " : "解密过程异常: ") + e.what();
    }

    return result;
}

jade::JadeCryptoUtil::CryptoResult CryptoUtil::processFile(
    const std::string& inputFile,
    const std::string& outputFile,
    bool encrypt) const
{
    jade::JadeCryptoUtil::CryptoResult result{false, "", 0};

    try
    {
        // 读取输入文件
        std::ifstream inFile(inputFile, std::ios::binary | std::ios::ate);
        if (!inFile.is_open())
        {
            result.message = "无法打开输入文件: " + inputFile;
            return result;
        }

        size_t fileSize = static_cast<size_t>(inFile.tellg());
        inFile.seekg(0, std::ios::beg);

        std::vector<unsigned char> inputData(fileSize);
        if (!inFile.read(reinterpret_cast<char*>(inputData.data()), static_cast<long long>(fileSize)))
        {
            result.message = "读取文件失败: " + inputFile;
            return result;
        }
        inFile.close();

        // 处理数据
        std::vector<unsigned char> outputData;
        result = encrypt ? encryptDataToMemory(inputData, outputData) : decryptData(inputData, outputData);

        if (!result.success)
        {
            return result;
        }

        // 写入输出文件
        std::ofstream outFile(outputFile, std::ios::binary);
        if (!outFile.is_open())
        {
            result.success = false;
            result.message = "无法创建输出文件: " + outputFile;
            return result;
        }

        outFile.write(reinterpret_cast<const char*>(outputData.data()), static_cast<long long>(outputData.size()));
        outFile.close();

        result.success = true;
        result.message = encrypt ? "文件加密成功" : "文件解密成功";
        result.dataSize = outputData.size();
    }
    catch (const std::exception& e)
    {
        result.message = std::string("文件操作失败: ") + e.what();
    }

    return result;
}

// 加密方法实现
jade::JadeCryptoUtil::CryptoResult CryptoUtil::encryptDataToMemory(
    const std::vector<unsigned char>& inputData,
    std::vector<unsigned char>& outputData) const
{
    return processData(inputData, outputData, true);
}

jade::JadeCryptoUtil::CryptoResult CryptoUtil::encryptFileToMemory(
    const std::string& inputFile,
    std::vector<unsigned char>& outputData) const
{
    jade::JadeCryptoUtil::CryptoResult result{false, "", 0};

    try
    {
        std::ifstream file(inputFile, std::ios::binary | std::ios::ate);
        if (!file.is_open())
        {
            result.message = "无法打开输入文件: " + inputFile;
            return result;
        }

        const size_t fileSize = static_cast<size_t>(file.tellg());
        file.seekg(0, std::ios::beg);

        std::vector<unsigned char> inputData(fileSize);
        if (!file.read(reinterpret_cast<char*>(inputData.data()), static_cast<long long>(fileSize)))
        {
            result.message = "读取文件失败: " + inputFile;
            return result;
        }
        file.close();

        return encryptDataToMemory(inputData, outputData);
    }
    catch (const std::exception& e)
    {
        result.message = std::string("加密失败: ") + e.what();
        return result;
    }
}

jade::JadeCryptoUtil::CryptoResult CryptoUtil::encryptFileToFile(
    const std::string& inputFile,
    const std::string& outputFile) const
{
    return processFile(inputFile, outputFile, true);
}

// 解密方法实现
jade::JadeCryptoUtil::CryptoResult CryptoUtil::decryptData(
    const std::vector<unsigned char>& inputData,
    std::vector<unsigned char>& outputData) const
{
    return processData(inputData, outputData, false);
}

jade::JadeCryptoUtil::CryptoResult CryptoUtil::decryptFileToMemory(const std::string& inputFile) const
{
    return processFile(inputFile, "", false); // 修改为使用processFile
}

jade::JadeCryptoUtil::CryptoResult CryptoUtil::decryptFileToFile(
    const std::string& inputFile,
    const std::string& outputFile) const
{
    return processFile(inputFile, outputFile, false);
}


std::vector<unsigned char> CryptoUtil::generateRandomIV(jade::JadeCryptoUtil::CryptoMode mode)
{
    size_t ivSize = 0;
    switch (mode)
    {
    case jade::JadeCryptoUtil::CryptoMode::AES_256_CBC:
    case jade::JadeCryptoUtil::CryptoMode::AES_128_CBC:
        ivSize = 16;
        break;
    case jade::JadeCryptoUtil::CryptoMode::AES_256_ECB:
    case jade::JadeCryptoUtil::CryptoMode::AES_128_ECB:
        return {}; // ECB模式不需要IV
    default:
        throw std::invalid_argument("Unsupported crypto mode");
    }

    std::vector<unsigned char> iv(ivSize);
    if (RAND_bytes(iv.data(), static_cast<int>(ivSize)) != 1)
    {
        throw std::runtime_error("Failed to generate random IV");
    }
    return iv;
}
