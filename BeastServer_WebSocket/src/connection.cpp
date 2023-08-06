#include "connection.h"
#include "ConnectionMgr.h"
#include "boost/asio/buffer.hpp"
#include "boost/asio/io_context.hpp"
#include "boost/asio/strand.hpp"
#include "boost/beast/core/buffers_to_string.hpp"
#include "boost/beast/core/error.hpp"
#include "boost/beast/core/stream_traits.hpp"
#include "boost/uuid/random_generator.hpp"
#include "boost/uuid/uuid.hpp"
#include "boost/uuid/uuid_io.hpp"
#include <exception>
#include <iostream>
#include <memory>
#include <mutex>

Connection::Connection(net::io_context& ioc)
    : _ioc(ioc),
      _ws_ptr(std::make_unique<stream<tcp_stream>>(boost::asio::make_strand(_ioc))) {
    boost::uuids::random_generator generator;
    boost::uuids::uuid uuid = generator();

    _uuid = boost::uuids::to_string(uuid);
}

std::string Connection::GetUid() {
    return _uuid;
}

net::ip::tcp::socket& Connection::GetSocket() {
    auto& socket = boost::beast::get_lowest_layer(*_ws_ptr).socket();

    return socket;
}
void Connection::AsyncSend(std::string msg) {
    {
        std::lock_guard<std::mutex> lock(_send_mutex);
        size_t len = _send_que.size();
        _send_que.push(msg);
        if (len > 0)
            return;
    }

    auto self = shared_from_this();
    _ws_ptr->async_write(net::buffer(msg.c_str(), msg.length()), [self](error_code err, std::size_t nsize) {
        try {
            if (err) {
                std::cout << "[ASYNCSEND] " << err.what() << std::endl;
                ConnectionMgr::GetInstance().RmvConnection(self->GetUid());
                return;
            }

            std::string send_msg;
            {
                std::lock_guard<std::mutex> lock(self->_send_mutex);
                self->_send_que.pop();
                if (self->_send_que.empty()) {
                    return;
                }
                send_msg = self->_send_que.front();
            }
            self->AsyncSend(send_msg);
        }
        catch (std::exception& e) {
            std::cout << "[ASYNCSEND] " << e.what() << std::endl;
            ConnectionMgr::GetInstance().RmvConnection(self->GetUid());
        }
    });
}

void Connection::AsyncAccpet() {
    auto self = shared_from_this();

    _ws_ptr->async_accept([self](boost::system::error_code ec) {
        try {
            if (!ec) {
                ConnectionMgr::GetInstance().AddConnection(self);
                self->Start();
            }
            else
                std::cout << "[ASYNCACCEPT] " << ec.what() << std::endl;
        }
        catch (std::exception& e) {
            std::cout << "[ASYNCACCEPT] " << e.what() << std::endl;
        }
    });
}
void Connection::Start() {
    auto self = shared_from_this();

    _ws_ptr->async_read(_recv_buffer, [self](error_code err, std::size_t buffer_bytes) {
        try {
            if (err) {
                std::cout << "[START] " << err.what() << std::endl;
                ConnectionMgr::GetInstance().RmvConnection(self->GetUid());
                return;
            }
            self->_ws_ptr->text(self->_ws_ptr->got_text());
            std::string recv_data = boost::beast::buffers_to_string(self->_recv_buffer.data());
            self->_recv_buffer.consume(self->_recv_buffer.size());
            std::cout << "Websocket receive msg is: " << recv_data << std::endl;
            self->AsyncSend(recv_data);
            self->Start();
        }
        catch (std::exception& e) {
            std::cout << "[START] " << e.what() << std::endl;
            ConnectionMgr::GetInstance().RmvConnection(self->GetUid());
        }
    });
}
