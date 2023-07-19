//
// Created by hunz1 on 2023/7/7.
//

#include "CSession.h"
#include "CServer.h"
#include <iostream>
#include <iomanip>
#include "MsgData.pb.h"

using namespace std;

CSession::CSession(boost::asio::io_context &ioc, CServer *server) : _socket(ioc), _server(server), _b_close(false),
                                                                    _b_head_parse(false) {
    boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
    _uuid = boost::uuids::to_string(a_uuid);
    _recv_head_node = make_shared<MsgNode>(HEAD_LENGTH);
}

void CSession::Close() {
    _socket.close();
    _b_close = true;
}

void CSession::Start() {
    ::memset(_data, 0, MAX_LENGTH);
    _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
                            bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2,
                                 shared_from_this()));
}

void CSession::HandleRead(const boost::system::error_code &ec, size_t bytes_transferred,
                          std::shared_ptr<CSession> _self_shared) {
    if (!ec) {
        PrintRecvData(_data, bytes_transferred);
        std::chrono::milliseconds dura(2000);
        std::this_thread::sleep_for(dura);

        // 已经移动的字符数
        int copy_len = 0;
        while (bytes_transferred > 0) {
            if (!_b_head_parse) {
                // 收到的数据不足头部大小
                if (_recv_head_node->_cur_len + bytes_transferred < HEAD_LENGTH) {
                    memcpy(_recv_head_node->_data + _recv_head_node->_cur_len, _data + copy_len, bytes_transferred);
                    _recv_head_node->_cur_len += bytes_transferred;
                    ::memset(_data, 0, MAX_LENGTH);
                    _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
                                            bind(&CSession::HandleRead, this, std::placeholders::_1,
                                                 std::placeholders::_2, _self_shared));
                    return;
                }

                // 收到数据>=头部大小
                int head_remain = HEAD_LENGTH - _recv_head_node->_cur_len;
                memcpy(_recv_head_node->_data + _recv_head_node->_cur_len, _data + copy_len, head_remain);
                copy_len += head_remain;
                bytes_transferred -= head_remain;
                // 获取头部的数据
                short data_len = 0;
                memcpy(&data_len, _recv_head_node->_data, HEAD_LENGTH);
                // 字节序处理
                data_len = boost::asio::detail::socket_ops::network_to_host_short(data_len);
                cout << "Received data length is: " << data_len << endl;

                // 接收到的数据长度非法
                if (data_len > MAX_LENGTH) {
                    cout << "Received data length is invalid!" << endl;
                    _server->ClearCSession(_uuid);
                    return;
                }

                _recv_msg_node = make_shared<MsgNode>(data_len);
                _b_head_parse = true;

                // 接收到的消息长度比实际长度小，数据未收全，先将部分消息放到接收节点里
                if (bytes_transferred < data_len) {
                    memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, bytes_transferred);
                    _recv_msg_node->_cur_len += bytes_transferred;
                    ::memset(_data, 0, MAX_LENGTH);
                    _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
                                            bind(&CSession::HandleRead, this, std::placeholders::_1,
                                                 std::placeholders::_2, _self_shared));
                    // _b_head_parse = true; 个人认为这条语句应当在头部处理完之后立刻执行
                    return;
                }

                memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, data_len);
                _recv_msg_node->_cur_len += data_len;
                copy_len += data_len;
                bytes_transferred -= data_len;
                _recv_msg_node->_data[_recv_msg_node->_total_len] = 0;

                MsgData msgdata;
                string receive_data;
                msgdata.ParseFromString(string(_recv_msg_node->_data, _recv_msg_node->_total_len));
                cout << "Received msg id is: " << msgdata.id() << ", msg data is: " << msgdata.data() << endl;
                std::string return_str = "server has received msg, msg data is " + msgdata.data();
                MsgData msgreturn;
                msgreturn.set_id(msgdata.id());
                msgreturn.set_data(return_str);
                msgreturn.SerializeToString(&return_str);

                // Send测试
                Send(return_str);

                _b_head_parse = false;
                _recv_head_node->Clear();
                if (!bytes_transferred) {
                    ::memset(_data, 0, MAX_LENGTH);
                    _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
                                            bind(&CSession::HandleRead, this, std::placeholders::_1,
                                                 std::placeholders::_2, _self_shared));
                }
            } else {
                // 头部处理之前已经处理完，接收剩下的内容
                int remain_msg = _recv_msg_node->_total_len - _recv_msg_node->_cur_len;
                // 接收的数据仍不足补全
                if (bytes_transferred < remain_msg) {
                    memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, bytes_transferred);
                    _recv_msg_node->_cur_len += bytes_transferred;
                    ::memset(_data, 0, MAX_LENGTH);
                    _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
                                            bind(&CSession::HandleRead, this, std::placeholders::_1,
                                                 std::placeholders::_2, _self_shared));
                    return;
                }
                memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, remain_msg);
                _recv_msg_node->_cur_len += remain_msg;
                bytes_transferred -= remain_msg;
                copy_len += remain_msg;
                _recv_msg_node->_data[_recv_msg_node->_total_len] = 0;

                MsgData msgdata;
                string receive_data;
                msgdata.ParseFromString(string(_recv_msg_node->_data, _recv_msg_node->_total_len));
                cout << "Received msg id is: " << msgdata.id() << ", msg data is: " << msgdata.data() << endl;
                std::string return_str = "server has received msg, msg data is " + msgdata.data();
                MsgData msgreturn;
                msgreturn.set_id(msgdata.id());
                msgreturn.set_data(return_str);
                msgreturn.SerializeToString(&return_str);

                // Send测试
                Send(return_str);

                _b_head_parse = false;
                _recv_head_node->Clear();
                if (!bytes_transferred) {
                    ::memset(_data, 0, MAX_LENGTH);
                    _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
                                            bind(&CSession::HandleRead, this, std::placeholders::_1,
                                                 std::placeholders::_2, _self_shared));
                }
            }
        }
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
            auto &msgnode = _send_que.front();
            boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len),
                                     std::bind(&CSession::HandleWrite, this, std::placeholders::_1, _self_shared));
        }
    } else {
        cout << "Write error" << endl;
        Close();
        _server->ClearCSession(_uuid);
    }
}

