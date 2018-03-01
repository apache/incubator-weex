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
package com.taobao.weex.ui.component.list;

import android.content.Context;

import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.ui.component.WXVContainer;
import com.taobao.weex.ui.view.listview.WXRecyclerView;

/**
 * A simple list component based on regular recyclerview, do not support refreshing and loading.
 * Created by sospartan on 13/12/2016.
 *
 */
public class SimpleListComponent extends BasicListComponent<SimpleRecyclerView>{

  public SimpleListComponent(WXSDKInstance instance, WXDomObject node, WXVContainer parent) {
    super(instance, node, parent);
  }

  @Override
  protected SimpleRecyclerView generateListView(Context context, int orientation) {
    SimpleRecyclerView view = new SimpleRecyclerView(context);
    view.initView(context, WXRecyclerView.TYPE_LINEAR_LAYOUT, orientation);
    return view;
  }
}
