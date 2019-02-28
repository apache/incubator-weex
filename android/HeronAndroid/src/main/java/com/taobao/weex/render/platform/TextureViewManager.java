package com.taobao.weex.render.platform;

import android.graphics.RectF;
import android.view.MotionEvent;
import android.view.ViewGroup;
import android.view.ViewParent;

import com.taobao.weex.render.frame.RenderView;
import com.taobao.weex.render.platform.texture.TextureComponent;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class TextureViewManager {

    private Map<String, TextureComponent> mTextureViews;
    private Map<String, RectF> mTextureParams;
    private RenderView mRenderView;

    public TextureViewManager(RenderView renderView) {
        this.mRenderView = renderView;
        this.mTextureViews = new HashMap<>();
        this.mTextureParams = new HashMap<>();
    }


    public void createTextureView(String ref, String type, Map<String,String> attrs,
                                   Map<String,String> styles, List<String> events) {
        PlatformView view = mTextureViews.get(ref);
        if(view == null){
            view = PlatformViewFactory.createView(mRenderView, ref, type, attrs, styles, events);
        }
        if(view instanceof TextureComponent){
            mTextureViews.put(ref, (TextureComponent) view);
        }
    }

    public TextureComponent getView(String ref){
        return mTextureViews.get(ref);
    }


    public boolean needUpdate(String ref, float x, float y, float width, float height) {
        RectF params = mTextureParams.get(ref);
        if(params == null){
            params = new RectF();
            mTextureParams.put(ref, params);
        }
        if(params.width() != width || params.height() != height){
            return true;
        }
        if(params.top != y || params.left != x){
            return true;
        }
        return false;
    }

    public void onUpdateTextureFrame(String ref, float x, float y, float width, float height) {
        RectF rect = mTextureParams.get(ref);
        rect.set(x, y, x+width, y + height);
        TextureComponent textureComponent = mTextureViews.get(ref);
        if(textureComponent  == null){
            return;
        }
        textureComponent.onUpdateTextureFrame(x, y, width, height);
    }

    public void onTextureTouch(String ref, MotionEvent motionEvent, float viewScreenX, float viewScreenY) {
        TextureComponent view = mTextureViews.get(ref);
        if(view != null){
            view.onTextureTouch(motionEvent, viewScreenX, viewScreenY);
        }
    }

    public void removeTexture(String ref){
        TextureComponent view = mTextureViews.remove(ref);
        if(view != null){
            ViewParent parent = view.getView().getParent();
            if(parent instanceof ViewGroup){
                ((ViewGroup)parent).removeView(view.getView());
            }
            view.onDestroy();
        }
    }

    public void onDestroy() {
        Object[] keySets = mTextureViews.keySet().toArray();
        for(Object keySet : keySets){
            if(keySet != null){
                removeTexture(keySet.toString());
            }
        }
    }
}
