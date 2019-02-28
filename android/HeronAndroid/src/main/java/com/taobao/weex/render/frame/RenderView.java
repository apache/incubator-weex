package com.taobao.weex.render.frame;

import android.content.Context;
import android.graphics.PixelFormat;
import android.graphics.SurfaceTexture;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.TextureView;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;

import com.taobao.weex.render.bridge.WXRenderUI;
import com.taobao.weex.render.manager.RenderManager;
import com.taobao.weex.render.platform.PlatformViewManager;
import com.taobao.weex.render.platform.TextureViewManager;

public class RenderView extends FrameLayout  implements  TextureView.SurfaceTextureListener, SurfaceHolder.Callback{


    private SurfaceTextureHolder mSurfaceTextureHolder;
    private String mRef;
    private RenderFrame mRenderFrame;
    private View mSurfaceView;
    private PlatformViewManager mPlatformViewManager;
    private TextureViewManager mTextureViewManager;
    private boolean mUseSurfaceView = true;
    private boolean mDestroy = false;


    public RenderView(@NonNull Context context) {
        super(context);
    }

    public RenderView(@NonNull Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
    }

    public RenderView(@NonNull Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    public RenderView(@NonNull Context context, @Nullable AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
    }

    public void createSurfaceView() {
        if(mUseSurfaceView){
            RenderSurfaceView surfaceView = new RenderSurfaceView(getContext());
            surfaceView.setZOrderOnTop(true);
            surfaceView.getHolder().setFormat(PixelFormat.TRANSPARENT);
            surfaceView.getHolder().addCallback(this);
            mSurfaceView = surfaceView;
            addView(surfaceView, new LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT));
        }else{
            RenderTextureView renderFrameView = new RenderTextureView(getContext());
            renderFrameView.setSurfaceTextureListener(this);
            mSurfaceView = renderFrameView;
            addView(renderFrameView, new LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT));
        }
        mRenderFrame = new RenderFrame(this,getContext());
        if(mRef != null){
            RenderManager.getInstance().registerRenderView(mRef, mRenderFrame);
        }
    }


    @Override
    public void onSurfaceTextureAvailable(SurfaceTexture surfaceTexture, int width, int height) {
        mSurfaceTextureHolder = new SurfaceTextureHolder(surfaceTexture, width, height);
        mSurfaceTextureHolder.createSurface();
        mSurfaceTextureHolder.setNativeWindow(WXRenderUI.getInstance().frameRenderAttach(mRef, mSurfaceTextureHolder.getSurface(), width, height, false));
    }

    @Override
    public void onSurfaceTextureSizeChanged(SurfaceTexture surfaceTexture, int width, int height) {
        if(mSurfaceTextureHolder != null){
            WXRenderUI.getInstance().frameRenderSizeChanged(mRef,  width, height);
        }
    }

    @Override
    public void onSurfaceTextureUpdated(SurfaceTexture surfaceTexture) {

    }

    @Override
    public boolean onSurfaceTextureDestroyed(SurfaceTexture surfaceTexture) {
        if(mSurfaceTextureHolder != null){
            WXRenderUI.getInstance().frameRenderDettach(mRef, mSurfaceTextureHolder.getSurface(), mSurfaceTextureHolder.getNativeWindow());
            mSurfaceTextureHolder = null;
        }
        return true;
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {

    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        if(mSurfaceTextureHolder == null){
            mSurfaceTextureHolder = new SurfaceTextureHolder(null, width, height);
            mSurfaceTextureHolder.setSurface(holder.getSurface());
            mSurfaceTextureHolder.setNativeWindow(WXRenderUI.getInstance().frameRenderAttach(mRef, mSurfaceTextureHolder.getSurface(), width, height, true));
        }else{
            WXRenderUI.getInstance().frameRenderSizeChanged(mRef,  width, height);
        }
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        if(mSurfaceTextureHolder != null){
            WXRenderUI.getInstance().frameRenderDettach(mRef, mSurfaceTextureHolder.getSurface(), mSurfaceTextureHolder.getNativeWindow());
            mSurfaceTextureHolder = null;
        }
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        if(mSurfaceTextureHolder != null){
            WXRenderUI.getInstance().frameRenderOnTouchEvent(mRef,  event);
        }
        super.onTouchEvent(event);
        return true;
    }

    public void frameOnResume() {
        if(mSurfaceTextureHolder != null){
            WXRenderUI.getInstance().frameRenderOnResume(mRef);
        }
        if(mPlatformViewManager != null){
            mPlatformViewManager.onResume();
        }
    }

    public void frameOnPause() {
        if(mSurfaceTextureHolder != null){
            WXRenderUI.getInstance().frameRenderOnPause(mRef);
        }
        if(mPlatformViewManager != null){
            mPlatformViewManager.onPause();
        }
    }

    public void frameOnLowMemory() {
        if(mSurfaceTextureHolder != null){
            WXRenderUI.getInstance().frameRenderOnLowMemory(mRef);
        }
    }



    public void onDestroy(){
        if(!mDestroy){
            mDestroy = true;
            if(mPlatformViewManager != null){
                mPlatformViewManager.onDestory();
            }
            if(mTextureViewManager != null){
                mTextureViewManager.onDestroy();
            }
            RenderManager.getInstance().removeFrame(mRef);
        }
    }

    public PlatformViewManager getPlatformViewManager() {
        if(mPlatformViewManager == null){
            mPlatformViewManager = new PlatformViewManager(this);
        }
        return mPlatformViewManager;
    }

    public TextureViewManager getTextureViewManager(){
        if(mTextureViewManager == null){
            mTextureViewManager = new TextureViewManager(this);
        }
        return mTextureViewManager;
    }


    public String getRef() {
        return mRef;
    }

    public void setRef(String ref) {
        if(mRef != null){
            RenderManager.getInstance().removeFrame(mRef);
        }
        this.mRef = ref;
        if(mRenderFrame != null){
            RenderManager.getInstance().registerRenderView(mRef, mRenderFrame);
        }
    }

    public boolean isUseSurfaceView() {
        return mUseSurfaceView;
    }

    public void setUseSurfaceView(boolean useSurfaceView) {
        this.mUseSurfaceView = useSurfaceView;
    }

    public View getSurfaceView() {
        return mSurfaceView;
    }
}
