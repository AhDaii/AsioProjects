//
// Created by hunz1 on 2023/7/12.
//

#ifndef ASYCSERVER_CONST_H
#define ASYCSERVER_CONST_H

#define HEAD_LENGTH 2
#define MAX_LENGTH (1024 * 2)
#define MAX_SENDQUE 1000
#define HEAD_TOTAL_LEN 4
#define HEAD_ID_LEN 2
#define HEAD_DATA_LEN 2

enum MSG_IDS {
    MSG_HELLO_WORLD = 1001
};
#endif  // ASYCSERVER_CONST_H
