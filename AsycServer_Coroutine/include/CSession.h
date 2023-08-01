//
// Created by hunz1 on 2023/7/7.
//

#ifndef ASYCSERVER_CSession_H
#define ASYCSERVER_CSession_H
#include "MsgNode.h"
#include "const.h"
#include <boost/asio.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <iostream>
#include <memory>
#include <queue>
#include <unordered_map>

using boost::asio::ip::tcp;

namespace this_coro = boost::asio::this_coro;
class CServer;
class LogicSystem;
class CSession : public std::enable_shared_from_this<CSession> {
public:
    CSession(boost::asio::io_context& ioc, CServer* server);

    tcp::socket& Socket() { return _socket; }

    void Start();

    void Close();

    std::string& GetUuid();

    void Send(char* msg, short max_length, short msg_id);
    void Send(std::string msg, short msg_id);

    ~CSession() {
        std::cout << "CSession destruct, delete this[" << this << "]"
                  << std::endl;
    }

private:
    void HandleRead(const boost::system::error_code& ec,
                    size_t bytes_transferred,
                    std::shared_ptr<CSession> _self_shared);

    void HandleWrite(const boost::system::error_code& ec,
                     std::shared_ptr<CSession> _self_shared);

    void PrintRecvData(char* data, int length);

    tcp::socket _socket;

    char _data[MAX_LENGTH];
    CServer* _server;
    std::string _uuid;
    std::queue<std::shared_ptr<MsgNode>> _send_que;
    std::mutex _send_lock;
    bool _b_close;

    // 接收到的消息结构
    std::shared_ptr<RecvNode> _recv_msg_node;
    bool _b_head_parse;
    // 接收到的头部结构
    std::shared_ptr<MsgNode> _recv_head_node;
    boost::asio::io_context& _ioc;
};

class LogicNode {
    friend class LogicSystem;

public:
    LogicNode(shared_ptr<CSession> session, shared_ptr<RecvNode> recvnode)
        : _session(session), _recvnode(recvnode) {}

private:
    shared_ptr<CSession> _session;
    shared_ptr<RecvNode> _recvnode;
};
#endif  // ASYCSERVER_CSession_H
