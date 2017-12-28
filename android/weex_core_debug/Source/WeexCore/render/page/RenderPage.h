#ifndef RenderPage_H
#define RenderPage_H

#include <vector>
#include <string>
#include <map>
#include <jni.h>

namespace WeexCore {

  class RenderAction;

  class RenderObject;

  class RenderPerformance;

  class RenderPage {

  private:
    const float LAYOUT_FIRSTSCREEN_OVERFLOW_RADIO = 1.2;

    float mViewPortWidth;

    RenderObject *pRoot;

    std::string mPageId;

    std::vector<RenderAction *> mRenderActions;

    std::map<std::string, RenderObject *> mRenderObjectRegisterMap;

    RenderPerformance *mWXCorePerformance;

    void pushRenderToRegisterMap(RenderObject *render);

    void removeRenderFromRegisterMap(RenderObject *render);

    void sendCreateBodyAction(RenderObject *render);

    void sendAddElementAction(RenderObject *child, RenderObject *parent, const int &index);

    void sendRemoveElementAction(const std::string &ref);

    void sendMoveElementAction(const std::string &ref, const std::string &parentRef, int index);

    void sendLayoutAction(RenderObject *render);

    void
    sendUpdateStyleAction(RenderObject *render,
                          std::vector<std::pair<std::string, std::string> *> *style,
                          std::vector<std::pair<std::string, std::string> *> *margin,
                          std::vector<std::pair<std::string, std::string> *> *padding,
                          std::vector<std::pair<std::string, std::string> *> *border);

    void sendUpdateAttrAction(RenderObject *render,
                              std::vector<std::pair<std::string, std::string> *> *attrs);

    void sendAddEventAction(RenderObject *render);

    void sendCreateFinishAction();

    // TODO / SDKInstance bind
    jobject mInstance_Impl_Android;

    void *mInstance_Impl_iOS;

  public:
    RenderPage(const std::string &pageID);

    ~RenderPage();

    void calculateLayout();

    bool createRootRender(RenderObject *root);

    bool addRenderObject(const std::string &parentRef, const int &insertPosiotn, RenderObject *child);

    bool removeRenderObject(const std::string &ref);

    bool moveRenderObject(const std::string &ref, const std::string &parentRef, int &index);

    bool updateStyle(const std::string &ref, std::vector<std::pair<std::string, std::string> *> *styles);

    bool updateAttr(const std::string &ref,
                    std::vector<std::pair<std::string, std::string> *> *attrs);

    bool addEvent(const std::string &ref, const std::string &event);

    bool removeEvent(const std::string &ref, const std::string &event);

    void postRenderAction(RenderAction *action);

    bool createFinish();

    void batch();

    void traverseTree(RenderObject *render);

    void jniCallTime(const long long &time);

    void cssLayoutTime(const long long &time);

    void addElementActionJNITime(const long long &time);

    void layoutActionJniTime(const long long &time);

    void parseJsonTime(const long long &time);

    void buildRenderObjectTime(const long long &time);

    void createJMapJNITime(const long long &time);

    void jniCallBridgeTime(const long long &time);

    bool bindInstance_Impl_Android(jobject instance);

    bool bindInstance_Impl_iOS(void *instance);

    void printFirstScreenLog();

    void printRenderSuccessLog();

    inline RenderObject *getRenderObject(const std::string &ref) {
      return mRenderObjectRegisterMap.find(ref)->second;
    }

    inline void setRootRenderObject(RenderObject *root) {
      if (root != nullptr) {
        pRoot = root;
      }
    }

    inline RenderObject *getRootRenderObject() {
      return pRoot;
    }

    inline std::string getPageId() {
      return mPageId;
    }

    inline float getViewPortWidth() {
      return mViewPortWidth;
    }

    inline void setViewPortWidth(float &viewPortWidth) {
      this->mViewPortWidth = viewPortWidth;
    }
  };
}

#endif //RenderManager_h