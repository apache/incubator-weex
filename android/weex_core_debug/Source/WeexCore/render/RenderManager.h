#ifndef RenderManager_h
#define RenderManager_h

#include "RenderObject.h"
#include "RenderPage.h"
#include <map>
#include <string>
#include "./base/LogUtils.h"
#include "./rapidjson/weexjsontools.h"

namespace WeexCore {
  class RenderPage;

  class RenderManager {

  private:
    RenderManager();

    ~RenderManager();

    static RenderManager *m_pInstance;

    //just to release singleton object
    class Garbo {
    public:
      ~Garbo() {
        if (RenderManager::m_pInstance) {
          delete RenderManager::m_pInstance;
        }
      }
    };

    static Garbo garbo;

  public:
    static RenderManager *getInstance() {
      if (!m_pInstance) {
        m_pInstance = new RenderManager();
      }
      return m_pInstance;
    }

    //TODO: how to triggle a Frame event
    void batch();

    // create root node
    void createPage(std::string pageId, std::string data);

    void addRenderObject(std::string pageId, std::string parentRef, int index, std::string data);

    void removeRenderObject(std::string pageId, std::string ref);

    void
    moveRenderObject(std::string pageId, std::string ref, std::string parentRef, std::string index);

    void updateAttr(std::string pageId, std::string ref, std::string key, std::string value);

    void updateStyle(std::string pageId, std::string ref, std::string key, std::string value);

    void addEvent(std::string pageId, std::string ref, std::string event);

    void removeEvent(std::string pageId, std::string ref, std::string event);

    RenderPage *getPage(std::string id);

    void printRenderAndLayoutTree(std::string pageId);
  };
}

#endif //RenderManager_h