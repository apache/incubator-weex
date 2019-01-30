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
enum TaskPlatform {
  WEEXCORE,
  JSS_ENGINE
};

class TimeCalculator {
 public:
  TimeCalculator(TaskPlatform taskPlatform, std::string name, std::string id) :
      task_name(name),
      instance_id(id),
      end(TimePoint::Now()),
      start(TimePoint::Now()),
      task_end(TimePoint::Now()),
      task_start(TimePoint::Now()),
      task_id(genTaskId()) {
    if (taskPlatform == TaskPlatform::JSS_ENGINE) {
      task_platform = "jsengine";
    } else {
      task_platform = "weexcore";
    }
  }

  int genTaskId() {
    static int taskIdGenerator = 0;
    return __sync_fetch_and_add(&taskIdGenerator, 1);
  }

  ~TimeCalculator() {
    if (!task_end_flag) {
      task_end = TimePoint::Now();
    }
    end = TimePoint::Now();
    print();
  }

  void taskStart() {
//    LOGE(
//        "dyyLog %s taskName is %s : instanceId %s : task_id %d: taskStart",
//        task_platform.c_str(),
//        task_name.c_str(),
//        instance_id.c_str(),
//        task_id);
    this->task_start = TimePoint::Now();
  }

  void taskEnd() {
//    LOGE(
//        "dyyLog %s taskName is %s : instanceId %s : task_id %d: taskEnd",
//        task_platform.c_str(),
//        task_name.c_str(),
//        instance_id.c_str(),
//        task_id);
    this->task_end = TimePoint::Now();
    task_end_flag = true;
  }

  void set_task_name(std::string name) {
    this->task_name = name;
  }

  void print() {
    const TimeUnit &allCost = end.ToTimeUnit() - start.ToTimeUnit();
    const TimeUnit &taskWait = task_start.ToTimeUnit() - start.ToTimeUnit();
    const TimeUnit &taskCost = task_end.ToTimeUnit() - task_start.ToTimeUnit();

//    int64_t taskCostMS = taskCost.ToMilliseconds();
//    if (taskCostMS < 5) {
//      LOGD("dyyLog %s taskName is %s cost less than 5ms", task_platform.c_str(),
//           task_name.c_str());
//    } else {
//      std::string msg = "normal";
//
//      if (taskCostMS > 100) {
//        msg = "task cost than 100, ";
//      }
//
//      if (taskWait.ToMilliseconds() > 100) {
//        std::string a = "wait to long time than 100ms";
//        msg += a;
//      }
//
//      LOGE(
//          "dyyLog %s taskName is %s : instanceId %s : task_id %d: start : %lld  ---  end : %lld  ---  allCost:%lld  ---  taskCost:%lld  ---  taskWait:%lld --- msg:%s",
//          task_platform.c_str(),
//          task_name.c_str(),
//          instance_id.c_str(),
//          task_id,
//          start.ToTimeUnit().ToMilliseconds(),
//          end.ToTimeUnit().ToMilliseconds(),
//          allCost.ToMilliseconds(),
//          taskCostMS,
//          taskWait.ToMilliseconds(),
//          msg.c_str());
//    }
  }

 private:
  std::string task_name;
  int task_id;
  std::string instance_id;
  TimePoint start;
  TimePoint end;
  TimePoint task_start;
  TimePoint task_end;
  bool task_end_flag = false;
  std::string task_platform;
};
}  // namespace base
}  // namespace weex

#endif //WEEX_PROJECT_TIME_CALCULATOR_H
