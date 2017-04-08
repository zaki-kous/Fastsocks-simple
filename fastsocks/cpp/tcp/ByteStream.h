//
// Created by 朱乾 on 17/3/13.
//

#ifndef TCPLIB_BYTESTREAM_H
#define TCPLIB_BYTESTREAM_H

#include <stdint.h>
#include <vector>

class NativeByteBuffer;

class ByteStream {
public:
    ByteStream();
    ~ByteStream();
    void append(NativeByteBuffer *buffer);
    bool hasData();
    void get(NativeByteBuffer *dst);
    void discard(uint32_t count);
    void clear();

private:
    std::vector<NativeByteBuffer *> buffersQueue;
};

#endif //TCPLIB_BYTESTREAM_H
