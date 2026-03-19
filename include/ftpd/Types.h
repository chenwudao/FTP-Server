/**
 * @file Types.h
 * @brief FTP服务器通用类型定义
 * @author Junxi Chen
 * @date 2026-03-03
 */

#ifndef FTPD_TYPES_H
#define FTPD_TYPES_H

#include <string>

namespace ftpd {

/**
 * @enum AuthState
 * @brief 客户端认证状态
 */
enum class AuthState {
    WAITING_FOR_USER,  ///< 等待USER命令
    WAITING_FOR_PASS,  ///< 等待PASS命令
    AUTHENTICATED      ///< 已认证
};

/**
 * @enum TransferMode
 * @brief 数据传输模式
 */
enum class TransferMode {
    ASCII,   ///< ASCII模式（需换行符转换）
    BINARY   ///< 二进制模式（原样传输）
};

/**
 * @struct FTPResponse
 * @brief FTP响应结构
 */
struct FTPResponse {
    int code;              ///< FTP响应码
    std::string message;   ///< 响应消息
    
    /**
     * @brief 构造函数
     * @param c 响应码
     * @param msg 响应消息
     */
    FTPResponse(int c = 0, const std::string& msg = "") 
        : code(c), message(msg) {}
};

} // namespace ftpd

#endif // FTPD_TYPES_H
