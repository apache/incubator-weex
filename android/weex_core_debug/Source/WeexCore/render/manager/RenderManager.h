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

    void Batch(const std::string &pageId);

    // create root node
    bool CreatePage(std::string pageId, const std::string &data);

    bool AddRenderObject(std::string pageId, std::string parentRef, int index,
                         const std::string &data);

    bool RemoveRenderObject(std::string pageId, std::string ref);

    bool
    MoveRenderObject(std::string pageId, std::string ref,
                     std::string parentRef, int index);

    bool UpdateAttr(std::string pageId, std::string ref, const std::string &data);

    bool UpdateStyle(std::string pageId, std::string ref, const std::string &data);

    bool AddEvent(std::string pageId, std::string ref, std::string event);

    bool RemoveEvent(std::string pageId, std::string ref, std::string event);

    bool CreateFinish(std::string pageId);

    RenderPage *GetPage(const std::string &id);

    void PrintRenderAndLayoutTree(const std::string &pageId);

    bool ClosePage(std::string pageId);

    static RenderManager *GetInstance() {
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