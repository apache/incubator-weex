/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
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
import android.support.annotation.RestrictTo;
import android.support.annotation.RestrictTo.Scope;
import com.taobao.weex.ui.flat.FlatGUIContext;
import java.util.LinkedList;
import java.util.List;

@RestrictTo(Scope.LIBRARY)
public class WidgetGroup extends BaseWidget {

  private List<Widget> mChildren = new LinkedList<>();

  public WidgetGroup(@NonNull FlatGUIContext context) {
    super(context);
  }

  public void replaceAll(@NonNull List<Widget> widgets) {
    mChildren = widgets;
    invalidate();
  }

  public List<Widget> getChildren() {
    return mChildren;
  }

  @Override
  public void onDraw(@NonNull Canvas canvas) {
    for (Widget child : mChildren) {
      child.draw(canvas);
    }
  }
}
