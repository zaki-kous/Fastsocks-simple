package com.me.fastsocks.base;

import android.os.Handler;
import android.os.Looper;
import android.os.Message;

import java.util.concurrent.CountDownLatch;

/**
 * 底层回调上层队列
 *
 * Created by zaki on 17/3/26.
 */
public class DispatchQueue extends Thread {
    private volatile Handler handler = null;
    private volatile Handler mainHandler;
    private CountDownLatch syncLatch = new CountDownLatch(1);

    public DispatchQueue(final String threadName) {
        setName(threadName);
        start();
    }

    private void sendMessage(Message msg, int delay) {
        try {
            syncLatch.await();
            if (delay <= 0) {
                handler.sendMessage(msg);
            } else {
                handler.sendMessageDelayed(msg, delay);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void cancelRunnable(Runnable runnable) {
        try {
            syncLatch.await();
            handler.removeCallbacks(runnable);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void postRunnable(Runnable runnable) {
        postRunnable(runnable, 0);
    }

    public void postRunnable(Runnable runnable, long delay) {
        try {
            syncLatch.await();
            if (delay <= 0) {
                handler.post(runnable);
            } else {
                handler.postDelayed(runnable, delay);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void postMainRunnable(final Runnable runnable){
        postRunnable(new Runnable() {
            @Override
            public void run() {
                mainHandler.post(runnable);
            }
        }, 0);
    }

    public void cleanupQueue() {
        try {
            syncLatch.await();
            handler.removeCallbacksAndMessages(null);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    @Override
    public void run() {
        Looper.prepare();
        handler = new Handler();
        mainHandler = new Handler(Looper.getMainLooper());
        syncLatch.countDown();
        Looper.loop();
    }
}
