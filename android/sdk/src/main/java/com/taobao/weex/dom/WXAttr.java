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

import static java.lang.Boolean.parseBoolean;

import android.support.annotation.NonNull;
import android.support.v4.util.ArrayMap;
import android.text.TextUtils;
import com.taobao.weex.common.Constants;
import com.taobao.weex.common.Constants.Name;
import com.taobao.weex.common.WXImageSharpen;
import com.taobao.weex.ui.view.listview.WXRecyclerView;
import com.taobao.weex.utils.WXLogUtils;
import com.taobao.weex.utils.WXUtils;
import com.taobao.weex.utils.WXViewUtils;
import java.util.Collection;
import java.util.Locale;
import java.util.Map;
import java.util.Set;

/**
 * store value of component attribute
 *
 */
public class WXAttr implements Map<String, Object>,Cloneable {

  private static final long serialVersionUID = -2619357510079360946L;
  private @NonNull final ArrayMap<String, Object> map;

  public WXAttr(){
    map=new ArrayMap<>();
  }

  public WXAttr(@NonNull Map<String,Object> standardMap) {
    this();
    map.putAll(standardMap);
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
    return map.equals(o);
  }

  @Override
  public int hashCode() {
    return map.hashCode();
  }

  @Override
  public void clear() {
    map.clear();
  }

  @Override
  public boolean containsKey(Object key) {
    return map.containsKey(key);
  }

  @Override
  public boolean containsValue(Object value) {
    return map.containsValue(value);
  }

  @NonNull
  @Override
  public Set<Entry<String, Object>> entrySet() {
    return map.entrySet();
  }

  @Override
  public Object get(Object key) {
    return map.get(key);
  }

  @Override
  public boolean isEmpty() {
    return map.isEmpty();
  }

  @NonNull
  @Override
  public Set<String> keySet() {
    return map.keySet();
  }

  @Override
  public Object put(String key, Object value) {
    return map.put(key,value);
  }

  @Override
  public void putAll(Map<? extends String, ?> map) {
    this.map.putAll(map);
  }

  @Override
  public Object remove(Object key) {
    return map.remove(key);
  }

  @Override
  public int size() {
    return map.size();
  }

  @NonNull
  @Override
  public Collection<Object> values() {
    return map.values();
  }

  @Override
  protected WXAttr clone(){
    return new WXAttr(map);
  }
}
