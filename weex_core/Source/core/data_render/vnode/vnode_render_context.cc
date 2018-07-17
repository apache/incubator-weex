//
// Created by Xu Jiacheng on 2018/7/17.
//

#include "core/data_render/vnode/vnode_render_context.h"

namespace weex {
namespace core {
namespace data_render {
void VNodeRenderContext::setVNodeRoot(VNode* v_node) {
  root_.reset(v_node);
}

VNode* VNodeRenderContext::find_node(const std::string& ref) {
  auto it = node_map_.find(ref);
  if (it == node_map_.end()) {
    return nullptr;
  }
  return it->second;
}
}
}
}