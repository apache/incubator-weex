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
package org.apache.weex.ui.action;

import android.support.annotation.NonNull;
import org.apache.weex.WXSDKInstance;
import org.apache.weex.common.RenderTypes;
import org.apache.weex.common.WXRenderStrategy;
import org.apache.weex.performance.WXInstanceApm;
import org.apache.weex.ui.component.WXComponent;

/**
 * Created by listen on 18/01/09.
 */
public class GraphicActionCreateFinish extends BasicGraphicAction {

  private int mLayoutWidth;
  private int mLayoutHeight;

  public GraphicActionCreateFinish(@NonNull WXSDKInstance instance) {
    super(instance, "");
    WXComponent component = instance.getRootComponent();
    if (null != component) {
        this.mLayoutWidth = (int) component.getLayoutWidth();
        this.mLayoutHeight = (int) component.getLayoutHeight();
    }
    instance.getApmForInstance().onStage(WXInstanceApm.KEY_PAGE_STAGES_CREATE_FINISH);
    instance.getApmForInstance().extInfo.put(WXInstanceApm.KEY_PAGE_STAGES_CREATE_FINISH,true);
  }

  @Override
  public void executeAction() {
    final WXSDKInstance instance = getWXSDKIntance();
    if (instance == null || instance.getContext() == null) {
      return;
    }
    if(instance.mHasCreateFinish){
        return;
    }

    if (instance.getRenderStrategy() == WXRenderStrategy.APPEND_ONCE) {
      instance.onCreateFinish();
    }else{
      if(!RenderTypes.RENDER_TYPE_NATIVE.equals(instance.getRenderType())){
          instance.onCreateFinish();
      }
    }

    instance.mHasCreateFinish = true;

    if (null != instance.getWXPerformance()){
      instance.getWXPerformance().callCreateFinishTime = System.currentTimeMillis()-instance.getWXPerformance().renderTimeOrigin;
    }
    instance.onOldFsRenderTimeLogic();
  }
}
