//
// Created by 18404 on 2021/9/2.
//
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <unistd.h>
#include <cerrno>
#include <iostream>
#include <csignal>
#include <vector>
#include <fcntl.h>
#include <thread>
#include <mutex>
#include <atomic>
#include <cassert>
#include <unordered_map>
#include <list>
#include <map>
#include <fstream>

#ifndef CXX_CONFIG_H
#define CXX_CONFIG_H
using namespace std;
using page_id_t = unsigned int;//页面id类型
using page_size_t = unsigned int;//页面大小类型
using page_num_t = size_t;//页面数量类型
using pool_size_t = unsigned int;//缓冲池大小类型
using frame_id_t = unsigned int;    // 页框类型与缓冲池类型保持一致
static bool g_program_shutdown = false;
static int server_socket = -1;

enum MSG_TYPE {
    GET = 0,
    SET,
    INVALID_TYPE
};

struct __attribute__((packed)) Header {
    unsigned char msg_type;
    unsigned int page_no;
    unsigned int page_size;
};
#endif //CXX_CONFIG_H
