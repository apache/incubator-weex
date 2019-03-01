package com.taobao.weex.render.bridge;

import android.graphics.SurfaceTexture;
import android.os.SystemClock;
import android.text.TextUtils;
import android.view.MotionEvent;

import com.taobao.weex.base.CalledByNative;
import com.taobao.weex.render.frame.RenderFrame;
import com.taobao.weex.render.frame.RenderView;
import com.taobao.weex.render.frame.TouchHelper;
import com.taobao.weex.render.manager.RenderManager;
import com.taobao.weex.render.platform.texture.TextureComponent;
import com.taobao.weex.render.platform.texture.TextureFactory;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

public class WXTextureBridge {


    /**
     * return create The SurfaceTexture.
     * */
    @CalledByNative
    public static Object createNativeView(long nativeWindow, final String pageId,
                                          final String ref, final String type, final Map<String,String> attrs,
                                          final Map<String,String> styles,
                                          final Set<String> eventsSet){

        final SurfaceTexture surfaceTexture = TextureFactory.createSurfaceTexture();
        RenderManager.getInstance().getUiHandler().post(new Runnable() {
            @Override
            public void run() {
                UTThreadSetViewRenderToTexture(surfaceTexture, pageId, ref, type, attrs, styles, eventsSet);
            }
        });
        return surfaceTexture;
    }

    private static void UTThreadSetViewRenderToTexture(SurfaceTexture surfaceTexture, String pageId,
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
        renderView.getTextureViewManager().createTextureView(ref, type, attrs, styles, eventsList);
        TextureComponent textureComponent = renderView.getTextureViewManager().getView(ref);
        textureComponent.setViewRenderTexture(surfaceTexture);
    }

    @CalledByNative
    public static void onUpdateTextureFrame(final String pageId, final String ref, final float x, final float y, final float width, final float height) {
        RenderManager.getInstance().getUiHandler().post(new Runnable() {
            @Override
            public void run() {
               UIThreadOnUpdateTextureFrame(pageId, ref, x, y, width, height);
            }
        });
    }

    private static void UIThreadOnUpdateTextureFrame(String pageId, final String ref, final float x, final float y, final float width, final float height){
        final RenderFrame renderFrame = RenderManager.getInstance().getRenderView(pageId);
        if(renderFrame == null){
            return;
        }
        final RenderView renderView = renderFrame.getRenderView();
        if(renderView == null){
            return;
        }
        if(!renderView.getTextureViewManager().needUpdate(ref, x, y, width, height)){
            return;
        }
        renderView.getTextureViewManager().onUpdateTextureFrame(ref, x, y, width, height);
    }


    @CalledByNative
    public static void destroyNativeView(final long nativeWindow, final String pageId, final String ref){
        RenderManager.getInstance().getUiHandler().post(new Runnable() {
            @Override
            public void run() {
                UIThreadDestroyNativeView(nativeWindow, pageId, ref);
            }
        });
    }

    private static void UIThreadDestroyNativeView(long nativeWindow, String pageId, final String ref){
        final RenderFrame renderFrame = RenderManager.getInstance().getRenderView(pageId);
        if(renderFrame == null){
            return;
        }
        final RenderView renderView = renderFrame.getRenderView();
        if(renderView == null){
            return;
        }
        renderView.getTextureViewManager().removeTexture(ref);
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
    public static void onTextureShow(String pageId, final String ref){

    }

    @CalledByNative
    public static void onTextureHide(String pageId, final String ref){

    }

    @CalledByNative
    public static void onTextureTouch(String pageId, final String ref, int actionAgi, float x, float y,
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
                renderView.getTextureViewManager().onTextureTouch(ref, motionEvent, viewScreenX, viewScreenY);
            }
        });
    }

    public static void updatePlatformTexture(String pageId, String ref){
        nativeUpdatePlatformTexture(pageId, ref);
    }

    private static native void nativeUpdatePlatformTexture(String pageId, String ref);

    private static void flushMainThread(){
        final CountDownLatch countDownLatch = new CountDownLatch(1);
        RenderManager.getInstance().getUiHandler().post(new Runnable() {
            @Override
            public void run() {
                 countDownLatch.countDown();
            }
        });
        try {
            countDownLatch.await(100, TimeUnit.MILLISECONDS);
            Thread.sleep(100);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

}
