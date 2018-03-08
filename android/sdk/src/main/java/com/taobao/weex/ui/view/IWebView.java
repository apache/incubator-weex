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
package com.taobao.weex.ui.view;

import android.view.View;

import java.util.Map;

public interface IWebView {
    public View getView();
    public void destroy();
    public void loadUrl(String url);
    public void loadDataWithBaseURL(String source);
    public void reload();
    public void goBack();
    public void goForward();
    public void postMessage(Object msg);
    public void setShowLoading(boolean shown);
    public void setOnErrorListener(OnErrorListener listener);
    public void setOnPageListener(OnPageListener listener);
    public void setOnMessageListener(OnMessageListener listener);

    public interface OnErrorListener {
        public void onError(String type, Object message);
    }

    public interface OnPageListener {
        public void onReceivedTitle(String title);
        public void onPageStart(String url);
        public void onPageFinish(String url, boolean canGoBack, boolean canGoForward);
    }

    public interface OnMessageListener {
        public void onMessage(Map<String, Object> params);
    }
}
