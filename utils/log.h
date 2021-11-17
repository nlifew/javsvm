//
// Created by nlifew on 2020/10/15.
//

#ifndef JAVSVM_LOG_H
#define JAVSVM_LOG_H

#include <cstdio>
#include <cerrno>
#include <cstring>

#define LOGI(fmt, ...)  ::std::printf("info: "  fmt, ##__VA_ARGS__)
#define LOGD(fmt, ...)  ::std::printf("debug: " fmt, ##__VA_ARGS__)
#define LOGE(fmt, ...)  ::std::fprintf(stderr, "error: " fmt, ##__VA_ARGS__)
#define PLOGE(fmt, ...) ::std::fprintf(stderr, "error: " fmt "%s(%d)\n", ##__VA_ARGS__, strerror(errno), errno)


#endif //JAVSVM_LOG_H
