package com.taobao.weex.ui.component.list.template;

import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.common.Constants;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.ui.component.WXVContainer;

/**
 * Created by furture on 2017/9/7.
 */

public class WXHRecyclerTemplateList extends  WXRecyclerTemplateList {


    public WXHRecyclerTemplateList(WXSDKInstance instance, WXDomObject node, WXVContainer parent) {
        super(instance, node, parent);
    }

    @Override
    public int getOrientation() {
        return Constants.Orientation.HORIZONTAL;
    }
}
