//
// Created by 18404 on 2021/9/3.
//

#ifndef CXX_LRUREPLACER_H
#define CXX_LRUREPLACER_H

#include "replace.h"

class LRUReplacer : public Replacer {
public:
    explicit LRUReplacer(pool_size_t num_pages);

    ~LRUReplacer() override;

    bool Victim(frame_id_t *frame_id) override;

    void Pin(frame_id_t frame_id) override;

    void Unpin(frame_id_t frame_id) override;

    size_t Size() override;

private:
    pool_size_t capacity;
    std::unordered_map<frame_id_t, std::list<frame_id_t>::iterator> m;
    std::list<frame_id_t> l;
    std::mutex latch;
};

#endif //CXX_LRUREPLACER_H
