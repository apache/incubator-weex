#ifndef WEEX_PROJECT_RENDERLIST_H
#define WEEX_PROJECT_RENDERLIST_H

#include <WeexCore/css/ConstantsName.h>
#include "RenderObject.h"

namespace WeexCore {

  class RenderList : public RenderObject {

    std::map<std::string, std::string> *GetDefaultStyle() {
      std::map<std::string, std::string> *style = new std::map<std::string, std::string>();

      bool isVertical = true;
      RenderObject *parent = (RenderObject *) getParent();

      if (parent != nullptr) {
        if (parent->Type() == kHList) {
          isVertical = false;
        }
      }

      std::string prop = isVertical ? HEIGHT : WIDTH;

      if (prop == HEIGHT && getStyleHeight() == 0.0 && GetStyle(FLEX).empty()) {
        style->insert(std::pair<std::string, std::string>(FLEX, "1"));
      } else if (prop == WIDTH && getStyleWidth() == 0.0 && GetStyle(FLEX).empty()) {
        style->insert(std::pair<std::string, std::string>(FLEX, "1"));
      }

      return style;
    }
  };
}

#endif //WEEX_PROJECT_RENDERLIST_H
