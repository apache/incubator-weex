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
package com.taobao.weex.ui.component;

import android.content.Context;
import android.content.Intent;
import android.support.annotation.RestrictTo;
import android.support.annotation.RestrictTo.Scope;
import android.util.Pair;
import android.support.annotation.Nullable;
import android.util.Pair;
import android.view.Menu;
import android.view.View;
import android.view.ViewGroup;

import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.common.Constants;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.utils.WXLogUtils;
import com.taobao.weex.utils.WXViewUtils;

import java.util.ArrayList;

/**
 * All container components must implement this class
 */
public abstract class WXVContainer<T extends ViewGroup> extends WXComponent<T> {

  private static final String TAG="WXVContainer";
  protected ArrayList<WXComponent> mChildren = new ArrayList<>();
  private BoxShadowHost mBoxShadowHost;

  @Deprecated
  public WXVContainer(WXSDKInstance instance, WXDomObject dom, WXVContainer parent, String instanceId, boolean isLazy) {
    this(instance,dom,parent,isLazy);
  }

  @Deprecated
  public WXVContainer(WXSDKInstance instance, WXDomObject node, WXVContainer parent, boolean lazy) {
    super(instance, node, parent);
  }

  public WXVContainer(WXSDKInstance instance, WXDomObject node, WXVContainer parent) {
    super(instance, node, parent);
  }

  /**
   * Container will get focus before any of its descendants.
   */
  public void interceptFocus() {
    T host = getHostView();
    if (host != null) {
      host.setFocusable(true);
      host.setFocusableInTouchMode(true);
      host.setDescendantFocusability(ViewGroup.FOCUS_BEFORE_DESCENDANTS);
      host.requestFocus();
    }
  }

  /**
   * Container will can not receive focus
   */
  public void ignoreFocus() {
    T host = getHostView();
    if (host != null) {
      host.setFocusable(false);
      host.setFocusableInTouchMode(false);
      host.clearFocus();
    }
  }

  /**
   * Offset top for children layout.
   */
  protected int getChildrenLayoutTopOffset(){
    return 0;
  }

  /**
   * use {@link #getHostView()} instead
   */
  @Deprecated
  public ViewGroup getView(){
    return getHostView();
  }

  @Override
  public void applyLayoutAndEvent(WXComponent component) {
    long startNanos = System.nanoTime();
    if(!isLazy()) {
      if (component == null) {
        component = this;
      }
      super.applyLayoutAndEvent(component);
      int count = childCount();
      for (int i = 0; i < count; i++) {
        WXComponent child = getChild(i);
        child.applyLayoutAndEvent(((WXVContainer)component).getChild(i));
      }

    }
    mTraceInfo.uiThreadNanos += (System.nanoTime() - startNanos);
  }

  /**
   * Get or generate new layout parameter for child view
   *
   */
  public ViewGroup.LayoutParams getChildLayoutParams(WXComponent child,View childView, int width, int height, int left, int right, int top, int bottom){
    ViewGroup.LayoutParams lp = null;
    if (childView != null) {
      lp = childView.getLayoutParams();
    }

    if(lp == null) {
      lp = new ViewGroup.LayoutParams(width,height);
    }else{
      lp.width = width;
      lp.height = height;
      if(lp instanceof ViewGroup.MarginLayoutParams){
        ((ViewGroup.MarginLayoutParams) lp).setMargins(left,top,right,bottom);
      }
    }
    return lp;
  }


  public Scrollable getFirstScroller(){
    if(this instanceof Scrollable){
      return (Scrollable)this;
    }else{
      for (int i=0;i<getChildCount();i++){
        Scrollable scrollable = getChild(i).getFirstScroller();
        if(scrollable!=null){
          return scrollable;
        }
      }
    }
    return null;
  }
  @Override
  public void bindData(WXComponent component) {
    long startNanos = System.nanoTime();
    if(!isLazy()) {
      if (component == null) {
        component = this;
      }
      super.bindData(component);
      int count = childCount();
      for (int i = 0; i < count; i++) {
        getChild(i).bindData(((WXVContainer)component).getChild(i));
      }
    }
    mTraceInfo.uiThreadNanos += (System.nanoTime() - startNanos);
  }

  @Override
  public void refreshData(WXComponent component) {
      if (component == null) {
        component = this;
      }
      super.refreshData(component);
      int count = childCount();
      for (int i = 0; i < count; i++) {
        getChild(i).refreshData(((WXVContainer)component).getChild(i));
      }
  }

