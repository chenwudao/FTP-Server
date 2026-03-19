/**
 * @file FTPServer.h
 * @brief FTP服务器主类
 * @author Junxi Chen
 * @date 2026-03-03
 */

#ifndef FTPD_FTPSERVER_H
#define FTPD_FTPSERVER_H

#include <ace/Event_Handler.h>
#include <ace/SOCK_Acceptor.h>
#include <ace/INET_Addr.h>
#include <ace/Reactor.h>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace ftpd {

// 前向声明
class FTPConnection;

/**
 * @class FTPServer
 * @brief FTP服务器，基于ACE Reactor事件驱动模型
 * 
 * 职责：
 * - 初始化并监听控制连接端口（默认21）
 * - 接受客户端连接请求（handle_input回调）
 * - 为每个客户端创建FTPConnection对象并注册到Reactor
 * - 管理全局配置（根目录、用户密码文件等）
 * - 运行Reactor事件循环
 * 
 * 使用方式：
 * @code
 * FTPServer server;
 * if (server.open(21, "/var/ftp") == 0) {
 *     server.run();  // 进入事件循环
 * }
 * @endcode
 */
class FTPServer : public ACE_Event_Handler {
public:
    /**
     * @brief 构造函数
     */
    FTPServer();
    
    /**
     * @brief 析构函数，自动清理资源
     */
    virtual ~FTPServer();
    
    /**
     * @brief 初始化服务器
     * @param port 监听端口（默认21）
     * @param root_dir 根目录（默认当前目录）
     * @param passwd_file 用户密码文件路径（可选）
     * @return 成功返回0，失败返回-1
     * 
     * 执行步骤：
     * 1. 创建ACE_SOCK_Acceptor并绑定端口
     * 2. 加载用户密码文件（如果提供）
     * 3. 注册到Reactor监听ACCEPT事件
     */
    int open(int port = 21, 
             const std::string& root_dir = ".",
             const std::string& passwd_file = "");
    
    /**
     * @brief 运行服务器事件循环（阻塞）
     * @return 正常退出返回0
     * 
     * 调用ACE_Reactor::instance()->run_reactor_event_loop()
     * 进入事件循环，处理所有注册的handler。
     * 按Ctrl+C或调用stop()可退出循环。
     */
    int run();
    
    /**
     * @brief 停止服务器
     * 
     * 结束Reactor事件循环，关闭acceptor，
     * 断开所有客户端连接。
     */
    void stop();
    
    /**
     * @brief 获取根目录
     */
    const std::string& get_root_dir() const { return root_dir_; }
    
    /**
     * @brief 验证用户密码
     * @param username 用户名
     * @param password 密码
     * @return 验证通过返回true
     * 
     * 从加载的密码文件中查找匹配用户名和密码。
     * 如果未配置密码文件，默认拒绝所有用户。
     */
    virtual bool authenticate(const std::string& username, 
                     const std::string& password) const;
    
    // ACE_Event_Handler接口实现
    
    /**
     * @brief Reactor回调：处理新连接请求
     * @param fd 文件描述符（acceptor的socket）
     * @return 成功返回0，失败返回-1
     * 
     * 调用acceptor_.accept()接受新连接，创建FTPConnection对象，
     * 调用其open()方法初始化并注册到Reactor。
     */
    virtual int handle_input(ACE_HANDLE fd = ACE_INVALID_HANDLE) override;
    
    /**
     * @brief Reactor回调：处理acceptor关闭
     * @param fd 文件描述符
     * @param mask 事件掩码
     * @return 返回-1通知Reactor移除此handler
     */
    virtual int handle_close(ACE_HANDLE fd, ACE_Reactor_Mask mask) override;
    
    /**
     * @brief 获取事件处理器关联的句柄
     * @return acceptor的文件描述符
     */
    virtual ACE_HANDLE get_handle() const override;

private:
    /**
     * @brief 加载用户密码文件
     * @param filepath 密码文件路径
     * @return 成功返回0，失败返回-1
     * 
     * 文件格式：每行"username:password"（明文）。
     * 支持匿名用户："anonymous:any" 或 "ftp:any"。
     */
    int load_passwd_file(const std::string& filepath);

private:
    ACE_SOCK_Acceptor acceptor_;        ///< 监听acceptor
    std::string root_dir_;              ///< 服务器根目录
    
    // 用户认证数据（username -> password）
    std::unordered_map<std::string, std::string> users_;
    
    // 活动连接列表（用于统计或强制关闭，可选）
    std::vector<FTPConnection*> connections_;
};

} // namespace ftpd

#endif // FTPD_FTPSERVER_H
