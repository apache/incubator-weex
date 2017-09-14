// Copyright 2017 The Weex Authors. All rights reserved.

#ifndef MESSAGE_PUMP_IOS_H_
#define MESSAGE_PUMP_IOS_H_

#include "message_pump.h"

#include <CoreFoundation/CoreFoundation.h>
#import <Foundation/Foundation.h>

namespace base {
    class MessagePumpIOS : public MessagePump{
    public:
        MessagePumpIOS(Delegate* delegate);

        virtual ~MessagePumpIOS();

        virtual void Run(Delegate* delegate) {}

        virtual void ScheduleWork();

        virtual void ScheduleDelayedWork(Clouse* clouse, int delayed_time) {}

        virtual void ScheduleIntervalWork(Clouse* clouse, int delayed_time) {}
    private:
        static void RunWorkSource(void* info);
        void RunWork();
        CFRunLoopRef run_loop_;
        CFRunLoopSourceRef work_source_;
        Delegate* delegate_;
    };
}

#endif /* MESSAGE_PUMP_IOS_H_ */
