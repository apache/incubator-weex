#include "RenderAction.h"

using namespace WeexCore;

namespace WeexCore {

  RenderAction::RenderAction() {}

  RenderAction::~RenderAction() {}

  void RenderAction::ExecuteAction() {
    switch (mActionType) {
      case ACTION_CREATE_BODY:
        BridgeAndroid::getInstance()->callCreateBodyByWeexCore(mPageId, mComponentType, mRef,
                                                               mPosition.mTop,
                                                               mPosition.mBottom, mPosition.mLeft,
                                                               mPosition.mRight,
                                                               mRenderSize.mHeight,
                                                               mRenderSize.mWidth);
        break;
      case ACTION_ADD_ELEMENT:
        BridgeAndroid::getInstance()->callAddElementByWeexCore(mPageId, mComponentType, mRef,
                                                               mPosition.mTop,
                                                               mPosition.mBottom, mPosition.mLeft,
                                                               mPosition.mRight,
                                                               mRenderSize.mHeight,
                                                               mRenderSize.mWidth, mIndex,
                                                               mParentRef);
        break;
      case ACTION_REMOVE_ELEMENT:
        break;
      case ACTION_MOVE_ELEMENT:
        break;
      case ACTION_ADD_EVENT:
        break;
      case ACTION_REMOVE_EVENT:
        break;
      case ACTION_CREATE_FINISH:
        break;
      case ACTION_UPDATE_ATTR:
        break;
      case ACTION_UPDATE_STYLE:
        BridgeAndroid::getInstance()->callUpdateStyleByWeexCore(mPageId, mRef, mKey, mValue);
        break;
      case ACTION_UPDATE_FINISH:
        break;
      case ACTION_REFRESH_FINISH:
        break;
      default:
        break;
    }
  }
}
