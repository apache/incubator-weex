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
#include <android/base/log_utils.h>
#include <core/config/core_environment.h>
#include <android/base/string/string_utils.h>
#include <android/base/jni_type.h>
#include "android/utils/so_utils.h"

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

    bool g_is_single_process = false;
    bool isSingleProcess() {
        return true;
    }

    std::vector<INIT_FRAMEWORK_PARAMS*> initFromParam(JNIEnv* env,
                                                             jobject params) {
        LOGE("initFromParam is running ");
        std::vector<INIT_FRAMEWORK_PARAMS*> initFrameworkParams;

#define ADDSTRING(name)                                                   \
  {                                                                       \
    const char* myname = #name;                                           \
    ScopedJStringUTF8 scopedString(env, (jstring)name);                   \
    const char* chars = scopedString.getChars();                          \
    initFrameworkParams.push_back(genInitFrameworkParams(myname, chars)); \
  }
        jclass c_params = env->GetObjectClass(params);

        jmethodID m_platform =
                env->GetMethodID(c_params, "getPlatform", "()Ljava/lang/String;");
        jobject platform = env->CallObjectMethod(params, m_platform);
        if (!WXCoreEnvironment::getInstance()->SetPlatform(
                jString2StrFast(env, reinterpret_cast<jstring&>(platform)))) {
            LOGD("setPlatform");
        }
        ADDSTRING(platform);

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
            g_is_single_process = strstr(use_single_process, "true") != nullptr;
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
            }
        }

        jmethodID m_osVersion =
                env->GetMethodID(c_params, "getOsVersion", "()Ljava/lang/String;");
        jobject osVersion = env->CallObjectMethod(params, m_osVersion);
        ADDSTRING(osVersion);

        // use param ti get cacheDir
        jmethodID m_cacheMethod =
                env->GetMethodID(c_params, "getCacheDir", "()Ljava/lang/String;");
        if (m_cacheMethod != NULL) {
            jobject cacheDir = env->CallObjectMethod(params, m_cacheMethod);
            if (cacheDir != NULL) {
                ADDSTRING(cacheDir);
            }
        }

        jmethodID m_appVersion =
                env->GetMethodID(c_params, "getAppVersion", "()Ljava/lang/String;");
        jobject appVersion = env->CallObjectMethod(params, m_appVersion);
        ADDSTRING(appVersion);

        jmethodID m_weexVersion =
                env->GetMethodID(c_params, "getWeexVersion", "()Ljava/lang/String;");
        jobject weexVersion = env->CallObjectMethod(params, m_weexVersion);
        ADDSTRING(weexVersion);

        jmethodID m_deviceModel =
                env->GetMethodID(c_params, "getDeviceModel", "()Ljava/lang/String;");
        jobject deviceModel = env->CallObjectMethod(params, m_deviceModel);
        ADDSTRING(deviceModel);

        jmethodID m_appName =
                env->GetMethodID(c_params, "getAppName", "()Ljava/lang/String;");
        jobject appName = env->CallObjectMethod(params, m_appName);
        ADDSTRING(appName);

        jmethodID m_deviceWidth =
                env->GetMethodID(c_params, "getDeviceWidth", "()Ljava/lang/String;");
        jobject deviceWidth = env->CallObjectMethod(params, m_deviceWidth);
        if (!WXCoreEnvironment::getInstance()->SetDeviceWidth(
                jString2StrFast(env, reinterpret_cast<jstring&>(deviceWidth)))) {
            LOGD("setDeviceWidth");
        }
        ADDSTRING(deviceWidth);

        jmethodID m_deviceHeight =
                env->GetMethodID(c_params, "getDeviceHeight", "()Ljava/lang/String;");
        jobject deviceHeight = env->CallObjectMethod(params, m_deviceHeight);
        if (!WXCoreEnvironment::getInstance()->SetDeviceHeight(
                jString2StrFast(env, reinterpret_cast<jstring&>(deviceHeight)))) {
            LOGD("setDeviceHeight");
        }
        ADDSTRING(deviceHeight);

        jmethodID m_options =
                env->GetMethodID(c_params, "getOptions", "()Ljava/lang/Object;");
        jobject options = env->CallObjectMethod(params, m_options);
        jclass jmapclass = env->FindClass("java/util/HashMap");
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
                    // const char* c_key = env->GetStringUTFChars(jkey, NULL);
                    // addString(vm, WXEnvironment, c_key, jString2String(env, jvalue));
                    // serializer->add(c_key, strlen(c_key));
                    // env->DeleteLocalRef(jkey);
                    // if (jvalue != NULL) {
                    //     env->DeleteLocalRef(jvalue);
                    // }
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
                new WXJSObject(env, base::android::ScopedLocalJavaRef<jobject>(env,
                                                                               env->GetObjectArrayElement(
                                                                                       jargs,
                                                                                       index)).Get()));
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
            jdouble jDoubleObj = base::android::JNIType::DoubleValue(env, jDataObj.Get());
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
        } else {
            param->type = ParamsType::JSUNDEFINED;
        }

        if (param != nullptr) {
            params.push_back(param);
        }

    }


    void freeInitFrameworkParams(std::vector<INIT_FRAMEWORK_PARAMS *> &params) {
        for (auto &param : params) {
            free(param->type);
            free(param->value);
            free(param);
        }
    }

    void freeParams(std::vector<VALUE_WITH_TYPE *> &params) {
        for (auto &param : params) {
            if (param->type == ParamsType::STRING ||
                param->type == ParamsType::JSONSTRING) {
                free(param->value.string);
            }
            if (param->type == ParamsType::BYTEARRAY) {
                free(param->value.byteArray);
            }
            free(param);
        }
    }
}