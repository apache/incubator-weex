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

#include "core/render/action/render_action_createfinish.h"
#include "core/render/action/render_action_render_success.h"
#include "render_page_base.h"
#include "core/moniter/render_performance.h"
#include "core/manager/weex_core_manager.h"

namespace WeexCore {

    RenderPageBase::RenderPageBase(const std::string& page_id, const std::string& page_type):
        page_id_(page_id), page_type_(page_type), render_performance_(nullptr) {
            is_platform_page_ = page_type == "platform";
            render_performance_ = new RenderPerformance();
    }
    
    RenderPageBase::~RenderPageBase() {
        if (render_performance_) {
            delete render_performance_;
        }
    }
    
    void RenderPageBase::CssLayoutTime(const int64_t &time) {
        if (this->render_performance_ != nullptr)
            this->render_performance_->cssLayoutTime += time;
    }
    
    void RenderPageBase::ParseJsonTime(const int64_t &time) {
        if (this->render_performance_ != nullptr)
            this->render_performance_->parseJsonTime += time;
    }
    
    void RenderPageBase::CallBridgeTime(const int64_t &time) {
        if (this->render_performance_ != nullptr)
            this->render_performance_->callBridgeTime += time;
    }
    
    std::vector<int64_t> RenderPageBase::PrintFirstScreenLog() {
        std::vector<int64_t> ret;
        if (this->render_performance_ != nullptr)
            ret = this->render_performance_->PrintPerformanceLog(onFirstScreen);
        return ret;
    }
    
    std::vector<int64_t> RenderPageBase::PrintRenderSuccessLog() {
        std::vector<int64_t> ret;
        if (this->render_performance_ != nullptr)
            ret = this->render_performance_->PrintPerformanceLog(onRenderSuccess);
        return ret;
    }
    
    std::unique_ptr<ValueWithType>
    RenderPageBase::CallNativeModule(const char *module, const char *method,
                                     const char *arguments, int arguments_length, const char *options,
                                     int options_length) {
        return WeexCoreManager::Instance()->
        getPlatformBridge()->
        platform_side()->
        CallNativeModule(page_id_.c_str(), module, method, arguments, arguments_length, options, options_length);
    }
    
    void RenderPageBase::CallNativeComponent(const char *ref, const char *method, const char *arguments, int arguments_length,
                                             const char *options, int options_length) {
        WeexCoreManager::Instance()
        ->getPlatformBridge()
        ->platform_side()
        ->CallNativeComponent(page_id_.c_str(), ref, method, arguments, arguments_length,
                              options, options_length);
    }

    void RenderPageBase::SendCreateFinishAction() {
        RenderAction *action = new RenderActionCreateFinish(page_id());
        PostRenderAction(action);
    }

    void RenderPageBase::SendRenderSuccessAction() {
        RenderAction *action = new RenderActionRenderSuccess(page_id());
        PostRenderAction(action);
    }

    void RenderPageBase::PostRenderAction(RenderAction *action) {
        if (action != nullptr) {
            action->ExecuteAction();
            delete action;
            action = nullptr;
        }
    }

    RenderPerformance* RenderPageBase::getPerformance() {
        return this->render_performance_;
    }

}
