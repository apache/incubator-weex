package com.taobao.weex.performance;


import android.os.Build;
import android.support.annotation.RequiresApi;
import android.view.Choreographer;

import com.taobao.weex.common.WXPerformance;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

/**
 * @author chenpeihan
 * @date 2017/12/12
 */

public class FpsCollector {
    private Map<String, IFrameCallBack> mListenerMap = new ConcurrentHashMap<>();


    private static class SingleTonHolder {
        private static FpsCollector INSTANCE = new FpsCollector();
    }

    public static FpsCollector getInstance() {
        return SingleTonHolder.INSTANCE;
    }

    public void init() {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.JELLY_BEAN) {
            return;
        }
        if (WXPerformance.TRACE_DATA) {
            Choreographer.getInstance().postFrameCallback(new OnFrameListener());
        }
    }

    @RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN)
    private class OnFrameListener implements Choreographer.FrameCallback {

        @Override
        public void doFrame(long frameTimeNanos) {
            for (Map.Entry<String, IFrameCallBack> entry : mListenerMap.entrySet()) {
                entry.getValue().doFrame(frameTimeNanos);
            }
        }
    }

    public interface IFrameCallBack {
        void doFrame(long frameTimeNanos);
    }


    public void registerListener(String key, IFrameCallBack listener) {
        mListenerMap.put(key, listener);
    }

    public void unRegister(String key) {
        mListenerMap.remove(key);
    }
}
