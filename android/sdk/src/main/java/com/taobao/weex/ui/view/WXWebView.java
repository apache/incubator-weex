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

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.net.Uri;
import android.net.http.SslError;
import android.os.Build;
import android.os.Handler;
import android.os.Message;
import android.support.annotation.Nullable;
import android.text.TextUtils;
import android.view.Gravity;
import android.view.View;
import android.webkit.JavascriptInterface;
import android.webkit.JsPromptResult;
import android.webkit.SslErrorHandler;
import android.webkit.ValueCallback;
import android.webkit.WebChromeClient;
import android.webkit.WebResourceError;
import android.webkit.WebResourceRequest;
import android.webkit.WebResourceResponse;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.FrameLayout;
import android.widget.ProgressBar;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONException;
import com.alibaba.fastjson.JSONObject;
import com.taobao.weex.utils.WXLogUtils;

import java.lang.ref.WeakReference;

public class WXWebView implements IWebView {

    private Context mContext;
    private WebView mWebView;
    private ProgressBar mProgressBar;
    private boolean mShowLoading = true;
    private Handler mMessageHandler;
    private static final int POST_MESSAGE = 1;
    private static final String BRIDGE_NAME = "__WEEX_WEB_VIEW_BRIDGE";
    private static final int SDK_VERSION = Build.VERSION.SDK_INT;
    // downgraded by CVE-2012-6636(https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2012-6636)
    private static final boolean DOWNGRADE_JS_INTERFACE = SDK_VERSION < 17;

    private OnErrorListener mOnErrorListener;
    private OnPageListener mOnPageListener;
    private OnMessageListener mOnMessageListener;

    public WXWebView(Context context) {
        mContext = context;
    }

    @Override
    public View getView() {
        FrameLayout root = new FrameLayout(mContext);
        root.setBackgroundColor(Color.WHITE);

        mWebView = new WebView(mContext);//mContext.getApplicationContext();
        FrameLayout.LayoutParams wvLayoutParams =
                new FrameLayout.LayoutParams(FrameLayout.LayoutParams.MATCH_PARENT,
                        FrameLayout.LayoutParams.MATCH_PARENT);
        wvLayoutParams.gravity = Gravity.CENTER;
        mWebView.setLayoutParams(wvLayoutParams);
        root.addView(mWebView);
        initWebView(mWebView);

        mProgressBar = new ProgressBar(mContext);
        showProgressBar(false);
        FrameLayout.LayoutParams pLayoutParams =
                new FrameLayout.LayoutParams(FrameLayout.LayoutParams.WRAP_CONTENT,
                        FrameLayout.LayoutParams.WRAP_CONTENT);
        mProgressBar.setLayoutParams(pLayoutParams);
        pLayoutParams.gravity = Gravity.CENTER;
        root.addView(mProgressBar);

        mMessageHandler = new MessageHandler(this);

        return root;
    }

    @Override
    public void destroy() {
        if (getWebView() != null) {
            getWebView().removeAllViews();
            getWebView().destroy();
            mWebView = null;
        }
    }

    @Override
    public void loadUrl(String url) {
        if(getWebView() == null)
            return;
        getWebView().loadUrl(url);
    }

    @Override
    public void loadDataWithBaseURL(String source, String baseUrl) {
        if (mWebView == null) return;
        mWebView.loadDataWithBaseURL(baseUrl, source, "text/html", "utf-8", null);
    }

    @Override
    public void reload() {
        if(getWebView() == null)
            return;
        getWebView().reload();
    }

    @Override
    public void goBack() {
        if(getWebView() == null)
            return;
        getWebView().goBack();
    }

    @Override
    public void goForward() {
        if(getWebView() == null)
            return;
        getWebView().goForward();
    }

    @Override
    public void postMessage(Object msg) {
        if(getWebView() == null)
            return;

        try {
            JSONObject jsonMsg = new JSONObject();
            jsonMsg.put("data", msg);
            evaluateJS("javascript:(function () {" +
                "var event;" +
                "var data = " + jsonMsg.toString() + ";" +
                "try {" +
                "event = new MessageEvent('message', data);" +
                "} catch (e) {" +
                "event = document.createEvent('MessageEvent');" +
                "event.initMessageEvent('message', true, true, data.data, data.origin, data.lastEventId, data.source);" +
                "}" +
                "document.dispatchEvent(event);" +
                "})();");
        } catch (JSONException e) {
            throw new RuntimeException(e);
        }
    }

    /*@Override
    public void setVisibility(int visibility) {
        if (mRootView != null) {
            mRootView.setVisibility(visibility);
        }
    }*/

    @Override
    public void setShowLoading(boolean shown) {
        mShowLoading = shown;
    }

    @Override
    public void setOnErrorListener(OnErrorListener listener) {
        mOnErrorListener = listener;
    }

    @Override
    public void setOnPageListener(OnPageListener listener) {
        mOnPageListener = listener;
    }

    @Override
    public void setOnMessageListener(OnMessageListener listener) {
        mOnMessageListener = listener;
    }

    private void showProgressBar(boolean shown) {
        if (mShowLoading) {
            mProgressBar.setVisibility(shown ? View.VISIBLE : View.GONE);
        }
    }

    private void showWebView(boolean shown) {
        mWebView.setVisibility(shown ? View.VISIBLE : View.INVISIBLE);
    }

