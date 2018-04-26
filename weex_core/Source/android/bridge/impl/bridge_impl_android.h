#ifndef BridgeAndroid_h
#define BridgeAndroid_h

#include <core/bridge/bridge.h>
#include "weexcore_impl_android.h"

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

    void setGlobalRef(jobject &jRef);

    void setJSVersion(const char* version);

    void reportException(const char* pageId, const char *func, const char *exception_string);

    int callNative(const char* pageId, const char *task, const char *callback);

    jobject callNativeModule(const char* pageId, const char *module, const char *method,
                             const char *argString, const char *optString);

    void callNativeComponent(const char* pageId, const char* ref, const char *method,
                             const char *argString, const char *optString);

    void setTimeout(const char* callbackID, const char* time);

    void callNativeLog(const char* str_array);

    int callUpdateFinish(const char* pageId, const char *task, const char *callback);

    int callRefreshFinish(const char* pageId, const char *task, const char *callback);

    int callAddEvent(const char* pageId, const char* ref, const char *event);

    int callRemoveEvent(const char* pageId, const char* ref, const char *event);

    int callCreateBody(const char* pageId, const char *componentType, const char* ref,
                       std::map<std::string, std::string> *styles,
                       std::map<std::string, std::string> *attributes,
                       std::set<std::string> *events,
                       const WXCoreMargin &margins,
                       const WXCorePadding &paddings,
                       const WXCoreBorderWidth &borders);

    int callAddElement(const char* pageId, const char *componentType,
                       const char* ref, int &index, const char* parentRef,
                       std::map<std::string, std::string> *styles,
                       std::map<std::string, std::string> *attributes,
                       std::set<std::string> *events,
                       const WXCoreMargin &margins,
                       const WXCorePadding &paddings,
                       const WXCoreBorderWidth &borders);

    int callRemoveElement(const char* pageId, const char* ref);

    int callMoveElement(const char* pageId, const char* ref, const char* parentRef, int index);

    int callLayout(const char* pageId, const char* ref,
                   int top, int bottom, int left, int right,
                   int height, int width, int index);

    int callUpdateStyle(const char* pageId, const char* ref,
                        std::vector<std::pair<std::string, std::string>> *style,
                        std::vector<std::pair<std::string, std::string>> *margin,
                        std::vector<std::pair<std::string, std::string>> *padding,
                        std::vector<std::pair<std::string, std::string>> *border);

    int callUpdateAttr(const char* pageId, const char* ref,
                       std::vector<std::pair<std::string, std::string>> *attrs);

    int callCreateFinish(const char* pageId);

    int callHasTransitionPros(const char* pageId, const char* ref,
                              std::vector<std::pair<std::string, std::string>> *style);
  };
} //end WeexCore
#endif //BridgeAndroid_h