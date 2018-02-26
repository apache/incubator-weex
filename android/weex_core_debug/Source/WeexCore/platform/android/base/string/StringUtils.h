#ifndef _STRING_UTILS_H_
#define _STRING_UTILS_H_

#include <jni.h>
#include <IPC/IPCArguments.h>
#include <IPC/Serializing/IPCSerializer.h>
#include <IPC/IPCString.h>
#include <IPC/IPCByteArray.h>
#include <malloc.h>
#include "ScopedJString.h"
#include <iostream>
#include <sstream>
#include <string>

namespace WeexCore {

static float Str2Float(const std::string &str) {
  if (str.empty() || str == "")
    return 0.0f;
  return std::stof(str);
}

static std::string jString2Str(JNIEnv *env, jstring jstr) {
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

  std::string stemp(rtn);
  free(rtn);
  return stemp;
}

static std::string jByteArray2Str(JNIEnv *env, jbyteArray barr) {
  char *rtn = NULL;
  jsize alen = env->GetArrayLength(barr);
  jbyte *ba = env->GetByteArrayElements(barr, JNI_FALSE);
  if (alen > 0) {
    rtn = (char *) malloc(alen + 1);
    memcpy(rtn, ba, alen);
    rtn[alen] = 0;
  }
  env->ReleaseByteArrayElements(barr, ba, 0);

  std::string stemp(rtn);
  free(rtn);
  return stemp;
}

static const char *GetUTFChars(JNIEnv *env, jstring str) {
  const char *c_str = NULL;
  if (env) {
    c_str = env->GetStringUTFChars(str, NULL);
  }
  env->ReleaseStringUTFChars(str, c_str);
  return c_str;
}

static char* getArumentAsCStr(IPCArguments *arguments, int argument) {
    char* ret = nullptr;

    if (argument >= arguments->getCount())
      return nullptr;
    if (arguments->getType(argument) == IPCType::BYTEARRAY) {
      const IPCByteArray *ipcBA = arguments->getByteArray(argument);
      int strLen = ipcBA->length;
      ret = new char[strLen+1];
      for (int i = 0; i < strLen; ++i) {
        ret[i] = ipcBA->content[i];
      }
      ret[strLen] = '\0';
    }

    return ret;
}

static jstring getArgumentAsJString(JNIEnv *env, IPCArguments *arguments, int argument) {
  jstring ret = nullptr;
  if (arguments->getType(argument) == IPCType::STRING) {
    const IPCString *s = arguments->getString(argument);
    ret = env->NewString(s->content, s->length);
  }
  return ret;
}

static char* getArgumentAsCStr(JNIEnv *env, IPCArguments *arguments, int argument) {
  char* ret = nullptr;
  if (arguments->getType(argument) == IPCType::STRING) {
    const IPCString *s = arguments->getString(argument);
    ret = (char *) malloc(s->length);
    char *temp = (char *) &s->content;
    strcpy(ret, temp);
  }
  return ret;
}

static jbyteArray getArgumentAsJByteArray(JNIEnv *env, IPCArguments *arguments, size_t argument) {
  jbyteArray ba = nullptr;
  if (argument >= arguments->getCount())
    return nullptr;
  if (arguments->getType(argument) == IPCType::BYTEARRAY) {
    const IPCByteArray *ipcBA = arguments->getByteArray(argument);
    int strLen = ipcBA->length;
    ba = env->NewByteArray(strLen);
    env->SetByteArrayRegion(ba, 0, strLen,
                            reinterpret_cast<const jbyte *>(ipcBA->content));
  }
  return ba;
}

static int getArgumentAsInt32(JNIEnv* env, IPCArguments* arguments, int argument) {
  int ret = 0;
  if (arguments->getType(argument) == IPCType::INT32) {
    const int32_t type = arguments->get<int32_t>(argument);
    ret = type;
  }
  return ret;
}

static void addString(JNIEnv *env, IPCSerializer *serializer, jstring str) {
  ScopedJString scopedString(env, str);
  const uint16_t *chars = scopedString.getChars();
  size_t charsLength = scopedString.getCharsLength();
  serializer->add(chars, charsLength);
}

static void addJSONString(JNIEnv *env, IPCSerializer *serializer, jstring str) {
  ScopedJString scopedString(env, str);
  const uint16_t *chars = scopedString.getChars();
  size_t charsLength = scopedString.getCharsLength();
  serializer->addJSON(chars, charsLength);
}
}
#endif //_STRING_UTILS_H_
