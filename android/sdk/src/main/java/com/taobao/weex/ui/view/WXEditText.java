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
package com.taobao.weex.ui.view;

import android.content.Context;
import android.os.Build;
import android.view.MotionEvent;
import android.view.ViewParent;
import android.widget.EditText;

import com.taobao.weex.common.WXThread;
import com.taobao.weex.ui.view.gesture.WXGesture;
import com.taobao.weex.ui.view.gesture.WXGestureObservable;

/**
 * Wrapper class for editText
 */
public class WXEditText extends EditText implements WXGestureObservable {

  private WXGesture wxGesture;
  private int mLines = 1;
  private boolean mAllowDisableMovement = true;

  public WXEditText(Context context) {
    super(context);
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN) {
      setBackground(null);
    } else {
      setBackgroundDrawable(null);
    }
  }

  @Override
  public void registerGestureListener(WXGesture wxGesture) {
    this.wxGesture = wxGesture;
  }

  @Override
  public void setLines(int lines) {
    super.setLines(lines);
    mLines = lines;
  }

  @Override
  public boolean onTouchEvent(MotionEvent event) {
    boolean result = super.onTouchEvent(event);
    if (wxGesture != null) {
      result |= wxGesture.onTouch(this, event);
    }

    ViewParent parent = getParent();
    if(parent != null){
      switch (event.getAction() & MotionEvent.ACTION_MASK){
        case MotionEvent.ACTION_DOWN:
          if(mLines < getLineCount()) {
            //scrollable
            parent.requestDisallowInterceptTouchEvent(true);
          }
          break;
        case MotionEvent.ACTION_UP:
        case MotionEvent.ACTION_CANCEL:
          parent.requestDisallowInterceptTouchEvent(false);
          break;
      }
    }
    return result;
  }

  @Override
  protected void onSizeChanged(int w, int h, int oldw, int oldh) {
    super.onSizeChanged(w, h, oldw, oldh);
    int contentH = getLayout().getHeight();
    //TODO: known issue,set movement to null will make cursor disappear.
    if(mAllowDisableMovement && h < contentH){
      setMovementMethod(null);
    } else {
      setMovementMethod(getDefaultMovementMethod());
    }
  }
  
  @Override
  public boolean postDelayed(Runnable action, long delayMillis) {
    return super.postDelayed(WXThread.secure(action), delayMillis);
  }

  public void setAllowDisableMovement(boolean allow) {
    mAllowDisableMovement = allow;
  }
}
