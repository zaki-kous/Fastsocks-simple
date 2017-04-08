//
// Created by 朱乾 on 17/3/14.
//

#ifndef TCPLIB_DATACENTER_H
#define TCPLIB_DATACENTER_H

#include <map>
#include "TcpConstant.h"

class NativeByteBuffer;
class TcpConnection;
class ConnectionsManager;

class Datacenter{
public:
    Datacenter();
    ~Datacenter();

    void addAddressAndPort(std::string address, uint16_t port);
    void setAddressAndPort(std::string address, uint16_t port);
    void switchAddressOrPort();
    TcpConnection *getConnection(bool create);
    std::string getCurrentAddress();
    uint16_t getCurrentPort();
    bool isHandshakeing();
    bool isHandshakeSuccess();
    int32_t widthSpecialSeq(int32_t cmd);

private:
    void onHandshakeConnectionClosed(TcpConnection *connection);
    void onHandshakeConnectionConnected(TcpConnection *connection);
    void onHandshakeResult(NativeByteBuffer *buffer);
    TcpConnection *createConnection();
    void beginHandshake(bool reCreate);
    void sendPing();

    std::vector<std::string> addresses;
    const int16_t *defaultPorts = new int16_t[11] {-1, 80, -1, 443, -1, 443, -1, 80, -1, 443, -1};
    std::map<std::string, uint16_t *> ports;
    HandshakeState handshakeState = HandshakeStateIdel;
    TcpConnection *connection = nullptr;

    uint32_t currentAddressIndex = 0;//当前地址索引
    uint32_t currentPortIndex = 0;//当前端口索引

    NioServer nioServer;

    friend class ConnectionsManager;
};
#endif //TCPLIB_DATACENTER_H
