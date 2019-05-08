// Tencent is pleased to support the open source community by making GAutomator available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.



#ifndef MTAF_LOCK_H_
#define MTAF_LOCK_H_

#include <unistd.h>

#include "__mtaf_assert.h"
#include "mtaf_mutex.h"
#include "mtaf_spinlock.h"
#include "mtaf_time_utils.h"

namespace MTAppenderFile {
template <typename MutexType>
class BaseScopedLock;
}

template <typename MutexType>
class MTAppenderFile::BaseScopedLock
{
  public:
    explicit BaseScopedLock(MutexType &mtaf_mtaf_mutex, bool initiallyLocked = true)
        : mutex_(mtaf_mtaf_mutex)
        , islocked_(false) {
        if (!initiallyLocked) return;

        lock();
    }

    explicit BaseScopedLock(MutexType &mtaf_mtaf_mutex, long _millisecond)
        : mutex_(mtaf_mtaf_mutex)
        , islocked_(false) {
        timedlock(_millisecond);
    }

    ~BaseScopedLock() {
        if (islocked_) unlock();
    }

    bool islocked() const {
        return islocked_;
    }

    void lock() {
        MTAF_ASSERT(!islocked_);

        if (!islocked_ && mutex_.lock()) {
            islocked_ = true;
        }

        MTAF_ASSERT(islocked_);
    }

    void unlock() {
        MTAF_ASSERT(islocked_);

        if (islocked_) {
            mutex_.unlock();
            islocked_ = false;
        }
    }

    bool trylock() {
        if (islocked_) return false;

        islocked_ = mutex_.trylock();
        return islocked_;
    }

#ifdef __linux__
    bool timedlock(long _millisecond) {
        MTAF_ASSERT(!islocked_);

        if (islocked_) return true;

        islocked_ = mtaf_mutex.timedlock(_millisecond);
        return islocked_;
    }
#else
    bool timedlock(long _millisecond) {
        MTAF_ASSERT(!islocked_);

        if (islocked_) return true;

        unsigned long start = mtaf_gettickcount();
        unsigned long cur = start;

        while (cur <= start + _millisecond) {
            if (trylock()) break;

            usleep(50 * 1000);
            cur = mtaf_gettickcount();
        }

        return islocked_;
    }
#endif

    MutexType &internal() {
        return mutex_;
    }

  private:
    MutexType &mutex_;
    bool islocked_;
};

typedef MTAppenderFile::BaseScopedLock<MTAppenderFile::Mutex> ScopedLock;
typedef MTAppenderFile::BaseScopedLock<MTAppenderFile::SpinLock> ScopedSpinLock;

#endif /* MTAF_LOCK_H_ */
