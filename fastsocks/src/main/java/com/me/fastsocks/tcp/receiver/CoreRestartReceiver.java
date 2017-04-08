package com.me.fastsocks.tcp.receiver;

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import com.me.fastsocks.service.CoreService;
import com.me.fastsocks.tcp.ConnectionsManager;
import com.me.fastsocks.utils.StartUtils;



public class CoreRestartReceiver extends BroadcastReceiver {

    @Override
    public void onReceive(Context context, Intent intent) {
        if("com.me.core.restart".equals(intent.getAction())){
            //启动coreService
            StartUtils.startSdkService(context);
            if (android.os.Build.VERSION.SDK_INT >= 19) {
//                Calendar cal = Calendar.getInstance();
//                PendingIntent pintent = PendingIntent.getService(applicationContext, 0, new Intent(applicationContext, NotificationsService.class), 0);
//                AlarmManager alarm = (AlarmManager) applicationContext.getSystemService(Context.ALARM_SERVICE);
//                alarm.setRepeating(AlarmManager.RTC_WAKEUP, cal.getTimeInMillis(), 30000, pintent);

                PendingIntent pintent = PendingIntent.getService(ConnectionsManager.getInstance().getContext(), 0, new Intent(ConnectionsManager.getInstance().getContext(), CoreService.class), 0);
                AlarmManager alarm = (AlarmManager) ConnectionsManager.getInstance().getContext().getSystemService(Context.ALARM_SERVICE);
                alarm.cancel(pintent);
            }
        }
    }
}
