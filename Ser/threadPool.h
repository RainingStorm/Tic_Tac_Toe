#ifndef THREADPOOL_H
#define THREADPOOL_H

#include<iostream>
#include<vector>
#include<thread>
#include <sstream>
#include <mutex>
#include <queue>
#include"sem.h"
#include"safeQueue.h"
#include "tic.h"

#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#define MAXLINE 10

using namespace std;



// typedef unsigned long long ULL;

// std::string getThreadIdOfString(const std::thread::id & id)
// {
//     std::stringstream ssin;
//     ssin << id;
//     return ssin.str();
// }

// ULL getThreadIdOfULL(const std::thread::id & id)
// {
//     return std::stoull(getThreadIdOfString(id));
// }




template <typename T>
class threadpool {
private:
    vector<thread> pool_ser;
    vector<thread> pool_ai;
    vector<thread> pool_p2p;

    // 线程的数量
    int m_thread_number;

    // 请求队列中最多允许的、等待处理的请求的数量  
    int m_max_requests;

    // 请求队列
    SafeQueue<T>* que;

    // 是否结束线程          
    bool m_stop;

    // 是否有任务需要处理
    sem mission_ser;
    sem mission_ai;
    sem mission_p2p;

    // 任务队列是否有空位可以添加
    sem rest_ser;
    sem rest_ai;
    sem rest_p2p;

    std::mutex mutex_player;
    std::queue<T> queue_player;

    static void work_ser(threadpool* this_pool) {
        while (!this_pool->m_stop) {
            this_pool->mission_ser.wait();
            T cfd;
            this_pool->que->dequeue_ser(cfd);
            this_pool->rest_ser.post();
            printf("有用户接入...\n");
            LOOP:
            int choice = -1;
            char buffer[MAXLINE];
            int n = read(cfd, buffer, MAXLINE);
            if (n < 0) {
                // 处理错误
                perror("read error");
                exit(-1);
            }
            else if (n == 0) {
                // 客户端断开连接
                printf("客户端断开连接\n");
                close(cfd);
                cout<<"the "<<cfd-3<<"th client has been disconnected"<<endl;
            }
            else {
                // 处理读取到的数据
                std::string message(buffer, n);
                choice = 0 + message[0] - '0';
                if (choice == 0) {
                    close(cfd);
                    cout<<"the "<<cfd-3<<"th client has been disconnected"<<endl;
                } else if (choice == 1) {
                    this_pool->append_ai(cfd);
                } else if (choice == 2) {
                    this_pool->append_p2p(cfd);
                    // printf("1\n");
                } else {
                    goto LOOP;
                }
            
            }   
        }
        pthread_exit(0);
    }

    static void work_ai(threadpool* this_pool) {
        while (!this_pool->m_stop) {
            HumanPlay humanPlay;
            this_pool->mission_ai.wait();
            T cfd;
            this_pool->que->dequeue_ai(cfd);
            this_pool->rest_ai.post();
            humanPlay.play(cfd);
            this_pool->append_ser(cfd);
            // close(cfd);
            // cout<<"the "<<cfd-3<<"th client has been disconnected"<<endl;
        }
        pthread_exit(0);
    }

    static void work_p2p(threadpool* this_pool) {
        // std::thread::id tid = std::this_thread::get_id();
        // int priority = getThreadIdOfULL(tid) % this_pool->m_thread_number + 1;
        while (!this_pool->m_stop) {
            // while (this_pool->que->size_p2p()/2 < priority) {}

            std::pair<T,T> cfds{};
            this_pool->mission_p2p.wait();
            this_pool->que->dequeue_p2p(cfds);
            this_pool->rest_p2p.post();

            p2p_play(cfds.first, cfds.second);

            this_pool->append_ser(cfds.first);
            this_pool->append_ser(cfds.second);


            // T cfd1;
            // this_pool->mission_p2p.wait();
            // this_pool->que->dequeue_p2p(cfd1);
            // this_pool->rest_p2p.post();

            // T cfd2;
            // this_pool->mission_p2p.wait();
            // this_pool->que->dequeue_p2p(cfd2);
            // this_pool->rest_p2p.post();

            // p2p_play(cfd1, cfd2);

            // this_pool->append_ser(cfd1);
            // this_pool->append_ser(cfd2);

        }
        pthread_exit(0);
    }

