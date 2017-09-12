#ifndef WEEXV8_RENDERACTION_H
#define WEEXV8_RENDERACTION_H

#include "../../layout/Position.h"
#include "../../layout/RenderSize.h"
#include "../../platform/android/BridgeAndroid.h"

using namespace WeexCore;

namespace WeexCore {

  enum ActionType {
    ACTION_CREATE_BODY,
    ACTION_ADD_ELEMENT,
    ACTION_REMOVE_ELEMENT,
    ACTION_MOVE_ELEMENT,
    ACTION_ADD_EVENT,
    ACTION_REMOVE_EVENT,
    ACTION_CREATE_FINISH,
    ACTION_UPDATE_ATTR,
    ACTION_UPDATE_STYLE,
    ACTION_UPDATE_FINISH,
    ACTION_REFRESH_FINISH,
  };

  class Position;

  class RenderSize;

  class RenderAction {
  public:
    std::string mPageId;
    int mActionType;
    std::string mComponentType;
    std::string mParentRef;
    std::string mRef;
    int mIndex;
    Position mPosition;
    RenderSize mRenderSize;
    std::string mKey;
    std::string mValue;

  public:
    RenderAction();

    ~RenderAction();

    void ExecuteAction();
  };

}

#endif //WEEXV8_RENDERACTION_H
