//
// Created by JoshuaChang on 2020/2/28.
//
extern "C" {
#include <libavutil/time.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

#include "VideoChannel.h"

VideoChannel::VideoChannel(int id, JavaCallHelper *javaCallHelper, AVCodecContext *avCodecContext)
        : BaseChannel(id, javaCallHelper, avCodecContext) {
    this->javaCallHelper=javaCallHelper;
    this->avCodecContext=avCodecContext;

}

void *decode(void *args) {
    VideoChannel *videoChannel = static_cast<VideoChannel *>(args);
    videoChannel->decodePacket();
    return 0;
}

void *synchronize(void *args) {
    VideoChannel *videoChannel = static_cast<VideoChannel *>(args);
    videoChannel->synchronizeFrame();
    return 0;
}

void VideoChannel::play() {
    pkt_queue.setWork(1);
    frame_queue.setWork(1);
    isPlaying = true;
    pthread_create(&pid_video_play, NULL, decode, this);//解码
    pthread_create(&pid_synchronize, NULL, synchronize, this);//播放
}

void VideoChannel::decodePacket() {//解码子线程
    AVPacket *packet = 0;
    while (isPlaying) {
        int ret = pkt_queue.get(packet);
        if (!isPlaying) {
            break;
        }
        if (!ret) {
            continue;
        }
        ret = avcodec_send_packet(avCodecContext, packet);
        releaseAvPacket(packet);
        if (ret == AVERROR(EAGAIN)) {
            continue;
        } else if (ret < 0) {
            break;
        }
        AVFrame *frame = av_frame_alloc();
        ret = avcodec_receive_frame(avCodecContext, frame);
        frame_queue.put(frame);
        while (frame_queue.size() > 100 && isPlaying) {//生产速度大小消费速度
            av_usleep(1000 * 10);//队列满了 延缓生产速度 休眠10毫秒
            continue;
        }
    }
    releaseAvPacket(packet);
}

void VideoChannel::stop() {

}

void VideoChannel::synchronizeFrame() {//播放子线程
    /**
* 绘制准备（frame）YUV-》（image）RGB
* 1、获得转换上下文
* 2、yuv的frame->data---》rgb的dst_data
*/
    //初始化转化器上下文
    SwsContext *sws_ctx = sws_getContext(//AVPixelFormat
            avCodecContext->width, avCodecContext->height, avCodecContext->pix_fmt,//输入
            avCodecContext->width, avCodecContext->height, AV_PIX_FMT_RGBA,//输出
            SWS_BILINEAR,//重视速度
            0, 0, 0);
//    double frame_delay = 1.0 / fps;
    uint8_t *dst_data[4];//接收容器 argb
    int dst_linesize[4];//每一行的首地址
    //方法内部赋值dst_data
    av_image_alloc(dst_data, dst_linesize, avCodecContext->width, avCodecContext->height,
                   AV_PIX_FMT_RGBA, 1);//左对齐
    AVFrame *frame = 0;
    while (isPlaying) {
        int ret = frame_queue.get(frame);
        if (!isPlaying) {
            break;
        }
        if (!ret) {
            continue;
        }
        sws_scale(sws_ctx,//转换context
                  frame->data, frame->linesize, 0, frame->height,//需要转换的
                  dst_data, dst_linesize);//转换后的容器和首地址

        /**
         * todo 渲染 转换后的rgb的dst_data 回调 一层
         * 底层的渲染只有一个途径ANativeWindow  不停的将数据内存拷贝到缓冲区绘制
         */
         renderFrame(dst_data[0],dst_linesize[0],avCodecContext->width,avCodecContext->height);
         av_usleep(16*1000);
         releaseAvFrame(frame);
    }
    av_freep(&dst_data[0]);
    isPlaying= false;
    releaseAvFrame(frame);
    sws_freeContext(sws_ctx);
}

void VideoChannel::setRenderFrame(RenderFrame renderFrame) {
    /**
   * todo 渲染 转换后的rgb的dst_data 回调 一层*/
this->renderFrame = renderFrame;
}
