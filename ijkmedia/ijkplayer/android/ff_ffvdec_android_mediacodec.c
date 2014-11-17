/*
 * ff_ffvdec_android_mediacodec.c
 *
 * Copyright (c) 2013 Zhang Rui <bbcallen@gmail.com>
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

#include "ff_ffvdec_android_mediacodec.h"

#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <jni.h>
#include "ijkutil/ijkutil.h"
#include "../ff_ffplay_def.h"
#include "../ff_ffplay.h"
#include "../ff_ffvdec.h"
#include "ijkplayer_android_def.h"
#include "ijkplayer_android.h"
#include "ijksdl/android/ijksdl_android_jni.h"
#include "ijksdl/android/ijksdl_codec_android_mediacodec_java.h"
#include "ijksdl/android/ijksdl_codec_android_mediaformat_java.h"

typedef struct IJKFF_VideoDecoder_Opaque {
    FFPlayer                 *ffp;
    Decoder                  *decoder;

    SDL_AMediaFormat         *aformat;
    SDL_AMediaCodec          *acodec;

    SDL_mutex                *surface_mutex;
    jobject                   jsurface;
    volatile int              surface_need_reconfigure;

    AVCodecContext           *avctx; // not own
    AVBitStreamFilterContext *bsfc;  // own

    uint8_t                  *orig_extradata;
    int                       orig_extradata_size;

    int                       abort_request;
} IJKFF_VideoDecoder_Opaque;

static void ffvdec_android_mediacodec_destroy(IJKFF_VideoDecoder *vdec)
{
    IJKFF_VideoDecoder_Opaque *opaque = vdec->opaque;

    SDL_DestroyMutexP(&opaque->surface_mutex);

    SDL_AMediaCodec_deleteP(&opaque->acodec);
    SDL_AMediaFormat_deleteP(&opaque->aformat);

    if (opaque->orig_extradata) {
        av_freep(&opaque->orig_extradata);
    }

    if (opaque->bsfc) {
        av_bitstream_filter_close(opaque->bsfc);
        opaque->bsfc = NULL;
    }
}

static int ffvdec_android_mediacodec_setup(IJKFF_VideoDecoder *vdec, FFPlayer *ffp, Decoder *decoder)
{
    if (SDL_Android_GetApiLevel() < IJK_API_16_JELLY_BEAN)
        return -1;

    IJKFF_VideoDecoder_Opaque *opaque = vdec->opaque;
    opaque->ffp     = ffp;
    opaque->decoder = decoder;

    // VideoState *is = ffp->is;
    const char *codec_mime = NULL;

    AVCodecContext *avctx = decoder->avctx;
    switch (avctx->codec_id) {
    case AV_CODEC_ID_H264:
        codec_mime = SDL_AMIME_VIDEO_AVC;
        break;
    default:
        return -1;
    }

    JNIEnv *env = NULL;
    if (JNI_OK != SDL_JNI_SetupThreadEnv(&env)) {
        ALOGE("ffvdec_android_mediacodec_setup: SetupThreadEnv failed\n");
        return -1;
    }

    opaque->avctx = avctx;
    opaque->acodec = SDL_AMediaCodecJava_createDecoderByType(env, codec_mime);
    if (!opaque->acodec) {
        ALOGE("ffvdec_android_mediacodec_setup: SDL_AMediaCodecJava_createDecoderByType failed\n");
        return -1;
    }

    SDL_AMediaFormat *aformat = SDL_AMediaFormatJava_createVideoFormat(env, codec_mime, avctx->width, avctx->height);
    if (avctx->extradata && avctx->extradata_size > 0) {
        if (avctx->codec_id == AV_CODEC_ID_H264 && avctx->extradata[0] == 1) {
            opaque->bsfc = av_bitstream_filter_init("h264_mp4toannexb");

            opaque->orig_extradata_size = avctx->extradata_size + FF_INPUT_BUFFER_PADDING_SIZE;
            opaque->orig_extradata      = (uint8_t*) av_mallocz(opaque->orig_extradata_size);
            if (!opaque->orig_extradata) {
                goto fail;
            }
            memcpy(opaque->orig_extradata, avctx->extradata, avctx->extradata_size);
            SDL_AMediaFormat_setBuffer(aformat, "csd-0", opaque->orig_extradata, opaque->orig_extradata_size);
        } else {
            // Codec specific data
            SDL_AMediaFormat_setBuffer(aformat, "csd-0", avctx->extradata, avctx->extradata_size);
        }
    }

    opaque->surface_mutex = SDL_CreateMutex();
    if (!opaque->surface_mutex)
        goto fail;

    return 0;
fail:

    ffvdec_android_mediacodec_destroy(vdec);
    return -1;
}

static int ffvdec_android_mediacodec_start(IJKFF_VideoDecoder *vdec)
{
    return 0;
}

static int ffvdec_android_mediacodec_stop(IJKFF_VideoDecoder *vdec)
{
    return 0;
}

static int ffvdec_android_mediacodec_dequeue_video_frame(IJKFF_VideoDecoder *vdec, AVFrame *frame)
{
    return 0;
}

IJKFF_VideoDecoder *ffvdec_android_mediacodec_create()
{
    IJKFF_VideoDecoder *vdec = ffvdec_alloc(sizeof(IJKFF_VideoDecoder_Opaque));
    if (!vdec)
        return vdec;

    // IJKFF_VideoDecoder_Opaque *opaque = vdec->opaque;

    vdec->func_setup                = ffvdec_android_mediacodec_setup;
    vdec->func_destroy              = ffvdec_android_mediacodec_destroy;
    vdec->func_start                = ffvdec_android_mediacodec_start;
    vdec->func_stop                 = ffvdec_android_mediacodec_stop;
    vdec->func_dequeue_video_frame  = ffvdec_android_mediacodec_dequeue_video_frame;
    return vdec;
}

int ffvdec_android_mediacodec_set_surface(JNIEnv *env, IJKFF_VideoDecoder* vdec, jobject surface)
{
    IJKFF_VideoDecoder_Opaque *opaque = vdec->opaque;
    if (!opaque->surface_mutex)
        return -1;

    SDL_LockMutex(opaque->surface_mutex);
    {
        SDL_JNI_DeleteGlobalRefP(env, &opaque->jsurface);
        if (surface) {
            opaque->surface_need_reconfigure = 1;
            opaque->jsurface = (*env)->NewGlobalRef(env, surface);
        }
    }
    SDL_UnlockMutex(opaque->surface_mutex);

    return 0;
}




typedef struct IJKFF_VideoDecoderFactory_Opaque {
} IJKFF_VideoDecoderFactory_Opaque;

static void ffvdec_android_mediacodec_factory_destroy(IJKFF_VideoDecoderFactory *factory)
{
    // do nothing
}

static IJKFF_VideoDecoder *ffvdec_android_mediecodec_factory_open_decoder(IJKFF_VideoDecoderFactory *factory, FFPlayer *ffp, Decoder *decoder)
{
    IJKFF_VideoDecoderFactory_Opaque *opaque = factory->opaque;
    if (!opaque)
        return NULL;

    IJKFF_VideoDecoder *vdec = ffvdec_android_mediacodec_create();
    if (ffvdec_setup(vdec, ffp, decoder) < 0) {
        ffvdec_free_p(&vdec);
        return NULL;
    }

    return vdec;
}

IJKFF_VideoDecoderFactory *ffvdec_android_mediacodec_factory_create()
{
    IJKFF_VideoDecoderFactory *factory = ffvdec_factory_alloc(sizeof(IJKFF_VideoDecoderFactory_Opaque));
    if (!factory)
        return factory;

    // IJKFF_VideoDecoderFactory_Opaque *opaque = factory->opaque;

    factory->func_open_decoder = ffvdec_android_mediecodec_factory_open_decoder;
    factory->func_destroy      = ffvdec_android_mediacodec_factory_destroy;
    return factory;
}

