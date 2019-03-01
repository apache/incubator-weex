package com.taobao.weex.render.platform.view.web;

import android.content.Context;
import android.util.AttributeSet;
import android.webkit.WebSettings;
import android.webkit.WebView;

public class RWebView extends WebView {

    public RWebView(Context context) {
        super(context);
        initView();
    }

    public RWebView(Context context, AttributeSet attrs) {
        super(context, attrs);
        initView();
    }

    public RWebView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        initView();
    }

    public RWebView(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
        initView();
    }

    public RWebView(Context context, AttributeSet attrs, int defStyleAttr, boolean privateBrowsing) {
        super(context, attrs, defStyleAttr, privateBrowsing);
        initView();
    }

    private void initView() {
        WebSettings settings= getSettings();
        settings.setJavaScriptEnabled(true);
        settings.setAppCacheEnabled(true);
        settings.setUseWideViewPort(true);
        settings.setDomStorageEnabled(true);
        settings.setSupportZoom(false);
        settings.setBuiltInZoomControls(false);
        settings.setAllowFileAccess(false);

        requestFocusFromTouch();
        setWebViewClient(new RWebViewClient());
        setWebChromeClient(new RWebChromeClient());

    }
}
