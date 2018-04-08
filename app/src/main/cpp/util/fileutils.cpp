//
// Created by WrBug on 2018/4/8.
//


#include "fileutils.h"

#define TAG "dumpDex->"

static char pname[256];

//生成dex文件
static void get_file_name(char *name, int len, int dexlen) {
    memset(name, 0, len);
    sprintf(name, "/data/data/%s/dump/source-%u.dex", pname, dexlen);

}

//保存dex文件
void save_dex_file(u_int8_t *data, size_t length) {
    char dname[1024];
    get_file_name(dname, sizeof(dname), length);
    void *p = fopen(dname, "r");
    if (p != NULL) {
        __android_log_print(ANDROID_LOG_INFO, TAG, "%s file exist", dname);
        return;
    }
    __android_log_print(ANDROID_LOG_INFO, TAG, "dump dex file name is : %s", dname);
    __android_log_print(ANDROID_LOG_INFO, TAG, "start dump");
    int dex = open(dname, O_CREAT | O_WRONLY, 0644);
    if (dex < 0) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "open or create file error");
        return;
    }
    int ret = (int) write(dex, data, length);
    if (ret < 0) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "write file error");
    } else {
        __android_log_print(ANDROID_LOG_INFO, TAG, "dump dex file success `%s`", dname);
    }
    close(dex);
}


void init_package_name(char *package_name) {
    strcpy(pname, package_name);
}