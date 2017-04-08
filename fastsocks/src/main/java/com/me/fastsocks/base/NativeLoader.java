package com.me.fastsocks.base;

/**
 * Created by zaki on 17/3/23.
 */
public class NativeLoader {

    static {
        System.loadLibrary("fastsocks");
    }

    public static native void init(String crashPath, boolean enable);
}
