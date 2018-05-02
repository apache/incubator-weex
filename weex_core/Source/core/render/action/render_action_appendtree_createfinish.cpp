#include "render_action_appendtree_createfinish.h"
#include "../../../android/bridge/impl/bridge_impl_android.h"

namespace WeexCore {

  RenderActionAppendTreeCreateFinish::RenderActionAppendTreeCreateFinish(const std::string &pageId, const std::string &ref) {
    this->mPageId = pageId;
    this->mRef = ref;
  }

  void RenderActionAppendTreeCreateFinish::ExecuteAction() {
    Bridge_Impl_Android::getInstance()->callAppendTreeCreateFinish(mPageId.c_str(), mRef.c_str());
  }
}
