#ifndef BridgeAndroid_h
#define BridgeAndroid_h

#include "../../bridge/Bridge.h"
#include "WeexCore.h"

namespace WeexCore {

  class BridgeAndroid : public Bridge {

  private:
    jobject jThis;

  public:
    static BridgeAndroid *m_instance;

    //just to release singleton object
    class Garbo {
    public:
      ~Garbo() {
        if (BridgeAndroid::m_instance) {
          delete BridgeAndroid::m_instance;
        }
      }
    };

    static Garbo garbo;

    BridgeAndroid();

    ~BridgeAndroid();

    static BridgeAndroid *getInstance() {
      if (m_instance == nullptr) {
        m_instance = new BridgeAndroid();
      }
      return m_instance;
    }

    void setGlobalRef(jobject &jRef) {
      jThis = jRef;
    }

    void setJSVersion(jstring &jversion);

    void reportException(jstring &jInstanceId, jstring &jFunc, jstring &jExceptionString);

    int callNative(jstring &instanceId, jbyteArray &taskString, jstring &callback);

    jobject callNativeModule(jstring &jInstanceId, jstring &jmodule, jstring &jmethod,
                             jbyteArray &jArgString, jbyteArray &jOptString);

    void callNativeComponent(jstring &jInstanceId, jstring &jcomponentRef, jstring &jmethod,
                             jbyteArray &jArgString, jbyteArray &jOptString);

    void setTimeout(jstring &jCallbackID, jstring &jTime);

    void callNativeLog(jbyteArray &str_msg);

    int callUpdateFinish(jstring &instanceId, jbyteArray &taskString,
                         jstring &callback);

    int callRefreshFinish(jstring &instanceId, jbyteArray &taskString,
                          jstring &callback);

    int callAddEvent(jstring &instanceId,
                     jstring &ref, jstring &event, jstring &Callback);

    int callRemoveEvent(jstring &instanceId,
                        jstring &ref, jstring &event, jstring &callback);

    int callCreateBodyByWeexCore(std::string &pageId, std::string &componentType, std::string &ref,
                                 std::map<std::string, std::string> *styles,
                                 std::map<std::string, std::string> *attributes,
                                 std::set<std::string> *events,
                                 std::map<std::string, std::string> *paddings,
                                 std::map<std::string, std::string> *margins,
                                 std::map<std::string, std::string> *borders);

    int callAddElementByWeexCore(std::string &pageId, std::string &componentType, std::string &ref,
                                 int index,
                                 std::string parentRef,
                                 std::map<std::string, std::string> *styles,
                                 std::map<std::string, std::string> *attributes,
                                 std::set<std::string> *events,
                                 std::map<std::string, std::string> *paddings,
                                 std::map<std::string, std::string> *margins,
                                 std::map<std::string, std::string> *borders);

    int callRemoveElement(std::string &pageId, std::string &ref);

    int callMoveElement(std::string &pageId, std::string &ref, std::string &parentRef,
                        int index);

    int callLayoutByWeexCore(std::string &pageId, std::string &ref,
                             int top, int bottom, int left, int right,
                             int height, int width);

    int callUpdateStyleByWeexCore(std::string &pageId, std::string &ref,
                                  std::vector<std::pair<std::string, std::string> *> *style,
                                  std::vector<std::pair<std::string, std::string> *> *margin,
                                  std::vector<std::pair<std::string, std::string> *> *padding,
                                  std::vector<std::pair<std::string, std::string> *> *border);

    int callUpdateAttrByWeexCore(std::string &pageId, std::string &ref,
                                 std::vector<std::pair<std::string, std::string> *> *attrs);

    int callCreateFinishByWeexCore(std::string &pageId);
  };
} //end WeexCore
#endif //BridgeAndroid_h