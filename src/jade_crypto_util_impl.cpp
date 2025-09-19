/**
# @File     : jade_crypto_util_impl.cpp
# @Author   : jade
# @Date     : 2025/9/3 14:10
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : jade_crypto_util_impl.cpp
*/
#if defined(__has_include)
#  if __has_include( <openssl/evp.h>)  // 标准化的头文件存在性检查
#include <openssl/rand.h>
#include "include/crypto_utils.h"
#include <openssl/evp.h>
#    define OPENSSL_ENABLED 1
#  endif
#endif
#include "include/jade_tools.h"
using namespace jade;
namespace jade
{
    class CryptoUtilImpl{
    public:
#ifdef OPENSSL_ENABLED
        CryptoUtil* crypto;
        CryptoUtilImpl(): crypto(nullptr){};
        ~CryptoUtilImpl()
        {
            delete crypto;
        };
#endif

    };
}

JadeCryptoUtil::CryptoResult JadeCryptoUtil::decryptData(const std::vector<unsigned char>& inputData, std::vector<unsigned char>& outputData) const
{
#ifdef OPENSSL_ENABLED
    return crypto_util_impl_->crypto->decryptData(inputData, outputData);
#else
    return CryptoResult{};
#endif

}

JadeCryptoUtil::CryptoResult JadeCryptoUtil::decryptFileToFile(const std::string& inputFile, const std::string& outputFile) const
{
#ifdef OPENSSL_ENABLED
    return crypto_util_impl_->crypto->decryptFileToFile(inputFile, outputFile);
#else
    return CryptoResult{};
#endif
}

JadeCryptoUtil::CryptoResult JadeCryptoUtil::decryptFileToMemory(const std::string& inputFile, std::vector<unsigned char>& outputData) const
{
#ifdef OPENSSL_ENABLED
    return crypto_util_impl_->crypto->decryptFileToMemory(inputFile, outputData);
#else
    return CryptoResult{};
#endif

}

JadeCryptoUtil::CryptoResult JadeCryptoUtil::encryptDataToMemory(const std::vector<unsigned char>& inputData, std::vector<unsigned char>& outputData) const
{
#ifdef OPENSSL_ENABLED
    return crypto_util_impl_->crypto->encryptDataToMemory(inputData, outputData);
#else
    return CryptoResult{};
#endif
}

JadeCryptoUtil::CryptoResult JadeCryptoUtil::encryptFileToFile(const std::string& inputFile, const std::string& outputFile) const
{
#ifdef OPENSSL_ENABLED
    return crypto_util_impl_->crypto->encryptFileToFile(inputFile, outputFile);
#else
    return CryptoResult{};
#endif
}

JadeCryptoUtil::CryptoResult JadeCryptoUtil::encryptFileToMemory(const std::string& inputFile, std::vector<unsigned char>& outputData) const
{
#ifdef OPENSSL_ENABLED
    return crypto_util_impl_->crypto->encryptFileToMemory(inputFile, outputData);
#else
    return CryptoResult{};
#endif
}

std::vector<unsigned char> JadeCryptoUtil::generateRandomIV(const CryptoMode mode)
{
    size_t ivSize = 0;
    switch (mode) {
    case CryptoMode::AES_256_CBC:
    case CryptoMode::AES_128_CBC:
        ivSize = 16; break;
    case CryptoMode::AES_256_ECB:
    case CryptoMode::AES_128_ECB:
        return {}; // ECB模式不需要IV
    default:
        throw std::invalid_argument("Unsupported crypto mode");
    }

    std::vector<unsigned char> iv(ivSize);
#ifdef OPENSSL_ENABLED
    if (RAND_bytes(iv.data(), static_cast<int>(ivSize)) != 1) {
        throw std::runtime_error("Failed to generate random IV");
    }
#endif

    return iv;
}

std::vector<unsigned char> JadeCryptoUtil::generateRandomKey(CryptoMode mode)
{
    size_t keySize = 0;
    switch (mode) {
    case CryptoMode::AES_256_CBC:
    case CryptoMode::AES_256_ECB:
        keySize = 32; break;
    case CryptoMode::AES_128_CBC:
    case CryptoMode::AES_128_ECB:
        keySize = 16; break;
    default:
        throw std::invalid_argument("Unsupported crypto mode");
    }

    std::vector<unsigned char> key(keySize);
#ifdef OPENSSL_ENABLED
    if (RAND_bytes(key.data(), static_cast<int>(keySize)) != 1) {
        throw std::runtime_error("Failed to generate random key");
    }
#endif
    return key;
}

std::string JadeCryptoUtil::getModeName(CryptoMode mode)
{
    switch (mode) {
    case CryptoMode::AES_256_CBC: return "AES-256-CBC";
    case CryptoMode::AES_256_ECB: return "AES-256-ECB";
    case CryptoMode::AES_128_CBC: return "AES-128-CBC";
    case CryptoMode::AES_128_ECB: return "AES-128-ECB";
    default: return "Unknown";
    }
}

JadeCryptoUtil::JadeCryptoUtil(const CryptoMode mode):mode_(mode), crypto_util_impl_(new CryptoUtilImpl())
{
}

void JadeCryptoUtil::setIV(const std::vector<unsigned char>& iv) const
{
#ifdef OPENSSL_ENABLED
    crypto_util_impl_->crypto->setIV(iv);
#endif

}


void JadeCryptoUtil::setKey(const std::vector<unsigned char>& key) const
{
#ifdef OPENSSL_ENABLED
    crypto_util_impl_->crypto->setKey(key);
#endif
}

bool JadeCryptoUtil::validateIVSize(CryptoMode mode, size_t ivSize)
{
    switch (mode) {
    case CryptoMode::AES_256_CBC:
    case CryptoMode::AES_128_CBC:
        return ivSize == 16; // 128位 = 16字节
    case CryptoMode::AES_256_ECB:
    case CryptoMode::AES_128_ECB:
        return ivSize == 0; // ECB模式不需要IV
    default:
        return false;
    }
}

bool JadeCryptoUtil::validateKeySize(CryptoMode mode, size_t keySize)
{
    switch (mode) {
    case CryptoMode::AES_256_CBC:
    case CryptoMode::AES_256_ECB:
        return keySize == 32; // 256位 = 32字节
    case CryptoMode::AES_128_CBC:
    case CryptoMode::AES_128_ECB:
        return keySize == 16; // 128位 = 16字节
    default:
        return false;
    }
}

JadeCryptoUtil::~JadeCryptoUtil()
{
    delete crypto_util_impl_;
}















