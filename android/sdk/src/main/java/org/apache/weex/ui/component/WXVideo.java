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

package org.apache.weex.ui.component;

import android.content.Context;
import android.media.MediaPlayer;
import android.net.Uri;
import android.support.annotation.NonNull;
import android.text.TextUtils;
import android.view.View;
import android.widget.FrameLayout;

import org.apache.weex.WXEnvironment;
import org.apache.weex.WXSDKInstance;
import org.apache.weex.WXSDKManager;
import org.apache.weex.annotation.Component;
import org.apache.weex.adapter.URIAdapter;
import org.apache.weex.common.Constants;
import org.apache.weex.ui.action.BasicComponentData;
import org.apache.weex.ui.view.WXVideoView;
import org.apache.weex.utils.WXLogUtils;
import org.apache.weex.utils.WXUtils;

import java.util.HashMap;
import java.util.Map;
import org.apache.weex.ui.view.WXVideoView.Wrapper;

@Component(lazyload = false)

public class WXVideo extends WXComponent<FrameLayout> {

  private boolean mAutoPlay;
  private Wrapper mWrapper;

  /**
   * package
   **/
  boolean mPrepared;
  private boolean mError;

  @Deprecated
  public WXVideo(WXSDKInstance instance, WXVContainer parent, String instanceId, boolean isLazy, BasicComponentData basicComponentData) {
    this(instance, parent, isLazy, basicComponentData);
  }

  public WXVideo(WXSDKInstance instance, WXVContainer parent, boolean isLazy, BasicComponentData basicComponentData) {
    super(instance, parent, isLazy, basicComponentData);
  }

  @Override
  protected FrameLayout initComponentHostView(@NonNull Context context) {
    final Wrapper video = new Wrapper(context);
    video.setOnErrorListener(new MediaPlayer.OnErrorListener() {

      @Override
      public boolean onError(MediaPlayer mp, int what, int extra) {
        if (WXEnvironment.isApkDebugable()) {
          WXLogUtils.d("Video", "onError:" + what);
        }
        video.getProgressBar().setVisibility(View.GONE);
        mPrepared = false;
        mError = true;

        if (getEvents().contains(Constants.Event.FAIL)) {
          WXVideo.this.notify(Constants.Event.FAIL, Constants.Value.STOP);
        }
        return true;
      }
    });

    video.setOnPreparedListener(new MediaPlayer.OnPreparedListener() {

      @Override
      public void onPrepared(MediaPlayer mediaPlayer) {
        if (WXEnvironment.isApkDebugable()) {
          WXLogUtils.d("Video", "onPrepared");
        }
        video.getProgressBar().setVisibility(View.GONE);
        mPrepared = true;
        if (mAutoPlay) {
          video.start();
        }

        //callback from video view, so videoview should not null
        WXVideoView videoView = video.getVideoView();
        videoView.seekTo(5);

        if (video.getMediaController() != null) {
          if (!mStopped) {
            video.getMediaController().show(3);
          } else {
            video.getMediaController().hide();
          }
        }

        mStopped = false;
      }
    });

    video.setOnCompletionListener(new MediaPlayer.OnCompletionListener() {

      @Override
      public void onCompletion(MediaPlayer mediaPlayer) {
        if (WXEnvironment.isApkDebugable()) {
          WXLogUtils.d("Video", "onCompletion");
        }
        if (getEvents().contains(Constants.Event.FINISH)) {
          WXVideo.this.notify(Constants.Event.FINISH, Constants.Value.STOP);
        }
      }
    });

    video.setOnVideoPauseListener(new WXVideoView.VideoPlayListener() {

      @Override
      public void onPause() {
        if (WXEnvironment.isApkDebugable()) {
          WXLogUtils.d("Video", "onPause");
        }
        if (getEvents().contains(Constants.Event.PAUSE)) {
          WXVideo.this.notify(Constants.Event.PAUSE, Constants.Value.PAUSE);
        }
      }

      @Override
      public void onStart() {
        if (WXEnvironment.isApkDebugable()) {
          WXLogUtils.d("Video", "onStart");
        }

        if (getEvents().contains(Constants.Event.START)) {
          WXVideo.this.notify(Constants.Event.START, Constants.Value.PLAY);
        }
      }
    });
    mWrapper = video;
    return video;
  }

