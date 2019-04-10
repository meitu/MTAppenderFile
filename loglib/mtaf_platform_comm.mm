//
//  platform_comm.mm
//  loglib
//
//  Created by EuanC on 19/12/2017.
//  Copyright © 2017 meitu.com. All rights reserved.
//

#ifndef mtaf_platform_comm_h
#define mtaf_platform_comm_h

#import <Foundation/Foundation.h>
#include "mtaf_base.h"

void mtaf_console_log(const char *_log) {
    NSLog(@"[appender_file] %s", _log);
}

void mtaf_console_log(const mtaf_log_info *_info, const char *_log) {
    NSLog(@"[appender_file] %s", _log);
}

#endif /* mtaf_platform_comm_h */
