package com.me.fastsocks.tcp;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.os.PowerManager;
import android.text.TextUtils;
import com.me.fastsocks.base.NativeByteBuffer;
import com.me.fastsocks.base.NativeLoader;
import com.me.fastsocks.tcp.listener.OnNativeSendListener;
import com.me.fastsocks.tcp.listener.OnSendRequestListener;
import com.me.fastsocks.tcp.receiver.ScreenStatusReceiver;
import com.me.fastsocks.tcp.tools.NetWorkTools;
import com.me.fastsocks.utils.BaseUtils;
import com.me.fastsocks.utils.LogWrapper;

/**
 * Created by zaki on 17/3/22.
 */
public class ConnectionsManager {
    private static ConnectionsManager Instance = null;

    private boolean isInit = false;

    private PacketDispatcherDelegate dispatcherDelegate;
    private BroadcastReceiver networkStateReceiver;
    private BroadcastReceiver screenReceiver;

    private PowerManager.WakeLock wakeLock = null;

    private Context context;

    public Context getContext() {
        return context;
    }

    public static ConnectionsManager getInstance() {
        ConnectionsManager localInstance = Instance;
        if (localInstance == null) {
            synchronized (ConnectionsManager.class) {
                localInstance = Instance;
                if (localInstance == null) {
                    Instance = localInstance = new ConnectionsManager();
                }
            }
        }
        return localInstance;
    }

    private ConnectionsManager() {

    }

    /**
     * 连接状态
     */
    public enum ConnectionState{
        UNKNOWN(-1),
        ConnectionStateConnecting(1),
        ConnectionStateWaitingForNetwork(2),
        ConnectionStateConnected(3);

        private final int state;

        ConnectionState(int state){
            this.state = state;
        }

        public static ConnectionState valueOf(int state){
            for (ConnectionState c : ConnectionState.values()) {
                if (state == c.state) {
                    return c;
                }
            }
            ConnectionState unknownType = ConnectionState.UNKNOWN;
            return unknownType;
        }
        public int value(){
            return state;
        }
    }

    /**
     * 发送错误
     */
    public enum ERROR {
        WithoutConnectin(300101, "current witout connect."),//当前没有连接
        WithoutLogin(300102, "current witout login."),//当前连接没有握手成功
        ConnectFail(300103, "current witout connect."),//当前连接中断
        Timeout(300104, "request timeout."),//发送数据包超时
        RetryLimit(300105, "request retry limit."),//重发次数限制
        UNKNOWN(300100, "UNKNOWN.");//未知
        private final int errorCode;
        private final String desc;

        ERROR(int errorCode, String desc) {
            this.errorCode = errorCode;
            this.desc = desc;
        }

        public static ERROR valueOf(int errorCode, String desc){
            if(errorCode == 0){
                return null;
            }
            for (ERROR c : ERROR.values()) {
                if (errorCode == c.errorCode) {
                    return c;
                }
            }
            ERROR unknownType = ERROR.UNKNOWN;
            return unknownType;
        }

        @Override
        public String toString() {
            return "ERROR{" +
                    "errorCode=" + errorCode +
                    ", desc='" + desc + '\'' +
                    '}';
        }
    }

    public enum RequestFlag{
        UNKNOWN(0),//未知
        FlagWithoutAck(1),//没有回包
        FlagReSend(2),//数据包可以重发
        FlagWithoutLogin(4);//数据包在握手之前发送

        private final int flags;
        RequestFlag(int flags){
            this.flags = flags;
        }

        public static RequestFlag valueOf(int value){
            for (RequestFlag c : RequestFlag.values()) {
                if (value == c.flags) {
                    return c;
                }
            }
            RequestFlag unknownType = RequestFlag.UNKNOWN;
            return unknownType;
        }

        public int value(){
            return flags;
        }
    }

