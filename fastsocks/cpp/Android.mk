LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE   := TcpNet

LOCAL_SRC_FILES     += \
./TcpConnection.cpp \
./TcpManager.cpp \
./TcpServer.cpp \
./ByteArray.cpp \
./NativeByteBuffer.cpp \
./BufferStorage.cpp \
./EventsDispatcher.cpp \
./Timer.cpp \
./ConnectionsManager.cpp \
./FileLog.cpp \
./TcpNetWapper.cpp \
./jni.c \

LOCAL_CPPFLAGS := -Wall -std=c++11 -DANDROID

LOCAL_LDLIBS :=-llog

include $(BUILD_SHARED_LIBRARY)