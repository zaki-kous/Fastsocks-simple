package com.me.fastsocks.tcp;

/**
 * 负责底层socket数据包跟应用层的分发
 *
 * 已经切换到主线程
 */
public interface PacketDispatcherDelegate {

    // 新消息
    void msgRecv(int cmd, byte[] datas);

    //连接状态改变
    void onConnectionStateChanged(int status);

    //底层库线程执行时会回调该方法
    void onUpdate();
}
