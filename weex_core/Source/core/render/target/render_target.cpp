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

#include "render_target.h"
#include "core/render/manager/render_manager.h"

namespace WeexCore {
    
    RenderTargetManager* RenderTargetManager::sharedInstance() {
        // c++11 guarantees thread-safe on static initialization
        static RenderTargetManager* instance = new RenderTargetManager();
        return instance;
    };

    std::string RenderTargetManager::getRenderTargetName(const std::string& page) {
        return RenderManager::GetInstance()->getPageArgument(page, "renderType");
    }
    
    void RenderTargetManager::registerRenderTarget(RenderTarget* target) {
        if (target) {
            targets_.push_back(target);
        }
    }
    
    RenderTarget* RenderTargetManager::getRenderTarget(const std::string& type) {
        for (auto t : targets_) {
            if (t->type() == type) {
                return t;
            }
        }
        return nullptr;
    }
    
    std::set<std::string> RenderTargetManager::getAvailableTargetNames() {
        std::set<std::string> result;
        for (auto t : targets_) {
            result.insert(t->type());
        }
        return result;
    }
    
}
