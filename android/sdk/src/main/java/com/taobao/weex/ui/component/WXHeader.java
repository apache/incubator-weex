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
package com.taobao.weex.ui.component;

import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.annotation.Component;
import com.taobao.weex.common.Constants;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.ui.component.list.WXCell;

/**
 * The same as sticky cell
 */
@Component(lazyload = false)
public class WXHeader extends WXCell {

  @Deprecated
  public WXHeader(WXSDKInstance instance, WXDomObject dom, WXVContainer parent, String instanceId, boolean isLazy) {
    this(instance,dom,parent,isLazy);
  }

  public WXHeader(WXSDKInstance instance, WXDomObject node, WXVContainer parent, boolean lazy) {
    super(instance, node, parent, lazy);
    String parantType = parent.getDomObject().getType();
    if(WXBasicComponentType.LIST.equals(parantType)
            || WXBasicComponentType.RECYCLE_LIST.equals(parantType)){
      setSticky(Constants.Value.STICKY);
    }
  }

  @Override
  public boolean isLazy() {
    return false;
  }

  @Override
  public boolean isSticky() {
    return true;
  }

  @Override
  public boolean canRecycled() {
    return false;
  }
}
