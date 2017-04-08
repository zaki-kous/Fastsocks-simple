//
// Created by 朱乾 on 17/4/1.
//

#ifndef ANDROIDAPP_NETWORKMESSAGE_H
#define ANDROIDAPP_NETWORKMESSAGE_H

#include <cstdint>

class NativeByteBuffer;
class ConnectionsManager;
class Datacenter;

class NetworkMessage{
public:
    NetworkMessage();
    ~NetworkMessage();
    void serializeToBuffer(NativeByteBuffer *buffer);

private:
    NativeByteBuffer *outgoingBody;
    int32_t cmdId;
    int32_t messageSeq;
    uint16_t packLength;

    friend class ConnectionsManager;
    friend class Datacenter;
};

#endif //ANDROIDAPP_NETWORKMESSAGE_H
