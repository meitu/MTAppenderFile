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
 * log_buffer.h
 *
 *  Created on: 2015-7-28
 *      Author: yanguoyue
 */

#ifndef MTAF_LOGBUFFER_H_
#define MTAF_LOGBUFFER_H_

#include <stdint.h>
#include <string>
#include <zlib.h>
#include "mtaf_autobuffer.h"
#include "mtaf_ptrbuffer.h"

class MTAppenderFileLogCrypt;

class MTAppenderFileLogBuffer
{
  public:
    MTAppenderFileLogBuffer(void *_pbuffer, size_t _len);
    ~MTAppenderFileLogBuffer();

  public:
    static bool Write(const void *_data, size_t _inputlen, void *_output, size_t &_len);

  public:
    MTAppenderFile::PtrBuffer &GetData();

    void Flush(MTAppenderFile::AutoBuffer &_buff);
    bool Write(const void *_data, size_t _length);

  private:
    bool __Reset();
    void __Flush();
    void __Clear();

    void __Fix();

  private:
    MTAppenderFile::PtrBuffer buff_;
    z_stream cstream_;
};


#endif /* MTAF_LOGBUFFER_H_ */
