// Tencent is pleased to support the open source community by making GAutomator available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

/*
 * log_buffer.cpp
 *
 *  Created on: 2015-7-28
 *      Author: yanguoyue
 */

#include "mtaf_log_buffer.h"

#include <algorithm>
#include <assert.h>
#include <cstdio>

using namespace MTAppenderFile;

bool MTAppenderFileLogBuffer::Write(const void *_data, size_t _input_len, void *_output, size_t &_output_len) {
    if (NULL == _data || NULL == _output || 0 == _input_len) {
        return false;
    }

    memcpy(_output, _data, _input_len);
    _output_len = _input_len;

    return true;
}

MTAppenderFileLogBuffer::MTAppenderFileLogBuffer(void *_pbuffer, size_t _len) {
    buff_.Attach(_pbuffer, _len);
    __Fix();

    memset(&cstream_, 0, sizeof(cstream_));
}

MTAppenderFileLogBuffer::~MTAppenderFileLogBuffer() {
    if (Z_NULL != cstream_.state) {
        deflateEnd(&cstream_);
    }
}

PtrBuffer &MTAppenderFileLogBuffer::GetData() {
    return buff_;
}

void MTAppenderFileLogBuffer::Flush(AutoBuffer &_buff) {
    if (Z_NULL != cstream_.state) {
        deflateEnd(&cstream_);
    }

    if ((uint32_t)strnlen((char *)buff_.Ptr(), buff_.Length()) == 0) {
        __Clear();
        return;
    }

    __Flush();
    _buff.Write(buff_.Ptr(), buff_.Length());
    __Clear();
}

bool MTAppenderFileLogBuffer::Write(const void *_data, size_t _length) {
    if (NULL == _data || 0 == _length) {
        return false;
    }

    if (buff_.Length() == 0) {
        if (!__Reset()) return false;
    }

    buff_.Write(_data, _length);

    return true;
}

bool MTAppenderFileLogBuffer::__Reset() {
    __Clear();
    return true;
}

void MTAppenderFileLogBuffer::__Flush() {
}

void MTAppenderFileLogBuffer::__Clear() {
    // 有游标控制数据位置，理论上只需将第一个字节置空即可
    memset(buff_.Ptr(), 0, 1);
    buff_.Length(0, 0);
}


void MTAppenderFileLogBuffer::__Fix() {
}
