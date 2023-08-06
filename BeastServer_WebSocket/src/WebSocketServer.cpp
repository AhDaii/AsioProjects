#include "WebSocketServer.h"
#include "boost/beast/core/error.hpp"
#include <exception>
#include <iostream>
#include <memory>

WebSocketServer::WebSocketServer(net::io_context& ioc, unsigned short port)
    : _ioc(ioc),
      _acceptor(ioc, net::ip::tcp::endpoint(net::ip::tcp::v4(), port)) {
    std::cout << "Server started, port: " << port << std::endl;
}
void WebSocketServer::StartAccept() {
    auto con_ptr = std::make_shared<Connection>(_ioc);

    _acceptor.async_accept(con_ptr->GetSocket(), [this, con_ptr](error_code ec) {
        try {
            if (ec) {
                std::cout << "[ACCEPT] " << ec.what() << std::endl;
                return;
            }
            con_ptr->AsyncAccpet();
            StartAccept();
        }
        catch (std::exception& ec) {
            std::cout << "[ACCEPT] " << ec.what() << std::endl;
        }
    });
}
