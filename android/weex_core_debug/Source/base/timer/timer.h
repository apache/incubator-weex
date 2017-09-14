
#ifndef WEEX_BASE_TIMER_TIMER_H_
#define WEEX_BASE_TIMER_TIMER_H_

#include "timer_heap.h"

namespace base {
class Timer {
 public:
    void Loop();

    uint64_t NextTimeout() { return timer_heap_.NextTimeout(); }

    void SetTimerNode(TimerNode node) { timer_heap_.Push(node); }

 private:
    TimerHeap timer_heap_;
};
}  // namespace base

#endif  // WEEX_BASE_TIMER_TIMER_H_
