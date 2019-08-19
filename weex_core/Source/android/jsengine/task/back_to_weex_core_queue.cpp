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
// Created by Darin on 2019/1/8.
//

#include <base/android/log_utils.h>
#include <object/weex_env.h>
#include "back_to_weex_core_queue.h"


static void *startThread(void *td) {
    auto *self = static_cast<BackToWeexCoreQueue *>(td);
    WeexEnv::getEnv()->initIPC();
    self->isInitOk = true;
    self->start();
    return NULL;
}

int BackToWeexCoreQueue::addTask(BackToWeexCoreQueue::IPCTask *task) {
    threadLocker.lock();
    taskQueue_.push_back(task);
    int size = taskQueue_.size();
    threadLocker.unlock();
    threadLocker.signal();
    return size;
}

void BackToWeexCoreQueue::init() {
    pthread_t thread;
    pthread_create(&thread, nullptr, startThread, this);
    pthread_setname_np(thread, "BackToWeexCoreQueue");
}

void BackToWeexCoreQueue::start() {
    while (!m_stop) {
        BackToWeexCoreQueue::IPCTask *task = getTask();
        if (task == nullptr) {
            continue;
        }
        task->run();
        delete task;
    }

}

BackToWeexCoreQueue::IPCTask *BackToWeexCoreQueue::getTask() {
    BackToWeexCoreQueue::IPCTask *task = nullptr;
    while (task == nullptr) {
        threadLocker.lock();
        while (taskQueue_.empty() || !isInitOk) {
            threadLocker.wait();
        }

        if (taskQueue_.empty()) {
            threadLocker.unlock();
            continue;
        }

        assert(!taskQueue_.empty());
        task = taskQueue_.front();
        taskQueue_.pop_front();
        threadLocker.unlock();
    }

    return task;
}

void BackToWeexCoreQueue::stop() {
    m_stop = true;
}

void BackToWeexCoreQueue::IPCTask::run() {
    if (params.empty() || WeexEnv::getEnv()->is_app_crashed())
        return;

    std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());;
    serializer->setMsg(static_cast<uint32_t>(this->m_type));
    std::vector<BackToWeexCoreQueue::IPCArgs *>::iterator it;
    it = params.begin();
    while (it != params.end()) {
        IPCArgs *&reference = *it;
        serializer->add(reference->m_str, reference->m_length);
        delete reference;
        it++;
    }

    std::unique_ptr<IPCBuffer> buffer = serializer->finish();
    std::unique_ptr<IPCResult> result = WeexEnv::getEnv()->m_ipc_client_->getSender()->send(
            buffer.get());
    if (m_future != nullptr) {
        m_future->setResult(result);
    }
}

void BackToWeexCoreQueue::IPCTask::addParams(const char *str, size_t len) {
    IPCArgs *x = new BackToWeexCoreQueue::IPCArgs(str, len);
    this->params.push_back(x);
}

void BackToWeexCoreQueue::IPCTask::set_future(BackToWeexCoreQueue::Future *m_feature) {
    IPCTask::m_future = m_feature;
}

BackToWeexCoreQueue::IPCTask::~IPCTask() {
}


void BackToWeexCoreQueue::Future::setResult(std::unique_ptr<IPCResult> &result) {
    thread_locker_.lock();
    has_result_ = true;
    result_ = std::move(result);
    thread_locker_.unlock();
    thread_locker_.signal();
}

std::unique_ptr<IPCResult> BackToWeexCoreQueue::Future::waitResult() {
    thread_locker_.lock();
    while (!has_result_) {
        thread_locker_.wait();
    }
    thread_locker_.unlock();
    return std::move(result_);
}

BackToWeexCoreQueue::IPCArgs::IPCArgs(const char *str, size_t len) {
    char *ret = nullptr;
    size_t strLen = 0;
    if (str != nullptr) {
        strLen = len == 0 ? strlen(str) : len;
        ret = new char[strLen + 1];
        memcpy(ret, str, static_cast<size_t>(strLen));
        ret[strLen] = '\0';

    }
    this->m_str = ret;
    this->m_length = strLen;
}
