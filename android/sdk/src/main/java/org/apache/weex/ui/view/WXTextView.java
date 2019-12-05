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

import android.annotation.SuppressLint;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Context;
import android.graphics.Canvas;
import android.support.annotation.ColorInt;
import android.support.annotation.Nullable;
import android.support.v7.widget.PopupMenu;
import android.text.Layout;
import android.text.TextUtils;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import org.apache.weex.ui.component.WXText;
import org.apache.weex.ui.view.gesture.WXGesture;
import org.apache.weex.ui.view.gesture.WXGestureObservable;
import java.lang.ref.WeakReference;

/**
 * TextView wrapper
 */
public class WXTextView extends View implements WXGestureObservable, IWXTextView,
        IRenderStatus<WXText>, IRenderResult<WXText> {

  private WeakReference<WXText> mWeakReference;
  private WXGesture wxGesture;
  private Layout textLayout;
  private boolean mIsLabelSet = false;

  public WXTextView(Context context) {
    super(context);
  }

  @Override
  protected void onDraw(Canvas canvas) {
    super.onDraw(canvas);
    canvas.save();
    Layout layout= getTextLayout();
    if(layout!=null){
      canvas.translate(getPaddingLeft(),getPaddingTop());
      layout.draw(canvas);
    }
    canvas.restore();
  }

  @SuppressLint("ClickableViewAccessibility")
  @Override
  public boolean onTouchEvent(MotionEvent event) {
    boolean result = super.onTouchEvent(event);
    if (wxGesture != null) {
      result |= wxGesture.onTouch(this, event);
    }
    return result;
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
  public CharSequence getText() {
    return textLayout != null ? textLayout.getText() : null;
  }

  public Layout getTextLayout() {
    return textLayout;
  }

  public void setTextLayout(Layout layout) {
    this.textLayout = layout;
    if(layout!=null && !mIsLabelSet){
      setContentDescription(layout.getText());
    }
    if (mWeakReference != null) {
      WXText wxText = mWeakReference.get();
      if (wxText != null) {
        wxText.readyToRender();
      }
    }
  }

  public void setAriaLabel(String label){
    if(!TextUtils.isEmpty(label)){
      mIsLabelSet = true;
      setContentDescription(label);
    }else{
      mIsLabelSet = false;
      if(textLayout != null){
        setContentDescription(textLayout.getText());
      }
    }

  }

  /**
   * Sets the text color for the text layout, it will be invalid
   * when {@link #setTextLayout(Layout)} happens, and color has to be
   * reset.
   *
   * @see #setTextLayout(Layout)
   *
   * @param color A color value in the form 0xAARRGGBB.
   */
  public void setTextColor(@ColorInt int color) {
    Layout layout = getTextLayout();
    if (layout != null) {
      layout.getPaint().setColor(color);
    }
  }

  @Override
  public void holdComponent(WXText component) {
    mWeakReference = new WeakReference<>(component);
  }

  @Nullable
  @Override
  public WXText getComponent() {
    return null != mWeakReference ? mWeakReference.get() : null;
  }

  public void enableCopy(boolean enable) {
    if (enable) {
      setOnLongClickListener(new OnLongClickListener() {
        @Override
        public boolean onLongClick(View v) {
          PopupMenu popupMenu = new PopupMenu(getContext(), WXTextView.this);
          String s = "Copy";
          try {
            s = getContext().getResources().getString(android.R.string.copy);
          } catch (Throwable t) {
            //ignore
          }
          final String title = s;
          popupMenu.getMenu().add(title);
          popupMenu.setOnMenuItemClickListener(new PopupMenu.OnMenuItemClickListener() {
            @Override
            public boolean onMenuItemClick(MenuItem item) {
              if (title.equals(item.getTitle())) {
                String data = getText().toString();
                ClipboardManager clipboardManager = (ClipboardManager) getContext().getSystemService(Context.CLIPBOARD_SERVICE);
                if (clipboardManager != null) {
                  ClipData clipData = ClipData.newPlainText(data, data);
                  clipboardManager.setPrimaryClip(clipData);
                }
                return true;
              }
              return false;
            }
          });
          popupMenu.show();
          return true;
        }
      });
    } else {
      setOnLongClickListener(null);
    }
  }
}
