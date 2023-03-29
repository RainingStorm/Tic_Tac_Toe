#ifndef SEM_H
#define SEM_H

#include <pthread.h>
#include <exception>
#include <semaphore.h>
//线程同步机制封装类


// 信号量类
class sem{
public:
    sem(){
        if(sem_init(&m_sem,0,0)!=0){
            throw std::exception();
        }
    }
    sem(int num){
        if(sem_init(&m_sem,0,num)!=0){
            throw std::exception();
        }
    }
    ~sem(){
        sem_destroy(&m_sem);
    } 

    // 等待信号量
    bool wait(){
        return sem_wait(&m_sem)==0;
    }

    // 增加信号量
    bool post(){
        return sem_post(&m_sem)==0;
    }

private:
    sem_t m_sem;

};

#endif
