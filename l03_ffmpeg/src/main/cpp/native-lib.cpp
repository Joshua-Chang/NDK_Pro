#include <jni.h>
#include <string>
#include <android/native_window_jni.h>
#include <zconf.h>
#include <android/log.h>

#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,">>>>>>",FORMAT,##__VA_ARGS__);

extern "C" {
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
//重采样
#include "libswresample/swresample.h"
}
extern "C" JNIEXPORT jstring JNICALL
Java_com_example_l03_1ffmpeg_Player_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(av_version_info());
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_l03_1ffmpeg_Player_native_1start(JNIEnv *env, jobject thiz, jstring path_,
                                                    jobject surface) {
    const char *path = env->GetStringUTFChars(path_, 0);


    avformat_network_init();
    //todo 1、初始化总上下文AVFormatContext
    AVFormatContext *formatContext = avformat_alloc_context();
    AVDictionary *opts = NULL;
    av_dict_set(&opts, "timeout", "3000000", 0);//设置超时3秒
    /**
     * todo 2、打开视频
     * 1、format上下文
     * 2、路径或url地址
     * 3、inputformat 传null为视频原始的
     * 4、AVDictionary字典：配置打开设置，通过av_dict_set初始化（二级指针 内部初始化）
     * 返回值：ffmpeg中通用 0为成功，非0失败
     */
    int ret = avformat_open_input(&formatContext, path, NULL, &opts);//打开视频：需要参数四
    if (ret) {
        return;
    }
    //todo 3、寻找视频流
    /**
     * 1、通知ffmpeg将流解析出来
     * 2、遍历找到视频流的索引
     */
    int vidio_stream_idx = -1;
    avformat_find_stream_info(formatContext, NULL);//通知ffmpeg将流解析出来
    for (int i = 0; i < formatContext->nb_streams; ++i) {
        //编码参数--》编码类型
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            vidio_stream_idx = i;
            break;
        }
    }
    /**
     * todo 4、根据视频流编码参数创建绑定解码器上下文并打开解码器
     * 1、根据视频流取得相应的编码参数
     * 2、找到解码器
     * 3、使用解码器创建解码器上下文
     * 4、解码器参数copy到解码器上下文
     * 5、打开解码器
     */
    AVCodecParameters *codecpar = formatContext->streams[vidio_stream_idx]->codecpar;
    AVCodec *dec = avcodec_find_decoder(codecpar->codec_id);
    AVCodecContext *codecContext = avcodec_alloc_context3(dec);//创建解码器上下文
    avcodec_parameters_to_context(codecContext, codecpar);//编码参数copy到解码器上下文
    avcodec_open2(codecContext, dec, NULL);


    /**
     * todo 6.1 获得转换上下文
     */
    SwsContext *swsContext = sws_getContext(//AVPixelFormat
            codecContext->width, codecContext->height, codecContext->pix_fmt,//输入
            codecContext->width, codecContext->height, AV_PIX_FMT_RGBA,//输出
            SWS_BILINEAR,//重视速度
            0, 0, 0);



    //todo 7-1、根据surface创建window     （空白的使用getheight/getwidth）
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);
//    通过视频缓冲区绘制
    ANativeWindow_Buffer outBuffer;
    //设置缓冲区大小
    ANativeWindow_setBuffersGeometry(nativeWindow, codecContext->width,
                                     codecContext->height,
                                     WINDOW_FORMAT_RGBA_8888);



    /**
     * todo 5、开始解码 在解码队列 循环读取packet 发送packet 接收frame
     * 1、实例化Packet对象
     * 2、循环从视频流中读取Packet 赋值给创建的对象  av_read_frame
     * 3、发送packet avcodec_send_packet
     * 4、接收frame avcodec_receive_frame
     */
    AVPacket *packet = av_packet_alloc();
    while (av_read_frame(formatContext, packet) >= 0) {//循环读取
        avcodec_send_packet(codecContext, packet);
        AVFrame *frame = av_frame_alloc();
        ret = avcodec_receive_frame(codecContext, frame);
        if (ret == AVERROR(EAGAIN)) {
            //需要更多数据
            continue;
        } else if (ret < 0) {
            break;
        }


        /**
        * todo 6、绘制准备（frame）YUV-》（image）RGB
        * 1、获得转换上下文
        * 2、yuv的frame->data---》rgb的dst_data
        */
        uint8_t *dst_data[4];//接收容器
        int dst_linesize[4];//每一行的首地址
        //方法内部赋值dst_data
        av_image_alloc(dst_data, dst_linesize, codecContext->width, codecContext->height,
                       AV_PIX_FMT_RGBA, 1);//左对齐

        sws_scale(swsContext,//转换context
                  frame->data, frame->linesize, 0, frame->height,//需要转换的
                  dst_data, dst_linesize);//转换后的容器和首地址


        /**
         * todo 7、渲染 转换后的rgb的dst_data
         * 底层的渲染只有一个途径ANativeWindow  不停的将数据内存拷贝到缓冲区绘制
         */
        //上锁
        ANativeWindow_lock(nativeWindow, &outBuffer, NULL);//参数三：限制
        //渲染
        //rgb_frame是有画面数据
        uint8_t *dst = (uint8_t *) outBuffer.bits;
//            拿到一行有多少个字节 RGBA
        int destStride = outBuffer.stride * 4;
        uint8_t *src_data = dst_data[0];
        int src_linesize = dst_linesize[0];
        uint8_t *firstWindown = static_cast<uint8_t *>(outBuffer.bits);
        for (int i = 0; i < outBuffer.height; ++i) {
            //一行一行的内存拷贝 dst_data--->
            memcpy(firstWindown + i * destStride, src_data + i * src_linesize, destStride);
        }

        ANativeWindow_unlockAndPost(nativeWindow);
        usleep(1000 * 16);
        av_frame_free(&frame);
    }


    env->ReleaseStringUTFChars(path_, path);

}
#define MAX_AUDIO_FRME_SIZE 48000 * 4
extern "C"
JNIEXPORT void JNICALL
Java_com_example_l03_1ffmpeg_Player_native_sound(JNIEnv *env, jobject thiz, jstring input_,
                                            jstring output_) {
    const char *input = env->GetStringUTFChars(input_, 0);
    const char *output = env->GetStringUTFChars(output_, 0);
    avformat_network_init();
    //todo 1、初始化总上下文AVFormatContext
    AVFormatContext *formatContext = avformat_alloc_context();
    /**
     * todo 2、打开音频
     * 1、format上下文
     * 2、路径或url地址
     * 3、inputformat 传null为视频原始的
     * 4、AVDictionary字典：配置打开设置，通过av_dict_set初始化（二级指针 内部初始化）
     * 返回值：ffmpeg中通用 0为成功，非0失败
     */
    int ret = avformat_open_input(&formatContext, input, NULL, NULL);//打开视频：需要参数四
    if (ret) {
        LOGE("%s", "无法获取输入文件信息");
        return;
    }

    //todo 3、寻找音频流
    /**
     * 1、通知ffmpeg将流解析出来
     * 2、遍历找到视频流的索引
     */
    int audio_stream_idx = -1;
    avformat_find_stream_info(formatContext, NULL);//通知ffmpeg将流解析出来
    for (int i = 0; i < formatContext->nb_streams; ++i) {
        //编码参数--》编码类型
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audio_stream_idx = i;
            break;
        }
    }
    /**
     * todo 4、根据视频流编码参数创建绑定解码器上下文并打开解码器
     * 1、根据视频流取得相应的编码参数
     * 2、找到解码器
     * 3、使用解码器创建解码器上下文
     * 4、解码器参数copy到解码器上下文
     * 5、打开解码器
     */
    AVCodecParameters *codecpar = formatContext->streams[audio_stream_idx]->codecpar;
    AVCodec *dec = avcodec_find_decoder(codecpar->codec_id);
    AVCodecContext *codecContext = avcodec_alloc_context3(dec);//创建解码器上下文
    avcodec_parameters_to_context(codecContext, codecpar);//编码参数copy到解码器上下文
    avcodec_open2(codecContext, dec, NULL);




    //输入参数不定
    AVSampleFormat in_format = codecContext->sample_fmt;//    输入的这些参数
    int in_sample_rate = codecContext->sample_rate;// 输入采样率
    uint64_t in_ch_layout = codecContext->channel_layout;//    输入声道布局

    //输出参数固定
    AVSampleFormat out_sample = AV_SAMPLE_FMT_S16;//    输出采样位数
    int out_sample_rate = 44100;
    uint64_t out_ch_layout = AV_CH_LAYOUT_STEREO;

    /**
    * todo 6.1 获得转换上下文并配置 SwrContext用于音频
    */
    SwrContext *swrContext = swr_alloc();
    //设置转换器 的输入参数 和输出参数
    swr_alloc_set_opts(swrContext,
                       out_ch_layout, out_sample, out_sample_rate,//输出
                       in_ch_layout, in_format, in_sample_rate,//输入
                       0, NULL
    );
    //    初始化转换器其他的默认参数
    swr_init(swrContext);
    //初始化缓冲区
    uint8_t *out_buffer = (uint8_t *) (av_malloc(2 * 44100));//通道数*采样频率

    FILE *fp_pcm = fopen(output, "wb");
    //读取包  压缩数据


    /**
     * todo 5、开始解码 循环读取packet 发送packet 接收frame
     * 1、实例化Packet对象
     * 2、循环从视频流中读取Packet 赋值给创建的对象  av_read_frame
     * 3、发送packet avcodec_send_packet
     * 4、接收frame avcodec_receive_frame
     */
    AVPacket *packet = av_packet_alloc();//压缩数据
    int count = 0;
    while (av_read_frame(formatContext, packet) >= 0) {//循环读取
        avcodec_send_packet(codecContext, packet);
        AVFrame *frame = av_frame_alloc();//解压数据
        ret = avcodec_receive_frame(codecContext, frame);
        if (ret == AVERROR(EAGAIN)) {
            //需要更多数据
            continue;
        } else if (ret < 0) {
            break;
        }
        if (packet->stream_index != audio_stream_idx) {
            continue;
        }
        LOGE("正在解码%d", count++);


        /**
        * todo 6.2 转换、对齐、输出
        */
        swr_convert(swrContext,
                    &out_buffer, 2 * 44100,
                    (const uint8_t **) frame->data, frame->nb_samples
        );
        //声道数也可以直接写2
        int out_channerl_nb = av_get_channel_layout_nb_channels(out_ch_layout);
        //缓冲区的 大小 最后一个参数 1对齐 0默认不对齐
        int out_buffer_size = av_samples_get_buffer_size(NULL, out_channerl_nb, frame->nb_samples,
                                                         out_sample, 1);
        fwrite(out_buffer, 1,out_buffer_size,fp_pcm);//写入文件，1表示文件的最小单元是几个（音频流字节 最小1）（像素字节 最小4）
    }
    fclose(fp_pcm);
    av_free(out_buffer);
    swr_free(&swrContext);
    avcodec_close(codecContext);
    avformat_close_input(&formatContext);
    env->ReleaseStringUTFChars(input_, input);
    env->ReleaseStringUTFChars(output_, output);
}