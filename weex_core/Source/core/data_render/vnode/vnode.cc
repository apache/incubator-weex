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
#include "core/data_render/vnode/vnode.h"

namespace weex {
namespace core {
namespace data_render {

VNode::VNode(const std::string& id, const std::string& tag_name) {}

VNode::~VNode() {}

void VNode::SetStyles(const std::string& key, const std::string& value) {}

void VNode::SetAttributes(const std::string& key, const std::string& value) {}

void VNode::AddEvent(const std::string& event, const std::string& function,
                     const std::vector<std::string>& params) {}
}  // namespace data_render
}  // namespace core
}  // namespace weex