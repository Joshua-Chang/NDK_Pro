package com.example.l04_rtmp;

import androidx.appcompat.app.AppCompatActivity;

import android.Manifest;
import android.content.pm.PackageManager;
import android.hardware.Camera;
import android.os.Build;
import android.os.Bundle;
import android.view.SurfaceView;
import android.view.View;

public class MainActivity extends AppCompatActivity {
    private LivePusher livePusher;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            String[] perms = {Manifest.permission.CAMERA,Manifest.permission.RECORD_AUDIO,};
            if (checkSelfPermission(perms[0]) == PackageManager.PERMISSION_DENIED) {
                requestPermissions(perms, 200);
            }
        }

        setContentView(R.layout.activity_main);
        SurfaceView surfaceView = findViewById(R.id.surfaceView);
        livePusher = new LivePusher(this, 800, 480, 800_000, 10, Camera.CameraInfo.CAMERA_FACING_BACK);
        //  设置摄像头预览的界面
        livePusher.setPreviewDisplay(surfaceView.getHolder());
    }

    public void switchCamera(View view) {
    }

    public void startLive(View view) {
        livePusher.startLive("rtmp://47.107.132.117/myapp");
    }

    public void stopLive(View view) {
    }

}
