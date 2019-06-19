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
// Created by Darin on 23/05/2018.
//

#include "weex_task_queue.h"

#include <unistd.h>
#include "android/jsengine/task/impl/native_timer_task.h"
#include "android/jsengine/bridge/script/script_bridge_in_multi_process.h"
#include "android/jsengine/bridge/script/core_side_in_multi_process.h"
#include "android/jsengine/object/weex_env.h"
#include "android/jsengine/weex_jsc_utils.h"
void WeexTaskQueue::run(WeexTask *task) {
    if(task == nullptr || WeexEnv::getEnv()->is_app_crashed()) {
        return;
    }
    task->timeCalculator->set_task_name(task->taskName());
    task->timeCalculator->taskStart();
    task->run(weexRuntime);
    task->timeCalculator->taskEnd();
    delete task;
}


WeexTaskQueue::~WeexTaskQueue() {
    delete this->weexRuntime;
    weexRuntime = nullptr;
}

int WeexTaskQueue::addTask(WeexTask *task) {
    return _addTask(task, false);
}


WeexTask *WeexTaskQueue::getTask() {
    WeexTask *task = nullptr;
    while (task == nullptr) {
        threadLocker.lock();
        while (taskQueue_.empty() || !isInitOk) {
            threadLocker.wait();
        }

        if (taskQueue_.empty()) {
            threadLocker.unlock();
            continue;
        }

        if(WeexEnv::getEnv()->is_app_crashed()) {
          threadLocker.unlock();
          return nullptr;
        }

        assert(!taskQueue_.empty());
        task = taskQueue_.front();
        taskQueue_.pop_front();
        threadLocker.unlock();
    }

    return task;
}

int WeexTaskQueue::addTimerTask(String id, uint32_t function, int taskId, WeexGlobalObject* global_object, bool one_shot) {
    WeexTask *task = new NativeTimerTask(id, function,taskId, one_shot);
    task->set_global_object(global_object);
    return _addTask(
            task,
            false);
}

void WeexTaskQueue::removeTimer(int taskId) {
    threadLocker.lock();
    if (taskQueue_.empty()) {
        threadLocker.unlock();
        return;
    } else {
        for (std::deque<WeexTask *>::iterator it = taskQueue_.begin(); it < taskQueue_.end(); ++it) {
            auto reference = *it;
            if (reference->taskId == taskId) {
                NativeTimerTask* timer_task = static_cast<NativeTimerTask*>(reference);
                taskQueue_.erase(it);
                delete (timer_task);
            }
        }
    }
    threadLocker.unlock();
    threadLocker.signal();
}

void WeexTaskQueue::start() {
    while (true) {
        auto pTask = getTask();
        if (pTask == nullptr)
            continue;
        run(pTask);
    }
}


static void *startThread(void *td) {
    auto *self = static_cast<WeexTaskQueue *>(td);
    self->isInitOk = true;

    if (self->weexRuntime == nullptr) {
        self->weexRuntime = new WeexRuntime(new TimerQueue(self),WeexEnv::getEnv()->scriptBridge(), self->isMultiProgress);
        // init IpcClient in Js Thread
//        if (self->isMultiProgress) {
//            auto *client = new WeexIPCClient(WeexEnv::getEnv()->getIpcClientFd());
//            static_cast<weex::bridge::js::CoreSideInMultiProcess *>(weex::bridge::js::ScriptBridgeInMultiProcess::Instance()->core_side())->set_ipc_client(
//                    client);
//        }
    }

    auto pTask = self->getTask();
    self->run(pTask);
    self->start();
    return NULL;
}

void WeexTaskQueue::init() {
    pthread_t thread;
    LOGE("start weex queue init");
    pthread_create(&thread, nullptr, startThread, this);
    pthread_setname_np(thread, "WeexTaskQueueThread");
}

int WeexTaskQueue::_addTask(WeexTask *task, bool front) {
    threadLocker.lock();
    if (front) {
        taskQueue_.push_front(task);
    } else {
        taskQueue_.push_back(task);
    }

    int size = taskQueue_.size();
    threadLocker.unlock();
    threadLocker.signal();
    return size;
}

WeexTaskQueue::WeexTaskQueue(bool isMultiProgress) : weexRuntime(nullptr) {
    this->isMultiProgress = isMultiProgress;
    this->weexRuntime = nullptr;
}

void WeexTaskQueue::removeAllTask(String id) {
    threadLocker.lock();
    if (taskQueue_.empty()) {
        threadLocker.unlock();
        return;
    } else {
        for (std::deque<WeexTask *>::iterator it = taskQueue_.begin(); it < taskQueue_.end(); ++it) {
            auto reference = *it;
            if (reference->instanceId == id) {
                taskQueue_.erase(it);
                delete (reference);
                reference = nullptr;
            }
        }
    }
    threadLocker.unlock();
    threadLocker.signal();
}

