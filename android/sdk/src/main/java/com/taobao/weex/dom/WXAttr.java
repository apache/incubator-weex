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

import static com.taobao.weex.dom.binding.ELUtils.COMPONENT_PROPS;
import static java.lang.Boolean.parseBoolean;

import android.support.annotation.NonNull;
import android.support.v4.util.ArrayMap;
import android.text.TextUtils;

import com.taobao.weex.common.Constants;
import com.taobao.weex.common.Constants.Name;
import com.taobao.weex.common.WXImageSharpen;
import com.taobao.weex.dom.binding.ELUtils;
import com.taobao.weex.dom.binding.WXStatement;
import com.taobao.weex.ui.view.listview.WXRecyclerView;
import com.taobao.weex.utils.WXLogUtils;
import com.taobao.weex.utils.WXUtils;
import com.taobao.weex.utils.WXViewUtils;
import java.util.Collection;
import java.util.Iterator;
import java.util.Locale;
import java.util.Map;
import java.util.Set;

/**
 * store value of component attribute
 *
 */
public class WXAttr implements Map<String, Object>,Cloneable {

  private static final long serialVersionUID = -2619357510079360946L;

  /**
   * static attrs
   * */
  private @NonNull final ArrayMap<String, Object> attr;


  /**
   * dynamic binding attrs, can be null, only weex use
   * */
  private ArrayMap<String, Object>  mBindingAttrs;

  /**
   * dynamic binding statement for match, can be null, only weex use
   * */
  private WXStatement mStatement;

  public WXAttr(){
    attr =new ArrayMap<>();
  }

  public WXAttr(@NonNull Map<String,Object> standardMap) {
    this();
    attr.putAll(filterBindingStatement(standardMap));
  }

  public static String getPrefix(Map<String, Object> attr) {
    if (attr == null) {
      return null;
    }
    Object src = attr.get(Constants.Name.PREFIX);
    if (src == null) {
      return null;
    }
    return src.toString();
  }

  public static String getSuffix(Map<String, Object> attr) {
    if (attr == null) {
      return null;
    }
    Object src = attr.get(Constants.Name.SUFFIX);
    if (src == null) {
      return null;
    }
    return src.toString();
  }

  /**
   * Compatible with value、content
   *
   * @return
   */
  public static String getValue(Map<String, Object> attr) {
    if (attr == null) {
      return null;
    }
    Object src = attr.get(Constants.Name.VALUE);
    if (src == null) {
      src = attr.get("content");
      if (src == null) {
        return null;
      }
    }
    return src.toString();
  }

  public WXImageQuality getImageQuality() {
    Object obj = containsKey(Name.QUALITY) ? get(Name.QUALITY) : get(Name.IMAGE_QUALITY);
    WXImageQuality imageQuality = WXImageQuality.AUTO;
    String value;
    if (obj != null && !TextUtils.isEmpty(value = obj.toString())) {
      try {
        imageQuality = WXImageQuality.valueOf(value.toUpperCase(Locale.US));
      }catch (IllegalArgumentException e){
        WXLogUtils.e("Image", "Invalid value image quality. Only low, normal, high, original are valid");
      }
    }
    return imageQuality;
  }

  public WXImageSharpen getImageSharpen() {
    Object obj = get(Constants.Name.SHARPEN);
    if (obj == null) {
      obj = get(Constants.Name.IMAGE_SHARPEN);
    }
    if (obj == null) {
      return WXImageSharpen.UNSHARPEN;
    }
    String imageSharpen = obj.toString();
    WXImageSharpen waImageSharpen = WXImageSharpen.UNSHARPEN;
    if (imageSharpen.equals("sharpen")) {
      waImageSharpen = WXImageSharpen.SHARPEN;
    }

    return waImageSharpen;
  }

  public String getImageSrc() {
    Object src = get(Constants.Name.SRC);
    if (src == null) {
      return null;
    }
    return src.toString();
  }

  public boolean canRecycled() {
    Object obj = get(Constants.Name.RECYCLE);
    if (obj == null) {
      return true;
    }
    try {
      return parseBoolean(String.valueOf(obj));
    } catch (Exception e) {
      WXLogUtils.e("[WXAttr] recycle:", e);
    }
    return true;
  }

