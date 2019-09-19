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
import android.support.annotation.RestrictTo;
import android.support.annotation.RestrictTo.Scope;
import android.support.v4.util.ArrayMap;
import android.text.Layout;
import android.text.TextUtils;
import com.taobao.weex.common.Constants;
import com.taobao.weex.dom.binding.ELUtils;
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

  @NonNull
  private Map<String,Object> mStyles;

  @Nullable
  private Map<String,Map<String,Object>> mPesudoStyleMap;// clz_group:{styleMap}

  @Nullable
  private Map<String,Object> mPesudoResetStyleMap;

  /**
   * dynamic binding attrs, can be null, only weex use
   * */
  private ArrayMap<String, Object>  mBindingStyle;

  public WXStyle(){
    mStyles = new ArrayMap<>();
  }

  public WXStyle(Map<String, Object> styles){
    this.mStyles = styles;
    processPesudoClasses(this.mStyles);
  }

  public WXStyle(Map<String, Object> mStyles, boolean byPesudo) {
    this();
    this.putAll(mStyles, byPesudo);
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
    Object obj;
    WXTextDecoration ret;
    if (style == null || (obj = style.get(Constants.Name.TEXT_DECORATION)) == null) {
      ret = WXTextDecoration.NONE;
    } else {
      String textDecoration = obj.toString();
      switch (textDecoration) {
        case "underline":
          ret = WXTextDecoration.UNDERLINE;
          break;
        case "line-through":
          ret = WXTextDecoration.LINETHROUGH;
          break;
        case "none":
          ret = WXTextDecoration.NONE;
          break;
        default:
          ret = WXTextDecoration.INVALID;
          break;
      }
    }
    return ret;
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
    return getTextAlignment(style, false);
  }

  public static Layout.Alignment getTextAlignment(Map<String, Object> style, boolean isRTL){
    Layout.Alignment alignment= isRTL ? Layout.Alignment.ALIGN_OPPOSITE : Layout.Alignment.ALIGN_NORMAL;
    String textAlign= (String) style.get(Constants.Name.TEXT_ALIGN);
    if(TextUtils.equals(Constants.Value.LEFT,textAlign)){
      alignment= Layout.Alignment.ALIGN_NORMAL;
    }
    else if(TextUtils.equals(Constants.Value.CENTER,textAlign)){
      alignment= Layout.Alignment.ALIGN_CENTER;
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

  public float getBorderRadius() {
    float temp = WXUtils.getFloat(get(Constants.Name.BORDER_RADIUS));
    if (WXUtils.isUndefined(temp)) {
      return Float.NaN;
    }
    return temp;
  }

  public String getBorderColor() {
    Object color = get(Constants.Name.BORDER_COLOR);
    return color == null ? null : color.toString();
  }

  public String getBorderStyle() {
    Object borderStyle = get(Constants.Name.BORDER_STYLE);
    return borderStyle == null ? null : borderStyle.toString();
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
    float temp = WXUtils.getFloat(get(Constants.Name.LEFT));
    if (WXUtils.isUndefined(temp)) {
      return Float.NaN;
    }
    return temp;
  }

  public float getRight() {
    float temp = WXUtils.getFloat(get(Constants.Name.RIGHT));
    if (WXUtils.isUndefined(temp)) {
      return Float.NaN;
    }
    return temp;
  }

  public float getTop() {
    float temp = WXUtils.getFloat(get(Constants.Name.TOP));
    if (WXUtils.isUndefined(temp)) {
      return Float.NaN;
    }
    return temp;
  }

  public float getBottom() {
    float temp = WXUtils.getFloat(get(Constants.Name.BOTTOM));
    if (WXUtils.isUndefined(temp)) {
      return Float.NaN;
    }
    return temp;
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
    if(addBindingStyleIfStatement(key, value)){
      return null;
    }
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
      processPesudoClasses(map);
    }
  }

  public void updateStyle(Map<? extends String, ?> map, boolean byPesudo){
      parseBindingStylesStatements(map);
      putAll(map, byPesudo);
  }


  public Map<String, Object> getPesudoResetStyles() {
    if(mPesudoResetStyleMap == null){
      mPesudoResetStyleMap = new ArrayMap<>();
    }
    return mPesudoResetStyleMap;
  }

  public Map<String, Map<String, Object>> getPesudoStyles() {
    if(mPesudoStyleMap == null){
      mPesudoStyleMap = new ArrayMap<>();
    }
    return mPesudoStyleMap;
  }

  <T extends String, V> void processPesudoClasses(Map<T, V> styles) {
    Map<String, Object> tempMap = null;
    for(Map.Entry<T, V> entry:styles.entrySet()){
      //Key Format: "style-prop:pesudo_clz1:pesudo_clz2"
      String key = entry.getKey();
      int i;
      if ((i = key.indexOf(":")) > 0) {
        initPesudoMapsIfNeed(styles);
        String clzName = key.substring(i);
        if (clzName.equals(Constants.PSEUDO.ENABLED)) {
          //enabled, use as regular style
          String styleKey = key.substring(0, i);
          if(tempMap == null){
            tempMap = new ArrayMap<>();
          }
          tempMap.put(styleKey, entry.getValue());
          mPesudoResetStyleMap.put(styleKey, entry.getValue());
          continue;
        } else {
          clzName = clzName.replace(Constants.PSEUDO.ENABLED, "");//remove ':enabled' which is ignored
        }

        Map<String, Object> stylesMap = mPesudoStyleMap.get(clzName);
        if (stylesMap == null) {
          stylesMap = new ArrayMap<>();
          mPesudoStyleMap.put(clzName, stylesMap);
        }
        stylesMap.put(key.substring(0, i), entry.getValue());
      }
    }

    if (tempMap != null && !tempMap.isEmpty()) {
      this.mStyles.putAll(tempMap);
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



  private void initPesudoMapsIfNeed(Map<? extends String, ?> styles){
    if(mPesudoStyleMap == null){
      mPesudoStyleMap = new ArrayMap<>();
    }
    if(mPesudoResetStyleMap == null){
      mPesudoResetStyleMap = new ArrayMap<>();
    }
    if(mPesudoResetStyleMap.isEmpty()){
      mPesudoResetStyleMap.putAll(styles);
    }
  }



  public void  parseStatements(){
    if(this.mStyles != null){
      this.mStyles = parseBindingStylesStatements(this.mStyles);
    }
  }

  /**
   * filter dynamic state ment
   * */
  private Map<String, Object> parseBindingStylesStatements(Map styles) {
    if(styles == null || styles.size() == 0){
      return styles;
    }
    Set<Map.Entry<String,Object>> entries = styles.entrySet();
    Iterator<Entry<String,Object>> it =  entries.iterator();
    while (it.hasNext()){
      Map.Entry<String,Object> entry = it.next();
      if(addBindingStyleIfStatement(entry.getKey(), entry.getValue())){
        if(mPesudoStyleMap != null){
          mPesudoStyleMap.remove(entry.getKey());
        }
        if(mPesudoResetStyleMap != null){
          mPesudoResetStyleMap.remove(entry.getKey());
        }
        it.remove();
      }
    }
    return styles;
  }

  /**
   * filter dynamic attrs and statements
   * */
  private boolean addBindingStyleIfStatement(String key, Object value) {
    if(ELUtils.isBinding(value)){
      if(mBindingStyle == null){
        mBindingStyle = new ArrayMap<String, Object>();
      }
      value = ELUtils.bindingBlock(value);
      mBindingStyle.put(key, value);
      return  true;
    }
    return  false;
  }

  public ArrayMap<String, Object> getBindingStyle() {
    return mBindingStyle;
  }

  @Override
  public WXStyle clone(){
    WXStyle style = new WXStyle();
    style.mStyles.putAll(this.mStyles);
    if(mBindingStyle != null){
      style.mBindingStyle = new ArrayMap<>(mBindingStyle);
    }
    if(mPesudoStyleMap != null) {
      style.mPesudoStyleMap = new ArrayMap<>();
      for (Entry<String, Map<String, Object>> entry : this.mPesudoStyleMap.entrySet()) {
        Map<String, Object> valueClone = new ArrayMap<>();
        valueClone.putAll(entry.getValue());
        style.mPesudoStyleMap.put(entry.getKey(), valueClone);
      }
    }

    if(mPesudoResetStyleMap!=null) {
      style.mPesudoResetStyleMap = new ArrayMap<>();
      style.mPesudoResetStyleMap.putAll(this.mPesudoResetStyleMap);
    }


    return style;
  }

  @Override
  @RestrictTo(Scope.LIBRARY)
  public String toString() {
    return mStyles.toString();
  }
}
