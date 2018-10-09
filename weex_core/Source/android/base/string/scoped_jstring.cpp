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
#include "scoped_jstring.h"

namespace WeexCore {

ScopedJString::ScopedJString(JNIEnv *env, jstring _jstring)
        : m_env(env), m_jstring(_jstring), m_chars(nullptr), m_len(0) {
}

ScopedJString::~ScopedJString() {
  if (m_chars)
    m_env->ReleaseStringChars(m_jstring, m_chars);
}

const jchar *ScopedJString::getChars() {
  if (m_chars)
    return m_chars;
  if (m_jstring == nullptr)
    return nullptr;
  m_chars = m_env->GetStringChars(m_jstring, nullptr);
  m_len = m_env->GetStringLength(m_jstring);
  return m_chars;
}

size_t ScopedJString::getCharsLength() {
  if (m_chars)
    return m_len;
  if (m_jstring == nullptr)
    return 0;
  m_len = m_env->GetStringLength(m_jstring);
  return m_len;
}

} //WeexCore