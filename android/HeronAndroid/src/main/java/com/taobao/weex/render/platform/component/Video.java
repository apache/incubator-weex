package com.taobao.weex.render.platform.component;

import android.media.MediaPlayer;
import android.net.Uri;
import android.text.TextUtils;
import android.view.MotionEvent;
import android.view.View;

import com.taobao.weex.render.frame.RenderView;
import com.taobao.weex.render.platform.view.video.RVideoPlayer;
import com.taobao.weex.render.platform.view.video.RVideoView;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class Video extends Component {

    private RVideoPlayer mVideoPlayer;
    private boolean mAutoPlay;
    private boolean mPrepared;
    private boolean mError;
    private boolean mStopped;

    @Override
    public void createView(RenderView renderView, Map<String, String> attrs, Map<String, String> styles, List<String> events) {
        attrs.putAll(styles);
        super.init(renderView, attrs, styles, events);
        mVideoPlayer = new RVideoPlayer(renderView.getContext());
        mVideoPlayer.createIfNotExist();
        mVideoPlayer.getProgressBar().setVisibility(View.GONE);
        mVideoPlayer.setOnErrorListener(new MediaPlayer.OnErrorListener() {

            @Override
            public boolean onError(MediaPlayer mp, int what, int extra) {
                mVideoPlayer.getProgressBar().setVisibility(View.GONE);
                mPrepared = false;
                mError = true;
                if (getEvents().contains(Constants.Event.Video.FAIL)) {
                    notifyEvent(Constants.Event.Video.FAIL, Constants.Value.Video.STOP);
                }
                return true;
            }
        });

        mVideoPlayer.setOnPreparedListener(new MediaPlayer.OnPreparedListener() {

            @Override
            public void onPrepared(MediaPlayer mediaPlayer) {
                mVideoPlayer.getProgressBar().setVisibility(View.GONE);
                mPrepared = true;
                if (mAutoPlay) {
                    mediaPlayer.start();
                }

                //callback from video view, so videoview should not null
                RVideoView videoView = mVideoPlayer.getVideoView();
                videoView.seekTo(5);

                if (mVideoPlayer.getMediaController() != null) {
                    if (!mStopped) {
                        mVideoPlayer.getMediaController().show(3);
                    } else {
                        mVideoPlayer.getMediaController().hide();
                    }
                }

                mStopped = false;
            }
        });

        mVideoPlayer.setOnCompletionListener(new MediaPlayer.OnCompletionListener() {

            @Override
            public void onCompletion(MediaPlayer mediaPlayer) {
                if (getEvents().contains(Constants.Event.Video.FINISH)) {
                    notifyEvent(Constants.Event.Video.FINISH, Constants.Value.Video.STOP);
                }
            }
        });

        mVideoPlayer.setOnVideoPauseListener(new RVideoView.VideoPlayListener() {

            @Override
            public void onPause() {
                if (getEvents().contains(Constants.Event.Video.PAUSE)) {
                    notifyEvent(Constants.Event.Video.PAUSE, Constants.Value.Video.PAUSE);
                }
            }

            @Override
            public void onStart() {
                if (getEvents().contains(Constants.Event.Video.START)) {
                    notifyEvent(Constants.Event.Video.START, Constants.Value.Video.PLAY);
                }
            }
        });

        mVideoPlayer.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(mVideoPlayer.getVideoView() != null){
                    if(mVideoPlayer.getVideoView().isPlaying()){
                        mVideoPlayer.getVideoView().pause();
                    }else{
                        mVideoPlayer.start();
                    }
                }
            }
        });
        mVideoPlayer.setRenderView(renderView);
        mVideoPlayer.getMediaController().setAnchorView(mVideoPlayer.getVideoView());
    }



    @Override
    public View getView() {
        return mVideoPlayer;
    }

    @Override
    public void onAttach() {
        String src = getAttrs().get(Constants.Attr.Video.SRC);
        if(TextUtils.isEmpty(src)){
            return;
        }
        mVideoPlayer.setVideoURI(Uri.parse(src));
        mVideoPlayer.start();
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
        if(mVideoPlayer != null){
            mVideoPlayer.resume();
            mVideoPlayer.start();
        }
    }

    @Override
    public void onPause() {
        if(mVideoPlayer != null){
            mVideoPlayer.pause();
        }
    }

    @Override
    public void onTouch(MotionEvent motionEvent, float viewScreenX, float viewScreenY){
        mVideoPlayer.dispatchRenderTouchEvent(motionEvent);
    }

    @Override
    public void onPositionChange(float x, float y, float width, float height){
        if(mVideoPlayer != null && mVideoPlayer.getMediaController() != null){
            if(mVideoPlayer.getMediaController().isShowing()){
                mVideoPlayer.getMediaController().hide();
            }
        }
    }

    @Override
    public void onDestroy() {

    }


    private void notifyEvent(String event, String newStatus) {
        Map<String, Object> params = new HashMap<>(2);
        params.put(Constants.Name.Video.PLAY_STATUS, newStatus);
        params.put("timeStamp", System.currentTimeMillis());

        Map<String, Object> domChanges = new HashMap<>();
        Map<String, Object> attrsChanges = new HashMap<>();
        attrsChanges.put(Constants.Name.Video.PLAY_STATUS, newStatus);
        domChanges.put("attrs", attrsChanges);

       //FIXME FireEvent WXSDKManager.getInstance().fireEvent(getInstanceId(), getRef(), event, params, domChanges);
    }
}
