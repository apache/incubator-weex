#ifndef Bridge_h
#define Bridge_h

#include <jni.h>
#include <string>
#include <map>
#include <set>
#include <vector>

namespace WeexCore {

  class WXCoreMargin;

  class WXCorePadding;

  class WXCoreBorderWidth;

  class Bridge {
  public:
    virtual void setJSVersion(const char* version) = 0;

    virtual void reportException(const char* pageId, const char *func, const char *exception_string) = 0;

    virtual int callNative(const char* pageId, const char *task, const char *callback) = 0;

    virtual jobject callNativeModule(const char* pageId, const char *module, const char *method,
                                     const char *argString, const char *optString) = 0;

    virtual void callNativeComponent(const char* pageId, const char* ref, const char *method,
                                     const char *argString, const char *optString) = 0;

    virtual void setTimeout(const char* callbackID, const char* time) = 0;

    virtual void callNativeLog(const char* str_array) = 0;

    virtual int callUpdateFinish(const char* pageId, const char *task, const char *callback) = 0;

    virtual int callRefreshFinish(const char* pageId, const char *task, const char *callback) = 0;

    virtual int callAddEvent(const char* pageId, const char* ref, const char *event) = 0;

    virtual int callRemoveEvent(const char* pageId, const char* ref, const char *event) = 0;

    virtual int callCreateBody(const char* pageId, const char *componentType, const char* ref,
                               std::map<std::string, std::string> *styles,
                               std::map<std::string, std::string> *attributes,
                               std::set<std::string> *events,
                               const WXCoreMargin &margins,
                               const WXCorePadding &paddings,
                               const WXCoreBorderWidth &borders) = 0;

    virtual int callAddElement(const char* pageId, const char *componentType, const char* ref,
                               int &index, const char* parentRef,
                               std::map<std::string, std::string> *styles,
                               std::map<std::string, std::string> *attributes,
                               std::set<std::string> *events,
                               const WXCoreMargin &margins,
                               const WXCorePadding &paddings,
                               const WXCoreBorderWidth &borders) = 0;

    virtual int callLayout(const char* pageId, const char* ref,
                           int top, int bottom, int left, int right,
                           int height, int width) = 0;

    virtual int callUpdateStyle(const char* pageId, const char* ref,
                                std::vector<std::pair<std::string, std::string>> *style,
                                std::vector<std::pair<std::string, std::string>> *margin,
                                std::vector<std::pair<std::string, std::string>> *padding,
                                std::vector<std::pair<std::string, std::string>> *border) = 0;

    virtual int callUpdateAttr(const char* pageId, const char* ref,
                               std::vector<std::pair<std::string, std::string>> *attrs) = 0;

    virtual int callCreateFinish(const char* pageId) = 0;

    virtual int callRemoveElement(const char* pageId, const char* ref) = 0;

    virtual int callMoveElement(const char* pageId, const char* ref, const char* parentRef, int index) = 0;
  };
} //end WeexCore
#endif //Bridge_h
