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
package com.taobao.weex.adapter;

import android.content.Context;
import android.view.View;

import com.taobao.weex.ui.component.html.HtmlComponent;
import com.taobao.weex.ui.component.html.WxHtmlComponent;

/** Created by Bruce Too On 2019/1/10. At 09:39 */
public interface IWxHtmlTagAdapter<T extends View> {
  /**
   * Custom html tags mapped with native view,see the default implement at {@link
   * com.taobao.weex.ui.component.html.adapter.DefaultHtmlTagAdapter}
   *
   * @param context current activity context
   * @param tagType tag's type define in {@link HtmlComponent.ViewType}
   * @param html html string
   * @return the custom native view self
   */
  public T getHtmlTagView(
      Context context, WxHtmlComponent component, @HtmlComponent.ViewType int tagType, String html);

  public View.OnClickListener getTagViewClickListener(
      @HtmlComponent.ViewType int tagType, String html);
}
