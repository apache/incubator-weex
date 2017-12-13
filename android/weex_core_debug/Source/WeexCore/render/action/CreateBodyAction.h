#ifndef WEEX_PROJECT_CREATEBODYACTION_H
#define WEEX_PROJECT_CREATEBODYACTION_H

#include "RenderAction.h"

namespace WeexCore {
  class CreateBodyAction : public RenderAction {

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

    void ExecuteAction() {
      RenderPage *page = RenderManager::getInstance()->getPage(mPageId);
      long long startTime = getCurrentTime();
      BridgeAndroid::getInstance()->callCreateBodyByWeexCore(mPageId, mComponentType, mRef,
                                                             mStyles, mAttributes,
                                                             mEvents,
                                                             mPaddings, mMargins, mBorders);
      if (page != nullptr) {
        page->jniCallTime(getCurrentTime() - startTime);
        page->addElementActionJNITime(getCurrentTime() - startTime);
      }
    }

    void GenerateAction(std::string pageId, RenderObject *render) {
      this->mAttributes = render->getAttributes();
      this->mStyles = render->getStyles();
      this->mEvents = render->getEvents();
      this->mPaddings = render->getPaddings();
      this->mMargins = render->getMargins();
      this->mBorders = render->getBorders();
      this->mPageId = pageId;
      this->mComponentType = render->getType();
      this->mRef = render->getRef();
    }
  };
}

#endif //WEEX_PROJECT_CREATEBODYACTION_H
