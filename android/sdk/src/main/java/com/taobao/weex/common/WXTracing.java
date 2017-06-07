package com.taobao.weex.common;

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
}
