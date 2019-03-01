package com.taobao.weex.render.platform.view.video;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Color;
import android.graphics.Rect;
import android.media.MediaPlayer;
import android.net.Uri;
import android.os.Build;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.util.AttributeSet;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewTreeObserver;
import android.widget.FrameLayout;
import android.widget.MediaController;
import android.widget.ProgressBar;

import com.taobao.weex.render.frame.RenderView;

public class RVideoPlayer extends FrameLayout implements ViewTreeObserver.OnGlobalLayoutListener{

    private RVideoView mVideoView;
    private ProgressBar mProgressBar;
    private RMediaController mMediaController;
    private Uri mUri;
    private MediaPlayer.OnPreparedListener mOnPreparedListener;
    private MediaPlayer.OnErrorListener mOnErrorListener;
    private MediaPlayer.OnCompletionListener mOnCompletionListener;
    private RVideoView.VideoPlayListener mVideoPlayListener;
    private RenderView mRenderView;


    public RVideoPlayer(Context context) {
        super(context);
        init(context);
    }

    public RVideoPlayer(Context context, AttributeSet attrs) {
        super(context, attrs);
        init(context);
    }

    public RVideoPlayer(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init(context);
    }

    private void init(Context context) {
        setBackgroundColor(Color.parseColor("#ee000000"));
        mProgressBar = new ProgressBar(context);
        LayoutParams pLayoutParams =
                new LayoutParams(LayoutParams.WRAP_CONTENT,
                        LayoutParams.WRAP_CONTENT);
        mProgressBar.setLayoutParams(pLayoutParams);
        pLayoutParams.gravity = Gravity.CENTER;
        addView(mProgressBar);

        getViewTreeObserver().addOnGlobalLayoutListener(this);
    }


    @Override
    public boolean dispatchTouchEvent(MotionEvent ev) {
        return false;
    }

    public boolean dispatchRenderTouchEvent(MotionEvent ev){
        if(mMediaController != null){
            mMediaController.setHasTouch(true);
        }
        return super.dispatchTouchEvent(ev);
    }

    public ProgressBar getProgressBar() {
        return mProgressBar;
    }

    public RVideoView getVideoView() {
        return mVideoView;
    }

    /**
     * Create if not existed. Will cause request focus.
     *
     * @return
     */
    public @NonNull
    RVideoView createIfNotExist() {
        if (mVideoView == null) {
            createVideoView();
        }
        return mVideoView;
    }

    public
    @Nullable
    MediaController getMediaController() {
        return mMediaController;
    }

    public void setVideoURI(Uri uri) {
        mUri = uri;
        if (mVideoView != null) {
            mVideoView.setVideoURI(uri);
        }
    }

    public void start() {
        if (mVideoView != null) {
            mVideoView.start();
            mProgressBar.setVisibility(View.GONE);
        }
    }

    public void pause() {
        if (mVideoView != null) {
            mVideoView.pause();
        }
    }

    public void stopPlayback() {
        if (mVideoView != null) {
            mVideoView.stopPlayback();
        }
    }

    public void resume() {
        if (mVideoView != null) {
            mVideoView.resume();
        }
    }

    public void setOnErrorListener(MediaPlayer.OnErrorListener l) {
        mOnErrorListener = l;
        if (mVideoView != null) {
            mVideoView.setOnErrorListener(l);
        }
    }

    public void setOnPreparedListener(MediaPlayer.OnPreparedListener l) {
        mOnPreparedListener = l;
        if (mVideoView != null) {
            mVideoView.setOnPreparedListener(l);
        }

    }

    public void setOnCompletionListener(MediaPlayer.OnCompletionListener l) {
        mOnCompletionListener = l;
        if (mVideoView != null) {
            mVideoView.setOnCompletionListener(l);
        }
    }

    public void setOnVideoPauseListener(RVideoView.VideoPlayListener listener) {
        mVideoPlayListener = listener;
        if (mVideoView != null) {
            mVideoView.setOnVideoPauseListener(listener);
        }
    }

    private synchronized void createVideoView() {
        if(mVideoView != null){
            return;
        }
        Context context = getContext();
        RVideoView video = new RVideoView(context);
        LayoutParams videoLayoutParams =
                new LayoutParams(LayoutParams.MATCH_PARENT,
                        LayoutParams.MATCH_PARENT);
        videoLayoutParams.gravity = Gravity.CENTER;
        video.setLayoutParams(videoLayoutParams);
        addView(video, 0);//first child
        video.setOnErrorListener(mOnErrorListener);
        video.setOnPreparedListener(mOnPreparedListener);
        video.setOnCompletionListener(mOnCompletionListener);
        video.setOnVideoPauseListener(mVideoPlayListener);
        RMediaController controller = new RMediaController(context);
        controller.setAnchorView(this);
        controller.setVideoView(video);
        video.setMediaController(controller);
        controller.setMediaPlayer(video);

        mMediaController = controller;
        mVideoView = video;

        if(mUri != null) {
            setVideoURI(mUri);
        }
    }

    @SuppressLint("NewApi")
    private void removeSelfFromViewTreeObserver() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN) {
            getViewTreeObserver().removeOnGlobalLayoutListener(this);
        } else {
            getViewTreeObserver().removeGlobalOnLayoutListener(this);
        }
    }


    public boolean createVideoViewIfVisible(){
        Rect visibleRect = new Rect();
        if (mVideoView != null) {
            return true;
        } else if (getGlobalVisibleRect(visibleRect) && !visibleRect.isEmpty()) {
            createVideoView();
            return true;
        }
        return false;
    }

    @Override
    public void onGlobalLayout() {
        if(createVideoViewIfVisible()){
            removeSelfFromViewTreeObserver();
        }
    }




    public RenderView getRenderView() {
        return mRenderView;
    }

    public void setRenderView(RenderView mRenderView) {
        this.mRenderView = mRenderView;
    }
}



