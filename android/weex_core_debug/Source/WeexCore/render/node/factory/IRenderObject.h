#ifndef WEEX_PROJECT_IRENDEROBJECT_H
#define WEEX_PROJECT_IRENDEROBJECT_H

#include <string>
#include <WeexCore/layout/WXCoreLayout.h>

namespace WeexCore {

  constexpr int kInvalidPageID = -2;
  constexpr int kInvalidRef = -2;

  class IRenderObject : public WXCoreLayoutNode {
  public:
    inline void SetRef(int ref) {
      mRef = ref;
    }

    inline const int &Ref() const {
      return mRef;
    }

    inline void SetPageId(int pageId) {
      this->mPageId = pageId;
    }

    inline const int &PageId() const {
      return mPageId;
    }

    inline void SetType(std::string type) {
      mType = type;
    }

    inline const std::string &Type() const {
      return mType;
    }

  private:
    int mPageId = kInvalidPageID;
    int mRef = kInvalidRef;
    std::string mType = "";
  };
}

#endif //WEEX_PROJECT_IRENDEROBJECT_H
