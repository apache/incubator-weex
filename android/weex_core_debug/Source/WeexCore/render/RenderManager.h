#ifndef RenderManager_h
#define RenderManager_h

#include <map>
#include <string>

namespace WeexCore {

  class RenderPage;

  class RenderManager {

  private:
    RenderManager() {}

    ~RenderManager() {}

    static RenderManager *m_pInstance;

    std::map<std::string, RenderPage *> mPages;

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

    //TODO: how to triggle a Frame event
    void batch();

    // create root node
    bool createPage(std::string pageId, std::string data);

    bool addRenderObject(std::string pageId, std::string parentRef, int index, std::string data);

    bool removeRenderObject(std::string pageId, std::string ref);

    bool
    moveRenderObject(std::string pageId, std::string ref, std::string parentRef, std::string index);

    bool updateAttr(std::string pageId, std::string ref, std::string data);

    bool updateStyle(std::string pageId, std::string ref, std::string data);

    bool addEvent(std::string pageId, std::string ref, std::string event);

    bool removeEvent(std::string pageId, std::string ref, std::string event);

    bool createFinish(std::string pageId);

    RenderPage *getPage(std::string id);

    void printRenderAndLayoutTree(std::string pageId);

    static RenderManager *getInstance() {
      if (!m_pInstance) {
        m_pInstance = new RenderManager();
      }
      return m_pInstance;
    }
  };
}

#endif //RenderManager_h