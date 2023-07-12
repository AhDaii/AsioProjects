//
// Created by hunz1 on 2023/7/7.
//

#ifndef ASYCSERVER_CSession_H
#define ASYCSERVER_CSession_H

#include <boost/asio.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <iostream>
#include <unordered_map>
#include <queue>
#include "const.h"

using boost::asio::ip::tcp;

class CServer;

class MsgNode {
    friend class CSession;

public:
    MsgNode(char *msg, short max_len) : _total_len(max_len + HEAD_LENGTH), _cur_len(0) {
        _data = new char[_total_len + 1];
        // 字节序处理
        short max_len_host = boost::asio::detail::socket_ops::host_to_network_short(max_len);
        memcpy(_data, &max_len_host, HEAD_LENGTH);
        memcpy(_data + HEAD_LENGTH, msg, max_len);
        _data[_total_len] = 0;
    }

    MsgNode(short max_len) : _total_len(max_len), _cur_len(0) {
        _data = new char[_total_len + 1];
    }

    void Clear() {
        ::memset(_data, 0, _total_len);
        _cur_len = 0;
    }

    ~MsgNode() {
        delete[] _data;
    }

private:
    short _cur_len;
//    int _max_len;
    short _total_len;
    char *_data;
};

class CSession : public std::enable_shared_from_this<CSession> {
public:
    CSession(boost::asio::io_context &ioc, CServer *server);

    tcp::socket &Socket() {
        return _socket;
    }

    void Start();

    void Close();

    std::string &GetUuid();

    void Send(char *msg, int max_length);

    ~CSession() {
        std::cout << "CSession destruct, delete this[" << this << "]" << std::endl;
    }

private:
    void
    HandleRead(const boost::system::error_code &ec, size_t bytes_transferred, std::shared_ptr<CSession> _self_shared);

    void HandleWrite(const boost::system::error_code &ec, std::shared_ptr<CSession> _self_shared);

    void PrintRecvData(char* data, int length);

    tcp::socket _socket;

    char _data[MAX_LENGTH];
    CServer *_server;
    std::string _uuid;
    std::queue<std::shared_ptr<MsgNode>> _send_que;
    std::mutex _send_lock;
    bool _b_close;

    // 接收到的消息结构
    std::shared_ptr<MsgNode> _recv_msg_node;
    bool _b_head_parse;
    // 接收到的头部结构
    std::shared_ptr<MsgNode> _recv_head_node;
};


#endif //ASYCSERVER_CSession_H