    //设置信息
    public void init(PacketDispatcherDelegate dispatcherDelegate, Context context,
                     String crashPath){
        this.context = context;
        this.dispatcherDelegate = dispatcherDelegate;
//        LogWrapper.d("crash path : "+crashPath);
        NativeLoader.init(crashPath, true);
        native_setJava();

        try {
            PowerManager powerManager = (PowerManager) this.context.getSystemService(Context.POWER_SERVICE);
            wakeLock = powerManager.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "lock");
            wakeLock.setReferenceCounted(false);
        } catch (Throwable throwable) {
            throwable.printStackTrace();
        }
    }

    /**
     * 启动底层库
     *
     * @param context
     * @param uin
     */
    public void startInit(Context context, long uin){
        if(isInit){
            native_setUin(uin);
            registerReceiver(context);
            return;
        }
        isInit = true;
        native_init(uin);
        registerReceiver(context);
        checkConnection(context);
        addSvr();
    }

    //退出登录
    public void logout(Context context){
        unRegisterReceiver(context);
        native_logout();
    }

    /**
     * 添加服务器ip列表
     */
    private void addSvr() {
        // TODO 模拟设置五个ip
        //请求服务器拿到服务器ip列表(一般都是json)，设置到底层库
        //一个ip最多设置2个端口,比如:127.0.0.1:80与127.0.0.1:443，后面设置的端口会覆盖前面的端口
        addSvrAddr("192.168.1.110", (short) 80);
        addSvrAddr("192.168.1.110", (short) 443);
        addSvrAddr("192.168.1.111", (short) 80);
        addSvrAddr("192.168.1.111", (short) 443);
        addSvrAddr("192.168.1.112", (short) 80);
        addSvrAddr("192.168.1.113", (short) 443);
        addSvrAddr("192.168.1.114", (short) 80);
        addSvrAddr("192.168.1.114", (short) 443);
    }

    /**
     * 应用初始化，告诉底层库是否有网
     */
    private void checkConnection(Context context){
        native_setNetworkAvailable(NetWorkTools.isNetWorkConnect(context));
    }

    //通知底层库进入暂停状态
    public void pauseNetwork() {
        native_pauseNetwork();
    }

    public void resumeNetwork(boolean isPush){
        native_resumeNetwork(isPush);
    }

    private void registerReceiver(Context context){
        if(networkStateReceiver == null) {
            networkStateReceiver = new BroadcastReceiver() {
                @Override
                public void onReceive(Context context, Intent intent) {
                    checkConnection(context);
                }
            };
            IntentFilter filter = new IntentFilter(ConnectivityManager.CONNECTIVITY_ACTION);
            context.registerReceiver(networkStateReceiver, filter);
        }

        if (screenReceiver == null) {
            //动态注册屏幕亮起和熄灭广播
            screenReceiver = new ScreenStatusReceiver();
            IntentFilter screenFilter = new IntentFilter();
            screenFilter.addAction(Intent.ACTION_SCREEN_ON);
            screenFilter.addAction(Intent.ACTION_SCREEN_OFF);
            context.registerReceiver(screenReceiver,screenFilter);
        }
    }

    private void unRegisterReceiver(Context context){
        if (networkStateReceiver != null) {
            context.unregisterReceiver(networkStateReceiver);
            networkStateReceiver = null;
        }
        if (screenReceiver != null) {
            context.unregisterReceiver(screenReceiver);
            screenReceiver = null;
        }
    }

    public static class Request {
        public int cmd;
        public int address;
        public OnSendRequestListener onSendRequestListener;
        public int flags;
        public long timeout;

        public Request(Request.Builder builder){
            this.cmd = builder.cmd;
            if(builder.buffer != null && builder.buffer.length > 0){
                this.address = NativeByteBuffer.wrap(builder.buffer);
            }
            this.onSendRequestListener = builder.onSendRequestListener;
            this.flags = builder.flags.value();
            this.timeout = builder.timeout;
        }

        public static class Builder{
            private int cmd;
            private byte[] buffer;
            private OnSendRequestListener onSendRequestListener;
            private RequestFlag flags;
            private long timeout;

            public Builder(int cmd){
                this.cmd = cmd;
            }

            public Builder buffer(byte[] buffer){
                this.buffer = buffer;
                return this;
            }

            public Builder timeout(long timeout){
                this.timeout = timeout;
                return this;
            }

            public Builder flags(RequestFlag flags){
                this.flags = flags;
                return this;
            }

            public Builder loadSendListener(OnSendRequestListener onSendRequestListener){
                this.onSendRequestListener = onSendRequestListener;
                return this;
            }

            public void start(){
                final Request request = new Request(this);
                ConnectionsManager.native_sendRequest(request, new OnNativeSendListener() {
                    @Override
                    public void onSendComplete(int address, int errorCode, String desc) {
                        if (request.onSendRequestListener != null) {
                            final ERROR error = ERROR.valueOf(errorCode, desc);
                            final byte[] buffer =  NativeByteBuffer.with(address);//先拷贝出来;
                            BaseUtils.nativeQueue.postMainRunnable(new Runnable() {
                                @Override
                                public void run() {
                                    request.onSendRequestListener.onSendComplete(buffer, error);
                                }
                            });
                        }
                    }
                });
            }
        }
    }

    public void onUpdate(){
        //update //
        if (dispatcherDelegate != null) {
            dispatcherDelegate.onUpdate();
        }
    }

    public void onConnectionStateChanged(final int connectState) {
        LogWrapper.d("---onConnectionStateChanged---" + ConnectionState.valueOf(connectState));
        /**
         * 0:链接成功
         * 1和2:正在连接
         * 3:连接断开
         */
        BaseUtils.nativeQueue.postMainRunnable(new Runnable() {
            @Override
            public void run() {
                if (dispatcherDelegate != null) {
                    dispatcherDelegate.onConnectionStateChanged(connectState);
                }
            }
        });
    }

    public void onRecvMessages(final int cmd, int address) {
        final byte[] recvDatas = NativeByteBuffer.with(address);
        if(recvDatas != null){
            LogWrapper.d("onRecvMessages---cmd : " + cmd + ", length : "+recvDatas.length);
            BaseUtils.nativeQueue.postMainRunnable(new Runnable() {
                @Override
                public void run() {
                    if (dispatcherDelegate != null) {
                        dispatcherDelegate.msgRecv(cmd, recvDatas);
                    }
                }
            });
        }
    }

    //底层库回调握手
    public int onHandshakeConnected(int buffer){
        if(buffer == 0){
            // buffer == 0 表示底层库需要上层的握手信息
            return requestHandShake();
        } else {
            // buffer != 0 表示底层库需要上层解析服务器返回的握手信息
            byte[] data = NativeByteBuffer.with(buffer);
            //TODO 判断data数据包里面的握手返回是否成功
            // 如果返回0表示握手成功，否则握手失败
            // 握手失败会重连
            return -1;
        }
    }


    private int requestHandShake(){
        //TODO 返回握手数据的地址(把服务器需要校验的数据转换成地址返回给底层库)
        int bufferAddress = NativeByteBuffer.wrap(new byte[128]);
        return bufferAddress;
    }


    public Request.Builder with(int cmd){
        return new Request.Builder(cmd);
    }

    /**
     * 增加长连接ip
     *
     * @param address
     * @param port
     */
    public void addSvrAddr(String address, short port) {
        if (!TextUtils.isEmpty(address) && port > 0) {
            native_addSvrAddr(address, port);
        }
    }

    /**
     * 强制使用该ip
     *
     * @param address
     * @param port
     */
    public void setSvrAddr(String address, short port) {
        if (!TextUtils.isEmpty(address) && port > 0) {
            native_setSvrAddr(address, port);
        }
    }

    //唤醒设备
    public void wakeUp(){
        BaseUtils.nativeQueue.postMainRunnable(new Runnable() {
            @Override
            public void run() {
                try {
                    if (!wakeLock.isHeld()) {
                        wakeLock.acquire(10000);
                    }
                } catch (Throwable throwable) {
                    throwable.printStackTrace();
                }

            }
        });
    }
    public native void native_setJava();
    public static native void native_init(long uin);
    public static native void native_setUin(long uin);
    public static native void native_logout();
    public static native void native_setNetworkAvailable(boolean isAvailable);
    public static native void native_sendRequest(Request request, OnNativeSendListener onNativeSendListener);
    public static native void native_addSvrAddr(String address, short port);
    public static native void native_setSvrAddr(String address, short port);
    public static native void native_pauseNetwork();
    public static native void native_resumeNetwork(boolean isPush);
}
