//
//  timer_heap.hpp
//  samples
//
//  Created by dli on 11/2/16.
//
//

#ifndef WEEX_BASE_TIMER_TIMER_HEAP_H_
#define WEEX_BASE_TIMER_TIMER_HEAP_H_

#include <vector>

#include "timer_node.h"
#include "../threading/lock.h"

namespace base {
class TimerHeap {
 public:
    TimerHeap(): lock_() {}
    ~TimerHeap() {}
    uint64_t NextTimeout() { return !min_heap_.empty() ? min_heap_[0].next_timeout_ : ULONG_MAX; }
    bool IsEmpty() { return min_heap_.empty(); }
    void Remove();
    TimerNode Pop();
    void Push(TimerNode node);
 private:
    void ShiftUp(int start);
    void ShiftDown(int start);

    std::vector<TimerNode> min_heap_;

    Lock lock_;
};
}  // namespace base

#endif  // WEEX_BASE_TIMER_TIMER_HEAP_H_
