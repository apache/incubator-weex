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

#include "core/bridge/eagle_bridge.h"

#include "core/render/node/render_object.h"
#include "core/render/node/factory/render_creator.h"
#include "core/render/manager/render_manager.h"
#include "core/manager/weex_core_manager.h"
#include "core/network/http_module.h"

namespace WeexCore {
    EagleBridge* EagleBridge::g_instance = nullptr;
    
    EagleRenderObject::EagleRenderObject(RenderObject* render_object):render_object_impl_(render_object) {}
    
    void EagleRenderObject::AddAttr(std::string key, std::string value) {
        render_object_impl_->AddAttr(key, value);
    }
    
    void EagleRenderObject::AddStyle(std::string key, std::string value) {
        render_object_impl_->AddStyle(key, value);
    }
    
    void EagleRenderObject::AddEvent(std::string event) {
        render_object_impl_->AddEvent(event);
    }
    
    std::set<std::string>* EagleRenderObject::events() {
        return render_object_impl_->events();
    }
    
    void EagleRenderObject::set_page_id(const std::string& page_id) {
        render_object_impl_->set_page_id(page_id);
    }
    
    void EagleRenderObject::ApplyDefaultAttr() {
        render_object_impl_->ApplyDefaultAttr();
    }
    
    void EagleRenderObject::ApplyDefaultStyle() {
        render_object_impl_->ApplyDefaultStyle();
    }
    
    int EagleRenderObject::AddRenderObject(int index, RenderObject *child) {
        return render_object_impl_->AddRenderObject(index, child);
    }
    
    EagleRenderObject EagleBridge::WeexCoreHandler::GetEagleRenderObject(const std::string &type, const std::string &ref) {
        return EagleRenderObject(static_cast<WeexCore::RenderObject*>(WeexCore::RenderCreator::GetInstance()->CreateRender(type, ref)));
    }
    
    bool EagleBridge::WeexCoreHandler::CreatePage(const std::string& page_id, RenderObject *root) {
        return RenderManager::GetInstance()->CreatePage(page_id, root);
    }

    RenderPage* EagleBridge::WeexCoreHandler::GetPage(const std::string& page_id) {
        return RenderManager::GetInstance()->GetPage(page_id);
    }
    
    bool EagleBridge::WeexCoreHandler::CreateFinish(const std::string &page_id) {
        return RenderManager::GetInstance()->CreateFinish(page_id);
    }
    
    bool EagleBridge::WeexCoreHandler::ClosePage(const std::string &page_id) {
        return RenderManager::GetInstance()->ClosePage(page_id);
    }
    
    void EagleBridge::WeexCoreHandler::ReportException(const char* page_id, const char* func, const char* exception_string) {
        WeexCore::WeexCoreManager::Instance()->getPlatformBridge()->platform_side()->ReportException(page_id, nullptr, exception_string);
    }
    
    void EagleBridge::WeexCoreHandler::Send(const char* instance_id, const char* url, std::function<void(const std::string&)> callback) {
        weex::core::network::HttpModule http_module;
        http_module.Send(instance_id, url, callback);
    }
    
    int EagleBridge::WeexCoreHandler::RefreshFinish(const char* page_id, const char* task, const char* callback) {
        return WeexCore::WeexCoreManager::Instance()
        ->getPlatformBridge()
        ->platform_side()
        ->RefreshFinish(page_id, task, callback);
    }
    
    std::unique_ptr<ValueWithType> EagleBridge::WeexCoreHandler::CallNativeModule(const char* page_id, const char* module, const char* method,const char* arguments, int arguments_length, const char* options, int options_length) {
        return WeexCoreManager::Instance()
        ->getPlatformBridge()
        ->platform_side()
        ->CallNativeModule(page_id, module, method, arguments, arguments_length, options, options_length);
    }
    
    void EagleBridge::WeexCoreHandler::NativeLog(const char* str_array) {
        WeexCoreManager::Instance()->getPlatformBridge()->platform_side()->NativeLog(str_array);
    }
    
    bool EagleBridge::WeexCoreHandler::RemoveRenderObject(const std::string &page_id, const std::string &ref) {
        return RenderManager::GetInstance()->RemoveRenderObject(page_id, ref);
    }
    
    bool EagleBridge::WeexCoreHandler::AddRenderObject(const std::string &page_id, const std::string &parent_ref, int index,RenderObject *root) {
        return RenderManager::GetInstance()->AddRenderObject(page_id, parent_ref, index, root);
    }
    
    bool EagleBridge::WeexCoreHandler::MoveRenderObject(const std::string &page_id, const std::string &ref,const std::string &parent_ref, int index) {
        return RenderManager::GetInstance()->MoveRenderObject(page_id, ref, parent_ref, index);
    }
    
    bool EagleBridge::WeexCoreHandler::RemoveEvent(const std::string &page_id, const std::string &ref,
                                  const std::string &event) {
        return RenderManager::GetInstance()->RemoveEvent(page_id, ref, event);
    }
    
    bool EagleBridge::WeexCoreHandler::AddEvent(const std::string &page_id, const std::string &ref,
                               const std::string &event) {
        return RenderManager::GetInstance()->AddEvent(page_id, ref, event);
    }
    
    bool EagleBridge::WeexCoreHandler::UpdateAttr(const std::string &page_id, const std::string &ref,
                    const char *data) {
        return RenderManager::GetInstance()->UpdateAttr(page_id, ref, data);
    }
    
    bool EagleBridge::WeexCoreHandler::UpdateAttr(const std::string &page_id, const std::string &ref,
                                 std::vector<std::pair<std::string, std::string>> *attrPair) {
        return RenderManager::GetInstance()->UpdateAttr(page_id, ref, attrPair);
    }
    
    bool EagleBridge::WeexCoreHandler::UpdateStyle(const std::string &page_id, const std::string &ref,
                                  const char *data) {
        return RenderManager::GetInstance()->UpdateStyle(page_id, ref, data);
    }
    
    bool EagleBridge::WeexCoreHandler::UpdateStyle(const std::string &page_id, const std::string &ref,
                                  std::vector<std::pair<std::string, std::string>> *stylePair) {
        return RenderManager::GetInstance()->UpdateStyle(page_id, ref, stylePair);
    }
    
    int EagleBridge::DataRenderHandler::DestroyInstance(const char *instanceId) {
        return WeexCoreManager::Instance()
        ->script_bridge()
        ->script_side()
        ->DestroyInstance(instanceId);
    }
}

