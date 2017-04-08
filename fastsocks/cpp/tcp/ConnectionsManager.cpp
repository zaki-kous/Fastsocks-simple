//
// Created by zaki on 17/2/27.
//
#include <stdlib.h>
#include <sys/eventfd.h>
#include <algorithm>
#include <zlib.h>
#include "openssl/rand.h"
#include "Request.h"
#include "ConnectionsManager.h"
#include "EventsDispatcher.h"
#include "FileLog.h"
#include "TcpSocket.h"
#include "TcpConnection.h"
#include "BufferStorage.h"
#include "NativeByteBuffer.h"
#include "Datacenter.h"
#include "NetworkMessage.h"

#ifdef ANDROID
#include <jni.h>
JavaVM *javaVM = nullptr;
JNIEnv *jniEnv = nullptr;
jclass jclass_ByteBuffer = nullptr;
jmethodID jclass_ByteBuffer_allocateDirect = 0;
#endif

static bool done = false;

NativeByteBuffer *decompressGZip(NativeByteBuffer *data);

ConnectionsManager::ConnectionsManager() {
    if((epollFd = epoll_create(128)) == -1){
        DEBUG_E("unable to create epoll instance.");
        exit(1);
    }
    int flags;
    if((flags = fcntl(epollFd, F_GETFD, NULL)) < 0){
        DEBUG_W("fcntl(%d, F_GETFD) fail", epollFd);
    } else{
        DEBUG_D("fcntl(%d, F_GETFD) success", epollFd);
    }
    if(!(flags | FD_CLOEXEC)){
        if(fcntl(epollFd, F_SETFD, flags | FD_CLOEXEC) < 0){
            DEBUG_W("fcntl(%d, F_SETFD) fail", epollFd);
        } else{
            DEBUG_D("fcntl(%d, F_SETFD) success", epollFd);
        }
    }

    if((epollEvents = new epoll_event[128]) == nullptr){
        DEBUG_W("unable to allocate epoll events.");
        exit(1);
    }

    eventFd = eventfd(0, EFD_NONBLOCK);
    if(eventFd != -1){
        struct epoll_event event = {0};
        event.data.ptr = new EventsDispatcher(&eventFd, EventTypeFd);
        event.events = EPOLLIN | EPOLLET;
        if(epoll_ctl(epollFd, EPOLL_CTL_ADD, eventFd, &event) == -1){
            DEBUG_E("unable add eventFd to epoll.");
            eventFd = -1;
        }
    }

    if(eventFd == -1){
        pipeFd = new int[2];
        if(pipe(pipeFd) == -1){
            DEBUG_E("unable to create pipe");
            exit(1);
        }

        int flags;
        if((flags = fcntl(pipeFd[0], F_GETFD)) == -1){
            DEBUG_E("unable to fcntl(pipeFd[0] : %d) F_GETFD error.",pipeFd[0]);
            exit(1);
        }

        if(fcntl(pipeFd[0], F_SETFD, flags | O_NONBLOCK) == -1){
            DEBUG_E("unable to fcntl(pipeFd[0] : %d) F_SETFD error.", pipeFd[0]);
            exit(1);
        }

        if((flags = fcntl(pipeFd[1], F_GETFD)) == -1){
            DEBUG_E("unable to fcntl(pipeFd[1] : %d) F_GETFD error.",pipeFd[1]);
            exit(1);
        }

        if(fcntl(pipeFd[1], F_SETFD, flags | O_NONBLOCK) == -1){
            DEBUG_E("unable to fcntl(pipeFd[1] : %d) F_SETFD error.", pipeFd[1]);
            exit(1);
        }

        struct epoll_event pipeEvent = {};
        pipeEvent.data.ptr = new EventsDispatcher(pipeFd, EventTypePip);
        pipeEvent.events = EPOLLIN;
        if(epoll_ctl(epollFd, EPOLL_CTL_ADD, pipeFd[0], &pipeEvent) != 0){
            DEBUG_E("unable add pipe to epoll.");
            exit(1);
        }
    }
    networkBuffer = new NativeByteBuffer((uint32_t) READ_BUFFER_SIZE);
    pthread_mutex_init(&mutex, NULL);
    mDatacenter = new Datacenter();
    initDatacenter(mDatacenter);
}

