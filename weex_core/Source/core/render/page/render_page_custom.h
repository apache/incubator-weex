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

#pragma once

#include "render_page_base.h"
#include "core/render/target/render_target.h"

namespace WeexCore {
    
    class RenderTarget;
    
    class RenderPageCustom: public RenderPageBase {
    public:
        struct PageOptions {
            bool is_round_off = false;
            float viewport_width = -1;
            float device_width = -1;
            float view_scale = 1;
            std::map<std::string, std::string> args;
        };
        
    public:
        RenderPageCustom() = delete;
        explicit RenderPageCustom(const std::string& page_id, const std::string& page_type, const PageOptions& options);
        
        RenderTarget* GetRenderTarget() { return target_; }
        bool IsValid() const { return valid_; }
        void Invalidate() { valid_ = false; }
        
        virtual bool CreateBody(const std::string& ref, const std::string& type,
                                std::map<std::string, std::string>* styles,
                                std::map<std::string, std::string>* attrs,
                                std::set<std::string>* events) override;
        
        virtual bool AddRenderObject(const std::string& ref, const std::string& type,
                                     const std::string &parent_ref, int index,
                                     std::map<std::string, std::string>* styles,
                                     std::map<std::string, std::string>* attrs,
                                     std::set<std::string>* events) override;
        
        virtual bool RemoveRenderObject(const std::string &ref) override;
        
        virtual bool MoveRenderObject(const std::string &ref, const std::string &parent_ref, int index) override;
        
        virtual bool UpdateStyle(const std::string &ref, std::vector<std::pair<std::string, std::string>> *styles) override;
        
        virtual bool UpdateAttr(const std::string &ref, std::vector<std::pair<std::string, std::string>> *attrs) override;
        
        virtual bool AddEvent(const std::string &ref, const std::string &event) override;
        
        virtual bool RemoveEvent(const std::string &ref, const std::string &event) override;
        
        virtual bool CreateFinish() override;
        
        virtual std::unique_ptr<ValueWithType> CallNativeModule(const char *module, const char *method,
                                                                const char *arguments, int arguments_length, const char *options,
                                                                int options_length) override;
        virtual void CallNativeComponent(const char *ref, const char *method, const char *arguments, int arguments_length,
                                         const char *options, int options_length) override;

        // Life cycle
        virtual void SetDefaultHeightAndWidthIntoRootRender(const float default_width, const float default_height,
                                                            const bool is_width_wrap_content, const bool is_height_wrap_content) override;
        
        virtual void OnRenderPageClose() override;
        
        virtual bool ReapplyStyles() override;
        
        virtual float GetViewportWidth() override;
        virtual void SetViewportWidth(float value) override;
        virtual bool GetRoundOffDeviation() override;
        virtual void SetRoundOffDeviation(bool value) override;
        virtual float GetDeviceWidth() override;
        virtual void SetDeviceWidth(float value) override;
        
    private:
        bool valid_;
        RenderTarget* target_;
    };
    
}  // namespace WeexCore
