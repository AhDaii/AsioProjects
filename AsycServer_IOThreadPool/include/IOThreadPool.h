#ifndef ASYCSERVER_IOThreadPool_H
#define ASYCSERVER_IOThreadPool_H

#include "Singleton.h"
#include "boost/asio/io_context.hpp"
#include <cstddef>
#include <iostream>
#include <thread>
#include <vector>
class AsioIOThreadPool : public Singleton<AsioIOThreadPool> {
    friend class Singleton<AsioIOThreadPool>;

public:
    ~AsioIOThreadPool() {
        std::cout << "AsioIOThreadPool destruct" << std::endl;
    }
    AsioIOThreadPool(const AsioIOThreadPool&) = delete;
    AsioIOThreadPool& operator=(const AsioIOThreadPool&) = delete;
    boost::asio::io_context& GetIOService();
    void Stop();

private:
    AsioIOThreadPool(std::size_t size = std::thread::hardware_concurrency());
    boost::asio::io_context _service;
    std::unique_ptr<boost::asio::io_context::work> _work;
    std::vector<std::thread> _threads;
};

#endif
