// Copyright 2017 The Weex Authors. All rights reserved.

#ifndef WEEX_BASE_THREADING_THREAD_H_
#define WEEX_BASE_THREADING_THREAD_H_

#include <pthread.h>

#include "base/threading/message_loop.h"

namespace base {

class Thread {
 public:
    class ThreadParams {
     public:
        explicit ThreadParams(Thread* thread): thread_(thread) {}
        Thread* thread_;
    };

    explicit Thread(MessageLoop::MESSAGE_LOOP_TYPE type);

    virtual ~Thread();

    virtual void Start();

    virtual void Quit(base::Clouse* closue);

    void Run();

    void Join(const Thread& thread);

    MessageLoop* Looper() { return &message_loop_; }

    static Thread* current_thread() {
        return NULL;
    }
 private:
    void ThreadMain();
    pthread_t thread_handle_;
    MessageLoop message_loop_;
};

}  // namespace base
#endif  // WEEX_BASE_THREADING_THREAD_H_
