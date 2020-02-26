#include <jni.h>

//
// Created by JoshuaChang on 2020/2/23.
//
extern int test();

JNIEXPORT void JNICALL
Java_com_example_l02_1mk_1cmake_MainActivity_nativeTest(JNIEnv
* env,
jobject thiz
) {
test();

}