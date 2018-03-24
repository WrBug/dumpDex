/*
 *
 *  @author : rrrfff@foxmail.com
 *  https://github.com/rrrfff/ndk_dlopen
 *
 */
#pragma once

#include <jni.h>
#include <dlfcn.h>

#ifdef __cplusplus
extern "C" {
#endif

void ndk_init(JNIEnv *env);
void *ndk_dlopen(const char *filename, int flag);
int ndk_dlclose(void *handle);
const char *ndk_dlerror(void);
void *ndk_dlsym(void *handle, const char *symbol);
int ndk_dladdr(const void *ddr, Dl_info *info);

#ifdef __cplusplus
}
#endif