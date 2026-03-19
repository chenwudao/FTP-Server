/**
 * @file DataConnection.h
 * @brief FTP被动模式数据连接类
 * @author Junxi Chen
 * @date 2026-03-03
 */

#ifndef FTPD_DATACONNECTION_H
#define FTPD_DATACONNECTION_H

#include "Types.h"
#include <ace/Event_Handler.h>
#include <ace/SOCK_Acceptor.h>
#include <ace/SOCK_Stream.h>
#include <ace/INET_Addr.h>
#include <memory>

namespace ftpd {

/**
 * @class DataConnection
 * @brief 处理FTP被动模式（PASV）数据传输
 * 
 * 负责：
 * - 创建随机端口监听器并注册到Reactor
 * - 接受客户端数据连接
 * - 发送数据（LIST、RETR）
 * - 接收数据（STOR）
 * - 传输完成后关闭连接
 * 
 * 使用场景：
 * 1. 收到PASV命令时创建DataConnection对象并调用pasv_listen()
 * 2. 返回"227 Entering Passive Mode (h1,h2,h3,h4,p1,p2)"
 * 3. 客户端连接后，handle_input被触发，accept数据连接
 * 4. 后续LIST/RETR/STOR命令通过send_data/recv_data传输数据
 * 5. 传输完成调用close()
 */
class DataConnection : public ACE_Event_Handler {
public:
    /**
     * @brief 构造函数
     */
    DataConnection();
    
    /**
     * @brief 析构函数，自动清理资源
     */
    virtual ~DataConnection();
    
    /**
     * @brief 创建被动模式监听器
     * @param local_addr 本地地址（用于绑定，端口为0表示随机）
     * @return 成功返回监听端口号，失败返回-1
     * 
     * 创建ACE_SOCK_Acceptor并绑定随机端口（>1023），
     * 注册到Reactor等待客户端连接。
     */
    int pasv_listen(const ACE_INET_Addr& local_addr);
    
    /**
     * @brief 获取监听地址（用于构造PASV响应）
     * @param addr 输出参数，存储监听地址和端口
     * @return 成功返回0，失败返回-1
     */
    int get_local_addr(ACE_INET_Addr& addr) const;
    
    /**
     * @brief 发送数据到客户端
     * @param buf 数据缓冲区
     * @param len 数据长度
     * @return 成功发送的字节数，失败返回-1
     */
    ssize_t send_data(const char* buf, size_t len);
    
    /**
     * @brief 从客户端接收数据
     * @param buf 接收缓冲区
     * @param len 缓冲区大小
     * @return 成功接收的字节数，0表示连接关闭，-1表示错误
     */
    ssize_t recv_data(char* buf, size_t len);
    
    /**
     * @brief 关闭数据连接
     */
    void close();
    
    /**
     * @brief 检查是否已建立数据连接
     * @return 已连接返回true
     */
    bool is_connected() const { return is_connected_; }
    
    // ACE_Event_Handler接口实现
    
    /**
     * @brief Reactor回调：处理新连接或数据到达
     * @param fd 文件描述符
     * @return 成功返回0
     */
    virtual int handle_input(ACE_HANDLE fd = ACE_INVALID_HANDLE) override;
    
    /**
     * @brief Reactor回调：处理连接关闭
     * @param fd 文件描述符
     * @param mask 事件掩码
     * @return 返回0
     */
    virtual int handle_close(ACE_HANDLE fd, ACE_Reactor_Mask mask) override;
    
    /**
     * @brief 获取事件处理器关联的句柄
     * @return 文件描述符
     */
    virtual ACE_HANDLE get_handle() const override;

private:
    ACE_SOCK_Acceptor acceptor_;   ///< 监听acceptor
    ACE_SOCK_Stream data_stream_;   ///< 数据传输socket
    bool is_listening_;              ///< 是否正在监听
    bool is_connected_;              ///< 是否已建立数据连接
};

/// 数据连接智能指针类型
using DataConnectionPtr = std::unique_ptr<DataConnection>;

} // namespace ftpd

#endif // FTPD_DATACONNECTION_H
