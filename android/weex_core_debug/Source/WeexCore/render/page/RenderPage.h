#ifndef RenderPage_H
#define RenderPage_H

#include <vector>
#include <string>
#include <map>
#include <jni.h>
#include <cmath>

namespace WeexCore {

  constexpr float kLayoutFirstScreenOverflowRadio = 1.2f;

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

    void SendRemoveElementAction(const int &ref);

    void SendMoveElementAction(const int &ref, const int &parentRef, int index);

    void SendLayoutAction(RenderObject *render);

    void
    SendUpdateStyleAction(RenderObject *render,
                          std::vector<std::pair<std::string, std::string>> *style,
                          std::vector<std::pair<std::string, std::string>> *margin,
                          std::vector<std::pair<std::string, std::string>> *padding,
                          std::vector<std::pair<std::string, std::string>> *border);

    void SendUpdateAttrAction(RenderObject *render, std::vector<std::pair<std::string, std::string>> *attrs);

    void SendCreateFinishAction();

    void PostRenderAction(RenderAction *action);

  public:
    static constexpr bool useVSync = true;
    std::atomic_bool needLayout{false};
    std::atomic_bool hasForeLayoutAction{false};
    RenderPage(int pageId);

    ~RenderPage();

    void CalculateLayout();

    bool CreateRootRender(RenderObject *root);

    bool AddRenderObject(const int &parentRef, int insertPosiotn, RenderObject *child);

    bool RemoveRenderObject(const int &ref);

    bool MoveRenderObject(const int &ref, const int &parentRef, int index);

    bool UpdateStyle(const int &ref, std::vector<std::pair<std::string, std::string>> *styles);

    bool UpdateAttr(const int &ref, std::vector<std::pair<std::string, std::string>> *attrs);

    void SetDefaultHeightAndWidthIntoRootRender(const float defaultWidth, const float defaultHeight, const bool isWidthWrapContent, const bool isHeightWrapContent);

    bool AddEvent(const int &ref, const std::string &event);

    bool RemoveEvent(const int &ref, const std::string &event);

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

    void LayoutImmediately();

    inline RenderObject *GetRenderObject(const int &ref) {
        std::map<int, RenderObject *>::iterator iter = mRenderObjectRegisterMap.find(ref);
        if (iter != mRenderObjectRegisterMap.end()) {
            return iter->second;
        } else {
            return nullptr;
        }
    }

    void SetRootRenderObject(RenderObject *root);

    inline RenderObject * GetRootRenderObject() const {
      return render_root;
    }

    inline int PageId() {
      return mPageId;
    }

    inline float ViewPortWidth() const {
      return mViewPortWidth;
    }

    inline void SetViewPortWidth(float viewPortWidth) {
      this->mViewPortWidth = viewPortWidth;
    }

    inline bool isDirty(){
      return dirty.load();
    }

    inline void updateDirty(bool dirty){
      this->dirty.store(dirty);
    }

    inline void SetRenderContainerWidthWrapContent(bool wrap) {
      this->isRenderContainerWidthWrapContent.store(wrap);
    }

    inline bool GetRenderContainerWidthWrapContent() {
      return isRenderContainerWidthWrapContent.load();
    }

    // ****** Life Cycle ****** //

    void OnRenderPageInit();

    void OnRenderProcessStart();

    void OnRenderProcessExited();

    void OnRenderProcessGone();

    void OnRenderPageClose();

  private:
    bool mAlreadyCreateFinish = false;
    float mViewPortWidth;
    RenderObject *render_root = nullptr;
    int mPageId;
    std::pair<float,float> renderPageSize;
    std::map<int, RenderObject *> mRenderObjectRegisterMap;
    RenderPerformance *mWXCorePerformance;
    std::atomic_bool dirty{true};
    std::atomic_bool isRenderContainerWidthWrapContent{false};
    std::atomic_bool isRenderContainerHeightWrapContent{false};
  };
}

#endif //RenderManager_h