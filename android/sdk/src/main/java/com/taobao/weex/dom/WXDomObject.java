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
package com.taobao.weex.dom;

import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.text.TextUtils;

import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.bridge.WXValidateProcessor;
import com.taobao.weex.common.Constants;
import com.taobao.weex.common.Constants.Name;
import com.taobao.weex.dom.flex.CSSLayoutContext;
import com.taobao.weex.dom.flex.CSSNode;
import com.taobao.weex.dom.flex.Spacing;
import com.taobao.weex.ui.component.WXBasicComponentType;
import com.taobao.weex.utils.WXLogUtils;
import com.taobao.weex.utils.WXViewUtils;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.atomic.AtomicBoolean;

/**
 * WXDomObject contains all the info about the given node, including style, attribute and event.
 * Unlike {@link com.taobao.weex.ui.component.WXComponent}, WXDomObject only contains info about
 * the dom, has nothing to do with rendering.
 * Actually, {@link com.taobao.weex.ui.component.WXComponent} hold references to
 * {@link android.view.View} and {@link WXDomObject}.
 */
public class WXDomObject extends CSSNode implements Cloneable,ImmutableDomObject {
  public static final String CHILDREN = "children";
  public static final String TYPE = "type";
  public static final String TAG = WXDomObject.class.getSimpleName();
  public static final String ROOT = "_root";

  /**
   * Use {@link Name#TRANSFORM} instead.
   */
  @Deprecated
  public static final String TRANSFORM = Name.TRANSFORM;

  /**
   * Use {@link Name#TRANSFORM_ORIGIN} instead.
   */
  @Deprecated
  public static final String TRANSFORM_ORIGIN = Name.TRANSFORM_ORIGIN;
  static final WXDomObject DESTROYED = new WXDomObject();
  static{
    DESTROYED.mRef = "_destroyed";
  }
  private AtomicBoolean sDestroy = new AtomicBoolean();

  private int mViewPortWidth =750;

  private DomContext mDomContext;

  /** package **/ String mRef = ROOT;

  /** package **/ String mType = WXBasicComponentType.DIV;

  /** package **/ WXStyle mStyles;

  /** package **/ WXAttr mAttributes;

  /** package **/ WXEvent mEvents;

  private List<WXDomObject> mDomChildren;

  /** Do not access this field directly. This field will be removed soon. **/
  @Deprecated
  public WXDomObject parent;

  private ArrayList<String> fixedStyleRefs;

  private boolean mYoung = false;

  public long mDomThreadNanos;
  public long mDomThreadTimestamp;

  public void traverseTree(Consumer...consumers){
    long startNanos = System.nanoTime();
    if (consumers == null) {
      return;
    }

    for (Consumer consumer:consumers){
      consumer.accept(this);
    }

    int count = childCount();
    WXDomObject child;
    for (int i = 0; i < count; ++i) {
      child = getChild(i);
      child.traverseTree(consumers);
    }
    mDomThreadNanos += (System.nanoTime() - startNanos);
  }


  public int getViewPortWidth() {
    return mViewPortWidth;
  }

  public void setViewPortWidth(int mViewPortWidth) {
    this.mViewPortWidth = mViewPortWidth;
  }

  public String getRef(){
    return mRef;
  }

  public String getType(){
    return mType;
  }

  public @NonNull WXStyle getStyles(){
    if(mStyles == null ){
      mStyles = new WXStyle();
    }
    return mStyles;
  }

  public @NonNull WXAttr getAttrs(){
    if(mAttributes == null){
      mAttributes = new WXAttr();
    }
    return mAttributes;
  }

  public @NonNull WXEvent getEvents(){
    if(mEvents == null){
      mEvents = new WXEvent();
    }

    return mEvents;
  }

  public @NonNull DomContext getDomContext() {
    return mDomContext;
  }

  public void clearEvents(){
    if(mEvents != null){
      mEvents.clear();
    }
  }

  public static void prepareRoot(WXDomObject domObj,float defaultHeight,float defaultWidth) {
    domObj.mRef = WXDomObject.ROOT;

    WXStyle domStyles = domObj.getStyles();
    Map<String, Object> style = new HashMap<>(5);
    if (!domStyles.containsKey(Constants.Name.FLEX_DIRECTION)) {
      style.put(Constants.Name.FLEX_DIRECTION, "column");
    }
    if (!domStyles.containsKey(Constants.Name.BACKGROUND_COLOR)) {
      style.put(Constants.Name.BACKGROUND_COLOR, "#ffffff");
    }

    style.put(Constants.Name.DEFAULT_WIDTH, defaultWidth);
    style.put(Constants.Name.DEFAULT_HEIGHT, defaultHeight);

    domObj.updateStyle(style);
  }

