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
package com.taobao.weex.ui.flat.widget;


import android.graphics.Canvas;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.annotation.RestrictTo;
import android.support.annotation.RestrictTo.Scope;
import android.view.View;
import com.taobao.weex.common.Destroyable;
import com.taobao.weex.ui.flat.FlatGUIContext;

@RestrictTo(Scope.LIBRARY)
public class AndroidViewWidget extends BaseWidget implements Destroyable {

  private @Nullable View mView;
  public AndroidViewWidget(@NonNull FlatGUIContext context) {
    super(context);
  }

  public void setContentView(@Nullable View view){
    this.mView = view;
  }

  @Override
  public void setContentBox(int leftOffset, int topOffset, int rightOffset, int bottomOffset) {
    if(mView!=null) {
      mView.setPadding(leftOffset, topOffset, rightOffset, bottomOffset);
      invalidate();
    }
  }

  @Override
  public void onDraw(@NonNull Canvas canvas) {
    if(mView!=null) {
      mView.draw(canvas);
    }
  }

  @Override
  public void invalidate() {
    super.invalidate();
    if (mView != null) {
      mView.invalidate();
    }
  }

  public @Nullable View getView() {
    return mView;
  }

  @Override
  public void destroy() {
    if (mView != null) {
      mView = null;
    }
  }
}
