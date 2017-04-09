package com.me.fastsocks_simple.sys;

import com.google.firebase.messaging.FirebaseMessagingService;
import com.google.firebase.messaging.RemoteMessage;
import com.me.fastsocks.tcp.ConnectionsManager;
import com.me.fastsocks.utils.LogWrapper;
import com.me.fastsocks.utils.StartUtils;
import com.me.fastsocks_simple.MainApplication;

/**
 * fcm消息接收器
 * <p>
 * Created by ZaKi
 */
public class MsgReceptorService extends FirebaseMessagingService {

    /**
     * Called when message is received.
     *
     * @param remoteMessage Object representing the message received from Firebase Cloud Messaging.
     */
    // [START receive_message]
    @Override
    public void onMessageReceived(RemoteMessage remoteMessage) {
        // [START_EXCLUDE]
        // There are two types of messages data messages and notification messages. Data messages are handled
        // here in onMessageReceived whether the app is in the foreground or background. Data messages are the type
        // traditionally used with GCM. Notification messages are only received here in onMessageReceived when the app
        // is in the foreground. When the app is in the background an automatically generated notification is displayed.
        // When the user taps on the notification they are returned to the app. Messages containing both notification
        // and data payloads are treated as notification messages. The Firebase console always sends notification
        // messages. For more see: https://firebase.google.com/docs/cloud-messaging/concept-options
        // [END_EXCLUDE]

        // Not getting messages here? See why this may be: https://goo.gl/39bRNJ
        String recvFrom = remoteMessage.getFrom();
        LogWrapper.d("From: " + recvFrom);
        // Check if message contains a data payload.
        if (remoteMessage.getData().size() > 0) {
            LogWrapper.d("Message data payload: " + remoteMessage.getData());
        }

        // Check if message contains a notification payload.
        if (remoteMessage.getNotification() != null) {
            LogWrapper.d("Message Notification Body: " + remoteMessage.getNotification().getBody());
        }
        ConnectionsManager.getInstance().wakeUp();
        ConnectionsManager.getInstance().resumeNetwork(true);
        //检查
        StartUtils.startSdkService(MainApplication.getInstance());
    }
}
