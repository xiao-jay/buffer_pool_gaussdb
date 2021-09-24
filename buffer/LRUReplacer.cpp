//
// Created by 18404 on 2021/9/3.
//

#include "include/buffer/LRUReplacer.h"

LRUReplacer::LRUReplacer(pool_size_t num_pages) : capacity(num_pages) {}

LRUReplacer::~LRUReplacer() = default;

bool LRUReplacer::Victim(frame_id_t *frame_id) {
    lock_guard<std::mutex> lck(latch);
    if (l.empty()) {
        return false;
    }
    *frame_id = l.back();
    m.erase(l.back());
    l.pop_back();
    return true;
}

void LRUReplacer::Pin(frame_id_t frame_id) {
    lock_guard<std::mutex> lck(latch);
    const auto it = m.find(frame_id);
    if (it == m.cend()) {
        return;
    }
    l.erase(it->second);
    m.erase(it);
}

void LRUReplacer::Unpin(frame_id_t frame_id) {
    lock_guard<std::mutex> lck(latch);
    const auto it = m.find(frame_id);
    if (it != m.cend()) {
        return;
    }
    l.emplace_front(frame_id);
    m[frame_id] = l.begin();
}

size_t LRUReplacer::Size() { return l.size(); }