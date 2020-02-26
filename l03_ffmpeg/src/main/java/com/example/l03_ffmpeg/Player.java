package com.example.l03_ffmpeg;

import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class Player implements SurfaceHolder.Callback {
    static {
        System.loadLibrary("player");
    }
    private SurfaceHolder surfaceHolder;

    @Override
    public void surfaceCreated(SurfaceHolder holder) {

    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        this.surfaceHolder = surfaceHolder;
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {

    }

    public void setSurfaceView(SurfaceView surfaceView) {
        if (null != this.surfaceHolder) {
            this.surfaceHolder.removeCallback(this);
        }
        this.surfaceHolder = surfaceView.getHolder();
        this.surfaceHolder.addCallback(this);
    }

    public void start(String path) {
        native_start(path,surfaceHolder.getSurface());
    }
    public  native void native_start(String path, Surface surface);
    public native void sound(String input,String output);

}
