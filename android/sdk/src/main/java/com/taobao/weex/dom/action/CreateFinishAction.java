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
package com.taobao.weex.dom.action;

import com.taobao.weex.LayoutFinishListener;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.common.WXRenderStrategy;
import com.taobao.weex.common.WXThread;
import com.taobao.weex.dom.DOMActionContext;
import com.taobao.weex.dom.RenderActionContext;

/**
 * Created by sospartan on 02/03/2017.
 */

final class CreateFinishAction extends AbstractLayoutFinishAction {

  @Override
  public void executeDom(DOMActionContext context) {
    super.executeDom(context);
    final WXSDKInstance instance = context.getInstance();
    final LayoutFinishListener listener;
    if(instance != null && (listener = instance.getLayoutFinishListener()) != null) {
      WXSDKManager.getInstance().getWXRenderManager().postOnUiThread(WXThread.secure(new Runnable() {
        @Override
        public void run() {
          listener.onLayoutFinish(instance);
        }
      }),0);
    }
  }

  @Override
  public void executeRender(RenderActionContext context) {
    WXSDKInstance instance = context.getInstance();
    if (instance.getRenderStrategy() == WXRenderStrategy.APPEND_ONCE) {
      instance.onCreateFinish();
    }
    instance.onRenderSuccess(mLayoutWidth, mLayoutHeight);
  }
}
