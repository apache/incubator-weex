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
package org.apache.weex.ui.view.refresh.wrapper;

import android.annotation.SuppressLint;
import android.content.Context;

import org.apache.weex.ui.component.WXScroller;
import org.apache.weex.ui.view.WXScrollView;

@SuppressLint("ViewConstructor")
public class BounceScrollerView extends BaseBounceView<WXScrollView> {

    public BounceScrollerView(Context context, int orientation, WXScroller waScroller) {
        super(context,orientation);
        init(context);
        if (getInnerView() != null)
            getInnerView().setWAScroller(waScroller);
    }

    @Override
    public WXScrollView setInnerView(Context context) {
        return new WXScrollView(context);
    }

    @Override
    public void onRefreshingComplete() {
        //TODO update scroller dataset
    }

    @Override
    public void onLoadmoreComplete() {
        //TODO update scroller dataset
    }
}
