#include <jni.h>
#include <string>
#include <android/log.h>
#include <pthread.h>

// 定义一个宏  __VA_ARGS__ 代表... 可变参数
#define  LOGE(...) __android_log_print(ANDROID_LOG_ERROR,"MYJNI",__VA_ARGS__);
extern "C" JNIEXPORT jstring JNICALL
Java_com_example_ndk_1pro_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}
extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_ndk_1pro_MainActivity_test(JNIEnv *env, jobject thiz, jintArray i_,
                                            jobjectArray j_) {

//    C  调用：
//    (*env)->GetIntArrayElements(env,i_, NULL);

    //指向数组首元素地址
    // 第二个参数：指针：指向内存地址
    // 在这个地址存数据
    // true：是拷贝的一个新数据 (新申请内存)
    // false: 就是使用的java的数组 (地址)
    jint *i = env->GetIntArrayElements(i_, NULL);
    int32_t length = env->GetArrayLength(i_);
    for (int k = 0; k < length; ++k) {
        LOGE("获取Java的参数%d", *(i + k));
        *(i + k) = 100;
    }
    // 参数3：mode
    // 0 = 刷新java数组 并 释放c/c++数组
    // 1 = JNI_COMMIT:
    //      只刷新java数组
    // 2 = JNI_ABORT
    //      只释放
    env->ReleaseIntArrayElements(i_, i, 0);


    jint strLength = env->GetArrayLength(j_);
    for (int i = 0; i < strLength; ++i) {
        jstring str = static_cast<jstring>(env->GetObjectArrayElement(j_, i));
        //转成可操作的c/c++字符串
        const char *s = env->GetStringUTFChars(str, 0);
        LOGE("获取Java的参数%s", s);
        env->ReleaseStringUTFChars(str, s);
    }


}extern "C"
JNIEXPORT void JNICALL
Java_com_example_ndk_1pro_MainActivity_passObject(JNIEnv *env, jobject thiz, jobject bean,
                                                  jstring str_) {
    jclass beanCls = env->GetObjectClass(bean);
    jmethodID getI = env->GetMethodID(beanCls, "getI", "()I");    // 参数3： 签名
    //调用
    jint i = env->CallIntMethod(bean, getI);
    LOGE("C++ 调用Java getI方法:%d", i);

    jmethodID setI = env->GetMethodID(beanCls, "setI", "(I)V");    // 参数3： 签名
    env->CallVoidMethod(bean, setI, 200);

    //static 方法
    jmethodID printInfo = env->GetStaticMethodID(beanCls, "printInfo", "(Ljava/lang/String;)V");
    //创建java字符串
    jstring str2 = env->NewStringUTF("我是Bean类的静态方法，被C++调用");
    env->CallStaticVoidMethod(beanCls, printInfo, str2);
    //释放局部引用
    env->DeleteLocalRef(str2);

    jfieldID fieldI = env->GetFieldID(beanCls, "i", "I");
    env->SetIntField(bean, fieldI, 100);

    jmethodID printInfo2 = env->GetStaticMethodID(beanCls, "printInfo",
                                                  "(Lcom/example/ndk_pro/Bean2;)V");
    //在Jni创建java对象：
    jclass bean2Cls = env->FindClass("com/example/ndk_pro/Bean2");
    //1、获得类的构造方法
    jmethodID construct = env->GetMethodID(bean2Cls, "<init>", "(I)V");
    //2、创建对象
    jobject bean2 = env->NewObject(bean2Cls, construct, 88);
    env->CallStaticVoidMethod(beanCls, printInfo2, bean2);

//    释放
    const char *str = env->GetStringUTFChars(str_, 0);
    env->ReleaseStringUTFChars(str_, str); //get的release
    env->DeleteLocalRef(bean2Cls);//new （find）出来的delete
    env->DeleteLocalRef(bean2);

    /**
 * 在 JNI 规范中定义了三种引用：
 * 局部引用（Local Reference）、以上方法内的 都是局部引用 出了方法自动回收（在方法内回收delete）
 * 全局引用（Global Reference）、
 * 弱全局引用（Weak Global Reference）
 */
}
static jclass bean2Cls;
jobject bean2;
extern "C"
JNIEXPORT void JNICALL
Java_com_example_ndk_1pro_MainActivity_invokeBean2Method(JNIEnv *env, jobject thiz) {
    //局部引用
    //有问题 内部引用被释放
    //指针 指针有值，但是指向的地址数据被释放了  悬空指针
    if (bean2Cls == NULL) {
        bean2Cls = env->FindClass("com/example/ndk_pro/Bean2");
    }
    jmethodID constuct = env->GetMethodID(bean2Cls, "<init>", "(I)V");
    jobject bean2 = env->NewObject(bean2Cls, constuct, 99);
    //  ::bean2 会用外面的bean2
    ::bean2 = env->NewWeakGlobalRef(bean2);
    // 释放弱全局引用
    //env->DeleteWeakGlobalRef(bean2);

    //弱引用 ：不会阻止回收
    //问题： 当我们使用弱引用的时候  弱引用 引用的对象可能被回收了
    //使用的时候判断是否被回收了

    //对一个弱引用 与NULL相比较
    // true： 释放了
    // false: 还可以使用这个弱引用
    jboolean isEqual = env->IsSameObject(::bean2, NULL);

}
//全局引用可以跨方法、跨线程使用，直到它被手动释放才会失效 。
//
//由 NewGlobalRef 函数创建


