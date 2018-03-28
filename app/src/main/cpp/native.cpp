//
// Created by WrBug on 2018/3/23.
//
#include "native.h"
#include <unistd.h>
#include <android/log.h>
#include <sys/system_properties.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <string>
#include <dlfcn.h>
#include <string.h>

#define TAG "dumpDex->"
static char pname[256];

void dumpFileName(char *name, int len, const char *pname, int dexlen) {
    time_t now;
    time(&now);
    memset(name, 0, len);
    sprintf(name, "/data/data/%s/dump/source-%u.dex", pname, dexlen);
}

void writeToFile(const char *pname, u_int8_t *data, size_t length) {
    char dname[1024];
    dumpFileName(dname, sizeof(dname), pname, length);
    __android_log_print(ANDROID_LOG_ERROR, TAG, "dump dex file name is : %s", dname);
    __android_log_print(ANDROID_LOG_ERROR, TAG, "start dump");
    int dex = open(dname, O_CREAT | O_WRONLY, 0644);
    if (dex < 0) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "open or create file error");
        return;
    }
    int ret = write(dex, data, length);
    if (ret < 0) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "write file error");
    } else {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "dump dex file success `%s`", dname);
    }
    close(dex);
}


art::DexFile *(*old_opencommon)(void *DexFile_thiz, uint8_t *base, size_t size, void *location,
                                uint32_t location_checksum, void *oat_dex_file, bool verify,
                                bool verify_checksum,
                                void *error_meessage, void *verify_result) = NULL;

art::DexFile *new_opencommon(void *DexFile_thiz, uint8_t *base, size_t size, void *location,
                             uint32_t location_checksum, void *oat_dex_file, bool verify,
                             bool verify_checksum,
                             void *error_meessage, void *verify_result) {
    writeToFile(pname, base, size);
    return (*old_opencommon)(DexFile_thiz, base, size, location, location_checksum,
                             oat_dex_file, verify, verify_checksum, error_meessage,
                             verify_result);
}

JNIEXPORT void JNICALL Java_com_wrbug_dumpdex_Native_dump
        (JNIEnv *env, jclass obj, jstring packageName) {
    char *p = (char *) env->GetStringUTFChars(packageName, 0);
    __android_log_print(ANDROID_LOG_ERROR, TAG, "%s", p);
    strcpy(pname, p);
    ndk_init(env);
    void *handle = ndk_dlopen("libart.so", RTLD_NOW);
    if (handle == NULL) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Error: unable to find the SO : libart.so");
        return;
    }
    void *open_common_addr = NULL;
#if defined(__aarch64__)
    open_common_addr = ndk_dlsym(handle,
                                 "_ZN3art7DexFile10OpenCommonEPKhmRKNSt3__112basic_stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEEjPKNS_10OatDexFileEbbPS9_PNS0_12VerifyResultE");
    __android_log_print(ANDROID_LOG_ERROR, TAG,
                        "open_common_addr= _ZN3art7DexFile10OpenCommonEPKhmRKNSt3__112basic_stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEEjPKNS_10OatDexFileEbbPS9_PNS0_12VerifyResultE");
#elif defined(__arm__)
    open_common_addr = ndk_dlsym(handle,"_ZN3art7DexFile10OpenCommonEPKhjRKNSt3__112basic_stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEEjPKNS_10OatDexFileEbbPS9_PNS0_12VerifyResultE");
        __android_log_print(ANDROID_LOG_ERROR, TAG, "open_common_addr= _ZN3art7DexFile10OpenCommonEPKhjRKNSt3__112basic_stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEEjPKNS_10OatDexFileEbbPS9_PNS0_12VerifyResultE");
#endif

    if (open_common_addr == NULL) {
        __android_log_print(ANDROID_LOG_ERROR, TAG,
                            "Error: unable to find the Symbol : ");
        return;
    }
    if (registerInlineHook((uint32_t) open_common_addr, (uint32_t) new_opencommon,
                           (uint32_t **) &old_opencommon) != ELE7EN_OK) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "register1 hook failed!");
        return;
    } else {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "register1 hook success!");
    }
    if (inlineHook((uint32_t) open_common_addr) != ELE7EN_OK) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "register2 hook failed!");
        return;
    } else {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "register2 hook success!");
    }
    __android_log_print(ANDROID_LOG_DEFAULT, TAG, "loaded so: libart.so");
}
