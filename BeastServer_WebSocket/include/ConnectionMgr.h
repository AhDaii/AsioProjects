#ifndef CONNECTMGR_H
#define CONNECTMGR_H

#include "connection.h"
#include <unordered_map>
class ConnectionMgr {
private:
    ConnectionMgr();
    ConnectionMgr(const ConnectionMgr&) = delete;
    ConnectionMgr& operator=(const ConnectionMgr&) = delete;
    std::unordered_map<std::string, std::shared_ptr<Connection>> _map_cons;

public:
    static ConnectionMgr& GetInstance();
    void AddConnection(std::shared_ptr<Connection> conptr);
    void RmvConnection(std::string uuid);
};

#endif
