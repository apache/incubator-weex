#ifndef _EXTEND_JSAPI_H_
#define _EXTEND_JSAPI_H_

#include "../../../base/LogUtils.h"
#include "IPCHandler.h"
#include "IPCMessageJS.h"
#include "IPCResult.h"
#include "IPCSender.h"
#include "IPCString.h"
#include "Serializing/IPCSerializer.h"
#include "../../render/RenderManager.h"
#include "../../../rapidjson/weexjsontools.h"


static std::unique_ptr <IPCResult> handleSetJSVersion(IPCArguments *arguments);

static std::unique_ptr <IPCResult> handleReportException(IPCArguments *arguments);

static std::unique_ptr <IPCResult> handleCallNative(IPCArguments *arguments);

static std::unique_ptr <IPCResult> handleCallNativeModule(IPCArguments *arguments);

static std::unique_ptr <IPCResult> handleCallNativeComponent(IPCArguments *arguments);

static std::unique_ptr <IPCResult> handleCallAddElement(IPCArguments *arguments);

static std::unique_ptr <IPCResult> handleSetTimeout(IPCArguments *arguments);

static std::unique_ptr <IPCResult> handleCallNativeLog(IPCArguments *arguments);

static std::unique_ptr <IPCResult> functionCallCreateBody(IPCArguments *arguments);

static std::unique_ptr <IPCResult> functionCallUpdateFinish(IPCArguments *arguments);

static std::unique_ptr <IPCResult> functionCallCreateFinish(IPCArguments *arguments);

static std::unique_ptr <IPCResult> functionCallRefreshFinish(IPCArguments *arguments);

static std::unique_ptr <IPCResult> functionCallUpdateAttrs(IPCArguments *arguments);

static std::unique_ptr <IPCResult> functionCallUpdateStyle(IPCArguments *arguments);

static std::unique_ptr <IPCResult> functionCallRemoveElement(IPCArguments *arguments);

static std::unique_ptr <IPCResult> functionCallMoveElement(IPCArguments *arguments);

static std::unique_ptr <IPCResult> functionCallAddEvent(IPCArguments *arguments);

static std::unique_ptr <IPCResult> functionCallRemoveEvent(IPCArguments *arguments);


static void reportException(const char *instanceID, const char *func, const char *exception_string);

namespace WeexCore {

class ExtendJSApi {
public:
    ExtendJSApi(jobject &jThis);

    void initFunction(IPCHandler *handler);
};

} //WeexCore

#endif // _EXTENSION_JSAPI_H_