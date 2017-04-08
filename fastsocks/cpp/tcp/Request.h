//
// Created by 朱乾 on 17/3/19.
//

#ifndef TCPLIB_REQUEST_H
#define TCPLIB_REQUEST_H

#include "TcpConstant.h"

#ifdef ANDROID
#include <jni.h>
#endif

class NativeByteBuffer;

class Request{
public:
    Request(NativeByteBuffer *buffer, int32_t cmdId, uint32_t flags, uint64_t  timeout, onSendCompleteFunc sendCompleteFunc);
    ~Request();

    uint32_t requestFlags;
    uint64_t requestTimeout;
    onSendCompleteFunc sendCompleteCallback;
    int32_t requestCmdId;
    NativeByteBuffer *requestBuffer;
    int64_t requestStartTime = 0;
    int32_t currentRetryCount = 0;
    uint32_t currentConnectionToken = 0;
    int32_t messageSeq = 0;

    void onComplete(NativeByteBuffer *buffer, uint32_t code, std::string desc);
    bool isTimeout(int64_t currentTime);
    bool canRetry();

#ifdef ANDROID
    jobject onSendPtr = nullptr;//回调之后记得删除
#endif
private:

};

#endif //TCPLIB_REQUEST_H
