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
#include <algorithm>
#include <sstream>
#include "core/data_render/class.h"
#include "core/data_render/table.h"
#include "core/data_render/class_console.h"
#include "core/data_render/exec_state.h"
#include "core/data_render/common_error.h"
#include "core/data_render/vnode/vnode_render_manager.h"
#include "third_party/json11/json11.hpp"
#include "base/LogDefines.h"

namespace weex {
namespace core {
namespace data_render {

#define WXLOG_ERROR     "__ERROR"
#define WXLOG_WARN      "__WARN"
#define WXLOG_INFO      "__INFO"
#define WXLOG_DEBUG     "__DEBUG"
#define WXLOG_LOG       "__LOG"
    
static Value log(ExecState *exec_state);
static Value info(ExecState *exec_state);
static Value debug(ExecState *exec_state);
static Value warn(ExecState *exec_state);
static Value error(ExecState *exec_state);

ClassDescriptor *NewClassConsole() {
    ClassDescriptor *desc = new ClassDescriptor(nullptr);
    AddClassStaticCFunc(desc, "log", log);
    AddClassStaticCFunc(desc, "info", info);
    AddClassStaticCFunc(desc, "debug", debug);
    AddClassStaticCFunc(desc, "warn", warn);
    AddClassStaticCFunc(desc, "error", error);
    return desc;
}
    
static std::string loginfo2string(ExecState *exec_state, std::string level) {
    size_t argc = exec_state->GetArgumentCount();
    std::stringstream stream;
    stream << "[";
    for (int i = 0; i < argc; ++i) {
        Value *a = exec_state->GetArgument(i);
        switch (a->type) {
            case Value::Type::NIL:
                stream << "null";
                break;
            case Value::Type::NUMBER:
                stream << a->n;
                break;
            case Value::Type::BOOL:
                stream << (a->b ? "true" : "false");
                break;
            case Value::Type::INT:
                stream << a->i;
                break;
            case Value::Type::STRING:
                stream << "\"" << a->str->c_str() << "\"";
                break;
            case Value::Type::TABLE:
                stream << TableToString(ValueTo<Table>(a));
                break;
            case Value::Type::ARRAY:
                stream << ArrayToString(ValueTo<Array>(a));
                break;
            case Value::Type::CLASS_DESC:
                stream << "class descriptor";
                break;
            case Value::Type::CLASS_INST:
                stream << "class object";
                break;
            case Value::Type::CPTR:
                stream << "cptr";
                break;
            default:
                stream << "unknown type";
                break;
        }
    }
    stream << ",\"" <<  level << "\"" << "]";
    return stream.str();
}

static Value log(ExecState *exec_state) {
    const std::string& stream = loginfo2string(exec_state, WXLOG_LOG);
    weex::core::data_render::VNodeRenderManager::GetInstance()->WXLogNative(exec_state, stream);
    return Value();
}
    
static Value info(ExecState *exec_state) {
    const std::string& stream = loginfo2string(exec_state, WXLOG_INFO);
    weex::core::data_render::VNodeRenderManager::GetInstance()->WXLogNative(exec_state, stream);
    return Value();
}

static Value debug(ExecState *exec_state) {
    const std::string& stream = loginfo2string(exec_state, WXLOG_DEBUG);
    weex::core::data_render::VNodeRenderManager::GetInstance()->WXLogNative(exec_state, stream);
    return Value();
}

static Value warn(ExecState *exec_state) {
    const std::string& stream = loginfo2string(exec_state, WXLOG_WARN);
    weex::core::data_render::VNodeRenderManager::GetInstance()->WXLogNative(exec_state, stream);
    return Value();
}

static Value error(ExecState *exec_state) {
    const std::string& stream = loginfo2string(exec_state, WXLOG_ERROR);
    weex::core::data_render::VNodeRenderManager::GetInstance()->WXLogNative(exec_state, stream);
    return Value();
}

}  // namespace data_render
}  // namespace core
}  // namespace weex
