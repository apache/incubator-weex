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
// Copyright [2017-12-07] <WeexCore>
#include "WeexProxy.h"

#include <android/base/log_utils.h>
#include <android/base/string/string_utils.h>
#include <core/config/core_environment.h>
#include <android/jsengine/multiprocess/ExtendJSApi.h>
#include <android/jsengine/api/WeexJSCoreApi.h>

#include <dlfcn.h>
#include <errno.h>
#include <core/layout/measure_func_adapter_impl_android.h>
#include <core/manager/weex_core_manager.h>

const char *s_cacheDir;
const char *g_jssSoPath = nullptr;
const char *g_jssSoName = "libweexjss.so";
bool s_start_pie = true;

static IPCSender *sSender;
static std::unique_ptr<IPCHandler> sHandler;
static std::unique_ptr<WeexJSConnection> sConnection;
static WEEX_CORE_JS_SERVER_API_FUNCTIONS *js_server_api_functions = nullptr;
bool g_use_single_process = false;

namespace WeexCore {
    void WeexProxy::reset() {
        sConnection.reset();
        sHandler.reset();
    }

    static WeexString *genWeexString(const uint16_t *str, size_t length) {
        size_t byteSize = length * sizeof(uint16_t);
        auto *string = (WeexString *) malloc(byteSize + sizeof(WeexString));
        if (string == nullptr)
            return nullptr;

        memset(string, 0, byteSize + sizeof(WeexString));
        string->length = length;
        memcpy(string->content, str, byteSize);
        return string;
    }

    static WeexByteArray *genWeexByteArray(const char *str, size_t strLen) {
        auto *ret = (WeexByteArray *) malloc(strLen + sizeof(WeexByteArray));

        if (ret == nullptr)
            return nullptr;

        memset(ret, 0, strLen + sizeof(WeexByteArray));

        ret->length = strLen;
        memcpy(ret->content, str, strLen);
        ret->content[strLen] = '\0';
        return ret;
    }

    static INIT_FRAMEWORK_PARAMS *
    genInitFrameworkParams(const char *type, const char *value) {
        auto *init_framework_params = (INIT_FRAMEWORK_PARAMS *) malloc(
                sizeof(INIT_FRAMEWORK_PARAMS));

        if (init_framework_params == nullptr)
            return nullptr;

        memset(init_framework_params, 0, sizeof(INIT_FRAMEWORK_PARAMS));

        init_framework_params->type = genWeexByteArray(type, strlen(type));
        init_framework_params->value = genWeexByteArray(value, strlen(value));

        // LOGE("genInitFrameworkParams and type is %s and length is %d", init_framework_params->type->content,
        //     init_framework_params->type->length);
        // LOGE("genInitFrameworkParams and value is %s and length is %d", init_framework_params->value->content,
        //     init_framework_params->value->length);

        return init_framework_params;
    }

    static WeexString *jstring2WeexString(JNIEnv *env, jstring fromJString) {
        if (fromJString != nullptr) {
            ScopedJString scopedString(env, fromJString);
            return genWeexString(scopedString.getChars(), scopedString.getCharsLength());
        } else {
            uint16_t tmp = 0;
            return genWeexString(&tmp, 0);
        }
    }

    static VALUE_WITH_TYPE *getValueWithTypePtr() {
        auto *param = (VALUE_WITH_TYPE *) malloc(sizeof(VALUE_WITH_TYPE));
        if (param == nullptr)
            return nullptr;

        memset(param, 0, sizeof(VALUE_WITH_TYPE));

        return param;
    }

    inline void addParamsFromJArgs(std::vector<VALUE_WITH_TYPE *> &params, VALUE_WITH_TYPE *param,
                                   std::unique_ptr<IPCSerializer> &serializer, JNIEnv *env,
                                   jobject jArg) {
        jfieldID jTypeId = env->GetFieldID(jWXJSObject, "type", "I");
        jint jTypeInt = env->GetIntField(jArg, jTypeId);

        jfieldID jDataId = env->GetFieldID(jWXJSObject, "data", "Ljava/lang/Object;");
        jobject jDataObj = env->GetObjectField(jArg, jDataId);
        if (jTypeInt == 1) {
            if (jDoubleValueMethodId == NULL) {
                jclass jDoubleClazz = env->FindClass("java/lang/Double");
                jDoubleValueMethodId = env->GetMethodID(jDoubleClazz, "doubleValue", "()D");
                env->DeleteLocalRef(jDoubleClazz);
            }
            jdouble jDoubleObj = env->CallDoubleMethod(jDataObj, jDoubleValueMethodId);
            if (js_server_api_functions != nullptr) {
                param->type = ParamsType::DOUBLE;
                param->value.doubleValue = jDoubleObj;
            } else {
                serializer->add(jDoubleObj);
            }

        } else if (jTypeInt == 2) {
            jstring jDataStr = (jstring) jDataObj;
            if (js_server_api_functions != nullptr) {
                param->type = ParamsType::STRING;
                param->value.string = jstring2WeexString(env, jDataStr);
            } else {
                addString(env, serializer.get(), jDataStr);
            }
        } else if (jTypeInt == 3) {
            jstring jDataStr = (jstring) jDataObj;
            if (js_server_api_functions != nullptr) {
                param->type = ParamsType::JSONSTRING;
                param->value.string = jstring2WeexString(env, jDataStr);
            } else {
                addJSONString(env, serializer.get(), jDataStr);
            }
        } else if (jTypeInt == 4) {
            jbyteArray dataArray = (jbyteArray) jDataObj;
            if (js_server_api_functions != nullptr) {
                param->type = ParamsType::BYTEARRAY;
                jbyte *data = env->GetByteArrayElements(dataArray, 0);
                size_t length = env->GetArrayLength(dataArray);
                param->value.byteArray = genWeexByteArray((const char *) data, length);
            } else {
                addBinaryByteArray(env, serializer.get(), dataArray);
            }
        } else {
            if (js_server_api_functions != nullptr) {
                param->type = ParamsType::JSUNDEFINED;
            } else {
                serializer->addJSUndefined();
            }
        }

        if (param != nullptr) {
            params.push_back(param);
        }

        env->DeleteLocalRef(jDataObj);
    }

