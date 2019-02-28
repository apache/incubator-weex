package com.taobao.weex.render.platform.texture;

import android.annotation.TargetApi;
import android.app.Presentation;
import android.content.Context;
import android.graphics.SurfaceTexture;
import android.hardware.display.DisplayManager;
import android.hardware.display.VirtualDisplay;
import android.os.Build;
import android.support.annotation.RequiresApi;
import android.util.DisplayMetrics;
import android.view.Display;
import android.view.Surface;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewParent;
import android.widget.FrameLayout;
import com.taobao.weex.render.bridge.WXTextureBridge;
import com.taobao.weex.render.frame.RenderView;
import com.taobao.weex.render.platform.view.video.RVideoPlayer;

import java.util.concurrent.atomic.AtomicInteger;

public class NativeViewTextureRender implements SurfaceTexture.OnFrameAvailableListener{

    private static AtomicInteger mAtomicInt = new AtomicInteger(0);
    private TextureComponent mTextureComponent;
    private RenderView mRenderView;
    private VirtualDisplay mVirtualDisplay;
    private FrameLayout mRootView;
    private SurfaceTexture mSurfaceTexture;
    private Surface mSurface;
    private Presentation mPresentation;
    private View mNativeView;



    /**
     * Render All The View To SurfaceTexture
     * */
    public NativeViewTextureRender(RenderView renderView, TextureComponent textureComponent) {
        this.mRenderView = renderView;
        this.mTextureComponent = textureComponent;
    }

    @RequiresApi(api = Build.VERSION_CODES.KITKAT)
    public void setViewRenderTexture(SurfaceTexture surfaceTexture){
        mSurfaceTexture  = surfaceTexture;
        mSurfaceTexture.setOnFrameAvailableListener(this);
        mSurface = new Surface(mSurfaceTexture);
    }

    private View createNativeView(Context context) {
       return  mTextureComponent.createNativeView(context);
    }

    @TargetApi(Build.VERSION_CODES.KITKAT)
    @RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN_MR1)
    public void createViewTextureFrame(int width, int height) {

        if(mPresentation != null){
            mPresentation.hide();
        }
        if(mVirtualDisplay != null){
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
                mVirtualDisplay.release();
            }
            mVirtualDisplay = null;
        }
        DisplayManager displayManager = (DisplayManager) mRenderView.getContext().getSystemService(Context.DISPLAY_SERVICE);
        DisplayMetrics displayMetrics = mRenderView.getResources().getDisplayMetrics();


        mSurfaceTexture.setDefaultBufferSize(width, height);
        mVirtualDisplay = displayManager.createVirtualDisplay("Weex-VD" + mAtomicInt.incrementAndGet(), width, height,
                displayMetrics.densityDpi, mSurface, 0);
        mPresentation = new Presentation(mRenderView.getContext(), mVirtualDisplay.getDisplay());

        mNativeView = createNativeView(mPresentation.getContext());
        mRootView = new FrameLayout(mPresentation.getContext());
        mRootView.addView(mNativeView, new FrameLayout.LayoutParams(FrameLayout.LayoutParams.MATCH_PARENT, FrameLayout.LayoutParams.MATCH_PARENT));

        ViewParent viewParent = mRootView.getParent();
        if(viewParent instanceof ViewGroup){
            ((ViewGroup) viewParent).removeView(mRootView);
        }
        mPresentation.setContentView(mRootView);
        mPresentation.show();
    }

    public FrameLayout getRootView() {
        return mRootView;
    }

    public SurfaceTexture getSurfaceTexture() {
        return mSurfaceTexture;
    }

    @Override
    public void onFrameAvailable(SurfaceTexture surfaceTexture) {
        WXTextureBridge.updatePlatformTexture(mRenderView.getRef(), mTextureComponent.getRef());
    }

    public void onDestroy() {
        if(mPresentation != null){
            mPresentation.hide();
        }
        if(mVirtualDisplay != null){
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
                mVirtualDisplay.release();
            }
        }
        mSurface.release();
        //mSurfaceTexture.release();
    }
}
