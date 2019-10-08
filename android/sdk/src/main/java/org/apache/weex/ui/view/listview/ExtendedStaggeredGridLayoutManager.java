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
package org.apache.weex.ui.view.listview;

import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.StaggeredGridLayoutManager;

import org.apache.weex.WXEnvironment;
import org.apache.weex.utils.WXLogUtils;

/**
 * Created by zhengshihan on 2017/5/5.
 */

public class ExtendedStaggeredGridLayoutManager extends StaggeredGridLayoutManager{

  public ExtendedStaggeredGridLayoutManager(int spanCount, int orientation) {
    super(spanCount, orientation);
  }

  @Override
  public void onItemsRemoved(RecyclerView recyclerView, int positionStart, int itemCount) {
    if(positionStart ==-1){
      WXLogUtils.e("ExtendedStaggeredGridLayoutManager: onItemsRemoved  Error Invalid Index : positionStart :"+positionStart +"  itemCount:"+ itemCount);
      return ;
    }else {
      if(WXEnvironment.isApkDebugable()){
        WXLogUtils.e("ExtendedStaggeredGridLayoutManager: onItemsRemoved  positionStart :"+positionStart+"  itemCount:"+ itemCount);
      }
    }
    super.onItemsRemoved(recyclerView, positionStart, itemCount);
  }
}
