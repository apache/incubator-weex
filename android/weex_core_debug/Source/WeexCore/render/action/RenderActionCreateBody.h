#ifndef WEEX_PROJECT_CREATEBODYACTION_H
#define WEEX_PROJECT_CREATEBODYACTION_H

#include "RenderAction.h"

namespace WeexCore {
  class RenderActionCreateBody : public RenderAction {

  public:
    RenderActionCreateBody(const std::string &pageId, const RenderObject *render);

    void ExecuteAction();

  public:
    StylesMap *mStyles;
    AttributesMap *mAttributes;
    EventsSet *mEvents;
    WXCorePadding mPaddings;
    WXCoreMargin mMargins;
    WXCoreBorderWidth mBorders;
    std::string mPageId;
    std::string mComponentType;
    std::string mRef;
  };
}

#endif //WEEX_PROJECT_CREATEBODYACTION_H