ConnectionsManager::~ConnectionsManager() {
    if(epollFd != 0){
        close(epollFd);
        epollFd = 0;
    }
    pthread_mutex_destroy(&mutex);
}

void *ConnectionsManager::attachThread(void *data) {
#ifdef ANDROID
    DEBUG_D("attach android thread...");
    javaVM->AttachCurrentThread(&jniEnv, NULL);
#endif
    ConnectionsManager *connectionsManager = (ConnectionsManager *)data;

    do{
        connectionsManager->select();
    } while (!done);
    DEBUG_D("attach android thread...");
    return nullptr;
}

void ConnectionsManager::select() {
    checkPendingTasks();
    int eventCount = epoll_wait(epollFd, epollEvents, 128, callEvents(getCurrentTimeMillis()));
    checkPendingTasks();
    int64_t now = getCurrentTimeMillis();
    callEvents(now);

    for(int32_t i = 0; i < eventCount; i++){
        EventsDispatcher *eventsDispatcher = (EventsDispatcher *) epollEvents[i].data.ptr;
        eventsDispatcher->onEvent(epollEvents[i].events);
    }

    size_t actConnectionCount = activeConnections.size();
    for(uint32_t i = 0; i < actConnectionCount ; i++){
        activeConnections[i]->checkTimeout(now);
    }

    if(lastPauseTime != 0 && abs(int32_t (now - lastPauseTime)) >= nextSleepTimeout) {
        if(!networkPaused){
            DEBUG_E("join pasuse status");
            networkPaused = true;
            if (mDatacenter->getConnection(false)->getConnectionToken() != 0) {
                //连接成功，保证心跳不超时
                mDatacenter->getConnection(false)->setTimeout(70);
            }
        }
    }

    if(networkPaused){
        pingInterval = PUSH_PING_INTERVAL;//push连接的心跳时长
    } else {
        pingInterval = NORMAL_PING_INTERVAL;//普通连接的心跳时长
    }

    if(delegate != nullptr){
        delegate->onUpdate();
    }

    if (!mDatacenter->isHandshakeing() && uin != 0) {//只会调用一次
        //当前没有连接，准备连接
        mDatacenter->beginHandshake(true);
    } else {
        if(lastPackSendTime != 0 && (now - lastPackSendTime) >= pingInterval){
            lastPackSendTime = getCurrentTimeMillis();
            //发送心跳
            mDatacenter->sendPing();//
            if (networkPaused) {
                networkPaused = false;
                lastPauseTime = now - 50 * 1000;//十秒之后暂停
                DEBUG_D("reset lastPauseTime...");
            }
        }
        //处理数据包
        processRequestQueue();
    }
}

#ifdef ANDROID
void ConnectionsManager::useJavaVM(JavaVM *vm) {
    javaVM = vm;

    JNIEnv *env = NULL;
    if(javaVM->GetEnv((void **)&env, JNI_VERSION_1_6) != JNI_OK){
        DEBUG_E("cant't get jniEnv");
        exit(1);
    }

    jclass_ByteBuffer = (jclass) env->NewGlobalRef(env->FindClass("java/nio/ByteBuffer"));

    if(jclass_ByteBuffer == NULL){
        DEBUG_E("cant't find ByteBuffer class");
        exit(1);
    }

    jclass_ByteBuffer_allocateDirect = env->GetStaticMethodID(jclass_ByteBuffer, "allocateDirect", "(I)Ljava/nio/ByteBuffer;");
    if(jclass_ByteBuffer_allocateDirect == NULL){
        DEBUG_E("can't find ByteBuffer method allocateDirect.");
        exit(1);
    }
    DEBUG_D("get java ByteBuffer success..");
}
#endif

