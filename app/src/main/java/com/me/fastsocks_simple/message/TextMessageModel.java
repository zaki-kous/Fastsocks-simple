package com.me.fastsocks_simple.message;

/**
 * 文本消息
 *
 * Created by zaki on 17/4/8.
 */
public class TextMessageModel extends MessageModel{
    private String text;//文本内容
    @Override
    protected void toContent(byte[] data) {
        //TODO 解析出文本
        text = "hello";
    }

    public String getText() {
        return text;
    }

    @Override
    public String toString() {
        return "TextMessageModel{" + super.toString() +
                ", text='" + text + '\'' +
                '}';
    }
}
