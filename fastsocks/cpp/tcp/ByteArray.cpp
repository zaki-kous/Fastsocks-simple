//
// Created by 朱乾 on 17/3/2.
//
#include <stdlib.h>
#include "ByteArray.h"
#include "FileLog.h"

ByteArray::ByteArray() {
    buffer = nullptr;
    length = 0;
}

ByteArray::ByteArray(uint32_t len) {
    buffer = new uint8_t[len];

    if(buffer == nullptr){
        DEBUG_E("can't alloc buffer %u", len);
        exit(1);
    }
    length = len;
}

ByteArray::ByteArray(ByteArray *array) {
    buffer = new uint8_t[array->length];
    if(buffer == nullptr){
        DEBUG_E("can't alloc buffer %u", array->length);
        exit(1);
    }
    length = array->length;

    memcpy(buffer, array->buffer, length);
}

ByteArray::ByteArray(uint8_t *srcBuffer, uint32_t len) {
    buffer = new uint8_t[len];
    if(buffer == nullptr){
        DEBUG_E("can't alloc buffer %u", len);
        exit(1);
    }
    length = len;

    memcpy(buffer, srcBuffer, length);
}

void ByteArray::alloc(uint32_t len) {
    if(buffer != nullptr){
        delete buffer;
        buffer = nullptr;
    }

    buffer = new uint8_t[len];
    if(buffer == nullptr){
        DEBUG_E("can't alloc buffer %u", len);
        exit(1);
    }
    length = len;
}

ByteArray::~ByteArray() {
    if(buffer != nullptr){
        delete buffer;
        buffer = nullptr;
    }
    length = 0;
}