void ConnectionsManager::processRequestQueue() {
    static std::vector<std::unique_ptr<NetworkMessage>> neededSendMessages;//需要发送的网络消息
    bool resetPingTime = false;//有回包的数据包可以代替心跳
    neededSendMessages.clear();

    int64_t currentTime = getCurrentTimeMillis();

    for (requestIter iter = runningRequests.begin(); iter != runningRequests.end();) {
        Request *request = iter->get();
        TcpConnection *connection = mDatacenter->getConnection(true);
//        if(connection->getConnectionToken() != 0 &&
//                connection->getConnectionToken() == request->currentConnectionToken){
//            // 当前连接未收到回包
//            iter++;
//            continue;
//        }
        if(!networkAvailable || connection->getConnectionToken() == 0){
            //当前没有连接成功
            if(!(request->requestFlags & RequestFlagReSend)){
                DEBUG_E("witout connect packet error cmd : %d", request->requestCmdId);
                //回调发送失败
                request->requestStartTime = currentTime;
                request->onComplete(nullptr, errorCodeConnectFail, "current witout connect.");
                iter = runningRequests.erase(iter);
                continue;
            }
        }

        if(connection->getConnectionToken() > 0 && connection->getConnectionToken() != request->currentConnectionToken){
            //连接改变
            if(mDatacenter->isHandshakeSuccess() || request->requestFlags & RequestFlagWithoutLogin){
                if(!(request->requestFlags & RequestFlagWithoutAck)){
                    resetPingTime = true;
                }
                //登录成功，继续发送
                NetworkMessage *networkMessage = new NetworkMessage();
                networkMessage->cmdId = request->requestCmdId;
                networkMessage->packLength = (uint16_t) request->requestBuffer->limit();
                networkMessage->outgoingBody = request->requestBuffer;
                DEBUG_D("resend data pkgLen : %d, cmd : %d, seq :%d. ",
                        networkMessage->outgoingBody->limit(), request->requestCmdId, request->messageSeq);

                neededSendMessages.push_back(std::unique_ptr<NetworkMessage>(networkMessage));
                request->currentConnectionToken = connection->getConnectionToken();
                request->requestStartTime = currentTime;
                iter++;
                continue;
            }
        }

        if(request->isTimeout(currentTime)){
            DEBUG_E("request is timeout cmd : %d seq : %d flags : %d retry %d.",
                    request->requestCmdId, request->messageSeq, request->requestFlags, request->currentRetryCount);
            if(request->requestFlags & RequestFlagReSend) {
                //可以重发
                request->currentRetryCount++;
                if(!request->canRetry()){
                    //回调发送次数限制
                    request->onComplete(nullptr, errorCodeRequestRetryLimit, "request retry limit.");
                    iter = runningRequests.erase(iter);
                    continue;
                }
                request->requestStartTime = currentTime;
            } else {
                //回调超时
                DEBUG_E("packet timeout cmd : %d", request->requestCmdId);
                request->onComplete(nullptr, errorCodeRequestTimeout, "request timeout.");
                iter = runningRequests.erase(iter);
                continue;
            }
        }
        iter++;
    }

    for(requestIter iter = requestsQueue.begin(); iter != requestsQueue.end();){
        Request *request = iter->get();
        TcpConnection *connection = mDatacenter->getConnection(true);
        if (connection == nullptr) {
            iter++;
            continue;
        }
        bool removeToRunning = false;
        //不需要重发
        if(connection->getConnectionToken() == 0){
            //当前没有连接成功
            if(!(request->requestFlags & RequestFlagReSend)){
                DEBUG_D("request flags %d.", request->requestFlags);
                request->onComplete(nullptr, errorCodeWithoutConnectin, "current witout connect.");
            } else {
                removeToRunning = true;
            }
        } else {
            //可以发送
            if(mDatacenter->isHandshakeSuccess()){
                removeToRunning = true;
            } else {
                //没有登录成功
                if(request->requestFlags & RequestFlagWithoutLogin){
                    removeToRunning = true;
                } else {
                    //回调发送失败
                    request->onComplete(nullptr, errorCodeWithoutLogin, "current witout login.");
                }
            }
        }
        if(removeToRunning){
            request->currentConnectionToken = connection->getConnectionToken();
            request->requestStartTime = (uint32_t)(getCurrentTimeMillis() / 1000);

            NetworkMessage *networkMessage = new NetworkMessage();
            networkMessage->cmdId = request->requestCmdId;
            networkMessage->messageSeq = generateMessageSeq(0);
            networkMessage->serializeToBuffer(request->requestBuffer);//将上层数据包放到outgoingBody

            request->requestBuffer = networkMessage->outgoingBody;

            request->messageSeq = networkMessage->messageSeq;//赋值seq
            request->requestStartTime = (int64_t) getCurrentTimeMillis();

            neededSendMessages.push_back(std::unique_ptr<NetworkMessage>(networkMessage));

            if (!(request->requestFlags & RequestFlagWithoutAck)) {
                //有回包的数据包
                runningRequests.push_back(std::move(*iter));
                resetPingTime = true;
            } else {
                request->onComplete(nullptr, 0, "");
            }
        }
        DEBUG_D("erase request(%p) cmdId %d messageSeq %d flags : %d.", request,
                request->requestCmdId, request->messageSeq, request->requestFlags);
        iter = requestsQueue.erase(iter);
    }
    sendMessageToConnection(neededSendMessages, resetPingTime);
}

