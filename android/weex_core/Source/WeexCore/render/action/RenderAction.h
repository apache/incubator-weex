#ifndef WEEXV8_RENDERACTION_H
#define WEEXV8_RENDERACTION_H

#include <wtf/text/WTFString.h>
#include "../../layout/Position.h"
#include "../../layout/RenderSize.h"

using namespace WeexCore;
using namespace WTF;

namespace WeexCore {

class Position;

class RenderSize;

class RenderAction {
protected:
    String mPageId;
    int mActionType;
    String mComponentType;
    String mParentRef;
    String mRef;
    String index;
    String mEvent;
    Position position;
    RenderSize renderSize;
    String key;
    String value;

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
