package com.me.fastsocks_simple;

import android.app.Application;
import android.os.Environment;

import com.me.fastsocks.tcp.ConnectionsManager;
import com.me.fastsocks.utils.StartUtils;
import com.me.fastsocks_simple.base.PackageDispatcher;

import java.io.File;

/**
 * Created by zaki on 17/4/8.
 */
public class MainApplication extends Application{

    private static MainApplication instance;

    @Override
    public void onCreate() {
        super.onCreate();

        instance = this;

        ConnectionsManager.getInstance().init(PackageDispatcher.getInstance(), this, getCrashPath());

        //启动核心服务
        StartUtils.startSdkService(this);
    }

    public static String getCrashPath() {
        return Environment.getExternalStorageDirectory() +
                File.separator + "fastsocks" + File.separator + "crash";
    }

    public static MainApplication getInstance(){
        return instance;
    }
}
