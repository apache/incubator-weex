package com.taobao.weex.render.platform.view.video;

import android.content.Context;
import android.graphics.PixelFormat;
import android.util.AttributeSet;
import android.widget.VideoView;

public class RVideoView extends TextureVideoView {

    private VideoPlayListener mVideoPauseListener;


    public RVideoView(Context context) {
        super(context);
    }

    public RVideoView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public RVideoView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    public RVideoView(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
    }

    public void setOnVideoPauseListener(VideoPlayListener listener) {
        mVideoPauseListener = listener;
    }


    @Override
    public void start() {
        super.start();
        if (mVideoPauseListener != null) {
            mVideoPauseListener.onStart();
        }
    }

    @Override
    public void pause() {
        super.pause();
        if (mVideoPauseListener != null) {
            mVideoPauseListener.onPause();
        }
    }



    public interface VideoPlayListener {

        void onPause();

        void onStart();
    }
}
