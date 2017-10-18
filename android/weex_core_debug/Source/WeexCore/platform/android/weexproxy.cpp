#include "WeexProxy.h"
#include <base/android/LogUtils.h>
#include <base/android/string/StringUtils.h>
#include "WeexCore.h"
#include "ExtendJSApi.h"

const char *s_cacheDir;
bool s_start_pie = true;

static IPCSender *sSender;
static std::unique_ptr<IPCHandler> sHandler;
static std::unique_ptr<WeexJSConnection> sConnection;


namespace WeexCore {
void WeexProxy::reset() {
  sConnection.reset();
  sHandler.reset();
}

jint WeexProxy::doInitFramework(JNIEnv *env, jobject object, jstring script, jobject params,
                                jstring cacheDir, jboolean pieSupport) {
  const char *cache = env->GetStringUTFChars(reinterpret_cast<jstring>(cacheDir), nullptr);
  if (strlen(cache) > 0) {
    s_cacheDir = cache;
  }
  s_start_pie = pieSupport;
  return doInitFramework(env, jThis, script, params);
}

jint WeexProxy::doInitFramework(JNIEnv *env,
                                jobject object,
                                jstring script,
                                jobject params) {
  bool reinit = false;
  startInitFrameWork:
  try {
    sHandler = std::move(createIPCHandler());
    sConnection.reset(new WeexJSConnection());
    sSender = sConnection->start(sHandler.get(), reinit);
    // initHandler(sHandler.get());

    ExtendJSApi *pExtensionJSApi = new ExtendJSApi();
    BridgeAndroid::getInstance()->setGlobalRef(jThis);
    pExtensionJSApi->initFunction(sHandler.get());

    // using base::debug::TraceEvent;
    // TraceEvent::StartATrace(env);
    std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
    serializer->setMsg(static_cast<uint32_t>(IPCJSMsg::INITFRAMEWORK));
    initFromParam(env, script, params, serializer.get());
    std::unique_ptr<IPCBuffer> buffer = serializer->finish();
    std::unique_ptr<IPCResult> result = sSender->send(buffer.get());
    if (result->getType() != IPCType::INT32) {
      LOGE("initFramework Unexpected result type");
      reportException("", "initFramework",
                      "doInitFramework error, initFramework Unexpected result type");
      return false;
    }
    return result->get<jint>();
  } catch (IPCException &e) {
    if (!reinit) {
      reinit = true;
      goto startInitFrameWork;
    } else {
      LOGE("%s", e.msg());
      reportException("", "initFramework", e.msg());
      return false;
    }
  }
  return true;
}

bool WeexProxy::execJSService(JNIEnv *env, jobject object, jstring script) {
  if (!sSender) {
    LOGE("have not connected to a js server");
    return false;
  }
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

bool WeexProxy::execJS(JNIEnv *env,
                       jobject jthis,
                       jstring jinstanceid,
                       jstring jnamespace,
                       jstring jfunction,
                       jobjectArray jargs) {
  if (!sSender) {
    LOGE("have not connected to a js server");
    return false;
  }
  int length = 0;
  if (jargs != NULL) {
    length = env->GetArrayLength(jargs);
  }
  try {
    std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
    serializer->setMsg(static_cast<uint32_t>(IPCJSMsg::EXECJS));
    addString(env, serializer.get(), jinstanceid);
    if (jnamespace)
      addString(env, serializer.get(), jnamespace);
    else {
      uint16_t tmp = 0;
      serializer->add(&tmp, 0);
    }
    addString(env, serializer.get(), jfunction);

    for (int i = 0; i < length; i++) {
      jobject jArg = env->GetObjectArrayElement(jargs, i);

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
        serializer->add(jDoubleObj);

      } else if (jTypeInt == 2) {
        jstring jDataStr = (jstring) jDataObj;
        addString(env, serializer.get(), jDataStr);
      } else if (jTypeInt == 3) {
        jstring jDataStr = (jstring) jDataObj;
        addJSONString(env, serializer.get(), jDataStr);
      } else {
        serializer->addJSUndefined();
      }
      env->DeleteLocalRef(jDataObj);
      env->DeleteLocalRef(jArg);
    }
    std::unique_ptr<IPCBuffer> buffer = serializer->finish();
    std::unique_ptr<IPCResult> result = sSender->send(buffer.get());
    if (result->getType() != IPCType::INT32) {
      LOGE("execJS Unexpected result type");
      return false;
    }
    return result->get<jint>();
  } catch (IPCException &e) {
    LOGE("%s", e.msg());
    // report crash here
    WeexProxy::reportServerCrash(jinstanceid);
    return false;
  }
  return true;
}


void
WeexProxy::initFromParam(JNIEnv *env, jstring script, jobject params, IPCSerializer *serializer) {
  {
    ScopedJString scopedString(env, script);
    const jchar *chars = scopedString.getChars();
    int charLength = scopedString.getCharsLength();
    serializer->add(chars, charLength);
  }
#define ADDSTRING(name)                                     \
    {                                                       \
        const char* myname = #name;                         \
        serializer->add(myname, strlen(myname));            \
        ScopedJStringUTF8 scopedString(env, (jstring)name); \
        const char* chars = scopedString.getChars();        \
        int charLength = strlen(chars);                     \
        serializer->add(chars, charLength);                 \
    }
  jclass c_params = env->GetObjectClass(params);

  jmethodID m_platform = env->GetMethodID(c_params, "getPlatform", "()Ljava/lang/String;");
  jobject platform = env->CallObjectMethod(params, m_platform);
  ADDSTRING(platform);

  jmethodID m_osVersion = env->GetMethodID(c_params, "getOsVersion", "()Ljava/lang/String;");
  jobject osVersion = env->CallObjectMethod(params, m_osVersion);
  ADDSTRING(osVersion);

  // use param ti get cacheDir
  jmethodID m_cacheMethod = env->GetMethodID(c_params, "getCacheDir", "()Ljava/lang/String;");
  if (m_cacheMethod != NULL) {
    jobject cacheDir = env->CallObjectMethod(params, m_cacheMethod);
    if (cacheDir != NULL) {
      ADDSTRING(cacheDir);
    }
  }


  jmethodID m_appVersion = env->GetMethodID(c_params, "getAppVersion", "()Ljava/lang/String;");
  jobject appVersion = env->CallObjectMethod(params, m_appVersion);
  ADDSTRING(appVersion);

  jmethodID m_weexVersion = env->GetMethodID(c_params, "getWeexVersion", "()Ljava/lang/String;");
  jobject weexVersion = env->CallObjectMethod(params, m_weexVersion);
  ADDSTRING(weexVersion);

  jmethodID m_deviceModel = env->GetMethodID(c_params, "getDeviceModel", "()Ljava/lang/String;");
  jobject deviceModel = env->CallObjectMethod(params, m_deviceModel);
  ADDSTRING(deviceModel);

  jmethodID m_appName = env->GetMethodID(c_params, "getAppName", "()Ljava/lang/String;");
  jobject appName = env->CallObjectMethod(params, m_appName);
  ADDSTRING(appName);

  jmethodID m_deviceWidth = env->GetMethodID(c_params, "getDeviceWidth", "()Ljava/lang/String;");
  jobject deviceWidth = env->CallObjectMethod(params, m_deviceWidth);
  ADDSTRING(deviceWidth);

  jmethodID m_deviceHeight = env->GetMethodID(c_params, "getDeviceHeight",
                                              "()Ljava/lang/String;");
  jobject deviceHeight = env->CallObjectMethod(params, m_deviceHeight);
  ADDSTRING(deviceHeight);

  jmethodID m_options = env->GetMethodID(c_params, "getOptions", "()Ljava/lang/Object;");
  jobject options = env->CallObjectMethod(params, m_options);
  jclass jmapclass = env->FindClass("java/util/HashMap");
  jmethodID jkeysetmid = env->GetMethodID(jmapclass, "keySet", "()Ljava/util/Set;");
  jmethodID jgetmid = env->GetMethodID(jmapclass, "get",
                                       "(Ljava/lang/Object;)Ljava/lang/Object;");
  jobject jsetkey = env->CallObjectMethod(options, jkeysetmid);
  jclass jsetclass = env->FindClass("java/util/Set");
  jmethodID jtoArraymid = env->GetMethodID(jsetclass, "toArray", "()[Ljava/lang/Object;");
  jobjectArray jobjArray = (jobjectArray) env->CallObjectMethod(jsetkey, jtoArraymid);
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
      }
    }
    env->DeleteLocalRef(jobjArray);
  }
  env->DeleteLocalRef(options);
}

