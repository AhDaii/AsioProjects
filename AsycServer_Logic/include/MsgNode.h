#ifndef ASYCSERVER_MsgNode_H
#define ASYCSERVER_MsgNode_H

#include "boost/asio/detail/socket_ops.hpp"
#include "const.h"
#include <cstring>
#include <iostream>
using namespace std;

class MsgNode {
public:
    MsgNode(short max_len)
        : _total_len(max_len), _cur_len(0) {
        _data = new char[_total_len + 1];
        _data[_total_len] = 0;
    }

    ~MsgNode() {
        cout << "Destruct MsgNode" << endl;
        delete[] _data;
    }

    void Clear() {
        ::memset(_data, 0, _total_len);
        _cur_len = 0;
    }

    short _cur_len;
    short _total_len;
    char* _data;
};

class RecvNode : public MsgNode {
public:
    RecvNode(short max_len, short msg_id);

private:
    short _msg_id;
};

class SendNode : public MsgNode {
public:
    SendNode(const char* msg, short max_len, short msg_id);

private:
    short _msg_id;
};

#endif
