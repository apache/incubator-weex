#include "RenderAction.h"

namespace WeexCore {
RenderAction::RenderAction() {

}

RenderAction::~RenderAction() {

}

void RenderAction::ExecuteAction() {
    switch (mActionType) {
        case ACTION_CREATE_BODY:
            break;
        case ACTION_ADD_ELEMENT:
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
