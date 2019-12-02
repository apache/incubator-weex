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
package org.apache.weex.ui.component;

import android.view.ViewGroup;

import java.util.Map;
import org.apache.weex.common.Constants.Orientation;

/**
 * Created by sospartan on 7/5/16.
 */
public interface Scrollable {

  void bindStickStyle(WXComponent component);

  void unbindStickStyle(WXComponent component);

  void bindAppearEvent(WXComponent component);

  void bindDisappearEvent(WXComponent component);

  void unbindAppearEvent(WXComponent component);

  void unbindDisappearEvent(WXComponent component);

  ViewGroup getView();

  void scrollTo(WXComponent component, Map<String, Object> options);

  String getRef();

  int getScrollY();

  int getScrollX();

  /**
   *
   * @return {@link Orientation#HORIZONTAL} or {@link Orientation#VERTICAL}
   */
  int getOrientation();

  boolean isScrollable();
}
