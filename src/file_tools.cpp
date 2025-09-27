/**
# @File     : file_tools.cpp
# @Author   : jade
# @Date     : 2025/7/31 13:14
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : file_tools.cpp
*/
//
#include "include/jade_tools.h"
#include <fstream>
#include <iostream>
#ifdef LOW_GCC
#include <experimental/filesystem>
using namespace std::experimental::filesystem;
#else
#include <filesystem>
using namespace std::filesystem;
#endif
using namespace std;
using namespace jade;

[[maybe_unused]] bool FileTools::createDirectory(const std::string& path)
{
    try
    {
        const string fixedPath = fixPath(path);
        if (exists(fixedPath))
        {
            getLastErrorRef() = "Directory already exists";
            return false;
        }
        return create_directory(fixedPath);
    }
    catch (const std::exception& e)
    {
        getLastErrorRef() = e.what();
        return false;
    }
}

bool FileTools::createDirectories(const std::string& path)
{
    try
    {
        const string fixedPath = fixPath(path);
        if (exists(fixedPath))
        {
            getLastErrorRef() = "Directory already exists";
            return false;
        }
        return create_directories(fixedPath);
    }
    catch (const std::exception& e)
    {
        getLastErrorRef() = e.what();
        return false;
    }
}

[[maybe_unused]] bool FileTools::isExists(const std::string& path)
{
    try
    {
        const string fixedPath = fixPath(path);
        return exists(fixedPath) && is_directory(fixedPath);
    }
    catch (const std::exception& e)
    {
        getLastErrorRef() = e.what();
        return false;
    }
}


[[maybe_unused]] bool FileTools::remove(const std::string& path)
{
    try
    {
        const string fixedPath = fixPath(path);
        if (!exists(fixedPath))
        {
            getLastErrorRef() = "Directory does not exist";
            return false;
        }
        return remove_all(fixedPath) > 0;
    }
    catch (const std::exception& e)
    {
        getLastErrorRef() = e.what();
        return false;
    }
}

string FileTools::fixPath(const std::string& path)
{
#ifdef _WIN32
    std::string fixed = path;
    std::replace(fixed.begin(), fixed.end(), '/', '\\'); // 替换所有 `/` 为 `\`
    return fixed;
#else
    return path; // Linux/Unix 无需修改
#endif
}

[[maybe_unused]] string FileTools::getLastError()
{
    return getLastErrorRef();
}


[[maybe_unused]] vector<string> FileTools::getImageFiles(const std::string& path, const bool fullPath)
{
    std::vector<std::string> imageFiles;
    try
    {
        for (const auto& entry : directory_iterator(u8path(fixPath(path))))
        {
#ifdef LOW_GCC
            if (is_regular_file(entry) && isImageFile(entry.path().u8string()))
            {
                if (fullPath)
                {
                    imageFiles.push_back(entry.path().u8string());
                }
                else
                {
                    imageFiles.push_back(entry.path().filename().u8string());
                }
            }
#else
            if (entry.is_regular_file() && isImageFile(entry.path().u8string())) {
                if (fullPath) {
                    imageFiles.push_back(entry.path().u8string());
                } else {
                    imageFiles.push_back(entry.path().filename().u8string());
                }
            }
#endif
        }
    }
    catch (const filesystem_error& e)
    {
        getLastErrorRef() = e.what();
    }
    return imageFiles;
}

[[maybe_unused]] bool FileTools::writeBinaryToFile(const std::string& path, const float* data, const int size)
{
    // 写入二进制文件
    std::ofstream outFile(path, ios::out | ios::binary);
    if (!outFile.write(reinterpret_cast<const char*>(data), size * sizeof(float)))
    {
        std::cerr << "文件路径为:" << path << "写入文件失败！" << std::endl;
        return false;
    }
    outFile.close();
    return true;
}

[[maybe_unused]] bool FileTools::writeBinaryToFile(const std::string& path, const char* data, const int size)
{
    // 写入二进制文件
    std::ofstream outFile(path, ios::out | ios::binary);
    if (!outFile.write(data, size))
    {
        std::cerr << "文件路径为:" << path << "写入文件失败！" << std::endl;
        return false;
    }
    outFile.close();
    return true;
}
