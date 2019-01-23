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
// Created by Darin on 29/05/2018.
//

#include "exe_js_args.h"

#include "android/jsengine/weex_jsc_utils.h"

ExeJsArgs::ExeJsArgs(std::vector<VALUE_WITH_TYPE *> &params) {
    for (auto paramsObject : params) {
        auto pType = copyValueToSelf(paramsObject);
        if (pType != nullptr) {
            this->params.push_back(pType);
        }

    }
}

ExeJsArgs::ExeJsArgs(IPCArguments *arguments, size_t startCount) {
    size_t count = arguments->getCount();
    for (size_t i = startCount; i < count; ++i) {
        auto pType = getValueWithTypePtr();
        switch (arguments->getType(i)) {
            case IPCType::DOUBLE:
                pType->type = ParamsType::DOUBLE;
                pType->value.doubleValue = i;
                break;
            case IPCType::STRING: {
                pType->type = ParamsType::STRING;
                const IPCString *ipcstr = arguments->getString(i);
                pType->value.string = genWeexStringSS(ipcstr->content,ipcstr->length);

            }
                break;
            case IPCType::JSONSTRING: {

                pType->type = ParamsType::JSONSTRING;

                const IPCString *ipcstr = arguments->getString(i);

                pType->value.string = genWeexStringSS(ipcstr->content,ipcstr->length);
            }
                break;
            case IPCType::BYTEARRAY: {
                pType->type = ParamsType::BYTEARRAY;
                const IPCByteArray *array = arguments->getByteArray(i);
                pType->value.byteArray = genWeexByteArraySS(array->content,array->length);

            }
                break;
            default:
                pType->type = ParamsType::JSUNDEFINED;
                break;
        }

        this->params.push_back(pType);
    }

}

VALUE_WITH_TYPE *ExeJsArgs::getValueWithTypePtr() {
    auto *param = (VALUE_WITH_TYPE *) malloc(sizeof(VALUE_WITH_TYPE));
    if (param == nullptr)
        return nullptr;

    memset(param, 0, sizeof(VALUE_WITH_TYPE));

    return param;
}

VALUE_WITH_TYPE *ExeJsArgs::copyValueToSelf(VALUE_WITH_TYPE *value_with_type) {
    if (value_with_type == nullptr)
        return nullptr;

    auto pType = getValueWithTypePtr();
    pType->type = value_with_type->type;
    switch (value_with_type->type) {
        case ParamsType::DOUBLE: {
            pType->value.doubleValue = value_with_type->value.doubleValue;
        }
            break;
        case ParamsType::STRING:
        case ParamsType::JSONSTRING: {
            pType->value.string = genWeexStringSS(value_with_type->value.string->content,
                                                  value_with_type->value.string->length);

            const String &string = weexString2String(pType->value.string);

        }
            break;
        case ParamsType::BYTEARRAY: {
            pType->value.byteArray = genWeexByteArraySS(value_with_type->value.byteArray->content,
                                                        value_with_type->value.byteArray->length);
        }
            break;
        default:
            break;
    }
    return pType;
}
