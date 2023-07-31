//
// Created by hunz1 on 2023/7/10.
//

#ifndef ASYCSERVER_CSERVER_H
#define ASYCSERVER_CSERVER_H

#include "CSession.h"
#include <boost/asio.hpp>
#include <mutex>
using namespace std;
using boost::asio::ip::tcp;

class CServer {
public:
    CServer(boost::asio::io_context& ioc, unsigned short port);
    void ClearCSession(std::string uuid);

private:
    void start_accept();
    void handle_accept(std::shared_ptr<CSession> new_Session, const boost::system::error_code& ec);
    boost::asio::io_context& _ioc;
    tcp::acceptor _acceptor;
    std::unordered_map<std::string, std::shared_ptr<CSession>> _Sessions;
    std::mutex _mutex;
};

#endif  // ASYCSERVER_CSERVER_H
