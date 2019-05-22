//
// Created by meitu on 2017/3/23.
//

#include "mtaf_condition.h"
#include "mtaf_lock.h"
#include "mtaf_thread.h"

#include "mtaf_appender.h"
#include "mtaf_base.h"
#include "mtaf_log_buffer.h"
#include "mtaf_mmap_file.h"

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <iostream>
#include <vector>

#define LOG_EXT "mtlog"

using namespace MTAppenderFile;

#define kBufferBlockLength 150 * 1024 // default 150KB.

static void __async_log_thread_operation(void *arg);

static void __log2file(mtaf_log_appender *mlog, const void *_data, size_t _len);

extern void mtaf_log_formater(const mtaf_log_info *_info, const char *_logbody, PtrBuffer &_log);

extern void mtaf_console_log(const char *_log);

extern void mtaf_console_log(const mtaf_log_info *_info, const char *_log);

struct mtaf_log_appender_t {

    std::string sg_logdir;

    std::string sg_logfilename;

    int sg_bufferblock_length; // default 150 * 1024 byte

    Mutex sg_mutex_log_file;

    volatile bool sg_log_close;

    mtaf_mmapped_file *sg_mmap_file;

    MTAppenderFileLogBuffer *sg_log_buff;

    FILE *sg_logfile;

    time_t sg_openfiletime;

    std::string sg_current_dir;

    Mutex sg_mutex_buffer_async;

    Condition sg_cond_buffer_async;

    Thread *sg_thread_async;

    mtaf_append_mode sg_mode;

    uint8_t sg_security;

    bool sg_consolelog_open;

    bool sg_custom_name;
};


#pragma mark -

static void __async_log_shared_thread_operation();

// objc load() run before global cpp static variable inilization,
// so use the function instead of global static variable.
static Condition *__private_shared_condition = nullptr;
void _initialize_share_condition() {
    __private_shared_condition = new Condition();
}
Condition *shared_condition() {
    static std::once_flag initFlag;
    std::call_once(initFlag, _initialize_share_condition);
    return __private_shared_condition;
}

static SpinLock *__private_shared_spinlock;
void _initialize_shared_spinlock() {
    __private_shared_spinlock = new SpinLock();
}
SpinLock *shared_spinlock() {
    static std::once_flag initFlag;
    std::call_once(initFlag, _initialize_shared_spinlock);
    return __private_shared_spinlock;
}

static Thread *__private_shared_thread;
void _initialize_shared_thread() {
    __private_shared_thread = new Thread("com.meitu.log.share_appender_file");
}
Thread *shared_thread() {
    static std::once_flag initFlag;
    std::call_once(initFlag, _initialize_shared_thread);
    return __private_shared_thread;
}

static std::vector<mtaf_log_appender *> *p_shared_appenders;
void _initialize_shared_appenders() {
    p_shared_appenders = new std::vector<mtaf_log_appender *>;
}
std::vector<mtaf_log_appender *> *shared_appenders() {
    static std::once_flag initFlag;
    std::call_once(initFlag, _initialize_shared_appenders);
    return p_shared_appenders;
}

#pragma mark -
mtaf_log_appender *mtaf_log_appender_create(bool use_share_thread) {
    mtaf_log_appender *mlog = new mtaf_log_appender();
    if (!mlog) {
        return NULL;
    }

    mlog->sg_log_close = true;
    mlog->sg_log_buff = NULL;
    mlog->sg_logfile = NULL;
    mlog->sg_openfiletime = 0;
    mlog->sg_thread_async = use_share_thread ? shared_thread() : new Thread(__async_log_thread_operation, mlog, "com.meitu.log.appender_file");
    mlog->sg_mode = mtaf_append_mode_async;
    mlog->sg_consolelog_open = false;

    if (use_share_thread) {
        shared_spinlock()->lock();
        shared_appenders()->push_back(mlog);
        shared_spinlock()->unlock();
    }
    return mlog;
}

void mtaf_log_appender_destroy(mtaf_log_appender *mlog) {
    if (!mlog) {
        return;
    }

    mtaf_log_appender_close(mlog);
    if (mlog->sg_thread_async && mlog->sg_thread_async != shared_thread()) {
        delete mlog->sg_thread_async;
    }
    mlog->sg_thread_async = NULL;

    shared_spinlock()->lock();
    for (auto it = shared_appenders()->begin(); it != shared_appenders()->end(); it++) {
        if (*it == mlog) {
            it = shared_appenders()->erase(it);
            break;
        }
    }

    shared_spinlock()->unlock();

    delete mlog;
}

