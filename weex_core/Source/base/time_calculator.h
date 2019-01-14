//
// Created by 董亚运 on 2019/1/14.
//

#ifndef WEEX_PROJECT_TIME_CALCULATOR_H
#define WEEX_PROJECT_TIME_CALCULATOR_H

#include <string>
#include "time_point.h"
#include "time_unit.h"
#include "android/log_utils.h"

namespace weex {
namespace base {
class TimeCalculator {
 public:
  TimeCalculator(std::string name) : task_name(name), end(TimePoint::Now()),
                                     start(TimePoint::Now()),
                                     task_end(TimePoint::Now()),
                                     task_start(TimePoint::Now()) {
  }


  ~TimeCalculator() {
    if (!task_end_flag) {
      task_end = TimePoint::Now();
    }
    end = TimePoint::Now();
    print();
  }

  void taskStart() {
    this->task_start = TimePoint::Now();
  }

  void taskEnd() {
    this->task_end = TimePoint::Now();
    task_end_flag = true;
  }

  void print() {
    const TimeUnit &allCost = end.ToTimeUnit() - start.ToTimeUnit();
    const TimeUnit &taskWait = task_start.ToTimeUnit() - start.ToTimeUnit();
    const TimeUnit &taskCost = task_end.ToTimeUnit() - task_start.ToTimeUnit();


    LOGE("taskName is %s : start : %lld  ---  end : %lld  ---  allCost:%lld  ---  taskCost:%lld  ---  taskWait:%lld",
         task_name.c_str(),
         start.ToTimeUnit().ToMilliseconds(),
         end.ToTimeUnit().ToMilliseconds(),
         allCost.ToMilliseconds(),
         taskCost.ToMilliseconds(),
         taskWait.ToMilliseconds());
  }

 private:
  std::string task_name;
  TimePoint start;
  TimePoint end;
  TimePoint task_start;
  TimePoint task_end;
  bool task_end_flag = false;
};
}  // namespace base
}  // namespace weex

#endif //WEEX_PROJECT_TIME_CALCULATOR_H
