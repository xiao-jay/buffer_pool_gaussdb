//
// Created by 18404 on 2021/9/2.
//

#ifndef CXX_BUFFERPOOL_H
#define CXX_BUFFERPOOL_H

#include <include/common/config.h>
#include "BufferChunk.h"

//尽量使用引用传递，避免复制
class BufferPool {
    friend class Server;

public:
    BufferPool(const string &file_name, const map<page_size_t, page_num_t> &page_no_info);

    ~BufferPool();

    //页面大小->它所对应的BufferChunk
    unordered_map<page_size_t, BufferChunk *> chunks;
    int num_chunks_ = 32;
};


#endif //CXX_BUFFERPOOL_H
