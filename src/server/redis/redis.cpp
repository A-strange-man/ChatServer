#include "redis.h"
#include <iostream>
#include <thread>
#include <stdlib.h>
using namespace std;

Redis::Redis() : pulish_context_(nullptr), subscribe_context_(nullptr)
{
}


Redis::~Redis()
{
    if (pulish_context_ != nullptr) 
    {
        redisFree(pulish_context_);
    }

    if (subscribe_context_ != nullptr) 
    {
        redisFree(subscribe_context_);
    }
}


bool Redis::connect() 
{
    pulish_context_ = redisConnect("127.0.0.1", 6379);
    subscribe_context_ = redisConnect("127.0.0.1", 6379);

    if (subscribe_context_ == nullptr || pulish_context_ == nullptr) 
    {
        cerr << "connect redis failed!" << endl;
        return false;
    }

    // 在独立的线程中 监听通道上的事件，有消息则上报
    thread t([&](){
        observer_channel_message();
    });
    t.detach();

    cout << "connect redis-server success!" << endl;
    return true;
}


bool Redis::publish(int channel, string message)
{
    // PUBLISH 命令可以直接用redisCommand，该命令不会阻塞
    redisReply* replay = (redisReply*) redisCommand(this->pulish_context_, "PUBLISH %d %s", channel, message.c_str());
    if (replay == nullptr)
    {
        cerr << "publish command failed !" << endl;
        return false;
    }

    freeReplyObject(replay);
    return true;
}


bool Redis::subscribe(int channel)
{
    // 不能直接用redisCommand，会阻塞等待
    // 先将命令组装好，存入本地缓存，再将命令发送到redis-server,  取消了最后一步的等待响应
    
    if (REDIS_ERR == redisAppendCommand(this->subscribe_context_, "SUBSCRIBE %d", channel))
    {
        cerr << "subscribe command failed !" << endl;
        return false;
    }

    int done = 0;
    while (!done)
    {
        // redisBufferWrite 可以循环发送缓冲区，直到缓冲区数据发送完毕，done被置为 1
        if (REDIS_ERR == redisBufferWrite(this->subscribe_context_, &done))
        {
            cerr << "subscribe command failed !" << endl;
            return false;
        }
    }

    // 这里省略 redisGetReplay这一步，由另外的线程专门获取响应
    
    return true;
}


bool Redis::unsubscribe(int channel)
{
    if (REDIS_ERR == redisAppendCommand(this->subscribe_context_, "UNSUBSCRIBE %d", channel))
    {
        cerr << "unsubscribe command failed !" << endl;
        return false;
    }

    int done = 0;
    while (!done)
    {
        // redisBufferWrite 可以循环发送缓冲区，直到缓冲区数据发送完毕，done被置为 1
        if (REDIS_ERR == redisBufferWrite(this->subscribe_context_, &done))
        {
            cerr << "unsubscribe command failed !" << endl;
            return false;
        }
    }

    return true;
}


void Redis::init_notify_handler(function<void(int, string)> func)
{
    this->notify_message_handler_ = func;
}


void Redis::observer_channel_message()
{
    redisReply* reply = nullptr;
    // 循环阻塞等待消息
    while (REDIS_OK == redisGetReply(this->subscribe_context_, (void**)&reply))
    {
        // 订阅收到的消息是一个三元组
        if (reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr)
        {
            // 上报消息
            notify_message_handler_(atoi(reply->element[1]->str), reply->element[2]->str);
        }
    }
    
    cerr << "================observer_channel_message quit!=====================" << endl;
    freeReplyObject(reply);
}


