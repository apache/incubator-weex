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
package com.taobao.weex.dom;

import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.ui.component.WXComponent;

import java.util.Map;

/**
 * DOM operation context, use as compatible helper temporary.
 * Created by sospartan on 14/02/2017.
 */

public interface DOMActionContext {
  String getInstanceId();

  /**
   * For compatible, remove soon.
   **/
  WXDomObject.Consumer getAddDOMConsumer();

  WXDomObject.Consumer getApplyStyleConsumer();

  @Deprecated
  void addDomInfo(String ref, WXComponent component);

  void addAnimationForElement(String ref, Map<String, Object> animMap);

  /**
   * Post a Render task
   *
   * @param statement
   */
  void postRenderTask(RenderAction statement);

  void registerDOMObject(String ref, WXDomObject ojb);

  void unregisterDOMObject(String ref);

  void registerComponent(String ref, WXComponent comp);

  WXComponent getCompByRef(String ref);

  boolean isDestory();

  void markDirty();

  WXSDKInstance getInstance();

  WXDomObject getDomByRef(String ref);

  @Deprecated
  WXDomObject.Consumer getRemoveElementConsumer();
}
