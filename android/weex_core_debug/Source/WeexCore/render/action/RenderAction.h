#ifndef WEEXV8_RENDERACTION_H
#define WEEXV8_RENDERACTION_H

#include "../../layout/Position.h"
#include "../../layout/RenderSize.h"
#include <string>

using namespace WeexCore;
using namespace std;

namespace WeexCore {

  class Position;

  class RenderSize;

  class RenderAction {
  protected:
    string mPageId;
    int mActionType;
    string mComponentType;
    string mParentRef;
    string mRef;
    string index;
    string mEvent;
    Position position;
    RenderSize renderSize;
    string key;
    string value;

  public:
    RenderAction();

    ~RenderAction();

    void ExecuteAction();

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
  };

}

#endif //WEEXV8_RENDERACTION_H
