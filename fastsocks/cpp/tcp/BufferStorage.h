//
// Created by 朱乾 on 17/3/5.
//

#ifndef TCPWORK_BUFFERSTORAGE_H
#define TCPWORK_BUFFERSTORAGE_H

#include <vector>
#include <stdint.h>
#include <pthread.h>

class NativeByteBuffer;
class BufferStorage{
public:
    static BufferStorage& getInstance();
    BufferStorage(bool threadSafe);
    NativeByteBuffer *getBuffer(uint32_t size);
    void reuseFreeBuffer(NativeByteBuffer *buffer);

private:
    std::vector<NativeByteBuffer *> freeBuffers8;
    std::vector<NativeByteBuffer *> freeBuffers128;
    std::vector<NativeByteBuffer *> freeBuffers1024;
    std::vector<NativeByteBuffer *> freeBuffers4k;
    std::vector<NativeByteBuffer *> freeBuffers16k;
    std::vector<NativeByteBuffer *> freeBuffers32k;
    std::vector<NativeByteBuffer *> freeBuffers128k;

    bool isThreadSafe;

    pthread_mutex_t mutex;
};
#endif //TCPWORK_BUFFERSTORAGE_H
