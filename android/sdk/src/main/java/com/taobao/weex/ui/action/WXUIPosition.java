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
package com.taobao.weex.ui.action;

import com.taobao.weex.utils.WXViewUtils;

public class WXUIPosition {

  private float mLeft;
  private float mTop;
  private float mRight;
  private float mBottom;

  public WXUIPosition(float left, float top, float right, float bottom) {
    this.mLeft = left;
    this.mTop =  top;
    this.mRight =  right;
    this.mBottom = bottom;
  }

  public float getLeft() {
    return mLeft;
  }

  public void setLeft(float mLeft) {
    this.mLeft = mLeft;
  }

  public float getTop() {
    return mTop;
  }

  public void setTop(float mTop) {
    this.mTop = mTop;
  }

  public float getRight() {
    return mRight;
  }

  public void setRight(float mRight) {
    this.mRight = mRight;
  }

  public float getBottom() {
    return mBottom;
  }

  public void setBottom(float mBottom) {
    this.mBottom = mBottom;
  }
}
