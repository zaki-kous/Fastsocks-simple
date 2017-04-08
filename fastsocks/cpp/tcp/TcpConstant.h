//
// Created by 朱乾 on 17/2/27.
//

#ifndef TCPWORK_TCPCONSTANT_H
#define TCPWORK_TCPCONSTANT_H

#include <queue>
#include <list>
#include <functional>
#include <stdint.h>
#include <bits/unique_ptr.h>
#include "FormatTools.h"

#define PUSH_PING_INTERVAL 60 * 1000
#define NORMAL_PING_INTERVAL 20 * 1000
#define CONNECTION_BACKGROUND_KEEP_TIME 60 * 1000
#define READ_BUFFER_SIZE 1024 * 128
#define DEFAULT_PACK_TIMEOUT 30 * 1000
#define DEFAULT_RETRY_COUNT 3
#define MAX_SINGLE_PACK_SIZE 3 * 1024

class Request;

class NativeByteBuffer;

typedef std::function<void(NativeByteBuffer *buffer, uint32_t code, std::string desc)> onSendCompleteFunc;
typedef std::list<std::unique_ptr<Request>> requestList;

typedef requestList::iterator requestIter;

enum EventType {
    EventTypeConnection,
    EventTypePip,
    EventTypeTimer,
    EventTypeFd
};

enum ConnetionState{
    ConnectionStateConnecting = 1,
    ConnectionStateWaitingForNetwork = 2,
    ConnectionStateConnected = 3
};

enum errorCode {
    errorCodeWithoutConnectin = 300101,//当前没有连接
    errorCodeWithoutLogin = 300102,//当前连接没有握手成功
    errorCodeConnectFail = 300103,//当前连接中断
    errorCodeRequestTimeout = 300104,//发送数据包超时
    errorCodeRequestRetryLimit = 300105//重发次数限制
};

enum HandshakeState {
    HandshakeStateIdel = 0,
    HandshakeStateIng = 1,
    HandshakeStateSuccessed = 2
};

enum RequestFlag {
    RequestFlagWithoutAck = 1,//没有回包
    RequestFlagReSend = 2,//数据包可以重发
    RequestFlagWithoutLogin = 4,//数据包在握手之前发送
};

typedef struct {
    char nioIp[64];
    uint16_t  nioPort;
} NioServer;

typedef struct ConnectionManagerDelegate {
    virtual void onUpdate() = 0;
    virtual void onConnectionStateChanged(ConnetionState connetionState) = 0;
    virtual void onRecvMessages(int32_t cmd, NativeByteBuffer *data) = 0;
    virtual int onHandshakeConnected(NativeByteBuffer *buffer) = 0;
} ConnectionManagerDelegate;

#pragma pack(push, 1)
//一字节对齐，方便拷贝
typedef struct PacketHeader{
    uint16_t pkgLen; // 包大小(unsigned short)

    uint8_t headLen; // 头部大小

    uint8_t version; // 协议版本，当前为1

    int32_t cmd; // 协议命令字

    int64_t uin; // 用户账户uin（唯一的一个标识，跟登录相关）

    int32_t seq; // 包的seq
} PacketHeader;
#pragma pack(pop)

#endif //TCPWORK_TCPCONSTANT_H
