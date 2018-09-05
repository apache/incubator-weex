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
#include "core/data_render/class_string.h"
#include "core/data_render/class.h"
#include "core/data_render/class_array.h"
#include "core/data_render/exec_state.h"
#include "core/data_render/common_error.h"
#include "core/data_render/table.h"
#include "core/data_render/string_table.h"
#include <base/LogDefines.h>

namespace weex {
namespace core {
namespace data_render {

static Value split(ExecState *exec_state);
static Value trim(ExecState* exec_state);

ClassDescriptor *NewClassString() {
    ClassDescriptor *array_desc = new ClassDescriptor(nullptr);
    AddClassCFunc(array_desc, "split", split);
    AddClassCFunc(array_desc, "trim", trim);
    return array_desc;
}
    
std::vector<std::string> split_string(std::string srcStr, std::string delimStr, bool repeatedCharIgnored)
{
    std::vector<std::string> resultStringVector;
    std::replace_if(srcStr.begin(), srcStr.end(), [&](const char& c){if(delimStr.find(c)!=std::string::npos){return true;}else{return false;}}/*pred*/, delimStr.at(0));//将出现的所有分隔符都替换成为一个相同的字符（分隔符字符串的第一个）
    size_t pos=srcStr.find(delimStr.at(0));
    std::string addedString="";
    while (pos!=std::string::npos) {
        addedString=srcStr.substr(0,pos);
        if (!addedString.empty()||!repeatedCharIgnored) {
            resultStringVector.push_back(addedString);
        }
        srcStr.erase(srcStr.begin(), srcStr.begin()+pos+1);
        pos=srcStr.find(delimStr.at(0));
    }
    addedString=srcStr;
    if (!addedString.empty()||!repeatedCharIgnored) {
        resultStringVector.push_back(addedString);
    }
    return resultStringVector;
}
 
static Value split(ExecState *exec_state) {
    Value ret = exec_state->class_factory()->CreateArray();
    do {
        size_t length = exec_state->GetArgumentCount();
        if (length < 2) {
            break;
        }
        Value *string = exec_state->GetArgument(0);
        if (!IsString(string)) {
            throw VMExecError("split caller isn't a string");
        }
        Value *split = exec_state->GetArgument(1);
        if (!IsString(split)) {
            throw VMExecError("split caller isn't a string");
        }
        std::string src = CStringValue(string);
        std::string delim = CStringValue(split);
        std::vector<std::string> split_array = split_string(src, delim, true);
        Array *array = ValueTo<Array>(&ret);
        for (int i = 0; i < split_array.size(); i++) {
            Value string_value = exec_state->string_table()->StringFromUTF8(split_array[i]);
            array->items.push_back(string_value);
        }
        
    } while (0);
    
    return ret;
}
    
std::string& trim(std::string &s) {
    if (s.empty()) {
        return s;
    }
    
    s.erase(0, s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(" ") + 1);
    return s;
}

static Value trim(ExecState* exec_state) {
    size_t length = exec_state->GetArgumentCount();
    if (length != 1) {
        throw VMExecError("trim caller args wrong");
    }
    
    Value *string = exec_state->GetArgument(0);
    if (!IsString(string)) {
        throw VMExecError("trim caller isn't a string");
    }
    
    std::string src = CStringValue(string);
    trim(src);
    
    Value string_value = exec_state->string_table()->StringFromUTF8(src);
    return string_value;
}
    
}  // namespace data_render
}  // namespace core
}  // namespace weex
