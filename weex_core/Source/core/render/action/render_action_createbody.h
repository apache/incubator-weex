#ifndef WEEX_PROJECT_CREATEBODYACTION_H
#define WEEX_PROJECT_CREATEBODYACTION_H

#include "render_action.h"

namespace WeexCore {
  class RenderActionCreateBody : public render_action {

  public:
    RenderActionCreateBody(const std::string &pageId, const RenderObject *render);

    void ExecuteAction();

  public:
    StylesMap *mStyles;
    AttributesMap *mAttributes;
    EventsSet *mEvents;
    WXCoreMargin mMargins;
    WXCorePadding mPaddings;
    WXCoreBorderWidth mBorders;
    std::string mPageId;
    std::string mComponentType;
    std::string mRef;
  };
}

#endif //WEEX_PROJECT_CREATEBODYACTION_H
