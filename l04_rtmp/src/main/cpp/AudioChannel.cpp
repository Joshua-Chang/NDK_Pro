//
// Created by JoshuaChang on 2020/2/27.
//

#include <pty.h>
#include <jni.h>
#include <cstring>
#include "AudioChannel.h"
#include "faac.h"
#include "librtmp/rtmp.h"
RTMPPacket *AudioChannel::setAudioTag(){//第一帧空的帧 存放编码参数，用于接收播放时解码 类似与sps pps
    u_char *buf;
    u_long len;
    //编码器信息  为了解码
    faacEncGetDecoderSpecificInfo(audioCodec, &buf, &len);//后两个为入參出參
    int bodySize=2+len;

    RTMPPacket *packet = new RTMPPacket;
    RTMPPacket_Alloc(packet, bodySize);
    packet->m_body[0] = 0xAF;
    if (mChannels==1){//单声道
        packet->m_body[0]=0xAE;
    }
    packet->m_body[1] = 0x01;
    //编码后的aac内容
    memcpy(&packet->m_body[2], buffer, len);

//          aac
    packet->m_hasAbsTimestamp = 0;//相对时间
    packet->m_nBodySize = bodySize;
    packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
    packet->m_nChannel = 0x11;
    packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
    return packet;
}

void AudioChannel::encodeData(int8_t *data) {
    /**
     * todo 二、编码
     */
    int bytelen = faacEncEncode(audioCodec, reinterpret_cast<int32_t *>(data), inputSamples, buffer,
                                maxOutputBytes);//得到参数三五缓冲区，并把数据输出到buffer缓冲区
    if (bytelen > 0) {
//          得出的buffer相当于一个音频帧
        RTMPPacket *packet = new RTMPPacket;
        int bodySize = 2 + bytelen;
        RTMPPacket_Alloc(packet, bodySize);
        packet->m_body[0] = 0xAF;
        if (mChannels==1){//单声道
            packet->m_body[0]=0xAE;
        }
        packet->m_body[1] = 0x01;
        //编码后的aac内容
        memcpy(&packet->m_body[2], buffer, bytelen);

//          aac
        packet->m_hasAbsTimestamp = 0;//相对时间
        packet->m_nBodySize = bodySize;
        packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
        packet->m_nChannel = 0x11;
        packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
        audioCallback(packet);
    }
}

void AudioChannel::setAudioEncInfo(int samplesInHZ, int channels) {
    /**
     * todo 一、初始化faac编码器
     */
    //java-mic:getMinBufferSize的最小缓冲区大小
    //编码器的最小缓冲区大小
    //比较谁更小
    audioCodec = faacEncOpen(samplesInHZ, channels, &inputSamples,
                             &maxOutputBytes);//入參出參 打开编码器、对后面的两个参数赋值
    faacEncConfigurationPtr config = faacEncGetCurrentConfiguration(audioCodec);
    config->mpegVersion = MPEG4;//编码版本
    config->aacObjectType = LOW;//编码标准 越低速度越快
    config->inputFormat = FAAC_INPUT_16BIT;//16位
    config->outputFormat = 0;//编码出原始数据，既不是1：adts（音频编码标准）也不是adif
    faacEncSetConfiguration(audioCodec, config);//配置
    buffer = new u_char[maxOutputBytes];

}

int AudioChannel::getInputSamples() {
    return inputSamples;
}

void AudioChannel::setAudioCallback(AudioChannel::AudioCallback audioCallback) {
    this->audioCallback = audioCallback;
}
