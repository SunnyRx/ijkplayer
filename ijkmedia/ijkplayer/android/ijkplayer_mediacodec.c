/*
 * ijkplayer_mediacodec.c
 *
 * Copyright (c) 2013-2014 Zhang Rui <bbcallen@gmail.com>
 *
 * This file is part of ijkPlayer.
 *
 * ijkPlayer is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * ijkPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with ijkPlayer; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <jni.h>
#include "ijkutil/ijkutil.h"
#include "../ff_ffplay.h"
#include "ijkplayer_android_def.h"
#include "ijkplayer_android.h"
#include "ijksdl/android/ijksdl_android_jni.h"
#include "ijksdl/android/ijksdl_codec_android_mediacodec_java.h"
#include "ijksdl/android/ijksdl_codec_android_mediaformat_java.h"

typedef struct IJK_HWDecoderContext {
    SDL_Thread *amc_enqueue_tid;
    SDL_Thread _amc_enqueue_tid;

    SDL_AMediaFormat *aformat;
    SDL_AMediaCodec  *acodec;

    jobject      jsurface;
    SDL_mutex   *surface_mutex;
    volatile int surface_need_reconfigure;

    AVCodecContext           *avctx; // not own
    AVBitStreamFilterContext *bsfc;  // own

    uint8_t *orig_extradata;
    int      orig_extradata_size;

    int abort_request;
} IJK_HWDecoderContext;

static int ijkvdec_init(FFPlayer *ffp)
{
    VideoState *is = ffp->is;
    IJK_HWDecoderContext *hw = ffp->hw_decoder_context;
    const char *codec_mime = NULL;

    AVCodecContext *avctx = is->viddec.avctx;
    switch (avctx->codec_id) {
    case AV_CODEC_ID_H264:
        codec_mime = SDL_AMIME_VIDEO_AVC;
        break;
    default:
        return -1;
    }

    JNIEnv *env = NULL;
    if (JNI_OK != SDL_JNI_SetupThreadEnv(&env)) {
        ALOGE("ijkvdec_init: SetupThreadEnv failed\n");
        return -1;
    }

    hw->avctx = avctx;
    hw->acodec = SDL_AMediaCodecJava_createDecoderByType(env, codec_mime);
    if (!hw->acodec) {
        ALOGE("ijkvdec_init: SDL_AMediaCodecJava_createDecoderByType failed\n");
        return -1;
    }
    // TODO: check codec name

    SDL_AMediaFormat *aformat = SDL_AMediaFormatJava_createVideoFormat(env, codec_mime, avctx->width, avctx->height);
    if (avctx->extradata && avctx->extradata_size > 0) {
        if (avctx->codec_id == AV_CODEC_ID_H264 && avctx->extradata[0] == 1) {
            hw->bsfc = av_bitstream_filter_init("h264_mp4toannexb");

            hw->orig_extradata_size = avctx->extradata_size + FF_INPUT_BUFFER_PADDING_SIZE;
            hw->orig_extradata      = (uint8_t*) av_mallocz(hw->orig_extradata_size);
            if (!hw->orig_extradata) {
                goto fail;
            }
            memcpy(hw->orig_extradata, avctx->extradata, avctx->extradata_size);
            SDL_AMediaFormat_setBuffer(aformat, "csd-0", hw->orig_extradata, hw->orig_extradata_size);
        } else {
            // Codec specific data
            SDL_AMediaFormat_setBuffer(aformat, "csd-0", avctx->extradata, avctx->extradata_size);
        }
    }

    hw->surface_mutex = SDL_CreateMutex();
    if (!hw->surface_mutex)
        goto fail;

    return 0;
fail:

    SDL_AMediaCodec_deleteP(&hw->acodec);
    SDL_AMediaFormat_deleteP(&hw->aformat);

    if (hw->orig_extradata) {
        av_freep(&hw->orig_extradata);
    }

    if (hw->bsfc) {
        av_bitstream_filter_close(hw->bsfc);
        hw->bsfc = NULL;
    }

    if (hw->surface_mutex) {
        SDL_DestroyMutex(hw->surface_mutex);
        hw->surface_mutex = NULL;
    }

    return -1;
}

void ijkvdec_set_surface(JNIEnv *env, FFPlayer *ffp, jobject android_surface)
{
    IJK_HWDecoderContext *hw = ffp->hw_decoder_context;

    SDL_LockMutex(hw->surface_mutex);
    {
        SDL_JNI_DeleteGlobalRefP(env, &hw->jsurface);
        if (android_surface) {
            hw->surface_need_reconfigure = 1;
            hw->jsurface = (*env)->NewGlobalRef(env, hw->jsurface);
        }
    }
    SDL_UnlockMutex(hw->surface_mutex);
}

static int ijkvdec_enqueue_thread(void *arg)
{
#if 0
    FFPlayer   *ffp = arg;
    VideoState *is  = ffp->is;
    IJK_HWDecoderContext *hw      = ffp->hw_decoder_context;
    SDL_AMediaCodec      *acodec  = hw->acodec;
    SDL_AMediaFormat     *aformat = hw->aformat;
    AVCodecContext       *avctx   = hw->avctx;
    sdl_amedia_status_t  md_ret   = SDL_AMEDIA_OK;
    Decoder              *d       = &is->viddec;

    int ret = -1;

    JNIEnv *env = NULL;
    if (JNI_OK != SDL_JNI_SetupThreadEnv(&env)) {
        ALOGE("ijkvdec_enqueue_thread: SetupThreadEnv failed\n");
        return -1;
    }

    while(1) {
        d->flushed = 0;

        if (d->queue->abort_request)
            return -1;

        if (!d->packet_pending || d->queue->serial != d->pkt_serial) {
            AVPacket pkt;
            do {
                if (d->queue->nb_packets == 0)
                    SDL_CondSignal(d->empty_queue_cond);
                if (packet_queue_get_or_buffering(ffp, d->queue, &pkt, &d->pkt_serial, &d->finished) < 0)
                    return -1;
                if (pkt.data == flush_pkt.data) {
                    avcodec_flush_buffers(d->avctx);
                    d->finished = 0;
                    d->flushed = 1;
                    d->next_pts = d->start_pts;
                    d->next_pts_tb = d->start_pts_tb;
                }
            } while (pkt.data == flush_pkt.data || d->queue->serial != d->pkt_serial);
        av_free_packet(&d->pkt);
        d->pkt_temp = d->pkt = pkt;
        d->packet_pending = 1;
    }
#endif
    return 0;
}

int ijkvdec_start(FFPlayer *ffp)
{
    IJK_HWDecoderContext *hw = ffp->hw_decoder_context;

    hw->amc_enqueue_tid = SDL_CreateThreadEx(&hw->_amc_enqueue_tid, ijkvdec_enqueue_thread, ffp, "ff_mediacodec_enqueue");
    if (!hw->amc_enqueue_tid)
        goto fail;

fail:
    return 0;
}

int ijkvdec_stop(FFPlayer *ffp)
{
    IJK_HWDecoderContext *hw = ffp->hw_decoder_context;
    hw->abort_request = 0;
    return 0;
}

int ijkvdec_get_video_frame(FFPlayer *ffp, AVFrame *frame)
{
    VideoState *is  = ffp->is;
    IJK_HWDecoderContext *hw = ffp->hw_decoder_context;
    SDL_AMediaCodec      *acodec  = hw->acodec;
    SDL_AMediaFormat     *aformat = hw->aformat;
    sdl_amedia_status_t  md_ret   = SDL_AMEDIA_OK;

    JNIEnv *env = NULL;
    if (JNI_OK != SDL_JNI_SetupThreadEnv(&env)) {
        ALOGE("ijkvdec_amc_enqueue_thread: SetupThreadEnv failed\n");
        return -1;
    }

    SDL_LockMutex(hw->surface_mutex);
    if (hw->surface_need_reconfigure && hw->jsurface) {
        md_ret = SDL_AMediaCodec_configure_surface(env, acodec, aformat, hw->jsurface, NULL, 0);
        // TODO: retry
        if (md_ret != SDL_AMEDIA_OK)
            return -1;
    }
    SDL_UnlockMutex(hw->surface_mutex);
    return 0;
}
