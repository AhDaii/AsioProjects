//
// Created by hunz1 on 2023/7/10.
//

#include "CServer.h"
#include "IOServicePool.h"
#include "Singleton.h"
#include <iostream>
#include <mutex>
using namespace std;

CServer::CServer(boost::asio::io_context& ioc, unsigned short port)
    : _ioc(ioc), _acceptor(ioc, tcp::endpoint(tcp::v4(), port)) {
    cout << "Server start success, on port: " << port << endl;
    start_accept();
}

void CServer::start_accept() {
    //    CSession* new_CSession = new CSession(_ioc);
    auto& ioc = AsioIOServicePool::GetInstance()->GetIOService();
    shared_ptr<CSession> new_CSession = make_shared<CSession>(ioc, this);
    _acceptor.async_accept(new_CSession->Socket(), std::bind(&CServer::handle_accept, this, new_CSession, std::placeholders::_1));
}

void CServer::handle_accept(shared_ptr<CSession> new_Session, const boost::system::error_code& ec) {
    if (!ec) {
        new_Session->Start();
        lock_guard<mutex> lock(_mutex);
        _Sessions.insert(make_pair(new_Session->GetUuid(), new_Session));
    }
    else {
        //        delete new_CSession;
    }
    start_accept();
}

void CServer::ClearCSession(std::string uuid) {
    lock_guard<mutex> lock(_mutex);
    _Sessions.erase(uuid);
}
