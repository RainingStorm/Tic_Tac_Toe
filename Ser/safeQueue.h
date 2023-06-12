#ifndef SAFEQUEUE_H
#define SAFEQUEUE_H

#include<queue>
#include <mutex>


template <typename T>
class SafeQueue
{
private:
    SafeQueue() {}
    ~SafeQueue() {}
    SafeQueue(SafeQueue &&other) {}
    SafeQueue(const SafeQueue &other) {}
    SafeQueue& operator=(const SafeQueue &other) {}

    std::queue<T> queue_ser; // 选择服务队列
    std::mutex mutex_ser; // 选择服务队列互斥信号量
    std::queue<T> queue_ai; // 选择服务队列
    std::mutex mutex_ai; // 选择服务队列互斥信号量
    std::queue<std::pair<T,T>> queue_p2p; // 选择服务队列
    std::mutex mutex_p2p; // 选择服务队列互斥信号量

public:
    bool empty_ser() // 返回队列是否为空
    {
        std::unique_lock<std::mutex> lock(mutex_ser); // 互斥信号变量加锁，防止m_queue被改变

        return queue_ser.empty();
    }

    int size_ser()
    {
        std::unique_lock<std::mutex> lock(mutex_ser); // 互斥信号变量加锁，防止m_queue被改变

        return queue_ser.size();
    }

    // 队列添加元素
    void enqueue_ser(T &t)
    {
        std::unique_lock<std::mutex> lock(mutex_ser);
        queue_ser.emplace(t);
    }


    // 队列取出元素
    void dequeue_ser(T &t)
    {
        std::unique_lock<std::mutex> lock(mutex_ser); // 队列加锁

        t = std::move(queue_ser.front()); // 取出队首元素，返回队首元素值，并进行右值引用

        queue_ser.pop(); // 弹出入队的第一个元素
    }

    bool empty_ai() // 返回队列是否为空
    {
        std::unique_lock<std::mutex> lock(mutex_ai); // 互斥信号变量加锁，防止m_queue被改变

        return queue_ai.empty();
    }

    int size_ai()
    {
        std::unique_lock<std::mutex> lock(mutex_ai); // 互斥信号变量加锁，防止m_queue被改变

        return queue_ai.size();
    }

    // 队列添加元素
    void enqueue_ai(T &t)
    {
        std::unique_lock<std::mutex> lock(mutex_ai);
        queue_ai.emplace(t);
    }


    // 队列取出元素
    void dequeue_ai(T &t)
    {
        std::unique_lock<std::mutex> lock(mutex_ai); // 队列加锁

        t = std::move(queue_ai.front()); // 取出队首元素，返回队首元素值，并进行右值引用

        queue_ai.pop(); // 弹出入队的第一个元素
    }

    bool empty_p2p() // 返回队列是否为空
    {
        std::unique_lock<std::mutex> lock(mutex_p2p); // 互斥信号变量加锁，防止m_queue被改变

        return queue_p2p.empty();
    }

    int size_p2p()
    {
        std::unique_lock<std::mutex> lock(mutex_p2p); // 互斥信号变量加锁，防止m_queue被改变

        return queue_p2p.size();
    }

    // 队列添加元素
    void enqueue_p2p(std::pair<T,T> &&t)  // 由于传过来的pair是个右值，所以这里用右值引用
    {
        std::unique_lock<std::mutex> lock(mutex_p2p);
        queue_p2p.emplace(t);
    }


    // 队列取出元素
    void dequeue_p2p(std::pair<T,T> &t)
    {
        std::unique_lock<std::mutex> lock(mutex_p2p); // 队列加锁

        t = std::move(queue_p2p.front()); // 取出队首元素，返回队首元素值，并进行右值引用

        queue_p2p.pop(); // 弹出入队的第一个元素
    }

    static SafeQueue* getInstance() {
        static SafeQueue instance;
        return &instance;
    }
};


#endif