  private void notify(String event, String newStatus) {
    Map<String, Object> params = new HashMap<>(2);
    params.put(Constants.Name.PLAY_STATUS, newStatus);
    params.put("timeStamp", System.currentTimeMillis());

    Map<String, Object> domChanges = new HashMap<>();
    Map<String, Object> attrsChanges = new HashMap<>();
    attrsChanges.put(Constants.Name.PLAY_STATUS, newStatus);
    domChanges.put("attrs", attrsChanges);

    WXSDKManager.getInstance().fireEvent(getInstanceId(), getRef(), event, params, domChanges);
  }

  @Override
  public void bindData(WXComponent component) {
    super.bindData(component);
    addEvent(Constants.Event.APPEAR);
  }

  @Override
  public void notifyAppearStateChange(String wxEventType, String direction) {
    super.notifyAppearStateChange(wxEventType, direction);
    mWrapper.createVideoViewIfVisible();
  }

  @Override
  public void destroy() {
    super.destroy();
  }

  @Override
  protected boolean setProperty(String key, Object param) {
    switch (key) {
      case Constants.Name.SRC:
        String src = WXUtils.getString(param, null);
        if (src != null) {
          setSrc(src);
        }
        return true;
      case Constants.Name.AUTO_PLAY:
      case Constants.Name.AUTOPLAY:
        Boolean result = WXUtils.getBoolean(param, null);
        if (result != null) {
          setAutoPlay(result);
        }
        return true;
      case Constants.Name.ZORDERTOP:
        Boolean zOrderTop = WXUtils.getBoolean(param, null);
        if (zOrderTop != null) {
            mWrapper.getVideoView().setZOrderOnTop(zOrderTop);
        }
        return true;
      case Constants.Name.PLAY_STATUS:
        String status = WXUtils.getString(param, null);
        if (status != null) {
          setPlaystatus(status);
        }
        return true;
    }
    return super.setProperty(key, param);
  }

  @WXComponentProp(name = Constants.Name.SRC)
  public void setSrc(String src) {
    if (TextUtils.isEmpty(src) || getHostView() == null) {
      return;
    }

    if (!TextUtils.isEmpty(src)) {
      WXSDKInstance instance = getInstance();
      mWrapper.setVideoURI(instance.rewriteUri(Uri.parse(src), URIAdapter.VIDEO));
      mWrapper.getProgressBar().setVisibility(View.VISIBLE);
    }
  }

  @WXComponentProp(name = Constants.Name.AUTO_PLAY)
  public void setAutoPlay(boolean autoPlay) {
    mAutoPlay = autoPlay;
    if(autoPlay){
      mWrapper.createIfNotExist();
      mWrapper.start();
    }
  }

  @WXComponentProp(name = Constants.Name.CONTROLS)
  public void setControls(String controls) {
    if (TextUtils.equals("controls", controls)) {
      mWrapper.setControls(true);
    } else if (TextUtils.equals("nocontrols", controls)) {
      mWrapper.setControls(false);
    }
  }

  private boolean mStopped;

  @WXComponentProp(name = Constants.Name.PLAY_STATUS)
  public void setPlaystatus(String playstatus) {

    if (mPrepared && !mError && !mStopped) {
      if (playstatus.equals(Constants.Value.PLAY)) {
        mWrapper.start();
      } else if (playstatus.equals(Constants.Value.PAUSE)) {
        mWrapper.pause();
      } else if (playstatus.equals(Constants.Value.STOP)) {
        mWrapper.stopPlayback();
        mStopped = true;
      }
    } else if ((mError || mStopped) && playstatus.equals(Constants.Value.PLAY)) {
      mError = false;
      mWrapper.resume();

      mWrapper.getProgressBar().setVisibility(View.VISIBLE);
    }
  }
}
