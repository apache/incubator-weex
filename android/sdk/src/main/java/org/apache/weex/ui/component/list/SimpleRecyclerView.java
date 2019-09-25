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
package org.apache.weex.ui.component.list;

import android.content.Context;

import org.apache.weex.ui.view.listview.WXRecyclerView;
import org.apache.weex.ui.view.listview.adapter.RecyclerViewBaseAdapter;

/**
 * Simple list is used for specific occasion, NOT Support sticky,load more,bounce etc.
 * Created by sospartan on 13/12/2016.
 */

class SimpleRecyclerView extends WXRecyclerView implements ListComponentView{
  private RecyclerViewBaseAdapter mAdapter = null;

  public SimpleRecyclerView(Context context) {
    super(context);
  }

  @Override
  public WXRecyclerView getInnerView() {
    return this;
  }

  @Override
  public void setRecyclerViewBaseAdapter(RecyclerViewBaseAdapter adapter) {
    setAdapter(adapter);
    this.mAdapter = adapter;
  }

  /**
   * @param component
   */
  public void notifyStickyShow(WXCell component) {
    //Simple list is used for specific occasion, NOT Support sticky,load more,bounce etc.
  }

  /**
   * @param component
   */
  public void notifyStickyRemove(WXCell component) {
    //Simple list is used for specific occasion, NOT Support sticky,load more,bounce etc.
  }

  @Override
  public void updateStickyView(int currentStickyPos) {
    //Simple list is used for specific occasion, NOT Support sticky,load more,bounce etc.
  }

  @Override
  public RecyclerViewBaseAdapter getRecyclerViewBaseAdapter() {
    return mAdapter;
  }
}
