#ifndef RenderPage_H
#define RenderPage_H

#include <vector>
#include <string>
#include <map>
#include <jni.h>
#include <cmath>

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

    void SendAddElementAction(RenderObject *child, RenderObject *parent, int index);

    void SendRemoveElementAction(std::string ref);

    void SendMoveElementAction(std::string ref, std::string parentRef, int index);

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
    RenderPage(std::string pageId);

    ~RenderPage();

    void CalculateLayout();

    bool CreateRootRender(RenderObject *root);

    bool
    AddRenderObject(std::string parentRef, int insertPosiotn, RenderObject *child);

    bool RemoveRenderObject(std::string ref);

    bool MoveRenderObject(std::string ref, std::string parentRef, int index);

    bool
    UpdateStyle(std::string ref, std::vector<std::pair<std::string, std::string> *> *styles);

    bool UpdateAttr(std::string ref,
                    std::vector<std::pair<std::string, std::string> *> *attrs);

    void
    SetDefaultHeightAndWidthIntoRootRender(const float defaultWidth, const float defaultHeight);

    bool AddEvent(std::string ref, std::string event);

    bool RemoveEvent(std::string ref, std::string event);

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

    int PrintFirstScreenLog();

    int PrintRenderSuccessLog();

    inline RenderObject *GetRenderObject(const std::string &ref) {
        std::map<std::string, RenderObject *>::iterator iter = mRenderObjectRegisterMap.find(ref);
        if (iter != mRenderObjectRegisterMap.end()) {
            return iter->second;
        } else {
            return nullptr;
        }
    }

    inline void SetRootRenderObject(RenderObject *root) {
      if (root != nullptr) {
        render_root = root;
      }
    }

    inline RenderObject * GetRootRenderObject() const {
      return render_root;
    }

    inline std::string PageId() {
      return mPageId;
    }

    inline float ViewPortWidth() const {
      return mViewPortWidth;
    }

    inline void SetViewPortWidth(float viewPortWidth) {
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
    bool mAlreadyCreateFinish = false;
    float mViewPortWidth;
    RenderObject *render_root = nullptr;
    std::string mPageId;
    std::pair<float,float> renderPageSize;
    std::map<std::string, RenderObject *> mRenderObjectRegisterMap;
    RenderPerformance *mWXCorePerformance;
  };
}

#endif //RenderManager_h