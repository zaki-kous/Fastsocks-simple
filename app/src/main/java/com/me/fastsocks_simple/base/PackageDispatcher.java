package com.me.fastsocks_simple.base;

import com.me.fastsocks.tcp.PacketDispatcherDelegate;
import com.me.fastsocks_simple.message.MessageModel;

import java.util.ArrayList;
import java.util.List;

/**
 * Created by zaki on 17/4/8.
 */
public class PackageDispatcher implements PacketDispatcherDelegate{

    private static PackageDispatcher Instance = null;

    private List<OnRecvListener> onRecvListeners;
    public static PackageDispatcher getInstance() {
        PackageDispatcher localInstance = Instance;
        if (localInstance == null) {
            synchronized (PackageDispatcher.class) {
                localInstance = Instance;
                if (localInstance == null) {
                    Instance = localInstance = new PackageDispatcher();
                }
            }
        }
        return localInstance;
    }

    private PackageDispatcher() {
        onRecvListeners = new ArrayList<>();
    }

    @Override
    public void msgRecv(int cmd, byte[] datas) {
        //TODO 根据命令码分发到不同的监听，这里模拟"1280"就是收到新消息。
        switch (cmd) {
            case 1280:
                //TODO 解析成消息实力类，一般都要入库和缓存，这里简单处理
                synchronized (this) {
                    for (OnRecvListener onRecvListener : onRecvListeners) {
                        onRecvListener.onRecv(MessageModel.build(datas));
                    }
                }
                break;
            default:
                break;
        }
    }

    @Override
    public void onConnectionStateChanged(int status) {
        //TODO 界面上显示连接状态
    }

    @Override
    public void onUpdate() {
        // TODO 可以在这里做一些更新数据库／刷新界面缓存之类的事情
    }

    //添加监听
    public synchronized void addReceiverListener(OnRecvListener onRecvListener) {
        if (onRecvListener != null) {
            onRecvListeners.add(onRecvListener);
        }
    }

    //移除监听
    public synchronized void removeReceiverListener(OnRecvListener onRecvListener) {
        if (onRecvListener != null) {
            onRecvListeners.remove(onRecvListener);
        }
    }
}
