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
#ifndef CORE_RENDER_NODE_FACTORY_RENDER_TYPE_H_
#define CORE_RENDER_NODE_FACTORY_RENDER_TYPE_H_

#include <string>

namespace WeexCore {
constexpr char kRenderCell[] = "cell";
constexpr char kRenderCellSlot[] = "cell-slot";
constexpr char kRenderList[] = "list";
constexpr char kHList[] = "hlist";
constexpr char kRenderRecycleList[] = "recycle-list";
constexpr char kRenderMask[] = "mask";
constexpr char kRenderScroller[] = "scroller";
constexpr char kRenderText[] = "text";
constexpr char kRenderHeader[] = "header";
constexpr char kRenderFooter[] = "footer";
constexpr char kRenderWaterfall[] = "waterfall";
constexpr char kRenderAppBar[] = "appbar";
}  // namespace WeexCore

#endif  // CORE_RENDER_NODE_FACTORY_RENDER_TYPE_H_
