//
// Created by 朱乾 on 17/2/28.
//

#include "TcpConnection.h"
#include "Timer.h"
#include "FileLog.h"
#include "BufferStorage.h"
#include "NativeByteBuffer.h"
#include "ConnectionsManager.h"
#include "Datacenter.h"

static uint32_t lastConnectionToken = 1;

TcpConnection::TcpConnection(Datacenter *datacenter) {
    mDatacenter = datacenter;
    tcpConnectionStatus = TcpConnectionStatusIdel;

    reconnectTimer = new Timer([&]{
        DEBUG_D("reconnectTimer execute , begin connect.");
        reconnectTimer->stop();
        connect();
    });
}

TcpConnection::~TcpConnection() {
    DEBUG_D("~TcpConnection() exe");
    if(reconnectTimer != nullptr){
        reconnectTimer->stop();
        delete reconnectTimer;
        reconnectTimer = nullptr;
    }
}

void TcpConnection::connect() {
    if(!ConnectionsManager::getInstance().isNetworkAvailable()){
        ConnectionsManager::getInstance().onConnectionClosed(this);
        return;
    }
    if(tcpConnectionStatus == TcpConnectionStatusConnected || tcpConnectionStatus == TcpConnectionStatusConnecting){
        //当前连接已经成功或者正在连接中，无需连接
//        DEBUG_W("current tcpConnectionStatus is %d , don't connect.", tcpConnectionStatus);
        return;
    }
    //从数据中心获取到服务器ip和端口
    serverAddress = mDatacenter->getCurrentAddress();
    serverPort = mDatacenter->getCurrentPort();
    tcpConnectionStatus = TcpConnectionStatusConnecting;

    reconnectTimer->stop();

    //清除上一个连接的数据
    if(resetOfData != nullptr) {
        resetOfData->reuse();
        resetOfData = nullptr;
    }
    lastPacketLength = 0;
    wasConnected = false;
    DEBUG_D("connect server address : %s, port : %d", serverAddress.c_str(), serverPort);
    currentConnectionRecvData = false;
    openConnection(serverAddress, serverPort);
    if(isTryingNextPort){
        //切换端口连接超时为8秒
        setTimeout(8);
    } else {
        //切换ip连接超时为15秒
        setTimeout(15);
    }
}

void TcpConnection::reconnect() {
    closeConnect();
    tcpConnectionStatus = TcpConnectionStatusReconnecting;
    connect();
}

void TcpConnection::sendData(NativeByteBuffer *buffer) {
    if(tcpConnectionStatus == TcpConnectionStatusIdel || tcpConnectionStatus == TcpConnectionStatusReconnecting
            || tcpConnectionStatus == TcpConnectionStatusClosed){
        connect();
        buffer->reuse();
        return;
    }
    if(isDisconnected()){
        DEBUG_D("tcpconnection is closed, don't send data. reset data length : %d.", buffer->remaining());
        buffer->reuse();
        return;
    }
    buffer->rewind();
    //发送数据包 包含多个(包头 ＋ 包体)
    writeBuffer(buffer);
}

