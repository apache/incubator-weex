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

#ifndef WEEX_PROJECT_BACK_TO_WEEX_CORE_QUEUE_H
#define WEEX_PROJECT_BACK_TO_WEEX_CORE_QUEUE_H


#include <deque>
#include <map>
#include <third_party/IPC/IPCResult.h>
#include <third_party/IPC/IPCMessageJS.h>
#include <vector>
#include "base/android/ThreadLocker.h"
#include "base/closure.h"

class BackToWeexCoreQueue {

public:

    class Future {

    public:

        Future() : has_result_(false) {}

        ~Future() {}

        void setResult(std::unique_ptr<IPCResult> &result);

        std::unique_ptr<IPCResult> waitResult();

    private:
        bool has_result_ = false;
        std::unique_ptr<IPCResult> result_;
        ThreadLocker thread_locker_;
    };

    class IPCArgs {

    public:
        explicit IPCArgs(const char *str, size_t len = 0);

        ~IPCArgs() {
            if (m_str != nullptr)
                delete (m_str);
            m_length = 0;
        }

    public:
        char *m_str;
        size_t m_length;
    };

    class IPCTask {
    public:
        std::vector<BackToWeexCoreQueue::IPCArgs *> params;

        explicit IPCTask(IPCProxyMsg type) : m_type(type),
                                             m_future(nullptr) {}

        ~IPCTask();

        void run();

        void addParams(const char *str, size_t len = 0);

        void set_future(Future *m_feature);

    private:
        IPCProxyMsg m_type;
        Future *m_future;

    };

    explicit BackToWeexCoreQueue():
            m_stop(false) {};

    BackToWeexCoreQueue::IPCTask *getTask();

    void start();

    void stop();

    void init();

    int addTask(BackToWeexCoreQueue::IPCTask *task);


private:
    std::deque<BackToWeexCoreQueue::IPCTask *> taskQueue_;
    ThreadLocker threadLocker;
    bool m_stop;

public:
    volatile bool isInitOk = false;
};


#endif //WEEX_PROJECT_BACK_TO_WEEX_CORE_QUEUE_H
