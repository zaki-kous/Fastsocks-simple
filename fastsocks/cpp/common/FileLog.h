//
// Created by 朱乾 on 17/2/27.
//

#ifndef TCPWORK_FILELOG_H
#define TCPWORK_FILELOG_H

class FileLog{
public:
    static void e(const char *message, ...) __attribute__((format (printf, 1, 2)));
    static void w(const char *message, ...) __attribute__((format (printf, 1, 2)));
    static void d(const char *message, ...) __attribute__((format (printf, 1, 2)));

};

#ifdef DEBUG
#define DEBUG_E FileLog::e
#define DEBUG_W FileLog::w
#define DEBUG_D FileLog::d
#else
#define DEBUG_E
#define DEBUG_W
#define DEBUG_D
#endif

#endif //TCPWORK_FILELOG_H
