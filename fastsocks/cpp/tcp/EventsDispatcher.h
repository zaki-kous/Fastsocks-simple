//
// Created by 朱乾 on 17/2/27.
// 事件分发器

#ifndef TCPWORK_EVENTSDISPATCHER_H
#define TCPWORK_EVENTSDISPATCHER_H

#include <stdint.h>
#include "TcpConstant.h"

class EventsDispatcher {
public:
    EventsDispatcher(void *object, EventType type);
    void onEvent(uint32_t events);

    int64_t time;
    void* eventObject;

    EventType eventType;
};


#endif //TCPWORK_EVENTSDISPATCHER_H