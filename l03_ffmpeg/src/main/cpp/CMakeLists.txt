
cmake_minimum_required(VERSION 3.4.1)
file(GLOB SOURCE *.cpp)
add_library( # Sets the name of the library.
        player
        SHARED
        ${SOURCE})
find_library( # Sets the name of the path variable.
        log-lib
        log)
include_directories(include)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/${ANDROID_ABI}")
target_link_libraries( # Specifies the target library.
        player
        avfilter avformat avcodec avutil swresample swscale
        ${log-lib}
        android
        z
        OpenSLES
        )