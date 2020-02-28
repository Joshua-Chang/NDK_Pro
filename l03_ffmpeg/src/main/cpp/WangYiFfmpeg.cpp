//
// Created by JoshuaChang on 2020/2/28.
//

#include "WangYiFfmpeg.h"
#include "macro.h"

void *prepareFFmpeg_(void *args) {
    WangYiFfmpeg *wangYiFfmpeg = static_cast<WangYiFfmpeg *>(args);
    wangYiFfmpeg->prepareFFmpeg();
    return 0;
}

WangYiFfmpeg::WangYiFfmpeg(JavaCallHelper *javaCallHelper, const char *dataSource) : javaCallHelper(
        javaCallHelper) {
    url = new char[strlen(dataSource) + 1];
//    构造方法在native-lib使用过后 内存会被释放 所以另拷贝一下
    strcpy(url, dataSource);

}

WangYiFfmpeg::~WangYiFfmpeg() {

}

void WangYiFfmpeg::prepare() {
    pthread_create(&pid_prepare, NULL, prepareFFmpeg_, this);//在子线程初始化ffmpeg
}

void WangYiFfmpeg::prepareFFmpeg() {
    /**
     * 子线程执行，但可以访问到对象的属性
     */
    avformat_network_init();
    avformat_network_init();
    //todo 1、初始化总上下文AVFormatContext
    formatContext = avformat_alloc_context();
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
    int ret = avformat_open_input(&formatContext, url, NULL, &opts);//打开视频：需要参数四
    if (ret) {//=0成功
        javaCallHelper->onError(THREAD_CHILD, FFMPEG_CANNOT_OPEN_URL);
        return;
    }
    //查找流
    if (avformat_find_stream_info(formatContext, NULL) < 0) {
        if (javaCallHelper) {
            javaCallHelper->onError(THREAD_CHILD, FFMPEG_CANNOT_FIND_STREAM);
        }
        return;
    }

    for (int i = 0; i < formatContext->nb_streams; ++i) {
        //编码参数--》编码类型
        AVCodecParameters *codecpar = formatContext->streams[i]->codecpar;
        AVCodec *dec = avcodec_find_decoder(codecpar->codec_id);//编码器
        if (!dec) {
            if (javaCallHelper) {
                javaCallHelper->onError(THREAD_CHILD, FFMPEG_FIND_DECODER_FAIL);
            }
            return;
        }
        AVCodecContext *codecContext = avcodec_alloc_context3(dec);//创建解码器上下文
        if (!codecContext) {
            if (javaCallHelper) {
                javaCallHelper->onError(THREAD_CHILD, FFMPEG_ALLOC_CODEC_CONTEXT_FAIL);
            }
            return;
        }
        //编码参数copy到解码器上下文
        if (avcodec_parameters_to_context(codecContext, codecpar) < 0) {
            if (javaCallHelper) {
                javaCallHelper->onError(THREAD_CHILD, FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL);
            }
            return;
        }
        //打开解码器
        if (avcodec_open2(codecContext, dec, NULL) != 0) {
            if (javaCallHelper) {
                javaCallHelper->onError(THREAD_CHILD, FFMPEG_OPEN_DECODER_FAIL);
            }
            return;
        }

        if (codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioChannel=new AudioChannel(i,javaCallHelper,codecContext);

        } else if (codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoChannel=new VideoChannel(i,javaCallHelper,codecContext);
            videoChannel->setRenderFrame(renderFrame);
        }
    }
    if (!audioChannel&&!videoChannel){
        if (javaCallHelper){
            javaCallHelper->onError(THREAD_CHILD,FFMPEG_NOMEDIA);
        }
        return;
    }
    if (javaCallHelper){
        javaCallHelper->onPrepare(THREAD_CHILD);
    }

}

void *startThread(void *args){
    WangYiFfmpeg *ffmpeg= static_cast<WangYiFfmpeg *>(args);
    ffmpeg->play();
    return 0;
}


void WangYiFfmpeg::start() {
    isPlaying= true;
//    if (audioChannel){audioChannel->play();}
    if (videoChannel){videoChannel->play();}
    pthread_create(&pid_play,NULL,startThread,this);
}

void WangYiFfmpeg::play() {
    int ret=0;
    while (isPlaying){
        if (audioChannel&&audioChannel->pkt_queue.size()>100){//生产速度大小消费速度
            av_usleep(1000*10);//队列满了 延缓生产速度 休眠10毫秒
            continue;
        }
        if (videoChannel&&videoChannel->pkt_queue.size()>100){//生产速度大小消费速度
            av_usleep(1000*10);//队列满了 延缓生产速度 休眠10毫秒
            continue;
        }

        AVPacket *packet = av_packet_alloc();
        ret=av_read_frame(formatContext,packet);

        if (ret==0){
            if (audioChannel&&packet->stream_index==audioChannel->channelId){
                audioChannel->pkt_queue.put(packet);
            } else if(videoChannel&&packet->stream_index==videoChannel->channelId){
                videoChannel->pkt_queue.put(packet);
            }
        } else if (ret==AVERROR_EOF){
            if (videoChannel->pkt_queue.empty()&&videoChannel->frame_queue.empty()&&
            audioChannel->pkt_queue.empty()&&audioChannel->frame_queue.empty()){
                LOGE("播放完毕");
                break;
            }
        } else{
            break;
        }
    }
    isPlaying=0;
    audioChannel->stop();
    videoChannel->stop();
}

void WangYiFfmpeg::setRenderCallBack(RenderFrame renderFrame) {
    this->renderFrame=renderFrame;

}
