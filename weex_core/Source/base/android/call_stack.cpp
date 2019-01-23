/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#include "base/android/call_stack.h"

#include <android/log.h>
#include <dlfcn.h>
#include <unwind.h>
#include <iostream>
#include <sstream>
#include <ostream>

namespace base {

struct State {
    State(uintptr_t* frames, size_t max_count)
        : stack_frames_(frames),
          stack_count_(0),
          max_count_(max_count) {}
    uintptr_t* stack_frames_;
    size_t stack_count_;
    size_t max_count_;
};
static _Unwind_Reason_Code CallStackCallBack(_Unwind_Context* context, void* arg) {
    State* state = static_cast<State*>(arg);
    uintptr_t ip = _Unwind_GetIP(context);
    state->stack_frames_[state->stack_count_++] = ip;
    if (state->stack_count_ > CallStack::kMaxCount)
        return _URC_END_OF_STACK;
    return _URC_NO_REASON;
}

CallStack::CallStack()
    : CallStack(kMaxCount) {
    
}

CallStack::CallStack(size_t count) {
    count_ = std::min(count, kMaxCount);
    State state(reinterpret_cast<uintptr_t*>(stack_), count_);
    _Unwind_Backtrace(&CallStackCallBack, &state);
    count_ = state.stack_count_;
}

std::string CallStack::ToString() {
    std::ostringstream os;
    for (int i = 0; i < count_; i++) {
        Dl_info info;
        uintptr_t address = reinterpret_cast<const uintptr_t>(stack_[i]) - 1;
        if (dladdr(reinterpret_cast<const void*>(address), &info)) {
            if (info.dli_sname)
                os << std::hex << address << " : " << info.dli_sname << std::endl;
        } else
            os << std::hex << address << " : " << "Unknow symbol" << std::endl;
    }
    return os.str();
}

void CallStack::LogOut() {
    std::string stack_str = ToString();
    __android_log_write(ANDROID_LOG_ERROR, "CALLSTACK", stack_str.c_str());
}

}
