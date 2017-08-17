#ifndef RenderManager_h
#define RenderManager_h

#include "RenderObject.h"
#include "RenderPage.h"
#include <wtf/HashMap.h>
#include "../../base/LogUtils.h"
#include "../../rapidjson/weexjsontools.h"


namespace WeexCore
{

class RenderManager
{

  private:
    RenderManager();
    ~RenderManager();

    static RenderManager *m_pInstance;

    //just to release singleton object
    class Garbo
    {
      public:
        ~Garbo()
        {
            if (RenderManager::m_pInstance)
            {
                delete RenderManager::m_pInstance;
            }
        }
    };
    static Garbo garbo;

    // save all pages info with RenderPage;
    HashMap<String, RenderPage *> *mPages = nullptr;

  public:
    static RenderManager *getInstance()
    {
        if (m_pInstance == NULL)
        {
            m_pInstance = new RenderManager();
        }
        return m_pInstance;
    }

    //TODO: how to triggle a Frame event
    void batch();

    // create root node
    void createPage(String pageId, String data);

    void addRenderObject(String pageId, String parentRef, int index, String data);

    void removeRenderObject(String pageId, String ref);

    void moveRenderObject(String pageId, String ref, String parentRef, String index);

    void updateAttr(String pageId, String ref, String key, String value);

    void updateStyle(String pageId, String ref, String key, String value);

    void addEvent(String pageId, String ref, String event);

    void removeEvent(String pageId, String ref, String event);

    RenderPage *getPage(String id);

    void printRenderAndLayoutTree(String pageId);
};
}

#endif //RenderManager_h