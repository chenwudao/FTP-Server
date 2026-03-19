/**
 * @file FTPConnection.cpp
 * @brief FTP客户端连接实现
 */

#include "ftpd/FTPConnection.h"
#include "ftpd/FTPServer.h"
#include "ftpd/commands/AuthCommands.h"
#include "ftpd/commands/DirectoryCommands.h"
#include "ftpd/commands/FileCommands.h"
#include "ftpd/commands/TransferCommands.h"
#include "ftpd/commands/MiscCommands.h"
#include <ace/Log_Msg.h>
#include <ace/Reactor.h>
#include <sstream>

namespace ftpd {

FTPConnection::FTPConnection() 
    : server_(nullptr),
      auth_state_(AuthState::WAITING_FOR_USER),
      work_dir_("/"),
      transfer_mode_(TransferMode::BINARY) {
}

FTPConnection::~FTPConnection() {
    control_stream_.close();
}

int FTPConnection::open(ACE_SOCK_Stream& stream, FTPServer* server) {
    control_stream_ = stream;
    server_ = server;
    
    // 创建文件系统操作对象
    filesystem_.reset(new FileSystem(server->get_root_dir()));
    
    // 获取客户端地址
    control_stream_.get_remote_addr(peer_addr_);
    
    // 注册到Reactor
    if (ACE_Reactor::instance()->register_handler(
            this, ACE_Event_Handler::READ_MASK) == -1) {
        ACE_ERROR_RETURN((LM_ERROR, "Failed to register connection: %p\n",
                        "register_handler"), -1);
    }
    
    // 初始化命令映射
    init_commands();
    
    // 发送欢迎消息
    send_response(220, "Welcome to ftpd FTP server");
    
    return 0;
}

int FTPConnection::send_response(int code, const std::string& message) {
    std::ostringstream oss;
    oss << code << " " << message << "\r\n";
    std::string response = oss.str();
    
    ssize_t sent = control_stream_.send_n(response.c_str(), response.length());
    if (sent == -1) {
        ACE_ERROR_RETURN((LM_ERROR, "Failed to send response: %p\n",
                        "send_n"), -1);
    }
    
    ACE_DEBUG((LM_DEBUG, "Sent: %s", response.c_str()));
    return 0;
}

int FTPConnection::send_response(const FTPResponse& response) {
    return send_response(response.code, response.message);
}

DataConnection* FTPConnection::create_data_connection() {
    if (data_conn_) {
        data_conn_->close();
    }
    data_conn_.reset(new DataConnection());
    return data_conn_.get();
}

void FTPConnection::close_data_connection() {
    if (data_conn_) {
        data_conn_->close();
        data_conn_.reset();
    }
}

int FTPConnection::handle_input(ACE_HANDLE) {
    char buffer[1024];
    ssize_t bytes = control_stream_.recv(buffer, sizeof(buffer) - 1);
    
    if (bytes <= 0) {
        ACE_DEBUG((LM_INFO, "Client disconnected\n"));
        return -1;
    }
    
    buffer[bytes] = '\0';
    recv_buffer_ += buffer;
    
    // 处理所有完整的命令行（以\r\n结尾）
    size_t pos;
    while ((pos = recv_buffer_.find("\r\n")) != std::string::npos) {
        std::string cmd_line = recv_buffer_.substr(0, pos);
        recv_buffer_.erase(0, pos + 2);
        
        if (!cmd_line.empty()) {
            ACE_DEBUG((LM_DEBUG, "Received: %s\n", cmd_line.c_str()));
            dispatch_command(cmd_line);
        }
    }
    
    return 0;
}

int FTPConnection::handle_close(ACE_HANDLE fd, ACE_Reactor_Mask) {
    ACE_DEBUG((LM_INFO, "Closing connection\n"));
    
    if (data_conn_) {
        data_conn_->close();
    }
    
    ACE_Reactor::instance()->remove_handler(this, 
        ACE_Event_Handler::READ_MASK | ACE_Event_Handler::DONT_CALL);
    
    control_stream_.close();
    delete this;  // 自我销毁
    return 0;
}

ACE_HANDLE FTPConnection::get_handle() const {
    return control_stream_.get_handle();
}

void FTPConnection::dispatch_command(const std::string& cmd_line) {
    // 解析命令和参数
    std::istringstream iss(cmd_line);
    std::string cmd, arg;
    iss >> cmd;
    std::getline(iss >> std::ws, arg);
    
    // 转换为大写
    for (char& c : cmd) {
        c = std::toupper(c);
    }
    
    // 查找并执行命令
    auto it = cmd_map_.find(cmd);
    if (it != cmd_map_.end()) {
        it->second->execute(this, arg);
    } else {
        send_response(502, "Command not implemented");
    }
}

void FTPConnection::init_commands() {
    // 注册所有命令
    cmd_map_["USER"] = std::make_shared<UserCommand>();
    cmd_map_["PASS"] = std::make_shared<PassCommand>();
    cmd_map_["PWD"] = std::make_shared<PwdCommand>();
    cmd_map_["CWD"] = std::make_shared<CwdCommand>();
    cmd_map_["MKD"] = std::make_shared<MkdCommand>();
    cmd_map_["RMD"] = std::make_shared<RmdCommand>();
    cmd_map_["DELE"] = std::make_shared<DeleCommand>();
    cmd_map_["SIZE"] = std::make_shared<SizeCommand>();
    cmd_map_["TYPE"] = std::make_shared<TypeCommand>();
    cmd_map_["PASV"] = std::make_shared<PasvCommand>();
    cmd_map_["LIST"] = std::make_shared<ListCommand>();
    cmd_map_["RETR"] = std::make_shared<RetrCommand>();
    cmd_map_["STOR"] = std::make_shared<StorCommand>();
    cmd_map_["QUIT"] = std::make_shared<QuitCommand>();
}

} // namespace ftpd
