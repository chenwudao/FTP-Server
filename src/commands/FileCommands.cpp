/**
 * @file FileCommands.cpp
 * @brief 文件操作命令实现
 */

#include "ftpd/commands/FileCommands.h"
#include "ftpd/FTPConnection.h"
#include "ftpd/FileSystem.h"
#include <sstream>

namespace ftpd {

void DeleCommand::execute(FTPConnection* conn, const std::string& arg) {
    if (conn->get_auth_state() != AuthState::AUTHENTICATED) {
        conn->send_response(530, "Not logged in");
        return;
    }
    
    if (arg.empty()) {
        conn->send_response(501, "Syntax error in parameters or arguments");
        return;
    }
    
    FileSystem* fs = conn->get_filesystem();
    std::string real_path;
    
    if (!fs->resolve_path(conn->get_work_dir(), arg, real_path)) {
        conn->send_response(550, "Delete operation failed");
        return;
    }
    
    if (fs->delete_file(real_path)) {
        conn->send_response(250, "DELE command successful");
    } else {
        conn->send_response(550, "Delete operation failed");
    }
}

void SizeCommand::execute(FTPConnection* conn, const std::string& arg) {
    if (conn->get_auth_state() != AuthState::AUTHENTICATED) {
        conn->send_response(530, "Not logged in");
        return;
    }
    
    if (arg.empty()) {
        conn->send_response(501, "Syntax error in parameters or arguments");
        return;
    }
    
    FileSystem* fs = conn->get_filesystem();
    std::string real_path;
    
    if (!fs->resolve_path(conn->get_work_dir(), arg, real_path)) {
        conn->send_response(550, "File not found");
        return;
    }
    
    ssize_t size = fs->get_file_size(real_path);
    if (size < 0) {
        conn->send_response(550, "File not found");
    } else {
        std::ostringstream oss;
        oss << size;
        conn->send_response(213, oss.str());
    }
}

void TypeCommand::execute(FTPConnection* conn, const std::string& arg) {
    if (arg.empty()) {
        conn->send_response(501, "Syntax error in parameters or arguments");
        return;
    }
    
    char type = std::toupper(arg[0]);
    
    if (type == 'A') {
        conn->set_transfer_mode(TransferMode::ASCII);
        conn->send_response(200, "Type set to A");
    } else if (type == 'I') {
        conn->set_transfer_mode(TransferMode::BINARY);
        conn->send_response(200, "Type set to I");
    } else {
        conn->send_response(504, "Command not implemented for that parameter");
    }
}

} // namespace ftpd
