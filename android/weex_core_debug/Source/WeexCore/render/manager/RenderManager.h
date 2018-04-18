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

    bool AddRenderObject(const std::string &pageId, const std::string &parentRef, int index,
                         const std::string &data);

    bool RemoveRenderObject(const std::string &pageId, const std::string &ref);

    bool
    MoveRenderObject(const std::string &pageId, const std::string &ref,
                     const std::string &parentRef, int index);

    bool UpdateAttr(const std::string &pageId, const std::string &ref, const std::string &data);

    bool UpdateStyle(const std::string &pageId, const std::string &ref, const std::string &data);

    bool AddEvent(const std::string &pageId, const std::string &ref, const std::string &event);

    bool RemoveEvent(const std::string &pageId, const std::string &ref, const std::string &event);

    bool CreateFinish(const std::string &pageId);

    RenderPage *GetPage(const std::string &id);

    bool ClosePage(const std::string &pageId);

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