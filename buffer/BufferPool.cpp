//
// Created by 18404 on 2021/9/2.
//

#include "include/buffer/BufferPool.h"

BufferPool::BufferPool(const string &file_name, const map<page_size_t, page_num_t> &page_no_info) {
    cout << "Start initial Buffer Pool" << endl;
    struct timeval sTime, eTime;
    gettimeofday(&sTime, NULL);
    size_t total_size = 0;
    //有几种类型的页面
    int type_ = 0;
    //1.1计算出如果需要缓存这些所有页面需要多少内存
    for (auto &page_info_ : page_no_info) {
        if (page_info_.second != 0) {
            total_size += (page_info_.first * page_info_.second);
            ++type_;
        }
    }
    //每种类型的页面赋予多少的并行度
    num_chunks_ /= type_;
    total_size /= 1024;//注意这里使用KB作为单位
    //TODO 用于定义缓冲池的最大容量，单位为KB
    long long max = 6L * 1024 * 1024;
    //1.2根据本机可用内存进行buffer chunk创建
    for (auto &page_info_ : page_no_info) {
        if (page_info_.second != 0) {//额确保页面数量不能为0,才有后面操作
            cout << "************************************************************ page size "
                 << page_info_.first
                 << "*********************************************************************" << endl;
            //1.3缓存此类型页面需要多少页面
            pool_size_t pool_size = max * page_info_.second / total_size;
            pool_size /= num_chunks_;//每个chunk的pool size
            for (int i = 0; i < num_chunks_; ++i) {
                //1.4每个chunk都有一个diskManager，负责管理与磁盘的交互
                DiskManager *diskManager = new DiskManager(file_name, page_info_.first);
                //1.5为该类型页面创建buffer chunk
                BufferChunk *chunk = new BufferChunk(pool_size, diskManager, page_no_info);
                chunks.insert({page_info_.first + i, chunk});
            }
            //todo 在初始化阶段提前预读数据
            for (int j = 0; j < pool_size * num_chunks_; ++j) {
                auto chunk = chunks.find(page_info_.first + (j & (num_chunks_ - 1)))->second;
                chunk->FetchPageImpl(j, false);
                chunk->UnpinPageImpl(j, false);
            }
        }
    }

    cout << "Buffer Pool has been initialed" << endl;
    gettimeofday(&eTime, NULL);
    long exeTime = (eTime.tv_sec - sTime.tv_sec) * 1000000 + (eTime.tv_usec - sTime.tv_usec);
    printf("buffer pool initialed all cost  %ld us \n", exeTime);
}

BufferPool::~BufferPool() {
    for (auto &chunk:chunks) {
        delete chunk.second;
    }
}