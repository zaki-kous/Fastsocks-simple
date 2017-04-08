package com.me.fastsocks.tcp.listener;

import com.me.fastsocks.tcp.ConnectionsManager;

/**
 * 上层应用的发送监听
 *
 * Created by zaki on 17/3/26.
 */
public interface OnSendRequestListener {

    void onSendComplete(byte[] cmpleteBuffer, ConnectionsManager.ERROR error);

    void onSendCancel();
}