  protected final void copyFields(WXDomObject dest) {
    dest.cssstyle.copy(this.cssstyle);
    dest.mRef = mRef;
    dest.mType = mType;
    dest.mStyles = mStyles == null ? null : mStyles.clone();//mStyles == null ? null : mStyles.clone();
    dest.mAttributes = mAttributes == null ? null : mAttributes.clone();//mAttrs == null ? null : mAttrs.clone();
    dest.mEvents = mEvents == null ? null : mEvents.clone();
    dest.csslayout.copy(this.csslayout);
  }

  /**
   * Parse the jsonObject to {@link WXDomObject} recursively
   * @param map the original JSONObject
   */
  public void parseFromJson(JSONObject map){
    if (map == null || map.size() <= 0) {
      return;
    }

    String type = (String) map.get("type");
    this.mType = type;
    this.mRef = (String) map.get("ref");
    Object style = map.get("style");
    if (style != null && style instanceof JSONObject) {
      WXStyle styles = new WXStyle();
      styles.putAll((JSONObject) style,false);
      this.mStyles = styles;
    }
    Object attr = map.get("attr");
    if (attr != null && attr instanceof JSONObject) {
      WXAttr attrs = new WXAttr((JSONObject) attr);
      //WXJsonUtils.putAll(attrs, (JSONObject) attr);
      this.mAttributes = attrs;
    }
    Object event = map.get("event");
    if (event != null && event instanceof JSONArray) {
      WXEvent events = new WXEvent();
      JSONArray eventArray = (JSONArray) event;
      int count = eventArray.size();
      for (int i = 0; i < count; ++i) {
        events.add(eventArray.getString(i));
      }
      this.mEvents = events;
    }

  }


  /**
   * Do pre-staff before layout. Subclass may provide different implementation.
   */
  public void layoutBefore() {

  }

  /**
   * Do post-staff before layout. Subclass may provide different implementation.
   */
  public void layoutAfter(){

  }

  /**
   * Tell whether this object need to be updated. This is usually called when
   * {@link CSSNode#calculateLayout(CSSLayoutContext)} finishes and new layout has been
   * calculated. This method is a simple wrapper method for {@link #hasNewLayout()} and
   * {@link #isDirty()}.
   * @return true for need update since last update.
   */
  public final boolean hasUpdate() {
    return hasNewLayout() || isDirty();
  }

  /**
   * Mark the current node is young and unconsumed.
   */
  void young() {
    mYoung = true;
  }

  /**
   * Mark the current node is old and consumed.
   */
  void old() {
    mYoung = false;
  }

  /**
   * Tell whether this node is consumed since last layout.
   * @return true for consumed, false for not.
   */
  boolean isYoung() {
    return mYoung;
  }

  /**
   * Mark the update has been seen. After this method call, following call for {@link
   * #hasUpdate()} will return false. This method is also a wrapper for {@link #markUpdateSeen()}
   */
  public final void markUpdateSeen() {
    if (hasNewLayout()) {
      markLayoutSeen();
    }
  }

  public boolean isFixed() {
    return mStyles == null ? false : mStyles.isFixed();
  }

  public boolean canRecycled() {
    return mAttributes == null ? false : mAttributes.canRecycled();
  }

  public Object getExtra() {
    return null;
  }

  public void remove(WXDomObject child) {
    if (child == null || mDomChildren == null || sDestroy.get()) {
      return;
    }

    int index = mDomChildren.indexOf(child);
    removeFromDom(child);
    if (index != -1) {
      super.removeChildAt(index);
    }
  }

  public void removeFromDom(WXDomObject child) {
    if (child == null || mDomChildren == null || sDestroy.get()) {
      return;
    }

    int index = mDomChildren.indexOf(child);
    if (index == -1) {
      if (WXEnvironment.isApkDebugable()) {
        WXLogUtils.e("[WXDomObject] remove function error");
      }
      return;
    }
    mDomChildren.remove(index).parent = null;
  }

