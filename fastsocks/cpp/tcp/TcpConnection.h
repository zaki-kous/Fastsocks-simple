//
// Created by 朱乾 on 17/2/28.
//

#ifndef TCPWORK_TCPMANAGER_H
#define TCPWORK_TCPMANAGER_H

#include "TcpSocket.h"
#include "TcpConstant.h"

class Datacenter;
class Timer;

class TcpConnection : public TcpSocket{
public:
    TcpConnection(Datacenter *datacenter);
    ~TcpConnection();

    void connect();
    void reconnect();
    void closeConnect();
    void sendData(NativeByteBuffer *buffer);

    uint32_t getConnectionToken();
    Datacenter *getDatacenter();


protected:
    void onRecvData(NativeByteBuffer *buffer) override;
    void onConnected() override;
    void onDisconnected(int reason) override;

private:
    bool checkHeader(PacketHeader &packetHeader);
    void restHeader(NativeByteBuffer *buffer);
    void printHeader(PacketHeader &packetHeader);
    enum TcpConnectionStatus{
        TcpConnectionStatusIdel,
        TcpConnectionStatusConnecting,
        TcpConnectionStatusReconnecting,
        TcpConnectionStatusConnected,
        TcpConnectionStatusClosed
    };
    Datacenter *mDatacenter;
    NativeByteBuffer *resetOfData = nullptr;
    uint32_t lastPacketLength;
    bool isTryingNextPort = false;
    bool wasConnected;
    bool currentConnectionRecvData = false;//纪录当前这个连接是否收到消息
    uint32_t willRetryConnectCount = 5;//连接失败尝试的次数限制
    uint32_t failedConnectionCount = 0;//连接失败尝试的次数
    std::string serverAddress;
    uint16_t serverPort;
    Timer *reconnectTimer;
    uint32_t connectionToken = 0;
    TcpConnectionStatus tcpConnectionStatus = TcpConnectionStatusIdel;
};

#endif //TCPWORK_TCPMANAGER_H
