#ifndef WEEX_PROJECT_ADDELEMENTACTION_H
#define WEEX_PROJECT_ADDELEMENTACTION_H

#include "RenderAction.h"

namespace WeexCore {

  class RenderActionAddElement : public RenderAction {

  public:
    RenderActionAddElement(const int &pageId, const RenderObject *render, const RenderObject *parent,
                           int index);

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
    std::string mParentRef;
    std::string mRef;
    int mIndex;
  };
}

#endif //WEEX_PROJECT_ADDELEMENTACTION_H
