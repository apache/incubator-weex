#ifndef _WEEX_PROXY_H_
#define _WEEX_PROXY_H_


#include <jni.h>
#include <unistd.h>
#include <IPC/Serializing/IPCSerializer.h>
#include <IPC/IPCHandler.h>
#include "BridgeAndroid.h"

namespace WeexCore {

  class WeexProxy {
  private:
  public:
    static void reset();

    static jint doInitFramework(JNIEnv *env, jobject object, jstring script, jobject params);

    static void
    initFromParam(JNIEnv *env, jstring script, jobject params, IPCSerializer *serializer);

    static void initHandler(IPCHandler *handler);

    static const char *getCacheDir(JNIEnv *env);

    static void setCacheDir(JNIEnv *env);

    static bool execJSService(JNIEnv *env, jobject object, jstring script);

    static bool
    execJS(JNIEnv *env, jobject jthis, jstring jinstanceid, jstring jnamespace, jstring jfunction,
           jobjectArray jargs);

    static void
    reportException(const char *instanceID, const char *func, const char *exception_string);

    static void reportServerCrash(jstring jinstanceid);
  };
}
#endif //_WEEX_PROXY_H_
