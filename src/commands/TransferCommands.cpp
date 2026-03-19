/**
 * @file TransferCommands.cpp
 * @brief 数据传输命令实现
 */

#include "ftpd/commands/TransferCommands.h"
#include "ftpd/FTPConnection.h"
#include "ftpd/DataConnection.h"
#include "ftpd/FileSystem.h"
#include <ace/INET_Addr.h>
#include <ace/Log_Msg.h>
#include <fstream>
#include <sstream>

namespace ftpd {

void PasvCommand::execute(FTPConnection* conn, const std::string& arg) {
    if (conn->get_auth_state() != AuthState::AUTHENTICATED) {
        conn->send_response(530, "Not logged in");
        return;
    }
    
    // 创建数据连接
    DataConnection* data_conn = conn->create_data_connection();
    if (!data_conn) {
        conn->send_response(425, "Can't open data connection");
        return;
    }
    
    // 监听随机端口（指定端口0，IP地址使用默认）
    ACE_INET_Addr local_addr((u_short)0);  // 端口0表示随机选择
    int port = data_conn->pasv_listen(local_addr);
    
    if (port < 0) {
        conn->send_response(425, "Can't open data connection");
        conn->close_data_connection();
        return;
    }
    
    // 获取本机IP
    ACE_INET_Addr actual_addr;
    data_conn->get_local_addr(actual_addr);
    
    // 构造PASV响应：227 Entering Passive Mode (h1,h2,h3,h4,p1,p2)
    // h1.h2.h3.h4 是IP地址，端口 = p1*256 + p2
    uint32_t ip = actual_addr.get_ip_address();
    int p1 = port / 256;
    int p2 = port % 256;
    
    std::ostringstream oss;
    oss << "Entering Passive Mode ("
        << ((ip >> 24) & 0xFF) << ","
        << ((ip >> 16) & 0xFF) << ","
        << ((ip >> 8) & 0xFF) << ","
        << (ip & 0xFF) << ","
        << p1 << ","
        << p2 << ")";
    
    conn->send_response(227, oss.str());
}

void ListCommand::execute(FTPConnection* conn, const std::string& arg) {
    if (conn->get_auth_state() != AuthState::AUTHENTICATED) {
        conn->send_response(530, "Not logged in");
        return;
    }
    
    DataConnection* data_conn = conn->get_data_connection();
    if (!data_conn) {
        conn->send_response(425, "Use PASV first");
        return;
    }
    
    // 等待数据连接建立（简化实现：阻塞等待）
    // 实际应该在 Reactor 中异步处理
    for (int i = 0; i < 50 && !data_conn->is_connected(); ++i) {
        ACE_OS::sleep(ACE_Time_Value(0, 100000));  // 100ms
    }
    
    if (!data_conn->is_connected()) {
        conn->send_response(425, "Can't open data connection");
        conn->close_data_connection();
        return;
    }
    
    conn->send_response(150, "Opening data connection");
    
    // 确定列出的目录
    std::string list_path = arg.empty() ? conn->get_work_dir() : arg;
    
    FileSystem* fs = conn->get_filesystem();
    std::string real_path;
    
    if (!fs->resolve_path(conn->get_work_dir(), list_path, real_path)) {
        conn->send_response(550, "Failed to list directory");
        conn->close_data_connection();
        return;
    }
    
    std::vector<FileInfo> files;
    if (!fs->list_directory(real_path, files)) {
        conn->send_response(550, "Failed to list directory");
        conn->close_data_connection();
        return;
    }
    
    // 构造列表（Unix ls -l 格式）
    std::ostringstream oss;
    for (const auto& file : files) {
        oss << file.permissions << " 1 "
            << file.owner << " "
            << file.group << " "
            << file.size << " "
            << file.mtime << " "
            << file.name << "\r\n";
    }
    
    std::string list_data = oss.str();
    data_conn->send_data(list_data.c_str(), list_data.length());
    
    conn->close_data_connection();
    conn->send_response(226, "Transfer complete");
}

void RetrCommand::execute(FTPConnection* conn, const std::string& arg) {
    if (conn->get_auth_state() != AuthState::AUTHENTICATED) {
        conn->send_response(530, "Not logged in");
        return;
    }
    
    if (arg.empty()) {
        conn->send_response(501, "Syntax error in parameters or arguments");
        return;
    }
    
    DataConnection* data_conn = conn->get_data_connection();
    if (!data_conn) {
        conn->send_response(425, "Use PASV first");
        return;
    }
    
    FileSystem* fs = conn->get_filesystem();
    std::string real_path;
    
    if (!fs->resolve_path(conn->get_work_dir(), arg, real_path)) {
        conn->send_response(550, "File not found");
        conn->close_data_connection();
        return;
    }
    
    // 等待数据连接
    for (int i = 0; i < 50 && !data_conn->is_connected(); ++i) {
        ACE_OS::sleep(ACE_Time_Value(0, 100000));
    }
    
    if (!data_conn->is_connected()) {
        conn->send_response(425, "Can't open data connection");
        conn->close_data_connection();
        return;
    }
    
    // 打开文件
    std::ifstream file(real_path.c_str(), std::ios::binary);
    if (!file) {
        conn->send_response(550, "File not found");
        conn->close_data_connection();
        return;
    }
    
    conn->send_response(150, "Opening data connection");
    
    // 发送文件内容
    char buffer[8192];
    while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) {
        ssize_t sent = data_conn->send_data(buffer, file.gcount());
        if (sent < 0) {
            break;
        }
    }
    
    file.close();
    conn->close_data_connection();
    conn->send_response(226, "Transfer complete");
}

void StorCommand::execute(FTPConnection* conn, const std::string& arg) {
    if (conn->get_auth_state() != AuthState::AUTHENTICATED) {
        conn->send_response(530, "Not logged in");
        return;
    }
    
    if (arg.empty()) {
        conn->send_response(501, "Syntax error in parameters or arguments");
        return;
    }
    
    DataConnection* data_conn = conn->get_data_connection();
    if (!data_conn) {
        conn->send_response(425, "Use PASV first");
        return;
    }
    
    FileSystem* fs = conn->get_filesystem();
    std::string real_path;
    
    if (!fs->resolve_path(conn->get_work_dir(), arg, real_path)) {
        conn->send_response(550, "Can't create file");
        conn->close_data_connection();
        return;
    }
    
    // 等待数据连接
    for (int i = 0; i < 50 && !data_conn->is_connected(); ++i) {
        ACE_OS::sleep(ACE_Time_Value(0, 100000));
    }
    
    if (!data_conn->is_connected()) {
        conn->send_response(425, "Can't open data connection");
        conn->close_data_connection();
        return;
    }
    
    // 创建文件
    std::ofstream file(real_path.c_str(), std::ios::binary);
    if (!file) {
        conn->send_response(550, "Can't create file");
        conn->close_data_connection();
        return;
    }
    
    conn->send_response(150, "Opening data connection");
    
    // 接收文件内容
    char buffer[8192];
    ssize_t received;
    while ((received = data_conn->recv_data(buffer, sizeof(buffer))) > 0) {
        file.write(buffer, received);
    }
    
    file.close();
    conn->close_data_connection();
    conn->send_response(226, "Transfer complete");
}

} // namespace ftpd
