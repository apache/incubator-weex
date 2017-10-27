#ifndef RenderPage_H
#define RenderPage_H

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

  typedef std::vector<RenderAction *>::iterator RENDERACTION_IT;
  typedef std::map<std::string, RenderObject *>::iterator RENDEROBJECT_COLLECTION_IT;

  class RenderPage {

  private:
    RenderObject *pRoot;

    std::string mPageId;

    std::vector<RenderAction *> mRenderActions;

    std::map<std::string, RenderObject *> mRenderObjectMap;

    void calculateLayout();

    inline void sendCreateBodyAction(RenderObject *render) {
      RenderAction *action = new CreateBodyAction();
      action->GenerateAction(getPageId(), render, nullptr, nullptr, nullptr);
      addRenderAction(action);
    }

    inline void sendAddElementAction(RenderObject *child, RenderObject *parent, int index) {
      RenderAction *action = new AddElementAction();
      action->GenerateAction(getPageId(), child, parent, nullptr, nullptr, index);
      addRenderAction(action);
    }

    inline void sendReLayoutAction(RenderObject *render) {
      RenderAction *action = new RelayoutRenderAction();
      action->GenerateAction(getPageId(), render, nullptr, nullptr, nullptr);
      addRenderAction(action);
    }

    inline void sendUpdateStyleAction(RenderObject *render) {
      STYLE_IT style_it_star = render->getStyleItBegin();
      STYLE_IT style_it_end = render->getStyleItEnd();

      for (; style_it_star != style_it_end; ++style_it_star) {
        RenderAction *action = new UpdateStyleAction();
        action->GenerateAction(getPageId(), render, nullptr,
                               style_it_star->first,
                               style_it_star->second);
        addRenderAction(action);
      }
    }

    inline void sendUpdateAttrAction(RenderObject *render) {
      ATTR_IT attr_it_star = render->getAttrItBegin();
      ATTR_IT attr_it_end = render->getAttrItEnd();

      for (; attr_it_star != attr_it_end; ++attr_it_star) {
        RenderAction *action = new UpdateStyleAction();
        action->GenerateAction(getPageId(), render, nullptr,
                               attr_it_star->first,
                               attr_it_star->second);
        addRenderAction(action);
      }
    }

    inline void sendAddEventAction(RenderObject *render) {
      EVENT_IT event_it_start = render->getEventItBegin();
      EVENT_IT event_it_end = render->getEventItEnd();

      for (; event_it_start != event_it_end; ++event_it_start) {
        // TODO AddEventAction
      }
    }

  public:
    RenderPage(std::string pageID, std::string data);

    ~RenderPage();

    void addRenderObject(std::string parentRef, int insertPosiotn, RenderObject *child);

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

    inline std::string getPageId() {
      return mPageId;
    }
  };
}

#endif //RenderManager_h