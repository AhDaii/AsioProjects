#include "IOThreadPool.h"

AsioIOThreadPool::AsioIOThreadPool(size_t size)
    : _work(new boost::asio::io_context::work(_service)) {
    for (int i = 0; i < size; ++i) {
        _threads.emplace_back([this] {
            _service.run();
        });
    }
}

boost::asio::io_context& AsioIOThreadPool::GetIOService() {
    return _service;
}

void AsioIOThreadPool::Stop() {
    _service.stop();
    _work.reset();
    for (auto& t : _threads)
        t.join();
}
