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
package com.taobao.weex.ui.prerenderaction;

import android.support.annotation.NonNull;

import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.performance.WXInstanceApm;
import com.taobao.weex.ui.action.BasicGraphicAction;

/**
 * Created by listen on 18/01/09.
 */
public class PrerenderGraphicActionCreateFinish extends BasicGraphicAction {

  PrerenderGraphicActionCreateFinish(@NonNull WXSDKInstance instance) {
    super(instance, "");
    instance.getApmForInstance().onStage(WXInstanceApm.KEY_PAGE_STAGES_CREATE_FINISH);
    instance.getApmForInstance().extInfo.put(WXInstanceApm.KEY_PAGE_STAGES_CREATE_FINISH,true);

  }

  @Override
  public void executeAction() {
    final WXSDKInstance instance = getWXSDKIntance();
    if (instance == null) {
      return;
    }

    if (instance.getPrerenderContext().rootNode != null) {
      instance.getPrerenderContext().rootNode.onCreateFinish();
    }

  }
}
