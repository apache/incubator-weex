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
package org.apache.weex.ui.component;

import android.content.Context;
import android.net.Uri;
import android.support.annotation.NonNull;
import android.text.TextUtils;
import android.view.View;

import org.apache.weex.WXSDKInstance;
import org.apache.weex.WXSDKManager;
import org.apache.weex.annotation.Component;
import org.apache.weex.adapter.URIAdapter;
import org.apache.weex.annotation.JSMethod;
import org.apache.weex.common.Constants;
import org.apache.weex.ui.action.BasicComponentData;
import org.apache.weex.ui.view.IWebView;
import org.apache.weex.ui.view.WXWebView;
import org.apache.weex.utils.WXUtils;

import java.util.HashMap;
import java.util.Map;

@Component(lazyload = false)

public class WXWeb extends WXComponent {

    public static final String GO_BACK = "goBack";
    public static final String GO_FORWARD = "goForward";
    public static final String RELOAD = "reload";
    public static final String POST_MESSAGE = "postMessage";
    protected IWebView mWebView;

    @Deprecated
    public WXWeb(WXSDKInstance instance, WXVContainer parent, String instanceId, boolean isLazy, BasicComponentData basicComponentData) {
        this(instance, parent, isLazy, basicComponentData);
    }

    public WXWeb(WXSDKInstance instance, WXVContainer parent, boolean isLazy, BasicComponentData basicComponentData) {
        super(instance, parent, isLazy, basicComponentData);
        createWebView();
    }

    protected void createWebView(){
        String origin = null;
        try {
            String bundleUrl = WXSDKManager.getInstance().getSDKInstance(getInstanceId()).getBundleUrl();
            Uri uri = Uri.parse(bundleUrl);
            String scheme = uri.getScheme();
            String authority = uri.getAuthority();
            if (!TextUtils.isEmpty(scheme) && !TextUtils.isEmpty(authority)) {
                origin = scheme + "://" + authority;
            }
        } catch (Exception e) {
            // do noting
        }
        mWebView = new WXWebView(getContext(), origin);
    }

    @Override
    protected View initComponentHostView(@NonNull Context context) {
        mWebView.setOnErrorListener(new IWebView.OnErrorListener() {
            @Override
            public void onError(String type, Object message) {
                fireEvent(type, message);
            }
        });
        mWebView.setOnPageListener(new IWebView.OnPageListener() {
            @Override
            public void onReceivedTitle(String title) {
                if (getEvents().contains(Constants.Event.RECEIVEDTITLE)) {
                    Map<String, Object> params = new HashMap<>();
                    params.put("title", title);
                    fireEvent(Constants.Event.RECEIVEDTITLE, params);
                }
            }

            @Override
            public void onPageStart(String url) {
                if (getEvents().contains(Constants.Event.PAGESTART)) {
                    Map<String, Object> params = new HashMap<>();
                    params.put("url", url);
                    fireEvent(Constants.Event.PAGESTART, params);
                }
            }

            @Override
            public void onPageFinish(String url, boolean canGoBack, boolean canGoForward) {
                if (getEvents().contains(Constants.Event.PAGEFINISH)) {
                    Map<String, Object> params = new HashMap<>();
                    params.put("url", url);
                    params.put("canGoBack", canGoBack);
                    params.put("canGoForward", canGoForward);
                    fireEvent(Constants.Event.PAGEFINISH, params);
                }
            }
        });
        mWebView.setOnMessageListener(new IWebView.OnMessageListener() {
            @Override
            public void onMessage(Map<String, Object> params) {
                fireEvent(Constants.Event.ONMESSAGE, params);
            }
        });
        return mWebView.getView();
    }

    @Override
    public void destroy() {
        super.destroy();
        getWebView().destroy();
    }

    @Override
    protected boolean setProperty(String key, Object param) {
        switch (key) {
            case Constants.Name.SHOW_LOADING:
                Boolean result = WXUtils.getBoolean(param,null);
                if (result != null)
                    setShowLoading(result);
                return true;
            case Constants.Name.SRC:
                String src = WXUtils.getString(param,null);
                if (src != null)
                    setUrl(src);
                return true;
            case Constants.Name.SOURCE:
                String source = WXUtils.getString(param,null);
                if (source != null)
                    setSource(source);
                return true;
        }
        return super.setProperty(key,param);
    }

    @WXComponentProp(name = Constants.Name.SHOW_LOADING)
    public void setShowLoading(boolean showLoading) {
        getWebView().setShowLoading(showLoading);
    }

    @WXComponentProp(name = Constants.Name.SRC)
    public void setUrl(String url) {
        if (TextUtils.isEmpty(url) || getHostView() == null) {
            return;
        }
        if (!TextUtils.isEmpty(url)) {
            loadUrl(getInstance().rewriteUri(Uri.parse(url), URIAdapter.WEB).toString());
        }
    }

    @WXComponentProp(name = Constants.Name.SOURCE)
    public void setSource(String source) {
        if (!TextUtils.isEmpty(source) && getHostView() != null) {
            loadDataWithBaseURL(source);
        }
    }

    public void setAction(String action, Object data) {
        if (!TextUtils.isEmpty(action)) {
            if (action.equals(GO_BACK)) {
                goBack();
            } else if (action.equals(GO_FORWARD)) {
                goForward();
            } else if (action.equals(RELOAD)) {
                reload();
            } else if (action.equals(POST_MESSAGE)) {
                postMessage(data);
            }
        }
    }

    private void fireEvent(String type, Object message) {
        if (getEvents().contains(Constants.Event.ERROR)) {
            Map<String, Object> params = new HashMap<>();
            params.put("type", type);
            params.put("errorMsg", message);
            fireEvent(Constants.Event.ERROR, params);
        }
    }

    private void loadUrl(String url) {
        getWebView().loadUrl(url);
    }

    private void loadDataWithBaseURL(String source) {
        getWebView().loadDataWithBaseURL(source);
    }

    @JSMethod
    public void reload() {
        getWebView().reload();
    }

    @JSMethod
    public void goForward() {
        getWebView().goForward();
    }

    @JSMethod
    public void goBack() {
        getWebView().goBack();
    }

    @JSMethod
    public void postMessage(Object msg) {
        getWebView().postMessage(msg);
    }

    private IWebView getWebView() {
        return mWebView;
    }

}
