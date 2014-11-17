/*
 * ff_ffvdec_android_mediacodec.h
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

#ifndef IJKPLAYER_ANDROID__FF_FFVDEC_ANDROID_MEDIACODEC_H
#define IJKPLAYER_ANDROID__FF_FFVDEC_ANDROID_MEDIACODEC_H

#include <jni.h>

typedef struct IJKFF_VideoDecoder IJKFF_VideoDecoder;

IJKFF_VideoDecoder *ffvdec_android_mediacodec_create();
IJKFF_VideoDecoder *ffvdec_android_mediacodec_set_surface(JNIEnv *env, IJKFF_VideoDecoder* vdec, jobject surface);

#endif