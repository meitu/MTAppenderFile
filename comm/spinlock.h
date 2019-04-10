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
//  spinlock.h
//
//

#ifndef MTAF_spinlock_h
#define MTAF_spinlock_h

#ifdef __APPLE__
#include <libkern/OSAtomic.h>

#define splock OSSpinLock
#define splockinit(lock) \
    { *lock = OS_SPINLOCK_INIT; }
#define splocklock OSSpinLockLock
#define splockunlock OSSpinLockUnlock
#define splocktrylock OSSpinLockTry

namespace MTAppenderFile {
class SpinLock;
}

class MTAppenderFile::SpinLock
{
  public:
    typedef splock handle_type;

  public:
    SpinLock() { splockinit(&lock_); }

    bool lock() {
        splocklock(&lock_);
        return true;
    }

    bool unlock() {
        splockunlock(&lock_);
        return true;
    }

    bool trylock() {
        return splocktrylock(&lock_);
    }

    splock *internal() { return &lock_; }

  private:
    SpinLock(const SpinLock &);
    SpinLock &operator=(const SpinLock &);

  private:
    splock lock_;
};

#endif // __APPLE__
#endif
