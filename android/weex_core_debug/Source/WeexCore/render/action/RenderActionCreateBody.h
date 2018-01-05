#ifndef WEEX_PROJECT_CREATEBODYACTION_H
#define WEEX_PROJECT_CREATEBODYACTION_H

#include "RenderAction.h"

namespace WeexCore {
  class RenderActionCreateBody : public RenderAction {

  public:
    RenderActionCreateBody(const std::string &pageId, RenderObject *render);

    void ExecuteAction();

  public:
    StylesMap *mStyles;
    AttributesMap *mAttributes;
    EventsSet *mEvents;
    PaddingsMap *mPaddings;
    MarginsMap *mMargins;
    BordersMap *mBorders;
    std::string mPageId;
    std::string mComponentType;
    std::string mRef;
  };
}

#endif //WEEX_PROJECT_CREATEBODYACTION_H