std::string &CSession::GetUuid() {
    return _uuid;
}

void CSession::Send(char *msg, int max_length) {
    std::lock_guard<std::mutex> lock(_send_lock);
    size_t _send_que_size = _send_que.size();
    if (_send_que_size > MAX_SENDQUE) {
        cout << "Session: " << _uuid << " send queue fulled, size is " << MAX_SENDQUE << endl;
        return;
    }
    _send_que.push(make_shared<MsgNode>(msg, max_length));
    if (_send_que_size > 0)
        return;
    auto &msg_node = _send_que.front();
    boost::asio::async_write(_socket, boost::asio::buffer(msg_node->_data, msg_node->_total_len),
                             std::bind(&CSession::HandleWrite, this, std::placeholders::_1, shared_from_this()));
}

void CSession::Send(string msg) {
    std::lock_guard<std::mutex> lock(_send_lock);
    size_t _send_que_size = _send_que.size();
    if (_send_que_size > MAX_SENDQUE) {
        cout << "Session: " << _uuid << " send queue fulled, size is " << MAX_SENDQUE << endl;
        return;
    }
    _send_que.push(make_shared<MsgNode>(msg.c_str(), msg.size()));
    if (_send_que_size > 0)
        return;
    auto &msg_node = _send_que.front();
    boost::asio::async_write(_socket, boost::asio::buffer(msg_node->_data, msg_node->_total_len),
                             std::bind(&CSession::HandleWrite, this, std::placeholders::_1, shared_from_this()));
}

void CSession::PrintRecvData(char *data, int length) {
    stringstream ss;
    string result = "0x";
    for (int i = 0; i < length; i++) {
        string hexstr;
        ss << hex << std::setw(2) << std::setfill('0') << int(data[i]) << endl;
        ss >> hexstr;
        result += hexstr;
    }
    std::cout << "receive raw data is : " << result << endl;;
}