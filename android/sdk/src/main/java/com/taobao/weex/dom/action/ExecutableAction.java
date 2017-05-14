package com.taobao.weex.dom.action;

import com.taobao.weex.dom.DOMAction;
import com.taobao.weex.dom.DOMActionContext;
import com.taobao.weex.dom.RenderAction;
import com.taobao.weex.dom.RenderActionContext;

class ExecutableAction implements DOMAction, RenderAction {

    private Runnable mRunnable;

    ExecutableAction(Runnable runnable) {
        this.mRunnable = runnable;
    }

    @Override
    public void executeDom(DOMActionContext context) {
        if (context.isDestory()) {
            return;
        }
        context.postRenderTask(this);
    }

    @Override
    public void executeRender(RenderActionContext context) {
        if (mRunnable != null) {
            mRunnable.run();
        }
    }
}
