//
// Created by zaki on 17/2/27.
//

#ifndef TCPWORK_TCPCONNECTION_H
#define TCPWORK_TCPCONNECTION_H

#include <sys/epoll.h>
#include <netinet/in.h>
#include <string>

class ConnectionsManager;
class EventsDispatcher;
class ByteStream;
class NativeByteBuffer;

class TcpSocket{
public:
    TcpSocket();
    virtual ~TcpSocket();

    void writeBuffer(NativeByteBuffer *buffer);
    void openConnection(std::string address, uint16_t port);
    void dropConnection();
    void setTimeout(time_t time);
    bool isDisconnected();

protected:
    void onEvent(uint32_t events);
    void checkTimeout(int64_t now);

    virtual void onRecvData(NativeByteBuffer *buffer) = 0;
    virtual void onDisconnected(int reason) = 0;
    virtual void onConnected() = 0;

private:
    int socketFd = -1;
    int64_t lastEventTime = 0;
    bool onConnectedSent = false;
    struct epoll_event eventMsk;
    time_t timeout;


    EventsDispatcher *eventsDispatcher;

    struct sockaddr_in socketAddress;
    ByteStream *outgoingByteStream = nullptr;

    void closeSocket(int reason);
    bool checkScoketError();
    void adjustWriteOp();

    friend class ConnectionsManager;
    friend class EventsDispatcher;
};
#endif //TCPWORK_TCPCONNECTION_H
