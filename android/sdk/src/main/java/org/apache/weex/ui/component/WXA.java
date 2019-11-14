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
package org.apache.weex.ui.component;

import org.apache.weex.WXSDKInstance;
import org.apache.weex.annotation.Component;
import org.apache.weex.common.Constants;
import org.apache.weex.dom.WXAttr;
import org.apache.weex.ui.action.BasicComponentData;
import org.apache.weex.ui.view.WXFrameLayout;
import org.apache.weex.utils.ATagUtil;

@Component(lazyload = false)
public class WXA extends WXDiv {

  @Deprecated
  public WXA(WXSDKInstance instance, WXVContainer parent, String instanceId, boolean isLazy, BasicComponentData basicComponentData) {
    this(instance, parent, basicComponentData);
  }

  public WXA(WXSDKInstance instance, WXVContainer parent, BasicComponentData basicComponentData) {
    super(instance, parent, basicComponentData);
  }

  @Override
  protected void onHostViewInitialized(WXFrameLayout host) {
    addClickListener(new OnClickListener() {
      @Override
      public void onHostViewClick() {
        String href;
        WXAttr attr = getAttrs();
        if (attr !=null && (href = (String)attr.get("href")) != null) {
          ATagUtil.onClick(null, getInstanceId(), href);
        }
      }
    });
    super.onHostViewInitialized(host);
  }

  @Override
  protected boolean setProperty(String key, Object param) {
    switch(key){
      case Constants.Name.HREF:
        return true;
    }
    return super.setProperty(key, param);
  }
}
