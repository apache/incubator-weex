#ifndef RenderPage_H
#define RenderPage_H

#include <vector>
#include <string>
#include <map>
#include <jni.h>

namespace WeexCore {

  class RenderAction;

  class RenderObject;

  class WXCorePerformance;

  class RenderPage {

  private:
    const float LAYOUT_FIRSTSCREEN_OVERFLOW_RADIO = 1.2;

    float mViewPortWidth;

    RenderObject *pRoot;

    std::string mPageId;

    std::vector<RenderAction *> mRenderActions;

    std::map<std::string, RenderObject *> mRenderObjectMap;

    WXCorePerformance *mWXCorePerformance;

    void pushRenderToMap(RenderObject *render);

    void sendCreateBodyAction(RenderObject *render);

    void sendAddElementAction(RenderObject *child, RenderObject *parent, int index);

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

    // TODO 和 SDKInstance 生命周期绑定
    jobject mInstance_Impl_Android;

    void *mInstance_Impl_iOS;

  public:
    RenderPage(std::string pageID);

    ~RenderPage();

    void calculateLayout();

    bool createRootRender(RenderObject *root);

    bool addRenderObject(std::string parentRef, int insertPosiotn, RenderObject *child);

    bool removeRenderObject(std::string ref);

    bool moveRenderObject(std::string ref, std::string parentRef, int index);

    bool updateStyle(std::string ref, std::vector<std::pair<std::string, std::string> *> *styles);

    bool updateAttr(std::string ref,
                    std::vector<std::pair<std::string, std::string> *> *attrs);

    bool addEvent(std::string ref, std::string event);

    bool removeEvent(std::string ref, std::string event);

    void addRenderAction(RenderAction *action);

    bool createFinish();

    void batch();

    void traverseTree(RenderObject *render);

    void jniCallTime(long long time);

    void cssLayoutTime(long long time);

    void addElementActionJNITime(long long time);

    void layoutActionJniTime(long long time);

    void parseJsonTime(long long time);

    void buildRenderObjectTime(long long time);

    bool bindInstance_Impl_Android(jobject instance);

    bool bindInstance_Impl_iOS(void *instance);

    void printFirstScreenLog();

    void printRenderSuccessLog();

    inline RenderObject *getRenderObject(std::string ref) {
      return mRenderObjectMap.find(ref)->second;
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

    inline void setViewPortWidth(float viewPortWidth) {
      this->mViewPortWidth = viewPortWidth;
    }
  };
}

#endif //RenderManager_h