package com.me.fastsocks_simple;

import android.Manifest;
import android.content.pm.PackageManager;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.TextView;

import com.me.fastsocks_simple.base.OnRecvListener;
import com.me.fastsocks_simple.base.PackageDispatcher;
import com.me.fastsocks_simple.message.MessageModel;

public class MainActivity extends AppCompatActivity implements OnRecvListener {
    private static int MY_PERMISSIONS_REQUEST_WRITE_EXTERNAL_STORAGE = 400;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        TextView tv = (TextView) findViewById(R.id.sample_text);
        tv.setText("hello fastsocks.");

        PackageDispatcher.getInstance().addReceiverListener(this);

        if (ContextCompat.checkSelfPermission(this,
                Manifest.permission.WRITE_EXTERNAL_STORAGE)
                != PackageManager.PERMISSION_GRANTED) {

            ActivityCompat.requestPermissions(this,
                    new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE},
                    MY_PERMISSIONS_REQUEST_WRITE_EXTERNAL_STORAGE);
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults)
    {

        if (requestCode == MY_PERMISSIONS_REQUEST_WRITE_EXTERNAL_STORAGE) {
            return;
        }
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
    }


    @Override
    protected void onDestroy() {
        super.onDestroy();
        PackageDispatcher.getInstance().removeReceiverListener(this);
    }

    @Override
    public void onRecv(MessageModel messageModel) {
        //TODO 更新界面
    }
}
