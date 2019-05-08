// Tencent is pleased to support the open source community by making GAutomator available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.


#ifndef MTAF_THREAD_H_
#define MTAF_THREAD_H_

#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include "__mtaf_assert.h"
#include "mtaf_condition.h"
#include "mtaf_runnable.h"

typedef pthread_t thread_tid;
//注意！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！
// 如果在pthread_join途中，其他线程进行pthread_detach, 会出现join函数不能退出的情况
namespace MTAppenderFile {
class ThreadUtil;
class Thread;
} // namespace MTAppenderFile

class MTAppenderFile::ThreadUtil
{
  public:
    static void yield() {
        ::sched_yield();
    }

    static void sleep(unsigned int _sec) {
        ::sleep(_sec);
    }

    static void usleep(unsigned int _usec) {
        ::usleep(_usec);
    }

    static thread_tid currentthreadid() {
        return pthread_self();
    }

    static bool isruning(thread_tid _id) {
        MTAF_ASSERT(0 != _id);
        int ret = pthread_kill(_id, 0);

        if (0 == ret)
            return true;
        else if (ESRCH == ret)
            return false;
        else if (EINVAL == ret)
            MTAF_ASSERT(false);

        return false;
    }

    static int join(thread_tid _id) {
        if (_id == ThreadUtil::currentthreadid()) return EDEADLK;
        int ret = pthread_join(_id, 0);
        MTAF_ASSERT2(0 == ret || ESRCH == ret, "pthread_join err:%d", ret);
        return ret;
    }
};

class MTAppenderFile::Thread
{
  private:
    class RunnableReference
    {
      public:
        RunnableReference(Runnable *_target)
            : target(_target)
            , count(0)
            , tid(0)
            , isjoined(false)
            , isended(true)
            , aftertime(LONG_MAX)
            , periodictime(LONG_MAX)
            , iscanceldelaystart(false)
            , condtime()
            , splock()
            , isinthread(false)
            , killsig(0) {
            memset(thread_name, 0, sizeof(thread_name));
        }

        ~RunnableReference() {
            delete target;
            MTAF_ASSERT(0 == count);
            MTAF_ASSERT(isended);
        }

        void AddRef() { count++; }
        void RemoveRef(ScopedSpinLock &_lock) {
            MTAF_ASSERT(0 < count);
            MTAF_ASSERT(_lock.islocked());

            bool willdel = false;
            count--;

            if (0 == count) willdel = true;

            _lock.unlock();

            if (willdel) delete this;
        }

      private:
        RunnableReference(const RunnableReference &);
        RunnableReference &operator=(const RunnableReference &);

      public:
        Runnable *target;
        void *arg;
        int count;
        thread_tid tid;
        bool isjoined;
        bool isended;
        long aftertime;
        long periodictime;
        bool iscanceldelaystart;
        Condition condtime;
        SpinLock splock;
        bool isinthread; // 猥琐的东西，是为了解决线程还没有起来的时就发送信号出现crash的问题
        int killsig;
        char thread_name[128];
    };

  public:
    Thread(void (*fn)(void *), void *arg, const char *_thread_name = NULL)
        : runable_ref_(NULL) {
        runable_ref_ = new RunnableReference(detail::transform(fn));
        ScopedSpinLock lock(runable_ref_->splock);
        runable_ref_->AddRef();
        runable_ref_->arg = arg;

        int res = pthread_attr_init(&attr_);
        MTAF_ASSERT2(0 == res, "res=%d", res);
        if (_thread_name) strncpy(runable_ref_->thread_name, _thread_name, sizeof(runable_ref_->thread_name));
    }

    template <class T>
    explicit Thread(const T &op, const char *_thread_name = NULL)
        : runable_ref_(NULL) {
        runable_ref_ = new RunnableReference(detail::transform(op));
        ScopedSpinLock lock(runable_ref_->splock);
        runable_ref_->AddRef();
        runable_ref_->arg = NULL;

        int res = pthread_attr_init(&attr_);
        MTAF_ASSERT2(0 == res, "res=%d", res);
        if (_thread_name) strncpy(runable_ref_->thread_name, _thread_name, sizeof(runable_ref_->thread_name));
    }

    Thread(const char *_thread_name = NULL)
        : runable_ref_(NULL) {
        runable_ref_ = new RunnableReference(NULL);
        ScopedSpinLock lock(runable_ref_->splock);
        runable_ref_->AddRef();
        runable_ref_->arg = NULL;

        int res = pthread_attr_init(&attr_);
        MTAF_ASSERT2(0 == res, "res=%d", res);
        if (_thread_name) strncpy(runable_ref_->thread_name, _thread_name, sizeof(runable_ref_->thread_name));
    }

