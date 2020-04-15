// Tencent is pleased to support the open source community by making GAutomator available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

//
//  ptrbuffer.cc
//
//  Created by yerungui on 13-4-4.
//

#include "mtaf_ptrbuffer.h"

#include <stdio.h>
#include <string.h>

#include "__mtaf_assert.h"

using namespace MTAppenderFile;

const PtrBuffer KNullPtrBuffer(0, 0, 0);

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

PtrBuffer::PtrBuffer(void *_ptr, size_t _len, size_t _maxlen)
    : parray_((unsigned char *)_ptr)
    , pos_(0)
    , length_(_len)
    , max_length_(_maxlen) {
    MTAF_ASSERT(length_ <= max_length_);
}

PtrBuffer::PtrBuffer(void *_ptr, size_t _len)
    : parray_((unsigned char *)_ptr)
    , pos_(0)
    , length_(_len)
    , max_length_(_len) {
    MTAF_ASSERT(length_ <= max_length_);
}

PtrBuffer::PtrBuffer() {
    Reset();
}

PtrBuffer::~PtrBuffer() {
}

void PtrBuffer::Write(const void *_pBuffer, size_t _nLen) {
    Write(_pBuffer, _nLen, Pos());
    Seek(_nLen, kSeekCur);
}

void PtrBuffer::Write(const void *_pBuffer, size_t _nLen, off_t _nPos) {
    MTAF_ASSERT(NULL != _pBuffer);
    MTAF_ASSERT(0 <= _nPos);
    MTAF_ASSERT((unsigned int)_nPos <= Length());
    size_t copylen = (size_t)min(_nLen, max_length_.load() - _nPos);
    length_.store((size_t)max(length_.load(), copylen + _nPos));
    memcpy((unsigned char *)Ptr() + _nPos, _pBuffer, copylen);

    if (length_.load() < max_length_.load() - 4) {
        memset((unsigned char *)Ptr() + _nPos + copylen, 0, sizeof(char) * 3);
        memset((unsigned char *)Ptr() + _nPos + copylen + 3, '\r', sizeof(char));
    }
}

size_t PtrBuffer::Read(void *_pBuffer, size_t _nLen) {
    size_t nRead = Read(_pBuffer, _nLen, Pos());
    Seek(nRead, kSeekCur);
    return nRead;
}

size_t PtrBuffer::Read(void *_pBuffer, size_t _nLen, off_t _nPos) const {
    MTAF_ASSERT(NULL != _pBuffer);
    MTAF_ASSERT(0 <= _nPos);
    MTAF_ASSERT((unsigned int)_nPos < Length());

    size_t nRead = (size_t)(Length() - _nPos);
    nRead = min(nRead, _nLen);
    memcpy(_pBuffer, PosPtr(), nRead);
    return nRead;
}

void PtrBuffer::Seek(off_t _nOffset, TSeek _eOrigin) {
    switch (_eOrigin) {
        case kSeekStart:
            pos_.store(_nOffset);
            break;

        case kSeekCur:
            pos_.store(pos_.load() + _nOffset);
            break;

        case kSeekEnd:
            pos_.store(length_.load() + _nOffset);
            break;

        default:
            MTAF_ASSERT(false);
            break;
    }

    if (pos_.load() < 0)
        pos_.store(0);

    if ((unsigned int)pos_.load() > length_.load())
        pos_.store(length_.load());
}

void PtrBuffer::Length(off_t _nPos, size_t _nLenght) {
    MTAF_ASSERT(0 <= _nPos);
    MTAF_ASSERT((size_t)_nPos <= _nLenght);
    MTAF_ASSERT(_nLenght <= MaxLength());
 
    length_.store(max_length_.load() < _nLenght ? max_length_.load() : _nLenght);
    Seek(_nPos, kSeekStart);
}

void *PtrBuffer::Ptr() {
    return parray_;
}

const void *PtrBuffer::Ptr() const {
    return parray_;
}

void *PtrBuffer::PosPtr() {
    return ((unsigned char *)Ptr()) + Pos();
}

const void *PtrBuffer::PosPtr() const {
    return ((unsigned char *)Ptr()) + Pos();
}

off_t PtrBuffer::Pos() const {
    return pos_.load();
}

size_t PtrBuffer::PosLength() const {
    return (size_t)(length_.load() - pos_.load());
}

size_t PtrBuffer::Length() const {
    return length_.load();
}

size_t PtrBuffer::MaxLength() const {
    return max_length_.load();
}

void PtrBuffer::Attach(void *_pBuffer, size_t _nLen, size_t _maxlen) {
    Reset();
    parray_ = (unsigned char *)_pBuffer;
    length_.store(_nLen);
    max_length_.store(_maxlen);
}

void PtrBuffer::Attach(void *_pBuffer, size_t _nLen) {
    Attach(_pBuffer, _nLen, _nLen);
}

void PtrBuffer::Reset() {
    parray_ = NULL;
    pos_.store(0);
    length_.store(0);
    max_length_.store(0);
}
