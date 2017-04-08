//
// Created by 朱乾 on 17/3/5.
//

#include "BufferStorage.h"
#include "NativeByteBuffer.h"
#include "FileLog.h"

BufferStorage& BufferStorage::getInstance() {
    static BufferStorage instnce(true);
    return instnce;
}

BufferStorage::BufferStorage(bool threadSafe) {
    isThreadSafe = threadSafe;
    if(isThreadSafe){
        pthread_mutex_init(&mutex, NULL);
    }

    for(int i = 0; i< 5; i++){
        freeBuffers8.push_back(new NativeByteBuffer((uint32_t) 8));
    }

    for(int i = 0; i< 5; i++){
        freeBuffers128.push_back(new NativeByteBuffer((uint32_t) 128));
    }
}

NativeByteBuffer* BufferStorage::getBuffer(uint32_t size) {

    std::vector<NativeByteBuffer*> *bufferGetFrom = nullptr;
    NativeByteBuffer *buffer = nullptr;
    uint32_t byteCount = 0;
    if (size <= 8) {
        bufferGetFrom = &freeBuffers8;
        byteCount = 8;
    } else if (size <= 128) {
        bufferGetFrom = &freeBuffers128;
        byteCount = 128;
    } else if (size <= 1024 + 200) {
        bufferGetFrom = &freeBuffers1024;
        byteCount = 1024 + 200;
    } else if (size <= 4 * 1024 + 200) {
        bufferGetFrom = &freeBuffers4k;
        byteCount = 4 * 1024 + 200;
    } else if (size <= 16 * 1024 + 200) {
        bufferGetFrom =&freeBuffers16k;
        byteCount = 16 * 1024 + 200;
    } else if (size <= 32 * 1024 + 200) {
        bufferGetFrom = &freeBuffers32k;
        byteCount = 32 * 1024 + 200;
    } else if (size <= 128 * 1024 + 200) {
        bufferGetFrom = &freeBuffers128k;
        byteCount = 128 * 1024 + 200;
    } else {
        buffer = new NativeByteBuffer(size);
        DEBUG_D("new NativeByteBuffer size : %d.", size);
    }

    if(bufferGetFrom != nullptr) {
        if(isThreadSafe){
            pthread_mutex_lock(&mutex);
        }

        if(bufferGetFrom->size() > 0) {
            buffer = (*bufferGetFrom) [0];
            bufferGetFrom->erase(bufferGetFrom->begin());
        }
        if(isThreadSafe) {
            pthread_mutex_unlock(&mutex);
        }

        if (buffer == nullptr) {
            buffer = new NativeByteBuffer(byteCount);
            DEBUG_D("new NativeByteBuffer from freeBufers size : %d.", byteCount);
        }
    }

    buffer->limit(size);
    buffer->rewind();
    return buffer;
}

void BufferStorage::reuseFreeBuffer(NativeByteBuffer *buffer) {
    if(buffer == nullptr){
        return;
    }
    uint32_t capacity = buffer->capacity();
    uint32_t maxSize = 10;
    std::vector<NativeByteBuffer*> *bufferFreeInto = nullptr;
    if (capacity <= 8) {
        bufferFreeInto = &freeBuffers8;
        maxSize = 80;
    } else if (capacity <= 128) {
        bufferFreeInto = &freeBuffers128;
        maxSize = 80;
    } else if (capacity <= 1024 + 200) {
        bufferFreeInto = &freeBuffers1024;
    } else if (capacity <= 4 * 1024 + 200) {
        bufferFreeInto = &freeBuffers4k;
    } else if (capacity <= 16 * 1024 + 200) {
        bufferFreeInto =&freeBuffers16k;
    } else if (capacity <= 32 * 1024 + 200) {
        bufferFreeInto = &freeBuffers32k;
    } else if (capacity <= 128 * 1024 + 200) {
        bufferFreeInto = &freeBuffers128k;
    }

    if(bufferFreeInto != nullptr) {
        if(isThreadSafe) {
            pthread_mutex_lock(&mutex);
        }

        if(bufferFreeInto->size() < maxSize) {
            bufferFreeInto->push_back(buffer);
        } else {
            delete buffer;
            DEBUG_D("bufferFreeInto(%p) size is largest maxSize : %d currentSize : %d.", bufferFreeInto, maxSize, bufferFreeInto->size());
        }

        if(isThreadSafe){
            pthread_mutex_unlock(&mutex);
        }
    } else {
        delete buffer;
        DEBUG_D("delete buffer(%p) don't into bufferFrees capcity : %d.", buffer, buffer->capacity());
    }
}