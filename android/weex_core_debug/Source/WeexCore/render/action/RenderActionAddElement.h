#ifndef WEEX_PROJECT_ADDELEMENTACTION_H
#define WEEX_PROJECT_ADDELEMENTACTION_H

#include "RenderAction.h"

namespace WeexCore {

  class RenderActionAddElement : public RenderAction {

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

    RenderActionAddElement(const std::string &pageId, RenderObject *render, RenderObject *parent,
                           const int &index);

    void ExecuteAction();
  };
}

#endif //WEEX_PROJECT_ADDELEMENTACTION_H
