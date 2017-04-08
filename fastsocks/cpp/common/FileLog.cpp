//
// Created by 朱乾 on 17/2/27.
//
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include "FileLog.h"

#ifdef ANDROID
#include <android/log.h>
#endif
static const char *TAG = "fastsocks";

void FileLog::w(const char *message, ...) {
    va_list argptr;
    va_start(argptr, message);
#ifdef ANDROID
    __android_log_vprint(ANDROID_LOG_WARN, TAG, message, argptr);
#else
#endif
    va_end(argptr);
}

void FileLog::e(const char *message, ...) {
    va_list argptr;
    va_start(argptr, message);
#ifdef ANDROID
    __android_log_vprint(ANDROID_LOG_ERROR, TAG, message, argptr);
#else
#endif
    va_end(argptr);
}

void FileLog::d(const char *message, ...) {va_list argptr;
    va_start(argptr, message);
#ifdef ANDROID
    __android_log_vprint(ANDROID_LOG_DEBUG, TAG, message, argptr);
#else
#endif
    va_end(argptr);
}



