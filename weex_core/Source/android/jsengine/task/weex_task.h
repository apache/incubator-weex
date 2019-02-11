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
// Created by Darin on 20/07/2018.
//

#ifndef WEEXV8_WEEXTASK_H
#define WEEXV8_WEEXTASK_H

#include "base/time_calculator.h"
#include "android/jsengine/weex_runtime.h"
#include "base/utils/ThreadLocker.h"

class WeexTask {

public:

    class Future {

    public:

        Future() : has_result_(false) {}

        ~Future() {}

        void setResult(std::unique_ptr<WeexJSResult> &result);

        std::unique_ptr<WeexJSResult> waitResult();

    private:
        bool has_result_ = false;
        std::unique_ptr<WeexJSResult> result_;
        ThreadLocker thread_locker_;
    };

    String instanceId;
    int taskId;
    explicit WeexTask(const String &instanceId, int taskId) : future_(nullptr), global_object_(nullptr) {
        this->instanceId = instanceId;
        this->taskId = taskId;
        this->timeCalculator = new weex::base::TimeCalculator(weex::base::TaskPlatform::JSS_ENGINE, "", this->instanceId.utf8().data());
    };

    explicit WeexTask(const String &instanceId) : WeexTask(instanceId, genTaskId()) {};

    virtual ~WeexTask() {if(timeCalculator != nullptr) delete timeCalculator;};

    virtual void run(WeexRuntime *runtime) = 0;
    virtual std::string taskName() = 0;

    inline void set_future(Future* future) {
        future_ = future;
    }

    inline Future* future() {
        return future_;
    }

    inline WeexGlobalObject* global_object() {
        return global_object_;
    }

    inline void set_global_object(WeexGlobalObject* global_object) {
        global_object_ = global_object;
    }

    weex::base::TimeCalculator *timeCalculator;
private:
    Future* future_;
    WeexGlobalObject* global_object_;
};


#endif //WEEXV8_WEEXTASK_H
