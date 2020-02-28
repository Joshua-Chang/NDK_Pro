//
// Created by JoshuaChang on 2020/2/28.
//
extern "C" {
#include <libavutil/time.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

#include "VideoChannel.h"

void dropPacket(queue<AVPacket *> &q) {
    while (!q.empty()) {
        AVPacket *pkt = q.front();
//        丢packet 压缩队列 有IBP帧
//        还是丢frame 已经被解码 没有关键帧
        if (pkt->flags != AV_PKT_FLAG_KEY) {//丢packet非关键帧
            q.pop();
            BaseChannel::releaseAvPacket(pkt);
        } else {
            break;
        }
    }
}

void dropFrame(queue<AVFrame *> &q) {
    while (!q.empty()) {
        AVFrame *frame = q.front();
        q.pop();
        BaseChannel::releaseAvFrame(frame);
    }
}

VideoChannel::VideoChannel(int id, JavaCallHelper *javaCallHelper, AVCodecContext *avCodecContext,
                           AVRational time_base)
        : BaseChannel(id, javaCallHelper, avCodecContext, time_base) {
    this->javaCallHelper = javaCallHelper;
    this->avCodecContext = avCodecContext;
    frame_queue.setSyncHandle(dropFrame);
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
        renderFrame(dst_data[0], dst_linesize[0], avCodecContext->width, avCodecContext->height);
        clock = frame->pts * av_q2d(time_base);//渲染时间戳
//        ----方案一
//         av_usleep(16*1000);//同步：不再使用固定的休眠16ms
//        double frame_delays = 1.0 / fps;
//        av_usleep(frame_delays * 1000000);

//        方案二
        double frame_delays = 1.0 / fps;
        double audioClock = audioChannel->clock;
        double diff = clock - audioClock;
//        av_usleep(frame_delays + diff * 1000000);//diff 有正负 所以 视频超前 让视频多休眠一会 视频滞后 少休眠一会

        //优化
//        fps帧率并没有把解码时间算进去 会有些许偏差
        double extra_delay = frame->repeat_pict / (2 * fps);//解码额外时间
        double delay = extra_delay + frame->repeat_pict;//总时间


        if (clock > audioClock) {//视频超前 让视频多休眠一会
            if (diff > 1) {// 差的比较大 视频太超前了
                av_usleep((delay * 2) * 1000000);
            } else {
                av_usleep(delay + diff * 1000000);
            }
        } else if (clock < audioClock) {//视频滞后 少休眠一会
            if (diff > 1) {// 差的比较大 不休眠了
//                av_usleep((delay*2)*1000000);
            } else if (diff > 0.05) {//视频需要追赶 ：丢帧
                releaseAvFrame(frame);//丢当前帧
                frame_queue.sync();//同步调用丢帧dropFrame/dropPacket
//                丢队列里的非关键帧
            }
        }
        releaseAvFrame(frame);
    }
    av_freep(&dst_data[0]);
    isPlaying = false;
    releaseAvFrame(frame);
    sws_freeContext(sws_ctx);
}

void VideoChannel::setRenderFrame(RenderFrame renderFrame) {
    /**
   * todo 渲染 转换后的rgb的dst_data 回调 一层*/
    this->renderFrame = renderFrame;
}

void VideoChannel::setFps(int fps) {
    this->fps = fps;
}
