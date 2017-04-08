//
// Created by 朱乾 on 17/3/14.
//

#include <algorithm>
#include "Datacenter.h"
#include "NativeByteBuffer.h"
#include "NetworkMessage.h"
#include "ConnectionsManager.h"
#include "TcpConnection.h"
#include "FileLog.h"

static const int HANDSHAKE_CMD = 257;//握手命令码
static const int PING_CMD = 259;//心跳命令码

static const char *adds[] = {
        "192.168.1.100",
        "192.168.1.110",
        "192.168.1.112",
        "192.168.1.113",
        "192.168.1.114"
};

Datacenter::Datacenter() {
    //随机一个默认的ip
    srand((uint32_t) time(NULL));
    uint32_t randomIndex = (uint32_t) rand() % 5;
    memset(&nioServer, 0, sizeof(nioServer));
    sprintf(nioServer.nioIp, "%s", adds[randomIndex]);
    nioServer.nioPort = 443;
//    addAddressAndPort(adds[randomIndex], 443);
    DEBUG_D("random address index %d : %s", randomIndex, nioServer.nioIp);
}

Datacenter::~Datacenter() {

}

//添加一个ip和端口
void Datacenter::addAddressAndPort(std::string address, uint16_t port) {
    //清除默认ip
    sprintf(nioServer.nioIp, "%s", "");
    if(std::find(addresses.begin(), addresses.end(), address) != addresses.end()){
        *(ports[address] + 1) = port;
        //已经存着该ip
        return;
    }
    DEBUG_D("add addr : %s port : %d.", address.c_str(), port);
    currentPortIndex = 0;
    currentAddressIndex = 0;
    addresses.push_back(address);
    std::random_shuffle(addresses.begin(), addresses.end());//随机打乱svr顺序
    ports[address] = new uint16_t[2]{0, 0};
    *(ports[address]) = port;
}

void Datacenter::setAddressAndPort(std::string address, uint16_t port) {
    sprintf(nioServer.nioIp, "%s", address.c_str());
    nioServer.nioPort = port;
    DEBUG_D("force set addr : %s port : %d.", nioServer.nioIp, port);
    //重连
    getConnection(false)->reconnect();
}

//切换ip或者端口
void Datacenter::switchAddressOrPort() {
    if (strlen(nioServer.nioIp)) {
        return;
    }
    //先增加端口的索引，再增加地址的索引
    if(currentPortIndex < 10){
        currentPortIndex++;
    } else {
        if(currentAddressIndex < addresses.size() - 1){
            currentAddressIndex++;
        } else {
            currentAddressIndex = 0;
        }
        currentPortIndex = 0;
    }
}

std::string Datacenter::getCurrentAddress() {
    if (strlen(nioServer.nioIp)) {
        DEBUG_D("used defalut ip : %s.", nioServer.nioIp);
        return nioServer.nioIp;
    }
    if(addresses.empty()){
        return "";
    }
    return addresses[currentAddressIndex];
}

uint16_t Datacenter::getCurrentPort() {
    if (strlen(nioServer.nioIp)) {
        return nioServer.nioPort;
    }
    if(ports.empty()){
        return 443;
    }
    if(currentPortIndex > 10){
        currentPortIndex = 0;
    }

    //先查找默认的端口
    int16_t port = defaultPorts[currentPortIndex];
    if(port == -1){
        uint16_t firstPort = (uint16_t) *(ports[getCurrentAddress()]);
        uint16_t secondPort = (uint16_t) *(ports[getCurrentAddress()] + 1);
        if (secondPort != 0) {
            if ((currentPortIndex / 2 + 1) % 2) {
                port = firstPort;
            } else {
                port = secondPort;
            }
        } else {
            port = firstPort;
        }
    }
    return (uint16_t) port;
}

TcpConnection* Datacenter::createConnection() {
    if(connection == nullptr){
        connection = new TcpConnection(this);
    }
    return connection;
}

void Datacenter::beginHandshake(bool reCreate) {//主动调用了该方法，底层才会去重连
    DEBUG_D("--beginHandshake---reCreate : %d", reCreate);
    handshakeState = HandshakeStateIng;
    //重新建立连接
    if(reCreate){
        createConnection()->closeConnect();
        createConnection()->connect();
    } else {
        NativeByteBuffer *buffer = (NativeByteBuffer *) ConnectionsManager::getInstance().beginHandshake();
        DEBUG_D("beginHandshake , send handshake request , buffer is %p.", buffer);
        if(buffer != nullptr){
            NetworkMessage networkMessage;
            networkMessage.cmdId = HANDSHAKE_CMD;
            networkMessage.messageSeq = widthSpecialSeq(HANDSHAKE_CMD);
            networkMessage.serializeToBuffer(buffer);
            getConnection(false)->sendData(networkMessage.outgoingBody);
        }
    }
}

void Datacenter::onHandshakeConnectionConnected(TcpConnection *connection) {
    if(handshakeState == HandshakeStateSuccessed){
        //已经握手成功
        return;
    }
    beginHandshake(false);
}

void Datacenter::onHandshakeConnectionClosed(TcpConnection *connection) {
    handshakeState = HandshakeStateIng;//模拟成正在握手
}

void Datacenter::onHandshakeResult(NativeByteBuffer *buffer) {
    if(ConnectionsManager::getInstance().handshakeResult(buffer) == 0){
        //握手成功
        DEBUG_D("-onHandshakeResult success.");
        handshakeState = HandshakeStateSuccessed;
        return;
    }
    DEBUG_E("-onHandshakeResult fail close connect.");
    //握手失败断开连接
    createConnection()->closeConnect();
    createConnection()->connect();
    return;
}

void Datacenter::sendPing() {
    NetworkMessage networkMessage;
    networkMessage.cmdId = PING_CMD;
    networkMessage.messageSeq = widthSpecialSeq(PING_CMD);
    networkMessage.serializeToBuffer(nullptr);
    getConnection(false)->sendData(networkMessage.outgoingBody);
    DEBUG_D("send ping...");
}

bool Datacenter::isHandshakeing() {
    return handshakeState != HandshakeStateIdel;
}

bool Datacenter::isHandshakeSuccess() {
    return handshakeState == HandshakeStateSuccessed;
}

int32_t Datacenter::widthSpecialSeq(int32_t cmd) {
    if(cmd == HANDSHAKE_CMD){
        return -1;
    } else if(cmd == PING_CMD) {
        return -2;
    }
    return 0;
}

TcpConnection* Datacenter::getConnection(bool create) {
    if(!isHandshakeing()){
        return nullptr;
    }
    if(create){
        createConnection()->connect();
    }
    return connection;
}