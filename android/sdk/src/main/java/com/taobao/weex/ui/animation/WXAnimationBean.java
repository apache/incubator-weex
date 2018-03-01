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
package com.taobao.weex.ui.animation;

import android.animation.PropertyValuesHolder;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v4.util.ArrayMap;
import android.text.TextUtils;
import android.util.Pair;
import android.util.Property;
import android.view.View;
import com.taobao.weex.WXEnvironment;
import com.taobao.weex.common.Constants;
import com.taobao.weex.common.Constants.Name;
import com.taobao.weex.utils.FunctionParser;
import com.taobao.weex.utils.WXDataStructureUtil;
import com.taobao.weex.utils.WXUtils;
import com.taobao.weex.utils.WXViewUtils;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

public class WXAnimationBean {

  public final static int NUM_CUBIC_PARAM = 4;
  public long delay;
  public long duration;
  public String timingFunction;
  public @Nullable Style styles;
  public boolean needLayout;

  public static class Style {

    public final static String WX_TRANSLATE = "translate";
    public final static String WX_TRANSLATE_X = "translateX";
    public final static String WX_TRANSLATE_Y = "translateY";
    public final static String WX_ROTATE = "rotate";
    public final static String WX_SCALE_X = "scaleX";
    public final static String WX_SCALE_Y = "scaleY";
    public final static String WX_SCALE = "scale";
    public final static String WX_ROTATE_X ="rotateX";
    public final static String WX_ROTATE_Y ="rotateY";
    public final static String BACKGROUND_COLOR = Constants.Name.BACKGROUND_COLOR;
    public final static String WIDTH = Constants.Name.WIDTH;
    public final static String HEIGHT = Constants.Name.HEIGHT;
    public final static String TOP = "top";
    public final static String BOTTOM = "bottom";
    public final static String RIGHT = "right";
    public final static String LEFT = "left";
    public final static String CENTER = "center";
    private static final String HALF = "50%";
    private static final String FULL = "100%";
    private static final String ZERO = "0%";
    private static final String PX = "px";
    private static final String DEG = "deg";
    public static Map<String, List<Property<View,Float>>> wxToAndroidMap = new ArrayMap<>();
    private static Map<Property<View, Float>, Float> defaultMap= new ArrayMap<>();


    static {
      wxToAndroidMap.put(WX_TRANSLATE, Arrays.asList
          (View.TRANSLATION_X, View.TRANSLATION_Y));
      wxToAndroidMap.put(WX_TRANSLATE_X, Collections.singletonList(View.TRANSLATION_X));
      wxToAndroidMap.put(WX_TRANSLATE_Y, Collections.singletonList(View.TRANSLATION_Y));
      wxToAndroidMap.put(WX_ROTATE, Collections.singletonList(View.ROTATION));
      wxToAndroidMap.put(WX_ROTATE_X, Collections.singletonList(View.ROTATION_X));
      wxToAndroidMap.put(WX_ROTATE_Y, Collections.singletonList(View.ROTATION_Y));
      wxToAndroidMap.put(WX_SCALE, Arrays.asList(View.SCALE_X, View.SCALE_Y));
      wxToAndroidMap.put(WX_SCALE_X, Collections.singletonList(View.SCALE_X));
      wxToAndroidMap.put(WX_SCALE_Y, Collections.singletonList(View.SCALE_Y));
      wxToAndroidMap.put(Name.PERSPECTIVE, Collections.singletonList(CameraDistanceProperty.getInstance()));
      wxToAndroidMap = Collections.unmodifiableMap(wxToAndroidMap);
      defaultMap.put(View.TRANSLATION_X, 0f);
      defaultMap.put(View.TRANSLATION_Y, 0f);
      defaultMap.put(View.SCALE_X, 1f);
      defaultMap.put(View.SCALE_Y, 1f);
      defaultMap.put(View.ROTATION, 0f);
      defaultMap.put(View.ROTATION_X, 0f);
      defaultMap.put(View.ROTATION_Y, 0f);
    }

    public String opacity;
    public String backgroundColor;
    public String width;
    public String height;
    public String transform;
    public String transformOrigin;
    private Map<Property<View, Float>, Float> transformMap = new LinkedHashMap<>();
    private Pair<Float, Float> pivot;
    private List<PropertyValuesHolder> holders=new LinkedList<>();
    private float cameraDistance = Float.MAX_VALUE;

    private static Map<Property<View,Float>, Float> parseTransForm(@Nullable String rawTransform, final int width,
                                                final int height,final int viewportW) {
      return  TransformParser.parseTransForm(rawTransform, width, height, viewportW);
    }

