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
// Created by Darin on 2019/1/14.
//

#ifndef WEEX_PROJECT_TIME_CALCULATOR_H
#define WEEX_PROJECT_TIME_CALCULATOR_H

#include <string>
#include <mutex>
#include "time_point.h"
#include "time_unit.h"
#include "time_utils.h"
#include "base/log_defines.h"

namespace weex {
namespace base {
enum TaskPlatform {
  WEEXCORE,
  JSS_ENGINE
};

enum Level {
  HIGH,
  MEDIUM,
  LOW,
};

class TimeCalculator {
 public:
  TimeCalculator(TaskPlatform taskPlatform, std::string name, std::string id, Level level = LOW) :
      m_task_name_(name),
      m_instance_id_(id),
      m_destructor_time_(getCurrentTime()),
      m_constructor_time_(getCurrentTime()),
      m_task_end_time_(getCurrentTime()),
      m_task_start_time_(getCurrentTime()),
      m_task_id_(genTaskId()),
      m_relative_task_id_(0) {
    if (taskPlatform == TaskPlatform::JSS_ENGINE) {
      m_task_platform_ = "JSEngine";
    } else {
      m_task_platform_ = "WeexCore";
    }
  }

  int genTaskId() {
    static int taskIdGenerator = 0;
    return __sync_fetch_and_add(&taskIdGenerator, 1);
  }

  ~TimeCalculator() {
    if (!m_task_end_flag_) {
      m_task_end_time_ = getCurrentTime();
    }
    m_destructor_time_ = getCurrentTime();
    print();
//    transform();
  }

  void taskStart();

  void taskEnd();

  void set_task_name(std::string name) {
    this->m_task_name_ = name;
  }

  void setArgs(std::string args) {
    this->args = args;
  }

  std::string formatData();
  void transform();
  void print();

  //println
  bool turnOn() {
    return weex::base::LogImplement::getLog()->perfMode();
  }

 private:
  std::string m_task_name_;
  std::string m_log_tag_;
  std::string m_log_level_;
  int m_task_id_;
  int m_relative_task_id_;
  std::string m_instance_id_;
  long long m_constructor_time_;
  long long m_destructor_time_;
  long long m_task_start_time_;
  long long m_task_end_time_;
  bool m_task_end_flag_ = false;
  std::string m_task_platform_;
  std::string m_final_info_string_;
  std::string args;

};
}  // namespace base
}  // namespace weex

#endif //WEEX_PROJECT_TIME_CALCULATOR_H
