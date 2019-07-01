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
// Created by 陈佩翰 on 2019/3/8.
//

#include "action_args_check.h"
#include "third_party/json11/json11.hpp"
#include "base/utils/log_utils.h"

namespace WeexCore {
    bool isCallNativeToFinish(const char *task) {
        if (nullptr == task) {
            return false;
        }
        //try match json str:[{"args": [], "method": "createFinish", "module": "dom"}]
        std::string task_str(task);
        if (task_str.length() != 57) {
            return false;
        }

        std::string errr;
        json11::Json task_json = json11::Json::parse(task_str, errr);
        if (!task_json.is_array() || task_json.array_items().size() <= 0) {
            return false;
        }
        auto array = task_json.array_items();
        auto item = array[0];
        if (!array[0].is_object()) {
            return false;
        }
        auto module_value = item["module"];
        auto method_value = item["method"];
        auto args_value = item["args"];

        bool isCreateFinishAction = false;
        if (module_value.is_string() && method_value.is_string() && args_value.is_array()) {
            isCreateFinishAction = module_value.dump() == "\"dom\"" && method_value.dump() == "\"createFinish\"" &&
                                   args_value.array_items().size() <= 0;
        }
        return isCreateFinishAction;
    }
}
