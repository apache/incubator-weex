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
import android.support.v7.widget.GridLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.StaggeredGridLayoutManager;
import android.support.v7.widget.helper.ItemTouchHelper;

import com.taobao.weex.ui.view.listview.adapter.ListBaseViewHolder;
import com.taobao.weex.utils.WXLogUtils;

/**
 * Description:
 * An implementation of {@link ItemTouchHelper.Callback}. with drag&drop support:)
 *
 * <p>
 * Created by rowandjj(chuyi)<br/>
 * Date: 17/4/22<br/>
 * Time: 上午11:25<br/>
 */
class DragSupportCallback extends ItemTouchHelper.Callback {
    private final DragHelper mDragHelper;
    private boolean mEnableDifferentViewTypeDrag = false;

    private int dragFrom = -1;
    private int dragTo = -1;

    private static final String TAG = "WXListExComponent";

    DragSupportCallback(@NonNull DragHelper DragHelper) {
        this.mDragHelper = DragHelper;
        this.mEnableDifferentViewTypeDrag = false;
    }

    DragSupportCallback(@NonNull DragHelper DragHelper, boolean enableDifferentViewTypeDrag) {
        this.mDragHelper = DragHelper;
        this.mEnableDifferentViewTypeDrag = enableDifferentViewTypeDrag;
    }

    @Override
    public int getMovementFlags(RecyclerView recyclerView, RecyclerView.ViewHolder viewHolder) {
        if (recyclerView.getLayoutManager() instanceof GridLayoutManager || recyclerView.getLayoutManager() instanceof StaggeredGridLayoutManager) {
            int dragFlags = ItemTouchHelper.UP | ItemTouchHelper.DOWN | ItemTouchHelper.LEFT | ItemTouchHelper.RIGHT;
            int swipeFlags = 0;
            return makeMovementFlags(dragFlags, swipeFlags);
        } else {
            int dragFlags = ItemTouchHelper.UP | ItemTouchHelper.DOWN;
            int swipeFlags = ItemTouchHelper.START | ItemTouchHelper.END;
            return makeMovementFlags(dragFlags, swipeFlags);
        }
    }

    @Override
    public boolean onMove(RecyclerView recyclerView, RecyclerView.ViewHolder viewHolder, RecyclerView.ViewHolder target) {
        if (viewHolder == null || target == null) {
            return false;
        }

        if (!mEnableDifferentViewTypeDrag && viewHolder.getItemViewType() != target.getItemViewType()) {
            return false;
        }

        if (mDragHelper.isDragExcluded(viewHolder)) {
            return false;
        }

        try {
            int fromPos = viewHolder.getAdapterPosition();
            int toPos = target.getAdapterPosition();

            if (dragFrom == -1) {
                dragFrom = fromPos;
            }
            dragTo = toPos;

            mDragHelper.onDragging(fromPos, toPos);
            return true;
        } catch (Exception e) {
            WXLogUtils.e(TAG, e.getMessage());
            return false;
        }
    }


    @Override
    public boolean isLongPressDragEnabled() {
        return mDragHelper.isDraggable() && mDragHelper.isLongPressDragEnabled();
    }

    @Override
    public void onSwiped(RecyclerView.ViewHolder viewHolder, int direction) {
    }

    @Override
    public boolean isItemViewSwipeEnabled() {
        return false;
    }

    @Override
    public void onSelectedChanged(RecyclerView.ViewHolder viewHolder, int actionState) {
        if (actionState != ItemTouchHelper.ACTION_STATE_IDLE && viewHolder instanceof ListBaseViewHolder) {
            ListBaseViewHolder vh = (ListBaseViewHolder) viewHolder;
            if (vh.getComponent() != null) {
                mDragHelper.onDragStart(vh.getComponent(), vh.getAdapterPosition());
            }
        }

        super.onSelectedChanged(viewHolder, actionState);
    }


    @Override
    public void clearView(RecyclerView recyclerView, RecyclerView.ViewHolder viewHolder) {
        super.clearView(recyclerView, viewHolder);

        if (viewHolder instanceof ListBaseViewHolder) {
            ListBaseViewHolder vh = (ListBaseViewHolder) viewHolder;
            if (vh.getComponent() != null) {
                if (dragFrom != -1 && dragTo != -1) {
                    mDragHelper.onDragEnd(vh.getComponent(), dragFrom, dragTo);
                }
            }

        }

        dragFrom = dragTo = -1;
    }
}
