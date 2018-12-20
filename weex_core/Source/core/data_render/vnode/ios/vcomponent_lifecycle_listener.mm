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

#include "core/data_render/vnode/vcomponent_lifecycle_listener.h"
#include "core/manager/weex_core_manager.h"
#include "core/data_render/vnode/vnode_render_manager.h"
#import "WXSDKManager.h"
#import "WXConvertUtility.h"

namespace weex {
namespace core {
namespace data_render {

void VComponentLifecycleListener::OnCreated(
    VComponent* component, Table* data, Table* props,
    const std::unordered_map<std::string, VComponent::VNodeRefs>& ref_map) {
    std::string page_id = component->exec_state()->context()->page_id();
    NSString *instanceId = NSSTRING(page_id.c_str());
    WXSDKInstance *instance = [WXSDKManager instanceForID:instanceId];
    if (!instance) {
        return;
    }

    int template_id = component->template_id();
    int component_id = component->id();
    int parent_id = -1;
    if (component->parent()) {
        parent_id = component->parent()->component()->id();
    }

    [[WXSDKManager bridgeMgr] callJSMethod:@"callJS" args:@[instanceId, @[@{@"method":@"componentHook", @"args":@[[NSNumber numberWithInt:template_id], @"lifecycle", @"created", @[[NSNumber numberWithInt:component_id],[NSNumber numberWithInt:parent_id], NSDICTIONARY(data), NSDICTIONARY(props), NSDICTIONARY(ref_map)]]}]]];
}

void VComponentLifecycleListener::OnUpdated(
    VComponent* component, Table* props, const std::unordered_map<std::string, VComponent::VNodeRefs>& ref_map) {
    std::string page_id = component->exec_state()->context()->page_id();
    NSString *instanceId = NSSTRING(page_id.c_str());
    WXSDKInstance *instance = [WXSDKManager instanceForID:instanceId];
    if (!instance) {
        return;
    }

    int component_id = component->id();
    [[WXSDKManager bridgeMgr] callJSMethod:@"callJS" args:@[instanceId, @[@{@"method":@"componentHook", @"args":@[[NSNumber numberWithInt:component_id], @"lifecycle", @"updated", @[NSDICTIONARY(props), NSDICTIONARY(ref_map)]]}]]];
}

void VComponentLifecycleListener::OnDestroyed(VComponent* component) {
    std::string page_id = component->exec_state()->context()->page_id();
    NSString *instanceId = NSSTRING(page_id.c_str());
    WXSDKInstance *instance = [WXSDKManager instanceForID:instanceId];
    if (!instance) {
        return;
    }

    int component_id = component->id();
    [[WXSDKManager bridgeMgr] callJSMethod:@"callJS" args:@[instanceId, @[@{@"method":@"componentHook", @"args":@[[NSNumber numberWithInt:component_id], @"lifecycle", @"destroyed", @[]]}]]];
}

}  // namespace data_render
}  // namespace core
}  // namespace weex
