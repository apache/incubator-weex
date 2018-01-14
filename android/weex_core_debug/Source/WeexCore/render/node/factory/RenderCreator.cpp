#include "RenderCreator.h"
#include "IRenderObject.h"
#include "IRenderFactory.h"
#include "RenderTextFactory.h"
#include "SimpleRenderFactory.h"
#include "RenderCellFactory.h"
#include "RenderType.h"

namespace WeexCore {

  RenderCreator *RenderCreator::m_pInstance = nullptr;

  IRenderObject *RenderCreator::CreateRender(const std::string &type, const std::string &ref) {
    IRenderFactory *factory;
    if (type == kRenderText) {
      factory = new RenderTextFactory();
    } else if (type == kRenderCell) {
      factory = new RenderCellFactory();
    } else {
      factory = new RenderCommonFactory();
    }

    IRenderObject *render = factory->CreateRender();
    render->SetRef(ref);
    render->SetType(type);
    delete factory;
    return render;
  }
}
