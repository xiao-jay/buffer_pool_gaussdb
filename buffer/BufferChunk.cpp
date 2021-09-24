//
// Created by 18404 on 2021/9/2.
//

#include <include/buffer/LRUReplacer.h>
#include "include/buffer/BufferChunk.h"

BufferChunk::BufferChunk(pool_size_t pool_size, DiskManager *disk_manager,
                         const map<page_size_t, page_num_t> &page_no_info)
        : pool_size_(pool_size), disk_manager_(disk_manager), page_no_info(page_no_info) {
    cout << "start initial " << disk_manager->page_size << " type BufferChunk" << endl;
    //动态创建构造函数带参数的对象数组
    pages_ = (Page *) operator new(sizeof(Page) * pool_size);
    replacer_ = new LRUReplacer(pool_size);
    for (pool_size_t i = 0; i < pool_size_; ++i) {
        new(&pages_[i])Page(disk_manager_->page_size);
        free_list_.emplace_back(static_cast<int >(i));
    }
    cout << disk_manager->page_size << " type BufferChunk has been initialed the size is " << free_list_.size() << endl;
}

BufferChunk::~BufferChunk() {
    delete pages_;
    delete replacer_;
    delete disk_manager_;
    
}

Page *BufferChunk::FetchPageImpl(page_id_t page_id, bool is_write) {
    std::lock_guard<std::mutex> lck(latch_);
    ++this->all_request_;
    const auto it = page_table_.find(page_id);
    if (it != page_table_.cend()) {
        ++hit_;
        frame_id_t frame_id = it->second;
        Page *frame = pages_ + frame_id;
        ++frame->pin_count_;
        replacer_->Pin(frame_id);
        return frame;
    }
    //std::lock_guard<std::mutex> lck(latch_);
    if (!free_list_.empty()) {
        frame_id_t frame_id = free_list_.front();
        free_list_.pop_front();
        page_table_[page_id] = frame_id;
        
        Page *page = pages_ + frame_id;
        if (!is_write) {//TODO 如果拿到该页面，只是为了读取操作，那么需要读磁盘
            disk_manager_->ReadPage(page_start_offset(page_id), page->data_);
        }
        page->page_id_ = page_id;
        page->pin_count_ = 1;
        page->is_dirty_ = false;
        const auto it = flush_map_.find(frame_id);
        if(it != flush_map_.end()){
            flush_list_.erase(it->second);
            flush_map_.erase(it);
        }
        return page;
    }
    frame_id_t victim_id = -1;
    if (replacer_->Victim(&victim_id)) {
        Page *victim_page = pages_ + victim_id;
        if (victim_page->IsDirty()) {
            //把节点从flush 删除
            const auto it = flush_map_.find(victim_id);
            if(it != flush_map_.end()){
                flush_list_.erase(it->second);
                flush_map_.erase(it);
            }
            
            disk_manager_->WritePage(page_start_offset(victim_page->GetPageId()), victim_page->GetData());
        }
        page_table_.erase(victim_page->GetPageId());
        
        page_table_[page_id] = victim_id;
        
        victim_page->ResetMemory();
        if (!is_write) {//todo 如果拿到该页面，只是为了写，那么先不读磁盘
            disk_manager_->ReadPage(page_start_offset(page_id), victim_page->data_);
        }
        victim_page->page_id_ = page_id;
        victim_page->pin_count_ = 1;
        victim_page->is_dirty_ = false;
        return victim_page;
    }
    return nullptr;
}

bool BufferChunk::UnpinPageImpl(page_id_t page_id, bool is_dirty) {
    std::lock_guard<std::mutex> lck(latch_);
    const auto it = page_table_.find(page_id);
    if (it == page_table_.cend()) {
        return false;
    }
    frame_id_t frame_id = it->second;
    Page *frame = pages_ + frame_id;
    frame->is_dirty_ |= is_dirty;
    //放入flush链表
    if(frame->IsDirty()){
        const auto it2 = flush_map_.find(frame_id);
        if(it2 == flush_map_.end()){
            flush_map_[frame_id] = it2->second;
            flush_list_.push_back(frame_id);
        }
    }
    if (frame->GetPinCount() > 0) {
        --frame->pin_count_;
    }
    if (frame->GetPinCount() == 0) {
        replacer_->Unpin(it->second);
    }
    return true;
}

bool BufferChunk::FlushPageImpl(frame_id_t frame_id) {
    std::lock_guard<std::mutex> lck(latch_);
    Page *frame = pages_ + frame_id;
    disk_manager_->WritePage(page_start_offset(frame->GetPageId()), frame->GetData());
    const auto it = flush_map_.find(frame_id);
    if(it != flush_map_.end()){
        flush_list_.erase(it->second);
        flush_map_.erase(it);
    }
    
    // the page flushed is still remaining in the buffer pool,
    // but the dirty state should be changed.
    frame->is_dirty_ = false;
    return true;
}

void BufferChunk::FlushAllPagesImpl() {
    while(flush_list_.size()!=0 ){
        FlushPageImpl(flush_list_.front());
        flush_list_.pop_front();
    }
    flush_map_.clear();
}

size_t BufferChunk::page_start_offset(page_id_t no) {
    size_t skip_page_count = 0;
    size_t boundary = 0;
    for (auto &range : page_no_info) {
        if (no >= range.second) {
            boundary += range.first * range.second;
            skip_page_count += range.second;
        } else {
            return boundary + ((no - skip_page_count) * range.first);
        }
    }
    return -1;
}

ostream &operator<<(ostream &os, const BufferChunk &chunk) {
    os << "all_request_: " << chunk.all_request_ << " hit_: " << chunk.hit_;
    return os;
}
