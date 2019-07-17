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
#include <wson_parser.h>
#include "base/log_defines.h"
#include "render_page_custom.h"

namespace WeexCore {
    
    template<typename T>
    static std::shared_ptr<T> SharedMove(T& source) {
        return std::make_shared<T>(std::move(source));
    }

    RenderPageCustom::RenderPageCustom(const std::string& page_id, const std::string& page_type, const PageOptions& options): RenderPageBase(page_id, page_type) {
        valid_ = true;
        target_ = RenderTargetManager::sharedInstance()->getRenderTarget(page_type);
        if (target_) {
            RenderTarget::PageOptions targetOptions;
            targetOptions.args = std::move(options.args);
            targetOptions.view_scale = options.view_scale;
            targetOptions.is_round_off = options.is_round_off;
            targetOptions.viewport_width = options.viewport_width;
            targetOptions.device_width = options.device_width;
            target_->createPage(page_id, targetOptions);;
        }
    }
    
    bool RenderPageCustom::CreateBody(const std::string& ref, const std::string& type,
                            std::map<std::string, std::string>* styles,
                            std::map<std::string, std::string>* attrs,
                            std::set<std::string>* events) {
        if (target_) {
            auto managedStyles = SharedMove(*styles);
            auto managedAttrs = SharedMove(*attrs);
            auto managedEvents = SharedMove(*events);
            target_->createBody(page_id_, ref, type, managedStyles, managedAttrs, managedEvents);
        }
        delete styles;
        delete attrs;
        delete events;
        return true;
    }
    
    bool RenderPageCustom::AddRenderObject(const std::string& ref, const std::string& type,
                                           const std::string &parent_ref, int index,
                                           std::map<std::string, std::string>* styles,
                                           std::map<std::string, std::string>* attrs,
                                           std::set<std::string>* events) {
        if (target_) {
            auto managedStyles = SharedMove(*styles);
            auto managedAttrs = SharedMove(*attrs);
            auto managedEvents = SharedMove(*events);
            target_->addElement(page_id_, ref, type, parent_ref, index, managedStyles, managedAttrs, managedEvents);
        }
        delete styles;
        delete attrs;
        delete events;
        return true;
    }
    
    bool RenderPageCustom::RemoveRenderObject(const std::string &ref) {
        if (target_) {
            target_->removeElement(page_id_, ref);
        }
        return true;
    }
    
    bool RenderPageCustom::MoveRenderObject(const std::string &ref, const std::string &parent_ref, int index) {
        if (target_) {
            target_->moveElement(page_id_, ref, parent_ref, index);
        }
        return true;
    }
    
    bool RenderPageCustom::UpdateStyle(const std::string &ref, std::vector<std::pair<std::string, std::string>> *styles) {
        if (target_) {
            std::shared_ptr<std::map<std::string, std::string>> managedStyles = std::make_shared<std::map<std::string, std::string>>();
            for (auto& p : *styles) {
                (*managedStyles)[std::move(p.first)] = std::move(p.second);
            }
            target_->updateStyles(page_id_, ref, managedStyles);
        }
        delete styles;
        return true;
    }
    
    bool RenderPageCustom::UpdateAttr(const std::string &ref, std::vector<std::pair<std::string, std::string>> *attrs) {
        if (target_) {
            std::shared_ptr<std::map<std::string, std::string>> managedAttrs = std::make_shared<std::map<std::string, std::string>>();
            for (auto& p : *attrs) {
                (*managedAttrs)[std::move(p.first)] = std::move(p.second);
            }
            target_->updateAttributes(page_id_, ref, managedAttrs);
        }
        delete attrs;
        return true;
    }
    
    bool RenderPageCustom::AddEvent(const std::string &ref, const std::string &event) {
        if (target_) {
            target_->addEvent(page_id_, ref, event);
        }
        return true;
    }
    
