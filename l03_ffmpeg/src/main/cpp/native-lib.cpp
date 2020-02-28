#include <jni.h>
#include <string>
#include <android/native_window_jni.h>
//#include <zconf.h>
#include <android/log.h>
#include "WangYiFfmpeg.h"
#include "JavaCallHelper.h"

#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,">>>>>>",FORMAT,##__VA_ARGS__);

extern "C" {
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
//重采样
#include "libswresample/swresample.h"
}
ANativeWindow *window;
WangYiFfmpeg *wangYiFfmpeg;
JavaCallHelper *javaCallHelper;

//把native线程绑定到jvm实例
JavaVM *javaVm = NULL;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    javaVm = vm;
    return JNI_VERSION_1_4;
}

void renderFrame(uint8_t *data, int linesize, int w, int h) {
//渲染

//    通过视频缓冲区绘制
    ANativeWindow_Buffer outBuffer;
    //设置缓冲区大小
    ANativeWindow_setBuffersGeometry(window, w, h,
                                     WINDOW_FORMAT_RGBA_8888);
    /* todo 7、渲染 转换后的rgb的dst_data
    * 底层的渲染只有一个途径ANativeWindow  不停的将数据内存拷贝到缓冲区绘制
    */
    //上锁
    if (ANativeWindow_lock(window, &outBuffer, NULL))//参数三：限制
    {
        ANativeWindow_release(window);
        window = 0;
        return;

    }
    //渲染
    //rgb_frame是有画面数据
    uint8_t *window_data = (uint8_t *) outBuffer.bits;
//            拿到一行有多少个字节 RGBA
    int window_linesize = outBuffer.stride * 4;//stride行像素*4（argb）
    uint8_t *src_data = data;//数据源
    int src_linesize = linesize;
    for (int i = 0; i < outBuffer.height; ++i) {
        //一行一行的内存拷贝 dst_data--->
//        memcpy(window_data,src_data,window_linesize);//从src——data 拷贝到window—data 每行 按目的地行大小为准
//        指针指向的第一行首地址 每拷贝一次都要偏移下一行
        memcpy(window_data+i*window_linesize,src_data+i*src_linesize,window_linesize);
    }

    ANativeWindow_unlockAndPost(nativeWindow);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_example_l03_1ffmpeg_player_Player_native_1prepare(JNIEnv *env, jobject thiz,
                                                           jstring data_source_) {
    const char *dataSource = env->GetStringUTFChars(data_source_, 0);
    javaCallHelper = new JavaCallHelper(javaVm, env, thiz);
    wangYiFfmpeg = new WangYiFfmpeg(javaCallHelper, dataSource);
    // TODO: 1初始化配置
    wangYiFfmpeg->setRenderCallBack(renderFrame);
    wangYiFfmpeg->prepare();
    env->ReleaseStringUTFChars(data_source_, dataSource);

}extern "C"
JNIEXPORT void JNICALL
Java_com_example_l03_1ffmpeg_player_Player_native_1start(JNIEnv *env, jobject thiz) {
    // TODO: 2开始播放
    if (wangYiFfmpeg) {
        wangYiFfmpeg->start();
    }
}extern "C"
JNIEXPORT void JNICALL
Java_com_example_l03_1ffmpeg_player_Player_native_1set_1surface(JNIEnv *env, jobject thiz,
                                                                jobject surface) {
    if (window) {
        ANativeWindow_release(window);
        window = 0;
    }
    window = ANativeWindow_fromSurface(env, surface);
}