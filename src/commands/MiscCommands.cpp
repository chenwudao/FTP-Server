/**
 * @file MiscCommands.cpp
 * @brief 其他命令实现
 */

#include "ftpd/commands/MiscCommands.h"
#include "ftpd/FTPConnection.h"

namespace ftpd {

void QuitCommand::execute(FTPConnection* conn, const std::string& arg) {
    conn->send_response(221, "Goodbye");
    // handle_close 会在读取到关闭事件时被调用
    conn->handle_close(conn->get_handle(), 0);
}

} // namespace ftpd
