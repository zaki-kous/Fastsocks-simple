package com.me.fastsocks.tcp.listener;

/**
 * Created by zhuqian on 17/3/26.
 */

public interface OnNativeSendListener {
    void onSendComplete(int address, int errorCode, String desc);
}
