

#include "timer.h"

namespace base {

void Timer::Loop() {
    while (!timer_heap_.IsEmpty()
            && timer_heap_.NextTimeout() <= CurrentTimeMillis()) {
        TimerNode node = timer_heap_.Pop();
        node.task_.Run();
        if (node.need_loop_) {
            node.next_timeout_ += node.interval_time_;
            timer_heap_.Push(node);
        }
    }
}

}  // namespace base
