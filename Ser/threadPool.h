#ifndef THREADPOOL_H
#define THREADPOOL_H

#include<iostream>
#include<vector>
#include<thread>
#include"sem.h"
#include"safeQueue.h"
#include "tic.h"

#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

using namespace std;

template <typename T>
class threadpool {
private:
    vector<thread> pool;

    // 线程的数量
    int m_thread_number;

    // 请求队列中最多允许的、等待处理的请求的数量  
    int m_max_requests;

    // 请求队列
    SafeQueue<T> que;

    // 是否结束线程          
    bool m_stop;

    // 是否有任务需要处理
    sem mission;

    // 任务队列是否有空位可以添加
    sem rest;

    static void work(threadpool* this_pool) {
        while (!this_pool->m_stop) {
            HumanPlay humanPlay;
            this_pool->mission.wait();
            T cfd;
            this_pool->que.dequeue(cfd);
            this_pool->rest.post();
            humanPlay.play(cfd);
            close(cfd);
            cout<<"the "<<cfd-3<<"th client has been disconnected"<<endl;
            
            
        }
        pthread_exit(0);
    }

public:
    threadpool(int thread_number = 8, int max_requests = 1000) : 
        m_thread_number(thread_number), m_max_requests(max_requests), 
        m_stop(false), mission(0), rest(max_requests) {

        if((thread_number <= 0) || (max_requests <= 0) ) {
            throw std::exception();
        }

        pool = vector<thread>(m_thread_number);

        // 创建thread_number 个线程，并将他们设置为脱离线程。
        //创建多个线程
	    for (int i = 0; i < m_thread_number; ++i){ 
            pool[i] = thread(work, this);
            pool[i].detach();
            printf( "create the %dth thread\n", i);
	    }
    }

    ~threadpool() {
        m_stop = true;
    }

    void append( T request ) {
        // 操作工作队列时一定要加锁，因为它被所有线程共享。
        rest.wait();
        que.enqueue(request);
        mission.post();
    }


};

#endif