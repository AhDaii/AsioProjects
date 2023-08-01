
#define BOOST_ASIO_HAS_CO_AWAIT
#include "CSession.h"
#include "CServer.h"
#include "LogicSystem.h"
#include "MsgNode.h"
#include "boost/asio/detail/socket_ops.hpp"
#include "const.h"
#include <exception>
#include <iomanip>
#include <iostream>
#include <json/json.h>
#include <json/reader.h>
#include <json/value.h>
#include <thread>

#if defined(BOOST_ASIO_ENABLE_HANDLER_TRACKING)
#    define use_awaitable \
        boost::asio::use_awaitable_t(__FILE__, __LINE__, __PRETTY_FUNCTION__)
#endif
using namespace std;
using boost::asio::awaitable;
using boost::asio::co_spawn;
using boost::asio::detached;
using boost::asio::io_context;
using boost::asio::strand;
using boost::asio::use_awaitable;

CSession::CSession(boost::asio::io_context& ioc, CServer* server)
    : _socket(ioc), _server(server), _b_close(false), _b_head_parse(false), _ioc(ioc) {
    boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
    _uuid = boost::uuids::to_string(a_uuid);
    _recv_head_node = make_shared<MsgNode>(HEAD_TOTAL_LEN);
}

void CSession::Close() {
    _socket.close();
    _b_close = true;
}

void CSession::Start() {
    auto shared_this = shared_from_this();
    co_spawn(
        _ioc, [=]() -> awaitable<void> {
            try {
                while (!_b_close) {
                    _recv_head_node->Clear();
                    size_t n = co_await boost::asio::async_read(_socket,
                                                                boost::asio::buffer(_recv_head_node->_data, HEAD_TOTAL_LEN),
                                                                use_awaitable);
                    if (!n) {
                        cout << "Receive peer closed" << endl;
                        Close();
                        _server->ClearCSession(_uuid);
                        co_return;
                    }

                    short msg_id = 0;
                    memcpy(&msg_id, _recv_head_node->_data, HEAD_ID_LEN);
                    msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
                    cout << "Msg id is: " << msg_id << endl;
                    if (msg_id > MAX_LENGTH) {
                        cout << "Invalid msg id is: " << msg_id << endl;
                        _server->ClearCSession(_uuid);
                        co_return;
                    }
                    short msg_len = 0;
                    memcpy(&msg_len, _recv_head_node->_data + HEAD_ID_LEN, HEAD_DATA_LEN);
                    msg_len = boost::asio::detail::socket_ops::network_to_host_short(msg_len);
                    cout << "Msg length is: " << msg_len << endl;
                    if (msg_len > MAX_LENGTH) {
                        cout << "Invalid msg length is: " << msg_len << endl;
                        _server->ClearCSession(_uuid);
                        co_return;
                    }

                    _recv_msg_node = make_shared<RecvNode>(msg_len, msg_id);
                    n = co_await boost::asio::async_read(_socket,
                                                         boost::asio::buffer(_recv_msg_node->_data, _recv_msg_node->_total_len),
                                                         use_awaitable);
                    if (!n) {
                        cout << "Receive peer closed" << endl;
                        Close();
                        _server->ClearCSession(_uuid);
                        co_return;
                    }
                    _recv_msg_node->_data[_recv_msg_node->_total_len] = 0;
                    cout << "Received data is: " << _recv_msg_node->_data << endl;
                    LogicSystem::GetInstance()->PostMsgToQue(make_shared<LogicNode>(shared_this, _recv_msg_node));
                }
            }
            catch (std::exception& e) {
                cout << "Exception is: " << e.what() << endl;
                Close();
                _server->ClearCSession(_uuid);
            }
        },
        detached);
}

