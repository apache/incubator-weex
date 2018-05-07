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

public class GraphicSize {

  private float mWidth;
  private float mHeight;

  public GraphicSize(float width, float height) {
    this.mWidth = width;
    this.mHeight = height;
  }

  public float getWidth() {
    return mWidth;
  }

  public void setWidth(float width) {
    this.mWidth = width;
  }

  public float getHeight() {
    return mHeight;
  }

  public void setHeight(float height) {
    this.mHeight = height;
  }

  public void update(float width, float height) {
    this.mWidth = width;
    this.mHeight = height;
  }
}
