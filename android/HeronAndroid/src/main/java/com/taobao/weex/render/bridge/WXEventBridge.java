package com.taobao.weex.render.bridge;

import com.taobao.weex.WXSDKEngine;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.base.CalledByNative;

import java.util.HashMap;
import java.util.Map;

public class WXEventBridge {


    @CalledByNative
    public static void fireEvent(final String pageId, final String ref, final String type, final Map<String,Object> params){
        final WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(pageId);
       if(instance == null){
           return;
       }
        WXSDKManager.getInstance().getWXRenderManager().postOnUiThread(new Runnable() {
            @Override
            public void run() {
                instance.fireEvent(ref, type, params);
            }
        });
    }
}
