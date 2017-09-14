// Copyright 2017 The Weex Authors. All rights reserved.

#include <iostream>

#if OS_ANDROID
#include "../android/android_jni.h"
#endif

#include "base/threading/thread.h"

namespace base {

void* ThreadFunc(void* params) {
    Thread* thread = static_cast<Thread*>(params);
    thread->Run();
#if OS_ANDROID
    android::DetachFromVM();
#endif
    return NULL;
}

Thread::Thread(MessageLoop::MESSAGE_LOOP_TYPE type) : message_loop_(type) {
}

Thread::~Thread() {
}


void Thread::Start() {
    bool err = pthread_create(&thread_handle_, NULL, ThreadFunc, this);
    if (err) {
        std::cout << "thread start failed!!!" << std::endl;
    }
}

void Thread::Join(const Thread& thread) {
    pthread_join(thread.thread_handle_, NULL);
}

void Thread::Run() {
    message_loop_.Run();
}

    void Thread::Quit(base::Clouse* closue) {
        message_loop_.Quit(closue);
    }


}  // namespace base
