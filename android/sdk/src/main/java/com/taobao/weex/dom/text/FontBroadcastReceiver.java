package com.taobao.weex.dom.text;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.dom.DOMActionContext;
import com.taobao.weex.dom.WXDomHandler;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.dom.WXTextDomObject;
import com.taobao.weex.utils.WXLogUtils;

import java.lang.ref.WeakReference;

/**
 * Created by furture on 2018/2/24.
 */

public class FontBroadcastReceiver extends BroadcastReceiver {

    private WeakReference<WXTextDomObject> wxTextDomObjectRef;

    private String mFontFamily;

    public FontBroadcastReceiver(WXTextDomObject wxTextDomObject, String mFontFamily) {
        this.wxTextDomObjectRef = new WeakReference<WXTextDomObject>(wxTextDomObject);
        this.mFontFamily = mFontFamily;
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        String fontFamily = intent.getStringExtra("fontFamily");
        if (!mFontFamily.equals(fontFamily)) {
            return;
        }
        WXTextDomObject wxTextDomObject = wxTextDomObjectRef.get();
        if(wxTextDomObject == null){
            return;
        }
        if(wxTextDomObject.isDestroy() || wxTextDomObject.getDomContext() == null){
            return;
        }

        DOMActionContext domActionContext = WXSDKManager.getInstance().getWXDomManager().getDomContext(wxTextDomObject.getDomContext().getInstanceId());
        if(domActionContext == null){
            return;
        }
        WXDomObject domObject = domActionContext.getDomByRef(wxTextDomObject.getRef());
        if(domObject == null){
            return;
        }
        domObject.markDirty();
        domActionContext.markDirty();
        WXSDKManager.getInstance().getWXDomManager().sendEmptyMessageDelayed(WXDomHandler.MsgType.WX_DOM_START_BATCH, 2);
        if(WXEnvironment.isApkDebugable()) {
            WXLogUtils.d("WXText", "Font family " + fontFamily + " is available");
        }
    }
}
