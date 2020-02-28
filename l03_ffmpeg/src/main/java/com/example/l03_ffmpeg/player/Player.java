package com.example.l03_ffmpeg.player;

import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class Player implements SurfaceHolder.Callback {
    static {
        System.loadLibrary("player");
    }

    private SurfaceHolder surfaceHolder;
    private String dataSource;

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        native_set_surface(surfaceHolder.getSurface());
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

    public native void native_prepare(String dataSource);

    public native void native_start();

    public native void native_set_surface(Surface surface);

    public void prepare() {
        native_prepare(dataSource);

    }

    public void setDataSource(String dataSource) {
        this.dataSource=dataSource;
    }

    /**
     * 提供方法给native反射调用, 再通过接口暴露给activity
     * */
    public void onPrepare(){
        if (onPrepareListener != null) {
            onPrepareListener.onPrepare();
        }
    }
    //进度不断调用
    public void onProgress(int progress){
        if (onProgressListener != null) {
            onProgressListener.onProgress(progress);
        }
    }
    public void onError(int errorCode){
        if (onErrorListener != null) {
            onErrorListener.onError(errorCode);
        }
    }

    public void start() {
        native_start();
    }

    /**
     * 提供接口给activity    调用
     * */
    public interface OnPrepareListener{void onPrepare();}
    public interface OnProgressListener{void onProgress(int progress);}
    public interface OnErrorListener{void onError(int errorCode);}
    private OnProgressListener onProgressListener;
    private OnPrepareListener onPrepareListener;
    private OnErrorListener onErrorListener;

    public void setOnProgressListener(OnProgressListener onProgressListener) {
        this.onProgressListener = onProgressListener;
    }

    public void setOnPrepareListener(OnPrepareListener onPrepareListener) {
        this.onPrepareListener = onPrepareListener;
    }

    public void setOnErrorListener(OnErrorListener onErrorListener) {
        this.onErrorListener = onErrorListener;
    }
}
