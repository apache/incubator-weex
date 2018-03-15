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

import com.taobao.weex.WXSDKManager;
import com.taobao.weex.dom.WXEvent;
import com.taobao.weex.tracing.Stopwatch;
import com.taobao.weex.ui.component.WXComponent;

/**
 * Created by listen on 18/01/11.
 */
public class GraphicActionAddEvent extends BasicGraphicAction {

  private final String mEvent;

  public GraphicActionAddEvent(String pageId, String ref, Object event) {
    super(pageId, ref);
    this.mEvent = WXEvent.getEventName(event);
  }

  @Override
  public void executeAction() {
    WXComponent component = WXSDKManager.getInstance().getWXRenderManager().getWXComponent(getPageId(), getRef());
    if (component == null) {
      return;
    }

    Stopwatch.tick();
    if (!component.getEvents().contains(mEvent)) {
      component.getEvents().addEvent(mEvent);
    }
    component.addEvent(mEvent);
    Stopwatch.split("addEventToComponent");

  }
}
