//
// Created by 18404 on 2021/9/2.
//

#ifndef CXX_BUFFERCHUNK_H
#define CXX_BUFFERCHUNK_H

#include <unordered_map>
#include <list>
#include <mutex>
#include <map>
#include <include/disk/DiskManager.h>
#include <include/page/Page.h>
#include <include/buffer/replace.h>
#include <ostream>

class BufferChunk {
public:
    BufferChunk(pool_size_t pool_size, DiskManager *disk_manager,
                const map<page_size_t, page_num_t> &page_no_info);

    ~BufferChunk();

    Page *FetchPageImpl(page_id_t page_id, bool is_write);

    bool UnpinPageImpl(page_id_t page_id, bool is_dirty);

    bool FlushPageImpl(page_id_t page_id);

    Page *NewPageImpl(page_id_t *page_id);

    bool DeletePageImpl(page_id_t page_id);

    void FlushAllPagesImpl();

    size_t page_start_offset(page_id_t no);

    friend ostream &operator<<(ostream &os, const BufferChunk &chunk);

private:
    //该缓冲池最大可以拥有的页面数量
    pool_size_t pool_size_;
    //页面数组
    Page *pages_;
    //磁盘管理器
    DiskManager *disk_manager_;
    //页面id->所在位置
    std::unordered_map<page_id_t, frame_id_t> page_table_;
   
    //替换策略
    Replacer *replacer_;
    //空闲列表
    std::list<frame_id_t> free_list_;
    std::list <frame_id_t> flush_list_;
    std::unordered_map<frame_id_t, std::list<frame_id_t>::iterator> flush_map_;
    std::mutex latch_;
    map<page_size_t, page_num_t> page_no_info;
    size_t all_request_ = {0};
    size_t hit_ = {0};
};


#endif //CXX_BUFFERCHUNK_H
