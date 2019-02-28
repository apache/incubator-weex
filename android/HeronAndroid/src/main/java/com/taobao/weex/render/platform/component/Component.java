package com.taobao.weex.render.platform.component;

import android.view.MotionEvent;
import android.view.View;

import com.taobao.weex.render.frame.RenderView;
import com.taobao.weex.render.platform.PlatformView;

import java.util.List;
import java.util.Map;

public abstract class Component implements PlatformView {

    private RenderView mRenderView;
    private Map<String, String> mAttrs;
    private Map<String, String> mStyles;
    private List<String> mEvents;
    private String mRef;
    private String mType;


    public void init(RenderView renderView, Map<String, String> attrs, Map<String, String> styles, List<String> events) {
        mRenderView = renderView;
        mAttrs = attrs;
        mStyles = styles;
        mEvents = events;
    }

    @Override
    public void onDetach() {

    }

    public RenderView getRenderView() {
        return mRenderView;
    }

    public Map<String, String> getAttrs() {
        return mAttrs;
    }

    public Map<String, String> getStyles() {
        return mStyles;
    }

    public List<String> getEvents() {
        return mEvents;
    }


    public void setRef(String ref){
        mRef = ref;
    }

    public void setType(String type){
        mType = type;
    }

    public String getRef() {
        return mRef;
    }

    public String getType() {
        return mType;
    }


    @Override
    public void onShow(){
        if(getView() != null){
            getView().setVisibility(View.VISIBLE);
        }
    }

    @Override
    public void onHide(){
        if(getView() != null){
            getView().setVisibility(View.INVISIBLE);
        }
    }

    @Override
    public void onTouch(MotionEvent motionEvent, float viewScreenX, float viewScreenY){

    }

    @Override
    public void onPositionChange(float x, float y, float width, float height){

    }
}
