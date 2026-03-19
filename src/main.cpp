/**
 * @file main.cpp
 * @brief FTP服务器主程序入口
 * @author Junxi Chen
 * @date 2026-03-03
 */

#include "ftpd/FTPServer.h"
#include <ace/Log_Msg.h>
#include <iostream>
#include <getopt.h>
#include <signal.h>

using namespace ftpd;

// 全局服务器实例（用于信号处理）
static FTPServer* g_server = nullptr;

/**
 * @brief 信号处理函数（Ctrl+C优雅退出）
 */
void signal_handler(int signum) {
    if (g_server) {
        ACE_DEBUG((LM_INFO, "Received signal %d, shutting down...\n", signum));
        g_server->stop();
    }
}

/**
 * @brief 打印使用帮助
 */
void print_usage(const char* prog_name) {
    std::cout << "Usage: " << prog_name << " [options]\n"
              << "Options:\n"
              << "  -p <port>        Listen port (default: 21)\n"
              << "  -r <root_dir>    Root directory (default: current dir)\n"
              << "  -u <passwd_file> User password file (optional)\n"
              << "  -h               Show this help\n";
}

/**
 * @brief 主函数
 * @param argc 参数个数
 * @param argv 参数数组
 * @return 成功返回0，失败返回非0
 */
int main(int argc, char* argv[]) {
    int port = 21;
    std::string root_dir = ".";
    std::string passwd_file;
    
    // 解析命令行参数
    int opt;
    while ((opt = getopt(argc, argv, "p:r:u:h")) != -1) {
        switch (opt) {
            case 'p':
                port = std::atoi(optarg);
                if (port <= 0 || port > 65535) {
                    std::cerr << "Invalid port: " << optarg << std::endl;
                    return 1;
                }
                break;
            case 'r':
                root_dir = optarg;
                break;
            case 'u':
                passwd_file = optarg;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }
    
    // 注册信号处理
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // 创建并启动服务器
    FTPServer server;
    g_server = &server;
    
    ACE_DEBUG((LM_INFO, "Starting FTP server...\n"));
    ACE_DEBUG((LM_INFO, "  Port: %d\n", port));
    ACE_DEBUG((LM_INFO, "  Root: %s\n", root_dir.c_str()));
    
    if (server.open(port, root_dir, passwd_file) != 0) {
        ACE_ERROR((LM_ERROR, "Failed to start server\n"));
        return 1;
    }
    
    ACE_DEBUG((LM_INFO, "Server started successfully. Press Ctrl+C to stop.\n"));
    
    int ret = server.run();
    
    ACE_DEBUG((LM_INFO, "Server stopped.\n"));
    g_server = nullptr;
    
    return ret;
}
