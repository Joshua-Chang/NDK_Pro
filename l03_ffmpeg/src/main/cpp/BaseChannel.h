//
// Created by JoshuaChang on 2020/2/28.
//

#ifndef NDK_PRO_BASECHANNEL_H
#define NDK_PRO_BASECHANNEL_H

#include "safe_queue.h"
#include "include/libavcodec/avcodec.h"
#include "include/libavutil/frame.h"
#include "JavaCallHelper.h"
#include "macro.h"

class BaseChannel {
public:
    BaseChannel(int id, JavaCallHelper *javaCallHelper,AVCodecContext *avCodecContext
//            ,AVRational base
                ) : channelId(id), avCodecContext(
            avCodecContext), javaCallHelper(javaCallHelper) {}

    virtual ~BaseChannel() {
        if (avCodecContext){
            avcodec_close(avCodecContext);
            avcodec_free_context(&avCodecContext);
            avCodecContext=0;
        }
        pkt_queue.clear();
        frame_queue.clear();
        LOGE("释放 channel: %d %d",pkt_queue.size(),frame_queue.size());

    };
    static void releaseAvPacket(AVPacket *&packet){
        if (packet){
            av_packet_free(&packet);
            packet=0;
        }
    }

    static void releaseAvFrame(AVFrame *&frame){
        if (frame){
            av_frame_free(&frame);
            frame=0;
        }
    }


    virtual void play()=0;
    virtual void stop()=0;


public:
    SafeQueue<AVPacket *> pkt_queue;
    SafeQueue<AVFrame *> frame_queue;
    volatile int channelId;
    volatile bool isPlaying;
    AVCodecContext *avCodecContext;
    JavaCallHelper *javaCallHelper;
};

#endif //NDK_PRO_BASECHANNEL_H