    private @Nullable WebView getWebView() {
        //TODO: remove this, duplicate with getView semantically.
        return mWebView;
    }

    private void initWebView(WebView wv) {
        WebSettings settings = wv.getSettings();
        settings.setJavaScriptEnabled(true);
        settings.setAppCacheEnabled(true);
        settings.setUseWideViewPort(true);
        settings.setDomStorageEnabled(true);
        settings.setSupportZoom(false);
        settings.setBuiltInZoomControls(false);
        wv.setWebViewClient(new WebViewClient() {

            @Override
            public boolean shouldOverrideUrlLoading(WebView view, String url) {
                view.loadUrl(url);
                WXLogUtils.v("tag", "onPageOverride " + url);
                return true;
            }

            @Override
            public void onPageStarted(WebView view, String url, Bitmap favicon) {
                super.onPageStarted(view, url, favicon);
                WXLogUtils.v("tag", "onPageStarted " + url);
                if (mOnPageListener != null) {
                    mOnPageListener.onPageStart(url);
                }
            }

            @Override
            public void onPageFinished(WebView view, String url) {
                super.onPageFinished(view, url);
                WXLogUtils.v("tag", "onPageFinished " + url);
                if (mOnPageListener != null) {
                    mOnPageListener.onPageFinish(url, view.canGoBack(), view.canGoForward());
                }
                if (mOnMessageListener != null) {
                    evaluateJS("javascript:(window.postMessage = function(data) {"
                        + (DOWNGRADE_JS_INTERFACE
                        ? "prompt('" + BRIDGE_NAME + "://postMessage?data=' + JSON.stringify(data))"
                        : BRIDGE_NAME + ".postMessage(JSON.stringify(data));")
                        + "})");
                }
            }

            @Override
            public void onReceivedError(WebView view, WebResourceRequest request, WebResourceError error) {
                super.onReceivedError(view, request, error);
                if (mOnErrorListener != null) {
                    //mOnErrorListener.onError("error", "page error code:" + error.getErrorCode() + ", desc:" + error.getDescription() + ", url:" + request.getUrl());
                    mOnErrorListener.onError("error", "page error");
                }
            }

            @Override
            public void onReceivedHttpError(WebView view, WebResourceRequest request, WebResourceResponse errorResponse) {
                super.onReceivedHttpError(view, request, errorResponse);
                if (mOnErrorListener != null) {
                    mOnErrorListener.onError("error", "http error");
                }
            }

            @Override
            public void onReceivedSslError(WebView view, SslErrorHandler handler, SslError error) {
                super.onReceivedSslError(view, handler, error);
                if (mOnErrorListener != null) {
                    mOnErrorListener.onError("error", "ssl error");
                }
            }
        });
        wv.setWebChromeClient(new WebChromeClient() {
            @Override
            public void onProgressChanged(WebView view, int newProgress) {
                super.onProgressChanged(view, newProgress);
                showWebView(newProgress == 100);
                showProgressBar(newProgress != 100);
                WXLogUtils.v("tag", "onPageProgressChanged " + newProgress);
            }

            @Override
            public void onReceivedTitle(WebView view, String title) {
                super.onReceivedTitle(view, title);
                if (mOnPageListener != null) {
                    mOnPageListener.onReceivedTitle(view.getTitle());
                }
            }

            @Override
            public boolean onJsPrompt(WebView view, String url, String message, String defaultValue, JsPromptResult result) {
                Uri uri = Uri.parse(message);
                String scheme = uri.getScheme();
                if (TextUtils.equals(scheme, BRIDGE_NAME)) {
                    if (TextUtils.equals(uri.getAuthority(), "postMessage")) {
                        String data = uri.getQueryParameter("data");
                        onMessage(data);
                        result.confirm("success");
                    } else {
                        result.confirm("fail");
                    }
                    return true;
                }
                return super.onJsPrompt(view, url, message, defaultValue, result);
            }
        });
        if (!DOWNGRADE_JS_INTERFACE) {
            wv.addJavascriptInterface(new Object() {
                @JavascriptInterface
                public void postMessage(String data) {
                    onMessage(data);
                }
            }, BRIDGE_NAME);
        }
    }

    private void onMessage(String data) {
        if (data != null && mOnMessageListener != null) {
            try {
                Message message = new Message();
                message.what = POST_MESSAGE;
                message.obj = JSON.parse(data);
                mMessageHandler.sendMessage(message);
            } catch (JSONException e) {
                throw new RuntimeException(e);
            }

        }
    }

    private void evaluateJS(String jsStr) {
        if (SDK_VERSION < 19) {
            mWebView.loadUrl(jsStr);
        } else {
            mWebView.evaluateJavascript(jsStr, new ValueCallback<String>() {
                @Override
                public void onReceiveValue(String value) {
                }
            });
        }
    }

    private static class MessageHandler extends Handler {
        private final WeakReference<WXWebView> mWv;

        private MessageHandler(WXWebView wv) {
            mWv = new WeakReference<>(wv);
        }

        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            switch (msg.what) {
                case POST_MESSAGE:
                    if (mWv.get() != null && mWv.get().mOnMessageListener != null) {
                        mWv.get().mOnMessageListener.onMessage(msg.obj);
                    }
                    break;
                default:
                    break;
            }
        }
    }
}
