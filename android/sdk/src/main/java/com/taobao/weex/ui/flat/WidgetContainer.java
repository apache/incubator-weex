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
package com.taobao.weex.ui.flat;


import android.support.annotation.RestrictTo;
import android.support.annotation.RestrictTo.Scope;
import android.util.Pair;
import android.view.ViewGroup;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXVContainer;
import com.taobao.weex.ui.flat.widget.AndroidViewWidget;
import com.taobao.weex.ui.flat.widget.Widget;
import java.util.LinkedList;
import java.util.List;

@RestrictTo(Scope.LIBRARY)
public abstract class WidgetContainer<T extends ViewGroup> extends WXVContainer<T> {

  protected List<Widget> widgets = new LinkedList<>();

  public WidgetContainer(WXSDKInstance instance, WXDomObject node, WXVContainer parent) {
    super(instance, node, parent);
  }

  protected abstract void mountFlatGUI();

  protected abstract void unmountFlatGUI();

  public boolean intendToBeFlatContainer() {
    return false;
  }

  @Override
  public void createChildViewAt(int index) {
    if (intendToBeFlatContainer()) {
      Pair<WXComponent, Integer> ret = rearrangeIndexAndGetChild(index);
      if (ret.first != null) {
        WXComponent child = ret.first;
        Widget flatChild;
        FlatGUIContext uiImp = getInstance().getFlatUIContext();
        WidgetContainer parent = uiImp.getFlatComponentAncestor(this);
        if (parent == null || uiImp.getAndroidViewWidget(this) != null) {
          parent = this;
        }
        uiImp.register(child, parent);

        if (child instanceof FlatComponent && !((FlatComponent) child).promoteToView(false)) {
          flatChild = ((FlatComponent) child).getOrCreateFlatWidget();
        } else {
          flatChild = new AndroidViewWidget(uiImp);
          uiImp.register(child, (AndroidViewWidget) flatChild);
          child.createView();
          ((AndroidViewWidget) flatChild).setContentView(child.getHostView());
          //TODO Use a sort algorithm to decide the childIndex of AndroidViewWidget
          parent.addSubView(child.getHostView(), -1);
        }
        uiImp.register(flatChild, child);
        addFlatChild(flatChild, ret.second);
      }
    } else {
      super.createChildViewAt(index);
    }
  }

  private void addFlatChild(Widget widget, int index) {
    if (index >= widgets.size()) {
      widgets.add(widget);
    } else {
      widgets.add(index, widget);
    }
    //TODO do a partial update, not mount the whole flatContainer.
    mountFlatGUI();
  }
}
