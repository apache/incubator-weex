#ifndef WEEX_PROJECT_CREATEBODYACTION_H
#define WEEX_PROJECT_CREATEBODYACTION_H

#include "RenderAction.h"

namespace WeexCore {
  class RenderActionCreateBody : public RenderAction {

  public:
    RenderActionCreateBody(const int &pageId, const RenderObject *render);

    void ExecuteAction();

  public:
    StylesMap *mStyles;
    AttributesMap *mAttributes;
    EventsSet *mEvents;
    WXCoreMargin mMargins;
    WXCorePadding mPaddings;
    WXCoreBorderWidth mBorders;
    int mPageId;
    std::string mComponentType;
    int mRef;
  };
}

#endif //WEEX_PROJECT_CREATEBODYACTION_H
