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
typedef void (*RenderFrame)(uint8_t *,int ,int ,int);

class VideoChannel :public BaseChannel{

public:
    VideoChannel(int id, JavaCallHelper *javaCallHelper, AVCodecContext *avCodecContext);

    virtual void play();

    virtual void stop();
    void decodePacket();

    void synchronizeFrame();

    void setRenderFrame(RenderFrame renderFrame);

private:
    pthread_t pid_video_play;
    pthread_t pid_synchronize;
    RenderFrame renderFrame;
};


#endif //NDK_PRO_VIDEOCHANNEL_H
