package com.taobao.weex.render.platform.component;

import android.text.TextUtils;
import android.view.View;

import com.taobao.weex.render.frame.RenderView;
import com.taobao.weex.render.platform.view.web.RWebView;

import java.util.List;
import java.util.Map;

public class Web extends Component {

    private RWebView mWebView;

    @Override
    public void createView(RenderView renderView, Map<String, String> attrs, Map<String, String> styles,
                           List<String> events) {
        super.init(renderView, attrs, styles, events);
        mWebView = new RWebView(renderView.getContext());
    }

    @Override
    public View getView() {
        return mWebView;
    }

    @Override
    public void onAttach() {
        String url = getAttrs().get(Constants.Attr.Web.HREF);
        if(TextUtils.isEmpty(url)){
            return;
        }
        mWebView.loadUrl(url);
    }

    @Override
    public void onUpdateAttr(Map<String, String> attrs) {
        onAttach();
    }

    @Override
    public void onUpdateStyle(Map<String, String> attrs) {

    }

    @Override
    public void onAddEvent(String event) {

    }

    @Override
    public void onRemoveEvent(String event) {

    }

    @Override
    public void onResume() {
        if(mWebView != null){
            mWebView.onResume();
        }
    }

    @Override
    public void onPause() {
        if(mWebView != null){
            mWebView.onPause();
        }
    }

    @Override
    public void onDestroy() {
        if(mWebView != null){
            mWebView.destroy();
        }
    }
}
