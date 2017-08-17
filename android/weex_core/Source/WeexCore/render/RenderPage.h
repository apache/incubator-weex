#ifndef RenderPage_H
#define RenderPage_H

#include "RenderPage.h"

#include <wtf/text/WTFString.h>
#include <wtf/HashSet.h>
#include "../../base/LogUtils.h"
#include "../../rapidjson/weexjsontools.h"
#include "RenderManager.h"
#include "RenderObject.h"
#include "action/RenderAction.h"

using namespace WTF;
namespace WeexCore
{

class RenderObject;
class RenderAction;

class RenderPage
{

  private:
    RenderObject *pRoot;
    int mPageId;
    Vector<RenderAction *> mRenderActions;
    HashMap<String, RenderObject*> *mRenderObjectMap;

  public:
    RenderPage(String pageID, String data);
    ~RenderPage();

    void addRenderObject(String parentRef, int insertPosiotn, String data);
    void removeRenderObject(String ref);
    void moveRenderObject(String ref, String parentRef, String index);

    void updateStyle(String ref, String key, String value);
    void updateAttr(String ref, String key, String value);
    void addEvent(String ref, String event);
    void removeEvent(String ref, String event);

    void addRenderAction(RenderAction *action);

    void batch();

    RenderObject* getRenderObject(String ref);

    void setRootRenderObject(RenderObject *root);

    RenderObject* getRootRenderObject();
};
}

#endif //RenderManager_h