    private static Pair<Float, Float> parsePivot(@Nullable String transformOrigin,
                                                 int width, int height,int viewportW) {
      if (!TextUtils.isEmpty(transformOrigin)) {
        int firstSpace = transformOrigin.indexOf(FunctionParser.SPACE);
        if (firstSpace != -1) {
          int i = firstSpace;
          for (; i < transformOrigin.length(); i++) {
            if (transformOrigin.charAt(i) != FunctionParser.SPACE) {
              break;
            }
          }
          if (i < transformOrigin.length() && transformOrigin.charAt(i) != FunctionParser.SPACE) {
            List<String> list = new ArrayList<>(2);
            list.add(transformOrigin.substring(0, firstSpace).trim());
            list.add(transformOrigin.substring(i, transformOrigin.length()).trim());
            return parsePivot(list, width, height,viewportW);
          }
        }
      }
      return null;
    }

    private static Pair<Float, Float> parsePivot(@NonNull List<String> list, int width, int height,int viewportW) {
      return new Pair<>(
          parsePivotX(list.get(0), width,viewportW), parsePivotY(list.get(1), height,viewportW));
    }

    private static float parsePivotX(String x, int width,int viewportW) {
      String value = x;
      if (WXAnimationBean.Style.LEFT.equals(x)) {
        value = ZERO;
      } else if (WXAnimationBean.Style.RIGHT.equals(x)) {
        value = FULL;
      } else if (WXAnimationBean.Style.CENTER.equals(x)) {
        value = HALF;
      }
      return parsePercentOrPx(value, width,viewportW);
    }

    private static float parsePivotY(String y, int height,int viewportW) {
      String value = y;
      if (WXAnimationBean.Style.TOP.equals(y)) {
        value = ZERO;
      } else if (WXAnimationBean.Style.BOTTOM.equals(y)) {
        value = FULL;
      } else if (WXAnimationBean.Style.CENTER.equals(y)) {
        value = HALF;
      }
      return parsePercentOrPx(value, height,viewportW);
    }

    private static float parsePercentOrPx(String raw, int unit,int viewportW) {
      final int precision = 1;
      int suffix;
      if ((suffix = raw.lastIndexOf(WXUtils.PERCENT)) != -1) {
        return parsePercent(raw.substring(0, suffix), unit, precision);
      } else if ((suffix = raw.lastIndexOf(PX)) != -1) {
        return WXViewUtils.getRealPxByWidth(WXUtils.fastGetFloat(raw.substring(0, suffix), precision),viewportW);
      }
      return WXViewUtils.getRealPxByWidth(WXUtils.fastGetFloat(raw, precision),viewportW);
    }

    private static float parsePercent(String percent, int unit, int precision) {
      return WXUtils.fastGetFloat(percent, precision) / 100 * unit;
    }

    private void resetToDefaultIfAbsent() {
      for (Entry<Property<View, Float>, Float> entry : defaultMap.entrySet()) {
        if (!transformMap.containsKey(entry.getKey())) {
          transformMap.put(entry.getKey(), entry.getValue());
        }
      }
    }

    public @Nullable Pair<Float, Float> getPivot() {
      return pivot;
    }

    public void init(@Nullable String transformOrigin,@Nullable String rawTransform,
                     final int width, final int height,int viewportW){
      pivot = parsePivot(transformOrigin,width,height,viewportW);
      transformMap.putAll(parseTransForm(rawTransform,width,height,viewportW));
      resetToDefaultIfAbsent();
      if (transformMap.containsKey(CameraDistanceProperty.getInstance())) {
        cameraDistance = transformMap.remove(CameraDistanceProperty.getInstance());
      }
      initHolders();
    }

    /**
     * Use this method to init if you already have a list of Property
     * The key is something like {@link View#TRANSLATION_X} and the value is a {@link Pair},
     * of which the first is beginning value and the second is ending value.
     * @param styles a list of Property
     */
    public void init(@NonNull Map<Property<View, Float>, Pair<Float, Float>> styles){
      for(Entry<Property<View, Float>, Pair<Float, Float>> entry:styles.entrySet()){
        holders.add(PropertyValuesHolder.ofFloat(entry.getKey(), entry.getValue().first, entry.getValue().second));
      }
    }

    private void initHolders(){
      for (Map.Entry<Property<View, Float>, Float> entry : transformMap.entrySet()) {
        holders.add(PropertyValuesHolder.ofFloat(entry.getKey(), entry.getValue()));
      }
      if (!TextUtils.isEmpty(opacity)) {
        holders.add(PropertyValuesHolder.ofFloat(View.ALPHA, WXUtils.fastGetFloat(opacity, 3)));
      }
    }

    public List<PropertyValuesHolder> getHolders(){
      return holders;
    }

    public float getCameraDistance(){
      return cameraDistance;
    }
  }
}
