//
// Created by WrBug on 2018/4/8.
//


#include "deviceutils.h"
#include "fileutils.h"

#define TAG "dumpDex->"

const static long DEX_MIN_LEN = 102400L;
static int sdk_int = 0;

void init_sdk_init() {
    if (sdk_int != 0) {
        return;
    }
    char sdk[PROP_VALUE_MAX];
    __system_property_get("ro.build.version.sdk", sdk);
    sdk_int = atoi(sdk);
}

bool is_arm_64() {
#if defined(__aarch64__)
    return true;
#else
    return false;
#endif
}

bool is_oreo() {
    return sdk_int == 27 || sdk_int == 26;
}

bool is_nougat() {
    return sdk_int == 25 || sdk_int == 24;
}

char *get_open_function_flag() {
    init_sdk_init();
    if (is_arm_64()) {
        if (is_oreo()) {
            return "_ZN3art7DexFile10OpenCommonEPKhmRKNSt3__112basic_stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEEjPKNS_10OatDexFileEbbPS9_PNS0_12VerifyResultE";
        }
    } else {
        if (is_oreo()) {
            return "_ZN3art7DexFile10OpenCommonEPKhjRKNSt3__112basic_stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEEjPKNS_10OatDexFileEbbPS9_PNS0_12VerifyResultE";
        }

        if (is_nougat()) {
            return "_ZN3art7DexFile10OpenMemoryEPKhjRKNSt3__112basic_stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEEjPNS_6MemMapEPKNS_10OatDexFileEPS9_";
        }
    }
    return "";
}

//64位

static void *
(*old_arm64_opencommon)(uint8_t *, size_t, void *, uint32_t, void *, bool, bool, void *, void *);

static void *new_arm64_opencommon(uint8_t *base, size_t size, void *location,
                                  uint32_t location_checksum, void *oat_dex_file,
                                  bool verify,
                                  bool verify_checksum,
                                  void *error_meessage, void *verify_result) {
    if (size < DEX_MIN_LEN) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "size =%u", size);
        return nullptr;
    }
    save_dex_file(base, size);
    void *result = old_arm64_opencommon(base, size, location, location_checksum,
                                        oat_dex_file, verify, verify_checksum,
                                        error_meessage,
                                        verify_result);
    return result;
}



//32位


static void *(*old_nougat_open_memory)(void *DexFile_thiz, uint8_t *base,
                                       size_t size, void *location, uint32_t location_checksum,
                                       void *mem_map,
                                       void *oat_dex_file, void *error_msg);

static void *
(new_nougat_open_memory)(void *DexFile_thiz, uint8_t *base, size_t size, void *location,
                         uint32_t location_checksum, void *mem_map,
                         void *oat_dex_file, void *error_msg) {
    if (size < DEX_MIN_LEN) {
        return nullptr;
    }
    save_dex_file(base, size);
    return (*old_nougat_open_memory)(DexFile_thiz, base, size, location, location_checksum, mem_map,
                                     oat_dex_file, error_msg);
}


static void *(*old_opencommon)(void *DexFile_thiz, uint8_t *base, size_t size, void *location,
                               uint32_t location_checksum, void *oat_dex_file, bool verify,
                               bool verify_checksum,
                               void *error_meessage, void *verify_result);


static void *new_opencommon(void *DexFile_thiz, uint8_t *base, size_t size, void *location,
                            uint32_t location_checksum, void *oat_dex_file, bool verify,
                            bool verify_checksum,
                            void *error_meessage, void *verify_result) {
    if (size < DEX_MIN_LEN) {
        return nullptr;
    }
    save_dex_file(base, size);
    return (*old_opencommon)(DexFile_thiz, base, size, location, location_checksum,
                             oat_dex_file, verify, verify_checksum, error_meessage,
                             verify_result);
}


void **get_old_open_function_addr() {
    if (is_arm_64()) {
        if (is_oreo()) {
            return reinterpret_cast<void **>(&old_arm64_opencommon);
        } else {
            return NULL;
        }
    } else {
        if (is_oreo()) {
            return reinterpret_cast<void **>(&old_opencommon);
        } else {
            return reinterpret_cast<void **>(&old_nougat_open_memory);
        }
    }
}

void *get_new_open_function_addr() {
    if (is_arm_64()) {
        if (is_oreo()) {
            return reinterpret_cast<void *>(new_arm64_opencommon);
        } else {
            return NULL;
        }
    } else {
        if (is_oreo()) {
            return reinterpret_cast<void *>(new_opencommon);
        } else {
            return reinterpret_cast<void *>(new_nougat_open_memory);
        }
    }
}