/**
 * @file DataConnection.cpp
 * @brief 数据连接实现
 */

#include "ftpd/DataConnection.h"
#include <ace/Log_Msg.h>
#include <ace/Reactor.h>

namespace ftpd {

DataConnection::DataConnection() 
    : is_listening_(false), is_connected_(false) {
}

DataConnection::~DataConnection() {
    close();
}

int DataConnection::pasv_listen(const ACE_INET_Addr& local_addr) {
    // 绑定随机端口
    if (acceptor_.open(local_addr, 1) == -1) {
        ACE_ERROR_RETURN((LM_ERROR, "Failed to open data acceptor: %p\n",
                        "acceptor_.open"), -1);
    }
    
    is_listening_ = true;
    
    // 注册到Reactor
    if (ACE_Reactor::instance()->register_handler(
            this->get_handle(), this, ACE_Event_Handler::ACCEPT_MASK) == -1) {
        is_listening_ = false;
        ACE_ERROR_RETURN((LM_ERROR, "Failed to register data acceptor: %p\n",
                        "register_handler"), -1);
    }
    
    // 获取实际端口
    ACE_INET_Addr assigned_addr;
    if (acceptor_.get_local_addr(assigned_addr) == -1) {
        ACE_ERROR_RETURN((LM_ERROR, "Failed to get local addr: %p\n",
                        "get_local_addr"), -1);
    }
    
    return assigned_addr.get_port_number();
}

int DataConnection::get_local_addr(ACE_INET_Addr& addr) const {
    return acceptor_.get_local_addr(addr);
}

ssize_t DataConnection::send_data(const char* buf, size_t len) {
    if (!is_connected_) {
        return -1;
    }
    return data_stream_.send_n(buf, len);
}

ssize_t DataConnection::recv_data(char* buf, size_t len) {
    if (!is_connected_) {
        return -1;
    }
    return data_stream_.recv(buf, len);
}

void DataConnection::close() {
    if (is_listening_) {
        ACE_Reactor::instance()->remove_handler(this,
            ACE_Event_Handler::ACCEPT_MASK | ACE_Event_Handler::DONT_CALL);
        acceptor_.close();
        is_listening_ = false;
    }
    
    if (is_connected_) {
        data_stream_.close();
        is_connected_ = false;
    }
}

int DataConnection::handle_input(ACE_HANDLE) {
    if (is_listening_) {
        // Accept数据连接
        ACE_INET_Addr peer_addr;
        if (acceptor_.accept(data_stream_, &peer_addr) == -1) {
            ACE_ERROR_RETURN((LM_ERROR, "Failed to accept data connection: %p\n",
                            "acceptor_.accept"), -1);
        }
        
        ACE_DEBUG((LM_DEBUG, "Data connection accepted\n"));
        
        // 关闭监听器
        ACE_Reactor::instance()->remove_handler(this,
            ACE_Event_Handler::ACCEPT_MASK | ACE_Event_Handler::DONT_CALL);
        acceptor_.close();
        is_listening_ = false;
        is_connected_ = true;
    }
    
    return 0;
}

int DataConnection::handle_close(ACE_HANDLE, ACE_Reactor_Mask) {
    close();
    return 0;
}

ACE_HANDLE DataConnection::get_handle() const {
    if (is_listening_) {
        return acceptor_.get_handle();
    }
    return data_stream_.get_handle();
}

} // namespace ftpd