  /**
   * return real View
   */
  @Override
  public ViewGroup getRealView() {
    return (ViewGroup) super.getRealView();
  }

  @Override
  public void createViewImpl() {
    super.createViewImpl();
    int count = childCount();
    for (int i = 0; i < count; ++i) {
      createChildViewAt(i);
    }
    if(getHostView()!=null){
       getHostView().setClipToPadding(false);
    }
  }

  @Override
  public void destroy() {
    if (mChildren != null) {
      int count = mChildren.size();
      for (int i = 0; i < count; ++i) {
        mChildren.get(i).destroy();
      }
      mChildren.clear();
    }
    super.destroy();
  }

  /**
   * recycle component resources
   */
  public void recycled() {
    if (mChildren != null && !getDomObject().isFixed() && getDomObject().getAttrs().canRecycled()) {
      int count = mChildren.size();
      for (int i = 0; i < count; ++i) {
        mChildren.get(i).recycled();
      }
    }
    super.recycled();

  }

  @Override
  public View detachViewAndClearPreInfo(){
    View original = super.detachViewAndClearPreInfo();
    if (mChildren != null) {
      int count = childCount();
      for (int i = 0; i < count; ++i) {
        mChildren.get(i).detachViewAndClearPreInfo();
      }
    }
    return original;
  }

  /**
   * Use {@link #getChildCount()} instead
   * @return
   */
  @Deprecated
  public int childCount() {
    return mChildren == null ? 0 : mChildren.size();
  }

  @Nullable
  public WXComponent getChild(int index) {
    if (mChildren == null || index < 0 || index >= mChildren.size()) {
      //To avoid index out of bounds
      return null;
    }
    return mChildren.get(index);
  }

  public int getChildCount() {
    return childCount();
  }

  public void addChild(WXComponent child) {
    addChild(child, -1);
  }

  public void addChild(WXComponent child, int index) {
    long startNanos = System.nanoTime();
    if (child == null || index < -1) {
      return;
    }
    int count = mChildren.size();
    index = index >= count ? -1 : index;
    if (index == -1) {
      mChildren.add(child);
    } else {
      mChildren.add(index, child);
    }
    mTraceInfo.uiThreadNanos += (System.nanoTime() - startNanos);
  }

  public final int indexOf(WXComponent comp){
    return mChildren.indexOf(comp);
  }

  public void createChildViewAt(int index) {
    long startNanos = System.nanoTime();
    Pair<WXComponent, Integer> ret = rearrangeIndexAndGetChild(index);
    if (ret.first != null) {
      WXComponent child = ret.first;
      child.createView();
      if (!child.isVirtualComponent()) {
        addSubView(child.getHostView(), ret.second);
      }
    }
    mTraceInfo.uiThreadNanos += (System.nanoTime() - startNanos);
  }

  protected Pair<WXComponent, Integer> rearrangeIndexAndGetChild(int index){
    int indexToCreate = index;
    if(indexToCreate < 0){
      indexToCreate = childCount()-1;
    }

    if (indexToCreate<0){
      return new Pair<>(null, indexToCreate);
    }
    else {
      return new Pair<>(getChild(indexToCreate), indexToCreate);
    }
  }

  @RestrictTo(Scope.LIBRARY)
  public void addSubView(View child, int index) {
    if (child == null || getRealView() == null) {
      return;
    }

    int count = getRealView().getChildCount();
    index = index >= count ? -1 : index;
    if (index == -1) {
      getRealView().addView(child);
    } else {
      getRealView().addView(child, index);
    }
  }

  public void remove(WXComponent child, boolean destroy){
    if (child == null || mChildren == null || mChildren.size() == 0) {
      return;
    }

    mChildren.remove(child);
    if(getInstance()!=null
            &&getInstance().getRootView()!=null
            && child.getDomObject().isFixed()){
      getInstance().removeFixedView(child.getHostView());
    }else if(getRealView() != null) {
      if(!child.isVirtualComponent()){
        getRealView().removeView(child.getHostView());
      }else{
        child.removeVirtualComponent();
      }
    }
    if(destroy) {
      child.destroy();
    }
  }

