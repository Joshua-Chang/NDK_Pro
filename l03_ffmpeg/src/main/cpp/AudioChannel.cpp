//
// Created by JoshuaChang on 2020/2/28.
//

#include "AudioChannel.h"
#include <SLES/OpenSLES_Android.h>

extern "C" {
#include <libavutil/time.h>
}

AudioChannel::AudioChannel(int id, JavaCallHelper *javaCallHelper, AVCodecContext *avCodecContext,
                           AVRational time_base)
        : BaseChannel(id, javaCallHelper, avCodecContext,time_base) {
    out_channels=av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    out_samplesize=av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
    out_sample_rate=44100;
    //out_sample_rate*out_samplesize 16位 2个字节 out_channels 2
    out_buffer= (uint8_t*)malloc(out_sample_rate * out_samplesize * out_channels);
}

void bqPlayerCallBack(SLAndroidSimpleBufferQueueItf bq, void *context) {
    AudioChannel *audioChannel = static_cast<AudioChannel *>(context);
    int datalen = audioChannel->getPcm();
    if (datalen>0){
        (*bq)->Enqueue(bq,audioChannel->out_buffer,datalen);
    }

}

void *audioPlay(void *args) {
    AudioChannel *audio = static_cast<AudioChannel *>(args);
    audio->initOpenSL();

    /**
     * todo 线程方法一定要    return 0;
     */
    return 0;
}

void *audioDecode(void *args) {
    AudioChannel *audio = static_cast<AudioChannel *>(args);
    audio->decode();
    return 0;
}

void AudioChannel::play() {
    swr_ctx = swr_alloc_set_opts(0, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, out_sample_rate,
                                 avCodecContext->channel_layout, avCodecContext->sample_fmt,
                                 avCodecContext->sample_rate, 0, 0);
    swr_init(swr_ctx);

    pkt_queue.setWork(1);
    frame_queue.setWork(1);
    isPlaying = true;
//    创建初始化opensles的线程
    pthread_create(&pid_audio_play, NULL, audioPlay, this);
//    创建音频解码的线程
    pthread_create(&pid_audio_decode, NULL, audioDecode, this);
}

void AudioChannel::stop() {

}

void AudioChannel::initOpenSL() {
    SLEngineItf engineInterface = NULL;//音频引擎
    SLObjectItf engineObject = NULL;//音频设置对象
//    混音器
    SLObjectItf outputMixObject = NULL;
    //播放器
    SLObjectItf bqPlayerObject = NULL;
    SLPlayItf bqPlayerInterface = NULL;//回调接口
    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue = NULL;//缓冲队列


    /*------------------------------------------------------------------------------------------------------1、初始化播放引擎------------------------------------------------------------------------------------------------------*/
    SLresult result;
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    //音频接口 相当于surfaceholder
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineInterface);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }


    /*------------------------------------------------------------------------------------------------------2、初始化混音器------------------------------------------------------------------------------------------------------*/

    result = (*engineInterface)->CreateOutputMix(engineInterface, &outputMixObject, 0, 0, 0);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
//    初始化混音器outputMixObject
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    SLDataLocator_AndroidBufferQueue android_queue = {SL_DATALOCATOR_ANDROIDBUFFERQUEUE, 2};
    SLDataFormat_PCM pcm = {
            SL_DATAFORMAT_PCM,
            2,
            SL_SAMPLINGRATE_44_1,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
            SL_BYTEORDER_LITTLEENDIAN
    };
    SLDataSource slDataSource = {&android_queue, &pcm};
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&outputMix, NULL,};
    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};
    (*engineInterface)->CreateAudioPlayer(engineInterface,
                                          &bqPlayerObject,//播放器
                                          &slDataSource,//播放器参数 播放器缓冲队列 播放格式
                                          &audioSnk,//播放缓冲区
                                          1,//播放接口回调个数
                                          ids,//播放队列id
                                          req//是否使用内置的播放队列
    );
    //初始化播放器
    (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerInterface);
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE, &bqPlayerBufferQueue);
    (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallBack, this);
    (*bqPlayerInterface)->SetPlayState(bqPlayerInterface, SL_PLAYSTATE_PLAYING);//设置播放状态
    bqPlayerCallBack(bqPlayerBufferQueue, this);
    LOGE("--手动调用播放packet：%d", this->pkt_queue.size());
}

void AudioChannel::decode() {
    AVPacket *packet = 0;
    while (isPlaying) {
        int ret = pkt_queue.get(packet);
        if (!isPlaying) {
            break;
        }
        if (!ret) {
            continue;
        }
        ret = avcodec_send_packet(avCodecContext, packet);
        releaseAvPacket(packet);
        if (ret == AVERROR(EAGAIN)) {
            continue;
        } else if (ret < 0) {
            break;
        }
        AVFrame *frame = av_frame_alloc();
        ret = avcodec_receive_frame(avCodecContext, frame);
        if (ret == AVERROR(EAGAIN)) {
            continue;
        } else if (ret < 0) {
            break;
        }

        while (frame_queue.size() > 100 && isPlaying) {//生产速度大小消费速度
            av_usleep(1000 * 10);//队列满了 延缓生产速度 休眠10毫秒
            continue;
        }
        frame_queue.put(frame);
    }
    releaseAvPacket(packet);
}

int AudioChannel::getPcm() {
    AVFrame *frame = 0;
    int data_size=0;
    while (isPlaying) {
        int ret = frame_queue.get(frame);
//        转换
        if (!isPlaying) {
            break;
        }
        if (!ret) {
            continue;
        }
        /**
* todo 6.2 转换、对齐、输出
*/
        uint64_t dst_nb_samples = av_rescale_rnd(
                swr_get_delay(swr_ctx, frame->sample_rate) + frame->nb_samples, out_sample_rate,
                frame->sample_rate, AV_ROUND_UP);

        int nb = swr_convert(swr_ctx,
                             &out_buffer, dst_nb_samples,//转换后存放的out_buffer
                             (const uint8_t **) frame->data, frame->nb_samples
        );
//        转换后的out——buffer大小
        data_size=nb*out_channels*out_samplesize;
        clock = frame->pts * av_q2d(time_base);//pts 数量*单位
        break;
    }
    releaseAvFrame(frame);
    return data_size;
}
