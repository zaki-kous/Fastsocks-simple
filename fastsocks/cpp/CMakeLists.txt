cmake_minimum_required(VERSION 3.4.1)

set(breakpad_DIR ${CMAKE_SOURCE_DIR}/breakpad )

add_library(lib_breakpad STATIC IMPORTED )

set_target_properties(lib_breakpad PROPERTIES IMPORTED_LOCATION
    ${breakpad_DIR}/library/${ANDROID_ABI}/libbreakpad.a )

set(crypto_DIR ${CMAKE_SOURCE_DIR}/boringssl )

add_library(lib_crypto STATIC IMPORTED )

set_target_properties(lib_crypto PROPERTIES IMPORTED_LOCATION
        ${crypto_DIR}/lib/${ANDROID_ABI}/libcrypto_armeabi-v7a.a )

add_library( TcpNet

             STATIC

             tcp/TcpSocket.cpp
             tcp/TcpConnection.cpp
             tcp/ByteArray.cpp
             tcp/ByteStream.cpp
             tcp/NativeByteBuffer.cpp
             tcp/BufferStorage.cpp
             tcp/EventsDispatcher.cpp
             tcp/Timer.cpp
             tcp/Datacenter.cpp
             tcp/Request.cpp
             tcp/NetworkMessage.cpp
             tcp/ConnectionsManager.cpp
             tcp/TcpNetWapper.cpp )

add_library( Common

             STATIC

             common/FormatTools.cpp
             common/android_jni.cpp
             common/FileLog.cpp )

include_directories(${CMAKE_SOURCE_DIR}/common/
                    ${breakpad_DIR}/
                    ${breakpad_DIR}/common/android/include
                    ${crypto_DIR}/include )

add_library( fastsocks

             SHARED

             jni.c
             NativeLoader.cpp )

find_library( log-lib

              log )

find_library( z-lib

              z )

target_link_libraries( TcpNet
                       Common )
target_link_libraries( fastsocks

                       ${log-lib}
                       ${z-lib}

                       TcpNet

                       lib_breakpad
                       lib_crypto
                       )