//  extern： 在其他地方声明的  有头文件直接头文件，没有也可以这样引入
extern void test();

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ndk_1pro_MainActivity_invokeBean2Method2(JNIEnv *env, jobject thiz) {
    test();

    if (bean2Cls == NULL) {
        jclass cls = env->FindClass("com/example/ndk_pro/Bean2");
        //把它变成全局引用
        bean2Cls = static_cast<jclass>(env->NewGlobalRef(cls));
        //释放局部引用
        env->DeleteLocalRef(cls);
    }
    jmethodID constuct = env->GetMethodID(bean2Cls, "<init>", "(I)V");
    jobject bean2 = env->NewObject(bean2Cls, constuct, 99);
//释放全局引用
    //env->DeleteGlobalRef(bean2Cls);

    //与全局引用类似，弱引用可以跨方法、线程使用。与全局引用不同的是，弱引用不会阻止GC回收它所指向的VM内部的对象 。
    //
    //在对Class进行弱引用是非常合适（FindClass），因为Class一般直到程序进程结束才会卸载。
    //
    //在使用弱引用时，必须先检查缓存过的弱引用是指向活动的对象，还是指向一个已经被GC的对象
}









//--------JNI_OnLoad----------------动态注册----------------JNI_OnLoad--------
//Java_PACKAGENAME_CLASSNAME_METHODNAME 来进行与java方法的匹配，这种方式我们称之为静态注册。

//而动态注册则意味着方法名可以不用这么长了，在android aosp源码中就大量的使用了动态注册的形式
//优点是编译的方法符号会变小 被反编译也会更加模糊


/**
 * 调用System.loadLibrary()函数时， 内部就会去查找so中的 JNI_OnLoad 函数，如果存在此函数则调用。

JNI_OnLoad会：

告诉 VM 此 native 组件使用的 JNI 版本。

​	对应了Java版本，android中只支持JNI_VERSION_1_2 、JNI_VERSION_1_4、JNI_VERSION_1_6

​	在JDK1.8有 JNI_VERSION_1_8。
 */
void dynamicNative1() {//无參的方法可以空着也能链接到，而有參的方法如果不写（JNIEnv *env, jobject jobj,）后边的参数可能会被误认为 env
    LOGE("dynamicNative1 动态注册");
}
//void dynamicNative1(JNIEnv *env, jobject jobj) {
//    LOGE("dynamicNative1 动态注册");
//}

