//
// Created by JoshuaChang on 2020/2/27.
//

#ifndef NDK_PRO_MACRO_H
#define NDK_PRO_MACRO_H

#include <android/log.h>
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,">>>",__VA_ARGS__)
//宏函数
#define DELETE(obj) if(obj){ delete obj; obj = 0; }

//标记线程
#define THREAD_MAIN 1
#define THREAD_CHILD 2

//错误代码
#define FFMPEG_CANNOT_OPEN_URL 1
#define FFMPEG_CANNOT_FIND_STREAM 2
#define FFMPEG_FIND_DECODER_FAIL 3
#define FFMPEG_ALLOC_CODEC_CONTEXT_FAIL 4
#define FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL 6
#define FFMPEG_NOMEDIA 7
#define FFMPEG_OPEN_DECODER_FAIL 8

#endif //NDK_PRO_MACRO_H
