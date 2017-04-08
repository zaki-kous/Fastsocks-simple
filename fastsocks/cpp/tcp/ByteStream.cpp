//
// Created by 朱乾 on 17/3/13.
//

#include "ByteStream.h"
#include "NativeByteBuffer.h"

ByteStream::ByteStream() {

}

ByteStream::~ByteStream() {

}


void ByteStream::append(NativeByteBuffer *buffer) {
    if(buffer == nullptr){
        return;
    }
    buffersQueue.push_back(buffer);
}

bool ByteStream::hasData() {
    ssize_t size = buffersQueue.size();
    for(uint32_t i = 0; i < size; i++){
        if(buffersQueue[i]->hasRemaining()){
            return true;
        }
    }
    return false;
}

void ByteStream::get(NativeByteBuffer *dst) {
    if(dst == nullptr){
        return;
    }
    ssize_t size = buffersQueue.size();
    for(int i = 0; i < size; i++){
        NativeByteBuffer *buffer = buffersQueue[i];
        if(buffer->remaining() > dst->remaining()){
            dst->writeBytes(buffer->bytes(), buffer->position(), dst->remaining());
            break;
        }
        dst->writeBytes(buffer->bytes(), buffer->position(), buffer->remaining());
        if(!dst->hasRemaining()){
            break;
        }
    }
}

void ByteStream::discard(uint32_t count) {
    uint32_t remaining = 0;
    NativeByteBuffer *buffer;
    while (count > 0){
        buffer = buffersQueue[0];
        remaining = buffer->remaining();
        if(count <remaining){
            buffer->position(buffer->position() + count);
            break;
        }
        buffer->reuse();
        buffersQueue.erase(buffersQueue.begin());
        count -= remaining;
    }
}

void ByteStream::clear() {
    if(buffersQueue.empty()){
        return;
    }
    ssize_t size = buffersQueue.size();
    for(uint32_t i = 0; i<size; i++){
        NativeByteBuffer *buffer = buffersQueue[i];
        buffer->reuse();
    }

    buffersQueue.clear();
}