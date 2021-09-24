//
// Created by 18404 on 2021/9/2.
//

#include "include/disk/DiskManager.h"

DiskManager::DiskManager(const std::string &db_file, page_size_t page_size)
        : file_name_(db_file), page_size(page_size) {
    cout << "start initial " << page_size << " type DiskManager" << endl;
    db_io_.open(db_file, std::ios::binary | std::ios::in | std::ios::out);
    if (!db_io_.is_open()) {
        cerr << "in diskManager can't open db file" << endl;
    }
    cout << page_size << " type DiskManager has been initialed" << endl;
}

void DiskManager::WritePage(size_t offset, const char *page_data) {
    db_io_.seekp(offset);
    db_io_.write(page_data, page_size);
    if (db_io_.bad()) {
        cerr << "in diskManager I/O error while writing" << endl;
        return;
    }
}

void DiskManager::ReadPage(size_t offset, char *page_data) {
    db_io_.seekp(offset);
    db_io_.read(page_data, page_size);
    if (db_io_.bad()) {
        cerr << "in diskManager I/O error while reading" << endl;
        return;
    }
}

void DiskManager::ShutDown() {
    if (db_io_.is_open()) {
        db_io_.flush();
        db_io_.close();
    }
}