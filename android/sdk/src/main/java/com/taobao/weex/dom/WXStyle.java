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

import android.graphics.Typeface;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v4.util.ArrayMap;
import android.text.Layout;
import android.text.TextUtils;

import com.taobao.weex.common.Constants;
import com.taobao.weex.dom.flex.CSSAlign;
import com.taobao.weex.dom.flex.CSSFlexDirection;
import com.taobao.weex.dom.flex.CSSJustify;
import com.taobao.weex.dom.flex.CSSPositionType;
import com.taobao.weex.dom.flex.CSSWrap;
import com.taobao.weex.ui.component.WXText;
import com.taobao.weex.ui.component.WXTextDecoration;
import com.taobao.weex.utils.WXUtils;
import com.taobao.weex.utils.WXViewUtils;

import java.util.Collection;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;

/**
 * Store value of component style
 *
 */
public class WXStyle implements Map<String, Object>,Cloneable {

  private static final long serialVersionUID = 611132641365274134L;
  public static final int UNSET = -1;

  private @NonNull final Map<String,Object> mStyles;
  private Map<String,Map<String,Object>> mPesudoStyleMap = new ArrayMap<>();// clz_group:{styleMap}
  private Map<String,Object> mPesudoResetStyleMap = new ArrayMap<>();


  public WXStyle(){
    mStyles = new ArrayMap<>();
  }

  @Nullable
  public String getBlur() {
    if(get(Constants.Name.FILTER) == null) {
      return null;
    }
    return get(Constants.Name.FILTER).toString().trim();
  }

  /*
   * text-decoration
   **/
  public static WXTextDecoration getTextDecoration(Map<String, Object> style) {
    Object obj = style.get(Constants.Name.TEXT_DECORATION);
    if (obj == null) {
      return WXTextDecoration.NONE;
    }
    String textDecoration = obj.toString();
    if (textDecoration.equals("underline")) {
      return WXTextDecoration.UNDERLINE;
    }
    if (textDecoration.equals("line-through")) {
      return WXTextDecoration.LINETHROUGH;
    }
    return WXTextDecoration.NONE;
  }

  public static String getTextColor(Map<String, Object> style) {
    if (style == null) {
      return "";
    }
    Object temp = style.get(Constants.Name.COLOR);
    return temp == null ? "" : temp.toString();
  }

  public static int getFontWeight(Map<String, Object> style) {
    int typeface = android.graphics.Typeface.NORMAL;
    if (style != null) {
      Object temp = style.get(Constants.Name.FONT_WEIGHT);
      if (temp != null) {
        String fontWeight = temp.toString();
        switch (fontWeight){
          case "600":
          case "700":
          case "800":
          case "900":
          case Constants.Value.BOLD:
            typeface=Typeface.BOLD;
            break;
        }
      }
    }
    return typeface;
  }

  public static int getFontStyle(Map<String, Object> style) {
    int typeface = android.graphics.Typeface.NORMAL;
    if (style == null) {
      return typeface;
    }
    Object temp = style.get(Constants.Name.FONT_STYLE);
    if (temp == null) {
      return typeface;
    }
    String fontWeight = temp.toString();
    if (fontWeight.equals(Constants.Value.ITALIC)) {
      typeface = android.graphics.Typeface.ITALIC;
    }
    return typeface;
  }

  public static int getFontSize(Map<String, Object> style,int viewPortW) {
    if (style == null) {
      return (int) WXViewUtils.getRealPxByWidth(WXText.sDEFAULT_SIZE,viewPortW);
    }
    int fontSize = WXUtils.getInt(style.get(Constants.Name.FONT_SIZE));
    if (fontSize <= 0) {
      fontSize = WXText.sDEFAULT_SIZE;
    }
    return (int) WXViewUtils.getRealPxByWidth(fontSize,viewPortW);
  }

  public static String getFontFamily(Map<String, Object> style) {
    String fontFamily = null;
    if (style != null) {
      Object temp;
      temp = style.get(Constants.Name.FONT_FAMILY);
      if (temp != null) {
        fontFamily = temp.toString();
      }
    }
    return fontFamily;
  }

  public static Layout.Alignment getTextAlignment(Map<String, Object> style){
    Layout.Alignment alignment= Layout.Alignment.ALIGN_NORMAL;
    String textAlign= (String) style.get(Constants.Name.TEXT_ALIGN);
    if(TextUtils.equals(Constants.Value.LEFT,textAlign)){
      alignment= Layout.Alignment.ALIGN_NORMAL;
    }
    else if(TextUtils.equals(Constants.Value.CENTER,textAlign)){
      alignment=Layout.Alignment.ALIGN_CENTER;
    }
    else if(TextUtils.equals(Constants.Value.RIGHT,textAlign)){
      alignment= Layout.Alignment.ALIGN_OPPOSITE;
    }
    return alignment;
  }

