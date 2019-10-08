/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
package org.apache.weex.ui.component.helper;

import android.app.Activity;
import android.graphics.Rect;
import android.os.Build;
import android.support.annotation.Nullable;
import android.view.View;
import android.view.ViewTreeObserver;
import android.view.WindowManager;

import org.apache.weex.utils.WXLogUtils;
import org.apache.weex.utils.WXViewUtils;

import java.lang.ref.WeakReference;

/**
 * Created by moxun on 17/3/24.
 */

public class SoftKeyboardDetector {

    private static final int KEYBOARD_VISIBLE_THRESHOLD_DIP = 100;

    public static Unregister registerKeyboardEventListener(Activity activity, final OnKeyboardEventListener listener) {
        if (activity == null || listener == null) {
            WXLogUtils.e("Activity or listener is null!");
            return null;
        }

        if (activity.getWindow() != null) {
            WindowManager.LayoutParams attributes = activity.getWindow().getAttributes();
            if (attributes != null) {
                int softInputMode = attributes.softInputMode;
                if (softInputMode == WindowManager.LayoutParams.SOFT_INPUT_ADJUST_NOTHING
                        || softInputMode == WindowManager.LayoutParams.SOFT_INPUT_ADJUST_PAN) {
                    WXLogUtils.e("SoftKeyboard detector can't work with softInputMode is SOFT_INPUT_ADJUST_NOTHING or SOFT_INPUT_ADJUST_PAN");
                    return null;
                }
            }
        }

        final View activityRoot = getActivityRoot(activity);

        if (activityRoot == null) {
            WXLogUtils.e("Activity root is null!");
            return null;
        }

        final ViewTreeObserver.OnGlobalLayoutListener layoutListener = new ViewTreeObserver.OnGlobalLayoutListener() {

            private final Rect visibleFrame = new Rect();
            private final int threshold = WXViewUtils.dip2px(KEYBOARD_VISIBLE_THRESHOLD_DIP);
            private boolean wasKeyboardOpened = false;

            @Override
            public void onGlobalLayout() {
                activityRoot.getWindowVisibleDisplayFrame(visibleFrame);
                int heightDiff = activityRoot.getRootView().getHeight() - visibleFrame.height();
                boolean isOpen = heightDiff > threshold;
                if (isOpen == wasKeyboardOpened) {
                    return;
                }

                wasKeyboardOpened = isOpen;
                listener.onKeyboardEvent(isOpen);
            }
        };

        activityRoot.getViewTreeObserver().addOnGlobalLayoutListener(layoutListener);
        return new DefaultUnRegister(activity, layoutListener);
    }

    public static boolean isKeyboardVisible(Activity activity) {
        Rect windowFrame = new Rect();
        View root = getActivityRoot(activity);

        if (root != null) {
            root.getWindowVisibleDisplayFrame(windowFrame);
            int heightDiff = root.getRootView().getHeight() - windowFrame.height();
            return heightDiff > WXViewUtils.dip2px(KEYBOARD_VISIBLE_THRESHOLD_DIP);
        }
        return false;
    }

    public static @Nullable View getActivityRoot(Activity activity) {
        if (activity != null) {
            return activity.findViewById(android.R.id.content);
        }
        return null;
    }

    public static final class DefaultUnRegister implements Unregister {

        private WeakReference<Activity> activityRef;
        private WeakReference<ViewTreeObserver.OnGlobalLayoutListener> listenerRef;

        public DefaultUnRegister(Activity activity, ViewTreeObserver.OnGlobalLayoutListener listener) {
            this.activityRef = new WeakReference<>(activity);
            this.listenerRef = new WeakReference<>(listener);
        }

        @Override
        public void execute() {
            Activity activity = activityRef.get();
            ViewTreeObserver.OnGlobalLayoutListener listener = listenerRef.get();

            if (activity != null && listener != null) {
                View root = SoftKeyboardDetector.getActivityRoot(activity);
                if (root != null) {
                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN) {
                        root.getViewTreeObserver().removeOnGlobalLayoutListener(listener);
                    } else {
                        root.getViewTreeObserver().removeGlobalOnLayoutListener(listener);
                    }
                }
            }

            activityRef.clear();
            listenerRef.clear();
        }
    }

    public interface Unregister {
        void execute();
    }

    public interface OnKeyboardEventListener {
        void onKeyboardEvent(boolean isShown);
    }
}
