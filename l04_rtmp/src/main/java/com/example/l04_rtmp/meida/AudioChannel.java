package com.example.l04_rtmp.meida;
import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;

import com.example.l04_rtmp.LivePusher;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class AudioChannel {
    private int channelIConfig;
    private LivePusher mLivePusher;
    private AudioRecord audioRecord;
    private int channels=2;
    private ExecutorService executor;
    private boolean isLiving;
    int minBufferSize;
    private int inputSamples;

    public AudioChannel(LivePusher livePusher) {
        executor= Executors.newSingleThreadExecutor();
        mLivePusher = livePusher;
        if (channels==2) {
            channelIConfig = AudioFormat.CHANNEL_IN_STEREO;//双声道
            minBufferSize= AudioRecord.getMinBufferSize(44100, channelIConfig, AudioFormat.ENCODING_PCM_16BIT)*2;
        }else {
            channelIConfig=AudioFormat.CHANNEL_IN_MONO;//单声道
        minBufferSize = AudioRecord.getMinBufferSize(44100, channelIConfig, AudioFormat.ENCODING_PCM_16BIT);
        }
        //java-mic:getMinBufferSize的最小缓冲区大小
        //编码器的最小缓冲区大小
        //比较谁更小
        mLivePusher.native_setAudioEncInfo(44100,channels);
        inputSamples=mLivePusher.getInputSamples()*channels;
        audioRecord=new AudioRecord(MediaRecorder.AudioSource.MIC,44100,channelIConfig,AudioFormat.ENCODING_PCM_16BIT,minBufferSize>inputSamples?inputSamples:minBufferSize);
    }

    public void setChannels(int channels) {
        this.channels = channels;
    }
    public void startLive(){
        isLiving=true;
        executor.submit(new AudioTask());
    }

    class AudioTask implements Runnable {
        @Override
        public void run() {
            audioRecord.startRecording();//pcm 原始数据
            byte[] bytes = new byte[inputSamples];
            while (isLiving) {
                int len = audioRecord.read(bytes, 0, bytes.length);
                mLivePusher.native_pushAideo(bytes);
            }
        }
    }
}