void TcpConnection::onRecvData(NativeByteBuffer *buffer) {
    failedConnectionCount = 0;//收到数据包代表该连接可用性比较高
    NativeByteBuffer *parseLaterBuffer = nullptr;

    if(resetOfData != nullptr){
        if(lastPacketLength == 0){
            //未处理完的包头
            if(resetOfData->capacity() - resetOfData->position() >= buffer->limit()){
                resetOfData->limit(resetOfData->position() + buffer->limit());
                resetOfData->writeBytes(buffer);
                buffer = resetOfData;
            } else {
                //数据过长，restOfData装不下
                NativeByteBuffer *newBuffer = BufferStorage::getInstance().getBuffer(resetOfData->limit() + buffer->limit());
                resetOfData->rewind();
                newBuffer->writeBytes(resetOfData);
                newBuffer->writeBytes(buffer);
                buffer = newBuffer;
                resetOfData->reuse();
                resetOfData = newBuffer;
            }
        } else {
            //说明有未处理完的数据包
            uint32_t len;
            if(lastPacketLength - resetOfData->position() <= buffer->limit()) {
                //buffer＋restOfData数据大于或者等于一个数据包的长度
                len = lastPacketLength - resetOfData->position();
            } else {
                //buffer＋restOfData数据小于一个数据包的长度
                len = buffer->limit();
            }
            uint32_t oldLimit = buffer->limit();
            buffer->limit(len);
            resetOfData->writeBytes(buffer);
            buffer->limit(oldLimit);
            if(resetOfData->position() == lastPacketLength){
                parseLaterBuffer = buffer->hasRemaining() ? buffer : nullptr;
                buffer = resetOfData;
            } else {
                return;
            }
        }
    }

    buffer->rewind();
    while (buffer->hasRemaining()) {
        if(!currentConnectionRecvData){
            //保存当前ip和端口的索引 TODO
            isTryingNextPort = false;
            setTimeout(30);
        }
        currentConnectionRecvData = true;
        uint32_t headLength = 0;//包头长度
        uint32_t packetLength = 0;//包长度
        uint32_t markPosition = buffer->position();//纪录posion;
        if(buffer->remaining() < 20) {
            DEBUG_D("recv data < DEFAULT_HEADER_LENGTH : %d.", 20);
            restHeader(buffer);
            break;
        }
        PacketHeader packetHeader = {0};
        memcpy(&packetHeader, (buffer->bytes() + markPosition), sizeof(PacketHeader));
        //检查包头
        if(!checkHeader(packetHeader)) {
            reconnect();
            return;
        }
        //获取到包头长度
        headLength = packetHeader.headLen;
        if(headLength > 20) {
            //继续获取包头
            if(buffer->remaining() < headLength){
                DEBUG_D("rest header %d to data.", (headLength - buffer->remaining()));
                restHeader(buffer);
                break;
            }
        }
        buffer->position(markPosition + headLength);
        packetLength = ntohs(packetHeader.pkgLen) - headLength;
        if(packetLength == buffer->remaining()) {
            //刚好读取到一个完整的数据包
            DEBUG_D("recv data equals pkgLen : %d.", packetLength);
        } else if(packetLength < buffer->remaining()) {
            //读取到的数据包比包体长
            DEBUG_D("recv data : %d more then pkgLen : %d.", buffer->remaining(), packetLength);
        } else {
            DEBUG_D("recv data : %d less then pkgLen : %d.", buffer->remaining(), packetLength);
            //未读取到一个完整的数据包
            uint32_t len = packetLength + headLength;//包头＋包体长度
            NativeByteBuffer *reuseLater = nullptr;
            if(resetOfData != nullptr && resetOfData->capacity() < len) {
                DEBUG_D("resetOfData capacity : %d less then packlen : %d.", resetOfData->capacity(), len);
                reuseLater = resetOfData;
                resetOfData = nullptr;
            }
            if(resetOfData == nullptr) {
                buffer->position(markPosition);
                resetOfData = BufferStorage::getInstance().getBuffer(len);
                resetOfData->writeBytes(buffer);
            } else {
                resetOfData->position(resetOfData->limit());
                resetOfData->limit(len);
            }
            lastPacketLength = len;
            if(reuseLater != nullptr) {
                reuseLater->reuse();
            }
            break;
        }
        //读取到了完整数据包，打印包头
        printHeader(packetHeader);
        uint32_t oldLimit = buffer->limit();
        buffer->limit(buffer->position() + packetLength);
        //回调给java层，解析成对应的数据
        ConnectionsManager::getInstance().onConnectionRecviedData(this, ntohl(packetHeader.cmd), buffer, ntohl(packetHeader.seq));
        buffer->position(buffer->limit());
        buffer->limit(oldLimit);

        if(resetOfData != nullptr) {
            if((lastPacketLength != 0 && resetOfData->position() == lastPacketLength)
                    || (lastPacketLength == 0 && !resetOfData->hasRemaining())) {
                resetOfData->reuse();
                resetOfData = nullptr;
            } else {
                resetOfData->compact();
                resetOfData->limit(resetOfData->position());
                resetOfData->position(0);
            }
        }

        if (parseLaterBuffer != nullptr) {
            buffer = parseLaterBuffer;
            parseLaterBuffer = nullptr;
        }
    }
}

