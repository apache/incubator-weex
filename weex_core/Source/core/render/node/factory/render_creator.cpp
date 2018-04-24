#include "render_creator.h"
#include "i_render_object.h"
#include "i_render_factory.h"
#include "render_text_factory.h"
#include "simple_render_factory.h"
#include "render_cell_factory.h"
#include "render_type.h"
#include "render_indicator_factory.h"
#include "render_input_factory.h"
#include "render_list_factory.h"
#include "render_mask_factory.h"
#include "render_scroller_factory.h"
#include "render_switch_factory.h"
#include "render_textarea_factory.h"
#include "render_appbar_factory.h"

namespace WeexCore {

  RenderCreator *RenderCreator::m_pInstance = nullptr;

  IRenderObject *RenderCreator::CreateRender(const std::string &type, const std::string &ref) {
    IRenderFactory *factory;
    if (type == kRenderText) {
      factory = new RenderTextFactory();
    } else if (type == kRenderCell || type == kRenderHeader) {
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
