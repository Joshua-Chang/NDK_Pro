//
// Created by JoshuaChang on 2020/2/28.
//

#include "JavaCallHelper.h"
#include "macro.h"


JavaCallHelper::~JavaCallHelper() {

}

JavaCallHelper::JavaCallHelper(JavaVM *_javaVM, JNIEnv *_env, jobject &_jobj) : javaVm(_javaVM),
                                                                                env(_env) {
    jobj = env->NewGlobalRef(_jobj);//实例化全局对象，防止方法调用后失去内存
//    this->javaVm=_javaVM;上方冒号赋值一样
    jclass jclazz = env->GetObjectClass(jobj);
//    jmid_error ArtMethod结构体
    jmid_error = env->GetMethodID(jclazz, "onError", "(I)V");
    jmid_prepare = env->GetMethodID(jclazz, "onPrepare", "()V");
    jmid_progress = env->GetMethodID(jclazz, "onProgress", "(I)V");

}

void JavaCallHelper::onError(int thread, int code) {
    if (thread==THREAD_CHILD){
        JNIEnv *jniEnv;
        if (javaVm->AttachCurrentThread(&jniEnv,0)!=JNI_OK){//线程绑定失败
            return;
        }
        jniEnv->CallVoidMethod(jobj,jmid_error,code);
        javaVm->DetachCurrentThread();
    } else{
        env->CallVoidMethod(jobj,jmid_error,code);
    }
}

void JavaCallHelper::onPrepare(int thread) {
    if (thread==THREAD_CHILD){
        JNIEnv *jniEnv;
        if (javaVm->AttachCurrentThread(&jniEnv,0)!=JNI_OK){//线程绑定失败
            return;
        }
        jniEnv->CallVoidMethod(jobj,jmid_prepare);
        javaVm->DetachCurrentThread();
    } else{
        env->CallVoidMethod(jobj,jmid_prepare);
    }
}

void JavaCallHelper::onProgress(int thread, int progress) {
    if (thread==THREAD_CHILD){
        JNIEnv *jniEnv;
        if (javaVm->AttachCurrentThread(&jniEnv,0)!=JNI_OK){//线程绑定失败
            return;
        }
        jniEnv->CallVoidMethod(jobj,jmid_progress,progress);
        javaVm->DetachCurrentThread();
    } else{
        env->CallVoidMethod(jobj,jmid_progress,progress);
    }
}