  public static TextUtils.TruncateAt getTextOverflow(Map<String, Object> style){
    TextUtils.TruncateAt truncateAt=null;
    String ellipse = (String) style.get(Constants.Name.TEXT_OVERFLOW);
    if(TextUtils.equals(Constants.Name.ELLIPSIS,ellipse)){
      truncateAt = TextUtils.TruncateAt.END;
    }
    return truncateAt;
  }

  public static int getLines(Map<String, Object> style) {
    return WXUtils.getInt(style.get(Constants.Name.LINES));
  }

  public static int getLineHeight(Map<String, Object> style,int viewPortW){
    if (style == null) {
      return UNSET;
    }
    int lineHeight = WXUtils.getInt(style.get(Constants.Name.LINE_HEIGHT));
    if (lineHeight <= 0) {
      lineHeight = UNSET;
      return lineHeight;
    }
    return (int) WXViewUtils.getRealPxByWidth(lineHeight,viewPortW);
  }
  /*
   * flexbox
   **/
  public CSSAlign getAlignItems() {
    Object alignItems = get(Constants.Name.ALIGN_ITEMS);
    if (alignItems == null) {
      return CSSAlign.STRETCH;
    }
    return CSSAlignConvert.convert2AlignItems(alignItems.toString().trim());
  }

  public CSSAlign getAlignSelf() {
    Object alignSelf = get(Constants.Name.ALIGN_SELF);
    if (alignSelf == null) {
      return CSSAlign.AUTO;
    }
    return CSSAlignConvert.convert2AlignSelf(alignSelf.toString().trim());
  }

  public float getFlex() {
    return WXUtils.getFloat(get(Constants.Name.FLEX));
  }

  public CSSFlexDirection getFlexDirection() {
    Object flexDirection = get(Constants.Name.FLEX_DIRECTION);
    if (flexDirection == null) {
      return CSSFlexDirection.COLUMN;
    }
    return CSSFlexDirectionConvert.convert(flexDirection.toString().trim());
  }

  public CSSJustify getJustifyContent() {
    Object justifyContent = get(Constants.Name.JUSTIFY_CONTENT);
    if (justifyContent == null) {
      return CSSJustify.FLEX_START;
    }
    return CSSJustifyConvert.convert(justifyContent.toString().trim());
  }

  public CSSWrap getCSSWrap() {
    Object cssWrap = get(Constants.Name.FLEX_WRAP);
    if (cssWrap == null) {
      return CSSWrap.NOWRAP;
    }
    return CSSWrapConvert.convert(cssWrap.toString().trim());
  }

  /*
   * base
   **/
  public float getWidth() {
    return WXUtils.getFloat(get(Constants.Name.WIDTH));
  }

  public float getDefaultWidth() {
    return WXUtils.getFloat(get(Constants.Name.DEFAULT_WIDTH));
  }

  public float getMinWidth() {
    return WXUtils.getFloat(get(Constants.Name.MIN_WIDTH));
  }

  public float getMaxWidth() {
    return WXUtils.getFloat(get(Constants.Name.MAX_WIDTH));
  }

  public float getHeight() {
    return WXUtils.getFloat(get(Constants.Name.HEIGHT));
  }

  public float getDefaultHeight() {
    return WXUtils.getFloat(get(Constants.Name.DEFAULT_HEIGHT));
  }

  public float getMinHeight() {
    return WXUtils.getFloat(get(Constants.Name.MIN_HEIGHT));
  }

  public float getMaxHeight() {
    return WXUtils.getFloat(get(Constants.Name.MAX_HEIGHT));
  }

  /*
   * border
   **/
  public float getBorderRadius() {
    float temp = WXUtils.getFloat(get(Constants.Name.BORDER_RADIUS));
    if (WXUtils.isUndefined(temp)) {
      return Float.NaN;
    }
    return temp;
  }

  public float getBorderTopWidth() {
    return getBorderWidth(Constants.Name.BORDER_TOP_WIDTH);
  }

  private float getBorderWidth(String key) {
    float temp = WXUtils.getFloat(get(key));
    if (WXUtils.isUndefined(temp)) {
      return getBorderWidth();
    }
    return temp;
  }

