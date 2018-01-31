#ifndef WEEX_PROJECT_IRENDEROBJECT_H
#define WEEX_PROJECT_IRENDEROBJECT_H

#include <string>

namespace WeexCore {
  class IRenderObject {
  public:
    inline void SetRef(std::string ref) {
      mRef = ref;
    }

    inline const std::string &Ref() {
      return mRef;
    }

    inline void SetPageId(std::string pageId) {
      this->mPageId = pageId;
    }

    inline const std::string &PageId() {
      return mPageId;
    }

    inline void SetType(std::string type) {
      mType = type;
    }

    inline const std::string &Type() {
      return mType;
    }

  private:
    std::string mPageId = "";
    std::string mRef = "";
    std::string mType = "";
  };
}

#endif //WEEX_PROJECT_IRENDEROBJECT_H
