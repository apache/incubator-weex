#ifndef WEEX_PROJECT_ADDELEMENTACTION_H
#define WEEX_PROJECT_ADDELEMENTACTION_H

#include "render_action.h"

namespace WeexCore {

  class RenderActionAddElement : public render_action {

  public:
    RenderActionAddElement(const std::string &pageId, const RenderObject *render, const RenderObject *parent,
                           int index);

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
    std::string mParentRef;
    std::string mRef;
    int mIndex;
  };
}

#endif //WEEX_PROJECT_ADDELEMENTACTION_H
