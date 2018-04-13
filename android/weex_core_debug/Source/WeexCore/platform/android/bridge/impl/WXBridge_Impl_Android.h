#ifndef BridgeAndroid_h
#define BridgeAndroid_h

#include <WeexCore/bridge/Bridge.h>
#include "WeexCore_Impl_Android.h"

namespace WeexCore {

  class Bridge_Impl_Android : public Bridge {

  private:
    jobject jThis;

  public:
    static Bridge_Impl_Android *m_instance;

    //just to release singleton object
    class Garbo {
    public:
      ~Garbo() {
        if (Bridge_Impl_Android::m_instance) {
          delete Bridge_Impl_Android::m_instance;
        }
      }
    };

    static Garbo garbo;

    Bridge_Impl_Android();

    ~Bridge_Impl_Android();

    static Bridge_Impl_Android *getInstance() {
      if (m_instance == nullptr) {
        m_instance = new Bridge_Impl_Android();
      }
      return m_instance;
    }

    void setGlobalRef(jobject &jRef) {
      jThis = jRef;
    }

    void setJSVersion(const char* version);

    void reportException(const char *pageId, const char *func, const char *exception_string);

    int callNative(const char *pageId, const char *task, const char *callback);

    jobject callNativeModule(const char *pageId, const char *module, const char *method,
                             const char *argString, const char *optString);

    void callNativeComponent(const char *pageId, const char *ref, const char *method,
                             const char *argString, const char *optString);

    void setTimeout(const char* callbackID, const char* time);

    void callNativeLog(const char* str_array);

    int callUpdateFinish(const char *pageId, const char *task, const char *callback);

    int callRefreshFinish(const char *pageId, const char *task, const char *callback);

    int callAddEvent(int &pageId, const char *ref, const char *event);

    int callRemoveEvent(int &pageId, const char *ref, const char *event);

    int callCreateBody(int &pageId, const char *componentType, const char *ref,
                       std::map<std::string, std::string> *styles,
                       std::map<std::string, std::string> *attributes,
                       std::set<std::string> *events,
                       const WXCoreMargin &margins,
                       const WXCorePadding &paddings,
                       const WXCoreBorderWidth &borders);

    int callAddElement(int &pageId, const char *componentType,
                       const char *ref, int &index, const char *parentRef,
                       std::map<std::string, std::string> *styles,
                       std::map<std::string, std::string> *attributes,
                       std::set<std::string> *events,
                       const WXCoreMargin &margins,
                       const WXCorePadding &paddings,
                       const WXCoreBorderWidth &borders);

    int callRemoveElement(int &pageId, const char *ref);

    int callMoveElement(int &pageId, const char *ref, const char *parentRef, int index);

    int callLayout(int &pageId, const char *ref,
                   int top, int bottom, int left, int right,
                   int height, int width);

    int callUpdateStyle(int &pageId, const char *ref,
                        std::vector<std::pair<std::string, std::string>> *style,
                        std::vector<std::pair<std::string, std::string>> *margin,
                        std::vector<std::pair<std::string, std::string>> *padding,
                        std::vector<std::pair<std::string, std::string>> *border);

    int callUpdateAttr(int &pageId, const char *ref,
                       std::vector<std::pair<std::string, std::string>> *attrs);

    int callCreateFinish(int &pageId);

    int callHasTransitionPros(int &pageId, const std::string &ref,
                              std::vector<std::pair<std::string, std::string>> *style);
  };
} //end WeexCore
#endif //BridgeAndroid_h