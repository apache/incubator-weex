// Copyright 2017 The Weex Authors. All rights reserved.

#ifndef WEEX_BASE_THREADING_MESSAGE_LOOP_H_
#define WEEX_BASE_THREADING_MESSAGE_LOOP_H_

#if OS_ANDROID
#include <queue>
#endif

#include "message_pump.h"

#include "../scoped_ptr.h"
#include "../task/task.h"
#include "condition.h"
#include "lock.h"

namespace base {
class MessageLoop : public MessagePump::Delegate {
 public:
    enum MESSAGE_LOOP_TYPE {
        MESSAGE_LOOP_UI,
        MESSAGE_LOOP_POSIX,
    };
    explicit MessageLoop(MESSAGE_LOOP_TYPE type = MESSAGE_LOOP_POSIX);
    void PostTask(Clouse* clouse);
    void PostDelayedTask(Clouse* clouse, int delayed_time);
    void PostIntervalTask(Clouse* clouse, int delayed_time);
    virtual bool DoWork();
    virtual void DoQuit();
    void Run();
    void Quit(base::Clouse* closue);
    void Stop();
 private:
    MessagePump* CreatePump(MESSAGE_LOOP_TYPE type);
    TaskQueue incoming_task_queue_;
    TaskQueue working_task_queue_;
    bool loop_running_;
    Task quit_task_;

    Lock lock_;
    MESSAGE_LOOP_TYPE loop_type_;
    ScopedPtr<MessagePump> pump_;
    WeakPtr<MessageLoop> weak_ptr_;
};
}  // namespace base

#endif  // WEEX_BASE_THREADING_MESSAGE_LOOP_H_
