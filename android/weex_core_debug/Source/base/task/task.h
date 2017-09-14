// Copyright 2017 The Weex Authors. All rights reserved.

#ifndef WEEX_BASE_TASK_TASK_H_
#define WEEX_BASE_TASK_TASK_H_

#include <queue>

#include "../scoped_ptr.h"
#include "callback.h"
#include "../threading/condition.h"

namespace base {
class Task {
 public:
    explicit Task(Clouse* clouse) : runnable_(clouse) {}

    Task(Task& other) : runnable_(other.runnable_) {}

    Task(const Task& other) : runnable_(const_cast<Task*>(&other)->runnable_) {}

    Task() {}

    ~Task() {}

    void Reset(Clouse* clouse) {
      runnable_.Reset(clouse);
    }

    void Run() { runnable_->Run(); }

    bool IsValid() {
        return runnable_.Get();
    }

 private:
    ScopedPtr<Clouse> runnable_;
};

class TaskQueue : public std::queue<Task> {
 public:
    void Swap(TaskQueue* queue) {
        c.swap(queue->c);
    }
};
}  // namespace base

#endif  // WEEX_BASE_TASK_TASK_H_
