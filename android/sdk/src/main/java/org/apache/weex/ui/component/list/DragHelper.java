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
package org.apache.weex.ui.component.list;

import android.support.annotation.NonNull;
import android.support.v7.widget.RecyclerView;

import org.apache.weex.ui.component.WXComponent;

/**
 * Description:
 *
 * interface for drag&drop support
 *
 */
interface DragHelper {

    void onDragStart(@NonNull WXComponent component, int from);

    void onDragEnd(@NonNull WXComponent component, int from, int to);

    void onDragging(int fromPos, int toPos);

    boolean isLongPressDragEnabled();

    void setLongPressDragEnabled(boolean enabled);

    void startDrag(@NonNull RecyclerView.ViewHolder viewHolder);

    boolean isDraggable();

    void setDraggable(boolean draggable);

    void setDragExcluded(@NonNull RecyclerView.ViewHolder viewHolder, boolean isExcluded);

    boolean isDragExcluded(@NonNull RecyclerView.ViewHolder viewHolder);
}
