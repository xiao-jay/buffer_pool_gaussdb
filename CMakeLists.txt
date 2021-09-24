cmake_minimum_required(VERSION 3.10)
project(cxx)

# 设置编译选项
# 允许c++11标准、O3优化、多线程。match选项可避免一些cpu上的问题
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -march=native -O3 -pthread")
#set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(example
        example.cc include/common/config.h buffer/BufferPool.cpp include/buffer/BufferPool.h include/buffer/replace.h buffer/BufferChunk.cpp include/buffer/BufferChunk.h disk/DiskManager.cpp include/disk/DiskManager.h server/Server.cpp include/server/Server.h include/page/Page.h include/common/rwlatch.h buffer/LRUReplacer.cpp include/buffer/LRUReplacer.h server/Server.cpp)