  public int index(WXDomObject child) {
    if (child == null || mDomChildren == null || sDestroy.get()) {
      return -1;
    }
    return mDomChildren.indexOf(child);
  }

  /**
   * Add the given WXDomObject as this object's child at specified index.
   * @param child the child to be added
   * @param index the index of child to be added. If the index is -1, the child will be added
   *              as the last child of current dom object.
   */
  public void add(WXDomObject child, int index) {
    if (child == null || index < -1 || sDestroy.get()) {
      return;
    }
    if (mDomChildren == null) {
      mDomChildren = new ArrayList<>();
    }

    int count = mDomChildren.size();
    index = index >= count ? -1 : index;
    if (index == -1) {
      mDomChildren.add(child);
      super.addChildAt(child, super.getChildCount());
    } else {
      mDomChildren.add(index, child);
      super.addChildAt(child, index);
    }
    child.parent = this;
  }

  @Deprecated
  public void add2Dom(WXDomObject child, int index) {
    if (child == null || index < -1 || sDestroy.get()) {
      return;
    }

    int count = super.getChildCount();
    index = index >= count ? -1 : index;
    if (index == -1) {
      super.addChildAt(child, super.getChildCount());
    } else {
      super.addChildAt(child, index);
    }
    child.parent = this;
  }

  public WXDomObject getChild(int index) {
    if (mDomChildren == null || sDestroy.get()) {
      return null;
    }
    return mDomChildren.get(index);
  }

  /**
   * Add the given event for current object.
   * @param e
   */
  public void addEvent(String e) {
    if (TextUtils.isEmpty(e)) {
      return;
    }
    if (mEvents == null) {
      mEvents = new WXEvent();
    }
    if (containsEvent(e)) {
      return;
    }
    mEvents.add(e);
  }

  public boolean containsEvent(String e) {
    if (mEvents == null) {
      return false;
    }
    return mEvents.contains(e);
  }

  public void removeEvent(String e) {
    if (TextUtils.isEmpty(e)) {
      return;
    }
    if (mEvents == null) {
      return;
    }
    mEvents.remove(e);
  }

  public void updateAttr(Map<String, Object> attrs) {
    if (attrs == null || attrs.isEmpty()) {
      return;
    }
    if (mAttributes == null) {
      mAttributes = new WXAttr();
    }
    mAttributes.putAll(attrs);
    super.dirty();
  }

  public void updateStyle(Map<String, Object> styles){
    updateStyle(styles,false);
  }

  public void updateStyle(Map<String, Object> styles, boolean byPesudo) {
    if (styles == null || styles.isEmpty()) {
      return;
    }
    if (mStyles == null) {
      mStyles = new WXStyle();
    }
    mStyles.putAll(styles,byPesudo);
    super.dirty();
  }

