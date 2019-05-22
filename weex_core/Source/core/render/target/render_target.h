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

#include "../../api/wx_api.h"
#if HERON_LIBRARY
#include <jsengine/WeexApiValue.h>
#else
#include "../../../include/WeexApiValue.h"
#endif
#include <string>
#include <vector>
#include <map>
#include <set>

namespace WeexCore {

    class WX_EXPORT RenderTarget {

    public:
        struct WX_EXPORT PageOptions {
            bool is_round_off = false;
            float viewport_width = -1;
            float device_width = -1;
            float view_scale = 1;
            void* user_data = nullptr;
            std::map<std::string, std::string> args;
        };

    public:
        RenderTarget(const std::string& type): type_(type) {}

        const std::string& type() { return type_; }

    public:
        // APIs that should be implemented by sub class
        virtual void createPage(const std::string& page, const PageOptions& options) = 0;

        virtual void deletePage(const std::string& page) = 0;

        virtual void createBody(const std::string& page, const std::string& ref, const std::string& type,
                                std::shared_ptr<std::map<std::string, std::string>> styles,
                                std::shared_ptr<std::map<std::string, std::string>> attrs,
                                std::shared_ptr<std::set<std::string>> events) = 0;

        virtual void addElement(const std::string& page, const std::string& ref, const std::string& type,
                                const std::string &parent_ref, int index,
                                std::shared_ptr<std::map<std::string, std::string>> styles,
                                std::shared_ptr<std::map<std::string, std::string>> attrs,
                                std::shared_ptr<std::set<std::string>> events) = 0;

        virtual void removeElement(const std::string& page, const std::string &ref) = 0;

        virtual void moveElement(const std::string& page, const std::string &ref, const std::string &parent_ref, int index) = 0;

        virtual void updateStyles(const std::string& page, const std::string &ref, std::shared_ptr<std::map<std::string, std::string>> styles) = 0;

        virtual void updateAttributes(const std::string& page, const std::string &ref, std::shared_ptr<std::map<std::string, std::string>> attrs) = 0;

        virtual void addEvent(const std::string& page, const std::string &ref, const std::string &event) = 0;

        virtual void removeEvent(const std::string& page, const std::string &ref, const std::string &event) = 0;

        virtual void createFinish(const std::string& page) = 0;
        
        virtual bool shouldHandleModuleMethod(const std::string& module, const std::string& method) = 0;
        
        virtual std::unique_ptr<ValueWithType> callNativeModule(const std::string& page,
                                                                const std::string& module,
                                                                const std::string& method,
                                                                const std::string& arguments,
                                                                int nArguments,
                                                                const std::string& options,
                                                                int nOptions, bool& handled) = 0;
        
        virtual void callNativeComponent(const std::string& page, const std::string& ref, const std::string& method, const std::string& arguments, int nArguments, const std::string& options, int nOptions) = 0;

        // Life cycle
        virtual uintptr_t createRootView(const std::string& page, float x, float y, float width, float height) = 0;

        //        virtual void SetDefaultHeightAndWidthIntoRootRender(const float default_width, const float default_height,
        //                                                            const bool is_width_wrap_content, const bool is_height_wrap_content) = 0;
        //
        //        virtual void OnRenderPageClose() = 0;
        //
        //        virtual float GetViewportWidth() = 0;
        //        virtual void SetViewportWidth(float value) = 0;
        //        virtual bool GetRoundOffDeviation() = 0;
        //        virtual void SetRoundOffDeviation(bool value) = 0;

    protected:
        std::string type_;
    };

    class WX_EXPORT RenderTargetManager {
    public:
        WX_EXPORT static RenderTargetManager* sharedInstance();
        
        WX_EXPORT static std::string getRenderTargetName(const std::string& page);

        WX_EXPORT void registerRenderTarget(RenderTarget* target);

        WX_EXPORT RenderTarget* getRenderTarget(const std::string& type);

        WX_EXPORT std::set<std::string> getAvailableTargetNames();

    private:
        std::vector<RenderTarget*> targets_;
    };

}  // namespace WeexCore
