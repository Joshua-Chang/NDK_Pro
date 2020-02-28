//
// Created by JoshuaChang on 2020/2/28.
//

#ifndef NDK_PRO_WANGYIFFMPEG_H
#define NDK_PRO_WANGYIFFMPEG_H

#include <pthread.h>
#include <android/native_window.h>
#include "JavaCallHelper.h"
#include "VideoChannel.h"
#include "AudioChannel.h"
#include "BaseChannel.h"

extern "C"{
#include <libavformat/avformat.h>
#include <libavutil/time.h>
};
//控制层
class WangYiFfmpeg {
private:

    char dataSource;
public:
    WangYiFfmpeg(JavaCallHelper *javaCallHelper, const char *dataSource);
    ~WangYiFfmpeg();
    void prepare();
    void prepareFFmpeg();

    void start();
    void play();

    void setRenderCallBack(RenderFrame renderFrame);

private:
    pthread_t pid_prepare;//运行完就销毁
    pthread_t pid_play;//一直存在到播放完毕
    AVFormatContext *formatContext;
    char *url;
    JavaCallHelper *javaCallHelper;
    VideoChannel *videoChannel;
    AudioChannel *audioChannel;
    bool isPlaying;
    RenderFrame  renderFrame;
};


#endif //NDK_PRO_WANGYIFFMPEG_H
