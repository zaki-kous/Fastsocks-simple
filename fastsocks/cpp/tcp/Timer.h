//
// Created by 朱乾 on 17/2/27.
//

#ifndef TCPWORK_TIMER_H
#define TCPWORK_TIMER_H

#include <stdint.h>
#include <unistd.h>
#include <functional>

class EventsDispatcher;

class Timer{
public:
    Timer(std::function<void()> function);
    ~Timer();

    void start();
    void stop();
    void setTimeout(uint32_t ms);

private:
    void onEvent();

    bool started = false;
    uint32_t timeout;
    EventsDispatcher *eventsDispatcher;
    std::function<void()> callback;

    friend class EventsDispatcher;
};


#endif //TCPWORK_TIMER_H
