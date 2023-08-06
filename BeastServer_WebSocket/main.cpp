#include "WebSocketServer.h"
#include "boost/asio/io_context.hpp"

int main() {
    net::io_context ioc;
    WebSocketServer server(ioc, 10086);
    server.StartAccept();
    ioc.run();
    return 0;
}
