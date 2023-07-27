#include "CServer.h"
#include "boost/asio/signal_set.hpp"
#include <boost/asio.hpp>
#include <iostream>
#include <sys/signal.h>

int main() {
    try {
        boost::asio::io_context ioc;
        using namespace std;
        boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc](auto, auto) {
            ioc.stop();
        });
        CServer s(ioc, 10086);
        ioc.run();
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}