    static void p2p_play(T cfd1, T cfd2) {
        char buf[MAXLINE];

        buf[0] = '1';
        buf[1] = '\0';
        write(cfd1, buf, strlen(buf));
        buf[0] = '2';
        write(cfd2, buf, strlen(buf));

        int front = -1, rear = -1;
        while (true) {
            char str1[MAXLINE];
            int n = read(cfd1, str1, MAXLINE);
            if (n == 0) {
			    printf("the other side has been closed.\n");
                str1[0] = '0';
                write(cfd2, str1, strlen(str1));
                return;
            }
            if (str1[0] == '*') {
                return;
            }
            front = str1[0] - '0';
            std::cout<<1<<":"<<front<<std::endl;

            char str1_[MAXLINE];
            str1_[0] = str1[0];
            str1_[1] = '\0';
            write(cfd2, str1_, MAXLINE);
            if (front == 0) return;


            char str2[MAXLINE];
            n = read(cfd2, str2, MAXLINE);
            if (n == 0) {
			    printf("the other side has been closed.\n");
                str2[0] = '0';
                write(cfd1, str2, strlen(str2));
                return;
            }
            if (str2[0] == '*') {
                return;
            }
            rear = str2[0] - '0';
            std::cout<<2<<":"<<rear<<std::endl;

            char str2_[MAXLINE];
            str2_[0] = str2[0];
            str2_[1] = '\0';
            write(cfd1, str2_, MAXLINE);
            if (rear == 0) return;
        }

    }

public:
    threadpool(int thread_number = 8, int max_requests = 1000) : 
        m_thread_number(thread_number), m_max_requests(max_requests), 
        que(SafeQueue<T>::getInstance()), m_stop(false), mission_ser(0), 
        mission_ai(0), mission_p2p(0), rest_ser(max_requests), rest_ai(max_requests), 
        rest_p2p(max_requests){

        if((thread_number <= 0) || (max_requests <= 0) ) {
            throw std::exception();
        }

        pool_ser = vector<thread>(m_thread_number);
        pool_ai = vector<thread>(m_thread_number);
        pool_p2p = vector<thread>(m_thread_number);

        // 创建thread_number 个线程，并将他们设置为脱离线程。
        //创建多个线程
	    for (int i = 0; i < m_thread_number; ++i){ 
            pool_ser[i] = thread(work_ser, this);
            pool_ser[i].detach();
            printf( "create the %dth ser_thread\n", i);
	    }

        for (int i = 0; i < m_thread_number; ++i){ 
            pool_ai[i] = thread(work_ai, this);
            pool_ai[i].detach();
            printf( "create the %dth ai_thread\n", i);
	    }

        for (int i = 0; i < m_thread_number; ++i){ 
            pool_p2p[i] = thread(work_p2p, this);
            pool_p2p[i].detach();
            printf( "create the %dth p2p_thread\n", i);
	    }
    }

    ~threadpool() {
        m_stop = true;
    }

    void append_ser( T request ) {
        // 操作工作队列时一定要加锁，因为它被所有线程共享。
        rest_ser.wait();
        que->enqueue_ser(request);
        mission_ser.post();
    }

    void append_ai( T request ) {
        // 操作工作队列时一定要加锁，因为它被所有线程共享。
        rest_ai.wait();
        que->enqueue_ai(request);
        mission_ai.post();
    }

    void append_p2p( T request ) {
        // 操作工作队列时一定要加锁，因为它被所有线程共享。
        std::unique_lock<std::mutex> lock(mutex_player);
        queue_player.push(request);
        if (queue_player.size() > 1) {
            T player1 = queue_player.front();
            queue_player.pop();
            T player2 = queue_player.front();
            queue_player.pop();

            rest_p2p.wait();
            que->enqueue_p2p({player1, player2});
            mission_p2p.post();
        }
    }


};

#endif