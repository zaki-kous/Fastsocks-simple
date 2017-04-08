package com.me.fastsocks.service;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.support.annotation.Nullable;
import com.me.fastsocks.tcp.ConnectionsManager;
import com.me.fastsocks.utils.LogWrapper;

/***
 * tcp启动的服务
 * <p/>
 * 简单实现了自启动
 */
public class CoreService extends Service {

    @Override
    public void onCreate() {
        LogWrapper.d("==========CoreService onCreate=========");
        super.onCreate();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        ConnectionsManager.getInstance().startInit(this, 10000000L);
        return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public void onDestroy() {
        //CoreService已经杀不死了，退出应该在别处调用
        LogWrapper.d("=========coreService onDestroy==============");
        ConnectionsManager.getInstance().logout(this);
        //重启CoreService
        Intent intent = new Intent("com.me.core.restart");
        sendBroadcast(intent);
    }

    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

}