void mtaf_log_appender_open(mtaf_log_appender *mlog, mtaf_append_mode _mode, const char *filedir, const char *filename) {
    assert(filedir);
    assert(filename);
    if (!mlog) {
        printf("mt_appender_open : MTLogAppender is NULL\n");
        return;
    }
    if (!mlog->sg_log_close) {
        printf("appender has already been opened. _dir:%s _name:%s\n", filedir, filename);
        return;
    }
    if (mlog->sg_bufferblock_length <= 0) {
        mlog->sg_bufferblock_length = kBufferBlockLength;
    }

    char mmap_file_path[512] = {0};
    snprintf(mmap_file_path, sizeof(mmap_file_path), "%s/%s.mmap2", filedir, filename);

    if (!mlog->sg_mmap_file) {
        mlog->sg_mmap_file = (mtaf_mmapped_file *)malloc(sizeof(mtaf_mmapped_file));
        mlog->sg_mmap_file->isOpen = false;
    }
    bool use_mmap;
    if (mtaf_open_mmap(mlog->sg_mmap_file, mmap_file_path, mlog->sg_bufferblock_length, false) >= 0) {
        mlog->sg_log_buff = new MTAppenderFileLogBuffer(mlog->sg_mmap_file->start, mlog->sg_bufferblock_length);
        use_mmap = true;
    } else {
        char *buffer = new char[mlog->sg_bufferblock_length];
        mlog->sg_log_buff = new MTAppenderFileLogBuffer(buffer, mlog->sg_bufferblock_length);
        use_mmap = false;
    }
    if (NULL == mlog->sg_log_buff->GetData().Ptr()) {
        if (use_mmap && mlog->sg_mmap_file->isOpen)
            mtaf_close_mmap(mlog->sg_mmap_file);
        return;
    }

    AutoBuffer buffer;
    mlog->sg_log_buff->Flush(buffer);
    ScopedLock lock(mlog->sg_mutex_log_file);
    mlog->sg_logdir = filedir;
    mlog->sg_logfilename = filename;
    mlog->sg_log_close = false;
    mtaf_log_appender_setmode(mlog, _mode);
    lock.unlock();

    // 上次的缓存回写到文件
    if (buffer.Ptr()) {
        __log2file(mlog, buffer.Ptr(), buffer.Length());
    }
}

static void __make_logfilename(const timeval &_tv, const std::string &_logdir, const char *_filename,
    const std::string &_fileext, char *_filepath, unsigned int _len) {

    std::string logfilepath = _logdir;
    logfilepath += "/";
    logfilepath += _filename;
    logfilepath += ".";
    logfilepath += _fileext;
    strncpy(_filepath, logfilepath.c_str(), _len - 1);
    _filepath[_len - 1] = '\0';
}

static bool __writefile(const void *_data, size_t _len, FILE *_file) {
    if (NULL == _file) {
        return false;
    }

    long before_len = ftell(_file);
    if (before_len < 0) return false;

    if (1 != fwrite(_data, _len, 1, _file)) {
        int err = ferror(_file);

        printf("write file error:%d\n", err);

        return false;
    }

    return true;
}

static bool __openlogfile(mtaf_log_appender *mlog, const std::string &_log_dir) {
    if (mlog->sg_logdir.empty()) return false;

    struct timeval tv;
    gettimeofday(&tv, NULL);

    if (NULL != mlog->sg_logfile) {
        time_t sec = tv.tv_sec;
        tm tcur = *localtime((const time_t *)&sec);
        tm filetm = *localtime(&mlog->sg_openfiletime);

        if (filetm.tm_year == tcur.tm_year && filetm.tm_mon == tcur.tm_mon && filetm.tm_mday == tcur.tm_mday && mlog->sg_current_dir == _log_dir)
            return true;

        fclose(mlog->sg_logfile);
        mlog->sg_logfile = NULL;
    }

    mlog->sg_openfiletime = tv.tv_sec;
    mlog->sg_current_dir = _log_dir;

    char logfilepath[1024] = {0};
    __make_logfilename(tv, _log_dir, mlog->sg_logfilename.c_str(), LOG_EXT, logfilepath, 1024);

    mlog->sg_logfile = fopen(logfilepath, "ab");

    if (NULL == mlog->sg_logfile) {
        printf("open file error:%d %s, path:%s", errno, strerror(errno), logfilepath);
    }

    return NULL != mlog->sg_logfile;
}

