//
// Created by 朱乾 on 17/3/1.
//

#include <stdlib.h>
#include "NativeByteBuffer.h"
#include "ConnectionsManager.h"
#include "FileLog.h"
#include "BufferStorage.h"

NativeByteBuffer::NativeByteBuffer(uint32_t size) {
#ifdef ANDROID
    if (jclass_ByteBuffer != nullptr) {
        JNIEnv *env;
        if(javaVM->GetEnv((void **)&env, JNI_VERSION_1_6) != JNI_OK){
            DEBUG_E("can't GetEnv ...");
            exit(1);
        }
        javaByteBuffer = env->CallStaticObjectMethod(jclass_ByteBuffer, jclass_ByteBuffer_allocateDirect, size);
        if(javaByteBuffer == NULL){
            DEBUG_E("can't call ByteBuffer allocByte method.");
            exit(1);
        }
        jobject globalRef = env->NewGlobalRef(javaByteBuffer);
        env->DeleteLocalRef(javaByteBuffer);
        javaByteBuffer = globalRef;

        buffer = (uint8_t *) env->GetDirectBufferAddress(javaByteBuffer);
        bufferOwner = false;
    } else {
#endif
        buffer = new uint8_t[size];
        bufferOwner = true;
#ifdef ANDROID
    }
#endif
    _limit = _capacity = size;
}

NativeByteBuffer::NativeByteBuffer(bool calculate) {
    calculateSizeOnly = calculate;
}


NativeByteBuffer::~NativeByteBuffer() {
#ifdef ANDROID
    DEBUG_D("delete buffer %p, javaByteBuffer == null ? %d. size : %d.", this, (javaByteBuffer == nullptr), _capacity);
    if(javaByteBuffer != nullptr){
        JNIEnv *env;
        if(javaVM->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK){
            DEBUG_E("can't get env...");
            exit(1);
        }
        env->DeleteGlobalRef(javaByteBuffer);
        javaByteBuffer = nullptr;
    }
#endif
    if(bufferOwner && buffer != nullptr){
        delete[] buffer;
        buffer = nullptr;
    }
}

uint32_t NativeByteBuffer::position() {
    return _position;
}

void NativeByteBuffer::position(uint32_t position) {
    if(position > _limit){
        return;
    }
    _position = position;
}

uint32_t NativeByteBuffer::capacity() {
    return _capacity;
}

uint32_t NativeByteBuffer::limit() {
    return _limit;
}

uint32_t NativeByteBuffer::remaining() {
    return _limit - _position;
}

void NativeByteBuffer::clearCapacity() {
    _capacity = 0;
}

void NativeByteBuffer::limit(uint32_t limit) {
    if(limit > _capacity){
        return;
    }
    if(_position > limit){
        _position = limit;
    }
    _limit = limit;
}

void NativeByteBuffer::flip() {
    _limit = _position;
    _position = 0;
}


void NativeByteBuffer::clear() {
    _position = 0;
    _limit = _capacity;
}

uint8_t* NativeByteBuffer::bytes() {
    return buffer;
}

void NativeByteBuffer::rewind() {
    _position = 0;
}

void NativeByteBuffer::compact() {
    if(_position == _limit){
        return;
    }

    memmove(buffer, buffer + _position, sizeof(uint8_t) * (_limit - _position));
    _position = (_limit - _position);
    _limit = _capacity;
}

bool NativeByteBuffer::hasRemaining() {
    return _position < _limit;
}

void NativeByteBuffer::skip(uint32_t length) {
    if(!calculateSizeOnly){
        if(_position + length > _limit){
            return;
        }
        _position += length;
    } else{
        _capacity += length;
    }
}

void NativeByteBuffer::writeBytesInternal(uint8_t* bytes, uint32_t offset, uint32_t length) {
    memcpy(buffer + _position, bytes + offset, sizeof(uint8_t) * length);
    _position += length;
}

void NativeByteBuffer::writeByte(uint8_t byte, bool *error){
    if(!calculateSizeOnly){
        if(_position + 1 > _limit){
            if(error != nullptr){
                *error = true;
                DEBUG_E("write byte fail.");
            }
            return;
        }
        buffer[_position++] = byte;
    }else {
        _capacity += 1;
    }
}

