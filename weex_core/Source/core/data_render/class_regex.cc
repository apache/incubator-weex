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
#include <regex>
#include <vector>
#include "core/data_render/class.h"
#include "core/data_render/class_regex.h"
#include "core/data_render/exec_state.h"
#include "core/data_render/common_error.h"
#include "core/data_render/table.h"
#include "base/LogDefines.h"

namespace weex {
namespace core {
namespace data_render {

static Value test(ExecState *exec_state);
static Value exec(ExecState *exec_state);

ClassDescriptor *NewClassRegex() {
    ClassDescriptor *desc = new ClassDescriptor(nullptr);
    AddClassCFunc(desc, "test", test);
    AddClassCFunc(desc, "exec", exec);
    return desc;
}

static Value test(ExecState *exec_state) {
    Value ret;
    do {
        size_t length = exec_state->GetArgumentCount();
        if (length != 2) {
            throw VMExecError("Regex.test arg count != 2");
        }
        Value *caller = exec_state->GetArgument(0);
        Value *arg = exec_state->GetArgument(1);
        if (!IsClassInstance(caller)) {
            throw VMExecError("Type Error For Class Instance with OP_CODE [OP_GETSUPER]");
        }
        ClassInstance *inst = ValueTo<ClassInstance>(caller);

        Value* reg = GetClassMember(inst, "_reg");
        Value* flag = GetClassMember(inst, "_flag");
        if (!IsString(reg) || !IsString(flag) || !IsString(arg)){
            throw VMExecError("Regex._reg || _flag is not string");
        }
        std::string reg_str = ValueTo<String>(reg)->c_str();
        std::string flag_str = ValueTo<String>(flag)->c_str();
        std::string test_str = ValueTo<String>(arg)->c_str();

        std::regex_constants::syntax_option_type type = std::regex_constants::ECMAScript;
        if (flag_str.find('i') != std::string::npos){
            type |= std::regex_constants::icase;
        }
        std::regex express(reg_str, type);
        std::smatch match;
        ret  = std::regex_search(test_str,match,express);
    } while (0);

    return ret;
}

static Value exec(ExecState *exec_state) {
  Value ret;
  do {
    size_t length = exec_state->GetArgumentCount();
    if (length != 2) {
      throw VMExecError("Regex.exec arg count != 2");
    }
    Value *caller = exec_state->GetArgument(0);
    Value *arg = exec_state->GetArgument(1);
    if (!IsClassInstance(caller)) {
      throw VMExecError("Type Error For Class Instance with OP_CODE [OP_GETSUPER]");
    }
    ClassInstance *inst = ValueTo<ClassInstance>(caller);

    Value* reg = GetClassMember(inst, "_reg");
    Value* flag = GetClassMember(inst, "_flag");
    if (!IsString(reg) || !IsString(flag) || !IsString(arg)){
      throw VMExecError("Regex._reg || _flag is not string");
    }
    std::string reg_str = ValueTo<String>(reg)->c_str();
    std::string flag_str = ValueTo<String>(flag)->c_str();
    std::string test_str = ValueTo<String>(arg)->c_str();

    std::regex_constants::syntax_option_type type = std::regex_constants::ECMAScript;
    if (flag_str.find('i') != std::string::npos){
      type |= std::regex_constants::icase;
    }

    bool g_mode =flag_str.find('g') != std::string::npos;

    ret = exec_state->class_factory()->CreateArray();
    Array* arr = ValueTo<Array>(&ret);

    if (g_mode){
      std::regex express(reg_str, type);
      std::smatch match;
      std::string::const_iterator iterStart = test_str.begin();
      std::string::const_iterator iterEnd = test_str.end();
      bool succ = false;
      while (std::regex_search(iterStart,iterEnd,match,express)){
        arr->items.push_back(exec_state->string_table()->StringFromUTF8(match[0]));
        iterStart = match[0].second;
        succ = true;
      }
      if (!succ){
        ret = Value();
      }
    } else{
      std::regex express(reg_str, type);
      std::smatch match;
      bool succ  = std::regex_search(test_str,match,express);

      if (succ){
        for (size_t i = 0; i < match.size(); ++i){
          arr->items.push_back(exec_state->string_table()->StringFromUTF8(match[i]));
        }
      } else {
        ret = Value();
      }
      // else return []
    }
  } while (0);

  return ret;
}

}  // namespace data_render
}  // namespace core
}  // namespace weex
