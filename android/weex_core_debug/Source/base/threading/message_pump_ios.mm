// Copyright 2017 The Weex Authors. All rights reserved.

#include "message_pump_ios.h"

namespace base {
    const CFStringRef kMessageLoopExclusiveRunLoopMode =
    CFSTR("kMessageLoopExclusiveRunLoopMode");
    
    void CallWithEHFrame(void (^block)(void)) {
        block();
    }
    
    MessagePumpIOS::MessagePumpIOS(Delegate* delegate) : delegate_(delegate) {
        run_loop_ = CFRunLoopGetCurrent();
        CFRetain(run_loop_);
        CFRunLoopSourceContext source_context = CFRunLoopSourceContext();
        source_context.info = this;
        source_context.perform = RunWorkSource;
        work_source_ = CFRunLoopSourceCreate(NULL,  // allocator
                                             1,     // priority
                                             &source_context);
        CFRunLoopAddSource(run_loop_, work_source_, kCFRunLoopCommonModes);
        CFRunLoopAddSource(run_loop_, work_source_, kMessageLoopExclusiveRunLoopMode);
    }
    
    MessagePumpIOS::~MessagePumpIOS() {
        CFRunLoopRemoveSource(run_loop_, work_source_, kCFRunLoopCommonModes);
        CFRunLoopRemoveSource(run_loop_, work_source_, kMessageLoopExclusiveRunLoopMode);
        CFRelease(work_source_);
        
        CFRelease(run_loop_);
    }
    
    void MessagePumpIOS::RunWorkSource(void* info) {
        MessagePumpIOS* self = static_cast<MessagePumpIOS*>(info);
        CallWithEHFrame(^{
            self->RunWork();
        });
    }
    
    void MessagePumpIOS::RunWork() {
        delegate_->DoWork();
    }
    
    void MessagePumpIOS::ScheduleWork() {
        CFRunLoopSourceSignal(work_source_);
        CFRunLoopWakeUp(run_loop_);
    }
}