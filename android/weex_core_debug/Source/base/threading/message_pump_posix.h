// Copyright 2017 The Weex Authors. All rights reserved.

#ifndef WEEX_BASE_THREADING_MESSAGE_PUMP_POSIX_H_
#define WEEX_BASE_THREADING_MESSAGE_PUMP_POSIX_H_

#include "../task/task.h"
#include "condition.h"
#include "message_pump.h"
#include "../timer/timer.h"

namespace base {

class MessagePumpPosix : public MessagePump {
 public:
    MessagePumpPosix();

    virtual ~MessagePumpPosix();

    virtual void Run(Delegate* delegate);

    virtual void ScheduleWork();

    virtual void ScheduleDelayedWork(Clouse* clouse, int delayed_time);

    virtual void ScheduleIntervalWork(Clouse* clouse, int delayed_time);

 private:
    Condition condition_;

    Timer timer_;

    bool loop_running_;
};
}  // namespace base

#endif  // WEEX_BASE_THREADING_MESSAGE_PUMP_POSIX_H_
