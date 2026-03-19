/**
 * @file AuthCommands.cpp
 * @brief 认证命令实现
 */

#include "ftpd/commands/AuthCommands.h"
#include "ftpd/FTPConnection.h"
#include "ftpd/FTPServer.h"

namespace ftpd {

void UserCommand::execute(FTPConnection* conn, const std::string& arg) {
    if (arg.empty()) {
        conn->send_response(501, "Syntax error in parameters or arguments");
        return;
    }
    
    conn->set_username(arg);
    conn->set_auth_state(AuthState::WAITING_FOR_PASS);
    conn->send_response(331, "User name okay, need password");
}

void PassCommand::execute(FTPConnection* conn, const std::string& arg) {
    if (conn->get_auth_state() != AuthState::WAITING_FOR_PASS) {
        conn->send_response(503, "Bad sequence of commands");
        return;
    }
    
    // 验证密码
    FTPServer* server = conn->get_server();
    if (server && server->authenticate(conn->get_username(), arg)) {
        conn->set_auth_state(AuthState::AUTHENTICATED);
        conn->send_response(230, "User logged in, proceed");
    } else {
        conn->set_auth_state(AuthState::WAITING_FOR_USER);
        conn->set_username("");
        conn->send_response(530, "Not logged in");
    }
}

} // namespace ftpd
