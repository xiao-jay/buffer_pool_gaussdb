#include <include/common/config.h>
#include <include/server/Server.h>

void signal_handler(int signum) {
    cout << "Got signal SIGINT." << endl;
    g_program_shutdown = true;
    shutdown(server_socket, SHUT_RDWR);
}

/**
 * 服务端主程序入口
 * @param argc 必须保证7个参数
 * @param argv 程序名 数据库文件 socket套接字文件名，8K大小的页面数量，16K大小的页面数量，32K数量，2M数量
 * @return
 */
int main(int argc, char *argv[]) {
    if (argc != 7) {
        cerr << "usage: " << argv[0] << " /path/to/datafile /tmp/sockfile.sock 8k 16k 32k 2m" << endl;
        return -1;
    }
    //注册ctrl+C信号,收到信号时，程序不会停止，而是转向执行对应的函数
    signal(SIGINT, signal_handler);
    string socket_file = string(argv[2]);
    cout << "Server will listen at file " << socket_file << endl;
    //页面大小->页面数量
    map<page_size_t, page_num_t> page_no_info;
    array<page_size_t, 4> page_sizes = {8 * 1024, 16 * 1024, 32 * 1024, 2 * 1024 * 1024};
    for (int i = 0; i < page_sizes.size() && i + 3 < argc; i++) {
        page_no_info.insert({page_sizes[i], stoi(argv[3 + i])});
    }
    //根据页面信息构建一个buffer pool
    BufferPool *bp = new BufferPool(argv[1], page_no_info);
    cout << "Start initializing Server..." << endl;
    //构建一个服务器对象
    Server server(bp, socket_file.c_str());
    server.create_socket();
    server.listen_forever();
    cout << "Start deinitializing EveryThing..." << endl;
    delete bp;
    return 0;
}
