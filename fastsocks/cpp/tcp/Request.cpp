//
// Created by 朱乾 on 17/3/19.
//

#include "Request.h"
#include "NativeByteBuffer.h"
#include "FileLog.h"
#include "ConnectionsManager.h"

static uint64_t MAX_REQUEST_TIMEOUT = DEFAULT_PACK_TIMEOUT;
static int32_t MAX_RETRY_COUNT = DEFAULT_RETRY_COUNT;

Request::Request(NativeByteBuffer *buffer, int32_t cmdId, uint32_t flags, uint64_t timeout,
                 onSendCompleteFunc sendCompleteFunc) {
    requestBuffer = buffer;
    requestCmdId = cmdId;
    requestFlags = flags;
    if(timeout > 0) {
        requestTimeout = timeout;
    } else {
        requestTimeout = MAX_REQUEST_TIMEOUT;
    }
    sendCompleteCallback = sendCompleteFunc;
}

Request::~Request() {
    if (requestBuffer != nullptr) {
        requestBuffer->reuse();
    }

#ifdef ANDROID
    if (onSendPtr != nullptr) {
        jniEnv->DeleteGlobalRef(onSendPtr);
        onSendPtr = nullptr;
    }
#endif
    DEBUG_D("pop request cmd : %d seq : %d durtion %lld ", requestCmdId, messageSeq,
            requestStartTime != 0 ? ConnectionsManager::getInstance().getCurrentTimeMillis() - requestStartTime : 0);
}

void Request::onComplete(NativeByteBuffer *buffer, uint32_t code, std::string desc) {
    if(sendCompleteCallback != nullptr){
        sendCompleteCallback(buffer, code, desc);
    }
}

bool Request::isTimeout(int64_t currentTime) {
    if(requestTimeout != 0 && (currentTime - requestStartTime) >= requestTimeout){
        return true;
    } else {
        return false;
    }
}

bool Request::canRetry() {
    if(currentRetryCount != 0 && currentRetryCount > (int32_t) MAX_RETRY_COUNT){
        return false;
    } else {
        return true;
    }
}




