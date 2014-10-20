/*****************************************************************************
 * ijksdl_codec_android_mediadef.h
 *****************************************************************************
 *
 * copyright (c) 2014 Zhang Rui <bbcallen@gmail.com>
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

#ifndef IJKSDL_ANDROID__ANDROID_CODEC_ANDROID_MEDIADEF_H
#define IJKSDL_ANDROID__ANDROID_CODEC_ANDROID_MEDIADEF_H

typedef enum sdl_amedia_status_t {
    SDL_AMEDIA_OK = 0,

    SDL_AMEDIA_ERROR_BASE                  = -10000,
    SDL_AMEDIA_ERROR_UNKNOWN               = SDL_AMEDIA_ERROR_BASE,
    SDL_AMEDIA_ERROR_MALFORMED             = SDL_AMEDIA_ERROR_BASE - 1,
    SDL_AMEDIA_ERROR_UNSUPPORTED           = SDL_AMEDIA_ERROR_BASE - 2,
    SDL_AMEDIA_ERROR_INVALID_OBJECT        = SDL_AMEDIA_ERROR_BASE - 3,
    SDL_AMEDIA_ERROR_INVALID_PARAMETER     = SDL_AMEDIA_ERROR_BASE - 4,

    SDL_AMEDIA_DRM_ERROR_BASE              = -20000,
    SDL_AMEDIA_DRM_NOT_PROVISIONED         = SDL_AMEDIA_DRM_ERROR_BASE - 1,
    SDL_AMEDIA_DRM_RESOURCE_BUSY           = SDL_AMEDIA_DRM_ERROR_BASE - 2,
    SDL_AMEDIA_DRM_DEVICE_REVOKED          = SDL_AMEDIA_DRM_ERROR_BASE - 3,
    SDL_AMEDIA_DRM_SHORT_BUFFER            = SDL_AMEDIA_DRM_ERROR_BASE - 4,
    SDL_AMEDIA_DRM_SESSION_NOT_OPENED      = SDL_AMEDIA_DRM_ERROR_BASE - 5,
    SDL_AMEDIA_DRM_TAMPER_DETECTED         = SDL_AMEDIA_DRM_ERROR_BASE - 6,
    SDL_AMEDIA_DRM_VERIFY_FAILED           = SDL_AMEDIA_DRM_ERROR_BASE - 7,
    SDL_AMEDIA_DRM_NEED_KEY                = SDL_AMEDIA_DRM_ERROR_BASE - 8,
    SDL_AMEDIA_DRM_LICENSE_EXPIRED         = SDL_AMEDIA_DRM_ERROR_BASE - 9,
} sdl_amedia_status_t;

#endif

