package com.taobao.weex.render.platform;

import android.view.MotionEvent;
import android.view.View;

import com.taobao.weex.render.frame.RenderView;

import java.util.List;
import java.util.Map;

public interface PlatformView {

    /**
     * when view is added to RenderView. Events, Help Me
     * */
    public void createView(RenderView renderView, Map<String, String> attrs, Map<String, String> styles, List<String> events);

    public View getView();

    public void onAttach();

    public void onUpdateAttr(Map<String, String> attrs);

    public void onUpdateStyle(Map<String, String> attrs);

    public void onAddEvent(String event);

    public void onRemoveEvent(String event);

    public void onResume();

    public void onPause();

    public void onDetach();

    public void onShow();

    public void onHide();

    public void onDestroy();

    public void setRef(String ref);

    public void setType(String type);

    public void onTouch(MotionEvent motionEvent, float viewScreenX, float viewScreenY);

    public void onPositionChange(float x, float y, float width, float heightC);
}
