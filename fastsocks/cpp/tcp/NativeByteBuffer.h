//
// Created by 朱乾 on 17/3/1.
//

#ifndef TCPWORK_NATIVEBYTEBUFFER_H
#define TCPWORK_NATIVEBYTEBUFFER_H

#include <stdint.h>
#include <string>

class ByteArray;

#ifdef ANDROID
#include <jni.h>
#endif

class NativeByteBuffer{
public:
    NativeByteBuffer(uint32_t size);
    NativeByteBuffer(bool calculate);
    ~NativeByteBuffer();

    uint32_t position();
    void position(uint32_t position);
    uint32_t limit();
    void limit(uint32_t limit);
    uint32_t capacity();
    uint32_t remaining();
    bool hasRemaining();
    void rewind();
    void compact();
    void flip();
    void clear();
    void skip(uint32_t length);
    void clearCapacity();
    uint8_t *bytes();


    void writeByte(uint8_t byte, bool *error);
    void writeInt16(int16_t x, bool *error);
    void writeInt32(int32_t x, bool *error);
    void writeInt64(int64_t x, bool *error);
    void writeDouble(double d, bool *error);
    void writeBool(bool value, bool *error);
    void writeByte(uint8_t byte);
    void writeInt16(int16_t x);
    void writeInt32(int32_t x);
    void writeInt64(int64_t x);
    void writeDouble(double d);
    void writeBool(bool value);
    void writeBytes(NativeByteBuffer *buffer);
    void writeBytes(NativeByteBuffer *buffer, bool *error);
    void writeBytes(uint8_t *buffer, uint32_t length);
    void writeBytes(uint8_t *buffer, uint32_t offset, uint32_t length);
    void writeBytes(uint8_t *buffer, uint32_t offset, uint32_t length, bool *error);

    uint8_t readByte(bool *error);
    uint16_t readInt16(bool *error);
    uint32_t readUint32(bool *error);
    uint64_t readUint64(bool *error);
    int32_t readInt32(bool *error);
    int64_t readInt64(bool *error);
    bool readBool(bool *error);
    double readDouble(bool *error);

    void reuse();

#ifdef ANDROID
    jobject getJavaByteBuffer();
#endif


private:
    void writeBytesInternal(uint8_t *bytes, uint32_t offset, uint32_t length);

    uint8_t *buffer;
    uint32_t _position = 0;
    uint32_t _limit = 0;
    uint32_t _capacity = 0;
    bool calculateSizeOnly = false;
    bool bufferOwner = true;

#ifdef ANDROID
    jobject javaByteBuffer = nullptr;
#endif
};


#endif //TCPWORK_NATIVEBYTEBUFFER_H
