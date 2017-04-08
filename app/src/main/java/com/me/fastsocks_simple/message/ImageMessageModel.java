package com.me.fastsocks_simple.message;

/**
 * 图片消息
 *
 * Created by zaki on 17/4/9.
 */
public class ImageMessageModel extends MessageModel{
    private int width;
    private int height;//图片宽高
    private String imageUrl;//图片url
    @Override
    protected void toContent(byte[] data) {
        // TODO 解析对应字段
        this.width = 480;
        this.height = 800;
        this.imageUrl = "http://baidu.com/123.png";
    }

    public int getWidth() {
        return width;
    }

    public int getHeight() {
        return height;
    }

    public String getImageUrl() {
        return imageUrl;
    }

    @Override
    public String toString() {
        return "ImageMessageModel{" + super.toString() +
                ", width=" + width +
                ", height=" + height +
                ", imageUrl='" + imageUrl + '\'' +
                '}';
    }
}
