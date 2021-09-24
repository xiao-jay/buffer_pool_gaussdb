//
// Created by 18404 on 2021/9/2.
//

#ifndef CXX_PAGE_H
#define CXX_PAGE_H

#include <include/common/config.h>
#include <include/common/rwlatch.h>

class Page {
    friend class BufferChunk;

    friend class Server;

public:
    Page(unsigned int page_size) : page_size(page_size) {
        data_ = new char[page_size];
        ResetMemory();
    }

    ~Page() {
        delete data_;
    };

    inline char *GetData() { return data_; }

    inline page_id_t GetPageId() { return page_id_; }

    inline int GetPinCount() { return pin_count_; }

    inline bool IsDirty() { return is_dirty_; }

    inline void WLatch() { rwlatch_.WLock(); }

    inline void WUnlatch() { rwlatch_.WUnlock(); }

    inline void RLatch() { rwlatch_.RLock(); }

    inline void RUnlatch() { rwlatch_.RUnlock(); }

private:
    inline void ResetMemory() { memset(data_, 0, page_size); }

    //存放数据
    char *data_;
    //页面大小
    page_size_t page_size;
    //页面ID
    page_id_t page_id_ = -1;
    //有多少线程在使用该页面
    int pin_count_ = 0;
    //是否为脏页
    bool is_dirty_ = false;
    //用于多线程场景下保护页面读写安全的锁
    ReaderWriterLatch rwlatch_;
};

#endif //CXX_PAGE_H
