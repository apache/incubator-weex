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
package org.apache.weex.ui.flat;


import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.annotation.RestrictTo;
import android.support.annotation.RestrictTo.Scope;
import android.support.v4.util.ArrayMap;
import android.text.TextUtils;
import android.view.View;

import org.apache.weex.common.Constants.Name;
import org.apache.weex.common.Destroyable;
import org.apache.weex.dom.WXAttr;
import org.apache.weex.dom.WXStyle;
import org.apache.weex.ui.component.WXComponent;
import org.apache.weex.ui.flat.widget.AndroidViewWidget;
import org.apache.weex.ui.flat.widget.Widget;

import java.util.Map;
import java.util.Map.Entry;

//TODO The constructor of FlatGUIContext should have a flag decide whether to enable flagGUI.

@RestrictTo(Scope.LIBRARY)
public class FlatGUIContext implements Destroyable {

  private Map<WXComponent, WidgetContainer> mWidgetRegistry = new ArrayMap<>();
  private Map<WXComponent, AndroidViewWidget> mViewWidgetRegistry = new ArrayMap<>();
  private Map<Widget, WXComponent> widgetToComponent = new ArrayMap<>();

  public boolean isFlatUIEnabled(WXComponent component) {
    return false;
  }

  public void register(@NonNull WXComponent descendant, @NonNull WidgetContainer ancestor) {
    if (!(ancestor instanceof FlatComponent) ||
            ((FlatComponent) ancestor).promoteToView(true)) {
      mWidgetRegistry.put(descendant, ancestor);
    }
  }

  public void register(@NonNull WXComponent component, @NonNull AndroidViewWidget viewWidget) {
    mViewWidgetRegistry.put(component, viewWidget);
  }

  public void register(@NonNull Widget widget, @NonNull WXComponent component) {
    widgetToComponent.put(widget, component);
  }

  public
  @Nullable
  WidgetContainer getFlatComponentAncestor(@NonNull WXComponent flatWidget) {
    return mWidgetRegistry.get(flatWidget);
  }

  public
  @Nullable
  AndroidViewWidget getAndroidViewWidget(@NonNull WXComponent component) {
    return mViewWidgetRegistry.get(component);
  }

  public boolean promoteToView(@NonNull WXComponent component, boolean checkAncestor,
                               @NonNull Class<? extends WXComponent<?>> expectedClass) {
    return !isFlatUIEnabled(component) ||
            !expectedClass.equals(component.getClass()) ||
            TextUtils.equals(component.getRef(), WXComponent.ROOT) ||
            (checkAncestor && getFlatComponentAncestor(component) == null) ||
            checkComponent(component);
  }

  public
  @Nullable
  View getWidgetContainerView(Widget widget) {
    WXComponent component, ancestor;
    View ret = null;
    if ((component = getComponent(widget)) != null) {
      if ((ancestor = getFlatComponentAncestor(component)) != null) {
        ret = ancestor.getHostView();
      }
    }
    return ret;
  }

  @Override
  @RestrictTo(Scope.LIBRARY)
  public void destroy() {
    widgetToComponent.clear();

    for (Entry<WXComponent, AndroidViewWidget> entry : mViewWidgetRegistry.entrySet()) {
      entry.getValue().destroy();
    }
    mViewWidgetRegistry.clear();

    for (Entry<WXComponent, WidgetContainer> entry : mWidgetRegistry.entrySet()) {
      entry.getValue().unmountFlatGUI();
    }
    mWidgetRegistry.clear();
  }

  private @Nullable
  WXComponent getComponent(@NonNull Widget widget) {
    return widgetToComponent.get(widget);
  }

  private boolean checkComponent(@NonNull WXComponent component) {
    boolean ret = false;
    if (component != null) {
      WXStyle style = component.getStyles();
      WXAttr attr = component.getAttrs();
      if (style.containsKey(Name.OPACITY) ||
              style.containsKey(Name.TRANSFORM) ||
              style.containsKey(Name.VISIBILITY) ||
              attr.containsKey(Name.ELEVATION) ||
              attr.containsKey(Name.ARIA_HIDDEN) ||
              attr.containsKey(Name.ARIA_LABEL) ||
              attr.containsKey(WXComponent.PROP_FIXED_SIZE) ||
              attr.containsKey(Name.DISABLED) ||
              style.isFixed() ||
              style.isSticky() ||
              !style.getPesudoStyles().isEmpty() ||
              component.getEvents().size() > 0) {
        ret = true;
      }
    }
    return ret;
  }
}
