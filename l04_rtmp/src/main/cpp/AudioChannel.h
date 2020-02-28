//
// Created by JoshuaChang on 2020/2/27.
//

#ifndef NDK_PRO_AUDIOCHANNEL_H
#define NDK_PRO_AUDIOCHANNEL_H


#include <faac.h>
#include <jni.h>
#include "librtmp/rtmp.h"

class AudioChannel {
    typedef void (*AudioCallback)(RTMPPacket* packet);
public:
    void encodeData(int8_t *data);
    void setAudioEncInfo(jint samplesInHZ, jint channels);
    void setAudioCallback(AudioCallback audioCallback);
    RTMPPacket *setAudioTag();
    int getInputSamples();

private:
    int mChannels;
    faacEncHandle  audioCodec;//编码器及参数
    u_long inputSamples;//缓冲区大小
    u_long maxOutputBytes;//最大缓冲区大小
    u_char *buffer=0;
    AudioCallback audioCallback;

};


#endif //NDK_PRO_AUDIOCHANNEL_H
