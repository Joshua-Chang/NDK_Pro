//
// Created by JoshuaChang on 2020/2/26.
//

#ifndef NDK_PRO_VIDEOCHANNEL_H
#define NDK_PRO_VIDEOCHANNEL_H


#include <x264.h>
#include <jni.h>
#include "librtmp/rtmp.h"

class VideoChannel {
    typedef void (*VideoCallback)(RTMPPacket* packet);
public:
    void setVideoEncInfo(jint width, jint height, jint fps, jint bitrate);

    void encodeData(int8_t* data);//将nv21转换为i420
    void setVideoCallback(VideoCallback videoCallback);

private:
    int mWidth;
    int mHeight;
    int mFps;
    int mBitrate;
    int ySize;
    int uvSize;
    x264_t *videoCodec;//编码器
    x264_picture_t *pic_in;//一帧
    VideoCallback videoCallback;

    void sendSpsPps(uint8_t *sps, uint8_t *pps, int len, int pps_len);

    void sendFrame(int type, uint8_t *payload, int i_payload);
};


#endif //NDK_PRO_VIDEOCHANNEL_H
