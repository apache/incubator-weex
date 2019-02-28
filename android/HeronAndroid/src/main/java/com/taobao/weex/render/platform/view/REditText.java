package com.taobao.weex.render.platform.view;

import android.content.Context;
import android.os.Build;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.widget.EditText;

import com.taobao.weex.render.frame.RenderView;

public class REditText extends EditText {

    private RenderView mRenderView;

    public REditText(Context context) {
        super(context);
        initView();
    }

    public REditText(Context context, AttributeSet attrs) {
        super(context, attrs);
        initView();
    }

    public REditText(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        initView();
    }


    public REditText(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
        initView();
    }


    private void initView() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN) {
            setBackground(null);
        } else {
            setBackgroundDrawable(null);
        }
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        if(mRenderView != null){
            mRenderView.onTouchEvent(event);
        }
        return super.onTouchEvent(event) && false;
    }


    public void setRenderView(RenderView renderView) {
        mRenderView = renderView;
    }
}