  public boolean showIndicators() {
    Object obj = get(Constants.Name.SHOW_INDICATORS);
    if (obj == null) {
      return true;
    }

    try {
      return parseBoolean(String.valueOf(obj));
    } catch (Exception e) {
      WXLogUtils.e("[WXAttr] showIndicators:", e);
    }
    return true;
  }

  public boolean autoPlay() {
    Object obj = get(Constants.Name.AUTO_PLAY);
    if (obj == null) {
      return false;
    }

    try {
      return parseBoolean(String.valueOf(obj));
    } catch (Exception e) {
      WXLogUtils.e("[WXAttr] autoPlay:", e);
    }
    return false;
  }

  public String getScope() {
    Object src = get(Constants.Name.SCOPE);
    if (src == null) {
      return null;
    }
    return src.toString();
  }
  public String getLoadMoreRetry() {
    Object src = get(Constants.Name.LOADMORERETRY);
    if (src == null) {
      return null;
    }
    return src.toString();
  }

  public String getLoadMoreOffset() {
    Object src = get(Constants.Name.LOADMOREOFFSET);
    if (src == null) {
      return null;
    }
    return src.toString();
  }

  public String optString(String key){
    if(containsKey(key)){
      Object value = get(key);
      if (value instanceof String) {
        return (String) value;
      } else if (value != null) {
        return String.valueOf(value);
      }
    }
    return "";
  }

  public boolean getIsRecycleImage() {
    Object obj = get(Constants.Name.RECYCLE_IMAGE);
    if (obj == null) {
      return true;
    }

    try {
      return parseBoolean(String.valueOf(obj));
    } catch (Exception e) {
      WXLogUtils.e("[WXAttr] recycleImage:", e);
    }
    return false;
  }
  public String getScrollDirection() {
    Object scrollDirection = get("scrollDirection");
    if (scrollDirection == null) {
      return "vertical";
    }
    return scrollDirection.toString();
  }

  public float getElevation(int viewPortW) {
    Object obj = get(Constants.Name.ELEVATION);
    float ret = Float.NaN;
    if (obj != null) {
      String number = obj.toString();
      if (!TextUtils.isEmpty(number)) {
        ret = WXViewUtils.getRealSubPxByWidth(WXUtils.getFloat(number),viewPortW);
      } else {
        ret = 0;
      }
    }
    return ret;
  }

  public float getColumnWidth(){

    Object obj = get(Constants.Name.COLUMN_WIDTH);
    if (obj == null) {
      return Constants.Value.AUTO;
    }

    String value = String.valueOf(obj);
    if(Constants.Name.AUTO.equals(value)){
      return Constants.Value.AUTO;
    }

    try {
      float columnWidth = Float.parseFloat(value);
      return columnWidth > 0 ? columnWidth : 0;
    } catch (Exception e) {
      WXLogUtils.e("[WXAttr] getColumnWidth:", e);
    }
    return Constants.Value.AUTO;
  }

  public int getColumnCount() {

    Object obj = get(Constants.Name.COLUMN_COUNT);
    if (obj == null) {
      return Constants.Value.AUTO;
    }

    String value = String.valueOf(obj);
    if(Constants.Name.AUTO.equals(value)){
      return Constants.Value.AUTO;
    }

    try {
      int columnCount = Integer.parseInt(value);
      return columnCount > 0 ? columnCount : Constants.Value.AUTO;
    } catch (Exception e) {
      WXLogUtils.e("[WXAttr] getColumnCount:", e);
      return Constants.Value.AUTO;
    }
  }

  public float getColumnGap() {

    Object obj = get(Constants.Name.COLUMN_GAP);
    if (obj == null) {
      return Constants.Value.COLUMN_GAP_NORMAL;
    }

    String value = String.valueOf(obj);
    if (Constants.Name.NORMAL.equals(value)) {
      return Constants.Value.COLUMN_GAP_NORMAL;
    }

    try {
      float columnGap = Float.parseFloat(value);
      return columnGap >= 0 ? columnGap : Constants.Value.AUTO;
    } catch (Exception e) {
      WXLogUtils.e("[WXAttr] getColumnGap:", e);
    }
    return Constants.Value.COLUMN_GAP_NORMAL;
  }

