//
// Created by 18404 on 2021/9/2.
//

#ifndef CXX_SERVER_H
#define CXX_SERVER_H

#include <include/buffer/BufferPool.h>
#include <include/common/config.h>

/**
 * 服务器类，为客户端提供服务
 */
class Server {
private:
    struct sockaddr_un m_server_addr{};
    BufferPool *m_bufferpool;
    const char *m_socket_file;

    struct ThreadData {//传递给线程的数据
        BufferPool *bufferpool;
        pthread_t thread_id{};
        int client_socket;

        ThreadData(BufferPool *bp, int socket)
                : bufferpool(bp), client_socket(socket) {}
    };

public:
    explicit Server(BufferPool *bp, const char *socket_file) : m_bufferpool(bp), m_socket_file(socket_file) {}

    int create_socket() {
        server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
        if (server_socket < 0) {
            cerr << "in  server Create server socket failed, errno = " << endl;
            return -1;
        }
        memset(&m_server_addr, 0, sizeof(m_server_addr));
        strcpy(m_server_addr.sun_path, m_socket_file);
        m_server_addr.sun_family = AF_UNIX;
        unlink(m_socket_file);
        if (bind(server_socket, (struct sockaddr *) &m_server_addr, sizeof(m_server_addr)) == -1) {
            cerr << "in  server Bind server socket failed, errno = " << endl;
            return -1;
        }
        cout << "Create socket success." << endl;
        return 0;
    }

    void listen_forever() {
        if (listen(server_socket, 1000) == -1) {
            cerr << "in server Listen server socket failed, errno = " << endl;
            return;
        }
        std::vector<ThreadData *> workers;
        while (!g_program_shutdown) {
            pthread_t worker_thread;
            int client_socket = accept(server_socket, nullptr, nullptr);
            if (client_socket == -1) {
                if (g_program_shutdown) {
                    cout << "Accecpt abort" << endl;
                } else {
                    cerr << "in 【server】 Accept failed, errno = " << endl;
                }
                break;
            }
            auto *worker_data = new ThreadData(m_bufferpool, client_socket);
            if (!pthread_create(&worker_thread, nullptr, thread_handler, (void *) worker_data)) {
                worker_data->thread_id = worker_thread;
                workers.push_back(worker_data);
                cout << "Thread created, id = " << worker_thread << endl;
            } else {
                cout << "in  server Create thread failed, errno = " << endl;
                shutdown(worker_data->client_socket, SHUT_RDWR);
            }
        }
        cout << "Start shutting down." << endl;
        for (auto worker : workers) {
            shutdown(worker->client_socket, SHUT_RDWR);
            pthread_join(worker->thread_id, nullptr);
        }
        //刷新所有脏页
        for (auto &chunk :m_bufferpool->chunks) {
            
            cout << *chunk.second << endl;
            chunk.second->FlushAllPagesImpl();
        }
        unlink(m_socket_file);
        cout << "Server closed." << endl;
    }

private:
    static int read_loop(int fd, char *buf, uint count) {
        int ret = count;
        while (count) {
            size_t recvcnt = read(fd, buf, count);
            if (recvcnt == -1) {
                if (errno == EINTR) {
                    continue;
                }
                cerr << "in server Package read error." << endl;
                return -1;
            } else if (recvcnt == 0) {
                cerr << "in server Connection disconnected." << endl;
                return 0;
            }

            count -= recvcnt;
            buf += recvcnt;
        }
        return ret;
    }

    static int write_loop(int fd, char *buf, uint count) {
        int ret = count;
        while (count) {
            size_t sendcnt = write(fd, buf, count);
            if (sendcnt == -1) {
                if (errno == EINTR) {
                    continue;
                }
                cerr << "in server Package write error." << endl;
                return -1;
            } else if (sendcnt == 0) {
                cerr << "in server Connection disconnected." << endl;
                return 0;
            }

            count -= sendcnt;
            buf += sendcnt;
        }
        return ret;
    }

    static void *thread_handler(void *data) {
        struct timeval sTime, eTime;
        gettimeofday(&sTime, NULL);
        auto *worker_data = (ThreadData *) data;
        Page *page;
        BufferChunk *chunk;
        Header header{};
        auto chunks = worker_data->bufferpool->chunks;
        int num_chunks_ = worker_data->bufferpool->num_chunks_;
        while (!g_program_shutdown) {
            //memset(&header, 0, sizeof(header));//清空数据
            //1.读取头部数据
//            if (read(worker_data->client_socket, (char *) &header, sizeof(header)) == 0) {
//                cout << "in 【server】 Connection disconnected." << endl;
//                break;
//            }
            if (read_loop(worker_data->client_socket, (char *) &header, sizeof(header)) <= 0) {
                break;
            }
            //2.根据不同的页面大小，选择一个chunk为其服务。
            chunk = chunks.find(header.page_size + (header.page_no & (num_chunks_ - 1)))->second;
            switch (header.msg_type) {
                //3.如果是要写页面
                case SET:
                    //3.1获取页
                    page = chunk->FetchPageImpl(header.page_no, true);
                    //3.2将要写的数据写入到该页面
                    //page->WLatch();//加锁
//                    if (read(worker_data->client_socket, page->GetData(), header.page_size) == 0) {
//                        cout << "in 【server】 Connection disconnected." << endl;
//                        break;
//                    }
                    if (read_loop(worker_data->client_socket, page->GetData(), header.page_size) <= 0) {
                        break;
                    }
                    //page->WUnlatch();
                    //write(worker_data->client_socket, (char *) &header.page_size, sizeof(header.page_size));
                    if (write_loop(worker_data->client_socket,
                                   (char *) &header.page_size,
                                   sizeof(header.page_size)) <= 0) {
                    }
                    //3.3解锁页面，
                    chunk->UnpinPageImpl(header.page_no, true);
                    break;
                case GET:
                    page = chunk->FetchPageImpl(header.page_no, false);
                    //write(worker_data->client_socket, (char *) &header.page_size, sizeof(header.page_size));
                    if (write_loop(worker_data->client_socket,
                                   (char *) &header.page_size,
                                   sizeof(header.page_size)) <= 0) {
                    }
                    //page->RLatch();
                    //write(worker_data->client_socket, page->GetData(), header.page_size);
                    if (write_loop(worker_data->client_socket, page->GetData(), header.page_size) <= 0) {
                    }
                    //page->RUnlatch();
                    chunk->UnpinPageImpl(header.page_no, false);
                    break;
                default:
                    cout << "Invalid msg type" << endl;
            }
        }
        cout << "Thread exit: " << endl;
        gettimeofday(&eTime, NULL);
        long exeTime = (eTime.tv_sec - sTime.tv_sec) * 1000000 + (eTime.tv_usec - sTime.tv_usec);
        printf("thread deal all cost  %ld\n us", exeTime);
        return NULL;
    }
};


#endif //CXX_SERVER_H
