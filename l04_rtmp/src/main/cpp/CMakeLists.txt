
cmake_minimum_required(VERSION 3.4.1)
#file(GLOB SOURCE *.cpp)
add_subdirectory(librtmp)
add_library( # Sets the name of the library.
        rtmpplayer
        SHARED
        native-lib.cpp
        VideoChannel.h VideoChannel.cpp AudioChannel.cpp AudioChannel.h macro.h)
include_directories(include)
find_library( # Sets the name of the path variable.
        log-lib
        log)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/${ANDROID_ABI}")
target_link_libraries( # Specifies the target library.
        rtmpplayer
        rtmp
        ${log-lib}
        x264
        faac)