void NativeByteBuffer::writeByte(uint8_t byte){
    writeByte(byte, nullptr);
}

void NativeByteBuffer::writeInt16(int16_t x, bool *error){
    if(!calculateSizeOnly){
        if(_position + 2 > _limit){
            if(error != nullptr){
                DEBUG_E("write int 16 fail.");
                *error = true;
            }
            return;
        }
        buffer[_position++] = (uint8_t) (x >> 8);
        buffer[_position++] = (uint8_t) x;
    } else {
        _position += 2;
    }
}

void NativeByteBuffer::writeInt16(int16_t x){
    writeInt16(x, nullptr);
}

void NativeByteBuffer::writeInt32(int32_t x, bool *error){
    if(!calculateSizeOnly){
        if(_position + 4 > _limit){
            if(error != nullptr){
                DEBUG_E("write int 32 fail value : %d", x);
                *error = true;
            }
            return;
        }
        buffer[_position++] = (uint8_t) (x >> 24);
        buffer[_position++] = (uint8_t) (x >> 16);
        buffer[_position++] = (uint8_t) (x >> 8);
        buffer[_position++] = (uint8_t) x;
    } else {
        _capacity += 4;
    }
}

void NativeByteBuffer::writeInt32(int32_t x){
    writeInt32(x, nullptr);
}

void NativeByteBuffer::writeInt64(int64_t x, bool *error){
    if(!calculateSizeOnly){
        if(_position + 8 > _limit){
            if(error != nullptr){
                DEBUG_E("write int 64 fail.");
                *error = true;
            }
            return;
        }
        uint32_t temPosition = _position;
        buffer[_position++] = (uint8_t) (x >> 56);
        buffer[_position++] = (uint8_t) (x >> 48);
        buffer[_position++] = (uint8_t) (x >> 40);
        buffer[_position++] = (uint8_t) (x >> 32);
        buffer[_position++] = (uint8_t) (x >> 24);
        buffer[_position++] = (uint8_t) (x >> 16);
        buffer[_position++] = (uint8_t) (x >> 8);
        buffer[_position++] = (uint8_t) x;
    } else {
        _capacity += 8;
    }
}

void NativeByteBuffer::writeInt64(int64_t x){
    writeInt64(x, nullptr);
}

void NativeByteBuffer::writeDouble(double d, bool *error){
    int64_t value;
    memcpy(&value, &d, sizeof(int64_t));
    writeInt64(value, error);
}

void NativeByteBuffer::writeDouble(double d){
    writeDouble(d, nullptr);
}

void NativeByteBuffer::writeBool(bool value, bool *error){
    if(!calculateSizeOnly){
        if(value){
            writeByte(1, error);
        }else{
            writeByte(0, error);
        }
    } else {
        _capacity += 1;
    }
}

void NativeByteBuffer::writeBool(bool value){
    writeBool(value, nullptr);
}

void NativeByteBuffer::writeBytes(NativeByteBuffer *buffer) {
    writeBytes(buffer, nullptr);
}


void NativeByteBuffer::writeBytes(NativeByteBuffer *buffer, bool *error) {
    uint32_t length = buffer->limit() - buffer->position();
    if(length <= 0){
        return;
    }

    if(!calculateSizeOnly){
        if(_position + length > _limit){
            if(error != nullptr){
                *error = true;
                DEBUG_E("writeBytes error...");
                return;
            }
            return;
        }
        writeBytesInternal(buffer->bytes(), buffer->position(), length);
        buffer->position(buffer->limit());
    } else {
        _capacity += length;
    }
}

void NativeByteBuffer::writeBytes(uint8_t *buffer, uint32_t length){
    writeBytes(buffer, 0, length);
}
void NativeByteBuffer::writeBytes(uint8_t *buffer, uint32_t offset, uint32_t length){
    writeBytes(buffer, offset, length, nullptr);
}
void NativeByteBuffer::writeBytes(uint8_t *buffer, uint32_t offset, uint32_t length, bool *error){
    if(!calculateSizeOnly){
        if(_position + length > _limit){
            if(error != nullptr){
                *error = true;
            }
            DEBUG_E("writeBytes error...");
            return;
        }
        writeBytesInternal(buffer, offset, length);
    } else {
        _capacity += length;
    }
}

