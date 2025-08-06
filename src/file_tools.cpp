/**
# @File     : file_tools.cpp
# @Author   : jade
# @Date     : 2025/7/31 13:14
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : file_tools.cpp
*/
//
#include "include/file_tools.h"

#include <fstream>
using namespace jade;
bool FileTools::CreateDirectory(const std::string& path)
{
    try {
        const string fixedPath = FixPath(path);
        if (exists(fixedPath)) {
            GetLastErrorRef() = "Directory already exists";
            return false;
        }
        return create_directory(fixedPath);
    } catch (const std::exception& e) {
        GetLastErrorRef() = e.what();
        return false;
    }
}

bool FileTools::CreateDirectories(const std::string& path)
{
    try {
        const string fixedPath = FixPath(path);
        if (exists(fixedPath)) {
            GetLastErrorRef() = "Directory already exists";
            return false;
        }
        return create_directories(fixedPath);
    } catch (const std::exception& e) {
        GetLastErrorRef() = e.what();
        return false;
    }
}

bool FileTools::Exists(const std::string& path)
{
    try {
        const string fixedPath = FixPath(path);
        return exists(fixedPath) && is_directory(fixedPath);
    } catch (const std::exception& e) {
        GetLastErrorRef() = e.what();
        return false;
    }
}

bool FileTools::Remove(const string& path)
{
    try {
        const string fixedPath = FixPath(path);
        if (!exists(fixedPath)) {
            GetLastErrorRef() = "Directory does not exist";
            return false;
        }
        return remove_all(fixedPath) > 0;
    } catch (const std::exception& e) {
        GetLastErrorRef() = e.what();
        return false;
    }
}
string FileTools::FixPath(const string& path)
{
#ifdef _WIN32
    std::string fixed = path;
    std::replace(fixed.begin(), fixed.end(), '/', '\\'); // 替换所有 `/` 为 `\`
    return fixed;
#else
    return path; // Linux/Unix 无需修改
#endif
}

string FileTools::GetLastError()
{
    return GetLastErrorRef();
}


vector<string> FileTools::getImageFiles(const string& path, const bool fullPath)
{
    std::vector<std::string> imageFiles;
    try {
        for (const auto& entry : directory_iterator(u8path(FixPath(path)))) {
            if (entry.is_regular_file() && isImageFile(entry.path().u8string())) {
                if (fullPath) {
                    imageFiles.push_back(entry.path().u8string());
                } else {
                    imageFiles.push_back(entry.path().filename().u8string());
                }
            }
        }
    } catch (const filesystem_error& e) {
        GetLastErrorRef() = e.what();
    }
    return imageFiles;
}

bool FileTools::writeBinaryToFile(const string& path, const float* data, const int size)
{
    // 写入二进制文件
    std::ofstream outFile(path, ios::out | ios::binary);
    if (!outFile.write(reinterpret_cast<const char*>(data), size * sizeof(float))) {
        std::cerr << "文件路径为:" << path << "写入文件失败！" << std::endl;
        return false;
    }
    outFile.close();
    return true;
}

bool FileTools::writeBinaryToFile(const string& path, const char* data, const int size)
{
    // 写入二进制文件
    std::ofstream outFile(path, ios::out | ios::binary);
    if (!outFile.write(data, size)) {
        std::cerr << "文件路径为:" << path << "写入文件失败！" << std::endl;
        return false;
    }
    outFile.close();
    return true;
}




