#ifndef BridgeAndroid_h
#define BridgeAndroid_h
#include "../../bridge/Bridge.h"
#include "WeexCore.h"

namespace WeexCore {

class BridgeAndroid : public Bridge {

private:
    jobject jThis;

public:
    BridgeAndroid();
    ~BridgeAndroid();

    void setGlobalRef(jobject& jRef)
    {
        jThis = jRef;
    }

    void setJSVersion(jstring& jversion);

    void reportException(jstring& jInstanceId, jstring& jFunc, jstring& jExceptionString);

    int callNative(jstring& instanceId, jbyteArray& taskString, jstring& callback);

    jobject callNativeModule(jstring& jInstanceId, jstring& jmodule, jstring& jmethod, jbyteArray& jArgString, jbyteArray& jOptString);

    void callNativeComponent(jstring& jInstanceId, jstring& jcomponentRef, jstring& jmethod,
        jbyteArray& jArgString, jbyteArray& jOptString);

    int callAddElement(jstring& jInstanceId, jstring& jref, jbyteArray& jdomString,
        jstring& jindex, jstring& jCallback);

    void setTimeout(jstring &jCallbackID, jstring& jTime);

    void callNativeLog(jstring& str_msg);

    int callCreateBody(jstring& instanceId, jbyteArray& taskString, jstring& callback);

    int callUpdateFinish(jstring& instanceId, jbyteArray& taskString,
        jstring& callback);

    int callCreateFinish(jstring& instanceId, jbyteArray& taskString,
        jstring& callback);

    int callRefreshFinish(jstring& instanceId, jbyteArray& taskString,
        jstring& callback);

    int callUpdateAttrs(jstring& instanceId, jstring& ref,
        jbyteArray& taskString, jstring& callback);

    int callUpdateStyle(jstring& instanceId, jstring& ref,
        jbyteArray& taskString, jstring& callback);

    int callRemoveElement(jstring& instanceId, jstring& ref,
        jstring& callback);

    int callMoveElement(jstring& instanceId, jstring& ref, jstring& parentref,
        jstring& index, jstring& callback);

    int callAddEvent(jstring& instanceId,
        jstring& ref, jstring& event, jstring& Callback);

    int callRemoveEvent(jstring& instanceId,
        jstring& ref, jstring& event, jstring& callback);
};
} //end WeexCore
#endif //BridgeAndroid_h