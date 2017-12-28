#ifndef Bridge_h
#define Bridge_h

#include <jni.h>
#include <string>
#include <map>
#include <set>
#include <vector>

namespace WeexCore {

  class Bridge {
  public:
    virtual void setJSVersion(jstring &jversion) = 0;

    virtual void
    reportException(jstring &jInstanceId, jstring &jFunc, jstring &jExceptionString) = 0;

    virtual int callNative(jstring &instanceId, jbyteArray &taskString, jstring &callback) = 0;

    virtual jobject callNativeModule(jstring &jInstanceId, jstring &jmodule, jstring &jmethod,
                                     jbyteArray &jArgString, jbyteArray &jOptString) = 0;

    virtual void callNativeComponent(jstring &jInstanceId, jstring &jcomponentRef, jstring &jmethod,
                                     jbyteArray &jArgString, jbyteArray &jOptString) = 0;

    virtual void setTimeout(jstring &jCallbackID, jstring &jTime) = 0;

    virtual void callNativeLog(jbyteArray &str_msg) = 0;


    virtual int callUpdateFinish(jstring &instanceId, jbyteArray &taskString,
                                 jstring &callback) = 0;

    virtual int callRefreshFinish(jstring &instanceId, jbyteArray &taskString,
                                  jstring &callback) = 0;

    virtual int callAddEvent(jstring &instanceId,
                             jstring &ref, jstring &event, jstring &Callback) = 0;

    virtual int callRemoveEvent(jstring &instanceId,
                                jstring &ref, jstring &event, jstring &callback) = 0;

    virtual int
    callCreateBodyByWeexCore(std::string &pageId, std::string &componentType, std::string &ref,
                             std::map<std::string, std::string> *styles,
                             std::map<std::string, std::string> *attributes,
                             std::set<std::string> *events,
                             std::map<std::string, std::string> *paddings,
                             std::map<std::string, std::string> *margins,
                             std::map<std::string, std::string> *borders) = 0;

    virtual int
    callAddElementByWeexCore(std::string &pageId, std::string &componentType, std::string &ref,
                             int &index,
                             std::string &parentRef,
                             std::map<std::string, std::string> *styles,
                             std::map<std::string, std::string> *attributes,
                             std::set<std::string> *events,
                             std::map<std::string, std::string> *paddings,
                             std::map<std::string, std::string> *margins,
                             std::map<std::string, std::string> *borders) = 0;

    virtual int callLayoutByWeexCore(std::string &pageId, std::string &ref,
                                     int top, int bottom, int left, int right,
                                     int height, int width) = 0;

    virtual int callUpdateStyleByWeexCore(std::string &pageId, std::string &ref,
                                          std::vector<std::pair<std::string, std::string> *> *style,
                                          std::vector<std::pair<std::string, std::string> *> *margin,
                                          std::vector<std::pair<std::string, std::string> *> *padding,
                                          std::vector<std::pair<std::string, std::string> *> *border) = 0;

    virtual int callUpdateAttrByWeexCore(std::string &pageId, std::string &ref,
                                         std::vector<std::pair<std::string, std::string> *> *attrs) = 0;

    virtual int callCreateFinishByWeexCore(std::string &pageId) = 0;

    virtual int callRemoveElement(std::string &pageId, std::string &ref) = 0;

    virtual int callMoveElement(std::string &pageId, std::string &ref, std::string &parentRef,
                                int index) = 0;
  };
} //end WeexCore
#endif //Bridge_h
