#ifndef RenderPage_H
#define RenderPage_H

#include "RenderPage.h"
#include "./base/LogUtils.h"
#include "./rapidjson/weexjsontools.h"
#include "./base/StringUtils.h"
#include "action/RenderAction.h"
#include "RenderManager.h"
#include "RenderObject.h"
#include <vector>
#include <string>
#include <map>

namespace WeexCore {
  class RenderObject;

  class RenderAction;

  class RenderPage {

  private:
    RenderObject *pRoot;
    int mPageId;
    std::vector<RenderAction *> mRenderActions;
    std::map<std::string, RenderObject *> *mRenderObjectMap;

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
  };
}

#endif //RenderManager_h