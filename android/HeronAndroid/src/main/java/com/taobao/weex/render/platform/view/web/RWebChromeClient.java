package com.taobao.weex.render.platform.view.web;

import android.webkit.WebChromeClient;
import android.webkit.WebView;

public class RWebChromeClient extends WebChromeClient {
    @Override
    public void onProgressChanged(WebView view, int newProgress) {
        super.onProgressChanged(view, newProgress);
    }
    @Override
    public void onReceivedTitle(WebView view, String title) {
        super.onReceivedTitle(view, title);
    }

}
