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
package com.taobao.weex.render.frame;

import android.content.Context;

public class RenderFrame {

    private RenderView mRenderView;
    private Context mContext;

    public RenderFrame(RenderView renderFrameView, Context context) {
        this.mRenderView = renderFrameView;
        this.mContext = context;
    }

    public void onResume() {
        if(mRenderView != null){
            mRenderView.frameOnResume();
        }
    }

    public void onPause() {
        if(mRenderView != null){
            mRenderView.frameOnPause();
        }
    }

    public void onLowMemory(){
        if(mRenderView != null){
            mRenderView.frameOnLowMemory();
        }
    }

    public void onDestroy() {
        if(mRenderView != null){
            mRenderView.onDestroy();
        }
    }


    public Context getContext() {
        return mContext;
    }

    public void setContext(Context context) {
        this.mContext = context;
    }

    public RenderView getRenderView() {
        return mRenderView;
    }

    public void setRenderView(RenderView renderView) {
        this.mRenderView = renderView;
    }




}
