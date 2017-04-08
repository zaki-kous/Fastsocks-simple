//
// Created by zaki on 17/3/26.
//

#ifndef FASTSOCKS_ANDROID_JNI_H
#define FASTSOCKS_ANDROID_JNI_H

#include <jni.h>
#include <string>

jvalue JNI_CallMethodByName(JNIEnv* _env, jobject obj, const char* _name, const char* descriptor, ...);
jvalue JNI_CallStaticMethodByName(JNIEnv* _env, jclass clazz, const char* _name, const char* descriptor, ...);
jvalue JNI_CallStaticMethodByName(JNIEnv* _env, const char* _class_name, const char* _name, const char* descriptor, ...);
jvalue JNI_GetStaticField(JNIEnv* _env, jclass clazz, const char* _name, const char* sig);
jvalue JNI_GetField(JNIEnv* _env, jobject obj, const char* _name, const char* sig);

jstring JNI_Chars2Jstring(JNIEnv* _env, const char* pat);
void JNI_FreeJstring(JNIEnv* _env, jstring str);

#endif //FASTSOCKS_ANDROID_JNI_H
