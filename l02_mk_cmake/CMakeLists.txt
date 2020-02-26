# cmake脚本
#​	CMake是一个跨平台的构建工具，Cmake 并不直接建构出最终的软件，而是产生其他工具的脚本（如Makefile ），
#   然后再依这个工具的构建方式使用。
# 指定cmake最小支持版本的方法
# CMake是一个比make更高级的编译配置工具，它可以根据不同平台、不同的编译器，
# 生成相应的Makefile或者vcproj项目。从而达到跨平台的目的。
# Android Studio利用CMake生成的是ninja，ninja是一个小型的关注速度的构建系统。
# 我们不需要关心ninja的脚本，知道怎么配置cmake就可以了。
# 从而可以看出cmake其实是一个跨平台的支持产出各种不同的构建脚本的一个工具。


cmake_minimum_required(VERSION 3.6)
#===============================不引入==========================================================================
# hello-jni： 变量名 最终会生成的so名字
# SHARED: 动态库  STATIC:静态库
#源文件
#add_library(hello-jni SHARED src/main/cmake/hello-jni.c)
add_library(hello-jni SHARED src/main/cmake/hello-jni.c)

#===============================引入静态库==========================================================================
#引入静态库

# IMPORTED: 表示我们这一个静态库是以导入的形式添加进来(预编译静态库)

#add_library(Test STATIC IMPORTED)

#怎么导入？ 设置导入的路径
#设置目标属性方法
#设置Test的 导入路径 属性

# CMAKE_SOURCE_DIR: 当前cmakelists.txt的路径 （cmake工具内置的）
# android cmake 内置的 ANDROID_ABI :  当前需要编译的cpu架构
# ANDROID_ABI 如果需要编译出多个平台的 so的时候

#message("ANDROID_ABI : ${ANDROID_ABI}")
#set_target_properties(Test PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/src/main/cmake/${ANDROID_ABI}/libTest.a)

# 链接   (1、不能换位置 2、add_library的名字一样)
# 第一个位置：要生成的目标 so
# 后边的位置：编译hello-jni模块 需要链接(依赖) Test 模块

#target_link_libraries(hello-jni Test  log)


#===============================引入动态库==========================================================================

add_library(Test SHARED IMPORTED)
#引入动态库必须在jniLibs下
# 有问题！！！
#set_target_properties(Test PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libTest.so)
#和makefile不同，cmake 有其他办法引入动态库 让他没问题

# c++: CMAKE_CXX_FLAGS
# set 方法 定义一个变量
#CMAKE_C_FLAGS = "${CMAKE_C_FLAGS} XXXX"
# -L: 库的查找路径 libTest.so
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -L${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI} ")
#查找 log 库
#如果查找到了 就会把 liblog.so 的绝对路径 赋值给 log-lib
#从哪里去找？
#可以不写 直接在 target_link_libraries 写上log
#find_library(a log)
target_link_libraries(hello-jni Test)
