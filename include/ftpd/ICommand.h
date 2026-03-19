/**
 * @file ICommand.h
 * @brief FTP命令接口定义
 * @author Junxi Chen
 * @date 2026-03-03
 */

#ifndef FTPD_ICOMMAND_H
#define FTPD_ICOMMAND_H

#include <string>
#include <memory>

namespace ftpd {

// 前向声明
class FTPConnection;

/**
 * @class ICommand
 * @brief FTP命令接口，所有命令实现必须继承此接口
 * 
 * 提供统一的命令执行接口，支持多态分发。
 * 每个具体命令（如USER、PASS、LIST等）实现execute方法。
 */
class ICommand {
public:
    /**
     * @brief 虚析构函数
     */
    virtual ~ICommand() = default;
    
    /**
     * @brief 执行FTP命令
     * @param conn 客户端连接对象指针
     * @param arg 命令参数（可能为空）
     * 
     * 命令执行过程中应：
     * - 检查连接状态（如认证状态）
     * - 执行具体操作（文件系统、数据传输等）
     * - 调用conn->send_response()发送响应
     */
    virtual void execute(FTPConnection* conn, const std::string& arg) = 0;
};

/// 命令智能指针类型
using CommandPtr = std::shared_ptr<ICommand>;

} // namespace ftpd

#endif // FTPD_ICOMMAND_H
