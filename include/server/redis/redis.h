#pragma once

#include <hiredis/hiredis.h>
#include <pthread.h>
#include <functional>

using namespace std;

class Redis
{
public:
    Redis();
    ~Redis();

    // 连接redis服务器
    bool connect();

    // 向指定的channel发布消息
    bool publish(int channel, string message);

    // 订阅消息
    bool subscribe(int channel);

    // 取消订阅
    bool unsubscribe(int channel);

    // 接收订阅通道中的消息，在独立的线程中进行
    void observer_channel_message();

    void init_notify_handler(function<void(int, string)> func);

private:
    // 负责发布消息
    redisContext* pulish_context_;
    // 负责订阅消息
    redisContext* subscribe_context_;

    // 回调，收到订阅的消息，给相应的服务器上报
    function<void(int, string)> notify_message_handler_;
};
