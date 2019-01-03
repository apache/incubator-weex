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
#ifndef _SCOPED_JSTRING_UTF_8_H_
#define _SCOPED_JSTRING_UTF_8_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <type_traits>
#include <jni.h>

#include "base/base64/base64.h"

namespace WeexCore {

class ScopedJStringUTF8 {
public:
    ScopedJStringUTF8(JNIEnv *env, jstring);

    ~ScopedJStringUTF8();

    const char *getChars();

private:
    JNIEnv *m_env;
    jstring m_jstring;
    const char *m_chars;
};

}
#endif //_SCOPED_JSTRING_UTF_8_H_