    virtual ~Thread() {
        int res = pthread_attr_destroy(&attr_);
        MTAF_ASSERT2(0 == res, "res=%d", res);
        ScopedSpinLock lock(runable_ref_->splock);
        runable_ref_->RemoveRef(lock);
    }

    int start(bool *_newone = NULL) {
        ScopedSpinLock lock(runable_ref_->splock);

        if (_newone) *_newone = false;

        if (isruning()) return 0;

        MTAF_ASSERT(runable_ref_->target);
        runable_ref_->isended = false;
        runable_ref_->AddRef();

        int ret = pthread_create(reinterpret_cast<thread_tid *>(&runable_ref_->tid), &attr_, start_routine, runable_ref_);
        MTAF_ASSERT(0 == ret);

        if (_newone) *_newone = true;

        if (0 != ret) {
            runable_ref_->isended = true;
            runable_ref_->RemoveRef(lock);
        }

        return ret;
    }

    template <typename T>
    int start(const T &op, bool *_newone = NULL) {
        ScopedSpinLock lock(runable_ref_->splock);

        if (_newone) *_newone = false;

        if (isruning()) return 0;

        delete runable_ref_->target;
        runable_ref_->target = detail::transform(op);

        runable_ref_->isended = false;
        runable_ref_->AddRef();

        int ret = pthread_create(reinterpret_cast<thread_tid *>(&runable_ref_->tid), &attr_, start_routine, runable_ref_);
        MTAF_ASSERT(0 == ret);

        if (_newone) *_newone = true;

        if (0 != ret) {
            runable_ref_->isended = true;
            runable_ref_->RemoveRef(lock);
        }

        return ret;
    }

    int start_after(long after) {
        ScopedSpinLock lock(runable_ref_->splock);

        if (isruning()) return 0;

        MTAF_ASSERT(runable_ref_->target);
        runable_ref_->condtime.cancelAnyWayNotify();
        runable_ref_->iscanceldelaystart = false;
        runable_ref_->isended = false;
        runable_ref_->aftertime = after;
        runable_ref_->AddRef();

        int ret = pthread_create(reinterpret_cast<thread_tid *>(&runable_ref_->tid), &attr_, start_routine_after, runable_ref_);
        MTAF_ASSERT(0 == ret);

        if (0 != ret) {
            runable_ref_->isended = true;
            runable_ref_->aftertime = LONG_MAX;
            runable_ref_->RemoveRef(lock);
        }

        return ret;
    }

    void cancel_after() {
        ScopedSpinLock lock(runable_ref_->splock);

        if (!isruning()) return;

        runable_ref_->iscanceldelaystart = true;
        runable_ref_->condtime.notifyAll(true);
    }

    int start_periodic(long after, long periodic) { // ms
        ScopedSpinLock lock(runable_ref_->splock);

        if (isruning()) return 0;

        MTAF_ASSERT(runable_ref_->target);
        runable_ref_->condtime.cancelAnyWayNotify();
        runable_ref_->iscanceldelaystart = false;
        runable_ref_->isended = false;
        runable_ref_->aftertime = after;
        runable_ref_->periodictime = periodic;
        runable_ref_->AddRef();

        int ret = pthread_create(reinterpret_cast<thread_tid *>(&runable_ref_->tid), &attr_, start_routine_periodic, runable_ref_);
        MTAF_ASSERT(0 == ret);

        if (0 != ret) {
            runable_ref_->isended = true;
            runable_ref_->aftertime = LONG_MAX;
            runable_ref_->periodictime = LONG_MAX;
            runable_ref_->RemoveRef(lock);
        }

        return ret;
    }

    void cancel_periodic() {
        ScopedSpinLock lock(runable_ref_->splock);

        if (!isruning()) return;

        runable_ref_->iscanceldelaystart = true;
        runable_ref_->condtime.notifyAll(true);
    }

    int join() const {
        int ret = 0;
        ScopedSpinLock lock(runable_ref_->splock);
        MTAF_ASSERT(!runable_ref_->isjoined);

        if (tid() == MTAppenderFile::ThreadUtil::currentthreadid()) return EDEADLK;

        if (isruning()) {
            runable_ref_->isjoined = true;
            lock.unlock();
            ret = pthread_join(tid(), 0);
            MTAF_ASSERT2(0 == ret || ESRCH == ret, "pthread_join err:%d", ret);
        }

        return ret;
    }

    void outside_join() const {
        ScopedSpinLock lock(runable_ref_->splock);
        MTAF_ASSERT(!runable_ref_->isjoined);
        MTAF_ASSERT(!isruning());
        if (runable_ref_->isjoined || isruning()) return;

        runable_ref_->isjoined = true;
    }

