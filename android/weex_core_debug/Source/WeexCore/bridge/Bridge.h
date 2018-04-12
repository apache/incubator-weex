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
    virtual void setJSVersion(jstring &jversion) = 0;

    virtual void
    reportException(jstring &jInstanceId, jstring &jFunc, jstring &jExceptionString) = 0;

    virtual int callNative(char *pageId, char *task, char *callback) = 0;

    virtual jobject callNativeModule(char *pageId, char *module, char *method,
                                     char *argString, char *optString) = 0;

    virtual void callNativeComponent(char *pageId, char *ref, char *method,
                                     char *argString, char *optString) = 0;

    virtual void setTimeout(jstring &jCallbackID, jstring &jTime) = 0;

    virtual void callNativeLog(jbyteArray &str_msg) = 0;

    virtual int callUpdateFinish(jstring &instanceId, jbyteArray &taskString,
                                 jstring &callback) = 0;

    virtual int callRefreshFinish(jstring &instanceId, jbyteArray &taskString,
                                  jstring &callback) = 0;

    virtual int callAddEvent(int &pageId,
                             std::string &ref, std::string &event) = 0;

    virtual int callRemoveEvent(int &pageId,
                                std::string &ref, std::string &event) = 0;

    virtual int
    callCreateBody(int &pageId, std::string &componentType, std::string &ref,
                   std::map<std::string, std::string> *styles,
                   std::map<std::string, std::string> *attributes,
                   std::set<std::string> *events,
                   const WXCoreMargin &margins,
                   const WXCorePadding &paddings,
                   const WXCoreBorderWidth &borders) = 0;

    virtual int
    callAddElement(int &pageId, std::string &componentType, std::string &ref,
                   int &index,
                   std::string &parentRef,
                   std::map<std::string, std::string> *styles,
                   std::map<std::string, std::string> *attributes,
                   std::set<std::string> *events,
                   const WXCoreMargin &margins,
                   const WXCorePadding &paddings,
                   const WXCoreBorderWidth &borders) = 0;

    virtual int callLayout(int &pageId, std::string &ref,
                           int top, int bottom, int left, int right,
                           int height, int width) = 0;

    virtual int callUpdateStyle(int &pageId, std::string &ref,
                                std::vector<std::pair<std::string, std::string>> *style,
                                std::vector<std::pair<std::string, std::string>> *margin,
                                std::vector<std::pair<std::string, std::string>> *padding,
                                std::vector<std::pair<std::string, std::string>> *border) = 0;

    virtual int callUpdateAttr(int &pageId, std::string &ref,
                               std::vector<std::pair<std::string, std::string>> *attrs) = 0;

    virtual int callCreateFinish(int &pageId) = 0;

    virtual int callRemoveElement(int &pageId, std::string &ref) = 0;

    virtual int callMoveElement(int &pageId, std::string &ref, std::string &parentRef,
                                int index) = 0;
  };
} //end WeexCore
#endif //Bridge_h