bool TcpConnection::checkHeader(PacketHeader &packetHeader) {
    if((packetHeader.version & 0xff) != 0 && (packetHeader.version & 0xff) != 1) {
        DEBUG_E("recv data version is error : %d ", (packetHeader.version & 0xff));
        return false;
    } else if(packetHeader.headLen > htons(packetHeader.pkgLen)) {
        DEBUG_E("recv data headLen %d > .pkgLen : %d", packetHeader.headLen, htons(packetHeader.pkgLen));
        return false;
    } else if(packetHeader.headLen < 20) {
        DEBUG_E("recv data headLen : %d < DEFAULT_HEADER_LENGTH : %d", packetHeader.headLen, 20);
        return false;
    }

    return true;
}

void TcpConnection::restHeader(NativeByteBuffer *buffer) {
    if(resetOfData == nullptr || (resetOfData != nullptr && resetOfData->position() != 0)) {
        NativeByteBuffer *reuseLater = resetOfData;
        //包头未读完整
        resetOfData = BufferStorage::getInstance().getBuffer(16 * 1028);
        resetOfData->writeBytes(buffer);
        resetOfData->limit(resetOfData->position());
        lastPacketLength = 0;
        if(reuseLater != nullptr) {
            //释放上一次的包头
            reuseLater->reuse();
        }
    } else {
        resetOfData->position(resetOfData->limit());
    }
}

void TcpConnection::printHeader(PacketHeader &packetHeader) {
    DEBUG_D("recv data pkgLen : %d, headLen : %d, version : %d, cmd : %d, uin : %lld, seq :%d. ",
            ntohs(packetHeader.pkgLen), (int32_t) packetHeader.headLen, (int32_t) packetHeader.version,
            ntohl(packetHeader.cmd), FormatTools::ntohll(packetHeader.uin), ntohl(packetHeader.seq));
}

void TcpConnection::closeConnect() {
    reconnectTimer->stop();
    if(tcpConnectionStatus == TcpConnectionStatusIdel || tcpConnectionStatus == TcpConnectionStatusClosed) {
        DEBUG_W("current tcpConnectionStatus is %d , don't have closeConnect.", tcpConnectionStatus);
        return;
    }
    tcpConnectionStatus = TcpConnectionStatusClosed;//以防重连
    dropConnection();
    ConnectionsManager::getInstance().onConnectionClosed(this);
}

void TcpConnection::onDisconnected(int reason) {
    reconnectTimer->stop();
    bool canSwitchToNextPort = wasConnected && !currentConnectionRecvData && reason == 2;//超时才需要切换ip
    if(resetOfData != nullptr) {
        resetOfData->reuse();
        resetOfData = nullptr;
    }
    connectionToken = 0;
    lastPacketLength = 0;
    wasConnected = false;
    if(tcpConnectionStatus != TcpConnectionStatusClosed || tcpConnectionStatus != TcpConnectionStatusIdel) {
        tcpConnectionStatus = TcpConnectionStatusIdel;
    }

    ConnectionsManager::getInstance().onConnectionClosed(this);

    if(tcpConnectionStatus == TcpConnectionStatusIdel && mDatacenter->isHandshakeing()) {//上层主动调用了握手才需要重连
        tcpConnectionStatus = TcpConnectionStatusReconnecting;
        failedConnectionCount++;
        if(failedConnectionCount == 1){
            if(currentConnectionRecvData){
                willRetryConnectCount = 5;
            } else {
                willRetryConnectCount = 1;
            }
        }

        if(ConnectionsManager::getInstance().isNetworkAvailable()) {
            if(canSwitchToNextPort || failedConnectionCount > willRetryConnectCount) {
                //超时和连接失败次数超过限制需要切换端口
                mDatacenter->switchAddressOrPort();
                failedConnectionCount = 0;
            }
            isTryingNextPort = true;
        }

        //准备重连
        reconnectTimer->setTimeout(1000);
        reconnectTimer->start();
    }
}

void TcpConnection::onConnected() {
    tcpConnectionStatus = TcpConnectionStatusConnected;
    connectionToken = lastConnectionToken++;//保证唯一
    wasConnected = true;
    DEBUG_D("connection success ip : %s, port : %d.", serverAddress.c_str(), serverPort);
    ConnectionsManager::getInstance().onConnectionConected(this);
}



uint32_t TcpConnection::getConnectionToken() {
    return connectionToken;
}

Datacenter* TcpConnection::getDatacenter() {
    return mDatacenter;
}
