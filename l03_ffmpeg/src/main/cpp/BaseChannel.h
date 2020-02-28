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
            ,AVRational time_base
                ) : channelId(id), avCodecContext(
            avCodecContext), javaCallHelper(javaCallHelper),time_base(time_base) {
        frame_queue.setReleaseCallback(releaseAvFrame2);
        pkt_queue.setReleaseCallback(releaseAvPacket2);
    }

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

    /**
 * 释放 AVPacket
 * @param packet
 */
    static void releaseAvPacket2(AVPacket **packet) {
        if (packet) {
            av_packet_free(packet);
            //为什么用指针的指针？
            // 指针的指针能够修改传递进来的指针的指向
            *packet = 0;
        }
    }

    static void releaseAvFrame2(AVFrame **frame) {
        if (frame) {
            av_frame_free(frame);
            //为什么用指针的指针？
            // 指针的指针能够修改传递进来的指针的指向
            *frame = 0;
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
    AVRational time_base;
    double clock=0;
};

#endif //NDK_PRO_BASECHANNEL_H
