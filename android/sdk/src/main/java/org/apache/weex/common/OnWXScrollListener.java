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
package org.apache.weex.common;

import android.support.v7.widget.RecyclerView;
import android.view.View;


public interface OnWXScrollListener {

  /**
   * The  view is not currently scrolling.
   */
  int IDLE = RecyclerView.SCROLL_STATE_IDLE;
  /**
   * The view is currently being dragged by outside input such as user touch input.
   */
  int DRAGGING = RecyclerView.SCROLL_STATE_DRAGGING;
  /**
   * The view is currently animating to a final position while not under
   * outside control.
   */
  int SETTLING = RecyclerView.SCROLL_STATE_SETTLING;

  /**
   * Callback method to be invoked when the view has been scrolled. This will be
   * called after the scroll has completed.
   * <p>
   * This callback will also be called if visible item range changes after a layout
   * calculation. In that case, dx and dy will be 0.
   *
   */
  void onScrolled(View view, int x, int y);

  /**
   * Callback method to be invoked when view's scroll state changes.
   *
   */
  void onScrollStateChanged(View view, int x, int y, int newState);
}
