package com.me.fastsocks_simple.sys;

import com.google.firebase.iid.FirebaseInstanceId;
import com.google.firebase.iid.FirebaseInstanceIdService;
import com.me.fastsocks.utils.LogWrapper;

/**
 * 刷新Token，并上传服务器
 * <p>
 * Created by ZaKi
 */
public class TokenCreaterService extends FirebaseInstanceIdService {
    @Override
    public void onTokenRefresh() {
        String token = FirebaseInstanceId.getInstance().getToken();
        LogWrapper.d("onTokenRefresh token :" + token);

        //TODO 保存到文件
    }

}
