/*
 * comm_assert.c
 *
 *  Created on: 2012-9-5
 *      Author: yerungui
 */


#include <assert.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>

#include "mtaf_compiler_util.h"

#ifndef XLOGGER_TAG
#define XLOGGER_TAG ""
#endif

#if defined(__APPLE__) && (defined(NDEBUG))
void __assert_rtn(const char *, const char *, int, const char *) __dead2;
#endif

#ifdef DEBUG
static int sg_enable_assert = 1;
#else
static int sg_enable_assert = 0;
#endif

void MTAF_ENABLE_ASSERT() {
    sg_enable_assert = 1;
}
void MTAF_DISABLE_ASSERT() {
    sg_enable_assert = 0;
}
int MTAF_IS_ASSERT_ENABLE() {
    return sg_enable_assert;
}

EXPORT_FUNC void __ASSERT(const char *_pfile, int _line, const char *_pfunc, const char *_pexpression) {
    if (MTAF_IS_ASSERT_ENABLE()) {
        __assert_rtn(_pfunc, _pfile, _line, _pexpression);
    }
}

void __ASSERTV2(const char *_pfile, int _line, const char *_pfunc, const char *_pexpression, const char *_format, va_list _list) {
    if (MTAF_IS_ASSERT_ENABLE()) {
        __assert_rtn(_pfunc, _pfile, _line, _pexpression);
    }
}

void __ASSERT2(const char *_pfile, int _line, const char *_pfunc, const char *_pexpression, const char *_format, ...) {
    va_list valist;
    va_start(valist, _format);
    __ASSERTV2(_pfile, _line, _pfunc, _pexpression, _format, valist);
    va_end(valist);
}
