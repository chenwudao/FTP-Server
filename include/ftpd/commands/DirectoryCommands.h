/**
 * @file DirectoryCommands.h
 * @brief 目录操作命令：PWD, CWD, MKD, RMD
 * @author Junxi Chen
 * @date 2026-03-03
 */

#ifndef FTPD_DIRECTORY_COMMANDS_H
#define FTPD_DIRECTORY_COMMANDS_H

#include "ftpd/ICommand.h"

namespace ftpd {

/**
 * @class PwdCommand
 * @brief PWD 命令 - 返回当前工作目录
 * 
 * 响应：257 "<path>"
 */
class PwdCommand : public ICommand {
public:
    virtual void execute(FTPConnection* conn, const std::string& arg) override;
};

/**
 * @class CwdCommand
 * @brief CWD 命令 - 改变工作目录
 * 
 * 响应：
 * - 250 CWD command successful
 * - 550 Failed to change directory
 */
class CwdCommand : public ICommand {
public:
    virtual void execute(FTPConnection* conn, const std::string& arg) override;
};

/**
 * @class MkdCommand
 * @brief MKD 命令 - 创建目录
 * 
 * 响应：
 * - 257 "<path>" created
 * - 550 Create directory failed
 */
class MkdCommand : public ICommand {
public:
    virtual void execute(FTPConnection* conn, const std::string& arg) override;
};

/**
 * @class RmdCommand
 * @brief RMD 命令 - 删除目录
 * 
 * 响应：
 * - 250 RMD command successful
 * - 550 Remove directory failed
 */
class RmdCommand : public ICommand {
public:
    virtual void execute(FTPConnection* conn, const std::string& arg) override;
};

} // namespace ftpd

#endif // FTPD_DIRECTORY_COMMANDS_H