void
WeexProxy::reportException(const char *instanceID, const char *func, const char *exception_string) {
  JNIEnv *env = getJNIEnv();
  jstring jExceptionString = env->NewStringUTF(exception_string);
  jstring jInstanceId = env->NewStringUTF(instanceID);
  jstring jFunc = env->NewStringUTF(func);
  jmethodID tempMethodId = env->GetMethodID(jBridgeClazz,
                                            "reportJSException",
                                            "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
  env->CallVoidMethod(jThis, tempMethodId, jInstanceId, jFunc, jExceptionString);
  env->DeleteLocalRef(jExceptionString);
  env->DeleteLocalRef(jInstanceId);
  env->DeleteLocalRef(jFunc);
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

const char *WeexProxy::getCacheDir(JNIEnv *env) {
  jclass activityThreadCls, applicationCls, fileCls;
  jobject applicationObj, fileObj, pathStringObj;
  jmethodID currentApplicationMethodId, getCacheDirMethodId, getAbsolutePathMethodId;
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
  currentApplicationMethodId = env->GetStaticMethodID(activityThreadCls, "currentApplication",
                                                      "()Landroid/app/Application;");
  if (!currentApplicationMethodId || env->ExceptionOccurred()) {
    goto no_currentapplication_method;
  }
  applicationObj = env->CallStaticObjectMethod(activityThreadCls, currentApplicationMethodId,
                                               nullptr);
  if (!applicationObj || env->ExceptionOccurred()) {
    goto no_application;
  }
  applicationCls = env->GetObjectClass(applicationObj);
  getCacheDirMethodId = env->GetMethodID(applicationCls, "getCacheDir", "()Ljava/io/File;");
  if (!getCacheDirMethodId || env->ExceptionOccurred()) {
    goto no_getcachedir_method;
  }
  fileObj = env->CallObjectMethod(applicationObj, getCacheDirMethodId, nullptr);
  if (!fileObj || env->ExceptionOccurred()) {
    goto no_file_obj;
  }
  fileCls = env->GetObjectClass(fileObj);
  getAbsolutePathMethodId = env->GetMethodID(fileCls, "getAbsolutePath", "()Ljava/lang/String;");
  if (!getAbsolutePathMethodId || env->ExceptionOccurred()) {
    goto no_getabsolutepath_method;
  }
  pathStringObj = env->CallObjectMethod(fileObj, getAbsolutePathMethodId, nullptr);
  if (!pathStringObj || env->ExceptionOccurred()) {
    goto no_path_string;
  }
  tmp = env->GetStringUTFChars(reinterpret_cast<jstring>(pathStringObj), nullptr);
  storage.assign(tmp);
  env->ReleaseStringUTFChars(reinterpret_cast<jstring>(pathStringObj), tmp);
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

} //WeexCore