    inline void freeParams(std::vector<VALUE_WITH_TYPE *> &params) {
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

    jint WeexProxy::doInitFramework(JNIEnv *env, jobject object,
                                    jstring script, jobject params,
                                    jstring cacheDir, jboolean pieSupport) {
        const char *cache = env->GetStringUTFChars(
                reinterpret_cast<jstring>(cacheDir), nullptr);
        if (strlen(cache) > 0) {
            s_cacheDir = cache;
        }
        s_start_pie = pieSupport;
        return doInitFramework(env, jThis, script, params);
    }

    jint
    WeexProxy::initFrameworkInMultiProcess(JNIEnv *env, jstring script, jobject params,
                                           IPCSerializer *serializer) {

        IPCSerializer *realSerializer = nullptr;
        std::unique_ptr<IPCSerializer> serializerTemp(createIPCSerializer());
        bool reinit = false;
        startInitFrameWork:
        try {
            sHandler = std::move(createIPCHandler());
            sConnection.reset(new WeexJSConnection());
            sSender = sConnection->start(sHandler.get(), reinit);
            if (sSender == nullptr) {
                LOGE("JSFreamwork init start sender is null");
                if (!reinit) {
                    reinit = true;
                    goto startInitFrameWork;
                } else {
                    return false;
                }
            } else {
                if (reinit) {
                    initFromParam(env, script,
                                  params,
                                  serializerTemp.get());
                    realSerializer = serializerTemp.get();
                } else {
                    realSerializer = serializer;
                }

                // initHandler(sHandler.get());

                ExtendJSApi *pExtensionJSApi = new ExtendJSApi();

                pExtensionJSApi->initFunction(sHandler.get());

                // using base::debug::TraceEvent;
                // TraceEvent::StartATrace(env);
                realSerializer->setMsg(static_cast<uint32_t>(IPCJSMsg::INITFRAMEWORK));
                std::unique_ptr<IPCBuffer> buffer = realSerializer->finish();
                std::unique_ptr<IPCResult> result = sSender->send(buffer.get());
                if (result->getType() != IPCType::INT32) {
                    LOGE("initFramework Unexpected result type");
                    reportException("", "initFramework",
                                    "error, initFramework Unexpected result type");
                    return false;
                }
                return result->get<jint>();
            }
        } catch (IPCException &e) {
            LOGE("WeexProxy catch：%s", e.msg());
            if (!reinit) {
                reinit = true;
                goto startInitFrameWork;
            } else {
                LOGE("%s", e.msg());
                reportException("", "initFramework", e.msg());
                return false;
            }
        }
    }

    jint WeexProxy::doInitFramework(JNIEnv *env,
                                    jobject object,
                                    jstring script,
                                    jobject params) {

        Bridge_Impl_Android::getInstance()->setGlobalRef(jThis);
        WeexCoreManager::getInstance()->setPlatformBridge(Bridge_Impl_Android::getInstance());
        WeexCoreManager::getInstance()->setJSBridge(new JSBridge());
        WeexCoreManager::getInstance()->SetMeasureFunctionAdapter(
                new MeasureFunctionAdapterImplAndroid());
        std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
        const std::vector<INIT_FRAMEWORK_PARAMS *> &initFrameworkParams = initFromParam(env,
                                                                                        script,
                                                                                        params,
                                                                                        serializer.get());
        LOGE("Single process ? %s", g_use_single_process ? "true" : "false");
        if (g_use_single_process) {
            if (initFrameworkInSingleProcess(env, script, initFrameworkParams)) {
                //reportNativeInitStatus("-1011", "init Single Process Success");
                return true;
            }

            if (initFrameworkInMultiProcess(env, script, params, serializer.get())) {
                return true;
            }
        } else {
            if (initFrameworkInMultiProcess(env, script, params, serializer.get())) {
                return true;
            }

            if (initFrameworkInSingleProcess(env, script, initFrameworkParams)) {
                reportNativeInitStatus("-1011", "init Single Process Success");
                return true;
            }
        }

        reportNativeInitStatus("-1010", "init Failed");
        return false;

    }

    bool WeexProxy::execJSService(JNIEnv *env, jobject object, jstring script) {
        if (!sSender && !js_server_api_functions) {
            LOGE("have not connected to a js server");
            return false;
        }

        if (js_server_api_functions != nullptr) {
            ScopedJStringUTF8 source(env, script);
            js_server_api_functions->funcExeJsService(source.getChars());
            return true;
        } else {
            try {
                std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
                serializer->setMsg(static_cast<uint32_t>(IPCJSMsg::EXECJSSERVICE));
                addString(env, serializer.get(), script);
                std::unique_ptr<IPCBuffer> buffer = serializer->finish();
                std::unique_ptr<IPCResult> result = sSender->send(buffer.get());
                if (result->getType() != IPCType::INT32) {
                    LOGE("execJSService Unexpected result type");
                    return false;
                }
                return result->get<jint>();
            } catch (IPCException &e) {
                LOGE("%s", e.msg());
                return false;
            }
            return true;
        }
    }

    bool WeexProxy::execJS(JNIEnv *env,
                           jobject jthis,
                           jstring jinstanceid,
                           jstring jnamespace,
                           jstring jfunction,
                           jobjectArray jargs) {
        std::string mMessage = "";
        if (!sSender && !js_server_api_functions) {
            LOGE("have not connected to a js server");
            return false;
        }
        if (jfunction == nullptr || jinstanceid == nullptr) {
            LOGE("native_execJS function is NULL");
            return false;
        }
        int length = 0;
        if (jargs != nullptr) {
            length = env->GetArrayLength(jargs);
        }
        try {
            std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
            std::vector<VALUE_WITH_TYPE *> params;
            if (js_server_api_functions == nullptr) {
                serializer->setMsg(static_cast<uint32_t>(IPCJSMsg::EXECJS));
                addString(env, serializer.get(), jinstanceid);
                if (jnamespace)
                    addString(env, serializer.get(), jnamespace);
                else {
                    uint16_t tmp = 0;
                    serializer->add(&tmp, 0);
                }
                addString(env, serializer.get(), jfunction);
            }

            for (int i = 0; i < length; i++) {
                VALUE_WITH_TYPE *param = nullptr;

                if (js_server_api_functions != nullptr) {
                    param = getValueWithTypePtr();
                    if (param == nullptr)
                        return false;
                }

                jobject jArg = env->GetObjectArrayElement(jargs, i);
                addParamsFromJArgs(params, param, serializer, env, jArg);
                env->DeleteLocalRef(jArg);
            }

            if (js_server_api_functions != nullptr) {

                ScopedJStringUTF8 idChar(env, jinstanceid);
                const char *name;
                if (jnamespace) {
                    ScopedJStringUTF8 nameSpaceChar(env, jnamespace);
                    name = nameSpaceChar.getChars();
                } else {
                    name = nullptr;
                }
                ScopedJStringUTF8 funcChar(env, jfunction);

                int i = js_server_api_functions->funcExeJS(idChar.getChars(),
                                                           name,
                                                           funcChar.getChars(),
                                                           params);
                freeParams(params);
                return static_cast<bool>(i);
            } else {
                std::unique_ptr<IPCBuffer> buffer = serializer->finish();

                std::unique_ptr<IPCResult> result = sSender->send(buffer.get());
                if (result->getType() != IPCType::INT32) {
                    LOGE("execJS Unexpected result type");
                    return false;
                }

                return result->get<jint>();
            }


        } catch (IPCException &e) {
            LOGE("%s", e.msg());
            // report crash here
            WeexProxy::reportServerCrash(jinstanceid);
            return false;
        }
        return true;
    }

    std::string WeexProxy::findLibJssSoPath() {
        std::string executablePath = "";
        unsigned long target = reinterpret_cast<unsigned long>(__builtin_return_address(0));
        FILE *f = fopen("/proc/self/maps", "r");
        if (!f) {
            return "";
        }
        char buffer[256];
        char *line;
        while ((line = fgets(buffer, 256, f))) {
            char *end;
            unsigned long val;
            errno = 0;
            val = strtoul(line, &end, 16);
            if (errno)
                continue;
            if (val > target)
                continue;
            end += 1;
            errno = 0;
            val = strtoul(end, &end, 16);
            if (errno)
                continue;
            if (val > target) {
                char *s = strstr(end, "/");
                if (s != nullptr)
                    executablePath.assign(s);
                std::size_t found = executablePath.rfind('/');
                if (found != std::string::npos) {
                    executablePath = executablePath.substr(0, found);
                }
            }
            if (!executablePath.empty()) {
                break;
            }
        }
        fclose(f);
        LOGE("find so path: %s", executablePath.c_str());
        std::string::size_type pos = std::string::npos;
        // dynamic deploy
        if (!executablePath.empty() && executablePath.find(".maindex") != std::string::npos) {
            std::string libs[] = {"/opt", "/oat/arm"};
            auto libsCount = sizeof(libs) / sizeof(std::string);
            for (int i = 0; i < libsCount; ++i) {
                auto lib = libs[i];
                pos = executablePath.find(lib);
                if (pos != std::string::npos) {
                    executablePath.replace(pos, lib.length(), "/lib");
                    break;
                }
            }
        }
        std::string soPath = executablePath + "/" + g_jssSoName;
        // -------------------------------------------------
        // -------------------------------------------------
        if (access(soPath.c_str(), 00) == 0) {
            return soPath;
        } else {
            const char *error = soPath.c_str();
            LOGE("so path: %s is not exsist, use full package lib", error);
            executablePath = s_cacheDir;
            std::string lib = "/cache";
            if ((pos = soPath.find(lib)) != std::string::npos) {
                executablePath.replace(pos, lib.length(), "/lib");
            }
            soPath = executablePath + "/" + g_jssSoName;
            if (access(soPath.c_str(), 00) != 0) {
                LOGE("so path: %s is not exsist", soPath.c_str());
                reportNativeInitStatus("-1004", error);
                //return false;
                //use libweexjss.so directly
                soPath = g_jssSoName;
            }
            return soPath;
        }
    }

    static WEEX_CORE_JS_API_FUNCTIONS *getWeexCoreApiFunctions() {
        WEEX_CORE_JS_API_FUNCTIONS tempFunctions = {
                _setJSVersion,
                _reportException,
                _callNative,
                _callNativeModule,
                _callNativeComponent,
                _callAddElement,
                _setTimeout,
                _callNativeLog,
                _callCreateBody,
                _callUpdateFinish,
                _callCreateFinish,
                _callRefreshFinish,
                _callUpdateAttrs,
                _callUpdateStyle,
                _callRemoveElement,
                _callMoveElement,
                _callAddEvent,
                _callRemoveEvent,
                _setInterval,
                _clearInterval,
                _callGCanvasLinkNative,
                _t3dLinkNative,
                callHandlePostMessage,
                callDIspatchMessage
        };

        auto *functions = (WEEX_CORE_JS_API_FUNCTIONS *) malloc(
                sizeof(WEEX_CORE_JS_API_FUNCTIONS));

        if (!functions) {
            return nullptr;
        }

        memset(functions, 0, sizeof(WEEX_CORE_JS_API_FUNCTIONS));
        memcpy(functions, &tempFunctions, sizeof(WEEX_CORE_JS_API_FUNCTIONS));

        return functions;
    }

    typedef WEEX_CORE_JS_SERVER_API_FUNCTIONS *(*InitMethodFunc)(
            WEEX_CORE_JS_API_FUNCTIONS *functions);

    jint
    WeexProxy::initFrameworkInSingleProcess(JNIEnv *env, jstring script,
                                            const std::vector<INIT_FRAMEWORK_PARAMS *> initFrameworkParams) {
        std::string soPath = "";

        // -----------------------------------------------
        // use find path to get lib path
        // use set path is better idea, should change in future
        if (g_jssSoPath != nullptr) {
            soPath = g_jssSoPath;
        }

        if (soPath.empty()) {
            soPath = findLibJssSoPath();
        }

        LOGE("final executablePath:%s", soPath.c_str());

        void *handle = dlopen(soPath.c_str(), RTLD_NOW);
        if (!handle) {
            const char *error = dlerror();
            LOGE("load %s failed,error=%s\n", g_jssSoName, error);
            reportNativeInitStatus("-1005", error);
            // try again use current path
            dlclose(handle);
            return false;
        }

        //clear dlopen error message
        dlerror();

        LOGE("dlopen so and call function");
        auto initMethod = (InitMethodFunc) dlsym(handle, "exchangeMethod");
        if (!initMethod) {
            const char *error = dlerror();
            LOGE("load External_InitFrameWork failed,error=%s\n", error);
            reportNativeInitStatus("-1006", error);
            dlclose(handle);
            return false;
        }

        //clear dlopen error message
        dlerror();

        WEEX_CORE_JS_API_FUNCTIONS *pFunctions = getWeexCoreApiFunctions();

        js_server_api_functions = initMethod(pFunctions);

        if (js_server_api_functions != nullptr) {
            js_server_api_functions->funcInitFramework(env->GetStringUTFChars(script, nullptr),
                                                       std::move(initFrameworkParams));
            return true;
        } else {
            dlclose(handle);
            free(pFunctions);
            free(js_server_api_functions);
            reportNativeInitStatus("-1007", "Init Functions failed");
            return false;
        }
    }


    std::vector<INIT_FRAMEWORK_PARAMS *> WeexProxy::initFromParam(JNIEnv *env,
                                                                  jstring script,
                                                                  jobject params,
                                                                  IPCSerializer *serializer) {
        {
            ScopedJString scopedString(env, script);
            const jchar *chars = scopedString.getChars();
            int charLength = scopedString.getCharsLength();
            serializer->add(chars, charLength);
        }
        LOGE("initFromParam is running ");
        std::vector<INIT_FRAMEWORK_PARAMS *> initFrameworkParams;

#define ADDSTRING(name)                                     \
    {                                                       \
        const char* myname = #name;                         \
        serializer->add(myname, strlen(myname));            \
        ScopedJStringUTF8 scopedString(env, (jstring)name); \
        const char* chars = scopedString.getChars();        \
        int charLength = strlen(chars);                     \
        serializer->add(chars, charLength);                 \
        initFrameworkParams.push_back(genInitFrameworkParams(myname,chars));\
    }
        jclass c_params = env->GetObjectClass(params);

        jmethodID m_platform = env->GetMethodID(c_params,
                                                "getPlatform",
                                                "()Ljava/lang/String;");
        jobject platform = env->CallObjectMethod(params, m_platform);
        if (!WXCoreEnvironment::getInstance()->SetPlatform(
                jString2StrFast(env, reinterpret_cast<jstring &>(platform)))) {
            LOGD("setPlatform");
        }
        ADDSTRING(platform);


        jmethodID m_use_single_process = env->GetMethodID(c_params, "getUseSingleProcess",
                                                          "()Ljava/lang/String;");
        if (m_use_single_process == nullptr) {
            LOGE("getUseSingleProcess method is missing");
        } else {
            jobject j_use_single_process = env->CallObjectMethod(params, m_use_single_process);
            const char *use_single_process = env->GetStringUTFChars(
                    (jstring) (j_use_single_process),
                    nullptr);
            LOGE("g_use_single_process is %s ", use_single_process);
            g_use_single_process = strstr(use_single_process, "true") != nullptr;
        }

        jmethodID m_get_jss_so_path = env->GetMethodID(c_params, "getLibJssPath",
                                                       "()Ljava/lang/String;");
        if (m_get_jss_so_path != nullptr) {
            jobject j_get_jss_so_path = env->CallObjectMethod(params, m_get_jss_so_path);
            if (j_get_jss_so_path != nullptr) {
                g_jssSoPath = env->GetStringUTFChars(
                        (jstring) (j_get_jss_so_path),
                        nullptr);
                LOGE("g_jssSoPath is %s ", g_jssSoPath);
            }
        }

        jmethodID m_osVersion = env->GetMethodID(
                c_params, "getOsVersion", "()Ljava/lang/String;");
        jobject osVersion = env->CallObjectMethod(params, m_osVersion);
        ADDSTRING(osVersion);

        // use param ti get cacheDir
        jmethodID m_cacheMethod = env->GetMethodID(
                c_params, "getCacheDir", "()Ljava/lang/String;");
        if (m_cacheMethod != NULL) {
            jobject cacheDir = env->CallObjectMethod(params, m_cacheMethod);
            if (cacheDir != NULL) {
                ADDSTRING(cacheDir);
            }
        }

        jmethodID m_appVersion = env->GetMethodID(
                c_params, "getAppVersion", "()Ljava/lang/String;");
        jobject appVersion = env->CallObjectMethod(params, m_appVersion);
        ADDSTRING(appVersion);

        jmethodID m_weexVersion = env->GetMethodID(
                c_params, "getWeexVersion", "()Ljava/lang/String;");
        jobject weexVersion = env->CallObjectMethod(params, m_weexVersion);
        ADDSTRING(weexVersion);

        jmethodID m_deviceModel = env->GetMethodID(
                c_params, "getDeviceModel", "()Ljava/lang/String;");
        jobject deviceModel = env->CallObjectMethod(params, m_deviceModel);
        ADDSTRING(deviceModel);

        jmethodID m_appName = env->GetMethodID(
                c_params, "getAppName", "()Ljava/lang/String;");
        jobject appName = env->CallObjectMethod(params, m_appName);
        ADDSTRING(appName);

        jmethodID m_deviceWidth = env->GetMethodID(
                c_params, "getDeviceWidth", "()Ljava/lang/String;");
        jobject deviceWidth = env->CallObjectMethod(params, m_deviceWidth);
        if (!WXCoreEnvironment::getInstance()->SetDeviceWidth(
                jString2StrFast(env, reinterpret_cast<jstring &>(deviceWidth)))) {
            LOGD("setDeviceWidth");
        }
        ADDSTRING(deviceWidth);

        jmethodID m_deviceHeight = env->GetMethodID(
                c_params, "getDeviceHeight", "()Ljava/lang/String;");
        jobject deviceHeight = env->CallObjectMethod(params, m_deviceHeight);
        if (!WXCoreEnvironment::getInstance()->SetDeviceHeight(
                jString2StrFast(env, reinterpret_cast<jstring &>(deviceHeight)))) {
            LOGD("setDeviceHeight");
        }
        ADDSTRING(deviceHeight);

        jmethodID m_options = env->GetMethodID(
                c_params, "getOptions", "()Ljava/lang/Object;");
        jobject options = env->CallObjectMethod(params, m_options);
        jclass jmapclass = env->FindClass("java/util/HashMap");
        jmethodID jkeysetmid = env->GetMethodID(jmapclass,
                                                "keySet",
                                                "()Ljava/util/Set;");
        jmethodID jgetmid = env->GetMethodID(
                jmapclass,
                "get",
                "(Ljava/lang/Object;)Ljava/lang/Object;");
        jobject jsetkey = env->CallObjectMethod(options, jkeysetmid);
        jclass jsetclass = env->FindClass("java/util/Set");
        jmethodID jtoArraymid = env->GetMethodID(jsetclass,
                                                 "toArray",
                                                 "()[Ljava/lang/Object;");
        jobjectArray jobjArray =
                (jobjectArray) env->CallObjectMethod(jsetkey, jtoArraymid);
        env->DeleteLocalRef(jsetkey);
        if (jobjArray != NULL) {
            jsize arraysize = env->GetArrayLength(jobjArray);
            for (int i = 0; i < arraysize; i++) {
                jstring jkey = (jstring) env->GetObjectArrayElement(jobjArray, i);
                jstring jvalue = (jstring) env->CallObjectMethod(options, jgetmid, jkey);

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
                    const char *c_key_chars = c_key.getChars();
                    int c_key_len = strlen(c_key_chars);
                    const char *c_value_chars = c_value.getChars();
                    int c_value_len = strlen(c_value_chars);
                    serializer->add(c_key_chars, c_key_len);
                    serializer->add(c_value_chars, c_value_len);
                    initFrameworkParams.push_back(
                            genInitFrameworkParams(c_key_chars, c_value_chars));
                    const std::string &key = jString2Str(env, jkey);
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

    void
    WeexProxy::reportException(const char *instanceID,
                               const char *func,
                               const char *exception_string) {
        _reportException(instanceID, func, exception_string);
//        JNIEnv *env = getJNIEnv();
//        jstring jExceptionString = env->NewStringUTF(exception_string);
//        jstring jInstanceId = env->NewStringUTF(instanceID);
//        jstring jFunc = env->NewStringUTF(func);
//        jmethodID tempMethodId = env->GetMethodID(
//                jBridgeClazz,
//                "reportJSException",
//                "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
//        env->CallVoidMethod(jThis,
//                            tempMethodId,
//                            jInstanceId,
//                            jFunc,
//                            jExceptionString);
//        env->DeleteLocalRef(jExceptionString);
//        env->DeleteLocalRef(jInstanceId);
//        env->DeleteLocalRef(jFunc);
    }

    void WeexProxy::reportServerCrash(jstring jinstanceid) {
        JNIEnv *env = getJNIEnv();
        jmethodID reportMethodId;
        jstring crashFile;
        std::string crashFileStr;
        reportMethodId = env->GetMethodID(jBridgeClazz,
                                          "reportServerCrash",
                                          "(Ljava/lang/String;Ljava/lang/String;)V");
        if (!reportMethodId)
            goto no_method;

        crashFileStr.assign("/jsserver_crash/jsserver_crash_info.log");

        crashFile = env->NewStringUTF(crashFileStr.c_str());
        env->CallVoidMethod(jThis, reportMethodId, jinstanceid, crashFile);
        env->DeleteLocalRef(crashFile);
        no_method:
        env->ExceptionClear();
    }

    void WeexProxy::reportNativeInitStatus(const char *statusCode, const char *errorMsg) {
        JNIEnv *env = getJNIEnv();
        jmethodID reportMethodId;
        jstring errorCodeString = env->NewStringUTF(statusCode);

        std::string m_errorMsgString = "useSingleProcess is ";
        m_errorMsgString.append(g_use_single_process ? "true" : "false");
        m_errorMsgString.append(" And Error Msg is ");
        m_errorMsgString.append(errorMsg);


        LOGE("reportNativeInitStatus error msg is %s ", m_errorMsgString.c_str());

        jstring errorMsgString = env->NewStringUTF(m_errorMsgString.c_str());
        reportMethodId = env->GetMethodID(jBridgeClazz,
                                          "reportNativeInitStatus",
                                          "(Ljava/lang/String;Ljava/lang/String;)V");


        if (!reportMethodId)
            goto no_method;


        env->CallVoidMethod(jThis, reportMethodId, errorCodeString, errorMsgString);

        no_method:
        env->DeleteLocalRef(errorCodeString);
        env->DeleteLocalRef(errorMsgString);
        env->ExceptionClear();
    }

    const char *WeexProxy::getCacheDir(JNIEnv *env) {
        jclass activityThreadCls, applicationCls, fileCls;
        jobject applicationObj, fileObj, pathStringObj;
        jmethodID currentApplicationMethodId,
                getCacheDirMethodId,
                getAbsolutePathMethodId;
        static std::string storage;
        const char *tmp;
        const char *ret = nullptr;
        if (!storage.empty()) {
            ret = storage.c_str();
            goto no_empty;
        }
        activityThreadCls = env->FindClass("android/app/ActivityThread");
        if (!activityThreadCls || env->ExceptionOccurred()) {
            goto no_class;
        }
        currentApplicationMethodId = env->GetStaticMethodID(
                activityThreadCls,
                "currentApplication",
                "()Landroid/app/Application;");
        if (!currentApplicationMethodId || env->ExceptionOccurred()) {
            goto no_currentapplication_method;
        }
        applicationObj = env->CallStaticObjectMethod(activityThreadCls,
                                                     currentApplicationMethodId,
                                                     nullptr);
        if (!applicationObj || env->ExceptionOccurred()) {
            goto no_application;
        }
        applicationCls = env->GetObjectClass(applicationObj);
        getCacheDirMethodId = env->GetMethodID(applicationCls,
                                               "getCacheDir",
                                               "()Ljava/io/File;");
        if (!getCacheDirMethodId || env->ExceptionOccurred()) {
            goto no_getcachedir_method;
        }
        fileObj = env->CallObjectMethod(applicationObj, getCacheDirMethodId, nullptr);
        if (!fileObj || env->ExceptionOccurred()) {
            goto no_file_obj;
        }
        fileCls = env->GetObjectClass(fileObj);
        getAbsolutePathMethodId = env->GetMethodID(fileCls,
                                                   "getAbsolutePath",
                                                   "()Ljava/lang/String;");
        if (!getAbsolutePathMethodId || env->ExceptionOccurred()) {
            goto no_getabsolutepath_method;
        }
        pathStringObj = env->CallObjectMethod(fileObj,
                                              getAbsolutePathMethodId,
                                              nullptr);
        if (!pathStringObj || env->ExceptionOccurred()) {
            goto no_path_string;
        }
        tmp = env->GetStringUTFChars(reinterpret_cast<jstring>(pathStringObj),
                                     nullptr);
        storage.assign(tmp);
        env->ReleaseStringUTFChars(reinterpret_cast<jstring>(pathStringObj),
                                   tmp);
        ret = storage.c_str();
        no_path_string:
        no_getabsolutepath_method:
        env->DeleteLocalRef(fileCls);
        env->DeleteLocalRef(fileObj);
        no_file_obj:
        no_getcachedir_method:
        env->DeleteLocalRef(applicationCls);
        env->DeleteLocalRef(applicationObj);
        no_application:
        no_currentapplication_method:
        env->DeleteLocalRef(activityThreadCls);
        no_class:
        env->ExceptionDescribe();
        env->ExceptionClear();
        no_empty:
        return ret;
    }

    void WeexProxy::setCacheDir(JNIEnv *env) {
        s_cacheDir = getCacheDir(env);
    }

    jbyteArray WeexProxy::execJSWithResult(JNIEnv *env, jobject jthis,
                                           jstring jinstanceid,
                                           jstring jnamespace,
                                           jstring jfunction,
                                           jobjectArray jargs) {
        if (!sSender && !js_server_api_functions) {
            LOGE("have not connected to a js server");
            return NULL;
        }
        if (jfunction == NULL || jinstanceid == NULL) {
            LOGE("native_execJS function is NULL");
            return NULL;
        }

        int length = 0;
        if (jargs != NULL) {
            length = env->GetArrayLength(jargs);
        }
        try {
            std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
            std::vector<VALUE_WITH_TYPE *> params;
            if (js_server_api_functions == nullptr) {
                serializer->setMsg(static_cast<uint32_t>(IPCJSMsg::EXECJSWITHRESULT));
                addString(env, serializer.get(), jinstanceid);
                if (jnamespace)
                    addString(env, serializer.get(), jnamespace);
                else {
                    uint16_t tmp = 0;
                    serializer->add(&tmp, 0);
                }
                addString(env, serializer.get(), jfunction);
            }

            for (int i = 0; i < length; i++) {
                VALUE_WITH_TYPE *param = nullptr;

                if (js_server_api_functions != nullptr) {
                    param = getValueWithTypePtr();
                    if (param == nullptr)
                        return nullptr;
                }

                jobject jArg = env->GetObjectArrayElement(jargs, i);
                addParamsFromJArgs(params, param, serializer, env, jArg);
                env->DeleteLocalRef(jArg);
            }

            if (js_server_api_functions != nullptr) {
                ScopedJStringUTF8 instanceidChar(env, jinstanceid);
                const char *namespaceChar;
                if (jnamespace) {
                    ScopedJStringUTF8 nameSpaceChar(env, jnamespace);
                    namespaceChar = nameSpaceChar.getChars();
                } else {
                    namespaceChar = nullptr;
                }
                ScopedJStringUTF8 functionChar(env, jfunction);
                WeexJSResult jsResultData = js_server_api_functions->funcExeJSWithResult(
                        instanceidChar.getChars(), namespaceChar, functionChar.getChars(), params);
                freeParams(params);
                jbyteArray array = newJByteArray(env, jsResultData.data, jsResultData.length);
                WeexJSResultDataFree(jsResultData);
                return array;
            } else {

                std::unique_ptr<IPCBuffer> buffer = serializer->finish();
                std::unique_ptr<IPCResult> result = sSender->send(buffer.get());
                if (result->getType() != IPCType::BYTEARRAY) {
                    return NULL;
                }
                if (result->getByteArrayLength() == 0) {
                    return NULL;
                }
                jbyteArray array = env->NewByteArray(result->getByteArrayLength());
                env->SetByteArrayRegion(array, 0, result->getByteArrayLength(),
                                        reinterpret_cast<const jbyte *>(result->getByteArrayContent()));
                return array;
            }
        } catch (IPCException &e) {
            LOGE("%s", e.msg());
            // report crash here
            reportServerCrash(jinstanceid);
            return NULL;
        }
        return NULL;
    }

    void WeexProxy::updateGlobalConfig(JNIEnv *env, jobject jcaller, jstring config) {
        if (!sSender && js_server_api_functions == nullptr) {
            LOGE("have not connected to a js server");
            return;
        }
        if (config == NULL) {
            LOGE("native_execJS function is NULL");
            return;
        }

        if (js_server_api_functions != nullptr) {
            js_server_api_functions->funcUpdateGlobalConfig(
                    env->GetStringUTFChars(config, nullptr));
            return;
        } else {
            try {
                std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
                serializer->setMsg(static_cast<uint32_t>(IPCJSMsg::UPDATEGLOBALCONFIG));
                addString(env, serializer.get(), config);
                std::unique_ptr<IPCBuffer> buffer = serializer->finish();
                std::unique_ptr<IPCResult> result = sSender->send(buffer.get());
            } catch (IPCException &e) {
                LOGE("%s", e.msg());
            }
        }
    }

    static jstring getJsonData(JNIEnv *env, jobjectArray jargs, int index) {
        int length = 0;
        if (jargs != NULL) {
            length = env->GetArrayLength(jargs);
        }
        jstring ret = NULL;
        if (length < (index + 1)) {
            return ret;
        }
        jobject jArg = env->GetObjectArrayElement(jargs, index);

        jfieldID jTypeId = env->GetFieldID(jWXJSObject, "type", "I");
        jint jTypeInt = env->GetIntField(jArg, jTypeId);
        jfieldID jDataId = env->GetFieldID(jWXJSObject, "data", "Ljava/lang/Object;");
        jobject jDataObj = env->GetObjectField(jArg, jDataId);
        if (jTypeInt == 3) {
            ret = (jstring) jDataObj;
        }
        // env->DeleteLocalRef(jDataObj);
        env->DeleteLocalRef(jArg);
        return ret;
    }

    jint
    WeexProxy::createInstanceContext(JNIEnv *env, jobject jcaller, jstring jinstanceid,
                                     jstring name,
                                     jstring jfunction, jobjectArray jargs) {
        if (!sSender && js_server_api_functions == nullptr) {
            LOGE("have not connected to a js server");
            return false;
        }
        if (jfunction == NULL || jinstanceid == NULL) {
            LOGE("native_createInstanceContext function is NULL");
            return false;
        }

        int length = 0;
        if (jargs != NULL) {
            length = env->GetArrayLength(jargs);
        }
        if (length < 4) {
            LOGE("native_createInstanceContext jargs format error");
            return false;
        }
// get temp data, such as js bundle
        jobject jArg = env->GetObjectArrayElement(jargs, 1);
        jfieldID jDataId = env->GetFieldID(jWXJSObject, "data", "Ljava/lang/Object;");
        jobject jDataObj = env->GetObjectField(jArg, jDataId);
        jstring jscript = (jstring) jDataObj;
        jstring opts = getJsonData(env, jargs, 2);
        // init jsonData
        jstring initData = getJsonData(env, jargs, 3);
        // get extend api data, such as rax-api
        jArg = env->GetObjectArrayElement(jargs, 4);
        jDataObj = env->GetObjectField(jArg, jDataId);
        jstring japi = (jstring) jDataObj;
        if (js_server_api_functions != nullptr) {
            ScopedJStringUTF8 idChar(env, jinstanceid);
            ScopedJStringUTF8 funcChar(env, jfunction);
            ScopedJStringUTF8 scriptChar(env, jscript);
            ScopedJStringUTF8 optsChar(env, opts);
            ScopedJStringUTF8 initDataChar(env, initData);
            ScopedJStringUTF8 apiChar(env, japi);

            return js_server_api_functions->funcCreateInstance(
                    idChar.getChars(),
                    funcChar.getChars(),
                    scriptChar.getChars(),
                    optsChar.getChars(),
                    initDataChar.getChars(),
                    apiChar.getChars());
        } else {
            try {
                std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
                serializer->setMsg(static_cast<uint32_t>(IPCJSMsg::CREATEINSTANCE));
                addString(env, serializer.get(), jinstanceid);
                addString(env, serializer.get(), jfunction);
                addString(env, serializer.get(), jscript);
                addJSONString(env, serializer.get(), opts);
                addJSONString(env, serializer.get(), initData);
                addString(env, serializer.get(), japi);

                std::unique_ptr<IPCBuffer> buffer = serializer->finish();
                std::unique_ptr<IPCResult> result = sSender->send(buffer.get());
                env->DeleteLocalRef(jArg);
                env->DeleteLocalRef(jDataObj);
                env->DeleteLocalRef(opts);
                if (result->getType() != IPCType::INT32) {
                    LOGE("createInstanceContext Unexpected result type");
                    return false;
                }
                return result->get<jint>();
            } catch (IPCException &e) {
                LOGE("%s", e.msg());
                // report crash here
                reportServerCrash(jinstanceid);
                return false;
            }
        }
        return true;
    }

    jint WeexProxy::destoryInstance(JNIEnv *env, jobject jcaller, jstring jinstanceid,
                                    jstring jnamespace,
                                    jstring jfunction, jobjectArray jargs) {
        int ret = execJS(env, nullptr, jinstanceid, jnamespace, jfunction, jargs);
        if (jfunction == NULL || jinstanceid == NULL) {
            LOGE("native_destoryInstance function is NULL");
            return false;
        }

        if (js_server_api_functions != nullptr) {
            ScopedJStringUTF8 idChar(env, jinstanceid);
            return js_server_api_functions->funcDestroyInstance(idChar.getChars());
        } else {
            try {
                std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
                serializer->setMsg(static_cast<uint32_t>(IPCJSMsg::DESTORYINSTANCE));
                addString(env, serializer.get(), jinstanceid);

                std::unique_ptr<IPCBuffer> buffer = serializer->finish();
                std::unique_ptr<IPCResult> result = sSender->send(buffer.get());
                if (result->getType() != IPCType::INT32) {
                    LOGE("destoryInstance Unexpected result type");
                    return false;
                }
                return result->get<jint>();
            } catch (IPCException &e) {
                LOGE("%s", e.msg());
                // report crash here
                reportServerCrash(jinstanceid);
                return false;
            }
        }
        return true;
    }

    jstring
    WeexProxy::execJSOnInstance(JNIEnv *env, jobject jcaller, jstring instanceId,
                                jstring script,
                                jint type) {
        if (instanceId == NULL || script == NULL) {
            return env->NewStringUTF("");
        }


        if (js_server_api_functions != nullptr) {
            ScopedJStringUTF8 idChar(env, instanceId);
            ScopedJStringUTF8 scriptChar(env, script);
            char *string = js_server_api_functions->funcExeJSOnInstance(
                    idChar.getChars(),
                    scriptChar.getChars());
            return env->NewStringUTF(string);
        } else {
            try {
                // base::debug::TraceScope traceScope("weex", "native_execJSOnInstance");
                std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
                serializer->setMsg(static_cast<uint32_t>(IPCJSMsg::EXECJSONINSTANCE));

                addString(env, serializer.get(), instanceId);
                addString(env, serializer.get(), script);

                std::unique_ptr<IPCBuffer> buffer = serializer->finish();
                std::unique_ptr<IPCResult> result = sSender->send(buffer.get());
                if (result->getType() != IPCType::BYTEARRAY) {
                    // LOGE("native_execJSOnInstance return type error");
                    return env->NewStringUTF("");
                }
                return env->NewStringUTF(result->getByteArrayContent());
            } catch (IPCException &e) {
                LOGE("%s", e.msg());
                // report crash here
                reportServerCrash(instanceId);
                return env->NewStringUTF("");
            }
        }
    }

    jint WeexProxy::initAppFramework(JNIEnv *env,
                                     jobject jcaller,
                                     jstring jinstanceid,
                                     jstring jframwork,
                                     jobjectArray jargs) {
        if (!sSender && js_server_api_functions == nullptr) {
            LOGE("have not connected to a js server");
            return false;
        }
        if (jframwork == NULL && jargs == NULL) {
            LOGE("native_initAppFramework jframwork is NULL");
            return false;
        }
        int length = 0;
        if (jargs != NULL) {
            length = env->GetArrayLength(jargs);
        }
        try {
            std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
            std::vector<INIT_FRAMEWORK_PARAMS *> params;
            if (js_server_api_functions == nullptr) {
                serializer->setMsg(static_cast<uint32_t>(IPCJSMsg::INITAPPFRAMEWORK));
                addString(env, serializer.get(), jinstanceid);
                addString(env, serializer.get(), jframwork);
            }


            for (int i = 0; i < length; i++) {
                jobject jArg = env->GetObjectArrayElement(jargs, i);

                jfieldID jTypeId = env->GetFieldID(jWXJSObject, "type", "I");
                jint jTypeInt = env->GetIntField(jArg, jTypeId);

                jfieldID jDataId = env->GetFieldID(jWXJSObject, "data", "Ljava/lang/Object;");
                jobject jDataObj = env->GetObjectField(jArg, jDataId);

                jfieldID jKeyId = env->GetFieldID(jWXJSObject, "key", "Ljava/lang/String;");
                jobject jKeyObj = env->GetObjectField(jArg, jKeyId);
                jstring jKeyStr = (jstring) jKeyObj;
                if (js_server_api_functions == nullptr) {

                    addByteArrayString(env, serializer.get(), jKeyStr);
                }

                if (jTypeInt == 1) {
                    if (jDoubleValueMethodId == NULL) {
                        jclass jDoubleClazz = env->FindClass("java/lang/Double");
                        jDoubleValueMethodId = env->GetMethodID(jDoubleClazz, "doubleValue",
                                                                "()D");
                        env->DeleteLocalRef(jDoubleClazz);
                    }
                    jdouble jDoubleObj = env->CallDoubleMethod(jDataObj, jDoubleValueMethodId);

                    serializer->add(jDoubleObj);

                } else if (jTypeInt == 2) {
                    jstring jDataStr = (jstring) jDataObj;
                    if (js_server_api_functions == nullptr) {
                        addByteArrayString(env, serializer.get(), jDataStr);
                    } else {
                        params.push_back(genInitFrameworkParams(env->GetStringUTFChars(jKeyStr,
                                                                                       nullptr),
                                                                env->GetStringUTFChars(jDataStr,
                                                                                       nullptr)));
                    }
                } else if (jTypeInt == 3) {
                    jstring jDataStr = (jstring) jDataObj;
                    if (js_server_api_functions == nullptr) {
                        addByteArrayString(env, serializer.get(), jDataStr);
                    } else {
                        params.push_back(genInitFrameworkParams(env->GetStringUTFChars(jKeyStr,
                                                                                       nullptr),
                                                                env->GetStringUTFChars(jDataStr,
                                                                                       nullptr)));
                    }
                }
//            else if (jTypeInt == 4) {
//                jbyteArray array = (jbyteArray)jDataObj;
//                addWSONByteArray(env, serializer.get(), array);
//            }
                else {
                    serializer->addJSUndefined();
                }

                env->DeleteLocalRef(jKeyObj);
                env->DeleteLocalRef(jDataObj);
                env->DeleteLocalRef(jArg);
            }

            if (js_server_api_functions != nullptr) {
                ScopedJStringUTF8 idChar(env, jinstanceid);
                ScopedJStringUTF8 frameworkChar(env, jframwork);
                return js_server_api_functions->funcInitAppFramework(
                        idChar.getChars(),
                        frameworkChar.getChars(),
                        params);
            } else {
                std::unique_ptr<IPCBuffer> buffer = serializer->finish();
                std::unique_ptr<IPCResult> result = sSender->send(buffer.get());
                return result->get<jint>();
            }

        } catch (IPCException &e) {
            LOGE("initAppFramework error %s", e.msg());
            // report crash here
            reportServerCrash(jinstanceid);
            return false;
        }
        return true;
    }

    jint WeexProxy::destoryAppContext(JNIEnv *env,
                                      jobject jcaller,
                                      jstring jinstanceid) {
        if (!sSender && js_server_api_functions == nullptr) {
            LOGE("have not connected to a js server");
            return false;
        }
        if (jinstanceid == NULL) {
            LOGE("createAppContext jbundle is NULL");
            return false;
        }

        if (js_server_api_functions != nullptr) {
            ScopedJStringUTF8 scopedJStringUTF8(env, jinstanceid);
            js_server_api_functions->funcDestroyAppContext(scopedJStringUTF8.getChars());
            return true;
        } else {
            try {
                std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
                serializer->setMsg(static_cast<uint32_t>(IPCJSMsg::DESTORYAPPCONTEXT));
                addString(env, serializer.get(), jinstanceid);
                std::unique_ptr<IPCBuffer> buffer = serializer->finish();
                std::unique_ptr<IPCResult> result = sSender->send(buffer.get());
                return true;
            } catch (IPCException &e) {
                LOGE("%s", e.msg());
                // report crash here
                reportServerCrash(jinstanceid);
                return false;
            }
        }
        return true;
    }

    jint WeexProxy::createAppContext(JNIEnv *env,
                                     jobject jcaller,
                                     jstring jinstanceid,
                                     jstring jbundle,
                                     jobject jargs) {
        if (!sSender && js_server_api_functions == nullptr) {
            LOGE("have not connected to a js server");
            return false;
        }
        if (jinstanceid == NULL || jbundle == NULL) {
            LOGE("createAppContext jbundle is NULL");
            return false;
        }

        if (js_server_api_functions != nullptr) {
            ScopedJStringUTF8 instanceIdChar(env, jinstanceid);
            ScopedJStringUTF8 jBundleChar(env, jbundle);
            return js_server_api_functions->funcCreateAppContext(instanceIdChar.getChars(),
                                                                 jBundleChar.getChars());
        } else {
            try {
                std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
                serializer->setMsg(static_cast<uint32_t>(IPCJSMsg::CREATEAPPCONTEXT));
                addString(env, serializer.get(), jinstanceid);
                addString(env, serializer.get(), jbundle);
                std::unique_ptr<IPCBuffer> buffer = serializer->finish();
                std::unique_ptr<IPCResult> result = sSender->send(buffer.get());
                return result->get<jint>();
            } catch (IPCException &e) {
                LOGE("%s", e.msg());
                // report crash here
                reportServerCrash(jinstanceid);
                return false;
            }
        }
        return true;
    }

    jbyteArray WeexProxy::execJsOnAppWithResult(JNIEnv *env,
                                                jobject jcaller,
                                                jstring jinstanceid,
                                                jstring jbundle,
                                                jobject jargs) {
        if (!sSender && js_server_api_functions == nullptr) {
            LOGE("have not connected to a js server");
            return NULL;
        }
        if (jinstanceid == NULL || jbundle == NULL) {
            LOGE("native_execJsOnApp jbundle is NULL");
            return NULL;
        }

        if (js_server_api_functions != nullptr) {
            ScopedJStringUTF8 instanceIdChar(env, jinstanceid);
            ScopedJStringUTF8 jBundleChar(env, jbundle);
            char *string = js_server_api_functions->funcExeJSOnAppWithResult(
                    instanceIdChar.getChars(), jBundleChar.getChars());
            return newJByteArray(env, string);
        } else {
            try {
                std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
                serializer->setMsg(static_cast<uint32_t>(IPCJSMsg::EXECJSONAPPWITHRESULT));
                addString(env, serializer.get(), jinstanceid);
                addString(env, serializer.get(), jbundle);
                std::unique_ptr<IPCBuffer> buffer = serializer->finish();
                std::unique_ptr<IPCResult> result = sSender->send(buffer.get());
                if (result->getType() != IPCType::BYTEARRAY) {
                    return NULL;
                }
                if (result->getByteArrayLength() == 0) {
                    return NULL;
                }
                jbyteArray array = env->NewByteArray(result->getByteArrayLength());
                env->SetByteArrayRegion(array, 0, result->getByteArrayLength(),
                                        reinterpret_cast<const jbyte *>(result->getByteArrayContent()));
                return array;
            } catch (IPCException &e) {
                LOGE("%s", e.msg());
                // report crash here
                reportServerCrash(jinstanceid);
                return NULL;
            }
        }
        return NULL;
    }

    jint WeexProxy::execJsOnApp(JNIEnv *env,
                                jobject jcaller,
                                jstring jinstanceid,
                                jstring jfunction,
                                jobjectArray jargs) {
        if (!sSender && js_server_api_functions == nullptr) {
            LOGE("have not connected to a js server");
            return false;
        }
        if (jinstanceid == NULL || jfunction == NULL) {
            LOGE("native_execJsOnApp jbundle is NULL");
            return false;
        }
        int length = 0;
        if (jargs != NULL) {
            length = env->GetArrayLength(jargs);
        }
        try {
            std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
            std::vector<VALUE_WITH_TYPE *> params;
            if (js_server_api_functions == nullptr) {
                serializer->setMsg(static_cast<uint32_t>(IPCJSMsg::CALLJSONAPPCONTEXT));
                addString(env, serializer.get(), jinstanceid);
                addString(env, serializer.get(), jfunction);
            }


            for (int i = 0; i < length; i++) {
                jobject jArg = env->GetObjectArrayElement(jargs, i);

                jfieldID jTypeId = env->GetFieldID(jWXJSObject, "type", "I");
                jint jTypeInt = env->GetIntField(jArg, jTypeId);

                jfieldID jDataId = env->GetFieldID(jWXJSObject, "data", "Ljava/lang/Object;");
                jobject jDataObj = env->GetObjectField(jArg, jDataId);

                VALUE_WITH_TYPE *param = nullptr;

                if (js_server_api_functions != nullptr) {
                    param = getValueWithTypePtr();
                    if (param == nullptr)
                        return false;
                }


                if (jTypeInt == 1) {
                    if (jDoubleValueMethodId == NULL) {
                        jclass jDoubleClazz = env->FindClass("java/lang/Double");
                        jDoubleValueMethodId = env->GetMethodID(jDoubleClazz, "doubleValue",
                                                                "()D");
                        env->DeleteLocalRef(jDoubleClazz);
                    }
                    jdouble jDoubleObj = env->CallDoubleMethod(jDataObj, jDoubleValueMethodId);

                    if (js_server_api_functions != nullptr) {
                        param->type = ParamsType::DOUBLE;
                        param->value.doubleValue = jDoubleObj;

                    } else {
                        serializer->add(jDoubleObj);
                    }

                } else if (jTypeInt == 2) {
                    jstring jDataStr = (jstring) jDataObj;
                    if (js_server_api_functions != nullptr) {
                        param->type = ParamsType::STRING;
                        param->value.string = jstring2WeexString(env, jDataStr);
                    } else {
                        addString(env, serializer.get(), jDataStr);
                    }

                } else if (jTypeInt == 3) {
                    jstring jDataStr = (jstring) jDataObj;
                    if (js_server_api_functions != nullptr) {
                        param->type = ParamsType::JSONSTRING;
                        param->value.string = jstring2WeexString(env, jDataStr);
                    } else {
                        addJSONString(env, serializer.get(), jDataStr);
                    }
                }
//            else if (jTypeInt == 4) {
//                jbyteArray array = (jbyteArray)jDataObj;
//                addWSONByteArray(env, serializer.get(), array);
//            }
                else {
                    if (js_server_api_functions != nullptr) {
                        param->type = ParamsType::JSUNDEFINED;
                    } else {
                        serializer->addJSUndefined();
                    }
                }
                if (param != nullptr)
                    params.push_back(param);
                env->DeleteLocalRef(jDataObj);
                env->DeleteLocalRef(jArg);
            }
            if (js_server_api_functions != nullptr) {
                ScopedJStringUTF8 instanceId(env, jinstanceid);
                ScopedJStringUTF8 funcChar(env, jfunction);
                return js_server_api_functions->funcCallJSOnAppContext(instanceId.getChars(),
                                                                       funcChar.getChars(),
                                                                       params);
            } else {
                std::unique_ptr<IPCBuffer> buffer = serializer->finish();
                std::unique_ptr<IPCResult> result = sSender->send(buffer.get());
                if (result->getType() != IPCType::INT32) {
                    LOGE("execJS Unexpected result type");
                    return false;
                }
                return result->get<jint>();
            }
        } catch (IPCException &e) {
            LOGE("%s", e.msg());
            // report crash here
            reportServerCrash(jinstanceid);
            return false;
        }
        return true;
    }

}  // namespace WeexCore
