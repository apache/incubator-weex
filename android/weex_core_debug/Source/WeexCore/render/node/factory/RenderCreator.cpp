#include "RenderCreator.h"
#include "IRenderObject.h"
#include "IRenderFactory.h"
#include "RenderTextFactory.h"
#include "SimpleRenderFactory.h"
#include "RenderCellFactory.h"
#include "RenderType.h"
#include "RenderIndicatorFactory.h"
#include "RenderInputFactory.h"
#include "RenderListFactory.h"
#include "RenderMaskFactory.h"
#include "RenderScrollerFactory.h"
#include "RenderSwitchFactory.h"
#include "RenderTextAreaFactory.h"
#include "RenderAppBarFactory.h"

namespace WeexCore {

  RenderCreator *RenderCreator::m_pInstance = nullptr;

  IRenderObject *RenderCreator::CreateRender(const std::string &type, const std::string &ref) {
    IRenderFactory *factory;
    if (type == kRenderText) {
      factory = new RenderTextFactory();
    } else if (type == kRenderCell) {
      factory = new RenderCellFactory();
    } else if (type == kRenderIndicator) {
      factory = new RenderIndicatorFactory();
    } else if (type == kRenderInput) {
      factory = new RenderInputFactory();
    } else if (type == kRenderList || type == kRenderWaterfall) {
      factory = new RenderListFactory();
    } else if (type == kRenderMask) {
      factory = new RenderMaskFactory();
    } else if (type == kRenderScroller) {
      factory = new RenderScrollerFactory();
    } else if (type == kRenderSwitch) {
      factory = new RenderSwitchFactory();
    } else if (type == kRenderTextArea) {
      factory = new RenderTextAreaFactory();
    } else if (type == kRenderAppBar) {
      factory = new RenderAppBarFactory();
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
