//
// Created by 朱乾 on 17/2/28.
//

#include <jni.h>

int registerNativeFunctions(JavaVM *vm, JNIEnv *env);

jint JNI_OnLoad(JavaVM *vm, void *reserved){
    JNIEnv *env = 0;
    if((*vm)->GetEnv(vm, (void **)&env, JNI_VERSION_1_6) != JNI_OK){
        return -1;
    }

    if(registerNativeFunctions(vm, env) != JNI_TRUE){
        return -1;
    }
    return JNI_VERSION_1_6;
}

