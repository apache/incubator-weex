package com.taobao.weex.render.platform.view.video;

import android.content.Context;
import android.util.AttributeSet;
import android.view.View;
import android.widget.MediaController;
import android.widget.VideoView;

public class RMediaController extends MediaController {

    private boolean hasTouch;
    private View mArchorView;
    private RVideoView mVideoView;

    public RMediaController(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public RMediaController(Context context, boolean useFastForward) {
        super(context, useFastForward);
    }

    public RMediaController(Context context) {
        super(context);
    }

    @Override
    public void onFinishInflate() {
        super.onFinishInflate();
    }

    @Override
    public void show(int timeout) {
        if(mVideoView != null && mVideoView.isPlaying()){
            //return;
        }
        try {
            super.show(timeout);
        }catch (Exception e){}
    }

    @Override
    public void hide() {
        super.hide();
    }

    public boolean isHasTouch() {
        return hasTouch;
    }

    public void setHasTouch(boolean hasTouch) {
        this.hasTouch = hasTouch;
    }



    @Override
    public void setAnchorView(View view) {
        mArchorView = view;
        super.setAnchorView(view);
    }

    public View getArchorView() {
        return mArchorView;
    }

    public RVideoView getVideoView() {
        return mVideoView;
    }

    public void setVideoView(RVideoView mVideoView) {
        this.mVideoView = mVideoView;
    }
}
