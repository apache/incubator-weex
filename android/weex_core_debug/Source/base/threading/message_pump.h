// Copyright 2017 The Weex Authors. All rights reserved.

#ifndef WEEX_BASE_THREADING_MESSAGE_PUMP_H_
#define WEEX_BASE_THREADING_MESSAGE_PUMP_H_

#include "../task/callback.h"

namespace base {

class MessagePump {
 public:
    class Delegate {
     public:
        virtual ~Delegate() {}
        virtual bool DoWork() = 0;
        virtual void DoQuit() = 0;
    };

    MessagePump() {}

    virtual ~MessagePump() {}

    virtual void Run(Delegate* delegate) {}

    virtual void Stop() {}

    virtual void ScheduleWork() = 0;

    virtual void ScheduleDelayedWork(Clouse* clouse, int delayed_time) = 0;

    virtual void ScheduleIntervalWork(Clouse* clouse, int delayed_time) = 0;
};
}  // namespace base

#endif  // WEEX_BASE_THREADING_MESSAGE_PUMP_H_
