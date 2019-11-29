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
package org.apache.weex.ui.view;

import android.content.Context;
import android.graphics.Canvas;
import android.support.annotation.Nullable;
import android.view.MotionEvent;
import android.view.View;
import org.apache.weex.WXSDKInstance;
import org.apache.weex.WXSDKManager;
import org.apache.weex.common.Constants;
import org.apache.weex.common.WXErrorCode;
import org.apache.weex.ui.component.WXComponent;
import org.apache.weex.ui.component.WXDiv;
import org.apache.weex.ui.view.gesture.WXGesture;
import org.apache.weex.ui.view.gesture.WXGestureObservable;
import org.apache.weex.utils.WXExceptionUtils;
import org.apache.weex.utils.WXLogUtils;
import java.lang.ref.WeakReference;
import java.util.HashMap;
import java.util.Map;

/**
 * FrameLayout wrapper
 *
 */
public class WXFrameLayout extends BaseFrameLayout implements WXGestureObservable,IRenderStatus<WXDiv>,IRenderResult<WXDiv>{

  private WeakReference<WXDiv> mWeakReference;
  private WXGesture wxGesture;


  public WXFrameLayout(Context context) {
    super(context);
  }

  @Nullable
  @Override
  public WXDiv getComponent() {
    return null != mWeakReference ? mWeakReference.get() : null;
  }

  @Override
  public void holdComponent(WXDiv component) {
    mWeakReference = new WeakReference<WXDiv>(component);
  }

  @Override
  public void registerGestureListener(WXGesture wxGesture) {
    this.wxGesture = wxGesture;
  }

  @Override
  public WXGesture getGestureListener() {
    return wxGesture;
  }

  @Override
  public boolean dispatchTouchEvent(MotionEvent event) {
    boolean result = super.dispatchTouchEvent(event);
    if (wxGesture != null) {
      result |= wxGesture.onTouch(this, event);
    }
    return result;
  }
  @Override
  protected void dispatchDraw(Canvas canvas) {
    try {
      super.dispatchDrawInterval(canvas);
    } catch (Throwable e) {
      if (getComponent() != null) {
        notifyLayerOverFlow();
        if (null != getComponent()){
          WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(getComponent().getInstanceId());
          if (null != instance && null != instance.getApmForInstance() &&!instance.getApmForInstance().hasReportLayerOverDraw){
            instance.getApmForInstance().hasReportLayerOverDraw = true;
            reportLayerOverFlowError();
          }
        }
      }
      WXLogUtils.e("Layer overflow limit error", WXLogUtils.getStackTrace(e));
    }
  }
  private int reportLayerOverFlowError() {
    int deep = calLayerDeep(this, 0);
    if (getComponent() != null) {
      WXExceptionUtils.commitCriticalExceptionRT(getComponent().getInstanceId(),
              WXErrorCode.WX_RENDER_ERR_LAYER_OVERFLOW,
              "draw android view",
              WXErrorCode.WX_RENDER_ERR_LAYER_OVERFLOW.getErrorMsg() + "Layer overflow limit error: " + deep + " layers!",
              null);
    }
    return deep;
  }
  private int calLayerDeep(View view, int deep) {
    deep++;
    if (view.getParent() != null && view.getParent() instanceof View) {
      return calLayerDeep((View) view.getParent(), deep);
    }
    return deep;
  }

  public void notifyLayerOverFlow() {
    if (getComponent() == null)
      return;

    WXSDKInstance instance = getComponent().getInstance();
    if (instance == null)
      return;

    if (instance.getLayerOverFlowListeners() == null)
      return;

    for (String ref : instance.getLayerOverFlowListeners()) {
      WXComponent component = WXSDKManager.getInstance().getWXRenderManager().getWXComponent(instance.getInstanceId(), ref);
      Map<String, Object> params = new HashMap<>();
      params.put(Constants.Weex.REF, ref);
      params.put(Constants.Weex.INSTANCEID, component.getInstanceId());
      component.fireEvent(Constants.Event.LAYEROVERFLOW, params);
    }
  }

}
