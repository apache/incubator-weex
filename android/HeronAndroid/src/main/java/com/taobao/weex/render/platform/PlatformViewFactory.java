package com.taobao.weex.render.platform;

import com.taobao.weex.render.frame.RenderView;
import com.taobao.weex.render.platform.component.Constants;
import com.taobao.weex.render.platform.component.Input;
import com.taobao.weex.render.platform.component.Video;
import com.taobao.weex.render.platform.component.Web;
import com.taobao.weex.render.platform.texture.TextureComponent;
import com.taobao.weex.render.platform.texture.video.TVideo;
import com.taobao.weex.utils.WXLogUtils;

import java.lang.reflect.Constructor;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class PlatformViewFactory {

    private static final Map<String, Class<?>> mViewClasses = new HashMap<>();
    static {
        registerViewClass(Constants.Tag.INPUT, Input.class);
        registerViewClass(Constants.Tag.WEB, Web.class);
        registerViewClass(Constants.Tag.VIDEO, Video.class);
        registerViewClass(Constants.Tag.TVIDEO, TVideo.class);
    }


    public static <T extends  PlatformView>  void  registerViewClass(String type, Class<T> platformView){
        mViewClasses.put(type, platformView);
    }




    public static PlatformView createView(RenderView renderView, String ref, String type,
                                          Map<String,String> attrs, Map<String,String> styles, List<String> events){
       Class<?> viewClass =  mViewClasses.get(type);
       try{
           Constructor<?> constructor =  viewClass.getDeclaredConstructor();
           constructor.setAccessible(true);
           PlatformView platformView = (PlatformView) constructor.newInstance();
           platformView.setRef(ref);
           platformView.setType(type);
           platformView.createView(renderView, attrs, styles, events);
           return platformView;
       }catch (Exception e){
           WXLogUtils.e("PlatformView createView", e);
           return null;
       }
    }

}