Request* ConnectionsManager::getRequestWithSeq(int32_t seq) {
    for(requestIter iter = runningRequests.begin(); iter != runningRequests.end(); iter++) {
        Request *request = iter->get();
        if(request->messageSeq == seq) {
            return request;
        }
    }
    return nullptr;
}

void ConnectionsManager::sendMessageToConnection(std::vector<std::unique_ptr<NetworkMessage>> &networkMessages, bool resetPingTime) {
    if(networkMessages.empty()){
        return;
    }
    TcpConnection *connection = mDatacenter->getConnection(true);
    if(resetPingTime){
        lastPackSendTime = getCurrentTimeMillis();
    }
    ssize_t count = networkMessages.size();
    uint32_t currentSize = 0;
    std::vector<std::unique_ptr<NetworkMessage>> bufferMessages;
    for(uint32_t i = 0 ; i < count; i++){
        NetworkMessage *message = networkMessages[i].get();
        bufferMessages.push_back(std::move(networkMessages[i]));
        currentSize += message->packLength;
        if (currentSize >= MAX_SINGLE_PACK_SIZE || i == (count - 1)) {
            //可以发送
            NativeByteBuffer *sendBuffer = BufferStorage::getInstance().getBuffer(currentSize);
            ssize_t bufferCount = bufferMessages.size();
            for (uint32_t j = 0; j < bufferCount; j++) {
                NativeByteBuffer *bodyBuffer = bufferMessages[j].get()->outgoingBody;
                bodyBuffer->rewind();
                sendBuffer->writeBytes(bodyBuffer);
            }
            connection->sendData(sendBuffer);
            bufferMessages.clear();
            currentSize = 0;
        }
    }
}

void ConnectionsManager::init(uint64_t uid) {
    uin = uid;
    pthread_create(&networkThread, NULL, (ConnectionsManager::attachThread), this);
}

void ConnectionsManager::setUin(uint64_t uid) {
    if (uid != 0 && uid != uin) {
        scheduleTask([&, uid]{
            DEBUG_D("update uid : %lld.", uid);
            uin = uid;
        });
    }
}

void ConnectionsManager::initDatacenter(Datacenter *datacenter) {
    //随机一个地址

}

void ConnectionsManager::applySvrAddr(std::string addr, uint16_t port, bool singleSvr) {
    scheduleTask([&, addr, port, singleSvr]{
        if (singleSvr) {
            mDatacenter->setAddressAndPort(addr, port);
        } else {
            mDatacenter->addAddressAndPort(addr, port);
        }
    });
}

