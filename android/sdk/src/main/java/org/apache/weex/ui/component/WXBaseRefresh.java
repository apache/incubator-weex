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

import android.content.Context;
import android.support.annotation.NonNull;
import org.apache.weex.WXSDKInstance;
import org.apache.weex.annotation.Component;
import org.apache.weex.ui.action.BasicComponentData;
import org.apache.weex.ui.view.WXFrameLayout;

/**
 * div component
 */
@Component(lazyload = false)

public class WXBaseRefresh extends WXVContainer<WXFrameLayout> {

  private WXLoadingIndicator mLoadingIndicator;

  public WXBaseRefresh(
      WXSDKInstance instance, WXVContainer parent, boolean lazy, BasicComponentData basicComponentData) {
    super(instance, parent, lazy, basicComponentData);
  }

  @Override
  public void addChild(WXComponent child) {
    super.addChild(child);
    this.checkLoadingIndicator(child);
  }

  @Override
  protected WXFrameLayout initComponentHostView(@NonNull Context context) {
    return new WXFrameLayout(context);
  }

  @Override
  public void addChild(WXComponent child, int index) {
    super.addChild(child, index);
    this.checkLoadingIndicator(child);
  }

  private void checkLoadingIndicator(WXComponent child) {
    if (child instanceof WXLoadingIndicator) {
      mLoadingIndicator = (WXLoadingIndicator) child;
    }
  }

}
