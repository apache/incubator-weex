#include <WeexCore/parser/DomParser.h>
#include <WeexCore/render/manager/RenderManager.h>
#include <WeexCore/render/page/RenderPage.h>
#include <WeexCore/render/node/RenderObject.h>
#include <WeexCore/render/node/RenderRoot.h>
#include <base/TimeUtils.h>
#include <WeexCore/platform/android/bridge/impl/WXBridge_Impl_Android.h>

namespace WeexCore {

  RenderManager *RenderManager::m_pInstance = nullptr;

    jboolean isAddRenderObject = false;
    jint count = 0;

  bool RenderManager::CreatePage(const std::string &pageId, const std::string &data) {
    isAddRenderObject = false;
    count = 0;

//    std::string mMessage = "createPage";
//    Bridge_Impl_Android::getInstance()->callLogOfFirstScreen(mMessage);

    RenderPage *page = new RenderPage(pageId);
    mPages.insert(std::pair<std::string, RenderPage *>(pageId, page));

    long long startTime = getCurrentTime();
    char *c_data = (char *) data.data();
    RenderRoot *root = (RenderRoot *) Json2RenderObject(c_data, pageId);
    page->ParseJsonTime(getCurrentTime() - startTime);
    page->BuildRenderTreeTime(getCurrentTime() - startTime);
    return page->CreateRootRender(root);
  }

  bool RenderManager::AddRenderObject(const std::string &pageId, const std::string &parentRef,
                                      const int &index,
                                      const std::string &data) {

    if (!isAddRenderObject) {
      isAddRenderObject = true;
    }

//    std::string mMessage = "start addRenderObject";
//    Bridge_Impl_Android::getInstance()->callLogOfFirstScreen(mMessage);

    RenderPage *page = GetPage(pageId);
    if (page == nullptr)
      return false;

    long long startTime = getCurrentTime();
    char *c_data = (char *) data.data();
    RenderObject *child = Json2RenderObject(c_data, pageId);
    page->ParseJsonTime(getCurrentTime() - startTime);
    page->BuildRenderTreeTime(getCurrentTime() - startTime);
    if (child == nullptr)
      return false;

    return page->AddRenderObject(parentRef, index, child);
  }

  bool RenderManager::RemoveRenderObject(const std::string &pageId, const std::string &ref) {
    RenderPage *page = this->GetPage(pageId);
    if (page == nullptr)
      return false;

    return page->RemoveRenderObject(ref);
  }

  bool RenderManager::MoveRenderObject(const std::string &pageId, const std::string &ref,
                                       const std::string &parentRef,
                                       int index) {
    RenderPage *page = this->GetPage(pageId);
    if (page == nullptr)
      return false;

    return page->MoveRenderObject(ref, parentRef, index);
  }

  bool RenderManager::UpdateAttr(const std::string &pageId, const std::string &ref,
                                 const std::string &data) {
    RenderPage *page = this->GetPage(pageId);
    if (page == nullptr)
      return false;

    long long startTime = getCurrentTime();
    char *c_data = (char *) data.data();
    std::vector<std::pair<std::string, std::string> *> *attrs = Json2Pairs(c_data);
    page->ParseJsonTime(getCurrentTime() - startTime);
    page->BuildRenderTreeTime(getCurrentTime() - startTime);
    return page->UpdateAttr(ref, attrs);
  }

  bool RenderManager::UpdateStyle(const std::string &pageId, const std::string &ref,
                                  const std::string &data) {
    RenderPage *page = this->GetPage(pageId);
    if (page == nullptr)
      return false;

    long long startTime = getCurrentTime();
    char *c_data = (char *) data.data();
    std::vector<std::pair<std::string, std::string> *> *styles = Json2Pairs(c_data);
    page->ParseJsonTime(getCurrentTime() - startTime);
    page->BuildRenderTreeTime(getCurrentTime() - startTime);
    return page->UpdateStyle(ref, styles);
  }

  bool RenderManager::AddEvent(const std::string &pageId, const std::string &ref,
                               const std::string &event, const std::string &callback) {
    RenderPage *page = this->GetPage(pageId);
    if (page == nullptr)
      return false;

    return page->AddEvent(ref, event, callback);
  }

  bool RenderManager::RemoveEvent(const std::string &pageId, const std::string &ref,
                                  const std::string &event, const std::string &callback) {
    RenderPage *page = this->GetPage(pageId);
    if (page == nullptr)
      return false;

    return page->RemoveEvent(ref, event, callback);
  }

  bool RenderManager::CreateFinish(const std::string &pageId) {
    RenderPage *page = GetPage(pageId);
    if (page == nullptr)
      return false;

    return page->CreateFinish();
  }

  RenderPage *RenderManager::GetPage(const std::string &id) {
    RenderPage *page = mPages.find(id)->second;
    return page;
  }

  bool RenderManager::ClosePage(std::string pageId) {
    RenderPage *page = GetPage(pageId);
    if (page == nullptr)
      return false;

    mPages.erase(pageId);
    delete page;
  }

  void RenderManager::PrintRenderAndLayoutTree(const std::string &pageId) {
    RenderPage *page = this->GetPage(pageId);
    if (page == nullptr)
      return;
    page->GetRootRenderObject()->PrintRenderMsg();
    page->GetRootRenderObject()->PrintLayoutMsg();
  }

  void RenderManager::Batch(const std::string &pageId) {
    RenderPage *page = this->GetPage(pageId);
    if (page == nullptr)
      return;

    page->Batch();

  }

} //namespace WeexCore
