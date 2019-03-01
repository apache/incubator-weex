package com.taobao.weex.render.platform.texture.video;

import android.content.Context;
import android.net.Uri;
import android.text.TextUtils;
import android.view.MotionEvent;
import android.view.View;

import com.taobao.weex.render.platform.component.Constants;
import com.taobao.weex.render.platform.texture.TextureComponent;
import com.taobao.weex.render.platform.view.video.RMediaController;
import com.taobao.weex.render.platform.view.video.RVideoPlayer;

public class TVideo extends TextureComponent {

    private RMediaController mMediaController;
    private RVideoPlayer mRVideoPlayer;

    @Override
    public View createNativeView(Context context){
        mRVideoPlayer = new RVideoPlayer(context);
        mRVideoPlayer.createIfNotExist();
        String src = getAttrs().get(Constants.Attr.Video.SRC);
        if(TextUtils.isEmpty(src)){
            src = getStyles().get(Constants.Attr.Video.SRC);
        }
        mRVideoPlayer.setVideoURI(Uri.parse(src));
        mMediaController = (RMediaController) mRVideoPlayer.getMediaController();
        mNativeView = mRVideoPlayer;
        if(!TextUtils.isEmpty(src)){
            mRVideoPlayer.post(new Runnable() {
                @Override
                public void run() {
                    mRVideoPlayer.start();
                }
            });
        }
        return mRVideoPlayer;
    }


    @Override
    public void onTextureTouch(MotionEvent motionEvent, float x, float y){
        if(mMediaController != null){
            if(mMediaController.getArchorView() != getView()){
                mMediaController.setAnchorView(getView());
            }
        }
        super.onTextureTouch(motionEvent, x, y);
    }

    @Override
    public void onDestroy() {
        if(mRVideoPlayer != null){
            mRVideoPlayer.pause();
        }
        super.onDestroy();
    }
}
