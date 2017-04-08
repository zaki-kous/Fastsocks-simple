//
// Created by 朱乾 on 17/2/27.
//

#ifndef TCPWORK_CONNECTIONSMANAGER_H
#define TCPWORK_CONNECTIONSMANAGER_H

#include <pthread.h>
#include <vector>
#include <stdint.h>
#include <sys/epoll.h>
#include "TcpConstant.h"

#ifdef ANDROID
#include <jni.h>
#endif
class EventsDispatcher;
class TcpSocket;
class TcpConnection;
class NativeByteBuffer;
class Datacenter;
class NetworkMessage;

class ConnectionsManager{
public:
    ConnectionsManager();
    ~ConnectionsManager();

    int64_t getCurrentTimeMillis();
    int32_t getCurrentTime();

    static ConnectionsManager &getInstance();

    void scheduleEvent(EventsDispatcher *event, uint32_t time);
    void scheduleTask(std::function<void()> task);
    void clearUp();

#ifdef ANDROID
    void senRequest(NativeByteBuffer *buffer, uint32_t cmdId, uint32_t flags, uint64_t  timeout, onSendCompleteFunc sendCompleteFunc, jobject androidPtr);
#endif
    void removeEvent(EventsDispatcher *eventsDispatcher);
    void onConnectionRecviedData(TcpConnection *connection, int32_t cmd, NativeByteBuffer *buffer, uint32_t packetLength);
    void setDelegate(ConnectionManagerDelegate *connectionManagerDelegate);
    int beginHandshake();
    int handshakeResult(NativeByteBuffer *buffer);
    void init(uint64_t uid);
    void applySvrAddr(std::string addr, uint16_t port, bool singleSvr);
    void setUin(uint64_t uid);
    void resumeNetwork(bool isPush);
    void pauseNetwork();
    void setNetworkAvailable(bool hasNetwork);
    ConnetionState getConnectionState();

    uint64_t getUin();
    int32_t generateMessageSeq(uint32_t cmd);
#ifdef ANDROID
    static void useJavaVM(JavaVM *vm);
#endif
private:
    static void *attachThread(void *data);
    void select();
    void wakeup();
    void checkPendingTasks();
    int callEvents(int64_t time);
    void initDatacenter(Datacenter *datacenter);
    void processRequestQueue();
    void sendMessageToConnection(std::vector<std::unique_ptr<NetworkMessage>> &networkMessages, bool resetPingTime);

    void attachConnection(TcpSocket *connection);
    void detachConnection(TcpSocket *connection);
    void onConnectionClosed(TcpConnection *connection);
    void onConnectionConected(TcpConnection *connection);
    bool isNetworkAvailable();
    Request *getRequestWithSeq(int32_t seq);

    Datacenter *mDatacenter = nullptr;
    std::list<EventsDispatcher *> events;
    std::queue<std::function<void()>> tasks;
    std::vector<TcpSocket *> activeConnections;
    ConnectionManagerDelegate *delegate = nullptr;
    ConnetionState connetionState = ConnectionStateConnecting;
    bool networkAvailable = true;
    bool networkPaused = false;
    int64_t lastPauseTime = 0;//上一次暂停的时间
    timespec timeSpec;
    uint64_t uin;
    int32_t seq = 0;//自增的seq

    requestList requestsQueue;
    requestList runningRequests;
    int epollFd;
    struct epoll_event *epollEvents;
    int eventFd;
    int *pipeFd;
    int64_t lastPackSendTime = 0;

    pthread_mutex_t mutex;
    int32_t pingInterval = NORMAL_PING_INTERVAL;
    int32_t nextSleepTimeout = CONNECTION_BACKGROUND_KEEP_TIME;
    NativeByteBuffer *networkBuffer;

    pthread_t networkThread;

    friend class TcpSocket;
    friend class TcpConnection;
};

#ifdef ANDROID
extern JavaVM *javaVM;
extern JNIEnv *jniEnv;
extern jclass jclass_ByteBuffer;
extern jmethodID jclass_ByteBuffer_allocateDirect;
#endif
#endif //TCPWORK_CONNECTIONSMANAGER_H
