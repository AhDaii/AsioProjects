//
// Created by hunz1 on 2023/7/6.
//

#ifndef ASYCAPI_SESSION_H
#define ASYCAPI_SESSION_H
#include <memory>
#include <queue>
#include <iostream>
#include <boost/asio.hpp>
using namespace boost;
using namespace std;
const int RECVSIZE = 1024;

class MsgNode {
public:
    // 发送节点
    MsgNode(const char* msg, int total_len): _total_len(total_len), _cur_len(0) {
        _msg = new char[total_len];
        memcpy(_msg, msg, total_len);
    }
    // 接收节点
    MsgNode(int total_len): _total_len(total_len), _cur_len(0) {
        _msg = new char[total_len];
    }
    ~MsgNode() {
        delete[] _msg;
    }

    // 总长度
    int _total_len;
    // 已发送长度
    int _cur_len;
    // 消息首地址
    char *_msg;
};

class Session {
public:
    Session(std::shared_ptr<asio::ip::tcp::socket> socket);
    void Connect(const asio::ip::tcp::endpoint& ep);
    void WriteCallBackErr(const boost::system::error_code& ec, std::size_t bytes_transferred,
                          std::shared_ptr<MsgNode> msg_node);
    void WriteToSocketErr(const std::string& buf);

    void WriteCallBack(const boost::system::error_code& ec, std::size_t bytes_transferred);
    void WriteToSocket(const std::string& buf);

    void WriteAllToSocket(const std::string& buf);
    void WriteAllCallBack(const boost::system::error_code& ec, std::size_t bytes_transferred);

    void ReadFromSocket();
    void ReadCallBack(const boost::system::error_code& ec, std::size_t bytes_transferred);
private:
    std::shared_ptr<asio::ip::tcp::socket> _socket;
    std::shared_ptr<MsgNode> _send_node;
    std::shared_ptr<MsgNode> _recv_node;
    std::queue<std::shared_ptr<MsgNode>> _send_queue;
    bool _send_pending;
    bool _recv_pending;
};

#endif //ASYCAPI_SESSION_H
