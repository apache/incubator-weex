#ifndef _WEEX_PROXY_H_
#define _WEEX_PROXY_H_


#include <jni.h>
#include <unistd.h>
#include <IPC/Serializing/IPCSerializer.h>
#include <IPC/IPCHandler.h>
#include <android/bridge/impl/bridge_impl_android.h>
#include <IPC/Buffering/IPCBuffer.h>
#include <IPC/IPCArguments.h>
#include <IPC/IPCByteArray.h>
#include <IPC/IPCException.h>
#include <IPC/IPCHandler.h>
#include <IPC/IPCMessageJS.h>
#include <IPC/IPCResult.h>
#include <IPC/IPCSender.h>
#include <IPC/IPCString.h>
#include <IPC/Serializing/IPCSerializer.h>

namespace WeexCore {

class WeexProxy {
private:
public:
    static void reset();

    static jint doInitFramework(JNIEnv *env, jobject object, jstring script, jobject params,
                                jstring cacheDir, jboolean pieSupport);

    static jint doInitFramework(JNIEnv *env, jobject object, jstring script, jobject params);

    static void
    initFromParam(JNIEnv *env, jstring script, jobject params, IPCSerializer *serializer);

    static const char *getCacheDir(JNIEnv *env);

    static void setCacheDir(JNIEnv *env);

    static bool execJSService(JNIEnv *env, jobject object, jstring script);

    static bool
    execJS(JNIEnv *env, jobject jthis, jstring jinstanceid, jstring jnamespace, jstring jfunction,
           jobjectArray jargs);

    static void
    reportException(const char *instanceID, const char *func, const char *exception_string);

    static void reportServerCrash(jstring jinstanceid);

    static jbyteArray execJSWithResult(JNIEnv* env, jobject jthis,
                                       jstring jinstanceid,
                                       jstring jnamespace,
                                       jstring jfunction,
                                       jobjectArray jargs);

    static void updateGlobalConfig(JNIEnv* env, jobject jcaller,
                                 jstring config);
    static jint createInstanceContext(JNIEnv* env, jobject jcaller, jstring instanceId,
                                             jstring name,
                                             jstring function,
                                             jobjectArray args);

    static jint destoryInstance(JNIEnv* env, jobject jcaller,
                                jstring instanceId,
                                jstring name,
                                jstring function,
                                jobjectArray args);

    static jstring execJSOnInstance(JNIEnv* env, jobject jcaller,
                                    jstring instanceId,
                                    jstring script,
                                    jint type);
};
}
#endif //_WEEX_PROXY_H_
