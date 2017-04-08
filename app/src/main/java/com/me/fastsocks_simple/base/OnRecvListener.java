package com.me.fastsocks_simple.base;

import com.me.fastsocks_simple.message.MessageModel;

/**
 * 接受监听
 *
 * Created by zaki on 17/4/8.
 */
public interface OnRecvListener {

    void onRecv(MessageModel messageModel);
}