  @Override
  public void notifyAppearStateChange(String wxEventType, String direction) {
    super.notifyAppearStateChange(wxEventType, direction);
    if(getHostView()==null || mChildren==null){
      return;
    }
    //appear should not notify child
    if(getDomObject().getAttrs().containsKey("appearNotifyChild")){
      for(WXComponent component:mChildren){
        if(component.getHostView()!=null && !(component.getHostView().getVisibility()==View.VISIBLE)){
          wxEventType= Constants.Event.DISAPPEAR;
        }
        component.notifyAppearStateChange(wxEventType,direction);
      }
    }
  }

  /********************************
   *  begin hook Activity life cycle callback
   ********************************************************/
  @Override
  public void onActivityCreate() {
    super.onActivityCreate();

    int count = childCount();
    for (int i = 0; i < count; i++) {
      getChild(i).onActivityCreate();
    }
  }

  @Override
  public void onActivityStart() {
    super.onActivityStart();

    int count = childCount();
    for (int i = 0; i < count; i++) {
      getChild(i).onActivityStart();
    }

  }

  @Override
  public void onActivityPause() {
    super.onActivityPause();

    int count = childCount();
    for (int i = 0; i < count; i++) {
      getChild(i).onActivityPause();
    }
  }

  @Override
  public void onActivityResume() {
    super.onActivityResume();

    int count = childCount();
    for (int i = 0; i < count; i++) {
      getChild(i).onActivityResume();
    }
  }

  @Override
  public void onActivityStop() {
    super.onActivityStop();

    int count = childCount();
    for (int i = 0; i < count; i++) {
      getChild(i).onActivityStop();
    }
  }

  @Override
  public void onActivityDestroy() {
    super.onActivityDestroy();

    int count = childCount();
    for (int i = 0; i < count; i++) {
      getChild(i).onActivityDestroy();
    }

  }

  @Override
  public boolean onActivityBack() {
    super.onActivityBack();

    int count = childCount();
    for (int i = 0; i < count; i++) {
      getChild(i).onActivityBack();
    }
    return false;
  }

  @Override
  public void onActivityResult(int requestCode, int resultCode, Intent data){
    super.onActivityResult(requestCode,resultCode,data);

    int count = childCount();
    for (int i = 0; i < count; i++) {
      getChild(i).onActivityResult(requestCode,resultCode,data);
    }

  }

  public boolean onCreateOptionsMenu(Menu menu){
    super.onCreateOptionsMenu(menu);

    int count = childCount();
    for (int i = 0; i < count; i++) {
      getChild(i).onCreateOptionsMenu(menu);
    }
    return false;
  }


  @Override
  public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults){
    super.onRequestPermissionsResult(requestCode,permissions,grantResults);

    int count = childCount();
    for (int i = 0; i < count; i++) {
      getChild(i).onRequestPermissionsResult(requestCode,permissions,grantResults);
    }
  }

  @Override
  public void onRenderFinish(@RenderState int state) {
    for (int i = 0; i < getChildCount(); i++) {
      WXComponent child = getChild(i);
      child.mTraceInfo.uiQueueTime = mTraceInfo.uiQueueTime;
      child.onRenderFinish(state);
    }
    super.onRenderFinish(state);
  }

  /********************************
   *  end hook Activity life cycle callback
   ********************************************************/

  public @Nullable View getBoxShadowHost(boolean isClear) {
    if (isClear) {
      // Return existed host if want clear shadow
      return mBoxShadowHost;
    }

    ViewGroup hostView = getHostView();
    if (hostView == null) {
      return null;
    }

    try {
      String type = getDomObject().getType();
      if (WXBasicComponentType.DIV.equals(type)) {
        WXLogUtils.d("BoxShadow", "Draw box-shadow with BoxShadowHost on div: " + toString());
        if (mBoxShadowHost == null) {
          mBoxShadowHost = new BoxShadowHost(getContext());
          WXViewUtils.setBackGround(mBoxShadowHost, null);
          mBoxShadowHost.setLayoutParams(new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT));
          hostView.addView(mBoxShadowHost);
        }
        hostView.removeView(mBoxShadowHost);
        hostView.addView(mBoxShadowHost);
        return mBoxShadowHost;
      }
    } catch (Throwable t) {
      WXLogUtils.w("BoxShadow", t);
    }
    return hostView;
  }

  private class BoxShadowHost extends View {
    public BoxShadowHost(Context context) {
      super(context);
    }
  }
}
