#ifndef RenderPage_H
#define RenderPage_H

#include <vector>
#include <string>
#include <map>
#include <jni.h>
#include <cmath>
#include <WeexCore/render/node/RenderRoot.h>

namespace WeexCore {

  class RenderAction;

  class RenderObject;

  class RenderPerformance;

  class RenderPage {

  private:

    void TraverseTree(RenderObject *render);

    void PushRenderToRegisterMap(RenderObject *render);

    void RemoveRenderFromRegisterMap(RenderObject *render);

    void SendCreateBodyAction(RenderObject *render);

    void SendAddElementAction(RenderObject *child, RenderObject *parent, const int &index);

    void SendRemoveElementAction(const std::string &ref);

    void SendMoveElementAction(const std::string &ref, const std::string &parentRef, int index);

    void SendLayoutAction(RenderObject *render);

    void
    SendUpdateStyleAction(RenderObject *render,
                          std::vector<std::pair<std::string, std::string> *> *style,
                          std::vector<std::pair<std::string, std::string> *> *margin,
                          std::vector<std::pair<std::string, std::string> *> *padding,
                          std::vector<std::pair<std::string, std::string> *> *border);

    void SendUpdateAttrAction(RenderObject *render,
                              std::vector<std::pair<std::string, std::string> *> *attrs);

    void SendCreateFinishAction();

    void PostRenderAction(RenderAction *action);

  public:
    RenderPage(const std::string &pageID);

    ~RenderPage();

    void CalculateLayout();

    bool CreateRootRender(RenderRoot *root);

    bool
    AddRenderObject(const std::string &parentRef, const int &insertPosiotn, RenderObject *child);

    bool RemoveRenderObject(const std::string &ref);

    bool MoveRenderObject(const std::string &ref, const std::string &parentRef, int &index);

    bool
    UpdateStyle(const std::string &ref, std::vector<std::pair<std::string, std::string> *> *styles);

    bool UpdateAttr(const std::string &ref,
                    std::vector<std::pair<std::string, std::string> *> *attrs);

    void
    SetDefaultHeightAndWidthIntoRootRender(const float defaultWidth, const float defaultHeight);

    bool AddEvent(const std::string &ref, const std::string &event);

    bool RemoveEvent(const std::string &ref, const std::string &event);

    bool CreateFinish();

    void Batch();

    void JniCallTime(const long long &time);

    void CssLayoutTime(const long long &time);

    void AddEventActionJNITime(const long long &time);

    void RemoveEventActionJNITime(const long long &time);

    void AddElementActionJNITime(const long long &time);

    void LayoutActionJniTime(const long long &time);

    void ParseJsonTime(const long long &time);

    void BuildRenderTreeTime(const long long &time);

    void CreateJMapJNITime(const long long &time);

    void CallBridgeTime(const long long &time);

    bool BindInstanceImplAndroid(jobject instance);

    bool BindInstanceImplIOS(void *instance);

    int PrintFirstScreenLog();

    int PrintRenderSuccessLog();

    inline RenderObject *GetRenderObject(const std::string &ref) {
      return mRenderObjectRegisterMap.find(ref)->second;
    }

    inline void SetRootRenderObject(RenderRoot *root) {
      if (root != nullptr) {
        render_root = root;
      }
    }

    inline RenderRoot * GetRootRenderObject() const {
      return render_root;
    }

    inline std::string PageId() {
      return mPageId;
    }

    inline float ViewPortWidth() const {
      return mViewPortWidth;
    }

    inline void SetViewPortWidth(float &viewPortWidth) {
      this->mViewPortWidth = viewPortWidth;
    }

    // ****** Life Cycle ****** //

    void OnRenderPageInit();

    void OnRenderProcessStart();

    void OnRenderProcessExited();

    void OnRenderProcessGone();

    void OnRenderPageClose();

  private:
    const float kLayoutFirstScreenOverflowRadio = 1.2;
    float mViewPortWidth;
    RenderRoot *render_root;
    std::string mPageId;
    std::vector<RenderAction *> mRenderActions;
    std::map<std::string, RenderObject *> mRenderObjectRegisterMap;
    RenderPerformance *mWXCorePerformance;
    jobject mInstance_Impl_Android; // TODO / SDKInstance bind
    void *mInstance_Impl_iOS;
  };
}

#endif //RenderManager_h