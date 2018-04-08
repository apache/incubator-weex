#include <WeexCore/parser/DomParser.h>
#include <WeexCore/render/manager/RenderManager.h>
#include <WeexCore/render/page/RenderPage.h>
#include <WeexCore/render/node/RenderObject.h>
#include <base/TimeUtils.h>

namespace WeexCore {

  RenderManager *RenderManager::m_pInstance = nullptr;

  bool RenderManager::CreatePage(int pageId, const std::string &data) {

#if RENDER_LOG
    LOGD("[RenderManager] CreatePage >>>> pageId: %d, dom data: %s", pageId, data.c_str());
#endif

    RenderPage *page = new RenderPage(pageId);
    mPages.insert(std::pair<int, RenderPage *>(pageId, page));
    long long startTime = getCurrentTime();
    char *c_data = (char *) data.data();
    RenderObject *root = Json2RenderObject(c_data, pageId);
    page->updateDirty(true);
    page->ParseJsonTime(getCurrentTime() - startTime);
    page->BuildRenderTreeTime(getCurrentTime() - startTime);
    return page->CreateRootRender(root);
  }

  bool RenderManager::AddRenderObject(const int &pageId, const std::string &parentRef,
                                      int index,
                                      const std::string &data) {

    RenderPage *page = GetPage(pageId);
    if (page == nullptr)
      return false;

#if RENDER_LOG
    LOGD("[RenderManager] AddRenderObject >>>> pageId: %d, parentRef: %s, index: %d, dom data: %s",
         pageId, parentRef.c_str(), index, data.c_str());
#endif

    long long startTime = getCurrentTime();
    char *c_data = (char *) data.data();
    RenderObject *child = Json2RenderObject(c_data, pageId);
    page->ParseJsonTime(getCurrentTime() - startTime);
    page->BuildRenderTreeTime(getCurrentTime() - startTime);
    if (child == nullptr)
      return false;

    page->updateDirty(true);
    return page->AddRenderObject(parentRef, index, child);
  }

  bool RenderManager::RemoveRenderObject(const int &pageId, const std::string &ref) {

    RenderPage *page = this->GetPage(pageId);
    if (page == nullptr)
      return false;

#if RENDER_LOG
    LOGD("[RenderManager] RemoveRenderObject >>>> pageId: %d, ref: %s", pageId,
         ref.c_str());
#endif

    page->updateDirty(true);
    return page->RemoveRenderObject(ref);
  }

  bool RenderManager::MoveRenderObject(const int &pageId, const std::string &ref,
                                       const std::string &parentRef,
                                       int index) {
    RenderPage *page = this->GetPage(pageId);
    if (page == nullptr)
      return false;

#if RENDER_LOG
    LOGD("[RenderManager] MoveRenderObject >>>> pageId: %d, ref: %s, parentRef: %s, index: %d",
         pageId, ref.c_str(), parentRef.c_str(), index);
#endif

    page->updateDirty(true);
    return page->MoveRenderObject(ref, parentRef, index);
  }

  bool RenderManager::UpdateAttr(const int &pageId, const std::string &ref,
                                 const std::string &data) {
    RenderPage *page = this->GetPage(pageId);
    if (page == nullptr)
      return false;

#if RENDER_LOG
    LOGD("[RenderManager] UpdateAttr >>>> pageId: %d, ref: %s, data: %s",
         pageId, ref.c_str(), data.c_str());
#endif

    long long startTime = getCurrentTime();
    char *c_data = (char *) data.data();
    std::vector<std::pair<std::string, std::string>> *attrs = Json2Pairs(c_data);
    page->updateDirty(true);
    page->ParseJsonTime(getCurrentTime() - startTime);
    page->BuildRenderTreeTime(getCurrentTime() - startTime);
    return page->UpdateAttr(ref, attrs);
  }

  bool RenderManager::UpdateStyle(const int &pageId, const std::string &ref,
                                  const std::string &data) {
    RenderPage *page = this->GetPage(pageId);
    if (page == nullptr)
      return false;

#if RENDER_LOG
    LOGD("[RenderManager] UpdateStyle >>>> pageId: %d, ref: %s, data: %s",
         pageId, ref.c_str(), data.c_str());
#endif

    long long startTime = getCurrentTime();
    char *c_data = (char *) data.data();
    std::vector<std::pair<std::string, std::string>> *styles = Json2Pairs(c_data);
    page->updateDirty(true);
    page->ParseJsonTime(getCurrentTime() - startTime);
    page->BuildRenderTreeTime(getCurrentTime() - startTime);
    return page->UpdateStyle(ref, styles);
  }

  bool RenderManager::AddEvent(const int &pageId, const std::string &ref,
                               const std::string &event) {
    RenderPage *page = this->GetPage(pageId);
    if (page == nullptr)
      return false;

#if RENDER_LOG
    LOGD("[RenderManager] AddEvent >>>> pageId: %d, ref: %s, event: %s",
         pageId, ref.c_str(), event.c_str());
#endif

    page->updateDirty(true);
    return page->AddEvent(ref, event);
  }

  bool RenderManager::RemoveEvent(const int &pageId, const std::string &ref,
                                  const std::string &event) {
    RenderPage *page = this->GetPage(pageId);
    if (page == nullptr)
      return false;

#if RENDER_LOG
    LOGD("[RenderManager] RemoveEvent >>>> pageId: %d, ref: %s, event: %s",
         pageId, ref.c_str(), event.c_str());
#endif

    page->updateDirty(true);
    return page->RemoveEvent(ref, event);
  }

  bool RenderManager::CreateFinish(const int &pageId) {
    RenderPage *page = GetPage(pageId);
    if (page == nullptr)
      return false;

#if RENDER_LOG
    LOGD("[RenderManager] CreateFinish >>>> pageId: %d", pageId);
#endif

    page->updateDirty(true);
    return page->CreateFinish();
  }

  RenderPage *RenderManager::GetPage(const int &id) {
    std::map<int, RenderPage *>::iterator iter = mPages.find(id);
    if (iter != mPages.end()) {
      return iter->second;
    } else {
      return nullptr;
    }
  }

  bool RenderManager::ClosePage(const int &pageId) {
    RenderPage *page = GetPage(pageId);
    if (page == nullptr)
      return false;

#if RENDER_LOG
    LOGD("[RenderManager] ClosePage >>>> pageId: %d", pageId);
#endif

    mPages.erase(pageId);
    delete page;
    page = nullptr;
  }

  void RenderManager::Batch(const int &pageId) {
    RenderPage *page = this->GetPage(pageId);
    if (page == nullptr)
      return;

    page->Batch();
  }

} //namespace WeexCore
