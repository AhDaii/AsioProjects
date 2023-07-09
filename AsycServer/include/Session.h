//
// Created by hunz1 on 2023/7/7.
//

#ifndef ASYCSERVER_SESSION_H
#define ASYCSERVER_SESSION_H

#include <boost/asio.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <unordered_map>

using boost::asio::ip::tcp;
class Server;

class Session: public std::enable_shared_from_this<Session> {
public:
    Session(boost::asio::io_context &ioc, Server* server) : _socket(ioc), _server(server) {
        boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
        _uuid = boost::uuids::to_string(a_uuid);
    }

    tcp::socket &Socket() {
        return _socket;
    }
    void Start();

    std::string& GetUuid();
private:
    void handle_read(const boost::system::error_code& ec, size_t bytes_transferred, std::shared_ptr<Session> _self_shared);
    void handle_write(const boost::system::error_code& ec, std::shared_ptr<Session> _self_shared);
    tcp::socket _socket;
    enum {
        max_length = 1024
    };
    char _data[max_length];
    Server* _server;
    std::string _uuid;
};

class Server {
public:
    Server(boost::asio::io_context& ioc, unsigned short port);
    void ClearSession(std::string uuid);
private:
    void start_accept();
    void handle_accept(std::shared_ptr<Session> new_session, const boost::system::error_code& ec);
    boost::asio::io_context& _ioc;
    tcp::acceptor _acceptor;
    std::unordered_map<std::string, std::shared_ptr<Session>> _sessions;
};

#endif //ASYCSERVER_SESSION_H
