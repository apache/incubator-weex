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

#include "third_party/json11/json11.hpp"
#include "core/bridge/eagle_bridge.h"
#include "base/string_util.h"
#include "base/closure.h"
#include "core/render/node/render_object.h"
#include "core/render/node/factory/render_creator.h"
#include "core/render/manager/render_manager.h"
#include "core/manager/weex_core_manager.h"
#include "core/network/http_module.h"

namespace WeexCore {

EagleBridge* EagleBridge::g_instance = nullptr;// NOLINT(cert-err58-cpp)

void EagleBridge::CreatePage(const char* plugin_type,
                             const char* instanceId,
                             const char* func,
                             const char* script,
                             int script_length,
                             const char* opts,
                             const char* initData,
                             const char* extendsApi, std::function<void(const char*, const char*)> callback) {
  const auto& it = pages_.find(instanceId);
  if (it == pages_.end()){
    DataRenderHandler* plugin = get_data_render_handler(plugin_type);
    if (!plugin) {
      std::string err = std::string("EagleBridge plugin not exist: ") + plugin_type;
      weex_core_handler()->ReportException(instanceId, "CreateInstance", err.c_str());
      LOGE("%s", err.c_str());
      return;
    }

    pages_[instanceId] = plugin;
    plugin->CreatePage(instanceId, func, script, script_length, opts, initData, extendsApi, callback);
  } else {
    LOGE("ReCreate Page of InstanceId: %s", instanceId);
  }
}

EagleBridge::EagleBridge() : weex_core_handler_(new WeexCoreHandler()) {

}

EagleBridge::DataRenderHandler* EagleBridge::data_render_handler(const std::string& instance_id) const {
  const auto& it = pages_.find(instance_id);
  if (it == pages_.end()){
    return nullptr;
  } else {
    return it->second;
  }
}

void EagleBridge::set_data_render_handler(const std::string& plugin_type,
                                          DataRenderHandler* data_render_handler) {
  std::lock_guard<std::mutex> lock(plugin_mutex_);
  plugins_[plugin_type] = data_render_handler;
}

EagleBridge::DataRenderHandler* EagleBridge::get_data_render_handler(const std::string& plugin_type) {
  std::lock_guard<std::mutex> lock(plugin_mutex_);
  const auto& it = plugins_.find(plugin_type);
  if (it == plugins_.end()) {
    return nullptr;
  } else {
    return it->second;
  }
}

EagleBridge* EagleBridge::GetInstance() {
  static std::once_flag s_flag;
  std::call_once(s_flag, []() {
    g_instance = new EagleBridge();
  });

  return g_instance;
}

EagleModeReturn EagleBridge::DestroyPage(const char* instanceId) {
  const auto& it = pages_.find(instanceId);
  if (it == pages_.end()){
    return EagleModeReturn::NOT_EAGLE;
  } else {
    EagleModeReturn mode = it->second->DestroyInstance(instanceId);
    pages_.erase(it);
    return mode;
  }
}

EagleModeReturn EagleBridge::RefreshPage(const char* instanceId, const char* initData) {
  const auto& it = pages_.find(instanceId);
  if (it == pages_.end()){
    return EagleModeReturn::NOT_EAGLE;
  } else {
    return it->second->RefreshPage(instanceId, initData);;
  }
}

void EagleBridge::UpdateComponentData(const char* instanceId, const char* cid, const char* json_data) {
  const auto& it = pages_.find(instanceId);
  if (it == pages_.end()){
    weex_core_handler()->ReportException(
        instanceId, "UpdateComponentData",
        "There is no data_render_handler when UpdateComponentData invoked");
    return;
  } else {
    return it->second->UpdateComponentData(instanceId, cid, json_data);;
  }
}

bool EagleBridge::CallEagleTaskFromWeex(const char* instanceId,
                                        const char* task,
                                        const char* options) {
  if (instanceId){
    //a single instance task
    const auto& it = pages_.find(instanceId);
    if (it == pages_.end()){
      return false;
    } else {
      return it->second->CallEagleTaskFromWeex(instanceId, task, options);
    }
  } else {
    //a global task; call every plugins.
    bool support = false;
    for (const auto& it: plugins_){
      support = support || it.second->CallEagleTaskFromWeex(nullptr, task ,options);
    }
    return support;
  }
  return false;
}

EagleRenderObject::EagleRenderObject() : render_object_impl_(nullptr) {}

EagleRenderObject::EagleRenderObject(RenderObject* render_object) : render_object_impl_(
    render_object) {}

void EagleRenderObject::AddAttr(const std::string& key, const std::string& value) {
  render_object_impl_->AddAttr(key, value);
}

void EagleRenderObject::AddStyle(const std::string& key, const std::string& value) {
  if (render_object_impl_->is_richtext_child()) {
    render_object_impl_->MapInsertOrAssign(render_object_impl_->styles(), key, value);
  } else {
    render_object_impl_->AddStyle(key, value, false);
  }
}

void EagleRenderObject::UpdateAttr(const std::string& key, const std::string& value) {
  render_object_impl_->UpdateAttr(key, value);
}

void EagleRenderObject::UpdateStyle(const std::string& key, const std::string& value) {
  render_object_impl_->UpdateStyle(key, value);
}

void EagleRenderObject::AddEvent(const std::string& event) {
  render_object_impl_->AddEvent(event);
}

void EagleRenderObject::RemoveEvent(const std::string& event) {
  render_object_impl_->RemoveEvent(event);
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
  render_object_impl_->ApplyDefaultStyle(false);
}

int EagleRenderObject::AddRenderObject(int index, EagleRenderObject child) {
  if (!render_object_impl_) {
    return -1;
  }
  return render_object_impl_->AddRenderObject(index, child.render_object_impl_);
}

int EagleRenderObject::getChildCount() {
  return static_cast<int>(render_object_impl_->getChildCount());
}

int EagleRenderObject::getChildIndex(EagleRenderObject child) {
  return static_cast<int>(render_object_impl_->IndexOf(child.render_object_impl_));
}

EagleRenderObject EagleRenderObject::GetChild(int index) {
  return render_object_impl_->GetChild(index);
}

void EagleRenderObject::set_is_richtext_child(const bool is_richtext_child) {
  render_object_impl_->set_is_richtext_child(is_richtext_child);
}

EagleRenderObject EagleRenderObject::parent_render() {
  return render_object_impl_->parent_render();
}

const std::string& EagleRenderObject::page_id() {
  return render_object_impl_->page_id();
}

const std::string& EagleRenderObject::ref() {
  return render_object_impl_->ref();
}

void EagleRenderObject::RemoveRenderObject(EagleRenderObject child) {
  render_object_impl_->RemoveRenderObject(child.render_object_impl_);
}

EagleRenderObject EagleBridge::WeexCoreHandler::GetEagleRenderObject(const std::string& type,
                                                                     const std::string& ref) {
  return EagleRenderObject(
      static_cast<WeexCore::RenderObject*>(WeexCore::RenderCreator::GetInstance()->CreateRender(
          type, ref)));
}

bool EagleBridge::WeexCoreHandler::CreatePage(const std::string& page_id, EagleRenderObject root) {
  return RenderManager::GetInstance()->CreatePage(page_id, root.render_object_impl_);
}

bool EagleBridge::WeexCoreHandler::HavePage(const std::string& page_id) {
  return RenderManager::GetInstance()->GetPage(page_id) ? true : false;
}

bool EagleBridge::WeexCoreHandler::CreateFinish(const std::string& page_id) {
  return RenderManager::GetInstance()->CreateFinish(page_id);
}

bool EagleBridge::WeexCoreHandler::ClosePage(const std::string& page_id) {
  return RenderManager::GetInstance()->ClosePage(page_id);
}

void EagleBridge::WeexCoreHandler::ReportException(const char* page_id,
                                                   const char* func,
                                                   const char* exception_string) {
  WeexCore::WeexCoreManager::Instance()->getPlatformBridge()->platform_side()->ReportException(
      page_id, func, exception_string);
}

void EagleBridge::WeexCoreHandler::Send(const char* instance_id,
                                        const char* url,
                                        std::function<void(const std::string&,
                                                           const std::string&)> callback) {
  weex::core::network::HttpModule http_module;
  http_module.Send(instance_id, url, callback);
}

void EagleBridge::WeexCoreHandler::GetBundleType(const char* instance_id,
                                                 const char* content,
                                                 std::function<void(const std::string&,
                                                                    const std::string&)> callback) {
  weex::core::network::HttpModule http_module;
  http_module.GetBundleType(instance_id, content, callback);
}

//void EagleBridge::WeexCoreHandler::RequrieMoudle(const std::string& class_name) {
//}

//void EagleBridge::WeexCoreHandler::RequrieComponent(const std::string& class_name) {
//}

bool EagleBridge::WeexCoreHandler::RefreshFinish(const char* page_id,
                                                 const char* task,
                                                 const char* callback) {
  WeexCore::WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->core_side()->ForceLayout(page_id);
  return static_cast<bool>(WeexCore::WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->RefreshFinish(page_id, task, callback));
}

//return empty for undefined
static std::string ParsePlatformResultToString(const std::unique_ptr<ValueWithType>& result) {
  switch (result->type) {
    case ParamsType::DOUBLE:
      return json11::Json(result->value.doubleValue).dump();
    case ParamsType::STRING: {
      if (!result->value.string) {
        return "";
      } else {
        std::string ret;
        if (result->value.string->length > 0) {
          const auto& basic_string = weex::base::to_utf8(result->value.string->content,
                                                         result->value.string->length);
          ret = json11::Json(basic_string).dump();
        } else {
          ret = json11::Json("").dump();
        }
        free(result->value.string);
        return ret;
      }
    }
    case ParamsType::BYTEARRAYSTRING: {
      if (!result->value.byteArray) {
        return "";//null
      } else {
        std::string ret;
        if (result->value.byteArray->length > 0) {
          ret = json11::Json(result->value.byteArray->content).dump();
        } else {
          ret = json11::Json("").dump();
        }
        free(result->value.byteArray);
        return ret;
      }
    }
    case ParamsType::JSONSTRING: {
      if (!result->value.string) {
        return "";
      } else {
        std::string ret;
        std::string err;
        if (result->value.string->length > 0) {
          const auto& raw_str = weex::base::to_utf8(result->value.string->content,
                                                    result->value.string->length);

          const json11::Json& json = json11::Json::parse(raw_str, err);
          if (err.empty()) {
            //succ
            ret = json.dump();
          } else {
            LOGE("VnodeManager CallNative return value to object err, %s", err.c_str());
          }
        }
        free(result->value.string);
        return ret;
      }
    }
    case ParamsType::BYTEARRAYJSONSTRING: {
      if (!result->value.byteArray) {
        return "";
      } else {
        std::string ret;
        std::string err;
        if (result->value.string->length > 0) {
          const json11::Json& json = json11::Json::parse(result->value.byteArray->content, err);
          if (err.empty()) {
            //succ
            ret = json.dump();
          } else {
            LOGE("VnodeManager CallNative return value to object err, %s", err.c_str());
          }
        }
        free(result->value.byteArray);
        return ret;
      }
    }
    case ParamsType::BYTEARRAY:
    default:
      return "";
  }
}

std::string EagleBridge::WeexCoreHandler::CallNativeModule(const char* page_id,
                                                           const char* module,
                                                           const char* method,
                                                           const char* arguments,
                                                           int arguments_length,
                                                           const char* options,
                                                           int options_length) {
  auto ptr = WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->CallNativeModule(page_id, module, method, arguments, arguments_length, options,
                         options_length);
  return ParsePlatformResultToString(ptr);
}

#if OS_IOS
void EagleBridge::WeexCoreHandler::PostTaskOnComponentThread(const weex::base::Closure& closure) {
    WeexCoreManager::Instance()->getPlatformBridge()->platform_side()->PostTaskOnComponentThread(closure);
}
#endif

void EagleBridge::WeexCoreHandler::CallNativeComponent(const char* page_id,
                                                       const char* module,
                                                       const char* method,
                                                       const char* arguments,
                                                       int arguments_length,
                                                       const char* options,
                                                       int options_length) {
  WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->CallNativeComponent(page_id, module, method, arguments, arguments_length, options,
                            options_length);
}

void EagleBridge::WeexCoreHandler::NativeLog(const char* str_array) {
  WeexCoreManager::Instance()->getPlatformBridge()->platform_side()->NativeLog(str_array);
}

bool EagleBridge::WeexCoreHandler::RemoveRenderObject(const std::string& page_id,
                                                      const std::string& ref) {
  return RenderManager::GetInstance()->RemoveRenderObject(page_id, ref);
}

bool EagleBridge::WeexCoreHandler::AddRenderObject(const std::string& page_id,
                                                   const std::string& parent_ref,
                                                   int index,
                                                   EagleRenderObject root) {
  return RenderManager::GetInstance()->AddRenderObject(page_id, parent_ref, index,
                                                       root.render_object_impl_);
}

bool EagleBridge::WeexCoreHandler::MoveRenderObject(const std::string& page_id,
                                                    const std::string& ref,
                                                    const std::string& parent_ref,
                                                    int index) {
  return RenderManager::GetInstance()->MoveRenderObject(page_id, ref, parent_ref, index);
}

bool EagleBridge::WeexCoreHandler::RemoveEvent(const std::string& page_id, const std::string& ref,
                                               const std::string& event) {
  return RenderManager::GetInstance()->RemoveEvent(page_id, ref, event);
}

bool EagleBridge::WeexCoreHandler::AddEvent(const std::string& page_id, const std::string& ref,
                                            const std::string& event) {
  return RenderManager::GetInstance()->AddEvent(page_id, ref, event);
}

bool EagleBridge::WeexCoreHandler::UpdateAttr(const std::string& page_id, const std::string& ref,
                                              const char* data) {
  return RenderManager::GetInstance()->UpdateAttr(page_id, ref, data);
}

bool EagleBridge::WeexCoreHandler::UpdateAttr(const std::string& page_id, const std::string& ref,
                                              std::vector<std::pair<std::string, std::string>>* attrPair) {
  return RenderManager::GetInstance()->UpdateAttr(page_id, ref, attrPair);
}

bool EagleBridge::WeexCoreHandler::UpdateStyle(const std::string& page_id, const std::string& ref,
                                               const char* data) {
  return RenderManager::GetInstance()->UpdateStyle(page_id, ref, data);
}

bool EagleBridge::WeexCoreHandler::UpdateStyle(const std::string& page_id, const std::string& ref,
                                               std::vector<std::pair<std::string, std::string>>* stylePair) {
  return RenderManager::GetInstance()->UpdateStyle(page_id, ref, stylePair);
}

int EagleBridge::WeexCoreHandler::ExecJS(const char* instanceId, const char* nameSpace,
                                         const char* func,
                                         std::vector<ValueWithType*>& params) {
  return WeexCore::WeexCoreManager::Instance()->script_bridge()->script_side()->ExecJS(
      instanceId,
      nameSpace,
      func,
      params
  );
}

//FIXME Please don't call this method, which will cause downgrade of Weex.
void EagleBridge::WeexCoreHandler::PostTaskToMsgLoop(const weex::base::Closure& closure) {
  WeexCore::WeexCoreManager::Instance()->getPlatformBridge()->platform_side()->ReportException(
      "", "PostTaskToMsgLoop",
      "PostTaskToMsgLoop is not supported anymore, please update to the latest version of Weex.");
}

bool EagleBridge::WeexCoreHandler::CallWeexTaskFromEagle(const std::string& type,
                                                         const std::string& task,
                                                         const std::string& options) {
  //No Extend Method now.
  return false;
}
}

