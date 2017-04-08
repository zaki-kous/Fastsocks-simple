
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/tcp.h>
#include "TcpSocket.h"
#include "EventsDispatcher.h"
#include "ConnectionsManager.h"
#include "ByteStream.h"
#include "NativeByteBuffer.h"
#include "FileLog.h"

#ifndef EPOLLRDHUP
#define EPOLLRDHUP 0x2000
#endif

TcpSocket::TcpSocket() {
    eventsDispatcher = new EventsDispatcher(this, EventTypeConnection);
    outgoingByteStream = new ByteStream();
    lastEventTime = ConnectionsManager::getInstance().getCurrentTimeMillis();
}

TcpSocket::~TcpSocket() {
    if(eventsDispatcher != nullptr){
        delete eventsDispatcher;
        eventsDispatcher = nullptr;
    }
    if(outgoingByteStream != nullptr){
        delete outgoingByteStream;
        outgoingByteStream = nullptr;
    }

}

void TcpSocket::openConnection(std::string address, uint16_t port) {
    int epollFd = ConnectionsManager::getInstance().epollFd;
    if((socketFd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        DEBUG_E("can't create socket...");
        closeSocket(1);
        return;
    }

    memset(&socketAddress, 0, sizeof(sockaddr_in));

    socketAddress.sin_family = AF_INET;
    socketAddress.sin_port = htons(port);

    if(inet_pton(AF_INET, address.c_str(), &socketAddress.sin_addr.s_addr) != 1){
        DEBUG_E("connection ip is error address : %s", address.c_str());
        closeSocket(1);
        return;
    }

    int ok = 1;
    //设置tcp立马发生数据包
    if(setsockopt(socketFd, IPPROTO_TCP, TCP_NODELAY, &ok, sizeof(int))){
        DEBUG_W("setsockopt TCP_NODELAY error socket : %d", socketFd);
    }

    if(fcntl(socketFd, F_SETFL, O_NONBLOCK) == -1){
        DEBUG_E("fcntl set socket O_NONBLOCK error...socket : %d", socketFd);
        closeSocket(1);
        return;
    }

    if(connect(socketFd, (sockaddr *) &socketAddress, (socklen_t) sizeof(sockaddr_in)) == -1 && errno != EINPROGRESS){
        DEBUG_E("connect error...socket : %d, errorno : %d.", socketFd, errno);
        closeSocket(1);
    }else{
        ConnectionsManager::getInstance().attachConnection(this);
        DEBUG_D("connect address : %s ,port : %d success.", address.c_str(), port);
        eventMsk.events = EPOLLOUT | EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLET;
        eventMsk.data.ptr = eventsDispatcher;
        if(epoll_ctl(epollFd, EPOLL_CTL_ADD, socketFd, &eventMsk) != 0){
            DEBUG_E("epoll_ctl EPOLL_CTL_ADD socket error, epollFd : %d - socket : %d", epollFd, socketFd);
            closeSocket(1);
        }
    }
}


void TcpSocket::closeSocket(int reason) {
    lastEventTime = ConnectionsManager::getInstance().getCurrentTimeMillis();
    ConnectionsManager::getInstance().detachConnection(this);
    int epollFd = ConnectionsManager::getInstance().epollFd;
    if(socketFd >= 0){
        epoll_ctl(epollFd, EPOLL_CTL_DEL, socketFd, NULL);
        if(close(socketFd) != 0){
            DEBUG_E("can't close socket : %d.", socketFd);
        }
        socketFd = -1;
    }
    onConnectedSent = false;
    outgoingByteStream->clear();//清除发送队列
    onDisconnected(reason);
}

void TcpSocket::onEvent(uint32_t events) {
    if(events & EPOLLIN){
        //可读事件
        if(checkScoketError()){//如果断网，会收到可读事件，判断socket状态是否错误。
            DEBUG_E("connect(%p) onEvent recv EPOLLIN scoket error.", this);
            closeSocket(1);
            return;
        } else {
            ssize_t readCount = 0;
            NativeByteBuffer *recvBuffer = ConnectionsManager::getInstance().networkBuffer;
            while(true){
                recvBuffer->rewind();
                readCount = recv(socketFd, recvBuffer->bytes(), READ_BUFFER_SIZE, 0);
                DEBUG_D("recv readCount : %d.", readCount);
                if(readCount < 0){
                    closeSocket(0);
                    DEBUG_E("connection recv return 0 ,fail %p", this);
                    return;
                }
                if(readCount > 0){
                    recvBuffer->limit((uint32_t) readCount);
                    lastEventTime = ConnectionsManager::getInstance().getCurrentTimeMillis();
                    onRecvData(recvBuffer);
                }
                if(readCount != READ_BUFFER_SIZE){
                    break;
                }
            }
        }
    }

    if(events & EPOLLOUT){
        //可写事件
        if(checkScoketError()){
            DEBUG_E("connect(%p) onEvent recv EPOLLOUT scoket error.", this);
            closeSocket(1);
            return;
        }else{
            if(!onConnectedSent){
                //第一次可写代表连接成功
//                ConnectionsManager::getInstance().attachConnection(this);
                onConnected();
                lastEventTime = ConnectionsManager::getInstance().getCurrentTimeMillis();
                onConnectedSent = true;
            }
            NativeByteBuffer *buffer = ConnectionsManager::getInstance().networkBuffer;
            buffer->clear();
            outgoingByteStream->get(buffer);
            buffer->flip();

            uint32_t remaining = buffer->remaining();
            if(remaining > 0){
                //发送
                ssize_t length;
                if((length = send(socketFd, buffer->bytes(), remaining, 0)) < 0){
                    //写入失败
                    closeSocket(1);
                    return;
                } else {
                    DEBUG_D("send length : %d.", length);
                    outgoingByteStream->discard((uint32_t) length);
                    adjustWriteOp();
                }
            }
        }
    }

    if ((events & EPOLLRDHUP) || (events & EPOLLHUP)) {
        DEBUG_E("connection(%p) epoll LRDHUP event : %d.", this, events);
        closeSocket(1);
        return;
    }
    if (events & EPOLLERR) {
        DEBUG_E("connection(%p) epoll error event : %d.", this, events);
        return;
    }
}

bool TcpSocket::checkScoketError() {
    if(socketFd < 0){
        return true;
    }
    int ret;
    int code;
    socklen_t len = sizeof(int);
    ret = getsockopt(socketFd, SOL_SOCKET, SO_ERROR, &code, &len);
    return (ret || code) != 0;

}

void TcpSocket::writeBuffer(NativeByteBuffer *buffer) {
    //写入队列
    outgoingByteStream->append(buffer);
    //通知有新数据
    adjustWriteOp();
}

void TcpSocket::adjustWriteOp() {
    eventMsk.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLET;
    if(outgoingByteStream->hasData()){
        eventMsk.events |= EPOLLOUT;
    }
    eventMsk.data.ptr = eventsDispatcher;
    if(epoll_ctl(ConnectionsManager::getInstance().epollFd, EPOLL_CTL_MOD, socketFd, &eventMsk) != 0){
        DEBUG_E("TcpConnection epoll_ctl error, modeify socekt : %d failed.", socketFd);
        closeSocket(1);
    }
}

void TcpSocket::checkTimeout(int64_t now) {
    if(timeout > 0 && (now - lastEventTime) > timeout * 1000){
        DEBUG_E("connect(%p) is timeout.", this);
        closeSocket(2);
    }
}

void TcpSocket::dropConnection() {
    closeSocket(0);
}

void TcpSocket::setTimeout(time_t time) {
    timeout = time;
    lastEventTime = ConnectionsManager::getInstance().getCurrentTimeMillis();
}

bool TcpSocket::isDisconnected() {
    return socketFd < 0;
}


