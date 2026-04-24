#pragma once

#ifdef __ANDROID__
#include <android/log.h>
#define LOG_TAG "tower-defence"
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))
#else
#include <cstdio>
#include <iostream>
#define LOGI(...) do { printf(__VA_ARGS__); printf("\n"); fflush(stdout); } while (0)
    #define LOGE(...) do { fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); fflush(stderr); } while (0)
#endif