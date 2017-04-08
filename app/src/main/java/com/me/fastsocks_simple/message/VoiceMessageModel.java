package com.me.fastsocks_simple.message;

/**
 * 语音消息
 *
 * Created by zaki on 17/4/9.
 */
public class VoiceMessageModel extends MessageModel{
    private long durtion;//语音时长
    private String voiceUrl;//语言下载连接
    @Override
    protected void toContent(byte[] data) {
        //TODO 解析对应字段
        this.durtion = 2000;
        this.voiceUrl = "http://baidu.com/123.pcm";
    }

    public long getDurtion() {
        return durtion;
    }

    public String getVoiceUrl() {
        return voiceUrl;
    }

    @Override
    public String toString() {
        return "VoiceMessageModel{" + super.toString() +
                ", durtion=" + durtion +
                ", voiceUrl='" + voiceUrl + '\'' +
                '}';
    }
}
