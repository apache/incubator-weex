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

    void Batch(const int &pageId);

    // create root node
    bool CreatePage(int pageId, const std::string &data);

    bool AddRenderObject(const int &pageId, const std::string &parentRef, int index,
                         const std::string &data);

    bool RemoveRenderObject(const int &pageId, const std::string &ref);

    bool
    MoveRenderObject(const int &pageId, const std::string &ref,
                     const std::string &parentRef, int index);

    bool UpdateAttr(const int &pageId, const std::string &ref, const std::string &data);

    bool UpdateStyle(const int &pageId, const std::string &ref, const std::string &data);

    bool AddEvent(const int &pageId, const std::string &ref, const std::string &event);

    bool RemoveEvent(const int &pageId, const std::string &ref, const std::string &event);

    bool CreateFinish(const int &pageId);

    RenderPage *GetPage(const int &id);

    bool ClosePage(const int &pageId);

    static RenderManager *GetInstance() {
      if (!m_pInstance) {
        m_pInstance = new RenderManager();
      }
      return m_pInstance;
    }

  private:
    static RenderManager *m_pInstance;
    std::map<int, RenderPage *> mPages;
  };
}

#endif //RenderManager_h