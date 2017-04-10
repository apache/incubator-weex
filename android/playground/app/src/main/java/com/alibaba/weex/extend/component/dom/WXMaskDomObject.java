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
package com.alibaba.weex.extend.component.dom;

import com.taobao.weex.common.Constants;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.utils.WXViewUtils;

import java.util.HashMap;
import java.util.Map;

/**
 * Created by lixinke on 2016/12/28.
 */

public class WXMaskDomObject extends WXDomObject {

  @Override
  protected Map<String, String> getDefaultStyle() {
    Map<String,String> styles=new HashMap<>();
    styles.put(Constants.Name.POSITION, "absolute");
    int width=WXViewUtils.getScreenWidth(getDomContext().getUIContext());
    styles.put(Constants.Name.WIDTH, String.valueOf(WXViewUtils.getWebPxByWidth(width, getViewPortWidth())));
    int height=WXViewUtils.getScreenHeight(getDomContext().getUIContext());
    styles.put(Constants.Name.HEIGHT, String.valueOf(WXViewUtils.getWebPxByWidth(height,getViewPortWidth())));
    styles.put(Constants.Name.TOP, "0");
    return styles;
  }
}