  //TODO fix : only when set backgroundColor
  public float getBorderWidth() {
    return WXUtils.getFloat(get(Constants.Name.BORDER_WIDTH));
  }

  public float getBorderRightWidth() {
    return getBorderWidth(Constants.Name.BORDER_RIGHT_WIDTH);
  }

  public float getBorderBottomWidth() {
    return getBorderWidth(Constants.Name.BORDER_BOTTOM_WIDTH);
  }

  public float getBorderLeftWidth() {
    return getBorderWidth(Constants.Name.BORDER_LEFT_WIDTH);
  }

  public String getBorderColor() {
    Object color = get(Constants.Name.BORDER_COLOR);
    return color == null ? null : color.toString();
  }

  public String getBorderStyle() {
    Object borderStyle = get(Constants.Name.BORDER_STYLE);
    return borderStyle == null ? null : borderStyle.toString();
  }

  public float getMargin(){
    return WXUtils.getFloat(get(Constants.Name.MARGIN));
  }

  public float getPadding(){
    return WXUtils.getFloat(get(Constants.Name.PADDING));
  }

  /*
   * margin
   **/
  public float getMarginTop() {
    float temp = WXUtils.getFloat(get(Constants.Name.MARGIN_TOP));
    if (WXUtils.isUndefined(temp)) {
      temp = WXUtils.getFloat(get(Constants.Name.MARGIN));
    }
    return temp;
  }

  public float getMarginLeft() {
    float temp = WXUtils.getFloat(get(Constants.Name.MARGIN_LEFT));
    if (WXUtils.isUndefined(temp)) {
      temp = WXUtils.getFloat(get(Constants.Name.MARGIN));
    }
    return temp;
  }

  public float getMarginRight() {
    float temp = WXUtils.getFloat(get(Constants.Name.MARGIN_RIGHT));
    if (WXUtils.isUndefined(temp)) {
      temp = WXUtils.getFloat(get(Constants.Name.MARGIN));
    }
    return temp;
  }

  public float getMarginBottom() {
    float temp = WXUtils.getFloat(get(Constants.Name.MARGIN_BOTTOM));
    if (WXUtils.isUndefined(temp)) {
      temp = WXUtils.getFloat(get(Constants.Name.MARGIN));
    }
    return temp;
  }

  /*
   * padding
   **/
  public float getPaddingTop() {
    float temp = WXUtils.getFloat(get(Constants.Name.PADDING_TOP));
    if (WXUtils.isUndefined(temp)) {
      temp = WXUtils.getFloat(get(Constants.Name.PADDING));
    }
    return temp;
  }

  public float getPaddingLeft() {
    float temp = WXUtils.getFloat(get(Constants.Name.PADDING_LEFT));
    if (WXUtils.isUndefined(temp)) {
      temp = WXUtils.getFloat(get(Constants.Name.PADDING));
    }
    return temp;
  }

  public float getPaddingRight() {
    float temp = WXUtils.getFloat(get(Constants.Name.PADDING_RIGHT));
    if (WXUtils.isUndefined(temp)) {
      temp = WXUtils.getFloat(get(Constants.Name.PADDING));
    }
    return temp;
  }

  public float getPaddingBottom() {
    float temp = WXUtils.getFloat(get(Constants.Name.PADDING_BOTTOM));
    if (WXUtils.isUndefined(temp)) {
      temp = WXUtils.getFloat(get(Constants.Name.PADDING));
    }
    return temp;
  }

  /*
   * position
   **/
  public CSSPositionType getPosition() {
    Object position = get(Constants.Name.POSITION);
    if (position == null) {
      return CSSPositionType.RELATIVE;
    }
    return CSSPositionTypeConvert.convert(position.toString().trim());
  }

  public boolean isSticky() {
    Object position = get(Constants.Name.POSITION);
    if (position == null) {
      return false;
    }
    return position.toString().equals(Constants.Value.STICKY);
  }

  public boolean isFixed() {
    Object position = get(Constants.Name.POSITION);
    if (position == null) {
      return false;
    }
    return position.toString().equals(Constants.Value.FIXED);
  }

  public float getLeft() {
    return WXUtils.getFloat(get(Constants.Name.LEFT));
  }

  public float getTop() {
    return WXUtils.getFloat(get(Constants.Name.TOP));
  }

  public float getRight() {
    return WXUtils.getFloat(get(Constants.Name.RIGHT));
  }

  public float getBottom() {
    return WXUtils.getFloat(get(Constants.Name.BOTTOM));
  }

