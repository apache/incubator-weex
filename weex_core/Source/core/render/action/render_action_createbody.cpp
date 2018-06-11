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

#include "core/render/node/render_object.h"
#include "core/manager/weex_core_manager.h"
#include "core/render/action/render_action_createbody.h"

namespace WeexCore {

  RenderActionCreateBody::RenderActionCreateBody(const std::string &page_id,
                                                 const RenderObject *render) {
    this->attributes = render->Attributes();
    this->styles = render->Styles();
    this->events = render->Events();
    this->margins = render->GetMargins();
    this->paddings = render->GetPaddings();
    this->borders = render->GetBorders();
    this->page_id = page_id;
    this->component_type = render->Type();
    this->ref = render->Ref();
  }

  void RenderActionCreateBody::ExecuteAction() {
    WeexCoreManager::getInstance()->getPlatformBridge()->callCreateBody(this->page_id.c_str(),
                                                                        this->component_type.c_str(),
                                                                        this->ref.c_str(),
                                                                        this->styles,
                                                                        this->attributes,
                                                                        this->events,
                                                                        this->margins,
                                                                        this->paddings,
                                                                        this->borders);
  }
}
