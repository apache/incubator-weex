package com.taobao.weex.render.platform.texture;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.view.MotionEvent;
import android.view.View;
import android.widget.FrameLayout;

import com.taobao.weex.render.frame.RenderView;
import com.taobao.weex.render.platform.component.Component;

import java.util.List;
import java.util.Map;

public abstract class TextureComponent extends Component {

    private MirrorView mMirrorView;
    private NativeViewTextureRender mNativeViewRender;
    protected View mNativeView;
    private float mWidth;
    private float mHeight;

    @Override
    public void createView(RenderView renderView, Map<String, String> attrs, Map<String, String> styles, List<String> events) {
        super.init(renderView, attrs, styles, events);
        mMirrorView = new MirrorView(renderView.getContext());
    }

    @Override
    public View getView() {
        return mMirrorView;
    }


    @Override
    public void onAttach() {

    }

    @Override
    public void onUpdateAttr(Map<String, String> attrs) {

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

    }

    @Override
    public void onPause() {

    }

    @Override
    public void onShow(){

    }

    @Override
    public void onHide(){

    }


    public abstract View createNativeView(Context context);



    public void onUpdateTextureFrame(float x, float y, float width, float height) {
        mWidth = width;
        mHeight = height;
        if(mMirrorView.getParent() == null){
            updateMirrorView((int)x, (int)y, (int)width, (int)height);
        }
        mNativeViewRender.createViewTextureFrame((int)width, (int)height);
    }


    public void onTextureTouch(MotionEvent motionEvent, float x, float y){

    }

    private void updateMirrorView(int x, int y ,int width, int height){

    }

    public void setViewRenderTexture(SurfaceTexture surfaceTexture) {
        mNativeViewRender = new NativeViewTextureRender(getRenderView(), this);
        mNativeViewRender.setViewRenderTexture(surfaceTexture);
    }


    @Override
    public void onDestroy() {
        if(mNativeViewRender != null){
            mNativeViewRender.onDestroy();
            mNativeViewRender = null;
        }
    }



}
