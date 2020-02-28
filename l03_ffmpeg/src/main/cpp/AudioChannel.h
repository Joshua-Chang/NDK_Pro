//
// Created by JoshuaChang on 2020/2/28.
//

#ifndef NDK_PRO_AUDIOCHANNEL_H
#define NDK_PRO_AUDIOCHANNEL_H

extern "C"{
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
}
#include "JavaCallHelper.h"
#include "BaseChannel.h"

class AudioChannel :public BaseChannel{

public:
    AudioChannel(int id, JavaCallHelper *javaCallHelper, AVCodecContext *avCodecContext);

    virtual void play();

    virtual void stop();

    void initOpenSL();

    void decode();
    int getPcm();

private:
    pthread_t pid_audio_play;
    pthread_t pid_audio_decode;
    SwrContext *swr_ctx=NULL;
    int out_channels;
    int out_samplesize;
    int out_sample_rate;

};


#endif //NDK_PRO_AUDIOCHANNEL_H