void CSession::HandleRead(const boost::system::error_code& ec,
                          size_t bytes_transferred,
                          std::shared_ptr<CSession> _self_shared) {
    try {
        if (!ec) {
            PrintRecvData(_data, bytes_transferred);
            std::chrono::milliseconds dura(2000);
            std::this_thread::sleep_for(dura);

            // 已经移动的字符数
            int copy_len = 0;
            while (bytes_transferred > 0) {
                if (!_b_head_parse) {
                    // 收到的数据不足头部大小
                    if (_recv_head_node->_cur_len + bytes_transferred <
                        HEAD_TOTAL_LEN) {
                        memcpy(_recv_head_node->_data + _recv_head_node->_cur_len, _data + copy_len, bytes_transferred);
                        _recv_head_node->_cur_len += bytes_transferred;
                        ::memset(_data, 0, MAX_LENGTH);
                        _socket.async_read_some(
                            boost::asio::buffer(_data, MAX_LENGTH),
                            bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, _self_shared));
                        return;
                    }

                    // 收到数据>=头部大小
                    int head_remain = HEAD_TOTAL_LEN - _recv_head_node->_cur_len;
                    memcpy(_recv_head_node->_data + _recv_head_node->_cur_len,
                           _data + copy_len,
                           head_remain);
                    copy_len += head_remain;
                    bytes_transferred -= head_remain;
                    // 获取头部的id数据
                    short msg_id = 0;
                    memcpy(&msg_id, _recv_head_node->_data, HEAD_ID_LEN);
                    msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
                    cout << "Received data id is: " << msg_id << endl;
                    if (msg_id > MAX_LENGTH) {
                        cout << "Received data length is invalid!" << endl;
                        _server->ClearCSession(_uuid);
                        return;
                    }

                    // 获取头部的数据
                    short data_len = 0;
                    memcpy(&data_len, _recv_head_node->_data + HEAD_ID_LEN, HEAD_DATA_LEN);
                    // 字节序处理
                    data_len = boost::asio::detail::socket_ops::network_to_host_short(data_len);
                    cout << "Received data length is: " << data_len << endl;

                    // 接收到的数据长度非法
                    if (data_len > MAX_LENGTH) {
                        cout << "Received data length is invalid!" << endl;
                        _server->ClearCSession(_uuid);
                        return;
                    }

                    _recv_msg_node = make_shared<RecvNode>(data_len, msg_id);
                    _b_head_parse = true;

                    // 接收到的消息长度比实际长度小，数据未收全，先将部分消息放到接收节点里
                    if (bytes_transferred < data_len) {
                        memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len,
                               _data + copy_len,
                               bytes_transferred);
                        _recv_msg_node->_cur_len += bytes_transferred;
                        ::memset(_data, 0, MAX_LENGTH);
                        _socket.async_read_some(
                            boost::asio::buffer(_data, MAX_LENGTH),
                            bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, _self_shared));
                        // _b_head_parse = true;
                        // 个人认为这条语句应当在头部处理完之后立刻执行
                        return;
                    }

                    memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len,
                           _data + copy_len,
                           data_len);
                    _recv_msg_node->_cur_len += data_len;
                    copy_len += data_len;
                    bytes_transferred -= data_len;
                    _recv_msg_node->_data[_recv_msg_node->_total_len] = 0;
                    // cout << "Received data is: " << _recv_msg_node->_data << endl;
                    LogicSystem::GetInstance()->PostMsgToQue(make_shared<LogicNode>(_self_shared, _recv_msg_node));

                    _b_head_parse = false;
                    _recv_head_node->Clear();
                    if (!bytes_transferred) {
                        ::memset(_data, 0, MAX_LENGTH);
                        _socket.async_read_some(
                            boost::asio::buffer(_data, MAX_LENGTH),
                            bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, _self_shared));
                    }
                }
                else {
                    // 头部处理之前已经处理完，接收剩下的内容
                    int remain_msg =
                        _recv_msg_node->_total_len - _recv_msg_node->_cur_len;
                    // 接收的数据仍不足补全
                    if (bytes_transferred < remain_msg) {
                        memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len,
                               _data + copy_len,
                               bytes_transferred);
                        _recv_msg_node->_cur_len += bytes_transferred;
                        ::memset(_data, 0, MAX_LENGTH);
                        _socket.async_read_some(
                            boost::asio::buffer(_data, MAX_LENGTH),
                            bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, _self_shared));
                        return;
                    }
                    memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len,
                           _data + copy_len,
                           remain_msg);
                    _recv_msg_node->_cur_len += remain_msg;
                    bytes_transferred -= remain_msg;
                    copy_len += remain_msg;
                    _recv_msg_node->_data[_recv_msg_node->_total_len] = 0;

                    // cout << "Received data is: " << _recv_msg_node->_data << endl;
                    LogicSystem::GetInstance()->PostMsgToQue(make_shared<LogicNode>(_self_shared, _recv_msg_node));

                    _b_head_parse = false;
                    _recv_head_node->Clear();
                    if (!bytes_transferred) {
                        ::memset(_data, 0, MAX_LENGTH);
                        _socket.async_read_some(
                            boost::asio::buffer(_data, MAX_LENGTH),
                            bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, _self_shared));
                    }
                }
            }
        }
        else {
            cout << "Read error" << endl;
            _server->ClearCSession(_uuid);
        }
    }
    catch (std::exception& e) {
        cout << "Exception is " << e.what() << endl;
    }
}

