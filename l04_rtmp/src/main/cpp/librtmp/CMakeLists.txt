
cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNO_CRYPTO")
#hashswf.c里关闭CRYPTO（NO_CRYPTO）-D：加一个参数
add_library(
        rtmp
        STATIC
        amf.c
        hashswf.c
        log.c
        parseurl.c
        rtmp.c
)