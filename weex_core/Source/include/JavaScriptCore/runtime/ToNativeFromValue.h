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
#pragma once

#include "JSCJSValue.h"

namespace JSC {

template<typename Adaptor>
typename Adaptor::Type toNativeFromValue(JSValue value)
{
    if (value.isInt32())
        return Adaptor::toNativeFromInt32(value.asInt32());
    return Adaptor::toNativeFromDouble(value.asDouble());
}

template<typename Adaptor>
typename Adaptor::Type toNativeFromValue(ExecState* exec, JSValue value)
{
    if (value.isInt32())
        return Adaptor::toNativeFromInt32(value.asInt32());
    if (value.isNumber())
        return Adaptor::toNativeFromDouble(value.asDouble());
    return Adaptor::toNativeFromDouble(value.toNumber(exec));
}

template<typename Adaptor>
std::optional<typename Adaptor::Type> toNativeFromValueWithoutCoercion(JSValue value)
{
    if (!value.isNumber())
        return std::nullopt;
    if (value.isInt32())
        return Adaptor::toNativeFromInt32WithoutCoercion(value.asInt32());
    return Adaptor::toNativeFromDoubleWithoutCoercion(value.asDouble());
}

} // namespace JSC
