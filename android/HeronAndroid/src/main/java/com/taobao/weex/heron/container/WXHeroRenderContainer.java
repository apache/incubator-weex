package com.taobao.weex.heron.container;

import android.content.Context;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.util.AttributeSet;
import android.view.ViewGroup;

import com.taobao.weex.render.AbstractRenderContainer;
import com.taobao.weex.render.component.WXRenderFrameComponent;
import com.taobao.weex.render.frame.RenderView;
import com.taobao.weex.ui.action.BasicComponentData;

public class WXHeroRenderContainer extends AbstractRenderContainer {

    private RenderView mRenderView;

    public WXHeroRenderContainer(@NonNull Context context) {
        super(context);
    }

    public WXHeroRenderContainer(@NonNull Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
    }

    public WXHeroRenderContainer(@NonNull Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    public WXHeroRenderContainer(@NonNull Context context, @Nullable AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
    }

    public void createInstanceRenderView(String instanceId){
        mRenderView = new RenderView(getContext());
        mRenderView.setRef(instanceId);
        mRenderView.createSurfaceView();
        addView(mRenderView, new LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT));
    }
}
