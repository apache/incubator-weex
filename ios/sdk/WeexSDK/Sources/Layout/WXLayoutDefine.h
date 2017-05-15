/*
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


#define WX_LAYOUT_NAMESPACE wx_

#ifdef WX_LAYOUT_NAMESPACE
    // prefix all layout symbols with "wx_" to prevent conflict
    #define WX_NAMESPACE_PREFIX_INNER(namespace, symbol) namespace ## symbol
    #define WX_NAMESPACE_PREFIX(namespace, symbol) WX_NAMESPACE_PREFIX_INNER(namespace, symbol)
    #define WX_LAYOUT_PREFIX(symbol) WX_NAMESPACE_PREFIX(WX_LAYOUT_NAMESPACE, symbol)

    #define css_direction_t                WX_LAYOUT_PREFIX(css_direction_t)
    #define css_flex_direction_t           WX_LAYOUT_PREFIX(css_flex_direction_t)
    #define css_justify_t                  WX_LAYOUT_PREFIX(css_justify_t)
    #define css_align_t                    WX_LAYOUT_PREFIX(css_align_t)
    #define css_position_type_t            WX_LAYOUT_PREFIX(css_position_type_t)
    #define css_wrap_type_t                WX_LAYOUT_PREFIX(css_wrap_type_t)
    #define css_position_t                 WX_LAYOUT_PREFIX(css_position_t)
    #define css_dimension_t                WX_LAYOUT_PREFIX(css_dimension_t)
    #define css_layout_t                   WX_LAYOUT_PREFIX(css_layout_t)
    #define css_dim_t                      WX_LAYOUT_PREFIX(css_dim_t)
    #define css_style_t                    WX_LAYOUT_PREFIX(css_style_t)
    #define css_node                       WX_LAYOUT_PREFIX(css_node)
    #define css_node_t                     WX_LAYOUT_PREFIX(css_node_t)
    #define new_css_node                   WX_LAYOUT_PREFIX(new_css_node)
    #define init_css_node                  WX_LAYOUT_PREFIX(init_css_node)
    #define free_css_node                  WX_LAYOUT_PREFIX(free_css_node)
    #define css_print_options_t            WX_LAYOUT_PREFIX(css_print_options_t)
    #define print_css_node                 WX_LAYOUT_PREFIX(print_css_node)
    #define layoutNode                     WX_LAYOUT_PREFIX(layoutNode)
    #define isUndefined                    WX_LAYOUT_PREFIX(isUndefined)
    #define resetNodeLayout                WX_LAYOUT_PREFIX(resetNodeLayout)

#endif


#import "Layout.h"



