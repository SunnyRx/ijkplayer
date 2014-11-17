/*
 * ff_ffvdec_avcodec.h
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

#ifndef FFPLAY__FF_FFVDEC_AVCODEC_H
#define FFPLAY__FF_FFVDEC_AVCODEC_H

IJKFF_VideoDecoder *ffvdec_avcodec_create(int (*func_get_video_frame)(FFPlayer *ffp, AVFrame *frame));

IJKFF_VideoDecoderFactory *ffvdec_avcodec_factory_create(int (*func_get_video_frame)(FFPlayer *ffp, AVFrame *frame));

#endif