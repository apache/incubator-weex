#ifndef RenderPage_H
#define RenderPage_H

#include <rapidjson/weexjsontools.h>
#include <base/android/string/StringUtils.h>
#include "action/AddElementAction.h"
#include "action/CreateBodyAction.h"
#include "action/UpdateStyleAction.h"
#include "action/RenderAction.h"
#include "WeexCore/render/action/LayoutRenderAction.h"
#include "RenderManager.h"
#include "RenderObject.h"
#include <vector>
#include <string>
#include <map>
#include <Layout/WXCoreLayout.h>
#include <Layout/WXCoreStyle.h>

namespace WeexCore {

  typedef std::vector<RenderAction *>::iterator RENDERACTION_IT;
  typedef std::map<std::string, RenderObject *>::iterator RENDEROBJECT_COLLECTION_IT;

  class RenderPage {

  private:
    RenderObject *pRoot;

    std::string mPageId;

    std::vector<RenderAction *> mRenderActions;

    std::map<std::string, RenderObject *> mRenderObjectMap;

    void calculateLayout();

    inline void pushRenderToMap(RenderObject *render) {
      mRenderObjectMap.insert(pair<std::string, RenderObject *>(render->getRef(), render));

      for (int i = 0; i < render->getChildCount(); ++i) {
        pushRenderToMap(render->getChild(i));
      }
    }

    inline void sendCreateBodyAction(RenderObject *render) {
      CreateBodyAction *action = new CreateBodyAction();
      action->GenerateAction(getPageId(), render);
      addRenderAction(action);

      for (int i = 0; i < render->getChildCount(); ++i) {
        sendAddElementAction(render->getChild(i), render, i);
      }
    }

    inline void sendAddElementAction(RenderObject *child, RenderObject *parent, int index) {
      AddElementAction *action = new AddElementAction();
      action->GenerateAction(getPageId(), child, parent, index);
      addRenderAction(action);

      for (int i = 0; i < child->getChildCount(); ++i) {
        sendAddElementAction(child->getChild(i), child, i);
      }
    }

    inline void sendLayoutAction(RenderObject *render) {
      LayoutRenderAction *action = new LayoutRenderAction();
      action->GenerateAction(getPageId(), render);
      addRenderAction(action);
    }

    inline void sendUpdateStyleAction(std::string key, std::string value, std::string ref) {
      UpdateStyleAction *action = new UpdateStyleAction();
      action->GenerateAction(getPageId(), ref, key, value);
      addRenderAction(action);
    }

    inline void sendUpdateAttrAction(std::string key, std::string value, std::string ref) {
      UpdateStyleAction *action = new UpdateStyleAction();
      action->GenerateAction(getPageId(), ref, key, value);
      addRenderAction(action);
    }

    inline void sendAddEventAction(RenderObject *render) {
      // TODO AddEventAction
    }

  public:
    RenderPage(std::string pageID);

    ~RenderPage();

    void createRootRender(std::string data);

    void addRenderObject(std::string parentRef, int insertPosiotn, RenderObject *child);

    void removeRenderObject(std::string ref);

    void moveRenderObject(std::string ref, std::string parentRef, std::string index);

    void updateStyle(std::string ref, std::string key, std::string value);

    void updateAttr(std::string ref, std::string key, std::string value);

    void addEvent(std::string ref, std::string event);

    void removeEvent(std::string ref, std::string event);

    void addRenderAction(RenderAction *action);

    void batch();

    void traverseTree(RenderObject *render);

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
  };
}

#endif //RenderManager_h