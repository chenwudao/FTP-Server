/**
 * @file MiscCommands.h
 * @brief 其他命令：QUIT
 * @author Junxi Chen
 * @date 2026-03-03
 */

#ifndef FTPD_MISC_COMMANDS_H
#define FTPD_MISC_COMMANDS_H

#include "ftpd/ICommand.h"

namespace ftpd {

/**
 * @class QuitCommand
 * @brief QUIT 命令 - 退出FTP会话
 * 
 * 响应：221 Goodbye
 */
class QuitCommand : public ICommand {
public:
    virtual void execute(FTPConnection* conn, const std::string& arg) override;
};

} // namespace ftpd

#endif // FTPD_MISC_COMMANDS_H
