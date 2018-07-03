/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "IPCStringResult.h"

IPCStringResult::IPCStringResult(JNIEnv* env, jstring val)
        : m_env(env)
        , m_value(static_cast<jstring>(env->NewLocalRef(val)))
{
    m_cvalue = env->GetStringChars(m_value, nullptr);
    m_length = env->GetStringLength(m_value);
}

IPCStringResult::~IPCStringResult()
{
    if (m_cvalue)
        m_env->ReleaseStringChars(m_value, m_cvalue);
    m_env->DeleteLocalRef(m_value);
}

const void* IPCStringResult::getData()
{
    return nullptr;
}

IPCType IPCStringResult::getType()
{
    return IPCType::STRING;
}

const uint16_t* IPCStringResult::getStringContent()
{
    return m_cvalue;
}

size_t IPCStringResult::getStringLength()
{
    return m_length;
}

const char* IPCStringResult::getByteArrayContent()
{
    return nullptr;
}

size_t IPCStringResult::getByteArrayLength()
{
    return 0U;
}

IPCJSONStringResult::IPCJSONStringResult(JNIEnv* env, jstring val)
        : IPCStringResult(env, val) {}

IPCJSONStringResult::~IPCJSONStringResult() {}

IPCType IPCJSONStringResult::getType()
{
    return IPCType::JSONSTRING;
}
