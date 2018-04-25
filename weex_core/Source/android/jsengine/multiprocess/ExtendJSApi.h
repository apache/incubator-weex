#ifndef _EXTEND_JSAPI_H_
#define _EXTEND_JSAPI_H_

#include <android/base/log_utils.h>
#include <android/bridge/impl/bridge_impl_android.h>
#include "IPC/IPCHandler.h"
#include "IPC/IPCMessageJS.h"
#include "IPC/IPCResult.h"
#include "IPC/IPCSender.h"
#include "IPC/IPCString.h"
#include "IPC/Serializing/IPCSerializer.h"
#include <core/render/manager/render_manager.h>
#include <jni.h>
#include <sys/types.h>


static std::unique_ptr<IPCResult> handleSetJSVersion(IPCArguments *arguments);

static std::unique_ptr<IPCResult> handleReportException(IPCArguments *arguments);

static std::unique_ptr<IPCResult> handleCallNative(IPCArguments *arguments);

static std::unique_ptr<IPCResult> handleCallNativeModule(IPCArguments *arguments);

static std::unique_ptr<IPCResult> handleCallNativeComponent(IPCArguments *arguments);

static std::unique_ptr<IPCResult> handleCallAddElement(IPCArguments *arguments);

static std::unique_ptr<IPCResult> handleSetTimeout(IPCArguments *arguments);

static std::unique_ptr<IPCResult> handleCallNativeLog(IPCArguments *arguments);

static std::unique_ptr<IPCResult> functionCallCreateBody(IPCArguments *arguments);

static std::unique_ptr<IPCResult> functionCallUpdateFinish(IPCArguments *arguments);

static std::unique_ptr<IPCResult> functionCallCreateFinish(IPCArguments *arguments);

static std::unique_ptr<IPCResult> functionCallRefreshFinish(IPCArguments *arguments);

static std::unique_ptr<IPCResult> functionCallUpdateAttrs(IPCArguments *arguments);

static std::unique_ptr<IPCResult> functionCallUpdateStyle(IPCArguments *arguments);

static std::unique_ptr<IPCResult> functionCallRemoveElement(IPCArguments *arguments);

static std::unique_ptr<IPCResult> functionCallMoveElement(IPCArguments *arguments);

static std::unique_ptr<IPCResult> functionCallAddEvent(IPCArguments *arguments);

static std::unique_ptr<IPCResult> functionCallRemoveEvent(IPCArguments *arguments);

static std::unique_ptr<IPCResult> handleSetInterval(IPCArguments *arguments);

static std::unique_ptr<IPCResult> handleClearInterval(IPCArguments *arguments);

static std::unique_ptr<IPCResult> handleCallGCanvasLinkNative(IPCArguments *arguments);

static std::unique_ptr<IPCResult> handleT3DLinkNative(IPCArguments *arguments);


static void reportException(const int instanceID, const char *func, const char *exception_string);

namespace WeexCore {

  typedef const char *(*FunType)(const char *, int, const char *);

  const char *callGCanvasFun(FunType fp, const char *conextId, int x, const char *args);

  extern "C" void Inject_GCanvasFunc(FunType fp);

  typedef const char* (*FunTypeT3d)(int, const char*);

  const char* weexCallT3dFunc(FunTypeT3d fp, int x, const char* args);

  extern "C" void Inject_T3dFunc(FunTypeT3d fp);

  class ExtendJSApi {
  public:
    ExtendJSApi();

    void initFunction(IPCHandler *handler);
  };

} //WeexCore

#endif // _EXTENSION_JSAPI_H_