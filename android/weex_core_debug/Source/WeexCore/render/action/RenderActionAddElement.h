#ifndef WEEX_PROJECT_ADDELEMENTACTION_H
#define WEEX_PROJECT_ADDELEMENTACTION_H

#include "RenderAction.h"

namespace WeexCore {

  class RenderActionAddElement : public RenderAction {

  public:

    STYLES_MAP *mStyles;
    ATTRIBUTES_MAP *mAttributes;
    EVENTS_SET *mEvents;
    PADDING_MAP *mPaddings;
    MARGIN_MAP *mMargins;
    BORDER_MAP *mBorders;
    std::string mPageId;
    std::string mComponentType;
    std::string mParentRef;
    std::string mRef;
    int mIndex;

    RenderActionAddElement(const std::string &pageId, RenderObject *render, RenderObject *parent,
                           const int &index);

    void ExecuteAction();
  };
}

#endif //WEEX_PROJECT_ADDELEMENTACTION_H
