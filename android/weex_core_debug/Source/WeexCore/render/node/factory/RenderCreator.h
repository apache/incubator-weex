#ifndef WEEX_PROJECT_RENDERCREATOR_H
#define WEEX_PROJECT_RENDERCREATOR_H

#include <string>

namespace WeexCore {

  class IRenderObject;

  class RenderCreator {

  private:
    RenderCreator() {}

    ~RenderCreator() {}

    //just to release singleton object
    class Garbo {
    public:
      ~Garbo() {
        if (RenderCreator::m_pInstance) {
          delete RenderCreator::m_pInstance;
        }
      }
    };

    static Garbo garbo;

  public:

    static RenderCreator *GetInstance() {
      if (!m_pInstance) {
        m_pInstance = new RenderCreator();
      }
      return m_pInstance;
    }

    IRenderObject *CreateRender(std::string type, std::string ref);

  private:
    static RenderCreator *m_pInstance;
  };
}

#endif //WEEX_PROJECT_RENDERCREATOR_H
