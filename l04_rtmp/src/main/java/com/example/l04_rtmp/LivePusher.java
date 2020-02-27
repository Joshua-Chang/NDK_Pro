package com.example.l04_rtmp;

import android.app.Activity;
import android.view.SurfaceHolder;

import com.example.l04_rtmp.meida.AudioChannel;
import com.example.l04_rtmp.meida.VideoChannel;


public class LivePusher {
    private AudioChannel audioChannel;
    private VideoChannel videoChannel;
    static {
        System.loadLibrary("rtmpplayer");
    }

    public LivePusher(Activity activity, int width, int height, int bitrate,
                      int fps, int cameraId) {
        native_init();
        videoChannel = new VideoChannel(this, activity, width, height, bitrate, fps, cameraId);
        audioChannel = new AudioChannel(this);
    }
    public void setPreviewDisplay(SurfaceHolder surfaceHolder) {
        videoChannel.setPreviewDisplay(surfaceHolder);
    }
    public void startLive(String path) {
        native_start(path);
        videoChannel.startLive();
        audioChannel.startLive();
    }
    public void switchCamera() {
        videoChannel.switchCamera();
    }
    public native void native_setVideoEncInfo(int width, int height, int fps, int bitrate);
    public native void native_setAudioEncInfo(int samplesInHZ, int channels);
    public native void native_start(String path);
    public native void native_pushVideo(byte[] data);
    public native void native_pushAideo(byte[] data);
    public native void native_init();
    public native int getInputSamples();
}
