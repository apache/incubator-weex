#ifndef WEEX_PROJECT_IRENDEROBJECT_H
#define WEEX_PROJECT_IRENDEROBJECT_H

#include <string>
#include <WeexCore/layout/WXCoreLayout.h>

namespace WeexCore {

  class IRenderObject : public WXCoreLayoutNode {
  public:
    inline void SetRef(std::string ref) {
      mRef = ref;
    }

    inline const std::string &Ref() const {
      return mRef;
    }

    inline void SetPageId(std::string pageId) {
      this->mPageId = pageId;
    }

    inline const std::string &PageId() const {
      return mPageId;
    }

    inline void SetType(std::string type) {
      mType = type;
    }

    inline const std::string &Type() const {
      return mType;
    }

  private:
    std::string mPageId = "";
    std::string mRef = "";
    std::string mType = "";
  };
}

#endif //WEEX_PROJECT_IRENDEROBJECT_H
