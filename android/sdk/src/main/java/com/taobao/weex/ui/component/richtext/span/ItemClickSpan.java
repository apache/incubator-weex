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
package com.taobao.weex.ui.component.richtext.span;


import android.text.style.ClickableSpan;
import android.view.View;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.ui.component.richtext.node.RichTextNode;
import com.taobao.weex.utils.WXDataStructureUtil;
import java.util.Map;

public class ItemClickSpan extends ClickableSpan {

  private final String mPseudoRef;
  private final String mInstanceId;
  private final String mComponentRef;

  public ItemClickSpan(String instanceId, String componentRef, String pseudoRef) {
    this.mPseudoRef = pseudoRef;
    this.mInstanceId = instanceId;
    this.mComponentRef = componentRef;
  }

  @Override
  public void onClick(View widget) {
    WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(mInstanceId);
    if (instance != null && !instance.isDestroy()) {
      Map<String, Object> param = WXDataStructureUtil.newHashMapWithExpectedSize(1);
      param.put(RichTextNode.PSEUDO_REF, mPseudoRef);
      instance.fireEvent(mComponentRef, RichTextNode.ITEM_CLICK, param);
    }
  }
}
