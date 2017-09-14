

#include "timer_heap.h"

namespace base {

TimerNode TimerHeap::Pop() {
    AutoLock lock(lock_);
    TimerNode node = min_heap_[0];
    min_heap_[0] = min_heap_.back();
    min_heap_.pop_back();
    ShiftDown(0);
    return node;
}

void TimerHeap::Push(TimerNode node) {
    AutoLock lock(lock_);
    min_heap_.push_back(node);
    ShiftUp(0);
}

void TimerHeap::ShiftUp(int start) {
    int current = start;
    int parent = (current - 1) / 2;
    while (current > 0 &&
        min_heap_[current].next_timeout_ < min_heap_[parent].next_timeout_) {
        TimerNode node = min_heap_[current];
        min_heap_[current] =  min_heap_[parent];
        min_heap_[parent] = node;
        current = parent;
        parent = (current - 1) / 2;
    }
}

void TimerHeap::ShiftDown(int start) {
    if (min_heap_.empty())
        return;
    while (true) {
        int left_child = start * 2 + 1;
        int right_child = start * 2 + 2;
        int index = 0;
        if (right_child <= (min_heap_.size() - 1)) {
            if (min_heap_[start].next_timeout_ <=
                    min_heap_[left_child].next_timeout_
                    && min_heap_[start].next_timeout_ <=
                    min_heap_[right_child].next_timeout_) {
                break;
            } else {
                if (min_heap_[left_child].next_timeout_ <
                        min_heap_[right_child].next_timeout_) {
                    index = left_child;
                } else {
                    index = right_child;
                }
            }
        } else if (left_child == min_heap_.size() - 1) {
            if (min_heap_[left_child].next_timeout_ <
                    min_heap_[start].next_timeout_) {
                index = left_child;
            } else {
                break;
            }
        } else {
            break;
        }

        TimerNode node = min_heap_[index];
        min_heap_[index] = min_heap_[start];
        min_heap_[start] = node;
        start = index;
    }
}
}  // namespace base
