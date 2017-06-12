package com.taobao.weex.common;

import android.os.Message;

import com.taobao.weex.dom.WXDomHandler;
import com.taobao.weex.dom.WXDomTask;
import com.taobao.weex.dom.action.Action;
import com.taobao.weex.utils.WXLogUtils;

import java.util.concurrent.atomic.AtomicInteger;

/**
 * Created by moxun on 2017/6/6.
 */

public class WXTracing {
    private static final AtomicInteger sIdGenerator = new AtomicInteger(0);

    public static int nextId() {
        return sIdGenerator.getAndIncrement();
    }

    public static void submit(TraceEvent event) {
        WXLogUtils.d("WXTracing", event.toString());
    }

    public static class TraceEvent {
        public String name;
        public String phase;
        public String threadName;
        public long timestamp;
        public long nanoTime;
        public long duration;

        @Override
        public String toString() {
            final StringBuffer sb = new StringBuffer("TraceEvent{");
            sb.append("name='").append(name).append('\'');
            sb.append(", threadName='").append(threadName).append('\'');
            sb.append(", phase='").append(phase).append('\'');
            sb.append(", timestamp=").append(timestamp);
            sb.append(", nanoTime=").append(nanoTime);
            sb.append(", duration=").append(duration);
            sb.append('}');
            return sb.toString();
        }
    }

    public static String getFunctionName(Message msg) {
        Object obj = msg.obj;
        if (obj != null && obj instanceof WXDomTask) {
            Object action = ((WXDomTask) obj).args.get(0);
            if (action instanceof Action) {
                return getFunctionName(action.getClass());
            }
        }

        String actionName = "unknown";
        int what = msg.what;
        if (what == WXDomHandler.MsgType.WX_DOM_BATCH) {
            actionName = "domBatch";
        } else if (what == WXDomHandler.MsgType.WX_DOM_UPDATE_STYLE) {
            actionName = "updateStyle";
        } else if (what == WXDomHandler.MsgType.WX_CONSUME_RENDER_TASKS) {
            actionName = "consumeRenderTasks";
        }
        return actionName;
    }

    public static String getFunctionName(Class clazz) {
        String simpleName = clazz.getSimpleName();
        char[] chars = simpleName.replace("Action", "").toCharArray();
        chars[0] = Character.toLowerCase(chars[0]);
        return new String(chars);
    }
}
