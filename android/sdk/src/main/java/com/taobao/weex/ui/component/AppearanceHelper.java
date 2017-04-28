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
package com.taobao.weex.ui.component;

import android.graphics.Rect;
import android.view.View;
import com.taobao.weex.utils.WXLogUtils;


/**
 * Created by sospartan on 8/19/16.
 */
public class AppearanceHelper {

  private final WXComponent mAwareChild;

  private boolean mAppearStatus = false;
  private boolean[] mWatchFlags = {false, false};

  public static final int APPEAR = 0;
  public static final int DISAPPEAR = 1;

  public static final int RESULT_APPEAR = 1;
  public static final int RESULT_DISAPPEAR = -1;
  public static final int RESULT_NO_CHANGE = 0;

  private Rect mVisibleRect = new Rect();

  private int mCellPositionInScrollable;

  /**
   * @param awareChild child to notify when appearance changed.
   */
  public AppearanceHelper(WXComponent awareChild) {
    this(awareChild, 0);
  }

  public AppearanceHelper(WXComponent awareChild, int cellPositionInScrollable) {
    mAwareChild = awareChild;
    mCellPositionInScrollable = cellPositionInScrollable;
  }

  public void setCellPosition(int pos){
    mCellPositionInScrollable = pos;
  }

  public int getCellPositionINScollable() {
    return mCellPositionInScrollable;
  }

  /**
   * @param event  {@link #APPEAR} and {@link #DISAPPEAR}
   * @param enable
   */
  public void setWatchEvent(int event, boolean enable) {
    mWatchFlags[event] = enable;
  }

  /**
   * @return
   */
  public boolean isWatch() {
    return mWatchFlags[APPEAR] || mWatchFlags[DISAPPEAR];
  }


  public WXComponent getAwareChild() {
    return mAwareChild;
  }

  public boolean isAppear() {
    return mAppearStatus;
  }

  public int setAppearStatus(boolean newIsAppear) {
    if (mAppearStatus != newIsAppear) {
      mAppearStatus = newIsAppear;
      return newIsAppear ? RESULT_APPEAR : RESULT_DISAPPEAR;
    }

    return RESULT_NO_CHANGE;
  }

  public boolean isViewVisible() {
    View view = mAwareChild.getHostView();
    return view != null && view.getLocalVisibleRect(mVisibleRect);

  }
}
