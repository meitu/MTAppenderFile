//
// Created by meitu on 2017/3/24.
//

#ifndef MTAF_APPENDER_FILE_H
#define MTAF_APPENDER_FILE_H

#include <assert.h>
#include <stdio.h>

#include "mtaf_base.h"

#ifdef __cplusplus
extern "C" {
#endif

enum mtaf_append_mode {
    mtaf_append_mode_async,
    mtaf_append_mode_sync,
};

typedef struct mtaf_log_appender_t mtaf_log_appender;

mtaf_log_appender *mtaf_log_appender_create(bool shared_thread);

void mtaf_log_appender_destroy(mtaf_log_appender *mlog);

void mtaf_log_appender_open(mtaf_log_appender *mlog, mtaf_append_mode _mode, const char *filedir, const char *filename);

void mtaf_log_appender_append(mtaf_log_appender *mlog, const char *log);

void mtaf_log_appender_append_ex(mtaf_log_appender *mlog, const char *log, const mtaf_log_info *mLogInfo);

void mtaf_log_appender_close(mtaf_log_appender *mlog);

void mtaf_log_appender_setmode(mtaf_log_appender *mlog, mtaf_append_mode _mode);

void mtaf_log_appender_set_console_log(mtaf_log_appender *mlog, bool _is_open);

void mtaf_log_appender_flush(mtaf_log_appender *mlog);

void mtaf_log_appender_flush_sync(mtaf_log_appender *mlog);


#ifdef __cplusplus
}
#endif

#endif // MTAF_APPENDER_FILE_H