uint8_t NativeByteBuffer::readByte(bool *error){
    if(_position + 1 > _limit){
        if(error != nullptr){
            DEBUG_E("read byte fail.");
            *error = true;
        }
        return 0;
    }
    return buffer[_position++];
}

uint16_t NativeByteBuffer::readInt16(bool *error){
    if(_position + 2 > _limit){
        if(error != nullptr){
            DEBUG_E("read int 16 fail.");
            *error = true;
        }
        return 0;
    }
    uint16_t result = (uint16_t) ((buffer[_position] & 0xff) << 8) | (buffer[_position + 1] & 0xff);
    _position += 2;
    return result;
}

uint32_t NativeByteBuffer::readUint32(bool *error){
    if(_position + 4 > _limit){
        if(error != nullptr){
            DEBUG_E("read uint 32 fail.");
            *error = true;
        }
        return 0;
    }
    uint32_t result = (uint32_t) ((buffer[_position] & 0xff) << 24) |
            ((buffer[_position + 1] & 0xff) << 16) |
            ((buffer[_position + 2] & 0xff) << 8) |
            ((buffer[_position + 3] & 0xff));
    _position += 4;
    return result;
}

uint64_t NativeByteBuffer::readUint64(bool *error){
    if(_position + 8 > _limit){
        if(error != nullptr){
            DEBUG_E("read uint 32 fail.");
            *error = true;
        }
        return 0;
    }
    int64_t result = ((buffer[_position] & 0xff) << 56) |
                      ((buffer[_position + 1] & 0xff) << 48) |
                      ((buffer[_position + 2] & 0xff) << 40) |
                      ((buffer[_position + 3] & 0xff) << 32) |
                      ((buffer[_position + 4] & 0xff) << 24) |
                      ((buffer[_position + 5] & 0xff) << 16) |
                      ((buffer[_position + 6] & 0xff) << 8) |
                      ((buffer[_position + 7] & 0xff));
    _position += 8;
    return (uint64_t) result;
}
int32_t NativeByteBuffer::readInt32(bool *error){
    return (int32_t) readUint32(error);
}
int64_t NativeByteBuffer::readInt64(bool *error){
    return (int64_t) readUint64(error);
}
bool NativeByteBuffer::readBool(bool *error){
    if(_position + 1 > _limit){
        if(error != nullptr){
            DEBUG_E("read bool fail.");
            *error = true;
        }
        return false;
    }
    _position += 1;
    if(buffer[_position] != 0){
        return true;
    } else {
        return false;
    }
}

double NativeByteBuffer::readDouble(bool *error){
    double doubleValue;
    int64_t intValue = readInt64(error);
    memcpy(&doubleValue, &intValue, sizeof(double));
    return doubleValue;
}

void NativeByteBuffer::reuse() {
    BufferStorage::getInstance().reuseFreeBuffer(this);
}

#ifdef ANDROID
jobject NativeByteBuffer::getJavaByteBuffer() {
    if(javaByteBuffer == nullptr && javaVM != nullptr){
        JNIEnv *env;
        if(javaVM->GetEnv((void **)&env, JNI_VERSION_1_6) != JNI_OK){
            DEBUG_E("can't GetEnv ...");
            exit(1);
        }
        javaByteBuffer = env->CallStaticObjectMethod(jclass_ByteBuffer, jclass_ByteBuffer_allocateDirect, _capacity);
        if(javaByteBuffer == NULL){
            DEBUG_E("can't call ByteBuffer allocByte method.");
            exit(1);
        }
        jobject globalRef = env->NewGlobalRef(javaByteBuffer);
        env->DeleteLocalRef(globalRef);
        javaByteBuffer = globalRef;
    }
    return javaByteBuffer;
}
#endif
