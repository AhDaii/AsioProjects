//
// Created by hunz1 on 2023/7/7.
//

#include "CSession.h"
#include "CServer.h"
#include <iostream>

using namespace std;

void CSession::Start() {
    memset(_data, 0, max_length);
    _socket.async_read_some(boost::asio::buffer(_data, max_length),
                            bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, shared_from_this()));
}

void CSession::HandleRead(const boost::system::error_code &ec, size_t bytes_transferred, std::shared_ptr<CSession> _self_shared) {
    if (!ec) {
        cout << "Server receive data is: " << _data << endl;
        Send(_data, bytes_transferred);
        memset(_data, 0, max_length);
        _socket.async_read_some(boost::asio::buffer(_data, max_length),
                                bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, _self_shared));
    } else {
        cout << "Read error" << endl;
        _server->ClearCSession(_uuid);
    }
}

void CSession::HandleWrite(const boost::system::error_code &ec, std::shared_ptr<CSession> _self_shared) {
    if (!ec) {
        std::lock_guard<std::mutex> lock(_send_lock);
        _send_que.pop();
        if (!_send_que.empty()) {
            auto& msgnode = _send_que.front();
            boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_data, msgnode->_max_len),
                                     std::bind(&CSession::HandleWrite, this, std::placeholders::_1,_self_shared));
        }
    } else {
        cout << "Write error" << endl;
        _server->ClearCSession(_uuid);
    }
}

std::string& CSession::GetUuid() {
    return _uuid;
}

void CSession::Send(char *msg, int max_length) {
    bool pending = false;
    std::lock_guard<std::mutex> lock(_send_lock);
    if (_send_que.size() > 0) {
        pending = true;
    }
    _send_que.push(make_shared<MsgNode>(msg, max_length));
    if (pending)
        return ;
    boost::asio::async_write(_socket, boost::asio::buffer(msg, max_length),
                             std::bind(&CSession::HandleWrite, this, std::placeholders::_1, shared_from_this()));
}