void ConnectionsManager::resumeNetwork(bool isPush) {
    scheduleTask([&, isPush]{
        if (isPush) {
            if(networkPaused){
                lastPauseTime = getCurrentTimeMillis();
                networkPaused = false;
                DEBUG_D("resumeNetwork due to isPush true , networkPaused is true.");
            } else if(lastPauseTime != 0){
                lastPauseTime = getCurrentTimeMillis();
                networkPaused = false;
                DEBUG_D("resumeNetwork due to isPush true , lastPauseTime ! = 0.");
            }
        } else {
            DEBUG_D("resumeNetwork due to isPush false.");
            lastPauseTime = 0;
            networkPaused = false;
            if (mDatacenter->getConnection(false)->getConnectionToken() != 0) {
                mDatacenter->getConnection(false)->setTimeout(30);
            }
        }
    });
}

void ConnectionsManager::pauseNetwork() {
    if(lastPauseTime != 0){
        return;
    }
    DEBUG_D("pauseNetwork reset lastPauseTime.");
    lastPauseTime = getCurrentTimeMillis();
}

void ConnectionsManager::setNetworkAvailable(bool hasNetwork) {
    scheduleTask([&, hasNetwork]{
        DEBUG_D("setNetworkAvailable hasNetwork is : %s.", FormatTools::toString(hasNetwork).c_str());
        networkAvailable = hasNetwork;
        if (!networkAvailable) {
            //如果没有网络，暂停连接
            connetionState = ConnectionStateWaitingForNetwork;
        } else {
            //准备连接
            if (mDatacenter->isHandshakeing() && uin != 0) {
                mDatacenter->createConnection()->connect();
            }
        }
        if(delegate != nullptr){
            delegate->onConnectionStateChanged(connetionState);
        }
    });
}

void ConnectionsManager::setDelegate(ConnectionManagerDelegate *connectionManagerDelegate) {
    delegate = connectionManagerDelegate;
}

int ConnectionsManager::beginHandshake() {
    lastPackSendTime = getCurrentTimeMillis();
    if(delegate != nullptr){
        return delegate->onHandshakeConnected(nullptr);
    }
    return 0;
}

int ConnectionsManager::handshakeResult(NativeByteBuffer *buffer) {
    if(delegate != nullptr){
        return delegate->onHandshakeConnected(buffer);
    }
    return 0;
}

void ConnectionsManager::onConnectionClosed(TcpConnection *connection) {
    if(connection == nullptr){
        return;
    }
    //清除握手标识
    if(mDatacenter->isHandshakeing()){
        mDatacenter->onHandshakeConnectionClosed(connection);
    }
    if(networkAvailable){
        //回调底层库重连
        if(connetionState != ConnectionStateConnecting){
            connetionState = ConnectionStateConnecting;
            if(delegate != nullptr){
                delegate->onConnectionStateChanged(connetionState);
            }
        }
    } else {
        if(connetionState != ConnectionStateWaitingForNetwork){
            connetionState = ConnectionStateWaitingForNetwork;
            if(delegate != nullptr){
                delegate->onConnectionStateChanged(connetionState);
            }
        }
    }
}

void ConnectionsManager::onConnectionConected(TcpConnection *connection) {
    if(mDatacenter->isHandshakeing()){
        //没有握手，开始握手
        mDatacenter->onHandshakeConnectionConnected(connection);
    } else {
        //握手成功，开始处理数据包
        processRequestQueue();
    }
}

bool ConnectionsManager::isNetworkAvailable() {
    return networkAvailable;
}

ConnetionState ConnectionsManager::getConnectionState() {
    return connetionState;
}

uint64_t ConnectionsManager::getUin() {
    return uin;
}
#ifdef ANDROID
void ConnectionsManager::senRequest(NativeByteBuffer *buffer, uint32_t cmdId, uint32_t flags,
                                    uint64_t timeout, onSendCompleteFunc sendCompleteFunc, jobject androidPtr) {
    if (uin == 0) {
        DEBUG_E("without login sendRequest error cmd %d.", cmdId);
        //当前已经登出，回调失败
        if (buffer != nullptr) {
            buffer->reuse();
        }
        if(sendCompleteFunc != nullptr){
            sendCompleteFunc(nullptr, errorCodeWithoutLogin, "current witout login.");
        }
        return;
    }
    scheduleTask([&, buffer, cmdId, flags, timeout, sendCompleteFunc, androidPtr] {
        Request *request = new Request(buffer, cmdId , flags, timeout, sendCompleteFunc);
        request->onSendPtr = androidPtr;
        requestsQueue.push_back(std::unique_ptr<Request>(request));
        processRequestQueue();
    });

}
#endif

