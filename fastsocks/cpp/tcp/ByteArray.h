//
// Created by 朱乾 on 17/3/2.
//

#ifndef TCPWORK_BYTEARRAY_H
#define TCPWORK_BYTEARRAY_H

#include <stdint.h>

class ByteArray {

public:
    ByteArray();
    ByteArray(uint32_t len);
    ByteArray(ByteArray *array);
    ByteArray(uint8_t *buffer, uint32_t len);

    ~ByteArray();
    void alloc(uint32_t len);


    uint32_t length;
    uint8_t *buffer;
};



#endif //TCPWORK_BYTEARRAY_H
