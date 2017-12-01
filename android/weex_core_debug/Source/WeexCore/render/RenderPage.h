#ifndef RenderPage_H
#define RenderPage_H

#include <vector>
#include <string>
#include <map>

namespace WeexCore {

  class RenderAction;

  class RenderObject;

  class RenderPage {

  private:
    RenderObject *pRoot;

    std::string mPageId;

    std::vector<RenderAction *> mRenderActions;

    std::map<std::string, RenderObject *> mRenderObjectMap;

    void pushRenderToMap(RenderObject *render);

    void sendCreateBodyAction(RenderObject *render);

    void sendAddElementAction(RenderObject *child, RenderObject *parent, int index);

    void sendLayoutAction(RenderObject *render);

    void sendUpdateStyleAction(std::string key, std::string value, std::string ref);

    void sendUpdateAttrAction(std::string key, std::string value, std::string ref);

    void sendAddEventAction(RenderObject *render);

  public:
    RenderPage(std::string pageID);

    ~RenderPage();

    void calculateLayout();

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