    bool RenderPageCustom::RemoveEvent(const std::string &ref, const std::string &event) {
        if (target_) {
            target_->removeEvent(page_id_, ref, event);
        }
        return true;
    }
    
    bool RenderPageCustom::CreateFinish() {
        if (target_) {
            target_->createFinish(page_id_);
        }
        SendCreateFinishAction();
        SendRenderSuccessAction();
        return true;
    }
    
    std::unique_ptr<ValueWithType> RenderPageCustom::CallNativeModule(const char *module, const char *method,
                                                                      const char *argumentsWson, int arguments_length, const char *optionsWson,
                                                                      int options_length) {
        // If render target can handle module method, we forward to render target.
        if (target_) {
            if (target_->shouldHandleModuleMethod(module, method)) {
                bool handled = false;
                //const char* will not assign, have some problem on android
                std::string argumentsJson;
                int jsonArgumentsLength = arguments_length;
                if (argumentsWson) {
                    wson_parser parser(argumentsWson, arguments_length);
                    argumentsJson = parser.toStringUTF8();
                    jsonArgumentsLength = (int)argumentsJson.length();
                }
                
                std::string jsonOptions;
                int jsonOptionsLength = options_length;
                if (optionsWson) {
                    wson_parser parser(optionsWson, options_length);
                    jsonOptions = parser.toStringUTF8();
                    jsonOptionsLength = (int)jsonOptions.length();
                }

                auto result = target_->callNativeModule(page_id_, module, method,
                                                        argumentsJson, jsonArgumentsLength,
                                                        jsonOptions, jsonOptionsLength, handled);
                if (handled) {
                    return result;
                }
            }
            // custom page cannot handle this module method
            return RenderPageBase::CallNativeModule(module, method, argumentsWson, arguments_length, optionsWson, options_length);
        }
        //None Render Target, Use Default CallNativeModule
        return RenderPageBase::CallNativeModule(module, method, argumentsWson, arguments_length, optionsWson, options_length);
    }
    
    void RenderPageCustom::CallNativeComponent(const char *ref, const char *method, const char *argumentsWson, int arguments_length,
                                               const char *optionsWson, int options_length) {
        if (target_) {
            std::string argumentsJson;
            int jsonArgumentsLength = arguments_length;
            if (argumentsWson) {
                wson_parser parser(argumentsWson, arguments_length);
                argumentsJson = parser.toStringUTF8();
                jsonArgumentsLength = (int)argumentsJson.length();
            }
            
            std::string jsonOptions;
            int jsonOptionsLength = options_length;
            if (optionsWson) {
                wson_parser parser(optionsWson, options_length);
                jsonOptions = parser.toStringUTF8();
                jsonOptionsLength = (int)jsonOptions.length();
            }
            
            target_->callNativeComponent(page_id_, ref, method, argumentsJson, jsonArgumentsLength, jsonOptions, jsonOptionsLength);
        }
        else {
            RenderPageBase::CallNativeComponent(ref, method, argumentsWson, arguments_length, optionsWson, options_length);
        }
    }
    
    // Life cycle
    void RenderPageCustom::SetDefaultHeightAndWidthIntoRootRender(const float default_width, const float default_height,
                                                                  const bool is_width_wrap_content, const bool is_height_wrap_content) {
        
    }
    
    void RenderPageCustom::OnRenderPageClose() {
        if (target_) {
            target_->deletePage(page_id_);
        }
    }
    
    bool RenderPageCustom::ReapplyStyles() {
        return false;
    }
    
    float RenderPageCustom::GetViewportWidth() {
        return 0;
    }
    
    void RenderPageCustom::SetViewportWidth(float value) {
        
    }
    
    bool RenderPageCustom::GetRoundOffDeviation() {
        return true;
    }
    
    void RenderPageCustom::SetRoundOffDeviation(bool value) {
        
    }
    
    float RenderPageCustom::GetDeviceWidth() {
        return 0;
    }
    
    void RenderPageCustom::SetDeviceWidth(float value) {
        
    }
    
}
