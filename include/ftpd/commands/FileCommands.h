/**
 * @file FileCommands.h
 * @brief 文件操作命令：DELE, SIZE, TYPE
 * @author Junxi Chen
 * @date 2026-03-03
 */

#ifndef FTPD_FILE_COMMANDS_H
#define FTPD_FILE_COMMANDS_H

#include "ftpd/ICommand.h"

namespace ftpd {

/**
 * @class DeleCommand
 * @brief DELE 命令 - 删除文件
 * 
 * 响应：
 * - 250 DELE command successful
 * - 550 Delete operation failed
 */
class DeleCommand : public ICommand {
public:
    virtual void execute(FTPConnection* conn, const std::string& arg) override;
};

/**
 * @class SizeCommand
 * @brief SIZE 命令 - 返回文件大小
 * 
 * 响应：
 * - 213 <size>
 * - 550 File not found
 */
class SizeCommand : public ICommand {
public:
    virtual void execute(FTPConnection* conn, const std::string& arg) override;
};

/**
 * @class TypeCommand
 * @brief TYPE 命令 - 设置传输类型
 * 
 * 支持参数：A (ASCII), I (Binary/Image)
 * 响应：200 Type set to <A/I>
 */
class TypeCommand : public ICommand {
public:
    virtual void execute(FTPConnection* conn, const std::string& arg) override;
};

} // namespace ftpd

#endif // FTPD_FILE_COMMANDS_H
