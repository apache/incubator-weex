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
#include "core/render/node/factory/render_creator.h"
#include "core/render/node/factory/render_appbar_factory.h"
#include "core/render/node/factory/render_factory_interface.h"
#include "core/render/node/factory/render_list_factory.h"
#include "core/render/node/factory/render_mask_factory.h"
#include "core/render/node/factory/render_object_interface.h"
#include "core/render/node/factory/render_scroller_factory.h"
#include "core/render/node/factory/render_text_factory.h"
#include "core/render/node/factory/render_type.h"
#include "core/render/node/factory/simple_render_factory.h"

namespace WeexCore {

RenderCreator *RenderCreator::g_pInstance = nullptr;

IRenderObject *RenderCreator::CreateRender(const std::string &type,
                                           const std::string &ref) {
  IRenderFactory *factory;
  if (type == kRenderText) {
    factory = new RenderTextFactory();
  } else if (type == kRenderList || type == kRenderWaterfall ||
             type == kRenderRecycleList) {
    factory = new RenderListFactory();
  } else if (type == kRenderMask) {
    factory = new RenderMaskFactory();
  } else if (type == kRenderScroller) {
    factory = new RenderScrollerFactory();
  } else if (type == kRenderAppBar) {
    factory = new RenderAppBarFactory();
  } else {
    factory = new RenderCommonFactory();
  }

  IRenderObject *render = factory->CreateRender();
  render->set_ref(ref);
  render->set_type(type);
  delete factory;
  return render;
}
}  // namespace WeexCore
