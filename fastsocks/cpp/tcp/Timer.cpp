//
// Created by 朱乾 on 17/2/27.
//

#include "Timer.h"
#include "EventsDispatcher.h"
#include "ConnectionsManager.h"

Timer::Timer(std::function<void()> function) {
    callback = function;
    eventsDispatcher = new EventsDispatcher(this, EventTypeTimer);
}

Timer::~Timer(){
    stop();
    if(eventsDispatcher != nullptr){
        delete eventsDispatcher;
        eventsDispatcher = nullptr;
    }
}

void Timer::start() {
    //
    if(started || timeout == 0){
        return;
    }
    started = true;
    ConnectionsManager::getInstance().scheduleEvent(eventsDispatcher, timeout);
}

void Timer::stop() {
    if(!started){
        return;
    }
    started = false;
    ConnectionsManager::getInstance().removeEvent(eventsDispatcher);
}

void Timer::setTimeout(uint32_t ms) {
    if(ms == timeout){
        return;
    }
    timeout = ms;
    if(started){
        ConnectionsManager::getInstance().removeEvent(eventsDispatcher);
        ConnectionsManager::getInstance().scheduleEvent(eventsDispatcher, timeout);
    }
}
void Timer::onEvent() {
    if(callback != nullptr){
        callback();
    }
}