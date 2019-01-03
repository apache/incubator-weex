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
#ifndef _STRING_UTILS_H_
#define _STRING_UTILS_H_

#include <jni.h>
#include <malloc.h>
#include <iostream>
#include <sstream>
#include <string>

#include "scoped_jstring.h"
#include "scoped_jstring_utf8.h"
#include "third_party/IPC/IPCArguments.h"
#include "third_party/IPC/Serializing/IPCSerializer.h"
#include "third_party/IPC/IPCString.h"
#include "third_party/IPC/IPCByteArray.h"

namespace WeexCore {

static inline std::string jString2Str(JNIEnv *env, const jstring &jstr) {
  char *rtn = NULL;
  jclass clsstring = env->FindClass("java/lang/String");
  jstring strencode = env->NewStringUTF("GB2312");
  jmethodID mid = env->GetMethodID(clsstring, "getBytes", "(Ljava/lang/String;)[B");
  jbyteArray barr = (jbyteArray) env->CallObjectMethod(jstr, mid, strencode);
  jsize alen = env->GetArrayLength(barr);
  jbyte *ba = env->GetByteArrayElements(barr, JNI_FALSE);
  if (alen > 0) {
    rtn = (char *) malloc(alen + 1);
    memcpy(rtn, ba, alen);
    rtn[alen] = 0;
  }
  env->ReleaseByteArrayElements(barr, ba, 0);

  env->DeleteLocalRef(clsstring);
  env->DeleteLocalRef(strencode);
  env->DeleteLocalRef(barr);

  if(rtn != NULL) {
    std::string stemp(rtn);
    free(rtn);
    return stemp;
  } else {
    return "";
  }
}

static inline std::string jString2StrFast(JNIEnv *env, const jstring &jstr){
  if (jstr == nullptr)
    return std::string("");
  auto nativeString = ScopedJStringUTF8(env, jstr);
  return std::string(nativeString.getChars());
}

static std::string jByteArray2Str(JNIEnv *env, jbyteArray barr) {
  if(barr == nullptr) {
    return "";
  }

  char *rtn = NULL;
  jsize alen = env->GetArrayLength(barr);
  jbyte *ba = env->GetByteArrayElements(barr, JNI_FALSE);
  if (alen > 0) {
    rtn = (char *) malloc(alen + 1);
    memcpy(rtn, ba, alen);
    rtn[alen] = 0;
  }
  env->ReleaseByteArrayElements(barr, ba, 0);

  if(rtn != NULL){
    std::string stemp(rtn);
    free(rtn);
    return stemp;
  } else {
    return "";
  }

}

static inline jbyteArray newJByteArray(JNIEnv *env, const char* data, int length) {
  jbyteArray jarray = nullptr;
  if (data == nullptr || length <= 0)
    return jarray;
  int byteSize = length;
  jbyte *jb =  (jbyte*) data;
  jarray = env->NewByteArray(byteSize);
  env->SetByteArrayRegion(jarray, 0, byteSize, jb);
  return jarray;
}

static inline jbyteArray newJByteArray(JNIEnv *env, const char* pat) {
  jbyteArray jarray = nullptr;
  if (pat == nullptr)
    return jarray;
  int byteSize = strlen(pat);
  jbyte *jb =  (jbyte*) pat;
  jarray = env->NewByteArray(byteSize);
  env->SetByteArrayRegion(jarray, 0, byteSize, jb);
  return jarray;
}

static inline jstring newJString(JNIEnv* env, const char* pat) {
  jstring jstr = nullptr;
  if (pat == nullptr)
    return jstr;
  return env->NewStringUTF(pat);
}

static inline char* getArumentAsCStr(IPCArguments *arguments, int argument) {
    char* ret = nullptr;

    if (argument >= arguments->getCount())
      return nullptr;
    if (arguments->getType(argument) == IPCType::BYTEARRAY) {
      const IPCByteArray *ipcBA = arguments->getByteArray(argument);
      int strLen = ipcBA->length;
      ret = new char[strLen+1];
      memcpy(ret, ipcBA->content, strLen);
      ret[strLen] = '\0';
    }

    return ret;
}

static inline int getArumentAsCStrLen(IPCArguments *arguments, int argument) {
  if (argument >= arguments->getCount())
    return 0;
  if (arguments->getType(argument) == IPCType::BYTEARRAY) {
    const IPCByteArray *ipcBA = arguments->getByteArray(argument);
    return  ipcBA->length;
  }
  return 0;
}

static inline jbyteArray getArgumentAsJByteArray(JNIEnv* env, IPCArguments* arguments, size_t argument)
{
  jbyteArray ba = nullptr;
  if (argument >= arguments->getCount())
    return nullptr;
  if (arguments->getType(argument) == IPCType::BYTEARRAY) {
    const IPCByteArray* ipcBA = arguments->getByteArray(argument);
    int strLen = ipcBA->length;
    ba = env->NewByteArray(strLen);
    env->SetByteArrayRegion(ba, 0, strLen,
                            reinterpret_cast<const jbyte*>(ipcBA->content));
  }
  return ba;
}

static inline jstring getArgumentAsJString(JNIEnv *env, IPCArguments *arguments, int argument) {
  jstring ret = nullptr;
  if (arguments->getType(argument) == IPCType::STRING) {
    const IPCString *s = arguments->getString(argument);
    ret = env->NewString(s->content, s->length);
  }
  return ret;
}

static inline int getArgumentAsInt32(JNIEnv* env, IPCArguments* arguments, int argument) {
  int ret = 0;
  if (arguments->getType(argument) == IPCType::INT32) {
    const int32_t type = arguments->get<int32_t>(argument);
    ret = type;
  }
  return ret;
}

static inline int getArgumentAsInt32(IPCArguments* arguments, int argument) {
  int ret = 0;
  if (arguments->getType(argument) == IPCType::INT32) {
    const int32_t type = arguments->get<int32_t>(argument);
    ret = type;
  }
  return ret;
}

static inline int64_t getArgumentAsInt64(IPCArguments* arguments, int argument) {
  int ret = 0;
  if (arguments->getType(argument) == IPCType::INT64) {
    const int64_t type = arguments->get<int64_t>(argument);
    ret = type;
  }
  return ret;
}

static inline float getArgumentAsFloat(IPCArguments* arguments, int argument) {
  float ret = 0;
  if (arguments->getType(argument) == IPCType::FLOAT) {
    const float type = arguments->get<float>(argument);
    ret = type;
  }
  return ret;
}

static inline void addString(JNIEnv *env, IPCSerializer *serializer, jstring str) {
  ScopedJString scopedString(env, str);
  const uint16_t *chars = scopedString.getChars();
  size_t charsLength = scopedString.getCharsLength();
  serializer->add(chars, charsLength);
}
static inline void addByteArrayString(JNIEnv *env, IPCSerializer *serializer, jstring str) {
  ScopedJStringUTF8 scopedJStringUTF8(env,str);
  const char *string = scopedJStringUTF8.getChars();
  serializer->add(string,strlen(string));
}

static inline void addJSONString(JNIEnv *env, IPCSerializer *serializer, jstring str) {
  ScopedJString scopedString(env, str);
  const uint16_t *chars = scopedString.getChars();
  size_t charsLength = scopedString.getCharsLength();
  serializer->addJSON(chars, charsLength);
}

static void addBinaryByteArray(JNIEnv* env, IPCSerializer* serializer, jbyteArray array)
{
  size_t length = env->GetArrayLength(array);
  jbyte* data = env->GetByteArrayElements(array, 0);
  serializer->add((const char*)data, length);
  env->ReleaseByteArrayElements(array, data, 0);
}
}
#endif //_STRING_UTILS_H_
