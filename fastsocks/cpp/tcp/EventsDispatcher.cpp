//
// Created by 朱乾 on 17/2/27.
//
#include <unistd.h>
#include <sys/eventfd.h>
#include "EventsDispatcher.h"
#include "TcpConnection.h"
#include "Timer.h"
#include "FileLog.h"

EventsDispatcher::EventsDispatcher(void *object, EventType type){
    eventObject = object;
    eventType = type;
}

void EventsDispatcher::onEvent(uint32_t events) {
    switch (eventType){
        case EventTypeConnection: {
            //连接事件
            TcpConnection *connection = (TcpConnection *) eventObject;
            connection->onEvent(events);
            break;
        }
        case EventTypeTimer: {
            //定时器事件
            Timer *timer = (Timer *) eventObject;
            timer->onEvent();
            break;
        }
        case EventTypePip: {
            //管道事件
            int *pipe = (int *) eventObject;
            char ch;
            ssize_t size = 1;
            while (size > 0) {
                size = read(pipe[0], &ch, 1);
            }
            break;
        }
        case EventTypeFd: {
            //EventFd事件
            int *eventFd = (int *) eventObject;
            uint64_t count;
            eventfd_read(eventFd[0], &count);
            break;
        }
        default:
            break;
    }
}