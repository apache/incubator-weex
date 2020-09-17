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

#include "core/render/page/reactor_page.h"

#include "third_party/json11/json11.hpp"
#include "base/string_util.h"
#include "core/render/manager/render_manager.h"
#include "core/render/page/render_page.h"
#include "core/render/node/factory/render_creator.h"
#include "core/manager/weex_core_manager.h"

namespace WeexCore {

void PostTaskOnComponentThread(const std::function<void()>& callback) {
#if OS_IOS
     WeexCoreManager::Instance()->getPlatformBridge()->platform_side()->PostTaskOnComponentThread(callback);
#else
    callback();
#endif
}

static std::string ResultToString(const std::unique_ptr<ValueWithType>& result) {
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

void ReactorPage::CreateBody(const std::string& ref,
                             const std::string& type,
                             const std::map<std::string, std::string>& styles,
                             const std::map<std::string, std::string>& attrs,
                             const std::vector<std::string>& events) {
    RenderManager::GetInstance()->CreatePage(page_id_, [&] (RenderPage* page_instance) -> RenderObject* {
#if OS_IOS
        page_instance->set_before_layout_needed(false); // do not need before and after layout on ios platform
        page_instance->set_after_layout_needed(false);
        page_instance->set_platform_layout_needed(true);
#endif
        return CreateRenderObject(ref, type, 0, styles, attrs, events, page_instance->reserve_css_styles(), nullptr);
    });
    
}

void ReactorPage::AddElement(const std::string& ref,
                             const std::string& type,
                             const std::map<std::string, std::string>& styles,
                             const std::map<std::string, std::string>& attrs,
                             const std::vector<std::string>& events,
                             const std::string parent_ref, int index) {
    RenderManager::GetInstance()->AddRenderObject(page_id_, parent_ref, index, [&] (RenderPage* page_instance) -> RenderObject* {
        return CreateRenderObject(ref, type, 0, styles, attrs, events, page_instance->reserve_css_styles(), nullptr);
     });
}

void ReactorPage::UpdateStyles(const std::string& ref, std::vector<std::pair<std::string, std::string>> styles) {
     WeexCore::RenderManager::GetInstance()->UpdateStyle(page_id_, ref, &styles);
}

void ReactorPage::UpdateAttrs(const std::string& ref, std::vector<std::pair<std::string, std::string>> attrs) {
     WeexCore::RenderManager::GetInstance()->UpdateAttr(page_id_, ref, &attrs);
}

void ReactorPage::CreateFinish() {
    WeexCore::WeexCoreManager::Instance()->script_bridge()->core_side()->CreateFinish(page_id_.c_str());
}

void ReactorPage::RemoveElement(const std::string& ref) {
    WeexCore::WeexCoreManager::Instance()->script_bridge()->core_side()->RemoveElement(page_id_.c_str(), ref.c_str());
}

std::string ReactorPage::CallNativeModule(const std::string& module,
                                   const std::string& method,
                                   const std::string& arguments,
                                   size_t arguments_length,
                                   const std::string& options,
                                   int options_length) {
  std::unique_ptr<ValueWithType> ptr = WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->CallNativeModule(page_id_.c_str(), module.c_str(), method.c_str(), arguments.c_str(), static_cast<int>(arguments_length), options.c_str(), options_length);

  return ResultToString(ptr);
}

void ReactorPage::CallNativeComponent(const std::string& ref,
                                      const std::string& method,
                                      const std::string& arguments,
                                      size_t arguments_length,
                                      const std::string& options,
                                      int options_length) {
    WeexCoreManager::Instance()->getPlatformBridge()
    ->platform_side()->CallNativeComponent(page_id_.c_str(),
                                           ref.c_str(),
                                           method.c_str(),
                                           arguments.c_str(),
                                           static_cast<int>(arguments_length),
                                           options.c_str(),
                                           options_length);
}

RenderObject* ReactorPage::CreateRenderObject(const std::string& ref,
                                              const std::string& type,
                                              unsigned index,
                                              const std::map<std::string, std::string>& styles,
                                              const std::map<std::string, std::string>& attrs,
                                              const std::vector<std::string>& events,
                                              bool reserve_styles,
                                              WeexCore::RenderObject* parent) {
    if (ref.empty() || type.empty()) {
        return nullptr;
    }

    RenderObject* render_object = (RenderObject *)RenderCreator::GetInstance()->CreateRender(type, ref);
    render_object->set_page_id(page_id_);
    if (parent) {
        parent->AddRenderObject(index, render_object);
    }

    //fill attributes, styles and events
    for (const auto& attr : attrs) {
        render_object->AddAttr(attr.first, attr.second);
    }
    for (const auto& style : styles) {
        render_object->AddStyle(style.first, style.second, reserve_styles);
    }
    for (const auto& event : events) {
        render_object->AddEvent(event);
    }

    render_object->ApplyDefaultStyle(reserve_styles);
    render_object->ApplyDefaultAttr();
    return render_object;
}

}  // namespace WeexCore

