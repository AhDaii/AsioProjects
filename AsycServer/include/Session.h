//
// Created by hunz1 on 2023/7/7.
//

#ifndef ASYCSERVER_SESSION_H
#define ASYCSERVER_SESSION_H

#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class Session {
public:
    Session(boost::asio::io_context &ioc) : _socket(ioc) {}

    tcp::socket &Socket() {
        return _socket;
    }
    void Start();
private:
    void handle_read(const boost::system::error_code& ec, size_t bytes_transferred);
    void handle_write(const boost::system::error_code& ec);
    tcp::socket _socket;
    enum {
        max_length = 1024
    };
    char _data[max_length];
};

class Server {
public:
    Server(boost::asio::io_context& ioc, unsigned short port);
private:
    void start_accept();
    void handle_accept(Session* new_session, const boost::system::error_code& ec);
    boost::asio::io_context& _ioc;
    tcp::acceptor _acceptor;
};

#endif //ASYCSERVER_SESSION_H
