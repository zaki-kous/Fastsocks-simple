package com.me.fastsocks.tcp.receiver;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import com.me.fastsocks.tcp.ConnectionsManager;
import com.me.fastsocks.utils.LogWrapper;

/**
 * 屏幕亮起和熄灭广播接受者
 *
 * Created by ZaKi on 2016/6/30.
 */
public class ScreenStatusReceiver extends BroadcastReceiver{
    @Override
    public void onReceive(Context context, Intent intent) {
        if(Intent.ACTION_SCREEN_ON.equals(intent.getAction())){
            //屏幕亮起，通知重连检查
            LogWrapper.d("receive :"+Intent.ACTION_SCREEN_ON+",check sockets....");
//            ReconnManager.getInstance().notifyReconnect();
            ConnectionsManager.getInstance().resumeNetwork(false);
        }else if(Intent.ACTION_SCREEN_OFF.equals(intent.getAction())){
            //屏幕亮起，通知重连检查
            LogWrapper.d("receive :"+Intent.ACTION_SCREEN_OFF+",please do something...");
            ConnectionsManager.getInstance().pauseNetwork();
        }
    }
}
