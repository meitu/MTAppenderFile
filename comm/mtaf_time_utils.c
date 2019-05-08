/*
 * utils.c
 *
 *  Created on: 2012-7-18
 *      Author: yerungui
 */

#include <stdint.h>
#include <sys/time.h>

#ifdef ANDROID

#include <errno.h>
#include <linux/ioctl.h>
#include <time.h>

#if __ANDROID_API__ < 21 && !defined(__LP64__)
#include <sys/atomics.h>
#else
#include <stdatomic.h>
#endif

#include <android_alarm.h>
#include <fcntl.h>

uint64_t mtaf_gettickcount() {
    static int s_fd = -1;
    static int errcode = 0;
    if (s_fd == -1 && EACCES != errcode) {
        int fd = open("/dev/alarm", O_RDONLY);
        if (-1 == fd) errcode = errno;
#if __ANDROID_API__ < 21 && !defined(__LP64__)
        if (__atomic_cmpxchg(-1, fd, &s_fd)) {
            close(fd);
        }
#else
        atomic_int x = ATOMIC_VAR_INIT(s_fd);
        int expect = -1;
        if (!atomic_compare_exchange_strong(&x, &expect, fd)) {
            close(fd);
        }
        s_fd = atomic_load(&x);
#endif
    }

    struct timespec ts;
    int result = ioctl(s_fd, ANDROID_ALARM_GET_TIME(ANDROID_ALARM_ELAPSED_REALTIME), &ts);

    if (result != 0) {
        // XXX: there was an error, probably because the driver didn't
        // exist ... this should return
        // a real error, like an exception!
        clock_gettime(CLOCK_BOOTTIME, &ts);
    }
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

uint64_t mtaf_clock_app_monotonic() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

#elif defined __APPLE__

#include <TargetConditionals.h>
#include <mach/mach_time.h>

uint64_t mtaf_gettickcount() {
    static mach_timebase_info_data_t timebase_info = {0};

    // Convert to nanoseconds - if this is the first time we've run, get the timebase.
    if (timebase_info.denom == 0) {
        (void)mach_timebase_info(&timebase_info);
    }

    // Convert the mach time to milliseconds
    uint64_t mach_time = mach_absolute_time();
    uint64_t millis = (mach_time * timebase_info.numer) / (timebase_info.denom * 1000000);
    return millis;
}

uint64_t mtaf_clock_app_monotonic() {
    return mtaf_gettickcount();
}

#else
#error "not support"
#endif

int64_t mtaf_gettickspan(uint64_t _old_tick) {
    uint64_t cur_tick = mtaf_gettickcount();
    if (_old_tick > cur_tick) return 0;

    return cur_tick - _old_tick;
}

uint64_t mtaf_timeMs() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000 + (uint64_t)tv.tv_usec / 1000;
}
