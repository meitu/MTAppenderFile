//
// Created by meitu on 2017/3/24.
//

#ifndef MTAF_BASE_H
#define MTAF_BASE_H

#include <sys/time.h>
#include <time.h>

#define LOG_LEVEL_ALL 0
#define LOG_LEVEL_VERBOSE 0
#define LOG_LEVEL_DEBUG 1
#define LOG_LEVEL_INFO 2
#define LOG_LEVEL_WARN 3
#define LOG_LEVEL_ERROR 4
#define LOG_LEVEL_FATAL 5
#define LOG_LEVEL_NONE 6

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mtaf_log_info_t {
    int level;
    const char *tag;
    struct timeval timeval;
} mtaf_log_info;

#ifdef __cplusplus
}
#endif

#endif // MTAF_BASE_H
