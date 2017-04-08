//
// Created by zaki on 17/3/26.
//

#include <arpa/inet.h>
#include "FormatTools.h"

namespace FormatTools{
    uint64_t htonll(uint64_t host) {
        unsigned long temp_low, temp_high;
        temp_low = htonl((long)host);
        temp_high = htonl((long)(host >> 32));

        host &= 0;
        host |= temp_low;
        host <<= 32;
        host |= temp_high;
        return host;
    }

    int64_t ntohll(int64_t host) {
        uint32_t temp_low, temp_high;
        temp_low = htonl((uint32_t) host);
        temp_high = htonl((uint32_t) (host >> 32));

        host &= 0;
        host |= temp_low;
        host <<= 32;
        host |= temp_high;
        return host;
    }
}