package com.taobao.weex.ui;

import com.taobao.weex.ui.component.WXComponent;

/**
 * Created by zhengshihan on 16/8/25.
 */
public interface IExternalComponentGetter {
    Class<? extends WXComponent> getExternalComponentClass(String type);
}