  /** package **/ void applyStyleToNode() {
    WXStyle stylesMap = getStyles();
    int vp = getViewPortWidth();
    if (!stylesMap.isEmpty()) {
      for(Map.Entry<String,Object> item:stylesMap.entrySet()) {
        switch (item.getKey()) {
          case Constants.Name.ALIGN_ITEMS:
            setAlignItems(stylesMap.getAlignItems());
            break;
          case Constants.Name.ALIGN_SELF:
            setAlignSelf(stylesMap.getAlignSelf());
            break;
          case Constants.Name.FLEX:
            setFlex(stylesMap.getFlex());
            break;
          case Constants.Name.FLEX_DIRECTION:
            setFlexDirection(stylesMap.getFlexDirection());
            break;
          case Constants.Name.JUSTIFY_CONTENT:
            setJustifyContent(stylesMap.getJustifyContent());
            break;
          case Constants.Name.FLEX_WRAP:
            setWrap(stylesMap.getCSSWrap());
            break;
          case Constants.Name.MIN_WIDTH:
            setMinWidth(WXViewUtils.getRealPxByWidth(stylesMap.getMinWidth(vp),vp));
            break;
          case Constants.Name.MIN_HEIGHT:
            setMinHeight(WXViewUtils.getRealPxByWidth(stylesMap.getMinHeight(vp),vp));
            break;
          case Constants.Name.MAX_WIDTH:
            setMaxWidth(WXViewUtils.getRealPxByWidth(stylesMap.getMaxWidth(vp),vp));
            break;
          case Constants.Name.MAX_HEIGHT:
            setMaxHeight(WXViewUtils.getRealPxByWidth(stylesMap.getMaxHeight(vp),vp));
            break;
          case Constants.Name.DEFAULT_HEIGHT:
          case Constants.Name.HEIGHT:
            setStyleHeight(WXViewUtils.getRealPxByWidth(stylesMap.containsKey(Constants.Name.HEIGHT)?stylesMap.getHeight(vp):stylesMap.getDefaultHeight(),vp));
            break;
          case Constants.Name.WIDTH:
          case Constants.Name.DEFAULT_WIDTH:
            setStyleWidth(WXViewUtils.getRealPxByWidth(stylesMap.containsKey(Constants.Name.WIDTH)?stylesMap.getWidth(vp):stylesMap.getDefaultWidth(),vp));
            break;
          case Constants.Name.POSITION:
            setPositionType(stylesMap.getPosition());
            break;
          case Constants.Name.LEFT:
            setPositionLeft(WXViewUtils.getRealPxByWidth(stylesMap.getLeft(vp),vp));
            break;
          case Constants.Name.TOP:
            setPositionTop(WXViewUtils.getRealPxByWidth(stylesMap.getTop(vp),vp));
            break;
          case Constants.Name.RIGHT:
            setPositionRight(WXViewUtils.getRealPxByWidth(stylesMap.getRight(vp),vp));
            break;
          case Constants.Name.BOTTOM:
            setPositionBottom(WXViewUtils.getRealPxByWidth(stylesMap.getBottom(vp),vp));
            break;
          case Constants.Name.MARGIN:
            setMargin(Spacing.ALL, WXViewUtils.getRealPxByWidth(stylesMap.getMargin(vp), vp));
            break;
          case Constants.Name.MARGIN_LEFT:
            setMargin(Spacing.LEFT, WXViewUtils.getRealPxByWidth(stylesMap.getMarginLeft(vp), vp));
            break;
          case Constants.Name.MARGIN_TOP:
            setMargin(Spacing.TOP, WXViewUtils.getRealPxByWidth(stylesMap.getMarginTop(vp), vp));
            break;
          case Constants.Name.MARGIN_RIGHT:
            setMargin(Spacing.RIGHT, WXViewUtils.getRealPxByWidth(stylesMap.getMarginRight(vp), vp));
            break;
          case Constants.Name.MARGIN_BOTTOM:
            setMargin(Spacing.BOTTOM, WXViewUtils.getRealPxByWidth(stylesMap.getMarginBottom(vp), vp));
            break;
          case Constants.Name.BORDER_WIDTH:
            setBorder(Spacing.ALL, WXViewUtils.getRealPxByWidth(stylesMap.getBorderWidth(vp), vp));
            break;
          case Constants.Name.BORDER_TOP_WIDTH:
            setBorder(Spacing.TOP, WXViewUtils.getRealPxByWidth(stylesMap.getBorderTopWidth(vp), vp));
            break;
          case Constants.Name.BORDER_RIGHT_WIDTH:
            setBorder(Spacing.RIGHT, WXViewUtils.getRealPxByWidth(stylesMap.getBorderRightWidth(vp), vp));
            break;
          case Constants.Name.BORDER_BOTTOM_WIDTH:
            setBorder(Spacing.BOTTOM, WXViewUtils.getRealPxByWidth(stylesMap.getBorderBottomWidth(vp), vp));
            break;
          case Constants.Name.BORDER_LEFT_WIDTH:
            setBorder(Spacing.LEFT, WXViewUtils.getRealPxByWidth(stylesMap.getBorderLeftWidth(vp), vp));
            break;
          case Constants.Name.PADDING:
            setPadding(Spacing.ALL, WXViewUtils.getRealPxByWidth(stylesMap.getPadding(vp), vp));
            break;
          case Constants.Name.PADDING_LEFT:
            setPadding(Spacing.LEFT, WXViewUtils.getRealPxByWidth(stylesMap.getPaddingLeft(vp), vp));
            break;
          case Constants.Name.PADDING_TOP:
            setPadding(Spacing.TOP, WXViewUtils.getRealPxByWidth(stylesMap.getPaddingTop(vp), vp));
            break;
          case Constants.Name.PADDING_RIGHT:
            setPadding(Spacing.RIGHT, WXViewUtils.getRealPxByWidth(stylesMap.getPaddingRight(vp), vp));
            break;
          case Constants.Name.PADDING_BOTTOM:
            setPadding(Spacing.BOTTOM, WXViewUtils.getRealPxByWidth(stylesMap.getPaddingBottom(vp), vp));
            break;
        }
      }
    }
  }

