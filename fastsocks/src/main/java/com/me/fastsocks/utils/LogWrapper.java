package com.me.fastsocks.utils;

import android.util.Log;

import com.me.fastsocks.BuildConfig;

/**
 * Created by zaki on 17/3/26.
 */

public class LogWrapper {

    private static boolean DEBUG = BuildConfig.DEBUG;

    public boolean setDebug(boolean debug) {
        return (DEBUG = BuildConfig.DEBUG & debug);
    }

    public static void v(String tag, String msg) {
        if (DEBUG) Log.v("" + tag, "" + msg);
    }

    public static void v(String tag, String msg, Throwable tr) {
        if (DEBUG) Log.v("" + tag, "" + msg, tr);
    }

    public static void v(String tag, Throwable tr) {
        if (DEBUG) Log.v("" + tag, "", tr);
    }

    public static void v(String tag, Object obj) {
        if (DEBUG) Log.v("" + tag, "" + obj);
    }

    public static void d(String msg) {
        Log.d("fastsocks", "" + msg);
    }
    public static void d(String tag, String msg) {
        if (DEBUG) Log.d("" + tag, "" + msg);
    }

    public static void d(String tag, String msg, Throwable tr) {
        if (DEBUG) Log.d("" + tag, "" + msg, tr);
    }

    public static void d(String tag, Throwable tr) {
        if (DEBUG) Log.d("" + tag, "", tr);
    }

    public static void d(String tag, Object obj) {
        if (DEBUG) Log.d("" + tag, "" + obj);
    }

    public static void i(String tag, String msg) {
        if (DEBUG) Log.i("" + tag, "" + msg);
    }

    public static void i(String tag, String msg, Throwable tr) {
        if (DEBUG) Log.i("" + tag, "" + msg, tr);
    }

    public static void i(String tag, Throwable tr) {
        if (DEBUG) Log.i("" + tag, "", tr);
    }

    public static void i(String tag, Object obj) {
        if (DEBUG) Log.i("" + tag, "" + obj);
    }

    public static void w(String tag, String msg) {
        if (DEBUG) Log.w("" + tag, "" + msg);
    }

    public static void w(String tag, String msg, Throwable tr) {
        if (DEBUG) Log.w("" + tag, "" + msg, tr);
    }

    public static void w(String tag, Throwable tr) {
        if (DEBUG) Log.w("" + tag, tr);
    }

    public static void w(String tag, Object obj) {
        if (DEBUG) Log.w("" + tag, "" + obj);
    }

    public static void e(String tag, String msg) {
        if (DEBUG) Log.e("" + tag, "" + msg);
    }

    public static void e(String tag, String msg, Throwable tr) {
        if (DEBUG) Log.e("" + tag, "" + msg, tr);
    }

    public static void e(String tag, Throwable tr) {
        if (DEBUG) Log.e("" + tag, "", tr);
    }

    public static void e(String tag, Object obj) {
        if (DEBUG) Log.e("" + tag, "" + obj);
    }
}
