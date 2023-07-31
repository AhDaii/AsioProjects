#include "LogicSystem.h"
#include "const.h"
#include <json/json.h>
#include <json/reader.h>
#include <json/value.h>
#include <mutex>
#include <thread>

LogicSystem::~LogicSystem() {
    _b_stop = true;
    _consume.notify_one();
    _worker_thread.join();
}
void LogicSystem::PostMsgToQue(shared_ptr<LogicNode> msg) {
    std::unique_lock<mutex> lock(_mutex);
    _msg_que.push(msg);
    if (_msg_que.size() == 1) {
        lock.unlock();
        _consume.notify_one();
    }
}

LogicSystem::LogicSystem()
    : _b_stop(false) {
    RegisterCallbacks();
    _worker_thread = std::thread(&LogicSystem::DealMsg, this);
}

void LogicSystem::DealMsg() {
    while (true) {
        std::unique_lock<mutex> lock(_mutex);
        // 队列空则等待
        while (_msg_que.empty() && !_b_stop) {
            _consume.wait(lock);
        }

        // 若关闭，则将剩余的逻辑执行完
        if (_b_stop) {
            while (!_msg_que.empty()) {
                auto msg_node = _msg_que.front();
                // cout << "Received msg id is " << msg_node->_recvnode->_msg_id << endl;
                auto callback_iter = _fun_callbacks.find(msg_node->_recvnode->_msg_id);
                if (callback_iter == _fun_callbacks.end()) {
                    _msg_que.pop();
                    continue;
                }
                callback_iter->second(msg_node->_session, msg_node->_recvnode->_msg_id,
                                      string(msg_node->_recvnode->_data, msg_node->_recvnode->_cur_len));
                _msg_que.pop();
            }
            break;
        }
        // 没有关闭，且有数据
        auto msg_node = _msg_que.front();
        // cout << "Received msg id is " << msg_node->_recvnode->_msg_id << endl;
        auto callback_iter = _fun_callbacks.find(msg_node->_recvnode->_msg_id);
        if (callback_iter == _fun_callbacks.end()) {
            _msg_que.pop();
            continue;
        }
        callback_iter->second(msg_node->_session, msg_node->_recvnode->_msg_id,
                              string(msg_node->_recvnode->_data, msg_node->_recvnode->_cur_len));
        _msg_que.pop();
    }
}

void LogicSystem::RegisterCallbacks() {
    _fun_callbacks[MSG_HELLO_WORLD] = std::bind(&LogicSystem::HelloWordCallback, this, placeholders::_1, placeholders::_2, placeholders::_3);
}

void LogicSystem::HelloWordCallback(shared_ptr<CSession> session, short msg_id, string msg_data) {
    Json::Reader reader;
    Json::Value root;
    reader.parse(msg_data, root);
    std::cout << "recevie msg id  is " << root["id"].asInt() << " msg data is "
              << root["data"].asString() << endl;
    root["data"] = "server has received msg, msg data is " + root["data"].asString();
    std::string return_str = root.toStyledString();
    session->Send(return_str, root["id"].asInt());
}
