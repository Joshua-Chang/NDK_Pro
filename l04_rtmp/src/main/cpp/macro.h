//
// Created by JoshuaChang on 2020/2/27.
//

#ifndef NDK_PRO_MACRO_H
#define NDK_PRO_MACRO_H

#include <android/log.h>
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,">>>",__VA_ARGS__)
//宏函数
#define DELETE(obj) if(obj){ delete obj; obj = 0; }

#endif //NDK_PRO_MACRO_H
