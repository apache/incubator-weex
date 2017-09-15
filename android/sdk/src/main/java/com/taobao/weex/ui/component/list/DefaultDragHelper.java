/*
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
package com.taobao.weex.ui.component.list;

import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.helper.ItemTouchHelper;

import com.taobao.weex.WXEnvironment;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.utils.WXLogUtils;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Description:
 * Drag-drop support for list
 * <p>
 * Created by rowandjj(chuyi)<br/>
 */

class DefaultDragHelper implements DragHelper {

    private boolean mLongPressEnabled;

    @NonNull
    private final EventTrigger mEventTrigger;
    @NonNull
    private final RecyclerView mRecyclerView;
    @NonNull
    private final List<WXComponent> mDataSource;
    @NonNull
    private ItemTouchHelper mItemTouchHelper;

    private static final String EVENT_START_DRAG = "dragstart";
    private static final String EVENT_END_DRAG = "dragend";

    private static final String TAG_EXCLUDED = "drag_excluded";

    private static final String TAG = "WXListExComponent";

    private boolean isDraggable = false;

    DefaultDragHelper(@NonNull List<WXComponent> dataSource, @NonNull RecyclerView recyclerView, @NonNull EventTrigger trigger) {
        this.mDataSource = dataSource;
        this.mEventTrigger = trigger;
        this.mRecyclerView = recyclerView;

        //attach
        this.mItemTouchHelper = new ItemTouchHelper(new DragSupportCallback(this, true));
        try {
            mItemTouchHelper.attachToRecyclerView(mRecyclerView);
        }catch (Throwable e) {
            //ignore
        }
    }

    @Override
    public void onDragStart(@NonNull WXComponent component, int from) {
        if (WXEnvironment.isApkDebugable()) {
            WXLogUtils.d(TAG, "list on drag start : from index " + from);
        }
        mEventTrigger.triggerEvent(EVENT_START_DRAG, buildEvent(component.getRef(), from, -1));
    }

    @Override
    public void onDragEnd(@NonNull WXComponent component, int from, int to) {
        if (WXEnvironment.isApkDebugable()) {
            WXLogUtils.d(TAG, "list on drag end : " + "from index " + from + " to index " + to);
        }
        mEventTrigger.triggerEvent(EVENT_END_DRAG, buildEvent(component.getRef(), from, to));
    }

    @Override
    public void onDragging(int fromPos, int toPos) {
        if (WXEnvironment.isApkDebugable()) {
            WXLogUtils.d(TAG, "list on dragging : from index " + fromPos + " to index " + toPos);
        }

        RecyclerView.Adapter adapter = mRecyclerView.getAdapter();
        if (adapter == null) {
            WXLogUtils.e(TAG, "drag failed because of RecyclerView#Adapter is not bound");
            return;
        }

        if (fromPos >= 0 && fromPos <= mDataSource.size() - 1 && toPos >= 0 && toPos <= mDataSource.size() - 1) {
            Collections.swap(mDataSource, fromPos, toPos);
            adapter.notifyItemMoved(fromPos, toPos);
        }
    }

    @Override
    public boolean isLongPressDragEnabled() {
        return mLongPressEnabled;
    }

    @Override
    public void setLongPressDragEnabled(boolean enabled) {
        this.mLongPressEnabled = enabled;
    }

    @Override
    public void startDrag(@NonNull RecyclerView.ViewHolder viewHolder) {
        if (isDraggable) {
            mItemTouchHelper.startDrag(viewHolder);
        }
    }

    @Override
    public boolean isDraggable() {
        return this.isDraggable;
    }

    @Override
    public void setDraggable(boolean draggable) {
        this.isDraggable = draggable;
    }

    @Override
    public void setDragExcluded(@NonNull RecyclerView.ViewHolder viewHolder, boolean isExcluded) {
        if (viewHolder.itemView == null) {
            if (WXEnvironment.isApkDebugable()) {
                WXLogUtils.e(TAG, "[error] viewHolder.itemView is null");
            }
            return;
        }
        if (isExcluded) {
            viewHolder.itemView.setTag(TAG_EXCLUDED);
        } else {
            viewHolder.itemView.setTag(null);
        }
    }

    @Override
    public boolean isDragExcluded(@NonNull RecyclerView.ViewHolder viewHolder) {
        if (viewHolder.itemView == null) {
            if (WXEnvironment.isApkDebugable()) {
                WXLogUtils.e(TAG, "[error] viewHolder.itemView is null");
            }
            return false;
        }
        return viewHolder.itemView.getTag() != null && TAG_EXCLUDED.equals(viewHolder.itemView.getTag());
    }

    private Map<String, Object> buildEvent(@Nullable String target, int fromIndex, int toIndex) {
        Map<String, Object> args = new HashMap<>(4);
        args.put("target", target == null ? "unknown" : target);
        args.put("fromIndex", fromIndex);
        args.put("toIndex", toIndex);
        args.put("timestamp", System.currentTimeMillis());
        return args;
    }
}
