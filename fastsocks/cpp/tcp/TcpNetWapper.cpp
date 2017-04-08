//
// Created by 朱乾 on 17/2/28.
//

#include <jni.h>
#include <stdlib.h>
#include <string>
#include "ConnectionsManager.h"
#include "BufferStorage.h"
#include "NativeByteBuffer.h"
#include "android_jni.h"

JavaVM *java;

static const char * ConnectionManagerClassPathName = "com/me/fastsocks/tcp/ConnectionsManager";

jobject jobject_ConnectionsManager;

class Delegate : public ConnectionManagerDelegate{
    void onUpdate() {
        JNI_CallMethodByName(jniEnv, jobject_ConnectionsManager, "onUpdate", "()V");
    }

    void onConnectionStateChanged(ConnetionState connetionState) {
        JNI_CallMethodByName(jniEnv, jobject_ConnectionsManager, "onConnectionStateChanged", "(I)V", connetionState);
    }

    void onRecvMessages(int32_t cmd, NativeByteBuffer *data) {
        JNI_CallMethodByName(jniEnv, jobject_ConnectionsManager, "onRecvMessages", "(II)V", cmd, (int32_t) data);
    }

    int onHandshakeConnected(NativeByteBuffer *buffer) {
        return JNI_CallMethodByName(jniEnv, jobject_ConnectionsManager, "onHandshakeConnected", "(I)I", (int32_t) buffer).i;
    }
};

void setJava(JNIEnv *env, jobject obj){
    jobject_ConnectionsManager = env->NewGlobalRef(obj);
    ConnectionsManager::useJavaVM(java);
    ConnectionsManager::getInstance().setDelegate(new Delegate());
}

void init(JNIEnv *env, jclass clazz, jlong uin){
    ConnectionsManager::getInstance().init((uint64_t) uin);
}

void setUin(JNIEnv *env, jclass clazz, jlong uin){
    ConnectionsManager::getInstance().setUin((uint64_t) uin);
}

void logout(JNIEnv *env, jclass clazz){
    ConnectionsManager::getInstance().clearUp();
}

void setNetworkAvailable(JNIEnv *env, jclass clazz, jboolean isAvailable){
    ConnectionsManager::getInstance().setNetworkAvailable(isAvailable);
}

void pauseNetwork(JNIEnv *env, jclass clazz){
    ConnectionsManager::getInstance().pauseNetwork();
}

void resumeNetwork(JNIEnv *env, jclass clazz, jboolean isPush){
    ConnectionsManager::getInstance().resumeNetwork(isPush);
}

void addSvrAddr(JNIEnv *env, jclass clazz, jstring addr, jshort port) {
    const char *addrValue = env->GetStringUTFChars(addr, NULL);

    ConnectionsManager::getInstance().applySvrAddr(std::string(addrValue), (uint16_t) port, false);

    if(addrValue != NULL){
        env->ReleaseStringUTFChars(addr, addrValue);
    }
}

void setSvrAddr(JNIEnv *env, jclass clazz, jstring addr, jshort port) {
    const char *addrValue = env->GetStringUTFChars(addr, NULL);

    ConnectionsManager::getInstance().applySvrAddr(std::string(addrValue), (uint16_t) port, true);

    if(addrValue != NULL){
        env->ReleaseStringUTFChars(addr, addrValue);
    }
}

void sendRequest(JNIEnv *env, jclass c, jobject request, jobject onSendFunc){
    int cmd = JNI_GetField(env, request, "cmd", "I").i;
    int address = JNI_GetField(env, request, "address", "I").i;
    int flags = JNI_GetField(env, request, "flags", "I").i;
    jlong timeout = JNI_GetField(env, request, "timeout", "J").j;

    if(onSendFunc != nullptr){
        onSendFunc = env->NewGlobalRef(onSendFunc);
    }

    ConnectionsManager::getInstance().senRequest((NativeByteBuffer *) address, (uint32_t) cmd, (uint32_t) flags, (uint64_t) timeout,
    [onSendFunc] (NativeByteBuffer *buffer, uint32_t code, std::string desc){
        if (onSendFunc != nullptr) {
            JNI_CallMethodByName(jniEnv, onSendFunc, "onSendComplete", "(IILjava/lang/String;)V",
                                 buffer, code, JNI_Chars2Jstring(jniEnv, desc.c_str()));
        }
    }, onSendFunc);
}

static JNINativeMethod connectionManagerMethods[] = {
        {"native_setJava", "()V", (void *) setJava},
        {"native_init", "(J)V", (void *) init},
        {"native_setUin", "(J)V", (void *) setUin},
        {"native_logout", "()V", (void *) logout},
        {"native_setNetworkAvailable", "(Z)V", (void *) setNetworkAvailable},
        {"native_pauseNetwork", "()V", (void *) pauseNetwork},
        {"native_resumeNetwork", "(Z)V", (void *) resumeNetwork},
        {"native_sendRequest", "(Lcom/me/fastsocks/tcp/ConnectionsManager$Request;Lcom/me/fastsocks/tcp/listener/OnNativeSendListener;)V", (void *) sendRequest},
        {"native_addSvrAddr", "(Ljava/lang/String;S)V", (void *) addSvrAddr},
        {"native_setSvrAddr", "(Ljava/lang/String;S)V", (void *) setSvrAddr}
};

static const char *NativeByteBufferClassPathName = "com/me/fastsocks/base/NativeByteBuffer";

jint getBuffer(JNIEnv *env, jclass c, jint length) {
    return (jint) BufferStorage::getInstance().getBuffer(length);
}

jint limit(JNIEnv *env, jclass c, jint address) {
    NativeByteBuffer *buffer = (NativeByteBuffer *) address;
    return buffer->limit();
}

jint position(JNIEnv *env, jclass c, jint address) {
    NativeByteBuffer *buffer = (NativeByteBuffer *) address;
    return buffer->position();
}

void reuse(JNIEnv *env, jclass c, jint address) {
    NativeByteBuffer *buffer = (NativeByteBuffer *) address;
    buffer->reuse();
}

jobject getJavaByteBuffer(JNIEnv *env, jclass c, jint address) {
    NativeByteBuffer *buffer = (NativeByteBuffer *) address;
    return buffer->getJavaByteBuffer();
}


static JNINativeMethod nativeByteBufferMethods[] = {
        {"native_getBuffer", "(I)I", (void *) getBuffer},
        {"native_getJavaByteBuffer", "(I)Ljava/nio/ByteBuffer;", (void *) getJavaByteBuffer},
        {"native_limit", "(I)I", (void *) limit},
        {"native_position", "(I)I", (void *) position},
        {"native_reuse", "(I)V", (void *) reuse}
};

inline int registerNativeMethods(JNIEnv *env, const char* className, JNINativeMethod *methods, int methodsCount){
    jclass clazz;
    clazz = env->FindClass(className);
    if(clazz == NULL){
        return JNI_FALSE;
    }
    if(env->RegisterNatives(clazz, methods, methodsCount) < 0){
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

extern "C" int registerNativeFunctions(JavaVM *vm, JNIEnv *env){

    java = vm;

    if(!registerNativeMethods(env, ConnectionManagerClassPathName, connectionManagerMethods, sizeof(connectionManagerMethods) /
            sizeof(JNINativeMethod))){
        return JNI_FALSE;
    }

    if(!registerNativeMethods(env, NativeByteBufferClassPathName, nativeByteBufferMethods, sizeof(nativeByteBufferMethods) /
                                                                                             sizeof(JNINativeMethod))){
        return JNI_FALSE;
    }
    return JNI_TRUE;
}