static void __closelogfile(mtaf_log_appender *mlog) {
    if (NULL == mlog->sg_logfile) return;

    mlog->sg_openfiletime = 0;
    fclose(mlog->sg_logfile);
    mlog->sg_logfile = NULL;
}

static void __log2file(mtaf_log_appender *mlog, const void *_data, size_t _len) {
    if (NULL == _data || 0 == _len || mlog->sg_logdir.empty()) {
        return;
    }

    ScopedLock lock_file(mlog->sg_mutex_log_file);

    bool write_sucess = false;
    bool open_success = __openlogfile(mlog, mlog->sg_logdir);
    if (open_success) {
        write_sucess = __writefile(_data, _len, mlog->sg_logfile);
        if (mtaf_append_mode_async == mlog->sg_mode) {
            __closelogfile(mlog);
        }
    }

    if (!write_sucess) {
        if (open_success && mtaf_append_mode_sync == mlog->sg_mode) {
            __closelogfile(mlog);
        }
    }
}

static void __log_thread_task(mtaf_log_appender *mlog) {
    if (NULL == mlog->sg_log_buff) return;

    ScopedLock lock_buffer(mlog->sg_mutex_buffer_async);
    AutoBuffer tmp;
    mlog->sg_log_buff->Flush(tmp);
    lock_buffer.unlock();

    if (NULL != tmp.Ptr()) {
        __log2file(mlog, tmp.Ptr(), tmp.Length());
    }
}

static void __async_log_shared_thread_operation() {
    while (true) {
        shared_spinlock()->lock();

        if (shared_appenders()->empty()) {
            shared_spinlock()->unlock();
            break;
        }

        for (auto it = shared_appenders()->begin(); it != shared_appenders()->end(); it++) {
            mtaf_log_appender *mlog = *it;
            if (mlog->sg_log_close) {
                continue;
            }

            __log_thread_task(mlog);
        }

        shared_spinlock()->unlock();

        shared_condition()->wait(15 * 60 * 1000);
    }
}

static void __async_log_thread_operation(void *arg) {
    mtaf_log_appender *mlog = (mtaf_log_appender *)arg;
    while (true) {
        if (NULL == mlog->sg_log_buff) {
            break;
        }

        __log_thread_task(mlog);

        if (mlog->sg_log_close) {
            break;
        }

        mlog->sg_cond_buffer_async.wait(15 * 60 * 1000);
    }
}

static void __appender_sync(mtaf_log_appender *mlog, const mtaf_log_info *_info, const char *_log) {
    char temp[16 * 1024] = {0}; // tell perry,ray if you want modify size.
    PtrBuffer log(temp, 0, sizeof(temp));
    mtaf_log_formater(_info, _log, log);
    char buffer_crypt[16 * 1024] = {0};
    size_t len = 16 * 1024;
    if (!MTAppenderFileLogBuffer::Write(log.Ptr(), log.Length(), buffer_crypt, len))
        return;
    __log2file(mlog, buffer_crypt, len);
}

static void __appender_async(mtaf_log_appender *mlog, const mtaf_log_info *_info, const char *_log) {
    ScopedLock lock(mlog->sg_mutex_buffer_async);
    if (NULL == mlog->sg_log_buff) return;

    char temp[16 * 1024] = {0}; //tell perry,ray if you want modify size.
    PtrBuffer log_buff(temp, 0, sizeof(temp));
    mtaf_log_formater(_info, _log, log_buff);

    if (mlog->sg_log_buff->GetData().Length() >= mlog->sg_bufferblock_length * 4 / 5) {
        int ret = snprintf(temp, sizeof(temp),
            "[F][ sg_buffer_async.Length() >= BUFFER_BLOCK_LENTH*4/5, len: %d\n",
            (int)mlog->sg_log_buff->GetData().Length());
        log_buff.Length(ret, ret);
    }

    if (!mlog->sg_log_buff->Write(log_buff.Ptr(), (unsigned int)log_buff.Length())) return;

    // mmap/内存超出一定限度就写通知异步线程写回到文件中
    if (mlog->sg_log_buff->GetData().Length() >= mlog->sg_bufferblock_length * 1 / 3 || (NULL != _info && LOG_LEVEL_FATAL == _info->level)) {
        mlog->sg_cond_buffer_async.notifyAll();
    }

    // 共享线程在有数据输入的时候才启动
    if (!shared_thread()->isruning()) {
        shared_thread()->start(&__async_log_shared_thread_operation, NULL);
    }

    for (auto it = shared_appenders()->begin(); it != shared_appenders()->end(); it++) {
        mtaf_log_appender *mlog = *it;
        if (mlog->sg_log_close)
            continue;

        if (mlog->sg_log_buff->GetData().Length() >= mlog->sg_bufferblock_length * 1 / 3 || (NULL != _info && LOG_LEVEL_FATAL == _info->level)) {
            shared_condition()->notifyAll();
            break;
        }
    }
}

