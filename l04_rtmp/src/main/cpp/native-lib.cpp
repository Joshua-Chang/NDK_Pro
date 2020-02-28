#include <jni.h>
#include <string>
#include "x264.h"
#include "librtmp/rtmp.h"
#include "VideoChannel.h"
#include "macro.h"
#include "safe_queue.h"
#include "AudioChannel.h"
int isStart = 0;//为了防止用户重复点击开始直播，导致重新初始化  顶一个start变量

VideoChannel *videoChannel;
AudioChannel *audioChannel;
pthread_t pid;

uint32_t start_time;
int readyPushing;
//队列
SafeQueue<RTMPPacket *> packets;

void callback(RTMPPacket *packet) {

    if (packet) {

        //设置时间戳
        packet->m_nTimeStamp = RTMP_GetTime() - start_time;
//        加入队列
        packets.put(packet);
    }
}

void releasePackets(RTMPPacket *&packet) {
    if (packet) {
        RTMPPacket_Free(packet);
        delete (packet);
        packet = 0;
    }
}

void *start(void *args) {
/**
 * todo 二、1、线程方法
 * 初始化rmtp 初始化队列
 * 发送packet、释放packet
 */
    char *url = static_cast<char *>(args);
    RTMP *rtmp = 0;
    rtmp = RTMP_Alloc();
    if (!rtmp) {
        LOGE("alloc rtmp失败");
        return NULL;
    }
    RTMP_Init(rtmp);
    int ret = RTMP_SetupURL(rtmp, url);
    if (!ret) {
        LOGE("设置地址失败:%s", url);
        return NULL;
    }
    rtmp->Link.timeout = 5;//连接时间
    RTMP_EnableWrite(rtmp);
    ret = RTMP_Connect1(rtmp, 0);//内部是socket长连接
    if (!ret) {
        LOGE("连接服务器:%s,%d", url,ret);
        return NULL;
    }
    ret = RTMP_ConnectStream(rtmp, 0);
    if (!ret) {
        LOGE("连接流:%s", url);
        return NULL;
    }
    start_time = RTMP_GetTime();
    //表示可以开始推流了
    readyPushing = 1;
    packets.setWork(1);//队列初始化
    RTMPPacket *packet = 0;
    /**
     * 将音频首帧空数据参数帧放入队列--》保证服务端接收到的第一帧就是
     */
    callback(audioChannel->setAudioTag());
    while (readyPushing) {//循环发送
        packets.get(packet);//packet取值
        LOGE("取出一帧数据");
        if (!readyPushing) {
            break;
        }
        if (!packet) {
            continue;
        }
        packet->m_nInfoField2 = rtmp->m_stream_id;//设置packet类型
        ret = RTMP_SendPacket(rtmp, packet, 1);//发送packet
        releasePackets(packet);//释放packet
    }
//    发送完复原
    isStart = 0;
    readyPushing = 0;
    packets.setWork(0);//队列清空
    packets.clear();
    if (rtmp) {//关闭rtmp
        RTMP_Close(rtmp);
        RTMP_Free(rtmp);
    }
    delete (url);
    return 0;

}



//#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,">>>>>>",FORMAT,##__VA_ARGS__);

extern "C"
JNIEXPORT void JNICALL
Java_com_example_l04_1rtmp_LivePusher_native_1setVideoEncInfo(JNIEnv *env, jobject thiz, jint width,
                                                              jint height, jint fps, jint bitrate) {
    if (!videoChannel) {
        return;
    }
    videoChannel->setVideoEncInfo(width, height, fps, bitrate);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_l04_1rtmp_LivePusher_native_1start(JNIEnv *env, jobject thiz, jstring path_) {
    const char *path = env->GetStringUTFChars(path_, 0);

    /**
     * todo 二、开线程 队列
     */
    if (isStart) {
        return;
    }
    isStart = 1;

    char *url = new char[strlen(path) + 1];//变量保存
    strcpy(url, path);
    pthread_create(&pid, 0, start, url);//开启线程
    env->ReleaseStringUTFChars(path_, path);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_l04_1rtmp_LivePusher_native_1pushVideo(JNIEnv *env, jobject thiz,
                                                        jbyteArray data_) {
    /**
     * todo 三、将nv21转换为i420 并推流
     */
    if (!videoChannel || !readyPushing) {
        return;
    }
    jbyte *data = env->GetByteArrayElements(data_, NULL);

    videoChannel->encodeData(data);//转换
    // TODO

    env->ReleaseByteArrayElements(data_, data, 0);
}extern "C"
JNIEXPORT void JNICALL
Java_com_example_l04_1rtmp_LivePusher_native_1init(JNIEnv *env, jobject thiz) {
    videoChannel = new VideoChannel;
    videoChannel->setVideoCallback(callback);
    audioChannel = new AudioChannel;
    audioChannel->setAudioCallback(callback);
}extern "C"
JNIEXPORT void JNICALL
Java_com_example_l04_1rtmp_LivePusher_native_1pushAideo(JNIEnv *env, jobject thiz,
                                                        jbyteArray bytes_) {
    /**
     * 音频从pcm 原始数据编码成aac要使用faac
     * 视频yuv h264 x264
     */
    jbyte *data = env->GetByteArrayElements(bytes_, NULL);
    if (!audioChannel||!readyPushing){
        return;
    }
    audioChannel->encodeData(data);//转换

    env->ReleaseByteArrayElements(bytes_, data, 0);
}extern "C"





/*----------------------------------------------------------------------------audio----------------------------------------------------------------------------*/
JNIEXPORT void JNICALL
Java_com_example_l04_1rtmp_LivePusher_native_1setAudioEncInfo(JNIEnv *env, jobject thiz,
                                                              jint samples_in_hz, jint channels) {
    if (!audioChannel) {
        return;
    }
    audioChannel->setAudioEncInfo(samples_in_hz, channels);
}extern "C"
JNIEXPORT jint JNICALL
Java_com_example_l04_1rtmp_LivePusher_getInputSamples(JNIEnv *env, jobject thiz) {
    if (audioChannel) {
        return audioChannel->getInputSamples();
    }
    return -1;
}