  public int childCount() {
    return mDomChildren == null ? 0 : mDomChildren.size();
  }

  public void hide() {
    setVisible(false);
  }

  public void show() {
    setVisible(true);
  }

  public boolean isVisible() {
    return super.isShow();
  }

  /**
   * Clone the current object. This is not a deep copy, only shadow copy of some reference.
   * @return The result object of clone.
   */
  @Override
  public WXDomObject clone() {
    if (sDestroy.get()) {
      return null;
    }
    WXDomObject dom = null;
    try {
      dom = WXDomObjectFactory.newInstance(mType);
      copyFields(dom);
    } catch (Exception e) {
      if (WXEnvironment.isApkDebugable()) {
        WXLogUtils.e("WXDomObject clone error: ", e);
      }
    }

    return dom;
  }

  public void destroy() {
    sDestroy.set(true);
    if (mStyles != null) {
      mStyles.clear();
    }
    if (mAttributes != null) {
      mAttributes.clear();
    }
    if (mEvents != null) {
      mEvents.clear();
    }
    if (mDomChildren != null) {
      int count = mDomChildren.size();
      for (int i = 0; i < count; ++i) {
        mDomChildren.get(i).destroy();
      }
      mDomChildren.clear();
    }
    mDomContext = null;
  }

  /** package **/
  /**
   * Get default style map for component.
   * @return
   */
  protected Map<String,String> getDefaultStyle(){
    return null;
  }

  public ArrayList<String> getFixedStyleRefs() {
    return fixedStyleRefs;
  }

  public void add2FixedDomList(String ref) {
    if (fixedStyleRefs == null) {
      fixedStyleRefs = new ArrayList<>();
    }
    fixedStyleRefs.add(ref);
  }

  public String dumpDomTree() {
    return mRef + ": " + toString();
  }

  /**
   * Parse the jsonObject to {@link WXDomObject} recursively
   * @param json the original JSONObject
   * @return Dom Object corresponding to the JSONObject.
   */
  public static  @Nullable WXDomObject parse(JSONObject json, WXSDKInstance wxsdkInstance) {
      return parse(json, wxsdkInstance, null);
  }

  public static  @Nullable WXDomObject parse(JSONObject json, WXSDKInstance wxsdkInstance, WXDomObject parentDomObject){
      long startNanos = System.nanoTime();
      long timestamp = System.currentTimeMillis();
      if (json == null || json.size() <= 0) {
        return null;
      }

      String type = (String) json.get(TYPE);

      if (wxsdkInstance.isNeedValidate()) {
        WXValidateProcessor processor = WXSDKManager.getInstance()
                .getValidateProcessor();
        if (processor != null) {
          WXValidateProcessor.WXComponentValidateResult result = processor
                  .onComponentValidate(wxsdkInstance, type, parentDomObject);
          if (result != null && !result.isSuccess) {
            type = TextUtils.isEmpty(result.replacedComponent) ? WXBasicComponentType.DIV
                    : result.replacedComponent;
            json.put(TYPE, type);
            if (WXEnvironment.isApkDebugable() && result.validateInfo != null) {
              String tag = "[WXDomObject]onComponentValidate failure. >>> " + result.validateInfo.toJSONString();
              WXLogUtils.e(tag);
            }
          } else if (result == null){
            return null;
          }
        }
      }

      WXDomObject domObject = WXDomObjectFactory.newInstance(type);

      domObject.setViewPortWidth(wxsdkInstance.getInstanceViewPortWidth());

      if(domObject == null){
        return null;
      }
      domObject.parseFromJson(json);
      domObject.mDomContext = wxsdkInstance;
      domObject.parent = parentDomObject;

      Object children = json.get(CHILDREN);
      if (children != null && children instanceof JSONArray) {
        JSONArray childrenArray = (JSONArray) children;
        int count = childrenArray.size();
        for (int i = 0; i < count; ++i) {
          domObject.add(parse(childrenArray.getJSONObject(i),wxsdkInstance, domObject),-1);
        }
      }

      domObject.mDomThreadNanos = System.nanoTime() - startNanos;
      domObject.mDomThreadTimestamp = timestamp;
      return domObject;
  }

  public interface Consumer{
    void accept(WXDomObject dom);
  }
}
