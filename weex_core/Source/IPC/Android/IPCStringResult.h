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

#ifndef IPCSTRINGRESULT_H
#define IPCSTRINGRESULT_H
#include <jni.h>
#include "IPCResult.h"

class IPCStringResult : public IPCResult {
public:
    IPCStringResult(JNIEnv* env, jstring val);
    virtual ~IPCStringResult();

    const void* getData() override;
    virtual IPCType getType() override;
    const uint16_t* getStringContent() override;
    size_t getStringLength() override;
    const char* getByteArrayContent() override;
    size_t getByteArrayLength() override;

private:
    JNIEnv* m_env;
    jstring m_value;
    const jchar* m_cvalue{ nullptr };
    size_t m_length{ 0U };
};

class IPCJSONStringResult : public IPCStringResult {
public:
    IPCJSONStringResult(JNIEnv* env, jstring val);
    ~IPCJSONStringResult();

    IPCType getType() override;
};

#endif //IPCSTRINGRESULT_H
