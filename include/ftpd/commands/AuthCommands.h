/**
 * @file AuthCommands.h
 * @brief 认证相关命令：USER, PASS
 * @author Junxi Chen
 * @date 2026-03-03
 */

#ifndef FTPD_AUTH_COMMANDS_H
#define FTPD_AUTH_COMMANDS_H

#include "ftpd/ICommand.h"

namespace ftpd {

/**
 * @class UserCommand
 * @brief USER 命令 - 接收用户名
 * 
 * 响应：
 * - 331 User name okay, need password
 * - 530 Not logged in (用户不存在或拒绝)
 */
class UserCommand : public ICommand {
public:
    virtual void execute(FTPConnection* conn, const std::string& arg) override;
};

/**
 * @class PassCommand
 * @brief PASS 命令 - 验证密码
 * 
 * 响应：
 * - 230 User logged in, proceed
 * - 530 Not logged in (密码错误)
 * - 503 Bad sequence of commands (未先发送USER)
 */
class PassCommand : public ICommand {
public:
    virtual void execute(FTPConnection* conn, const std::string& arg) override;
};

} // namespace ftpd

#endif // FTPD_AUTH_COMMANDS_H