void ConnectionsManager::clearUp() {
    scheduleTask([&] {
        uin = 0;
        //清除消息队列
        for (requestIter iter = requestsQueue.begin(); iter != requestsQueue.end();) {
            Request *request = iter->get();
            request->onComplete(nullptr, errorCodeWithoutLogin, "current witout login.");
            iter = requestsQueue.erase(iter);
        }

        for (requestIter iter = runningRequests.begin(); iter != runningRequests.end();) {
            Request *request = iter->get();
            request->onComplete(nullptr, errorCodeWithoutLogin, "current witout login.");
            iter = requestsQueue.erase(iter);
        }
        //关闭连接
        mDatacenter->getConnection(false)->closeConnect();
        //重置握手状态
        mDatacenter->handshakeState = HandshakeStateIdel;
    });
}

int32_t ConnectionsManager::generateMessageSeq(uint32_t cmd) {
    int32_t requestSeq = mDatacenter->widthSpecialSeq(cmd);
    if(requestSeq){
        return requestSeq;
    }
    return seq++;
}

int64_t ConnectionsManager::getCurrentTimeMillis() {
    clock_gettime(CLOCK_REALTIME, &timeSpec);
    return (uint64_t) timeSpec.tv_sec * 1000 + (uint64_t) timeSpec.tv_nsec / 1000000;
}

int32_t ConnectionsManager::getCurrentTime() {
    return getCurrentTimeMillis() / 1000;
}

ConnectionsManager& ConnectionsManager::getInstance() {
    static ConnectionsManager connectionsManager;
    return connectionsManager;
}

void ConnectionsManager::scheduleTask(std::function<void()> task) {
    if(task == nullptr){
        return;
    }
    pthread_mutex_lock(&mutex);
    tasks.push(task);
    pthread_mutex_unlock(&mutex);
    wakeup();
}

void ConnectionsManager::checkPendingTasks() {
    while (true){
        std::function<void()> task;
        pthread_mutex_lock(&mutex);
        if(tasks.empty()){
            pthread_mutex_unlock(&mutex);
            return;
        }
        task = tasks.front();
        tasks.pop();
        pthread_mutex_unlock(&mutex);
        task();
    }
}

void ConnectionsManager::attachConnection(TcpSocket *connection) {
    if(std::find(activeConnections.begin(), activeConnections.end(), connection) != activeConnections.end()){
        return;
    }
    activeConnections.push_back(connection);
}

void ConnectionsManager::detachConnection(TcpSocket *connection) {
    std::vector<TcpSocket *>::iterator iterator = std::find(activeConnections.begin(), activeConnections.end(), connection);
    if(iterator != activeConnections.end()){
        activeConnections.erase(iterator);
    }
}

int ConnectionsManager::callEvents(int64_t now) {
    //等待
    if (uin == 0) {
        return -1;
    }
    for(std::list<EventsDispatcher *>::iterator iter = events.begin(); iter != events.end();){
        EventsDispatcher *eventsDispatcher = (*iter);
        if(eventsDispatcher->time <= now){
            iter = events.erase(iter);
            eventsDispatcher->onEvent(0);
        }else{
            int diff = (int) (eventsDispatcher->time - now);
            return diff > 1000 ? 1000 : diff;
        }
    }

    if(!networkPaused){
        return 1000;
    }

    int32_t timeToPing = pingInterval - abs((int32_t) (now - lastPackSendTime));
    if(timeToPing <= 0){
        return 1000;
    }
    return timeToPing;
}

void ConnectionsManager::scheduleEvent(EventsDispatcher *event, uint32_t time) {
    if(event == nullptr){
        return;
    }
    event->time = time;
    std::list<EventsDispatcher *>::iterator iter;

    for(iter = events.begin(); iter != events.end(); iter++){
        if((*iter)->time > time){
            break;
        }
    }
    events.insert(iter, event);
}