void mtaf_log_appender_append(mtaf_log_appender *mlog, const char *log) {
    if (mlog->sg_log_close) return;

    if (mlog->sg_consolelog_open) mtaf_console_log(log);

    if (mtaf_append_mode_sync == mlog->sg_mode)
        __appender_sync(mlog, NULL, log);
    else
        __appender_async(mlog, NULL, log);
}

void mtaf_log_appender_append_ex(mtaf_log_appender *mlog, const char *log, const mtaf_log_info *mLogInfo) {
    if (mlog->sg_log_close) return;

    if (mlog->sg_consolelog_open) mtaf_console_log(mLogInfo, log);

    if (mtaf_append_mode_sync == mlog->sg_mode)
        __appender_sync(mlog, mLogInfo, log);
    else
        __appender_async(mlog, mLogInfo, log);
}

void mtaf_log_appender_close(mtaf_log_appender *mlog) {
    if (mlog->sg_log_close) return;

    mlog->sg_log_close = true;

    // 使用独立的线程
    mlog->sg_cond_buffer_async.notifyAll();
    if (mlog->sg_thread_async->isruning() && mlog->sg_thread_async != shared_thread()) {
        mlog->sg_thread_async->join();
    }

    // 使用共享线程
    shared_condition()->notifyAll();
    if (mlog->sg_thread_async == shared_thread()) {
        __log_thread_task(mlog);
    }

    ScopedLock buffer_lock(mlog->sg_mutex_buffer_async);
    if (mlog->sg_mmap_file->isOpen) {
        memset(mlog->sg_mmap_file->start, 0, mlog->sg_bufferblock_length);
        mtaf_close_mmap(mlog->sg_mmap_file);
    } else {
        delete[](char *)((mlog->sg_log_buff->GetData()).Ptr());
    }

    free(mlog->sg_mmap_file);
    mlog->sg_mmap_file = NULL;
    delete mlog->sg_log_buff;
    mlog->sg_log_buff = NULL;
    buffer_lock.unlock();

    ScopedLock lock(mlog->sg_mutex_log_file);
    __closelogfile(mlog);
}

void mtaf_log_appender_setmode(mtaf_log_appender *mlog, mtaf_append_mode _mode) {
    mlog->sg_mode = _mode;
    mlog->sg_cond_buffer_async.notifyAll();
    shared_condition()->notifyAll();

    if (mtaf_append_mode_async == mlog->sg_mode && !mlog->sg_thread_async->isruning() && mlog->sg_thread_async != shared_thread()) {
        mlog->sg_thread_async->start();
    }
}

void mtaf_log_appender_flush(mtaf_log_appender *mlog) {
    mlog->sg_cond_buffer_async.notifyAll();
    shared_condition()->notifyAll();
}

void mtaf_log_appender_flush_sync(mtaf_log_appender *mlog) {
    if (mtaf_append_mode_sync == mlog->sg_mode) {
        return;
    }
    ScopedLock lock_buffer(mlog->sg_mutex_buffer_async);

    if (NULL == mlog->sg_log_buff) return;

    AutoBuffer tmp;
    mlog->sg_log_buff->Flush(tmp);

    lock_buffer.unlock();

    if (tmp.Ptr()) __log2file(mlog, tmp.Ptr(), tmp.Length());
}

void mtaf_log_appender_set_console_log(mtaf_log_appender *mlog, bool _is_open) {
    mlog->sg_consolelog_open = _is_open;
}
