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

    virtual void reportException(const int pageId, const char *func, const char *exception_string) = 0;

    virtual int callNative(const int pageId, const char *task, const char *callback) = 0;

    virtual jobject callNativeModule(const int pageId, const char *module, const char *method,
                                     const char *argString, const char *optString) = 0;

    virtual void callNativeComponent(const int pageId, const char *ref, const char *method,
                                     const char *argString, const char *optString) = 0;

    virtual void setTimeout(const char* callbackID, const char* time) = 0;

    virtual void callNativeLog(const char* str_array) = 0;

    virtual int callUpdateFinish(const int pageId, const char *task, const char *callback) = 0;

    virtual int callRefreshFinish(const int pageId, const char *task, const char *callback) = 0;

    virtual int callAddEvent(int &pageId, const int &ref, const char *event) = 0;

    virtual int callRemoveEvent(int &pageId, const int &ref, const char *event) = 0;

    virtual int callCreateBody(int &pageId, const char *componentType, const int &ref,
                               std::map<std::string, std::string> *styles,
                               std::map<std::string, std::string> *attributes,
                               std::set<std::string> *events,
                               const WXCoreMargin &margins,
                               const WXCorePadding &paddings,
                               const WXCoreBorderWidth &borders) = 0;

    virtual int callAddElement(int &pageId, const char *componentType, const int &ref,
                               int &index, const int &parentRef,
                               std::map<std::string, std::string> *styles,
                               std::map<std::string, std::string> *attributes,
                               std::set<std::string> *events,
                               const WXCoreMargin &margins,
                               const WXCorePadding &paddings,
                               const WXCoreBorderWidth &borders) = 0;

    virtual int callLayout(int &pageId, const int &ref,
                           int top, int bottom, int left, int right,
                           int height, int width) = 0;

    virtual int callUpdateStyle(int &pageId, const int &ref,
                                std::vector<std::pair<std::string, std::string>> *style,
                                std::vector<std::pair<std::string, std::string>> *margin,
                                std::vector<std::pair<std::string, std::string>> *padding,
                                std::vector<std::pair<std::string, std::string>> *border) = 0;

    virtual int callUpdateAttr(int &pageId, const int &ref,
                               std::vector<std::pair<std::string, std::string>> *attrs) = 0;

    virtual int callCreateFinish(int &pageId) = 0;

    virtual int callRemoveElement(int &pageId, const int &ref) = 0;

    virtual int callMoveElement(int &pageId, const int &ref, const int &parentRef, int index) = 0;
  };
} //end WeexCore
#endif //Bridge_h
