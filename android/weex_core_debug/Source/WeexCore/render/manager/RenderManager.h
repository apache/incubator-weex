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

    void batch(const std::string &pageId);

    // create root node
    bool createPage(const std::string &pageId, const std::string &data);

    bool addRenderObject(const std::string &pageId, const std::string &parentRef, const int &index,
                         const std::string &data);

    bool removeRenderObject(const std::string &pageId, const std::string &ref);

    bool
    moveRenderObject(const std::string &pageId, const std::string &ref,
                     const std::string &parentRef, int index);

    bool updateAttr(const std::string &pageId, const std::string &ref, const std::string &data);

    bool updateStyle(const std::string &pageId, const std::string &ref, const std::string &data);

    bool addEvent(const std::string &pageId, const std::string &ref, const std::string &event);

    bool removeEvent(const std::string &pageId, const std::string &ref, const std::string &event);

    bool createFinish(const std::string &pageId);

    RenderPage *getPage(const std::string &id);

    void printRenderAndLayoutTree(const std::string &pageId);

    bool ClosePage(std::string pageId);

    static RenderManager *getInstance() {
      if (!m_pInstance) {
        m_pInstance = new RenderManager();
      }
      return m_pInstance;
    }

  private:
    static RenderManager *m_pInstance;
    std::map<std::string, RenderPage *> mPages;
  };
}

#endif //RenderManager_h