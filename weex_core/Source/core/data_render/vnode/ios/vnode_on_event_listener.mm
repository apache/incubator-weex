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

#include "core/data_render/vnode/vnode_on_event_listener.h"
#include "core/data_render/vnode/vcomponent.h"
#include "core/data_render/vnode/vnode_render_manager.h"
#import "WXSDKManager.h"
#import "WXConvertUtility.h"
#import "WXUtility.h"

namespace weex {
namespace core {
namespace data_render {

void VNodeOnEventListener::OnEvent(VNode *node, const std::string &event,
                                            const std::string &json_args, const std::string dom_changes,
                                            const VNode::Params &params)
 {
    std::string page_id = node->component()->exec_state()->context()->page_id();
    NSString *instanceId = NSSTRING(page_id.c_str());
    WXSDKInstance *instance = [WXSDKManager instanceForID:instanceId];
    if (!instance) {
        return;
    }
    int template_id = node->component()->template_id();
    int component_id = node->component()->id();

    [[WXSDKManager bridgeMgr] callJSMethod:@"callJS" args:@[instanceId, @[@{@"method":@"fireEvent", @"args":@[NSSTRING(node->ref().empty() ? node->node_id().c_str() : node->ref().c_str()), @"nodeEvent", [WXUtility objectFromJSON:NSSTRING(json_args.c_str())], NSSTRING(dom_changes.c_str()), @{@"params":@[@{@"templateId":[NSNumber numberWithInt:template_id], @"componentId":[NSNumber numberWithInt:component_id],@"type":NSSTRING(event.c_str()), @"params": NSARRAY(params)}]}]}]]];
  }

}  // namespace data_render
}  // namespace core
}  // namespace weex
