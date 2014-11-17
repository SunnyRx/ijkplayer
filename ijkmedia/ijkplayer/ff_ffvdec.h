/*
 * ff_ffvdec.h
 *
 * Copyright (c) 2003 Fabrice Bellard
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

#ifndef FFPLAY__FF_FFVDEC_H
#define FFPLAY__FF_FFVDEC_H

#include "ijksdl/ijksdl_mutex.h"

typedef struct FFPlayer     FFPlayer;
typedef struct AVFrame      AVFrame;
typedef struct Decoder      Decoder;

typedef struct IJKFF_VideoDecoder_Opaque IJKFF_VideoDecoder_Opaque;
typedef struct IJKFF_VideoDecoder IJKFF_VideoDecoder;
typedef struct IJKFF_VideoDecoder {
    SDL_mutex *mutex;
    void *opaque;

    int  (*func_setup)              (IJKFF_VideoDecoder *vdec, FFPlayer *ffp, Decoder *decoder);
    void (*func_destroy)            (IJKFF_VideoDecoder *vdec);
    int  (*func_start)              (IJKFF_VideoDecoder *vdec);
    int  (*func_stop)               (IJKFF_VideoDecoder *vdec);
    int  (*func_dequeue_video_frame)(IJKFF_VideoDecoder *vdec, AVFrame *frame);
} IJKFF_VideoDecoder;

IJKFF_VideoDecoder *ffvdec_alloc(size_t opaque_size);
void ffvdec_free(IJKFF_VideoDecoder *vdec);
void ffvdec_free_p(IJKFF_VideoDecoder **vdec);

int  ffvdec_setup              (IJKFF_VideoDecoder *vdec, FFPlayer *ffp, Decoder *decoder);
int  ffvdec_start              (IJKFF_VideoDecoder *vdec);
int  ffvdec_stop               (IJKFF_VideoDecoder *vdec);
int  ffvdec_dequeue_video_frame(IJKFF_VideoDecoder *vdec, AVFrame *frame);



typedef struct IJKFF_VideoDecoderFactory_Opaque IJKFF_VideoDecoderFactory_Opaque;
typedef struct IJKFF_VideoDecoderFactory IJKFF_VideoDecoderFactory;
typedef struct IJKFF_VideoDecoderFactory {
    void *opaque;

    IJKFF_VideoDecoder *(*func_open_decoder) (IJKFF_VideoDecoderFactory *factory, FFPlayer *ffp, Decoder *decoder);
    void                (*func_destroy)      (IJKFF_VideoDecoderFactory *factory);
} IJKFF_VideoDecoderFactory;

IJKFF_VideoDecoderFactory *ffvdec_factory_alloc(size_t opaque_size);
void ffvdec_factory_free(IJKFF_VideoDecoderFactory *factory);
void ffvdec_factory_free_p(IJKFF_VideoDecoderFactory **factory);


#endif