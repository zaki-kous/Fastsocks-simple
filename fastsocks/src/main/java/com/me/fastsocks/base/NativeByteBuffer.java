package com.me.fastsocks.base;

import com.me.fastsocks.utils.LogWrapper;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

/**
 * c层与java层的内存转换
 *
 * Created by zaki on 17/3/26.
 */
public class NativeByteBuffer {

    public int address;
    public ByteBuffer buffer;
    private boolean reuse;

    public static int wrap(byte[] datas){
        if(datas != null){
            NativeByteBuffer nativeByteBuffer = new NativeByteBuffer(datas.length);
            nativeByteBuffer.buffer.put(datas);
            return nativeByteBuffer.address;
        }
        return 0;
    }

    public static byte[] with(int address){
        if(address != 0){
            NativeByteBuffer buffer = new NativeByteBuffer();
            buffer.address = address;
            int limit = native_limit(address);
            int position = native_position(address);
            int remaing = limit - position;
            if(remaing <= 0){
                //无包体
                return new byte[]{};
            }
            buffer.buffer = native_getJavaByteBuffer(address);
            buffer.buffer.limit(limit);
            buffer.buffer.position(position);
            buffer.reuse = false;

            byte[] data = new byte[remaing];
            buffer.buffer.get(data, 0, remaing);
            return data;
        }
        return null;
    }

    private NativeByteBuffer(){}


    public NativeByteBuffer(int size){
        if(size >= 0){
            address = native_getBuffer(size);
            buffer = native_getJavaByteBuffer(address);
            LogWrapper.d("native address : " + Integer.toHexString(address));
            buffer.position(0);
            buffer.limit(size);
            buffer.order(ByteOrder.BIG_ENDIAN);
        } else {
            throw new IllegalArgumentException("native buffer size < 0 ?");
        }
    }

    public void resue(){
        if(address != 0){
            reuse = true;
            native_reuse(address);
        }
    }

    public static native int native_getBuffer(int size);
    public static native ByteBuffer native_getJavaByteBuffer(int address);
    public static native int native_limit(int address);
    public static native int native_position(int address);
    public static native void native_reuse(int address);
}
