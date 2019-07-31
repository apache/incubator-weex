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
package com.taobao.weex;

import android.view.View;

import com.taobao.weex.ui.component.WXComponent;

/**
 * Created by sospartan on 14/06/2017.
 */

public interface ComponentObserver {

  /**
   * Called after component is create.
   * Notice: View is not created at this moment.
   * @param component
   */
  void onCreate(WXComponent component);

  /**
   * Called before component destroy.
   * @param component
   */
  void onPreDestory(WXComponent component);

  /**
   * Called when component's view is created
   * @param component
   * @param view
   */
  void onViewCreated(WXComponent component,View view);
}
