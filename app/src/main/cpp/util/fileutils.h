//
// Created by WrBug on 2018/4/8.
//
#include <sys/types.h>
#include <android/log.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#ifndef DUMPDEX_FILEUTILS_H
#define DUMPDEX_FILEUTILS_H

#endif //DUMPDEX_FILEUTILS_H


void save_dex_file( u_int8_t *data, size_t length);

void init_package_name(char *package_name);