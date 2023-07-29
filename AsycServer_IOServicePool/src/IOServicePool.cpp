#include "IOServicePool.h"
#include <cstddef>

boost::asio::io_context& AsioIOServicePool::GetIOService() {
    auto& service = _ioServices[_nextIOService++];
    _nextIOService %= _ioServices.size();

    return service;
}

void AsioIOServicePool::Stop() {
    for (auto& work : _works)
        work.reset();
    for (auto& t : _threads)
        t.join();
}

AsioIOServicePool::AsioIOServicePool(std::size_t size)
    : _ioServices(size), _works(size), _nextIOService(0) {
    for (size_t i = 0; i < size; ++i) {
        _works[i] = std::unique_ptr<Work>(new Work(_ioServices[i]));
    }

    for (size_t i = 0; i < size; ++i) {
        _threads.emplace_back([this, i] {
            _ioServices[i].run();
        });
    }
}

AsioIOServicePool::~AsioIOServicePool() {
    std::cout << "AsioIOServicePool destruct" << std::endl;
}
