package com.taobao.weex.render.bridge;

import android.os.SystemClock;
import android.text.TextUtils;
import android.util.Log;
import android.view.Choreographer;
import android.view.MotionEvent;
import android.view.ViewGroup;
import android.view.ViewParent;

import com.taobao.weex.base.CalledByNative;
import com.taobao.weex.render.frame.RenderFrame;
import com.taobao.weex.render.frame.RenderView;
import com.taobao.weex.render.frame.TouchHelper;
import com.taobao.weex.render.manager.RenderManager;
import com.taobao.weex.render.platform.PlatformView;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

public class WXPlatformNativeViewBridge {


    @CalledByNative
    public static void createNativeView(final long nativeWindow, final String pageId,
                                        final String ref, final String type, final Map<String,String> attrs,
                                        final Map<String,String> styles,
                                        final Set<String> eventsSet){

        RenderManager.getInstance().getUiHandler().postAtFrontOfQueue(new Runnable() {
            @Override
            public void run() {
                  UiThreadCreateNativeView(nativeWindow, pageId, ref, type, attrs, styles, eventsSet);
            }
        });
    }

    private static void UiThreadCreateNativeView(long nativeWindow, String pageId,
                                                 final String ref, final String type, final Map<String,String> attrs,
                                                 final Map<String,String> styles,
                                                 final Set<String> eventsSet){
        final RenderFrame renderFrame = RenderManager.getInstance().getRenderView(pageId);
        if(renderFrame == null){
            return;
        }
        final RenderView renderView = renderFrame.getRenderView();
        if(renderView == null){
            return;
        }
        final List eventsList =  new ArrayList();
        if(eventsSet != null){
            eventsList.addAll(eventsSet);
        }
        renderView.getPlatformViewManager().createPlatformView(ref, type, attrs, styles, eventsList);
    }

    @CalledByNative
    public static void destroyNativeView(long nativeWindow, String pageId, final String ref){
        final RenderFrame renderFrame = RenderManager.getInstance().getRenderView(pageId);
        if(renderFrame == null){
            return;
        }
        final RenderView renderView = renderFrame.getRenderView();
        if(renderView == null){
            return;
        }
        RenderManager.getInstance().getUiHandler().post(new Runnable() {
            @Override
            public void run() {
                renderView.getPlatformViewManager().removePlatformView(ref);
            }
        });
    }

    @CalledByNative
    public static void updateNativeViewStyles(long nativeWindow, String pageId, final String ref, final Map<String,String> styles){
        if(styles == null){
            return;
        }
        final RenderFrame renderFrame = RenderManager.getInstance().getRenderView(pageId);
        if(renderFrame == null){
            return;
        }
        final RenderView renderView = renderFrame.getRenderView();
        if(renderView == null){
            return;
        }
        RenderManager.getInstance().getUiHandler().post(new Runnable() {
            @Override
            public void run() {
                renderView.getPlatformViewManager().updateViewStyles(ref, styles);
            }
        });
    }

    @CalledByNative
    public static void updateNativeViewAttrs(long nativeWindow, String pageId, final String ref, final Map<String,String> attrs){
        if(attrs == null){
            return;
        }
        final RenderFrame renderFrame = RenderManager.getInstance().getRenderView(pageId);
        if(renderFrame == null){
            return;
        }
        final RenderView renderView = renderFrame.getRenderView();
        if(renderView == null){
            return;
        }
        RenderManager.getInstance().getUiHandler().post(new Runnable() {
            @Override
            public void run() {
                renderView.getPlatformViewManager().updateViewAttrs(ref, attrs);
            }
        });
    }


    @CalledByNative
    public static void addNativeViewEvent(long nativeWindow, String pageId, final String ref, final String event){
        if(TextUtils.isEmpty(event)){
            return;
        }
        final RenderFrame renderFrame = RenderManager.getInstance().getRenderView(pageId);
        if(renderFrame  == null){
            return;
        }
        final RenderView renderView = renderFrame.getRenderView();
        if(renderView == null){
            return;
        }
        RenderManager.getInstance().getUiHandler().post(new Runnable() {
            @Override
            public void run() {
                renderView.getPlatformViewManager().addViewEvent(ref, event);
            }
        });
    }

    @CalledByNative
    public static void removeNativeViewEvent(long nativeWindow, String pageId, final String ref, final String event){
        final RenderFrame renderFrame = RenderManager.getInstance().getRenderView(pageId);
        if(renderFrame == null){
            return;
        }
        final RenderView renderView = renderFrame.getRenderView();
        if(renderView == null){
            return;
        }
        RenderManager.getInstance().getUiHandler().post(new Runnable() {
            @Override
            public void run() {
                renderView.getPlatformViewManager().removeViewEvent(ref, event);
            }
        });
    }


    @CalledByNative
    public static void onNativeViewShow(String pageId, final String ref){

    }

    @CalledByNative
    public static void onNativeViewHide(String pageId, final String ref){

    }

    @CalledByNative
    public static void onNativeViewTouch(String pageId, final String ref, int actionAgi, float x, float y,
                                      final float viewScreenX, final float viewScreenY){

        int action = TouchHelper.convertRenderToNative(actionAgi);
        final MotionEvent motionEvent = MotionEvent.obtain(SystemClock.uptimeMillis(), SystemClock.uptimeMillis(),
                action, x, y, 0);
        final RenderFrame renderFrame = RenderManager.getInstance().getRenderView(pageId);
        if(renderFrame == null){
            return;
        }
        final RenderView renderView = renderFrame.getRenderView();
        if(renderView == null){
            return;
        }
        RenderManager.getInstance().getUiHandler().postAtFrontOfQueue(new Runnable() {
            @Override
            public void run() {
                renderView.getPlatformViewManager().onNativeViewTouch(ref, motionEvent, viewScreenX, viewScreenY);
            }
        });

    }


    

    @CalledByNative
    public static void transformNativeView(String pageId, final String ref, final float x, final float y, final float width, final float height,
                                           final boolean dettached) {
        final RenderFrame renderFrame = RenderManager.getInstance().getRenderView(pageId);
        if(renderFrame == null){
            return;
        }
        final RenderView renderView = renderFrame.getRenderView();
        if(renderView == null){
            return;
        }
        if(dettached){
            RenderManager.getInstance().getUiHandler().post(new Runnable() {
                @Override
                public void run() {
                    /**
                    PlatformView platformView =  renderView.getPlatformViewManager().getView(ref);
                    platformView.onDetach();
                    if(platformView.getView() != null){
                        ViewParent parent = platformView.getView().getParent();
                        if(parent != null && parent instanceof ViewGroup){
                            ((ViewGroup) parent).removeView(platformView.getView());
                        }
                    }*/
                }
            });
            return;
        }

        if(!renderView.getPlatformViewManager().hasTransformUpdate(ref, x, y, width, height)){
            return;
        }

        final CountDownLatch gpuThreadWaitCountDown = new CountDownLatch(1);
        RenderManager.getInstance().getUiHandler().post(new Runnable() {
            @Override
            public void run() {
                renderView.getPlatformViewManager().paintPlatformView(ref, x, y, width, height);
                gpuThreadWaitCountDown.countDown();

            }
        });
        try {
            gpuThreadWaitCountDown.await(64, TimeUnit.MILLISECONDS);
        } catch (Exception e) {}
    }

    @CalledByNative
    public static void onPresentEnd(){
    }
}
