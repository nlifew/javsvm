//
// Created by nlifew on 2020/10/15.
//

#ifndef JAVSVM_LOG_H
#define JAVSVM_LOG_H

#include <cstdio>
#include <cerrno>
#include <cstring>

#define LOG_LEVEL_VERBOSE   1
#define LOG_LEVEL_INFO      2
#define LOG_LEVEL_DEBUG     3
#define LOG_LEVEL_WARN      4
#define LOG_LEVEL_ERROR     5

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_INFO
#endif


#define LOGV(fmt, ...)  ::std::printf("verbose: "  fmt, ##__VA_ARGS__)
#define LOGI(fmt, ...)  ::std::printf("info: "  fmt, ##__VA_ARGS__)
#define LOGD(fmt, ...)  ::std::printf("debug: " fmt, ##__VA_ARGS__)
#define LOGW(fmt, ...)  ::std::fprintf(stderr, "warn: " fmt, ##__VA_ARGS__)
#define LOGE(fmt, ...)  ::std::fprintf(stderr, "error: " fmt, ##__VA_ARGS__)
#define PLOGE(fmt, ...) ::std::fprintf(stderr, "error: " fmt "%s(%d)\n", ##__VA_ARGS__, strerror(errno), errno)

#if LOG_LEVEL > LOG_LEVEL_VERBOSE
#undef LOGV
#define LOGV(fmt, ...) ((void) 0)
#endif

#if LOG_LEVEL > LOG_LEVEL_INFO
#undef LOGI
#define LOGI(fmt, ...) ((void) 0)
#endif

#if LOG_LEVEL > LOG_LEVEL_DEBUG
#undef LOGD
#define LOGD(fmt, ...) ((void) 0)
#endif

#if LOG_LEVEL > LOG_LEVEL_WARN
#undef LOGW
#define LOGW(fmt, ...) ((void) 0)
#endif

// 非 debug 模式下不显示 LOGD

#ifdef NDEBUG
#undef LOGD
#define LOGD(fmt, ...) ((void) 0)
#endif

#endif //JAVSVM_LOG_H
