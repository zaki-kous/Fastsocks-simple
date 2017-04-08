package com.me.fastsocks_simple.message;


/**
 * 消息实体类，一般情况下都要入库和缓存
 *
 * Created by zaki on 17/4/8.
 */
public abstract class MessageModel {
    private long fromUin;//对方uin
    private String fromName;//对方昵称
    private String avatarUrl;//对方头像url
    private MsgType msgType;//消息类型
    //-----
    //-----
    //-----消息公共字段

    public long getFromUin() {
        return fromUin;
    }

    public void setFromUin(long fromUin) {
        this.fromUin = fromUin;
    }

    public String getFromName() {
        return fromName;
    }

    public void setFromName(String fromName) {
        this.fromName = fromName;
    }

    public String getAvatarUrl() {
        return avatarUrl;
    }

    public void setAvatarUrl(String avatarUrl) {
        this.avatarUrl = avatarUrl;
    }

    public MsgType getMsgType() {
        return msgType;
    }

    public void setMsgType(MsgType msgType) {
        this.msgType = msgType;
    }

    public static MessageModel build (byte[] data) {
        if (data != null && data.length > 0) {
            //TODO 模拟解析成不同的消息
            MsgType msgType = MsgType.valueOf(0);
            MessageModel messageModel = null;
            switch (msgType) {
                case TEXT:

                    break;
                case IMAGE:

                    break;
                case VIOCE:

                    break;
            }
            if (messageModel != null) {
                // TODO 先解析公共字段
                messageModel.setMsgType(msgType);
                messageModel.setFromUin(1000000000L);
                messageModel.setAvatarUrl("http://baidu.com/1314521.png");
                messageModel.setFromName("z_one");
                messageModel.toContent(data);
            }
            return messageModel;
        }
        return null;
    }

    protected abstract void toContent(byte[] data);


    public enum MsgType{
        UNKNOWN(-1),
        TEXT(1),
        IMAGE(2),
        VIOCE(3);

        private final int type;

        MsgType(int type){
            this.type = type;
        }

        public static MsgType valueOf(int type){
            for (MsgType m : MsgType.values()) {
                if (type == m.type) {
                    return m;
                }
            }
            return UNKNOWN;
        }
        public int value(){
            return type;
        }
    }

    @Override
    public String toString() {
        return "MessageModel{" +
                "fromUin=" + fromUin +
                ", fromName='" + fromName + '\'' +
                ", avatarUrl='" + avatarUrl + '\'' +
                ", msgType=" + msgType +
                '}';
    }
}
