/*
 * ff_ffvdec_avcodec.c
 *
 * Copyright (c) 2014 Zhang Rui <bbcallen@gmail.com>
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

#include "ff_ffvdec.h"

typedef struct IJKFF_VideoDecoder_Opaque {
    FFPlayer    *ffp;
    Decoder     *decoder;

    int        (*func_get_video_frame)(FFPlayer *ffp, AVFrame *frame);
} IJKFF_VideoDecoder_Opaque;

static int ffvdec_avcodec_setup(IJKFF_VideoDecoder *vdec, FFPlayer *ffp, Decoder *decoder)
{
    IJKFF_VideoDecoder_Opaque *opaque = vdec->opaque;
    opaque->ffp     = ffp;
    opaque->decoder = decoder;
    return 0;
}

static void ffvdec_avcodec_destroy(IJKFF_VideoDecoder *vdec)
{
    // do nothing
}

static int ffvdec_avcodec_start(IJKFF_VideoDecoder *vdec)
{
    return 0;
}

static int ffvdec_avcodec_stop(IJKFF_VideoDecoder *vdec)
{
    return 0;
}

static int ffvdec_avcodec_dequeue_video_frame(IJKFF_VideoDecoder *vdec, AVFrame *frame)
{
    IJKFF_VideoDecoder_Opaque *opaque = vdec->opaque;
    return opaque->func_get_video_frame(opaque->ffp, frame);
}

IJKFF_VideoDecoder *ffvdec_avcodec_create(int (*func_get_video_frame)(FFPlayer *ffp, AVFrame *frame))
{
    IJKFF_VideoDecoder *vdec = ffvdec_alloc(sizeof(IJKFF_VideoDecoder_Opaque));
    if (!vdec)
        return vdec;

    IJKFF_VideoDecoder_Opaque *opaque = vdec->opaque;
    opaque->func_get_video_frame = func_get_video_frame;

    vdec->func_setup   = ffvdec_avcodec_setup;
    vdec->func_destroy = ffvdec_avcodec_destroy;
    vdec->func_start   = ffvdec_avcodec_start;
    vdec->func_stop    = ffvdec_avcodec_stop;
    vdec->func_dequeue_video_frame = ffvdec_avcodec_dequeue_video_frame;
    return vdec;
}





typedef struct IJKFF_VideoDecoderFactory_Opaque {
    int (*func_get_video_frame)(FFPlayer *ffp, AVFrame *frame);
} IJKFF_VideoDecoderFactory_Opaque;

static void ffvdec_avcodec_factory_destroy(IJKFF_VideoDecoderFactory *factory)
{
    // do nothing
}

static IJKFF_VideoDecoder *ffvdec_avcodec_factory_open_decoder(IJKFF_VideoDecoderFactory *factory, FFPlayer *ffp, Decoder *decoder)
{
    IJKFF_VideoDecoderFactory_Opaque *opaque = factory->opaque;
    if (!opaque)
        return NULL;

    IJKFF_VideoDecoder *vdec = ffvdec_avcodec_create(opaque->func_get_video_frame);
    if (!vdec)
        return NULL;

    if (ffvdec_setup(vdec, ffp, decoder) < 0) {
        ffvdec_free_p(&vdec);
        return NULL;
    }

    return vdec;
}

IJKFF_VideoDecoderFactory *ffvdec_avcodec_factory_create(int (*func_get_video_frame)(FFPlayer *ffp, AVFrame *frame))
{
    IJKFF_VideoDecoderFactory *factory = ffvdec_factory_alloc(sizeof(IJKFF_VideoDecoderFactory_Opaque));
    if (!factory)
        return factory;

    IJKFF_VideoDecoderFactory_Opaque *opaque = factory->opaque;
    opaque->func_get_video_frame = func_get_video_frame;

    factory->func_open_decoder = ffvdec_avcodec_factory_open_decoder;
    factory->func_destroy      = ffvdec_avcodec_factory_destroy;
    return factory;
}
