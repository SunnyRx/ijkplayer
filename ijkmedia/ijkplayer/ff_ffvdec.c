/*
 * ff_ffvdec.c
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

#include "ff_ffvdec.h"

IJKFF_VideoDecoder *ffvdec_alloc(size_t opaque_size)
{
    IJKFF_VideoDecoder *vdec = (IJKFF_VideoDecoder*) calloc(1, sizeof(IJKFF_VideoDecoder));
    if (!vdec)
        return NULL;

    vdec->opaque = calloc(1, opaque_size);
    if (!vdec->opaque) {
        free(vdec);
        return NULL;
    }

    vdec->mutex = SDL_CreateMutex();
    if (vdec->mutex == NULL) {
        free(vdec->opaque);
        free(vdec);
        return NULL;
    }

    return vdec;
}

void ffvdec_free(IJKFF_VideoDecoder *vdec)
{
    if (!vdec)
        return;

    if (vdec->func_destroy) {
        vdec->func_destroy(vdec);
    }

    SDL_DestroyMutexP(&vdec->mutex);

    free(vdec->opaque);
    memset(vdec, 0, sizeof(IJKFF_VideoDecoder));
    free(vdec);
}

void ffvdec_free_p(IJKFF_VideoDecoder **vdec)
{
    if (!vdec)
        return;

    ffvdec_free(*vdec);
}

int ffvdec_setup(IJKFF_VideoDecoder *vdec, FFPlayer *ffp, Decoder *decoder)
{
    return vdec->func_setup(vdec, ffp, decoder);
}

int ffvdec_start(IJKFF_VideoDecoder *vdec)
{
    return vdec->func_start(vdec);
}

int ffvdec_stop(IJKFF_VideoDecoder *vdec)
{
    return vdec->func_stop(vdec);
}

int ffvdec_dequeue_video_frame(IJKFF_VideoDecoder *vdec, AVFrame *frame)
{
    return vdec->func_dequeue_video_frame(vdec, frame);
}




IJKFF_VideoDecoderFactory *ffvdec_factory_alloc(size_t opaque_size)
{
    IJKFF_VideoDecoderFactory *factory = (IJKFF_VideoDecoderFactory*) calloc(1, sizeof(IJKFF_VideoDecoderFactory));
    if (!factory)
        return NULL;

    factory->opaque = calloc(1, opaque_size);
    if (!factory->opaque) {
        free(factory);
        return NULL;
    }

    return factory;
}

void ffvdec_factory_free(IJKFF_VideoDecoderFactory *factory)
{
    if (!factory)
        return;

    if (factory->func_destroy) {
        factory->func_destroy(factory);
    }

    free(factory->opaque);
    memset(factory, 0, sizeof(IJKFF_VideoDecoderFactory));
    free(factory);
}

void ffvdec_factory_free_p(IJKFF_VideoDecoderFactory **factory)
{
    if (!factory)
        return;

    ffvdec_factory_free(*factory);
}

IJKFF_VideoDecoder* ffvdec_open_from_factory(IJKFF_VideoDecoderFactory *factory, FFPlayer *ffp, Decoder *decoder)
{
    return factory->func_open_decoder(factory, ffp, decoder);
}
