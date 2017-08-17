#ifndef Bridge_h
#define Bridge_h

#include "WTFString.h"
#include <jni.h>

//TODO: remove platform related code
namespace WeexCore {

class Bridge {
public:
    virtual void setJSVersion(jstring &jversion) = 0;

    virtual void reportException(jstring &jInstanceId, jstring &jFunc, jstring &jExceptionString) = 0;

    virtual int callNative(jstring &instanceId, jbyteArray &taskString, jstring &callback) = 0;

    virtual jobject
    callNativeModule(jstring &jInstanceId, jstring &jmodule, jstring &jmethod, jbyteArray &jArgString,
                     jbyteArray &jOptString) = 0;

    virtual void callNativeComponent(jstring &jInstanceId, jstring &jcomponentRef, jstring &jmethod,
                                     jbyteArray &jArgString, jbyteArray &jOptString) = 0;

    virtual int callAddElement(jstring &jInstanceId, jstring &jref, jbyteArray &jdomString,
                               jstring &jindex, jstring &jCallback) = 0;

    virtual int callCreateBody(jstring &instanceId, jbyteArray &taskString, jstring &callback) = 0;

    virtual void setTimeout(jstring &jCallbackID, jstring &jTime) = 0;

    virtual void callNativeLog(jstring &str_msg) = 0;


    virtual int callUpdateFinish(jstring &instanceId, jbyteArray &taskString,
                                 jstring &callback) = 0;

    virtual int callCreateFinish(jstring &instanceId, jbyteArray &taskString,
                                 jstring &callback) = 0;

    virtual int callRefreshFinish(jstring &instanceId, jbyteArray &taskString,
                                  jstring &callback) = 0;

    virtual int callUpdateAttrs(jstring &instanceId, jstring &ref,
                                jbyteArray &taskString, jstring &callback) = 0;

    virtual int callUpdateStyle(jstring &instanceId, jstring &ref,
                                jbyteArray &taskString, jstring &callback) = 0;

    virtual int callRemoveElement(jstring &instanceId, jstring &ref,
                                  jstring &callback) = 0;

    virtual int callMoveElement(jstring &instanceId, jstring &ref, jstring &parentref,
                                jstring &index, jstring &callback) = 0;

    virtual int callAddEvent(jstring &instanceId,
                             jstring &ref, jstring &event, jstring &Callback) = 0;

    virtual int callRemoveEvent(jstring &instanceId,
                                jstring &ref, jstring &event, jstring &callback) = 0;
};
} //end WeexCore
#endif //Bridge_h
