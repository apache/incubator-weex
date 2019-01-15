/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#ifndef CORE_DATA_RENDER_VNODE_VCOMPONENT_LIFECYCLE_LISTENER_H_
#define CORE_DATA_RENDER_VNODE_VCOMPONENT_LIFECYCLE_LISTENER_H_

#include "core/data_render/vnode/vcomponent.h"

namespace weex {
namespace core {
namespace data_render {
class VComponentLifecycleListener : public VComponent::LifecycleListener {
  void OnCreated(
      VComponent* component, Table* data, Table* props,
      const std::unordered_map<std::string, VComponent::VNodeRefs>& ref_map);
  void OnUpdated(
      VComponent* component, Table* props,
      const std::unordered_map<std::string, VComponent::VNodeRefs>& ref_map);
  void OnDestroyed(VComponent* component);
};
}  // namespace data_render
}  // namespace core
}  // namespace weex

#endif  // CORE_DATA_RENDER_VNODE_VCOMPONENT_LIFECYCLE_LISTENER_H_
