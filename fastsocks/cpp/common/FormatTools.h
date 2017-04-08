//
// Created by zaki on 17/3/25.
//

#ifndef TCPLIB_STRINGTOOLS_H
#define TCPLIB_STRINGTOOLS_H

#include <string>
#include <sstream>

//字符串工具类
namespace FormatTools{

    template<typename T> std::string toString(T t){
        std::ostringstream os;
        os<<t;
        return std::string(os.str());
    }

    template<typename T> T fromString(std::string value){
        T t;
        std::istringstream is(value);
        is>>t;
        return t;
    }

    uint64_t htonll(uint64_t host);

    int64_t ntohll(int64_t host);
}

#endif //TCPLIB_STRINGTOOLS_H
