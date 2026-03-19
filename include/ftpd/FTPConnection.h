/**
 * @file FTPConnection.h
 * @brief FTP客户端控制连接类
 * @author Junxi Chen
 * @date 2026-03-03
 */

#ifndef FTPD_FTPCONNECTION_H
#define FTPD_FTPCONNECTION_H

#include "Types.h"
#include "DataConnection.h"
#include "ICommand.h"
#include "FileSystem.h"
#include <ace/Event_Handler.h>
#include <ace/SOCK_Stream.h>
#include <ace/INET_Addr.h>
#include <string>
#include <memory>
#include <unordered_map>

namespace ftpd {

// 前向声明
class FTPServer;

/**
 * @class FTPConnection
 * @brief 表示单个FTP客户端会话
 * 
 * 职责：
 * - 维护客户端认证状态、工作目录等会话信息
 * - 处理控制连接上的命令输入（handle_input）
 * - 解析命令并分发给对应的Command对象
 * - 管理DataConnection（PASV模式）
 * - 发送FTP响应到客户端
 * 
 * 生命周期：
 * - 由FTPServer在accept新连接时创建
 * - 注册到Reactor监听控制连接socket
 * - 收到QUIT或连接断开时销毁
 */
class FTPConnection : public ACE_Event_Handler {
public:
    /**
     * @brief 构造函数
     */
    FTPConnection();
    
    /**
     * @brief 析构函数，自动清理资源
     */
    virtual ~FTPConnection();
    
    /**
     * @brief 初始化连接
     * @param stream 已accept的socket流
     * @param server 服务器指针（用于认证）
     * @return 成功返回0，失败返回-1
     * 
     * 设置控制连接socket，发送欢迎消息（220 Welcome），
     * 并注册到Reactor监听READ事件。
     */
    int open(ACE_SOCK_Stream& stream, FTPServer* server);
    
    /**
     * @brief 发送FTP响应到客户端
     * @param code FTP响应码（如220, 331, 550等）
     * @param message 响应消息
     * @return 成功返回0，失败返回-1
     * 
     * 格式化为"<code> <message>\r\n"并通过控制连接发送。
     */
    virtual int send_response(int code, const std::string& message);
    
    /**
     * @brief 发送FTPResponse对象
     * @param response 响应对象
     * @return 成功返回0，失败返回-1
     */
    virtual int send_response(const FTPResponse& response);

    /**
     * @brief 设置服务器指针
     */
    void set_server(FTPServer* server) { server_ = server; }
    
    // 会话状态访问器
    
    /**
     * @brief 获取认证状态
     */
    AuthState get_auth_state() const { return auth_state_; }
    
    /**
     * @brief 设置认证状态
     */
    void set_auth_state(AuthState state) { auth_state_ = state; }
    
    /**
     * @brief 获取用户名
     */
    const std::string& get_username() const { return username_; }
    
    /**
     * @brief 设置用户名
     */
    void set_username(const std::string& name) { username_ = name; }
    
    /**
     * @brief 获取当前工作目录（相对于根目录）
     */
    const std::string& get_work_dir() const { return work_dir_; }
    
    /**
     * @brief 设置工作目录
     */
    void set_work_dir(const std::string& dir) { work_dir_ = dir; }
    
    /**
     * @brief 获取服务器指针
     */
    FTPServer* get_server() { return server_; }
    
    /**
     * @brief 获取文件系统操作对象
     */
    FileSystem* get_filesystem() { return filesystem_.get(); }
    
    /**
     * @brief 设置文件系统操作对象（主要用于测试）
     */
    void set_filesystem(FileSystem* fs) { filesystem_.reset(fs); }
    
    /**
     * @brief 获取传输模式
     */
    TransferMode get_transfer_mode() const { return transfer_mode_; }
    
    /**
     * @brief 设置传输模式
     */
    void set_transfer_mode(TransferMode mode) { transfer_mode_ = mode; }
    
    /**
     * @brief 获取数据连接对象
     * @return 数据连接指针，可能为nullptr
     */
    DataConnection* get_data_connection() { return data_conn_.get(); }
    
    /**
     * @brief 创建新的数据连接（PASV模式）
     * @return 数据连接指针
     * 
     * 如果已有旧的数据连接会先关闭。
     */
    DataConnection* create_data_connection();
    
    /**
     * @brief 关闭并释放数据连接
     */
    void close_data_connection();
    
    // ACE_Event_Handler接口实现
    
    /**
     * @brief Reactor回调：处理命令输入
     * @param fd 文件描述符
     * @return 成功返回0，失败返回-1
     * 
     * 读取客户端发送的命令（行缓冲，以\r\n分割），
     * 解析命令名和参数，通过命令映射表分发执行。
     */
    virtual int handle_input(ACE_HANDLE fd = ACE_INVALID_HANDLE) override;
    
    /**
     * @brief Reactor回调：处理连接关闭
     * @param fd 文件描述符
     * @param mask 事件掩码
     * @return 返回-1通知Reactor移除此handler
     */
    virtual int handle_close(ACE_HANDLE fd, ACE_Reactor_Mask mask) override;
    
    /**
     * @brief 获取事件处理器关联的句柄
     * @return 控制连接socket的文件描述符
     */
    virtual ACE_HANDLE get_handle() const override;

private:
    /**
     * @brief 解析并分发命令
     * @param cmd_line 完整命令行（如"USER anonymous"）
     */
    void dispatch_command(const std::string& cmd_line);
    
    /**
     * @brief 初始化命令映射表
     * 
     * 注册所有支持的FTP命令（USER、PASS、PWD等）到cmd_map_。
     */
    void init_commands();

private:
    ACE_SOCK_Stream control_stream_;   ///< 控制连接socket
    ACE_INET_Addr peer_addr_;           ///< 客户端地址
    
    FTPServer* server_;                 ///< 服务器指针
    std::unique_ptr<FileSystem> filesystem_; ///< 文件系统操作对象
    
    // 会话状态
    AuthState auth_state_;              ///< 认证状态
    std::string username_;              ///< 登录用户名
    std::string work_dir_;              ///< 当前工作目录（相对路径，如"/"或"/subdir"）
    TransferMode transfer_mode_;        ///< 传输模式
    
    DataConnectionPtr data_conn_;       ///< 数据连接对象
    
    std::string recv_buffer_;           ///< 接收缓冲区（行缓冲）
    
    // 命令分发表
    std::unordered_map<std::string, CommandPtr> cmd_map_;
};

} // namespace ftpd

#endif // FTPD_FTPCONNECTION_H
