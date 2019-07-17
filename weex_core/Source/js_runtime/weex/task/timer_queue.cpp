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
// Created by Darin on 12/06/2018.
//

#include "timer_queue.h"
//#include "android/jsengine/task/weex_task_queue.h"
#include "weex_task_queue.h"
#include "android/jsengine/object/weex_env.h"
//#include "android/jsengine/weex_runtime.h"
#include "js_runtime/weex/object/weex_runtime.h"

static void *startThread(void *td) {
  auto *self = static_cast<TimerQueue *>(td);
  self->start();
  return NULL;
}

void TimerQueue::init() {

  if (this->isInit)
    return;

  this->isInit = true;

  pthread_t thread;
  pthread_create(&thread, nullptr, startThread, this);
  pthread_setname_np(thread, "TimerQueueThread");
}

void TimerQueue::start() {
  while (true) {
    auto pTask = getTask();
    LOGE("getTask return task");
    bool ptaskHasGlobalObject;
    ptaskHasGlobalObject = weexTaskQueue->weexRuntime->hasInstanceId(pTask->instanceID);
    //LOGE("[weex-task]getTask return task,ptaskHasGlobalObject :%d ",ptaskHasGlobalObject);

    if (ptaskHasGlobalObject && weexTaskQueue->weexRuntime->hasInstanceId(pTask->instanceID)) {
      //  LOGE("[weex-task]before add  to weexTaskQueue");
      weexTaskQueue->addTimerTask(pTask->instanceID,
                                  pTask->m_function,
                                  pTask->taskId,
                                  !pTask->repeat,
                                  pTask->from_instance_);
      //LOGE("[weex-task]end add  to weexTaskQueue");
      if (pTask->repeat && ptaskHasGlobalObject
          && weexTaskQueue->weexRuntime->hasInstanceId(pTask->instanceID)) {
        LOGE("repreat");
        addTimerTask(new TimerTask(pTask));
      }
    }
    delete (pTask);
    pTask = nullptr;
  }
}

TimerQueue::TimerQueue(WeexTaskQueue *taskQueue) {
  nextTaskWhen = 0;
  this->weexTaskQueue = taskQueue;
  init();
}

int TimerQueue::addTimerTask(TimerTask *timerTask) {
  threadLocker.lock();
  auto size = timerQueue_.size();
  if (timerQueue_.empty()) {
    timerQueue_.push_back(timerTask);
  } else {
    auto begin = timerQueue_.begin();
    int i = 0;
    for (; i < size; ++i) {
      auto it = timerQueue_[i];
      if (timerTask->when < it->when) {
        timerQueue_.insert(begin + i, timerTask);
        break;
      } else if (i == size - 1) {
        timerQueue_.push_back(timerTask);
        break;
      }
    }
  }

  size = timerQueue_.size();
  if (size > 0) {
    nextTaskWhen = timerQueue_.front()->when;
  }
  threadLocker.unlock();
  threadLocker.signal();
  return size;
}

//对比时间
TimerTask *TimerQueue::getTask() {
  TimerTask *task = nullptr;
  while (task == nullptr) {
    threadLocker.lock();
    while (timerQueue_.empty() || microTime() < nextTaskWhen) {
      if (timerQueue_.empty()) {
        threadLocker.wait();
      } else {
        auto i = threadLocker.waitTimeout(nextTaskWhen);
        if (i == ETIMEDOUT) {
          break;
        }
      }
    }

    if (timerQueue_.empty()) {
      threadLocker.unlock();
      continue;
    }
    assert(!taskQueue_.empty());
    TimerTask *header = timerQueue_.front();
    nextTaskWhen = header->when;
    if (microTime() > nextTaskWhen) {
      timerQueue_.pop_front();
      task = header;
    } else {
      threadLocker.unlock();
      continue;
    }
    threadLocker.unlock();
  }
  return task;
}

void TimerQueue::removeTimer(int timerId) {
  threadLocker.lock();
  if (timerQueue_.empty()) {
    threadLocker.unlock();
    return;
  } else {
    for (auto it = timerQueue_.begin(); it < timerQueue_.end(); ++it) {
      auto reference = *it;
      if (reference->taskId == timerId) {
        timerQueue_.erase(it);
        weexTaskQueue->removeTimer(reference->taskId);
        if (weexTaskQueue->weexRuntime) {
          weexTaskQueue->weexRuntime->removeTimerFunctionForRunTimeApi(reference->instanceID,
                                                                       reference->m_function,
                                                                       reference->from_instance_);
        }

        delete (reference);
        reference = nullptr;
      }
    }
  }

  int size = timerQueue_.size();
  if (size > 0) {
    nextTaskWhen = timerQueue_.front()->when;
  }
  threadLocker.unlock();
  threadLocker.signal();
}

void TimerQueue::destroyPageTimer(std::string instanceId) {
  threadLocker.lock();
  if (timerQueue_.empty()) {
    threadLocker.unlock();
    return;
  } else {
    for (std::deque<TimerTask *>::iterator it = timerQueue_.begin(); it < timerQueue_.end(); ++it) {
      auto reference = *it;
      if (reference->instanceID == instanceId) {
        timerQueue_.erase(it);
        weexTaskQueue->removeTimer(reference->taskId);
        if (weexTaskQueue->weexRuntime) {
          weexTaskQueue->weexRuntime->removeTimerFunctionForRunTimeApi(reference->instanceID,
                                                                       reference->m_function,
                                                                       reference->from_instance_);
        }

        delete (reference);
        reference = nullptr;
      }
    }
  }

  int size = timerQueue_.size();
  if (size > 0) {
    nextTaskWhen = timerQueue_.front()->when;
  }
  threadLocker.unlock();
  threadLocker.signal();
}
