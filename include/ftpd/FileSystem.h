/**
 * @file FileSystem.h
 * @brief 文件系统操作辅助类
 * @author Junxi Chen
 * @date 2026-03-03
 */

#ifndef FTPD_FILESYSTEM_H
#define FTPD_FILESYSTEM_H

#include <string>
#include <vector>

namespace ftpd {

/**
 * @struct FileInfo
 * @brief 文件/目录信息
 */
struct FileInfo {
    std::string name;        ///< 文件名
    bool is_directory;       ///< 是否为目录
    size_t size;             ///< 文件大小
    std::string permissions; ///< 权限字符串
    std::string owner;       ///< 所有者
    std::string group;       ///< 组
    std::string mtime;       ///< 修改时间
};

/**
 * @class FileSystem
 * @brief 文件系统操作封装
 * 
 * 提供安全的文件系统操作，确保所有路径都在根目录内。
 */
class FileSystem {
public:
    /**
     * @brief 构造函数
     * @param root_dir 根目录（绝对路径）
     */
    explicit FileSystem(const std::string& root_dir);
    
    /**
     * @brief 解析并规范化路径
     * @param work_dir 当前工作目录（相对于根）
     * @param path 要解析的路径（可能是相对或绝对）
     * @param out_real_path 输出：真实绝对路径
     * @return 成功返回true
     * 
     * 防止路径遍历攻击，确保结果路径在根目录内。
     */
    bool resolve_path(const std::string& work_dir, 
                     const std::string& path,
                     std::string& out_real_path) const;
    
    /**
     * @brief 检查路径是否存在
     */
    bool exists(const std::string& real_path) const;
    
    /**
     * @brief 检查是否为目录
     */
    bool is_directory(const std::string& real_path) const;
    
    /**
     * @brief 获取文件大小
     */
    ssize_t get_file_size(const std::string& real_path) const;
    
    /**
     * @brief 列出目录内容
     * @param real_path 真实路径
     * @param out_files 输出文件列表
     * @return 成功返回true
     */
    bool list_directory(const std::string& real_path, 
                       std::vector<FileInfo>& out_files) const;
    
    /**
     * @brief 创建目录
     */
    bool create_directory(const std::string& real_path) const;
    
    /**
     * @brief 删除目录（必须为空）
     */
    bool remove_directory(const std::string& real_path) const;
    
    /**
     * @brief 删除文件
     */
    bool delete_file(const std::string& real_path) const;

private:
    std::string root_dir_;  ///< 根目录
};

} // namespace ftpd

#endif // FTPD_FILESYSTEM_H
