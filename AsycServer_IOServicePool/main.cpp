#include "CServer.h"
#include "IOServicePool.h"
#include "Singleton.h"
#include <csignal>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
using namespace std;
bool bstop = false;
std::condition_variable cond_quit;
std::mutex mutex_quit;

int main() {
    try {
        auto pool = AsioIOServicePool::GetInstance();
        boost::asio::io_context io_context;
        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&io_context, pool](auto, auto) {
            io_context.stop();
            pool->Stop();
        });
        CServer s(io_context, 10086);
        io_context.run();
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << endl;
    }
}
