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
#ifndef BASE_MESSAGE_LOOP_MESSAGE_PUMP_H
#define BASE_MESSAGE_LOOP_MESSAGE_PUMP_H
#include "base/time_unit.h"
#include "base/common.h"

namespace weex {
namespace base {
class MessagePump {
 public:
  class Delegate {
   public:
    virtual ~Delegate() {}
    virtual void DoWork() = 0;
  };
  MessagePump() {}
  virtual ~MessagePump() {}

  virtual void Run(Delegate* delegate) = 0;
  virtual void Stop() = 0;
  virtual void ScheduleWork() = 0;
  virtual void ScheduleDelayedWork(TimeUnit delayed_time) = 0;
    
 private:
  DISALLOW_COPY_AND_ASSIGN(MessagePump);
};
}  // namespace base
}  // namespace weex
#endif  // BASE_MESSAGE_LOOP_MESSAGE_PUMP_H
