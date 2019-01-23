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
package com.taobao.weex.ui.action;

import android.support.annotation.NonNull;

import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.ui.component.node.WXComponentNode;

import java.util.Map;
import java.util.Set;

public class GraphicActionCreateBody extends GraphicActionAbstractAddElement {

  private WXComponentNode node;

  public GraphicActionCreateBody(@NonNull WXSDKInstance instance, String ref,
                                 String componentType,
                                 Map<String, String> style,
                                 Map<String, String> attributes,
                                 Set<String> events,
                                 float[] margins,
                                 float[] paddings,
                                 float[] borders) {
    super(instance, ref);
    this.mComponentType = componentType;
    this.mStyle = style;
    this.mAttributes = attributes;
    this.mEvents = events;
    this.mMargins = margins;
    this.mPaddings = paddings;
    this.mBorders = borders;

    BasicComponentData basicComponentData = new BasicComponentData(getRef(), mComponentType, null);
    node = createNode(instance, basicComponentData).build();
    node.createComponent();
  }

  @Override
  public void executeAction() {
    super.executeAction();
    if (node != null) {
      node.createBody();
    }
  }
}