    int kill(int sig) const {
        ScopedSpinLock lock(runable_ref_->splock);

        if (!isruning()) return ESRCH;

        if (!runable_ref_->isinthread) {
            runable_ref_->killsig = sig;
            return 0;
        }

        lock.unlock();

        int ret = pthread_kill(tid(), sig);
        return ret;
    }

    int unsafe_exit() const {
        return pthread_cancel(tid());
    }

    thread_tid tid() const {
        return runable_ref_->tid;
    }

    bool isruning() const {
        return !runable_ref_->isended;
    }

    void stack_size(size_t _stacksize) {
        if (_stacksize == 0) return;

        int res = pthread_attr_setstacksize(&attr_, _stacksize);
        MTAF_ASSERT2(0 == res, "res=%d", res);
    }

    size_t stack_size() const {
        size_t _stacksize = 0;
        int res = pthread_attr_getstacksize(&attr_, &_stacksize);
        MTAF_ASSERT2(0 == res, "res=%d", res);
        return _stacksize;
    }

    const char *thread_name() const {
        return runable_ref_->thread_name;
    }

  private:
    static void init(void *arg) {
        volatile RunnableReference *runableref = static_cast<RunnableReference *>(arg);
        ScopedSpinLock lock((const_cast<RunnableReference *>(runableref))->splock);
        MTAF_ASSERT(runableref != 0);
        MTAF_ASSERT(runableref->target != 0);
        MTAF_ASSERT(!runableref->isinthread);

        runableref->isinthread = true;

        if (0 < strnlen((const char *)runableref->thread_name, sizeof(runableref->thread_name))) {
            pthread_setname_np((const char *)runableref->thread_name);
        }

        if (!(0 < runableref->killsig && runableref->killsig <= 32))
            return;

        lock.unlock();
        pthread_kill(pthread_self(), runableref->killsig);
    }

    static void cleanup(void *arg) {
        volatile RunnableReference *runableref = static_cast<RunnableReference *>(arg);
        ScopedSpinLock lock((const_cast<RunnableReference *>(runableref))->splock);

        MTAF_ASSERT(runableref != 0);
        MTAF_ASSERT(runableref->target != 0);
        MTAF_ASSERT(runableref->tid != 0);
        MTAF_ASSERT(runableref->isinthread);

        runableref->isinthread = false;
        runableref->killsig = 0;
        runableref->isended = true;

        if (!runableref->isjoined) pthread_detach(pthread_self());

        runableref->isjoined = false;
        (const_cast<RunnableReference *>(runableref))->RemoveRef(lock);
    }

    static void *start_routine(void *arg) {
        init(arg);
        volatile RunnableReference *runableref = static_cast<RunnableReference *>(arg);
        pthread_cleanup_push(&cleanup, arg);
        runableref->target->run(runableref->arg);
        pthread_cleanup_pop(1);
        return 0;
    }

    static void *start_routine_after(void *arg) {
        init(arg);
        volatile RunnableReference *runableref = static_cast<RunnableReference *>(arg);
        pthread_cleanup_push(&cleanup, arg);

        if (!runableref->iscanceldelaystart) {
            (const_cast<RunnableReference *>(runableref))->condtime.wait(runableref->aftertime);

            if (!runableref->iscanceldelaystart) {
                runableref->target->run(runableref->arg);
            }
        }

        pthread_cleanup_pop(1);
        return 0;
    }

    static void *start_routine_periodic(void *arg) {
        init(arg);
        volatile RunnableReference *runableref = static_cast<RunnableReference *>(arg);
        pthread_cleanup_push(&cleanup, arg);

        if (!runableref->iscanceldelaystart) {
            (const_cast<RunnableReference *>(runableref))->condtime.wait(runableref->aftertime);

            while (!runableref->iscanceldelaystart) {

                runableref->target->run(runableref->arg);

                if (!runableref->iscanceldelaystart)
                    (const_cast<RunnableReference *>(runableref))->condtime.wait(runableref->periodictime);
            }
        }

        pthread_cleanup_pop(1);
        return 0;
    }

  private:
    Thread(const Thread &);
    Thread &operator=(const Thread &);

  private:
    RunnableReference *runable_ref_;
    pthread_attr_t attr_;
};


inline bool operator==(const MTAppenderFile::Thread &lhs, const MTAppenderFile::Thread &rhs) {
    return pthread_equal(lhs.tid(), rhs.tid()) != 0;
}

inline bool operator!=(const MTAppenderFile::Thread &lhs, const MTAppenderFile::Thread &rhs) {
    return pthread_equal(lhs.tid(), rhs.tid()) == 0;
}

#endif /* MTAF_THREAD_H_ */
