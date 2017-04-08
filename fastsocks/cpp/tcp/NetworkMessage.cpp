//
// Created by 朱乾 on 17/4/1.
//

#include "NetworkMessage.h"
#include "NativeByteBuffer.h"
#include "BufferStorage.h"
#include "ConnectionsManager.h"
#include "FileLog.h"

#define DEFAULT_HEAD_LENGTH 20
#define DEFAULT_VERSION 1

NetworkMessage::NetworkMessage() {
    DEBUG_D("NetworkMessage(%p).", this);
}

NetworkMessage::~NetworkMessage() {
    DEBUG_D("~NetworkMessage(%p) cmd : %d.", this, cmdId);
}

void NetworkMessage::serializeToBuffer(NativeByteBuffer *buffer) {
    if(buffer == nullptr){
        packLength = DEFAULT_HEAD_LENGTH;
    } else {
        packLength = (uint16_t) DEFAULT_HEAD_LENGTH + (uint16_t) buffer->limit();
    }
    outgoingBody = BufferStorage::getInstance().getBuffer(packLength);

    outgoingBody->writeInt16(packLength);
    outgoingBody->writeByte(DEFAULT_HEAD_LENGTH);
    outgoingBody->writeByte(DEFAULT_VERSION);
    outgoingBody->writeInt32(cmdId);
    outgoingBody->writeInt64(ConnectionsManager::getInstance().getUin());
    outgoingBody->writeInt32(messageSeq);

    if(buffer != nullptr){
        DEBUG_D("serializeToBuffer address(%p)", buffer);
        outgoingBody->writeBytes(buffer);
        buffer->reuse();
    }
    DEBUG_D("send data pkgLen : %d, headLen : %d, version : %d, cmd : %d, uin : %lld, seq :%d. ",
            packLength, DEFAULT_HEAD_LENGTH, DEFAULT_VERSION,
            cmdId, ConnectionsManager::getInstance().getUin(), messageSeq);
}