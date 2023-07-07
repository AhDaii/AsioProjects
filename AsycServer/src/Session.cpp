//
// Created by hunz1 on 2023/7/7.
//

#include "Session.h"
#include <iostream>

using namespace std;

void Session::Start() {
    memset(_data, 0, max_length);
    _socket.async_read_some(boost::asio::buffer(_data, max_length),
                            bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2));
}

void Session::handle_read(const boost::system::error_code &ec, size_t bytes_transferred) {
    if (!ec) {
        cout << "Server receive data is: " << _data << endl;
        boost::asio::async_write(_socket, boost::asio::buffer(_data, bytes_transferred),
                                 std::bind(&Session::handle_write, this, std::placeholders::_1));
    } else {
        cout << "Read error" << endl;
        delete this;
    }
}

void Session::handle_write(const boost::system::error_code &ec) {
    if (!ec) {
        memset(_data, 0, max_length);
        _socket.async_read_some(boost::asio::buffer(_data, max_length),
                                bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2));
    } else {
        cout << "Write error" << endl;
        delete this;
    }
}

Server::Server(boost::asio::io_context& ioc, unsigned short port):_ioc(ioc), _acceptor(ioc, tcp::endpoint(tcp::v4(), port)) {
    cout << "Server start success, on port: " << port << endl;
    start_accept();
}

void Server::start_accept() {
    Session* new_session = new Session(_ioc);
    _acceptor.async_accept(new_session->Socket(), std::bind(&Server::handle_accept, this, new_session, std::placeholders::_1));
}

void Server::handle_accept(Session* new_session, const boost::system::error_code& ec) {
    if (!ec) {
        new_session->Start();
    } else {
        delete new_session;
    }
    start_accept();
}
