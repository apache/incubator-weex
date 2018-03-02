#ifndef WEEX_PROJECT_ADDELEMENTACTION_H
#define WEEX_PROJECT_ADDELEMENTACTION_H

#include "RenderAction.h"

namespace WeexCore {

  class RenderActionAddElement : public RenderAction {

  public:
    RenderActionAddElement(const std::string &pageId, const RenderObject *render, const RenderObject *parent,
                           int index);

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
    std::string mParentRef;
    std::string mRef;
    int mIndex;
  };
}

#endif //WEEX_PROJECT_ADDELEMENTACTION_H