void ConnectionsManager::removeEvent(EventsDispatcher *eventsDispatcher) {
    if(eventsDispatcher == nullptr){
        return;
    }

    for(std::list<EventsDispatcher *>::iterator iter = events.begin(); iter != events.end(); iter ++){
        if(*iter == eventsDispatcher){
            events.erase(iter);
            break;
        }
    }
}

void ConnectionsManager::onConnectionRecviedData(TcpConnection *connection, int32_t cmd,
                                                 NativeByteBuffer *buffer, uint32_t seq) {
    if(connetionState != ConnectionStateConnected){
        //第一次收到消息回调上面连接成功
        connetionState = ConnectionStateConnected;
        if(delegate != nullptr){
            delegate->onConnectionStateChanged(connetionState);
        }
    }
    //特殊逻辑：包体的第一个字节等于0x07代表需要解压缩
    NativeByteBuffer *decompressBuffer = nullptr;
    if(buffer->hasRemaining() && *(buffer->bytes() + buffer ->position()) == 0x07){
        decompressBuffer = decompressGZip(buffer);
        DEBUG_D("needed decompress, orginal length : %d decompress length : %d.", buffer->remaining() - 1, decompressBuffer->limit());
        buffer = decompressBuffer;
    }

    if(seq == -1 && !mDatacenter->isHandshakeSuccess() && mDatacenter->isHandshakeing()){
        //握手数据包
        mDatacenter->onHandshakeResult(buffer);
    } else if(seq == -2 && mDatacenter->isHandshakeSuccess()){
        //心跳
        DEBUG_D("recv ping...");
    } else {
//        Request *request = getRequestWithSeq(seq);
        Request *request = nullptr;
        for (requestIter iter = runningRequests.begin(); iter != runningRequests.end(); iter++) {
            Request *sendRequest = iter->get();
            if (sendRequest->messageSeq == seq && sendRequest->requestCmdId == cmd - 1) {
                sendRequest->onComplete(buffer, 0, "");
                runningRequests.erase(iter);
                request = sendRequest;
                break;
            }
        }
        if (request == nullptr && delegate != nullptr) {
            //收到数据包
            delegate->onRecvMessages(cmd, buffer);
        }
    }

    if(decompressBuffer != nullptr){
        decompressBuffer->reuse();
    }
}

inline NativeByteBuffer *decompressGZip(NativeByteBuffer *data) {
    int retCode;
    z_stream stream;

    memset(&stream, 0, sizeof(z_stream));
    stream.avail_in = data->remaining() - 1;
    stream.next_in = data->bytes() + data->position() + 1;

    retCode = inflateInit2(&stream, 15 + 32);
    if (retCode != Z_OK) {
        DEBUG_E("can't decompress data");
        exit(1);
    }
    NativeByteBuffer *result = BufferStorage::getInstance().getBuffer((data->remaining() - 1) * 4);
    stream.avail_out = result->capacity();
    stream.next_out = result->bytes();
    while (1) {
        retCode = inflate(&stream, Z_NO_FLUSH);
        if (retCode == Z_STREAM_END) {
            break;
        }
        if (retCode == Z_OK) {
            NativeByteBuffer *newResult = BufferStorage::getInstance().getBuffer(result->capacity() * 2);
            memcpy(newResult->bytes(), result->bytes(), result->capacity());
            stream.avail_out = newResult->capacity() - result->capacity();
            stream.next_out = newResult->bytes() + result->capacity();
            result->reuse();
            result = newResult;
        } else {
            DEBUG_E("can't decompress data");
            exit(1);
        }
    }
    result->limit((uint32_t) stream.total_out);
    inflateEnd(&stream);
    return result;
}

void ConnectionsManager::wakeup() {
    if(pipeFd == nullptr){
        eventfd_write(eventFd, 1);
    } else{
        char ch = 'x';
        write(pipeFd[1], &ch, 1);
    }
}
