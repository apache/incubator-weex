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

    void setJSVersion(jstring &jversion);

    void reportException(jstring &jInstanceId, jstring &jFunc, jstring &jExceptionString);

    int callNative(char *pageId, char *task, char *callback);

    jobject callNativeModule(char *pageId, char *module, char *method,
                             char *argString, char *optString);

    void callNativeComponent(char *pageId, char *ref, char *method,
                             char *argString, char *optString);

    void setTimeout(jstring &jCallbackID, jstring &jTime);

    void callNativeLog(jbyteArray &str_msg);

    int callUpdateFinish(jstring &instanceId, jbyteArray &taskString,
                         jstring &callback);

    int callRefreshFinish(jstring &instanceId, jbyteArray &taskString,
                          jstring &callback);

    int callAddEvent(int &pageId,
                     std::string &ref, std::string &event);

    int callRemoveEvent(int &pageId,
                        std::string &ref, std::string &event);

    int callCreateBody(int &pageId, std::string &componentType, std::string &ref,
                       std::map<std::string, std::string> *styles,
                       std::map<std::string, std::string> *attributes,
                       std::set<std::string> *events,
                       const WXCoreMargin &margins,
                       const WXCorePadding &paddings,
                       const WXCoreBorderWidth &borders);

    int callAddElement(int &pageId, std::string &componentType, std::string &ref,
                       int &index,
                       std::string &parentRef,
                       std::map<std::string, std::string> *styles,
                       std::map<std::string, std::string> *attributes,
                       std::set<std::string> *events,
                       const WXCoreMargin &margins,
                       const WXCorePadding &paddings,
                       const WXCoreBorderWidth &borders);

    int callRemoveElement(int &pageId, std::string &ref);

    int callMoveElement(int &pageId, std::string &ref, std::string &parentRef,
                        int index);

    int callLayout(int &pageId, std::string &ref,
                   int top, int bottom, int left, int right,
                   int height, int width);

    int callUpdateStyle(int &pageId, std::string &ref,
                        std::vector<std::pair<std::string, std::string>> *style,
                        std::vector<std::pair<std::string, std::string>> *margin,
                        std::vector<std::pair<std::string, std::string>> *padding,
                        std::vector<std::pair<std::string, std::string>> *border);

    int callUpdateAttr(int &pageId, std::string &ref,
                       std::vector<std::pair<std::string, std::string>> *attrs);

    int callCreateFinish(int &pageId);

    int callHasTransitionPros(int &pageId, const std::string &ref,
                      std::vector<std::pair<std::string, std::string>> *style);


  };
} //end WeexCore
#endif //BridgeAndroid_h