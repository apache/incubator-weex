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
import android.view.ActionMode;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.ViewParent;
import android.widget.EditText;

import com.taobao.weex.ui.view.gesture.WXGesture;
import com.taobao.weex.ui.view.gesture.WXGestureObservable;

/**
 * Wrapper class for editText
 */
public class WXEditText extends EditText implements WXGestureObservable {

  private WXGesture wxGesture;
  private int mLines = 1;
  private boolean mAllowDisableMovement = true;
  private boolean mAllowCopyPaste = true;

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
  public WXGesture getGestureListener() {
    return wxGesture;
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
    if (getLayout() != null) {
      int contentH = getLayout().getHeight();
      //TODO: known issue,set movement to null will make cursor disappear.
      if (mAllowDisableMovement && h < contentH) {
        setMovementMethod(null);
      } else {
        setMovementMethod(getDefaultMovementMethod());
      }
    }
  }

  public void setAllowDisableMovement(boolean allow) {
    mAllowDisableMovement = allow;
  }

  public void setAllowCopyPaste(boolean allow) {
    mAllowCopyPaste = allow;
    if (allow) {
      setLongClickable(true);
      setCustomSelectionActionModeCallback(null);
      if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
        setCustomInsertionActionModeCallback(null);
      }
    } else {
      setLongClickable(false);
      ActionMode.Callback callback = new ActionMode.Callback() {
        @Override
        public boolean onCreateActionMode(ActionMode mode, Menu menu) {
          return false;
        }

        @Override
        public boolean onPrepareActionMode(ActionMode mode, Menu menu) {
          return false;
        }

        @Override
        public boolean onActionItemClicked(ActionMode mode, MenuItem item) {
          return false;
        }

        @Override
        public void onDestroyActionMode(ActionMode mode) {

        }
      };
      if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
        setCustomInsertionActionModeCallback(callback);
      }
      setCustomSelectionActionModeCallback(callback);
    }
  }

  @Override
  public boolean onTextContextMenuItem(int id) {
    return !mAllowCopyPaste || super.onTextContextMenuItem(id);
  }
}
