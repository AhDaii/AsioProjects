#ifndef ASYCSERVER_LogicSystem_H
#define ASYCSERVER_LogicSystem_H

#include "CSession.h"
#include "Singleton.h"
#include <condition_variable>
#include <functional>
#include <map>
#include <memory>
#include <queue>
#include <thread>

typedef function<void(std::shared_ptr<CSession>, short msg_id, string msg_data)> FunCallBack;

class LogicSystem : public Singleton<LogicSystem> {
    friend class Singleton<LogicSystem>;

public:
    ~LogicSystem();
    void PostMsgToQue(shared_ptr<LogicNode> msg);

private:
    LogicSystem();
    void DealMsg();
    void RegisterCallbacks();
    void HelloWordCallback(shared_ptr<CSession> session, short msg_id, string msg_data);
    std::thread _worker_thread;
    std::queue<shared_ptr<LogicNode>> _msg_que;
    std::mutex _mutex;
    std::condition_variable _consume;
    bool _b_stop;
    std::map<short, FunCallBack> _fun_callbacks;
};

#endif
