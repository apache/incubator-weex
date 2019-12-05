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
package org.apache.weex;

import android.annotation.TargetApi;
import android.content.Context;
import android.os.Build;
import android.util.AttributeSet;
import android.view.View;


import org.apache.weex.render.WXAbstractRenderContainer;

/**
 * Created by sospartan on 08/10/2016.
 */

public class RenderContainer extends WXAbstractRenderContainer implements WeexFrameRateControl.VSyncListener{
  private WeexFrameRateControl mFrameRateControl;

  public RenderContainer(Context context) {
    super(context);
    mFrameRateControl = new WeexFrameRateControl(this);
  }

  public RenderContainer(Context context, AttributeSet attrs) {
    super(context, attrs);
    mFrameRateControl = new WeexFrameRateControl(this);
  }

  public RenderContainer(Context context, AttributeSet attrs, int defStyleAttr) {
    super(context, attrs, defStyleAttr);
    mFrameRateControl = new WeexFrameRateControl(this);
  }

  @TargetApi(Build.VERSION_CODES.LOLLIPOP)
  public RenderContainer(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
    super(context, attrs, defStyleAttr, defStyleRes);
    mFrameRateControl = new WeexFrameRateControl(this);
  }



  @Override
  public void onAttachedToWindow() {
    super.onAttachedToWindow();
    if (mFrameRateControl != null) {
      mFrameRateControl.start();
    }
  }

  @Override
  protected void onDetachedFromWindow() {
    super.onDetachedFromWindow();
    if (mFrameRateControl != null) {
      mFrameRateControl.stop();
    }
  }
  @Override
  public void dispatchWindowVisibilityChanged(int visibility) {
    super.dispatchWindowVisibilityChanged(visibility);
    if (visibility == View.GONE) {
      if (mFrameRateControl != null) {
        mFrameRateControl.stop();
      }
    } else if (visibility == View.VISIBLE) {
      if (mFrameRateControl != null) {
        mFrameRateControl.start();
      }
    }
  }

  @Override
  public void OnVSync() {
    if (mSDKInstance != null && mSDKInstance.get() != null) {
      mSDKInstance.get().OnVSync();
    }
  }

}
