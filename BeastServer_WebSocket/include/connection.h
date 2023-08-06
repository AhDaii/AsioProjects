#ifndef CONNECTION_H
#define CONNECTION_H

#include "boost/beast/core/flat_buffer.hpp"
#include "boost/beast/core/tcp_stream.hpp"
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <memory>
#include <mutex>
#include <queue>

namespace net = boost::asio;
namespace beast = boost::beast;
using namespace beast;
using namespace beast::websocket;

class Connection : public std::enable_shared_from_this<Connection> {
private:
    net::io_context& _ioc;
    std::unique_ptr<stream<tcp_stream>> _ws_ptr;
    std::string _uuid;
    flat_buffer _recv_buffer;
    std::queue<std::string> _send_que;
    std::mutex _send_mutex;

public:
    Connection(net::io_context& ioc);
    std::string GetUid();
    net::ip::tcp::socket& GetSocket();
    void AsyncSend(std::string msg);
    void AsyncAccpet();
    void Start();
};

#endif
