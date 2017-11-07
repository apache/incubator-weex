#ifndef BridgeAndroid_h
#define BridgeAndroid_h

#include "../../bridge/Bridge.h"
#include "WeexCore.h"
#include <string>
#include <map>
#include <set>

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

    int callAddElement(jstring &jInstanceId, jstring &jref, jbyteArray &jdomString,
                       jstring &jindex, jstring &jCallback);

    void setTimeout(jstring &jCallbackID, jstring &jTime);

    void callNativeLog(jstring &str_msg);

    int callCreateBody(jstring &instanceId, jbyteArray &taskString, jstring &callback);

    int callUpdateFinish(jstring &instanceId, jbyteArray &taskString,
                         jstring &callback);

    int callCreateFinish(jstring &instanceId, jbyteArray &taskString,
                         jstring &callback);

    int callRefreshFinish(jstring &instanceId, jbyteArray &taskString,
                          jstring &callback);

    int callUpdateAttrs(jstring &instanceId, jstring &ref,
                        jbyteArray &taskString, jstring &callback);

    int callUpdateStyle(jstring &instanceId, jstring &ref,
                        jbyteArray &taskString, jstring &callback);

    int callRemoveElement(jstring &instanceId, jstring &ref,
                          jstring &callback);

    int callMoveElement(jstring &instanceId, jstring &ref, jstring &parentref,
                        jstring &index, jstring &callback);

    int callAddEvent(jstring &instanceId,
                     jstring &ref, jstring &event, jstring &Callback);

    int callRemoveEvent(jstring &instanceId,
                        jstring &ref, jstring &event, jstring &callback);

    int callCreateBodyByWeexCore(std::string &pageId, std::string &componentType, std::string &ref,
                                 int top,
                                 int bottom, int left, int right, int height, int width,
                                 std::map<std::string, std::string> *styles,
                                 std::map<std::string, std::string> *attributes,
                                 std::set<std::string> *events,
                                 std::map<std::string, std::string> *paddings,
                                 std::map<std::string, std::string> *margins,
                                 std::map<std::string, std::string> *borders);

    int callAddElementByWeexCore(std::string &pageId, std::string &componentType, std::string &ref,
                                 int top,
                                 int bottom, int left, int right, int height, int width, int index,
                                 std::string parentRef,
                                 std::map<std::string, std::string> *styles,
                                 std::map<std::string, std::string> *attributes,
                                 std::set<std::string> *events,
                                 std::map<std::string, std::string> *paddings,
                                 std::map<std::string, std::string> *margins,
                                 std::map<std::string, std::string> *borders);

    int callUpdateStyleByWeexCore(std::string &pageId, std::string &ref, std::string &key,
                                  std::string &value);

    int callReLayoutByWeexCore(std::string &pageId, std::string &ref,
                               int top, int bottom, int left, int right,
                               int height, int width);

  };
} //end WeexCore
#endif //BridgeAndroid_h