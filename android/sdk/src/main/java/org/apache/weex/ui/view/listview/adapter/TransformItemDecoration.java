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
package org.apache.weex.ui.view.listview.adapter;

import android.graphics.Canvas;
import android.support.v7.widget.RecyclerView;
import android.view.View;

public class TransformItemDecoration extends RecyclerView.ItemDecoration{
  boolean mIsVertical = true;
  float mAlpha = -1f;
  int mXTranslate = 0;
  int mYTranslate = 0;
  int mRotation = 0;
  float mScaleX = 0;
  float mScaleY  = 0;

  public TransformItemDecoration(boolean isVertical,float alpha,int translateX,int translateY,int rotation,float scaleX,float scaleY){
    this.mIsVertical = isVertical;
    this.mAlpha = alpha;
    this.mXTranslate = translateX;
    this.mYTranslate = translateY;
    this.mRotation = rotation;
    this.mScaleX = scaleX;
    this.mScaleY = scaleY;
  }


  @Override
  public void onDrawOver(Canvas c, RecyclerView parent, RecyclerView.State state) {
    super.onDrawOver(c, parent, state);
    int width = parent.getWidth();
    int height = parent.getHeight();
    for (int i = 0,count=parent.getChildCount(); i < count; i++) {
      updateItem(parent.getChildAt(i),width,height);
    }
  }

  private void updateItem(View child, int width, int height) {
    int size,childCenter,containerSize;
    if (mIsVertical) {
      containerSize = height;
      size = child.getHeight();
      childCenter = child.getTop() + size / 2;
    } else {
      containerSize = width;
      size = child.getWidth();
      childCenter = child.getLeft() + size / 2;
    }

    final int actionDistance = (containerSize + size) / 2;
    final float effectsAmount = Math.min(1.0f, Math.max(-1.0f, (1.0f / actionDistance) * (childCenter - containerSize/2)));


    if(mAlpha>0){
      child.setAlpha(1-mAlpha*Math.abs(effectsAmount));
    }

    if(mScaleX>0||mScaleY>0){
      child.setScaleX(1-mScaleX*Math.abs(effectsAmount));
      child.setScaleY(1-mScaleY*Math.abs(effectsAmount));
    }

    if(mRotation!=0){
      child.setRotation(mRotation * effectsAmount);
    }

    if(mXTranslate!=0){
      child.setTranslationX(mXTranslate * Math.abs( effectsAmount));
    }

    if(mYTranslate!=0){
      child.setTranslationY(mYTranslate * Math.abs( effectsAmount));
    }

  }
}
