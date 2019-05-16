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

#include "android/utils/params_utils.h"
#include "android/base/string/string_utils.h"
#include "android/utils/so_utils.h"
#include "base/android/jni_type.h"
#include "base/android/log_utils.h"
#include "core/config/core_environment.h"

namespace WeexCore {

WeexByteArray* genWeexByteArray(const char* str, size_t strLen) {
  auto* ret = (WeexByteArray*)malloc(strLen + sizeof(WeexByteArray));

  if (ret == nullptr) return nullptr;

  memset(ret, 0, strLen + sizeof(WeexByteArray));

  ret->length = strLen;
  memcpy(ret->content, str, strLen);
  ret->content[strLen] = '\0';
  return ret;
}

INIT_FRAMEWORK_PARAMS* genInitFrameworkParams(const char* type,
                                              const char* value) {
  auto* init_framework_params =
      (INIT_FRAMEWORK_PARAMS*)malloc(sizeof(INIT_FRAMEWORK_PARAMS));

  if (init_framework_params == nullptr) return nullptr;

  memset(init_framework_params, 0, sizeof(INIT_FRAMEWORK_PARAMS));

  init_framework_params->type = genWeexByteArray(type, strlen(type));
  init_framework_params->value = genWeexByteArray(value, strlen(value));

  return init_framework_params;
}

WeexString* genWeexString(const uint16_t* str, size_t length) {
  size_t byteSize = length * sizeof(uint16_t);
  auto* string = (WeexString*)malloc(byteSize + sizeof(WeexString));
  if (string == nullptr) return nullptr;

  memset(string, 0, byteSize + sizeof(WeexString));
  string->length = length;
  memcpy(string->content, str, byteSize);
  return string;
}

WeexString* jstring2WeexString(JNIEnv* env, jstring fromJString) {
  if (fromJString != nullptr) {
    ScopedJString scopedString(env, fromJString);
    return genWeexString(scopedString.getChars(),
                         scopedString.getCharsLength());
  } else {
    uint16_t tmp = 0;
    return genWeexString(&tmp, 0);
  }
}

VALUE_WITH_TYPE* getValueWithTypePtr() {
  auto* param = (VALUE_WITH_TYPE*)malloc(sizeof(VALUE_WITH_TYPE));
  if (param == nullptr) return nullptr;

  memset(param, 0, sizeof(VALUE_WITH_TYPE));

  return param;
}

void addParamsToIPCSerializer(IPCSerializer *serializer, VALUE_WITH_TYPE* param) {
  if (param == nullptr) {
    serializer->addJSUndefined();
  } else if (param->type == ParamsType::DOUBLE) {
    serializer->add(param->value.doubleValue);
  } else if (param->type == ParamsType::STRING) {

    if(param->value.string == nullptr) {
      uint16_t tmp = 0;
      serializer->add(&tmp, 0);
    } else {
      serializer->add(param->value.string->content,
                      param->value.string->length);
    }
  } else if(param->type == ParamsType::JSONSTRING) {
    if(param->value.string == nullptr) {
      uint16_t tmp = 0;
      serializer->addJSON(&tmp, 0);
    } else {
      serializer->addJSON(param->value.string->content,
                      param->value.string->length);
    }
  }else if (param->type == ParamsType::BYTEARRAY) {
    if (param->value.byteArray == nullptr) {
      char tmp = '\0';
      serializer->add(&tmp, 0);
    } else {
      serializer->add(param->value.byteArray->content,
                      param->value.byteArray->length);
    }
  } else {
    serializer->addJSUndefined();
  }
};

bool g_is_single_process = false;
bool isSingleProcess() { return g_is_single_process; }

std::vector<INIT_FRAMEWORK_PARAMS*> initFromParam(
    JNIEnv* env, jobject params,
    const std::function<void(const char*, const char*)>&
        ReportNativeInitStatus) {
  LOGE("initFromParam is running ");
  std::vector<INIT_FRAMEWORK_PARAMS*> initFrameworkParams;

#define ADDSTRING(name)                                                     \
  {                                                                         \
    if (name == nullptr) {                                                  \
      initFrameworkParams.clear();                                          \
    } else {                                                                \
      const char* myname = #name;                                           \
      ScopedJStringUTF8 scopedString(env, (jstring)name);                   \
      const char* chars = scopedString.getChars();                          \
      initFrameworkParams.push_back(genInitFrameworkParams(myname, chars)); \
    }                                                                       \
  }

  jclass c_params = env->GetObjectClass(params);
  if (c_params == nullptr) {
    ADDSTRING(nullptr);
    ReportNativeInitStatus("-1012", "get c_params failed");
    return initFrameworkParams;
  }

  jmethodID m_platform =
      env->GetMethodID(c_params, "getPlatform", "()Ljava/lang/String;");
  if (m_platform == nullptr) {
    ADDSTRING(nullptr);
    ReportNativeInitStatus("-1012", "get m_platform failed");
    return initFrameworkParams;
  }
  jobject platform = env->CallObjectMethod(params, m_platform);
  if (platform == nullptr) {
    ADDSTRING(nullptr);
    ReportNativeInitStatus("-1012", "get platform failed");
    return initFrameworkParams;
  }
  if (!WXCoreEnvironment::getInstance()->SetPlatform(
          jString2StrFast(env, reinterpret_cast<jstring&>(platform)))) {
    LOGD("setPlatform");
  }
  ADDSTRING(platform);
  env->DeleteLocalRef(platform);

  jmethodID m_layoutDirection =
          env->GetMethodID(c_params, "getLayoutDirection", "()Ljava/lang/String;");
  if (m_layoutDirection == nullptr) {
    ADDSTRING(nullptr);
    ReportNativeInitStatus("-1012", "get m_layoutDirection failed");
    return initFrameworkParams;
  }
  jobject layoutDirection = env->CallObjectMethod(params, m_layoutDirection);
  if (layoutDirection == nullptr) {
    ADDSTRING(nullptr);
    ReportNativeInitStatus("-1012", "get layoutDirection failed");
    return initFrameworkParams;
  }
  ADDSTRING(layoutDirection);
  env->DeleteLocalRef(layoutDirection);

  jmethodID m_use_single_process =
      env->GetMethodID(c_params, "getUseSingleProcess", "()Ljava/lang/String;");
  if (m_use_single_process == nullptr) {
    LOGE("getUseSingleProcess method is missing");
  } else {
    jobject j_use_single_process =
        env->CallObjectMethod(params, m_use_single_process);
    const char* use_single_process =
        env->GetStringUTFChars((jstring)(j_use_single_process), nullptr);
    LOGE("g_use_single_process is %s ", use_single_process);
    if (use_single_process == nullptr) {
      g_is_single_process = false;
    } else {
      g_is_single_process = strstr(use_single_process, "true") != nullptr;
      env->DeleteLocalRef(j_use_single_process);
    }
  }

  jmethodID m_get_jsc_so_path =
          env->GetMethodID(c_params, "getLibJscPath", "()Ljava/lang/String;");
  if (m_get_jsc_so_path != nullptr) {
    jobject j_get_jsc_so_path =
            env->CallObjectMethod(params, m_get_jsc_so_path);
    if (j_get_jsc_so_path != nullptr) {
      SoUtils::set_jsc_so_path(const_cast<char*>(
                                       env->GetStringUTFChars((jstring)(j_get_jsc_so_path), nullptr)));
      LOGE("g_jscSoPath is %s ", SoUtils::jsc_so_path());
      env->DeleteLocalRef(j_get_jsc_so_path);
    }
  }

  jmethodID m_get_jss_so_path =
      env->GetMethodID(c_params, "getLibJssPath", "()Ljava/lang/String;");
  if (m_get_jss_so_path != nullptr) {
    jobject j_get_jss_so_path =
        env->CallObjectMethod(params, m_get_jss_so_path);
    if (j_get_jss_so_path != nullptr) {
      SoUtils::set_jss_so_path(const_cast<char*>(
          env->GetStringUTFChars((jstring)(j_get_jss_so_path), nullptr)));
      LOGE("g_jssSoPath is %s ", SoUtils::jss_so_path());
      env->DeleteLocalRef(j_get_jss_so_path);
    }
  }

    jmethodID m_get_crash_file_path = env->GetMethodID(c_params, "getCrashFilePath", "()Ljava/lang/String;");
    if (m_get_jss_so_path != nullptr) {
        jobject j_get_crash_file_path = env->CallObjectMethod(params, m_get_crash_file_path);
        if (j_get_crash_file_path != nullptr) {
            SoUtils::set_crash_file_path(const_cast<char *>(env->GetStringUTFChars(
                                (jstring) (j_get_crash_file_path),
                                nullptr)));
            LOGE("g_crashFilePath is %s ", SoUtils::crash_file_path());
            env->DeleteLocalRef(j_get_crash_file_path);
        }
    }

  jmethodID m_get_jss_icu_path =
          env->GetMethodID(c_params, "getLibIcuPath", "()Ljava/lang/String;");
  if (m_get_jss_icu_path != nullptr) {
    jobject j_get_jss_icu_path =
            env->CallObjectMethod(params, m_get_jss_icu_path);
    if (j_get_jss_icu_path != nullptr) {
      SoUtils::set_jss_icu_path(const_cast<char*>(
                                       env->GetStringUTFChars((jstring)(j_get_jss_icu_path), nullptr)));
      LOGE("g_jssIcuPath is %s ", SoUtils::jss_icu_path());
      env->DeleteLocalRef(j_get_jss_icu_path);
    }
  }

  jmethodID m_get_lib_ld_path =
          env->GetMethodID(c_params, "getLibLdPath", "()Ljava/lang/String;");
  if (m_get_lib_ld_path != nullptr) {
    jobject j_get_lib_ld_path =
            env->CallObjectMethod(params, m_get_lib_ld_path);
    if (j_get_lib_ld_path != nullptr) {
      SoUtils::set_lib_ld_path(const_cast<char*>(
                                        env->GetStringUTFChars((jstring)(j_get_lib_ld_path), nullptr)));
      LOGE("lib_ld_path is %s ", SoUtils::lib_ld_path());
      env->DeleteLocalRef(j_get_lib_ld_path);
    }
  }

  jmethodID m_osVersion =
      env->GetMethodID(c_params, "getOsVersion", "()Ljava/lang/String;");
  if (m_osVersion == nullptr) {
    ADDSTRING(nullptr);
    ReportNativeInitStatus("-1012", "get m_osVersion failed");
    return initFrameworkParams;
  }
  jobject osVersion = env->CallObjectMethod(params, m_osVersion);
  if (osVersion == nullptr) {
    ADDSTRING(nullptr);
    ReportNativeInitStatus("-1012", "get osVersion failed");
    return initFrameworkParams;
  }
  ADDSTRING(osVersion);
  env->DeleteLocalRef(osVersion);

  jmethodID m_cacheMethod =
      env->GetMethodID(c_params, "getCacheDir", "()Ljava/lang/String;");
  if (m_cacheMethod != nullptr) {
    jobject cacheDir = env->CallObjectMethod(params, m_cacheMethod);
    if (cacheDir != nullptr) {
      ADDSTRING(cacheDir);
      env->DeleteLocalRef(cacheDir);
    }
  }

  jmethodID m_appVersion =
      env->GetMethodID(c_params, "getAppVersion", "()Ljava/lang/String;");
  if (m_appVersion == nullptr) {
    ADDSTRING(nullptr);
    ReportNativeInitStatus("-1012", "get m_appVersion failed");
    return initFrameworkParams;
  }
  jobject appVersion = env->CallObjectMethod(params, m_appVersion);
  if (appVersion == nullptr) {
    ADDSTRING(nullptr);
    ReportNativeInitStatus("-1012", "get appVersion failed");
    return initFrameworkParams;
  }
  ADDSTRING(appVersion);
  env->DeleteLocalRef(appVersion);

  jmethodID m_weexVersion =
      env->GetMethodID(c_params, "getWeexVersion", "()Ljava/lang/String;");
  if (m_weexVersion == nullptr) {
    ADDSTRING(nullptr);
    ReportNativeInitStatus("-1012", "get m_weexVersion failed");
    return initFrameworkParams;
  }
  jobject weexVersion = env->CallObjectMethod(params, m_weexVersion);
  if (weexVersion == nullptr) {
    ADDSTRING(nullptr);
    ReportNativeInitStatus("-1012", "get weexVersion failed");
    return initFrameworkParams;
  }
  ADDSTRING(weexVersion);
  env->DeleteLocalRef(weexVersion);

  jmethodID m_deviceModel =
      env->GetMethodID(c_params, "getDeviceModel", "()Ljava/lang/String;");
  if (m_deviceModel == nullptr) {
    ADDSTRING(nullptr);
    ReportNativeInitStatus("-1012", "get m_deviceModel failed");
    return initFrameworkParams;
  }
  jobject deviceModel = env->CallObjectMethod(params, m_deviceModel);
  if (deviceModel == nullptr) {
    ADDSTRING(nullptr);
    ReportNativeInitStatus("-1012", "get deviceModel failed");
    return initFrameworkParams;
  }
  ADDSTRING(deviceModel);
  env->DeleteLocalRef(deviceModel);

  jmethodID m_appName =
      env->GetMethodID(c_params, "getAppName", "()Ljava/lang/String;");
  if (m_appName == nullptr) {
    ADDSTRING(nullptr);
    ReportNativeInitStatus("-1012", "get m_appName failed");
    return initFrameworkParams;
  }
  jobject appName = env->CallObjectMethod(params, m_appName);
  if (appName == nullptr) {
    ADDSTRING(nullptr);
    ReportNativeInitStatus("-1012", "get appName failed");
    return initFrameworkParams;
  }
  ADDSTRING(appName);
  env->DeleteLocalRef(appName);

  jmethodID m_deviceWidth =
      env->GetMethodID(c_params, "getDeviceWidth", "()Ljava/lang/String;");
  if (m_deviceWidth == nullptr) {
    ADDSTRING(nullptr);
    ReportNativeInitStatus("-1012", "get m_deviceWidth failed");
    return initFrameworkParams;
  }
  jobject deviceWidth = env->CallObjectMethod(params, m_deviceWidth);
  if (deviceWidth == nullptr) {
    ADDSTRING(nullptr);
    ReportNativeInitStatus("-1012", "get deviceWidth failed");
    return initFrameworkParams;
  }
  if (!WXCoreEnvironment::getInstance()->SetDeviceWidth(
          jString2StrFast(env, reinterpret_cast<jstring&>(deviceWidth)))) {
    LOGD("setDeviceDisplay");
  }
  ADDSTRING(deviceWidth);
  env->DeleteLocalRef(deviceWidth);

  jmethodID m_deviceHeight =
      env->GetMethodID(c_params, "getDeviceHeight", "()Ljava/lang/String;");
  if (m_deviceHeight == nullptr) {
    ADDSTRING(nullptr);
    ReportNativeInitStatus("-1012", "get m_deviceHeight failed");
    return initFrameworkParams;
  }
  jobject deviceHeight = env->CallObjectMethod(params, m_deviceHeight);
  if (deviceHeight == nullptr) {
    ADDSTRING(nullptr);
    ReportNativeInitStatus("-1012", "get deviceHeight failed");
    return initFrameworkParams;
  }
  if (!WXCoreEnvironment::getInstance()->SetDeviceHeight(
          jString2StrFast(env, reinterpret_cast<jstring&>(deviceHeight)))) {
    LOGD("setDeviceHeight");
  }
  ADDSTRING(deviceHeight);
  env->DeleteLocalRef(deviceHeight);

  jmethodID m_options =
      env->GetMethodID(c_params, "getOptions", "()Ljava/lang/Object;");
  if (m_options == nullptr) {
    ADDSTRING(nullptr);
    ReportNativeInitStatus("-1012", "get m_options failed");
    return initFrameworkParams;
  }
  jobject options = env->CallObjectMethod(params, m_options);
  if (options == nullptr) {
    ADDSTRING(nullptr);
    ReportNativeInitStatus("-1012", "get options failed");
    return initFrameworkParams;
  }

  jclass jmapclass = env->FindClass("java/util/Map");
  jmethodID jkeysetmid =
      env->GetMethodID(jmapclass, "keySet", "()Ljava/util/Set;");
  jmethodID jgetmid = env->GetMethodID(
      jmapclass, "get", "(Ljava/lang/Object;)Ljava/lang/Object;");
  jobject jsetkey = env->CallObjectMethod(options, jkeysetmid);
  jclass jsetclass = env->FindClass("java/util/Set");
  jmethodID jtoArraymid =
      env->GetMethodID(jsetclass, "toArray", "()[Ljava/lang/Object;");
  jobjectArray jobjArray =
      (jobjectArray)env->CallObjectMethod(jsetkey, jtoArraymid);
  env->DeleteLocalRef(jsetkey);
  if (jobjArray != NULL) {
    jsize arraysize = env->GetArrayLength(jobjArray);
    for (int i = 0; i < arraysize; i++) {
      jstring jkey = (jstring)env->GetObjectArrayElement(jobjArray, i);
      jstring jvalue = (jstring)env->CallObjectMethod(options, jgetmid, jkey);

      if (jkey != NULL) {
        ScopedJStringUTF8 c_key(env, jkey);
        ScopedJStringUTF8 c_value(env, jvalue);
        const char* c_key_chars = c_key.getChars();
        int c_key_len = strlen(c_key_chars);
        const char* c_value_chars = c_value.getChars();
        int c_value_len = strlen(c_value_chars);
        initFrameworkParams.push_back(
            genInitFrameworkParams(c_key_chars, c_value_chars));
        const std::string& key = jString2Str(env, jkey);
        if (key != "") {
          WXCoreEnvironment::getInstance()->AddOption(key,
                                                      jString2Str(env, jvalue));
        }
      }
    }
    env->DeleteLocalRef(jobjArray);
  }
  env->DeleteLocalRef(options);
  return initFrameworkParams;
}

jstring getJsonData(JNIEnv* env, jobjectArray jargs, int index) {
  int length = 0;
  if (jargs != NULL) {
    length = env->GetArrayLength(jargs);
  }
  jstring ret = NULL;
  if (length < (index + 1)) {
    return ret;
  }

  auto jArg = std::unique_ptr<WXJSObject>(
      new WXJSObject(env, base::android::ScopedLocalJavaRef<jobject>(
                              env, env->GetObjectArrayElement(jargs, index))
                              .Get()));
  jint jTypeInt = jArg->GetType(env);
  auto jDataObj = jArg->GetData(env);

  if (jTypeInt == 3) {
    ret = (jstring)jDataObj.Release();
  }
  return ret;
}

void addParamsFromJArgs(std::vector<VALUE_WITH_TYPE*>& params,
                        VALUE_WITH_TYPE* param, JNIEnv* env,
                        std::unique_ptr<WXJSObject>& wx_js_object) {
  jint jTypeInt = wx_js_object->GetType(env);
  auto jDataObj = wx_js_object->GetData(env);

  if (jTypeInt == 1) {
    jdouble jDoubleObj =
        base::android::JNIType::DoubleValue(env, jDataObj.Get());
    param->type = ParamsType::DOUBLE;
    param->value.doubleValue = jDoubleObj;
  } else if (jTypeInt == 2) {
    jstring jDataStr = (jstring)jDataObj.Get();
    param->type = ParamsType::STRING;
    param->value.string = jstring2WeexString(env, jDataStr);
  } else if (jTypeInt == 3) {
    jstring jDataStr = (jstring)jDataObj.Get();
    param->type = ParamsType::JSONSTRING;
    param->value.string = jstring2WeexString(env, jDataStr);
  } else if (jTypeInt == 4) {
    jbyteArray dataArray = (jbyteArray)jDataObj.Get();
    param->type = ParamsType::BYTEARRAY;
    jbyte* data = env->GetByteArrayElements(dataArray, 0);
    size_t length = env->GetArrayLength(dataArray);
    param->value.byteArray = genWeexByteArray((const char*)data, length);
    env->ReleaseByteArrayElements(dataArray, data, 0);
  } else {
    param->type = ParamsType::JSUNDEFINED;
  }

  if (param != nullptr) {
    params.push_back(param);
  }
}



void freeValueWithType(VALUE_WITH_TYPE* param) {
  if (param->type == ParamsType::STRING ||
      param->type == ParamsType::JSONSTRING) {
    free(param->value.string);
  }
  if (param->type == ParamsType::BYTEARRAY) {
    free(param->value.byteArray);
  }
}

void freeParams(std::vector<VALUE_WITH_TYPE*>& params) {
  for (auto& param : params) {
    freeValueWithType(param);
    free(param);
  }
}

void freeParams(std::vector<InitFrameworkParams*>& params) {
  for (auto& param : params) {
    if (param->type != nullptr) {
      free(param->type);
    }
    if (param->value != nullptr) {
      free(param->value);
    }
    free(param);
  }
}
}  // namespace WeexCore
