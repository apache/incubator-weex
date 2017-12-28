#ifndef WEEX_PROJECT_CREATEBODYACTION_H
#define WEEX_PROJECT_CREATEBODYACTION_H

#include "RenderAction.h"

namespace WeexCore {
  class RenderActionCreateBody : public RenderAction {

  public:

    STYLES_MAP *mStyles;
    ATTRIBUTES_MAP *mAttributes;
    EVENTS_SET *mEvents;
    PADDING_MAP *mPaddings;
    MARGIN_MAP *mMargins;
    BORDER_MAP *mBorders;
    std::string mPageId;
    std::string mComponentType;
    std::string mRef;

    RenderActionCreateBody(const std::string &pageId, RenderObject *render);

    void ExecuteAction();
  };
}

#endif //WEEX_PROJECT_CREATEBODYACTION_H
