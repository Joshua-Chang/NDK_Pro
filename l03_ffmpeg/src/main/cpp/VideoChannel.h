//
// Created by JoshuaChang on 2020/2/28.
//

#ifndef NDK_PRO_VIDEOCHANNEL_H

extern "C"{
#include <libavcodec/avcodec.h>
};
#define NDK_PRO_VIDEOCHANNEL_H
#include "JavaCallHelper.h"
#include "BaseChannel.h"
#include "AudioChannel.h"

typedef void (*RenderFrame)(uint8_t *,int ,int ,int);

class VideoChannel :public BaseChannel{

public:
    VideoChannel(int id, JavaCallHelper *javaCallHelper, AVCodecContext *avCodecContext,
                 AVRational rational);

    virtual void play();

    virtual void stop();
    void decodePacket();

    void synchronizeFrame();

    void setRenderFrame(RenderFrame renderFrame);

private:
    pthread_t pid_video_play;
    pthread_t pid_synchronize;
    RenderFrame renderFrame;
    int fps;
public:
    void setFps(int fps);
    AudioChannel *audioChannel;
};


#endif //NDK_PRO_VIDEOCHANNEL_H
