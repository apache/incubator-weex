package com.taobao.weex.render.platform;

import android.view.MotionEvent;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewParent;
import android.widget.FrameLayout;

import com.taobao.weex.render.frame.RenderView;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;

public class PlatformViewManager {

    private Map<String, PlatformView> mPlatformViews;
    private Map<String, ViewTransformUpdate> mPlatformViewParams;
    private RenderView mRenderView;

    public PlatformViewManager(RenderView renderView) {
        this.mPlatformViews = new HashMap<>();
        this.mPlatformViewParams = new HashMap<>();
        this.mRenderView = renderView;
    }

    public void paintPlatformView(String ref, float x, float y, float width, float height){
        PlatformView view = mPlatformViews.get(ref);
        if(view  == null){
            return;
        }
       if(view.getView().getParent() == null){
            FrameLayout.LayoutParams params = new FrameLayout.LayoutParams((int)width, (int)height);
            params.leftMargin = (int)x;
            params.topMargin = (int)y;
            params.width = (int)width;
            params.height = (int)height;
            mRenderView.addView(view.getView(), 0, params);
            mPlatformViews.put(ref, view);
            mRenderView.getSurfaceView().bringToFront();
            view.onAttach();
           if(canSetHardwareLayerType(view.getView())){
               view.getView().setLayerType(View.LAYER_TYPE_HARDWARE, null);
           }
            return;
        }
        FrameLayout.LayoutParams params = (FrameLayout.LayoutParams) view.getView().getLayoutParams();
        if(params.width != ((int)width) || params.height != ((int)height)){
            params.width = (int)width;
            params.height = (int)height;
            view.getView().setLayoutParams(params);
        }
        view.getView().setTranslationX(x - params.leftMargin);
        view.getView().setTranslationY(y - params.topMargin);
        ViewTransformUpdate rect = mPlatformViewParams.get(ref);
        if(hasPositionUpdate(rect, x, y, width, height)){
            view.onPositionChange(x, y, width, height);
        }
        rect.setUpdate(x, y, width, height);
    }

    public boolean hasTransformUpdate(String ref, float x, float y, float width, float height){
        ViewTransformUpdate params = mPlatformViewParams.get(ref);
        if(params == null){
            params = new ViewTransformUpdate();
            mPlatformViewParams.put(ref, params);
            return true;
        }
        if(params.width != width || params.height != height){
            return true;
        }
        if(params.y != y || params.x != x){
            return true;
        }
        if(!mRenderView.isUseSurfaceView()){
            return false;
        }
        return true;
    }


    public boolean hasPositionUpdate(ViewTransformUpdate params, float x, float y, float width, float height) {
        if(params.width == 0 || params.height == 0){
            return false;
        }
        if(params.width != width || params.height != height){
            return true;
        }
        if(params.y != y || params.x != x){
            return true;
        }
        return false;
    }

    public void createPlatformView(String ref, String type, Map<String,String> attrs,
                                   Map<String,String> styles, List<String> events) {
        PlatformView view = mPlatformViews.get(ref);
        if(view == null){
            view = PlatformViewFactory.createView(mRenderView, ref, type, attrs, styles, events);
        }
        mPlatformViews.put(ref, view);
    }

    public PlatformView getView(String ref){
        return mPlatformViews.get(ref);
    }

    public void removePlatformView(String ref){
        PlatformView view = mPlatformViews.remove(ref);
        if(view != null){
            ViewParent parent = view.getView().getParent();
            if(parent instanceof  ViewGroup){
                ((ViewGroup)parent).removeView(view.getView());
            }
            view.onDestroy();
        }
    }

    public void updateViewStyles(String ref, Map<String,String> styles) {
        PlatformView view = mPlatformViews.remove(ref);
        if(view != null){
            view.onUpdateStyle(styles);
        }
    }

    public void updateViewAttrs(String ref, Map<String,String> attrs){
        PlatformView view = mPlatformViews.remove(ref);
        if(view != null){
            view.onUpdateAttr(attrs);
        }
    }

    public void addViewEvent(String ref, String event){
        PlatformView view = mPlatformViews.remove(ref);
        if(view != null){
            view.onAddEvent(event);
        }
    }

    public void removeViewEvent(String ref, String event){
        PlatformView view = mPlatformViews.remove(ref);
        if(view != null){
            view.onRemoveEvent(event);
        }
    }

    public void onPause(){
        Set<Map.Entry<String,PlatformView>> platformViews = mPlatformViews.entrySet();
        for(Map.Entry<String,PlatformView> platformViewEntry : platformViews){
            if(platformViewEntry.getValue() != null){
                platformViewEntry.getValue().onPause();
            }
        }
    }

    public void onResume(){
        Set<Map.Entry<String,PlatformView>> platformViews = mPlatformViews.entrySet();
        for(Map.Entry<String,PlatformView> platformViewEntry : platformViews){
            if(platformViewEntry.getValue() != null){
                platformViewEntry.getValue().onResume();
            }
        }
    }

    public void onDestory(){
        Set<Map.Entry<String,PlatformView>> platformViews = mPlatformViews.entrySet();
        for(Map.Entry<String,PlatformView> platformViewEntry : platformViews){
            if(platformViewEntry.getValue() != null){
                platformViewEntry.getValue().onDestroy();
            }
        }
    }

    public void onNativeViewTouch(String ref, MotionEvent motionEvent, float viewScreenX, float viewScreenY) {
        PlatformView view = mPlatformViews.get(ref);
        if(view != null){
            view.onTouch(motionEvent, viewScreenX, viewScreenY);
        }
    }

    private boolean canSetHardwareLayerType(View view) {
        if(view instanceof SurfaceView){
            return false;
        }
        if(view instanceof ViewGroup){
            ViewGroup viewGroup = (ViewGroup) view;
            for(int i=0; i<viewGroup.getChildCount(); i++){
                View child = viewGroup.getChildAt(i);
                if(!canSetHardwareLayerType(child)){
                    return false;
                }
            }
        }
        return true;
    }


}