  public int getLayoutType(){
    Object obj = get(Constants.Name.LAYOUT);
    if (obj == null) {
      return WXRecyclerView.TYPE_LINEAR_LAYOUT;
    }

    try {
      switch(String.valueOf(obj)){
        case Constants.Value.MULTI_COLUMN :
          return  WXRecyclerView.TYPE_STAGGERED_GRID_LAYOUT;
        case Constants.Value.GRID :
          return  WXRecyclerView.TYPE_GRID_LAYOUT;
        default:
          return WXRecyclerView.TYPE_LINEAR_LAYOUT;
      }
    } catch (Exception e) {
      WXLogUtils.e("[WXAttr] getLayoutType:", e);
    }
    return WXRecyclerView.TYPE_LINEAR_LAYOUT;
  }


  @Override
  public boolean equals(Object o) {
    return attr.equals(o);
  }

  @Override
  public int hashCode() {
    return attr.hashCode();
  }

  @Override
  public void clear() {
    attr.clear();
  }

  @Override
  public boolean containsKey(Object key) {
    return attr.containsKey(key);
  }

  @Override
  public boolean containsValue(Object value) {
    return attr.containsValue(value);
  }

  @NonNull
  @Override
  public Set<Entry<String, Object>> entrySet() {
    return attr.entrySet();
  }

  @Override
  public Object get(Object key) {
    return attr.get(key);
  }

  @Override
  public boolean isEmpty() {
    return attr.isEmpty();
  }

  @NonNull
  @Override
  public Set<String> keySet() {
    return attr.keySet();
  }

  @Override
  public Object put(String key, Object value) {
    if(filterBindingStatement(key, value)){
      return null;
    }
    return attr.put(key,value);
  }

  @Override
  public void putAll(Map<? extends String, ?> map) {
    this.attr.putAll(filterBindingStatement(map));
  }

  @Override
  public Object remove(Object key) {
    return attr.remove(key);
  }

  @Override
  public int size() {
    return attr.size();
  }

  @NonNull
  @Override
  public Collection<Object> values() {
    return attr.values();
  }


  /**
   * can by null, in most contion without template list, the value is null
   * */
  public ArrayMap<String, Object> getBindingAttrs() {
    return mBindingAttrs;
  }

  /**
   * can by null, in most contion without template list, the value is null
   * */
  public WXStatement getStatement() {
    return mStatement;
  }


  public void setBindingAttrs(ArrayMap<String, Object> mBindingAttrs) {
    this.mBindingAttrs = mBindingAttrs;
  }

  public void setStatement(WXStatement mStatement) {
    this.mStatement = mStatement;
  }


  /**
   * filter dynamic state ment
   * */
  private Map<String, Object> filterBindingStatement(Map attrs) {
    if(attrs == null || attrs.size() == 0){
      return attrs;
    }
    Set<Map.Entry<String,Object>> entries = attrs.entrySet();
    Iterator<Entry<String,Object>> it =  entries.iterator();
    while (it.hasNext()){
        Map.Entry<String,Object> entry = it.next();
        if(filterBindingStatement(entry.getKey(), entry.getValue())){
           it.remove();
        }
    }
    return attrs;
  }

  /**
   * filter dynamic attrs and statements
   * */
  private boolean filterBindingStatement(String key, Object value) {
        if(COMPONENT_PROPS.equals(key)){
          ELUtils.bindingBlock(value);
          return  false;
        }
        if(ELUtils.isBinding(value)){
          if(mBindingAttrs == null){
              mBindingAttrs = new ArrayMap<String, Object>();
          }
          value = ELUtils.bindingBlock(value);
          mBindingAttrs.put(key, value);
          return  true;
        }
        if(ELUtils.isVif(key)){
          if(mStatement == null){
             mStatement = new WXStatement();
          }
          mStatement.put(key, ELUtils.vifBlock(value.toString()));
          return  true;
        }

        if(ELUtils.isVfor(key)){
           if(mStatement == null){
              mStatement = new WXStatement();
           }
           value = ELUtils.vforBlock(value);
           if(value != null) {
              mStatement.put(key, value);
              return  true;
           }
        }
        return  false;
  }

  public void skipFilterPutAll(Map<String,Object> attrs){
    this.attr.putAll(attrs);
  }

  @Override
  protected WXAttr clone(){
    WXAttr wxAttr = new WXAttr();
    wxAttr.skipFilterPutAll(attr);
    wxAttr.mBindingAttrs = mBindingAttrs;
    wxAttr.mStatement = mStatement;
    return wxAttr;
  }
}
