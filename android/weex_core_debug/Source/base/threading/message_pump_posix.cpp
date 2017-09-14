// Copyright 2017 The Weex Authors. All rights reserved.

#include "base/threading/message_pump_posix.h"

namespace base {

MessagePumpPosix::MessagePumpPosix() : condition_(), loop_running_(true) {}

MessagePumpPosix::~MessagePumpPosix() {}

void MessagePumpPosix::ScheduleWork() {
    condition_.Singal();
}

void MessagePumpPosix::ScheduleDelayedWork(Clouse* clouse, int delayed_time) {
    TimerNode node(clouse, delayed_time);
    timer_.SetTimerNode(node);
    condition_.Singal();
}

void MessagePumpPosix::ScheduleIntervalWork(Clouse* clouse, int delayed_time) {
    TimerNode node(clouse, delayed_time, true);
    timer_.SetTimerNode(node);
    condition_.Singal();
}

void MessagePumpPosix::Run(Delegate *delegate) {
    while (loop_running_) {
        timer_.Loop();
        loop_running_ = delegate->DoWork();
        if(loop_running_) {
            timer_.Loop();
            condition_.Wait(timer_.NextTimeout() - CurrentTimeMillis());
        }
    }
    delegate->DoQuit();
}
}  // namespace base
