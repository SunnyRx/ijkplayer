/*****************************************************************************
 * ijksdl_codec_android_mediaformat_java.c
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

#include "ijksdl_codec_android_mediaformat_java.h"

#include "ijksdl_android_jni.h"
#include "ijksdl_codec_android_mediaformat_internal.h"
#include "ijksdl_inc_internal_android.h"
#include "android_bytebuffer.h"

typedef struct SDL_AMediaFormat_Opaque {
    jobject android_media_format;

    jobject android_byte_buffer;
} SDL_AMediaFormat_Opaque;

typedef struct SDL_AMediaFormatJava_fields_t {
    jclass clazz;

    jmethodID jmid__ctor;

    jmethodID jmid_createAudioFormat;
    jmethodID jmid_createVideoFormat;

    jmethodID jmid_getInteger;
    jmethodID jmid_setInteger;

    jmethodID jmid_setByteBuffer;

} SDL_AMediaFormatJava_fields_t;
static SDL_AMediaFormatJava_fields_t g_clazz;

int SDL_AMediaFormatJava__loadClass(JNIEnv *env)
{
    jint sdk_int = SDL_Android_GetApiLevel();
    if (sdk_int < IJK_API_16_JELLY_BEAN) {
        return 0;
    }

    IJK_FIND_JAVA_CLASS( env, g_clazz.clazz, "android/media/MediaFormat");

    IJK_FIND_JAVA_STATIC_METHOD(env, g_clazz.jmid_createAudioFormat,    g_clazz.clazz,
        "createAudioFormat",    "(Ljava/lang/String;II)Landroid/media/MediaFormat;");
    IJK_FIND_JAVA_STATIC_METHOD(env, g_clazz.jmid_createVideoFormat,    g_clazz.clazz,
        "createVideoFormat",    "(Ljava/lang/String;II)Landroid/media/MediaFormat;");

    IJK_FIND_JAVA_METHOD(env, g_clazz.jmid__ctor,           g_clazz.clazz,
        "<init>"    ,           "()V");
    IJK_FIND_JAVA_METHOD(env, g_clazz.jmid_getInteger,      g_clazz.clazz,
        "getInteger",           "(Ljava/lang/String;)I");
    IJK_FIND_JAVA_METHOD(env, g_clazz.jmid_setInteger,      g_clazz.clazz,
        "setInteger",           "(Ljava/lang/String;I)V");
    IJK_FIND_JAVA_METHOD(env, g_clazz.jmid_setByteBuffer,   g_clazz.clazz,
        "setByteBuffer",        "(Ljava/lang/String;Ljava/nio/ByteBuffer;)V");

    return 0;
}

jobject SDL_AMediaFormatJava_getObject(JNIEnv *env, const SDL_AMediaFormat *thiz)
{
    if (!thiz || !thiz->opaque)
        return NULL;

    SDL_AMediaFormat_Opaque *opaque = (SDL_AMediaFormat_Opaque *)thiz->opaque;
    return opaque->android_media_format;
}

static jobject getAndroidMediaFormat(JNIEnv *env, const SDL_AMediaFormat* thiz) {
    if (!thiz)
        return NULL;

    SDL_AMediaFormat_Opaque *opaque = (SDL_AMediaFormat_Opaque *)thiz->opaque;
    if (!opaque)
        return NULL;

    return opaque->android_media_format;
}

static sdl_amedia_status_t SDL_AMediaFormatJava_delete(SDL_AMediaFormat* aformat)
{
    if (!aformat)
        return SDL_AMEDIA_OK;

    JNIEnv *env = NULL;
    if (JNI_OK != SDL_JNI_SetupThreadEnv(&env)) {
        ALOGE("SDL_AMediaFormatJava_delete: SetupThreadEnv failed");
        return SDL_AMEDIA_ERROR_UNKNOWN;
    }

    SDL_AMediaFormat_Opaque *opaque = (SDL_AMediaFormat_Opaque *)aformat->opaque;
    if (opaque) {
        SDL_JNI_DeleteGlobalRefP(env, &opaque->android_media_format);
        SDL_JNI_DeleteGlobalRefP(env, &opaque->android_byte_buffer);
    }

    SDL_AMediaFormat_FreeInternal(aformat);
    return SDL_AMEDIA_OK;
}

static bool SDL_AMediaFormatJava_getInt32(SDL_AMediaFormat* aformat, const char* name, int32_t *out)
{
    JNIEnv *env = NULL;
    if (JNI_OK != SDL_JNI_SetupThreadEnv(&env)) {
        ALOGE("SDL_AMediaFormatJava_getInt32: SDL_JNI_SetupThreadEnv: failed");
        return false;
    }

    jobject android_media_format = getAndroidMediaFormat(env, aformat);
    if (!android_media_format) {
        ALOGE("SDL_AMediaFormatJava_getInt32: getAndroidMediaFormat: failed");
        return false;
    }

    jstring jname = (*env)->NewStringUTF(env, name);
    if (SDL_JNI_CatchException(env) || !jname) {
        ALOGE("SDL_AMediaFormatJava_getInt32: NewStringUTF: failed");
        SDL_JNI_DeleteLocalRefP(env, &jname);
        return false;
    }

    jint ret = (*env)->CallIntMethod(env, android_media_format, g_clazz.jmid_getInteger, jname);
    SDL_JNI_DeleteLocalRefP(env, &jname);
    if (SDL_JNI_CatchException(env)) {
        ALOGE("SDL_AMediaFormatJava_setInt32: CallIntMethod: failed");
        return false;
    }

    if (out)
        *out = ret;
    return true;
}

static void SDL_AMediaFormatJava_setInt32(SDL_AMediaFormat* aformat, const char* name, int32_t value)
{
    JNIEnv *env = NULL;
    if (JNI_OK != SDL_JNI_SetupThreadEnv(&env)) {
        ALOGE("SDL_AMediaFormatJava_setInt32: SDL_JNI_SetupThreadEnv: failed");
        return;
    }

    jobject android_media_format = getAndroidMediaFormat(env, aformat);
    if (!android_media_format) {
        ALOGE("SDL_AMediaFormatJava_getInt32: getAndroidMediaFormat: failed");
        return;
    }

    jstring jname = (*env)->NewStringUTF(env, name);
    if (SDL_JNI_CatchException(env) || !jname) {
        ALOGE("SDL_AMediaFormatJava_setInt32: NewStringUTF: failed");
        SDL_JNI_DeleteLocalRefP(env, &jname);
        return;
    }

    (*env)->CallVoidMethod(env, android_media_format, g_clazz.jmid_setInteger, jname, value);
    SDL_JNI_DeleteLocalRefP(env, &jname);
    if (SDL_JNI_CatchException(env)) {
        ALOGE("SDL_AMediaFormatJava_setInt32: CallVoidMethod: failed");
        return;
    }
}

static void SDL_AMediaFormatJava_setBuffer(SDL_AMediaFormat* aformat, const char* name, void* data, size_t size)
{
    JNIEnv *env = NULL;
    if (JNI_OK != SDL_JNI_SetupThreadEnv(&env)) {
        ALOGE("SDL_AMediaFormatJava_setBuffer: SDL_JNI_SetupThreadEnv: failed");
        return;
    }

    SDL_AMediaFormat_Opaque *opaque = (SDL_AMediaFormat_Opaque *)aformat->opaque;
    jobject android_media_format = opaque->android_media_format;
    jobject android_byte_buffer  = opaque->android_byte_buffer;
    if (!android_byte_buffer) {
        android_byte_buffer = ASDK_ByteBuffer_allocateDirectAsGlobalRef(env, size);
        if (SDL_JNI_CatchException(env) || android_byte_buffer) {
            ALOGE("SDL_AMediaFormatJava_setBuffer: ASDK_ByteBuffer_allocateDirectAsGlobalRef: failed");
            return;
        }
        opaque->android_byte_buffer = android_byte_buffer;;
    }

    jstring jname = (*env)->NewStringUTF(env, name);
    if (SDL_JNI_CatchException(env) || !jname) {
        ALOGE("SDL_AMediaFormatJava_setBuffer: NewStringUTF: failed");
        SDL_JNI_DeleteLocalRefP(env, &jname);
        return;
    }

    ASDK_ByteBuffer__setDataLimited(env, android_byte_buffer, data, size);
    if (SDL_JNI_CatchException(env)) {
        ALOGE("SDL_AMediaFormatJava_setBuffer: ASDK_ByteBuffer__setDataLimited: failed");
        return;
    }

    (*env)->CallVoidMethod(env, android_media_format, g_clazz.jmid_setByteBuffer, jname, android_byte_buffer);
    SDL_JNI_DeleteLocalRefP(env, &jname);
    if (SDL_JNI_CatchException(env)) {
        ALOGE("SDL_AMediaFormatJava_setBuffer: call jmid_setByteBuffer: failed");
        return;
    }
}

SDL_AMediaFormat *SDL_AMediaFormatJava_new()
{
    JNIEnv *env = NULL;
    if (JNI_OK != SDL_JNI_SetupThreadEnv(&env)) {
        ALOGE("SDL_AMediaFormatJava_new: SetupThreadEnv failed");
        return NULL;
    }

    jobject android_media_format = SDL_JNI_NewObjectAsGlobalRef(env, g_clazz.clazz, g_clazz.jmid__ctor);
    if (SDL_JNI_CatchException(env) || !android_media_format) {
        return NULL;
    }

    SDL_AMediaFormat *aformat = SDL_AMediaFormat_CreateInternal(sizeof(SDL_AMediaFormat_Opaque));
    if (!aformat) {
        SDL_JNI_DeleteGlobalRefP(env, &android_media_format);
        return NULL;
    }

    SDL_AMediaFormat_Opaque *opaque = aformat->opaque;
    opaque->android_media_format = android_media_format;

    aformat->func_delete    = SDL_AMediaFormatJava_delete;
    aformat->func_getInt32  = SDL_AMediaFormatJava_getInt32;
    aformat->func_setInt32  = SDL_AMediaFormatJava_setInt32;
    aformat->func_setBuffer = SDL_AMediaFormatJava_setBuffer;

    return aformat;
}