jstring dynamicNative2(JNIEnv *env, jobject jobj, jint i) {
    return env->NewStringUTF("我是动态注册的dynamicNative2方法");
}

//需要动态注册的方法数组
static const JNINativeMethod mMethods[] = {//java方法名、签名、本地方法
        {"dynamicNative", "()V",                   (void *) dynamicNative1},
        {"dynamicNative", "(I)Ljava/lang/String;", (jstring *) dynamicNative2}

};
//需要动态注册native方法的类名
static const char *mClassName = "com/example/ndk_pro/MainActivity";

JavaVM *_vm;

int JNI_OnLoad(JavaVM *vm, void *re) {
    __android_log_print(ANDROID_LOG_ERROR, "JNI", "JNI_Onload");
    //
    _vm = vm; //可以用来附加native线程到虚拟机
    // 获得JNIEnv
//    JNIEnv *env = 0;//表示指针还没有初始化
    JNIEnv *env = nullptr;//表示指针还没有初始化
    // 小于0 失败 ，等于0 成功
    int r = vm->GetEnv((void **) &env, JNI_VERSION_1_6);
    if (r != JNI_OK) {
        return -1;
    }
    //获得 class对象
    jclass jcls = env->FindClass(mClassName);
    //注册
    env->RegisterNatives(jcls, mMethods, sizeof(mMethods) / sizeof(JNINativeMethod));
    return JNI_VERSION_1_6;
}


/****************************线程操作********************/
//native调用java需要使用JNIEnv这个结构体，而JNIEnv是由Jvm传入与线程相关的变量。

//但是可以通过JavaVM的AttachCurrentThread方法来获取到当前线程中的JNIEnv指针。

struct Context {//在C++中结构体（默认public）和类（默认private）只有这点区别
    jobject instance;
//    JNIEnv *env;
};

void *threadTask(void *args) {
    // native线程 附加 到 Java 虚拟机
    JNIEnv *env;
    jint i = _vm->AttachCurrentThread(&env, 0);
    if (i != JNI_OK) {
        return 0;
    }
    Context *context = static_cast<Context *>(args);
    jclass cls = env->GetObjectClass(context->instance);
    jmethodID jmethodId = env->GetMethodID(cls, "updateUI", "()V");
    env->CallVoidMethod(context->instance, jmethodId);


    delete (context);
    context = 0;
    //分离
    _vm->DetachCurrentThread();

    return 0;
}


extern "C"
JNIEXPORT void JNICALL
Java_com_example_ndk_1pro_MainActivity_testThread(JNIEnv *env, jobject thiz) {
    pthread_t pid;//线程（id）句柄
    Context *context = new Context;
    context->instance = env->NewGlobalRef(thiz);
//    context->env=env; 此处的env 属于Java调用地方所在的线程，不能跨线程使用
    pthread_create(&pid, 0, threadTask, context);

}extern "C"
JNIEXPORT void JNICALL
Java_com_example_ndk_1pro_MainActivity_testList(JNIEnv *env, jobject thiz, jobject list) {
    jclass j_list = env->GetObjectClass(list);
    jmethodID size = env->GetMethodID(j_list, "size", "()I");
    jmethodID get = env->GetMethodID(j_list, "get", "(I)Ljava/lang/Object;");
    jmethodID set = env->GetMethodID(j_list, "set", "(ILjava/lang/Object;)Ljava/lang/Object;");
    jint length = env->CallIntMethod(list, size);
    for (int i = 0; i < length; ++i) {
        jobject bean = env->CallObjectMethod(list, get, i);//string
//        jclass beanCls = env->GetObjectClass(bean);
//        jmethodID getI = env->GetMethodID(beanCls, "getI", "()I");
//        jint ii = env->CallIntMethod(bean, getI);
//        LOGE("%d",ii)
        jstring jstring1= static_cast<jstring>(bean);
        const char *string = env->GetStringUTFChars(jstring1,0);
        LOGE("%s",string)
    }

}