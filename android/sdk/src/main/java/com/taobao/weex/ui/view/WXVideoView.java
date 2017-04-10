/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
package com.taobao.weex.ui.view;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Rect;
import android.media.MediaPlayer;
import android.net.Uri;
import android.os.Build;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.util.AttributeSet;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.ViewTreeObserver;
import android.widget.FrameLayout;
import android.widget.MediaController;
import android.widget.ProgressBar;
import android.widget.VideoView;

import com.taobao.weex.ui.view.gesture.WXGesture;
import com.taobao.weex.ui.view.gesture.WXGestureObservable;
import com.taobao.weex.utils.WXResourceUtils;


public class WXVideoView extends VideoView implements WXGestureObservable {

  private WXGesture wxGesture;
  private VideoPlayListener mVideoPauseListener;

  public WXVideoView(Context context) {
    super(context);
  }

  @Override
  public void registerGestureListener(WXGesture wxGesture) {
    this.wxGesture = wxGesture;
  }

  public void setOnVideoPauseListener(VideoPlayListener listener) {
    mVideoPauseListener = listener;
  }

  @Override
  public boolean onTouchEvent(MotionEvent event) {
    boolean result = super.onTouchEvent(event);
    if (wxGesture != null) {
      result |= wxGesture.onTouch(this, event);
    }
    return result;
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


  public static class Wrapper extends FrameLayout implements ViewTreeObserver.OnGlobalLayoutListener {

    private WXVideoView mVideoView;
    private ProgressBar mProgressBar;
    private MediaController mMediaController;
    private Uri mUri;
    private MediaPlayer.OnPreparedListener mOnPreparedListener;
    private MediaPlayer.OnErrorListener mOnErrorListener;
    private MediaPlayer.OnCompletionListener mOnCompletionListener;
    private WXVideoView.VideoPlayListener mVideoPlayListener;


    public Wrapper(Context context) {
      super(context);
      init(context);
    }

    public Wrapper(Context context, AttributeSet attrs) {
      super(context, attrs);
      init(context);
    }

    public Wrapper(Context context, AttributeSet attrs, int defStyleAttr) {
      super(context, attrs, defStyleAttr);
      init(context);
    }

    private void init(Context context) {
      setBackgroundColor(WXResourceUtils.getColor("#ee000000"));
      mProgressBar = new ProgressBar(context);
      FrameLayout.LayoutParams pLayoutParams =
          new FrameLayout.LayoutParams(FrameLayout.LayoutParams.WRAP_CONTENT,
              FrameLayout.LayoutParams.WRAP_CONTENT);
      mProgressBar.setLayoutParams(pLayoutParams);
      pLayoutParams.gravity = Gravity.CENTER;
      addView(mProgressBar);

      getViewTreeObserver().addOnGlobalLayoutListener(this);
    }

    public ProgressBar getProgressBar() {
      return mProgressBar;
    }

    public
    @Nullable
    WXVideoView getVideoView() {
      return mVideoView;
    }

    /**
     * Create if not existed. Will cause request focus.
     *
     * @return
     */
    public
    @NonNull
    WXVideoView createIfNotExist() {
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

    public void setOnVideoPauseListener(VideoPlayListener listener) {
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
      WXVideoView video = new WXVideoView(context);
      FrameLayout.LayoutParams videoLayoutParams =
          new FrameLayout.LayoutParams(FrameLayout.LayoutParams.MATCH_PARENT,
              FrameLayout.LayoutParams.MATCH_PARENT);
      videoLayoutParams.gravity = Gravity.CENTER;
      video.setLayoutParams(videoLayoutParams);
      addView(video, 0);//first child
      video.setOnErrorListener(mOnErrorListener);
      video.setOnPreparedListener(mOnPreparedListener);
      video.setOnCompletionListener(mOnCompletionListener);
      video.setOnVideoPauseListener(mVideoPlayListener);
      MediaController controller = new MediaController(context);
      controller.setAnchorView(this);
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
  }
}
