//
// Created by 18404 on 2021/9/2.
//

#ifndef CXX_DISKMANAGER_H
#define CXX_DISKMANAGER_H

#include <include/common/config.h>


class DiskManager {
    friend class BufferChunk;

public:
    explicit DiskManager(const std::string &db_file, page_size_t page_size);

    ~DiskManager() {
        ShutDown();
    };

    void ShutDown();

    void WritePage(size_t offset, const char *page_data);

    void ReadPage(size_t offset, char *page_data);

private:
    page_size_t page_size;
    fstream db_io_;
    string file_name_;
};


#endif //CXX_DISKMANAGER_H
