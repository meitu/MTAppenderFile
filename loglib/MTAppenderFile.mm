//
//  MTAppenderFile.m
//  MTAppenderFile
//
//  Created by EuanC on 16/12/2017.
//  Copyright © 2017 meitu.com. All rights reserved.
//

#import "MTAppenderFile.h"
#import <sys/xattr.h>
#include "mtaf_appender.h"


@interface MTAppenderFile () {
    mtaf_log_appender *_appender;
}

@property (nonatomic, copy) NSString *fileDir;
@property (nonatomic, copy) NSString *fileName;

@end

@implementation MTAppenderFile

- (void)dealloc {
    [self close];

    [_fileDir release];
    [_fileName release];

    [super dealloc];
}

- (instancetype)initWithFileDir:(NSString *)fileDir name:(NSString *)name {
    if (self = [super init]) {
        self.fileDir = fileDir;
        self.fileName = name;
        _appender = nil;
    }
    return self;
}

- (void)open {
    if (_appender) {
        NSAssert(NO, @"[appender] you should close previous appender firstly");
        return;
    }

    const char *filedir = [_fileDir UTF8String];
    const char *filename = [_fileName UTF8String];

    bool useShareThread = true;
    _appender = mtaf_log_appender_create(useShareThread);
    mtaf_log_appender_open(_appender, mtaf_append_mode_async, filedir, filename);
}

- (void)appendText:(NSString *)text {
    NSAssert(_appender, @"[appender] you should open appender first");
    mtaf_log_appender_append(_appender, [text UTF8String]);
}

- (void)appendUTF8Text:(const char *)utf8Text {
    NSAssert(_appender, @"[appender] you should open appender first");
    mtaf_log_appender_append(_appender, utf8Text);
}

#define kLogBufSize 3000

- (void)appendFormat:(const char *)str, ... {
    if (str == NULL)
        return;

    va_list args;
    va_start(args, str);

    char buf[kLogBufSize];
    vsnprintf(buf, kLogBufSize, str, args);
    va_end(args);

    mtaf_log_appender_append(_appender, buf);
}

- (void)close {
    if (_appender) {
        mtaf_log_appender_close(_appender);
        mtaf_log_appender_destroy(_appender);
        _appender = nil;
    }
}

@end
