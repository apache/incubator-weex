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
#include "time_calculator.h"
#include "log_defines.h"
namespace weex {
namespace base {
void weex::base::TimeCalculator::taskEnd() {
  if (!turnOn()) {
    return;
  }

  LOG_Performance(m_instance_id_.c_str(),
      "%s taskName is %s : instanceId %s : m_task_id_ %d: taskEnd",
      m_task_platform_.c_str(),
      m_task_name_.c_str(),
      m_instance_id_.c_str(),
      m_task_id_);
  this->m_task_end_time_ = getCurrentTime();
  m_task_end_flag_ = true;
}
void weex::base::TimeCalculator::taskStart() {

  if (!turnOn()) {
    return;
  }


  LOG_Performance(m_instance_id_.c_str(),
      "timeline %s taskName is %s : instanceId %s : m_task_id_ %d: taskStart",
      m_task_platform_.c_str(),
      m_task_name_.c_str(),
      m_instance_id_.c_str(),
      m_task_id_);
  this->m_task_start_time_ = getCurrentTime();
}
void weex::base::TimeCalculator::print() {
  if (!turnOn()) {
    return;
  }

  const long long &taskWait = m_task_start_time_ - m_constructor_time_;
  const long long &taskCost = m_task_end_time_ - m_task_start_time_;

  bool showLog = false;

  if (!args.empty() || taskWait > 100) {
    showLog = true;
  } else if (taskCost < 5) {
    LOG_Performance(m_instance_id_.c_str(),"timeline %s taskName is %s cost less than 5ms", m_task_platform_.c_str(),
         m_task_name_.c_str());
  } else {
    showLog = true;
    std::string msg = "normal";
    if (taskCost > 100) {
      msg = "task cost than 100, ";
    }

    if (taskWait > 100) {
      std::string a = "wait to long time than 100ms";
      msg += a;
    }
  }

  if (showLog) {
    LOG_Performance(m_instance_id_.c_str(),"timeline taskName: %s, result: %s", m_task_name_.c_str(), formatData().c_str());
  }
}
void weex::base::TimeCalculator::transform() {
  if (!turnOn()) {
    return;
  }
  LOG_Performance(m_instance_id_.c_str(), "%s", formatData().c_str());
}
std::string weex::base::TimeCalculator::formatData() {
  if (!turnOn()) {
    return "";
  }
  char buffer[1024];
  std::string format =
      "{\"time\":{\"execTime\":%lld,\"waitTime\":%lld,\"constructor\":%lld,\"destructor\":%lld,\"taskStart\":%lld,\"taskEnd\":%lld},"
      "\"Info\":{\"platform\":\"%s\",\"taskId\":%d,\"taskInfo\":{\"relateTaskId\":%d, \"args\":\"%s\",},\"taskName\":\"%s\"}}";

  snprintf(buffer, 1024, format.c_str(),
           m_task_end_time_ - m_task_start_time_,
           m_task_start_time_ - m_constructor_time_,
           m_constructor_time_,
           m_destructor_time_,
           m_task_start_time_,
           m_task_end_time_,
           m_task_platform_.c_str(),
           m_task_id_,
           m_relative_task_id_,
           args.c_str(),
           m_task_name_.c_str());

  return std::string(buffer);
}
}  // namespace base
}  // namespace weex


