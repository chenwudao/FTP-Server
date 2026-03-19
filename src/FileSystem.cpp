/**
 * @file FileSystem.cpp
 * @brief 文件系统操作实现
 */

#include "ftpd/FileSystem.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <ctime>
#include <cstring>
#include <sstream>
#include <algorithm>

namespace ftpd {

FileSystem::FileSystem(const std::string& root_dir) 
    : root_dir_(root_dir) {
    // 确保根目录以 / 结尾
    if (!root_dir_.empty() && root_dir_.back() != '/') {
        root_dir_ += '/';
    }
}

bool FileSystem::resolve_path(const std::string& work_dir, 
                             const std::string& path,
                             std::string& out_real_path) const {
    std::string full_path;
    
    // 如果是绝对路径（从根目录开始）
    if (!path.empty() && path[0] == '/') {
        full_path = path;
    } else {
        // 相对路径，相对于当前工作目录
        full_path = work_dir;
        if (full_path.back() != '/') full_path += '/';
        full_path += path;
    }
    
    // 规范化路径：处理 . 和 ..
    std::vector<std::string> parts;
    std::istringstream iss(full_path);
    std::string part;
    
    while (std::getline(iss, part, '/')) {
        if (part.empty() || part == ".") {
            continue;
        } else if (part == "..") {
            if (!parts.empty()) {
                parts.pop_back();
            }
        } else {
            parts.push_back(part);
        }
    }
    
    // 构建规范化路径
    std::string normalized = "/";
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i > 0) normalized += "/";
        normalized += parts[i];
    }
    
    // 转换为真实文件系统路径
    out_real_path = root_dir_;
    if (normalized != "/" && normalized.size() > 1) {
        out_real_path += normalized.substr(1);  // 去掉开头的 /
    }
    
    // 安全检查：确保路径在根目录内
    if (out_real_path.find(root_dir_) != 0) {
        return false;
    }
    
    return true;
}

bool FileSystem::exists(const std::string& real_path) const {
    struct stat st;
    return stat(real_path.c_str(), &st) == 0;
}

bool FileSystem::is_directory(const std::string& real_path) const {
    struct stat st;
    if (stat(real_path.c_str(), &st) != 0) {
        return false;
    }
    return S_ISDIR(st.st_mode);
}

ssize_t FileSystem::get_file_size(const std::string& real_path) const {
    struct stat st;
    if (stat(real_path.c_str(), &st) != 0) {
        return -1;
    }
    return st.st_size;
}

bool FileSystem::list_directory(const std::string& real_path, 
                               std::vector<FileInfo>& out_files) const {
    DIR* dir = opendir(real_path.c_str());
    if (!dir) {
        return false;
    }
    
    out_files.clear();
    struct dirent* entry;
    
    while ((entry = readdir(dir)) != nullptr) {
        // 跳过 . 和 ..
        if (std::strcmp(entry->d_name, ".") == 0 || 
            std::strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        std::string full_path = real_path;
        if (full_path.back() != '/') full_path += '/';
        full_path += entry->d_name;
        
        struct stat st;
        if (stat(full_path.c_str(), &st) != 0) {
            continue;
        }
        
        FileInfo info;
        info.name = entry->d_name;
        info.is_directory = S_ISDIR(st.st_mode);
        info.size = st.st_size;
        
        // 权限字符串
        info.permissions = "";
        info.permissions += info.is_directory ? 'd' : '-';
        info.permissions += (st.st_mode & S_IRUSR) ? 'r' : '-';
        info.permissions += (st.st_mode & S_IWUSR) ? 'w' : '-';
        info.permissions += (st.st_mode & S_IXUSR) ? 'x' : '-';
        info.permissions += (st.st_mode & S_IRGRP) ? 'r' : '-';
        info.permissions += (st.st_mode & S_IWGRP) ? 'w' : '-';
        info.permissions += (st.st_mode & S_IXGRP) ? 'x' : '-';
        info.permissions += (st.st_mode & S_IROTH) ? 'r' : '-';
        info.permissions += (st.st_mode & S_IWOTH) ? 'w' : '-';
        info.permissions += (st.st_mode & S_IXOTH) ? 'x' : '-';
        
        // 所有者和组
        struct passwd* pw = getpwuid(st.st_uid);
        struct group* gr = getgrgid(st.st_gid);
        info.owner = pw ? pw->pw_name : "unknown";
        info.group = gr ? gr->gr_name : "unknown";
        
        // 修改时间
        char time_buf[64];
        struct tm* tm_info = localtime(&st.st_mtime);
        strftime(time_buf, sizeof(time_buf), "%b %d %H:%M", tm_info);
        info.mtime = time_buf;
        
        out_files.push_back(info);
    }
    
    closedir(dir);
    return true;
}

bool FileSystem::create_directory(const std::string& real_path) const {
    return mkdir(real_path.c_str(), 0755) == 0;
}

bool FileSystem::remove_directory(const std::string& real_path) const {
    return rmdir(real_path.c_str()) == 0;
}

bool FileSystem::delete_file(const std::string& real_path) const {
    return unlink(real_path.c_str()) == 0;
}

} // namespace ftpd