  /*
   * others
   **/
  public String getBackgroundColor() {
    Object temp = get(Constants.Name.BACKGROUND_COLOR);
    return temp == null ? "" : temp.toString();
  }

  public int getTimeFontSize() {
    int fontSize = WXUtils.getInt(get("timeFontSize"));
    if (fontSize <= 0) {
      fontSize = WXText.sDEFAULT_SIZE;
    }
    return fontSize;
  }

  public float getOpacity() {
    Object object = get(Constants.Name.OPACITY);
    float opacity = 1;
    if (object == null) {
      return opacity;
    }
    return WXUtils.getFloat(object);
  }

  public String getOverflow() {
    Object obj = get(Constants.Name.OVERFLOW);
    return obj == null ? Constants.Value.VISIBLE : obj.toString();
  }

  @Override
  public boolean equals(Object o) {
    return mStyles.equals(o);
  }

  @Override
  public int hashCode() {
    return mStyles.hashCode();
  }

  @Override
  public void clear() {
    mStyles.clear();
  }

  @Override
  public boolean containsKey(Object key) {
    return mStyles.containsKey(key);
  }

  @Override
  public boolean containsValue(Object value) {
    return mStyles.containsValue(value);
  }

  @NonNull
  @Override
  public Set<Entry<String, Object>> entrySet() {
    return mStyles.entrySet();
  }

  @Override
  public Object get(Object key) {
    return mStyles.get(key);
  }

  @Override
  public boolean isEmpty() {
    return mStyles.isEmpty();
  }

  @NonNull
  @Override
  public Set<String> keySet() {
    return mStyles.keySet();
  }

  @Override
  public Object put(String key, Object value) {
    return mStyles.put(key,value);
  }

  @Override
  public void putAll(Map<? extends String, ?> map) {
    this.mStyles.putAll(map);
  }

  /**
   * Used by Dom Thread， new and update styles.
   * @param map
   * @param byPesudo
   */
  public void putAll(Map<? extends String, ?> map, boolean byPesudo) {
    this.mStyles.putAll(map);
    if (!byPesudo) {
      this.mPesudoResetStyleMap.putAll(map);
      processPesudoClasses(map);
    }
  }


  public Map<String, Object> getPesudoResetStyles() {
    return mPesudoResetStyleMap;
  }

  public Map<String, Map<String, Object>> getPesudoStyles() {
    return mPesudoStyleMap;
  }

  <T extends String, V> void processPesudoClasses(Map<T, V> styles) {
    Iterator<Map.Entry<T, V>> iterator = styles.entrySet().iterator();
    Map<String, Map<String, Object>> pesudoStyleMap = mPesudoStyleMap;
    while (iterator.hasNext()) {
      Map.Entry<T, V> entry = iterator.next();
      //Key Format: "style-prop:pesudo_clz1:pesudo_clz2"
      String key = entry.getKey();
      int i;
      if ((i = key.indexOf(":")) > 0) {
        String clzName = key.substring(i);
        if (clzName.equals(Constants.PSEUDO.ENABLED)) {
          //enabled, use as regular style
          String styleKey = key.substring(0, i);
          this.mStyles.put(styleKey, entry.getValue());
          this.mPesudoResetStyleMap.put(styleKey, entry.getValue());
          continue;
        } else {
          clzName = clzName.replace(Constants.PSEUDO.ENABLED, "");//remove ':enabled' which is ignored
        }

        Map<String, Object> stylesMap = pesudoStyleMap.get(clzName);
        if (stylesMap == null) {
          stylesMap = new ArrayMap<>();
          pesudoStyleMap.put(clzName, stylesMap);
        }
        stylesMap.put(key.substring(0, i), entry.getValue());
      }
    }
  }

  @Override
  public Object remove(Object key) {
    return mStyles.remove(key);
  }

  @Override
  public int size() {
    return mStyles.size();
  }

  @NonNull
  @Override
  public Collection<Object> values() {
    return mStyles.values();
  }

  @Override
  protected WXStyle clone(){
    WXStyle style = new WXStyle();
    style.mStyles.putAll(this.mStyles);

    for(Entry<String,Map<String,Object>> entry:this.mPesudoStyleMap.entrySet()){
      Map<String,Object> valueClone = new ArrayMap<>();
      valueClone.putAll(entry.getValue());
      style.mPesudoStyleMap.put(entry.getKey(),valueClone);
    }

    style.mPesudoResetStyleMap.putAll(this.mPesudoResetStyleMap);
    return style;
  }
}
