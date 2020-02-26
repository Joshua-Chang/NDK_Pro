LOCAL_PATH := $(call my-dir)

#预编译库的引入 (提前编译好的库)
include $(CLEAR_VARS)
LOCAL_MODULE := Test
LOCAL_SRC_FILES := libTest.a
#引入动态库
include $(PREBUILT_STATIC_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := hello-jni
LOCAL_SRC_FILES := hello-jni.c
# 编译hello-jni模块 需要链接 Test 模块
# Test模块是一个预编译库模块
LOCAL_STATIC_LIBRARIES := Test
#生成一个动态库
include $(BUILD_SHARED_LIBRARY)
