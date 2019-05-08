// Tencent is pleased to support the open source community by making GAutomator available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.



#ifndef MTAF_MUTEX_H_
#define MTAF_MUTEX_H_

#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/time.h>

#include "__mtaf_assert.h"
#include "mtaf_time_utils.h"
namespace MTAppenderFile {
class Mutex;
}

class MTAppenderFile::Mutex
{
  public:
    typedef pthread_mutex_t handle_type;
    Mutex(bool _recursive = false)
        : magic_(reinterpret_cast<uintptr_t>(this))
        , mutex_()
        , attr_() {
        //禁止重复加锁
        int ret = pthread_mutexattr_init(&attr_);

        if (ENOMEM == ret)
            MTAF_ASSERT(0 == ENOMEM);
        else if (0 != ret)
            MTAF_ASSERT(0 == ret);

        ret = pthread_mutexattr_settype(&attr_, _recursive ? PTHREAD_MUTEX_RECURSIVE : PTHREAD_MUTEX_ERRORCHECK);

        if (EINVAL == ret)
            MTAF_ASSERT(0 == EINVAL);
        else if (0 != ret)
            MTAF_ASSERT(0 == ret);

        ret = pthread_mutex_init(&mutex_, &attr_);

        if (EAGAIN == ret)
            MTAF_ASSERT(0 == EAGAIN);
        else if (ENOMEM == ret)
            MTAF_ASSERT(0 == ENOMEM);
        else if (EPERM == ret)
            MTAF_ASSERT(0 == EPERM);
        else if (EBUSY == ret)
            MTAF_ASSERT(0 == EBUSY);
        else if (EINVAL == ret)
            MTAF_ASSERT(0 == EINVAL);
        else if (0 != ret)
            MTAF_ASSERT(0 == ret);
    }

    ~Mutex() {
        magic_ = 0;
        int ret = pthread_mutex_destroy(&mutex_);

        if (EBUSY == ret)
            MTAF_ASSERT(0 == EBUSY);
        else if (EINVAL == ret)
            MTAF_ASSERT(0 == EINVAL);
        else if (0 != ret)
            MTAF_ASSERT(0 == ret);

        ret = pthread_mutexattr_destroy(&attr_);

        if (EINVAL == ret)
            MTAF_ASSERT(0 == EINVAL);
        else if (0 != ret)
            MTAF_ASSERT(0 == ret);
    }

    bool lock() {
        // 成功返回0，失败返回错误码
        MTAF_ASSERT2(reinterpret_cast<uintptr_t>(this) == magic_ && 0 != magic_, "this:%p != mageic:%p", this, (void *)magic_);

        if (reinterpret_cast<uintptr_t>(this) != magic_) return false;

        int ret = pthread_mutex_lock(&mutex_);

        if (EINVAL == ret)
            MTAF_ASSERT(0 == EINVAL);
        else if (EAGAIN == ret)
            MTAF_ASSERT(0 == EAGAIN);
        else if (EDEADLK == ret)
            MTAF_ASSERT(0 == EDEADLK);
        else if (0 != ret)
            MTAF_ASSERT(0 == ret);

        return 0 == ret;
    }

    bool unlock() {
        MTAF_ASSERT2(reinterpret_cast<uintptr_t>(this) == magic_ && 0 != magic_, "this:%p != mageic:%p", this, (void *)magic_);

        int ret = pthread_mutex_unlock(&mutex_);

        if (EINVAL == ret)
            MTAF_ASSERT(0 == EINVAL);
        else if (EAGAIN == ret)
            MTAF_ASSERT(0 == EAGAIN);
        else if (EPERM == ret)
            MTAF_ASSERT(0 == EPERM);
        else if (0 != ret)
            MTAF_ASSERT(0 == ret);

        return 0 == ret;
    }

    bool trylock() {
        MTAF_ASSERT2(reinterpret_cast<uintptr_t>(this) == magic_ && 0 != magic_, "this:%p != mageic:%p", this, (void *)magic_);

        if (reinterpret_cast<uintptr_t>(this) != magic_) return false;

        int ret = pthread_mutex_trylock(&mutex_);

        if (EBUSY == ret) return false;

        if (EINVAL == ret)
            MTAF_ASSERT(0 == EINVAL);
        else if (EAGAIN == ret)
            MTAF_ASSERT(0 == EAGAIN);
        else if (EDEADLK == ret)
            MTAF_ASSERT(0 == EDEADLK);
        else if (0 != ret)
            MTAF_ASSERT(0 == ret);

        return 0 == ret;
    }

    bool islocked() {
        MTAF_ASSERT(reinterpret_cast<uintptr_t>(this) == magic_);

        int ret = pthread_mutex_trylock(&mutex_);

        if (0 == ret) unlock();

        return 0 != ret;
    }

    handle_type &internal() { return mutex_; }

  private:
    Mutex(const Mutex &);
    Mutex &operator=(const Mutex &);

  private:
    static void MakeTimeout(struct timespec *pts, long millisecond) {
        struct timeval tv;
        gettimeofday(&tv, 0);
        pts->tv_sec = millisecond / 1000 + tv.tv_sec;
        pts->tv_nsec = (millisecond % 1000) * 1000 * 1000 + tv.tv_usec * 1000;

        pts->tv_sec += pts->tv_nsec / (1000 * 1000 * 1000);
        pts->tv_nsec = pts->tv_nsec % (1000 * 1000 * 1000);
    }

  private:
    uintptr_t magic_; // Dangling pointer will dead lock, so check it!!!
    pthread_mutex_t mutex_;
    pthread_mutexattr_t attr_;
};


#endif /* MTAF_MUTEX_H_ */
