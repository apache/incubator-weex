#ifndef RenderPage_H
#define RenderPage_H

#include "RenderPage.h"
#include "base/android/LogUtils.h"
#include "rapidjson/weexjsontools.h"
#include "base/android/string/StringUtils.h"
#include "action/AddElementAction.h"
#include "action/CreateBodyAction.h"
#include "action/UpdateStyleAction.h"
#include "action/RenderAction.h"
#include "RenderManager.h"
#include "RenderObject.h"
#include <vector>
#include <string>
#include <map>
#include <Layout/WXCoreLayout.h>
#include <Layout/WXCoreStyle.h>
#include "action/RelayoutRenderAction.h"

namespace WeexCore {
  class RenderObject;

  class RenderAction;

  typedef std::vector<RenderAction *>::iterator RENDERACTION_IT;
  typedef std::map<std::string, RenderObject *>::iterator RENDEROBJECT_COLLECTION_IT;

  class RenderPage {

  private:
    RenderObject *pRoot;

    std::string mPageId;

    std::vector<RenderAction *> mRenderActions;

    std::map<std::string, RenderObject *> mRenderObjectMap;

    void calculateLayout();

  public:
    RenderPage(std::string pageID, std::string data);

    ~RenderPage();

    void addRenderObject(std::string parentRef, int insertPosiotn, std::string data);

    void removeRenderObject(std::string ref);

    void moveRenderObject(std::string ref, std::string parentRef, std::string index);

    void updateStyle(std::string ref, std::string key, std::string value);

    void updateAttr(std::string ref, std::string key, std::string value);

    void addEvent(std::string ref, std::string event);

    void removeEvent(std::string ref, std::string event);

    void addRenderAction(RenderAction *action);

    void batch();

    RenderObject *getRenderObject(std::string ref);

    void setRootRenderObject(RenderObject *root);

    RenderObject *getRootRenderObject();

    void traverseTree(RenderObject *render);

    std::string getPageId() {
      return mPageId;
    }
  };
}

#endif //RenderManager_h