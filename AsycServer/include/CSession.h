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

using boost::asio::ip::tcp;

class CServer;

class MsgNode {
    friend class CSession;

public:
    MsgNode(char *msg, int max_len) {
        _data = new char[max_len];
        memcpy(_data, msg, max_len);
    }

    ~MsgNode() {
        delete[] _data;
    }

private:
    int _cur_len;
    int _max_len;
    char *_data;
};

class CSession : public std::enable_shared_from_this<CSession> {
public:
    CSession(boost::asio::io_context &ioc, CServer *server) : _socket(ioc), _server(server) {
        boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
        _uuid = boost::uuids::to_string(a_uuid);
    }

    tcp::socket &Socket() {
        return _socket;
    }

    void Start();

    std::string &GetUuid();

    void Send(char *msg, int max_length);

    ~CSession() {
        std::cout << "CSession destruct, delete this[" << this << "]" << std::endl;
    }

private:
    void
    HandleRead(const boost::system::error_code &ec, size_t bytes_transferred, std::shared_ptr<CSession> _self_shared);

    void HandleWrite(const boost::system::error_code &ec, std::shared_ptr<CSession> _self_shared);

    tcp::socket _socket;
    enum {
        max_length = 1024
    };
    char _data[max_length];
    CServer *_server;
    std::string _uuid;
    std::queue<std::shared_ptr<MsgNode>> _send_que;
    std::mutex _send_lock;
};


#endif //ASYCSERVER_CSession_H
