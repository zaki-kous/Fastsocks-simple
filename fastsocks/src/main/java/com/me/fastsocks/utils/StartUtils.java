package com.me.fastsocks.utils;

import android.content.Context;
import android.content.Intent;

import com.me.fastsocks.service.CoreService;

/**
 * Created by zaki on 17/4/8.
 */

public class StartUtils {

    public static void stopSdkService(Context context) {
        LogWrapper.d("stop CoreService by stopSdkService");
        //停止CoreService自启
        context.stopService((new Intent(context, CoreService.class)));
    }

    public static void startSdkService(Context context) {
        LogWrapper.d("start CoreService by startSdkService");
        context.startService(new Intent(context, CoreService.class));
    }
}
