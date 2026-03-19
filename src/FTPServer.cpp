/**
 * @file FTPServer.cpp
 * @brief FTP服务器主类实现
 */

#include "ftpd/FTPServer.h"
#include "ftpd/FTPConnection.h"
#include <ace/Log_Msg.h>
#include <ace/Reactor.h>
#include <fstream>
#include <sstream>

namespace ftpd {

FTPServer::FTPServer() {
}

FTPServer::~FTPServer() {
    // 清理资源
    acceptor_.close();
}

int FTPServer::open(int port, const std::string& root_dir, 
                   const std::string& passwd_file) {
    root_dir_ = root_dir;
    
    // 加载密码文件
    if (!passwd_file.empty()) {
        if (load_passwd_file(passwd_file) != 0) {
            ACE_ERROR_RETURN((LM_ERROR, "Failed to load password file: %s\n", 
                            passwd_file.c_str()), -1);
        }
    }
    
    // 绑定端口
    ACE_INET_Addr addr(port);
    if (acceptor_.open(addr, 1) == -1) {
        ACE_ERROR_RETURN((LM_ERROR, "Failed to bind port %d: %p\n", 
                        port, "acceptor_.open"), -1);
    }
    
    // 注册到Reactor
    if (ACE_Reactor::instance()->register_handler(
            this, ACE_Event_Handler::ACCEPT_MASK) == -1) {
        ACE_ERROR_RETURN((LM_ERROR, "Failed to register acceptor: %p\n",
                        "register_handler"), -1);
    }
    
    return 0;
}

int FTPServer::run() {
    return ACE_Reactor::instance()->run_reactor_event_loop();
}

void FTPServer::stop() {
    ACE_Reactor::instance()->end_reactor_event_loop();
}

bool FTPServer::authenticate(const std::string& username, 
                            const std::string& password) const {
    auto it = users_.find(username);
    if (it == users_.end()) {
        return false;
    }
    return it->second == password;
}

int FTPServer::handle_input(ACE_HANDLE) {
    ACE_SOCK_Stream new_stream;
    ACE_INET_Addr peer_addr;
    
    // 接受新连接
    if (acceptor_.accept(new_stream, &peer_addr) == -1) {
        ACE_ERROR_RETURN((LM_ERROR, "Failed to accept connection: %p\n",
                        "acceptor_.accept"), 0);
    }
    
    char peer_name[256];
    peer_addr.addr_to_string(peer_name, sizeof(peer_name));
    ACE_DEBUG((LM_INFO, "New connection from: %s\n", peer_name));
    
    // 创建FTPConnection
    FTPConnection* conn = new FTPConnection();
    if (conn->open(new_stream, this) != 0) {
        ACE_ERROR((LM_ERROR, "Failed to initialize connection\n"));
        delete conn;
        return 0;
    }
    
    connections_.push_back(conn);
    return 0;
}

int FTPServer::handle_close(ACE_HANDLE, ACE_Reactor_Mask) {
    acceptor_.close();
    return -1;
}

ACE_HANDLE FTPServer::get_handle() const {
    return acceptor_.get_handle();
}

int FTPServer::load_passwd_file(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file) {
        return -1;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        size_t pos = line.find(':');
        if (pos == std::string::npos) continue;
        
        std::string username = line.substr(0, pos);
        std::string password = line.substr(pos + 1);
        
        users_[username] = password;
        ACE_DEBUG((LM_DEBUG, "Loaded user: %s\n", username.c_str()));
    }
    
    return 0;
}

} // namespace ftpd