void CSession::HandleWrite(const boost::system::error_code& ec,
                           std::shared_ptr<CSession> _self_shared) {
    if (!ec) {
        std::lock_guard<std::mutex> lock(_send_lock);
        _send_que.pop();
        if (!_send_que.empty()) {
            auto& msgnode = _send_que.front();
            // cout << "[DEBUG] " << msgnode->_total_len << endl;
            boost::asio::async_write(
                _socket,
                boost::asio::buffer(msgnode->_data, msgnode->_total_len),
                std::bind(&CSession::HandleWrite, this, std::placeholders::_1, _self_shared));
        }
    }
    else {
        cout << "Write error" << endl;
        Close();
        _server->ClearCSession(_uuid);
    }
}

std::string& CSession::GetUuid() {
    return _uuid;
}

void CSession::Send(std::string msg, short msg_id) {
    std::lock_guard<std::mutex> lock(_send_lock);
    size_t _send_que_size = _send_que.size();
    if (_send_que_size > MAX_SENDQUE) {
        cout << "Session: " << _uuid << " send queue fulled, size is "
             << MAX_SENDQUE << endl;
        return;
    }
    _send_que.push(make_shared<SendNode>(msg.c_str(), msg.length(), msg_id));
    if (_send_que_size > 0)
        return;
    auto& msg_node = _send_que.front();
    boost::asio::async_write(_socket,
                             boost::asio::buffer(msg_node->_data, msg_node->_total_len),
                             std::bind(&CSession::HandleWrite, this, std::placeholders::_1, shared_from_this()));
}

void CSession::Send(char* msg, short max_length, short msg_id) {
    std::lock_guard<std::mutex> lock(_send_lock);
    size_t _send_que_size = _send_que.size();
    if (_send_que_size > MAX_SENDQUE) {
        cout << "Session: " << _uuid << " send queue fulled, size is "
             << MAX_SENDQUE << endl;
        return;
    }
    _send_que.push(make_shared<SendNode>(msg, max_length, msg_id));
    if (_send_que_size > 0)
        return;
    auto& msg_node = _send_que.front();
    boost::asio::async_write(_socket,
                             boost::asio::buffer(msg_node->_data, msg_node->_total_len),
                             std::bind(&CSession::HandleWrite, this, std::placeholders::_1, shared_from_this()));
}

void CSession::PrintRecvData(char* data, int length) {
    stringstream ss;
    string result = "0x";
    for (int i = 0; i < length; i++) {
        string hexstr;
        ss << hex << std::setw(2) << std::setfill('0') << int(data[i]) << endl;
        ss >> hexstr;
        result += hexstr;
    }
    std::cout << "receive raw data is : " << result << endl;
    ;
}
