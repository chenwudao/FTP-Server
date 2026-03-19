/**
 * @file TransferCommands.h
 * @brief 数据传输命令：PASV, LIST, RETR, STOR
 * @author Junxi Chen
 * @date 2026-03-03
 */

#ifndef FTPD_TRANSFER_COMMANDS_H
#define FTPD_TRANSFER_COMMANDS_H

#include "ftpd/ICommand.h"

namespace ftpd {

/**
 * @class PasvCommand
 * @brief PASV 命令 - 进入被动模式
 * 
 * 响应：227 Entering Passive Mode (h1,h2,h3,h4,p1,p2)
 * 其中端口 = p1*256 + p2
 */
class PasvCommand : public ICommand {
public:
    virtual void execute(FTPConnection* conn, const std::string& arg) override;
};

/**
 * @class ListCommand
 * @brief LIST 命令 - 列出目录内容
 * 
 * 响应：
 * - 150 Opening data connection
 * - 226 Transfer complete (传输完成后)
 * - 425 Can't open data connection
 * - 550 Failed to list directory
 */
class ListCommand : public ICommand {
public:
    virtual void execute(FTPConnection* conn, const std::string& arg) override;
};

/**
 * @class RetrCommand
 * @brief RETR 命令 - 下载文件
 * 
 * 响应：
 * - 150 Opening data connection
 * - 226 Transfer complete
 * - 425 Can't open data connection
 * - 550 File not found
 */
class RetrCommand : public ICommand {
public:
    virtual void execute(FTPConnection* conn, const std::string& arg) override;
};

/**
 * @class StorCommand
 * @brief STOR 命令 - 上传文件
 * 
 * 响应：
 * - 150 Opening data connection
 * - 226 Transfer complete
 * - 425 Can't open data connection
 * - 550 Can't create file
 */
class StorCommand : public ICommand {
public:
    virtual void execute(FTPConnection* conn, const std::string& arg) override;
};

} // namespace ftpd

#endif // FTPD_TRANSFER_COMMANDS_H
