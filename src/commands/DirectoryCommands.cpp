/**
 * @file DirectoryCommands.cpp
 * @brief 目录操作命令实现
 */

#include "ftpd/commands/DirectoryCommands.h"
#include "ftpd/FTPConnection.h"
#include "ftpd/FileSystem.h"
#include <sstream>

namespace ftpd {

void PwdCommand::execute(FTPConnection* conn, const std::string& arg) {
    if (conn->get_auth_state() != AuthState::AUTHENTICATED) {
        conn->send_response(530, "Not logged in");
        return;
    }
    
    std::ostringstream oss;
    oss << "\"" << conn->get_work_dir() << "\"";
    conn->send_response(257, oss.str());
}

void CwdCommand::execute(FTPConnection* conn, const std::string& arg) {
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
        conn->send_response(550, "Failed to change directory");
        return;
    }
    
    if (!fs->exists(real_path) || !fs->is_directory(real_path)) {
        conn->send_response(550, "Failed to change directory");
        return;
    }
    
    // 更新工作目录
    if (arg[0] == '/') {
        conn->set_work_dir(arg);
    } else {
        std::string new_dir = conn->get_work_dir();
        if (new_dir.back() != '/') new_dir += '/';
        new_dir += arg;
        conn->set_work_dir(new_dir);
    }
    
    conn->send_response(250, "CWD command successful");
}

void MkdCommand::execute(FTPConnection* conn, const std::string& arg) {
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
        conn->send_response(550, "Create directory failed");
        return;
    }
    
    if (fs->create_directory(real_path)) {
        std::ostringstream oss;
        oss << "\"" << arg << "\" created";
        conn->send_response(257, oss.str());
    } else {
        conn->send_response(550, "Create directory failed");
    }
}

void RmdCommand::execute(FTPConnection* conn, const std::string& arg) {
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
        conn->send_response(550, "Remove directory failed");
        return;
    }
    
    if (fs->remove_directory(real_path)) {
        conn->send_response(250, "RMD command successful");
    } else {
        conn->send_response(550, "Remove directory failed");
    }
}

} // namespace ftpd
