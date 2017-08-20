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
import com.taobao.weex.common.Constants;
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

public class WXAnimationBean {

  public final static String LINEAR = "linear";
  public final static String EASE_IN_OUT = "ease-in-out";
  public final static String EASE_IN = "ease-in";
  public final static String EASE_OUT = "ease-out";
  public final static String CUBIC_BEZIER = "cubic-bezier";
  public final static int NUM_CUBIC_PARAM = 4;
  public long delay;
  public long duration;
  public String timingFunction;
  public Style styles;
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
    public static Map<String, List<Property<View,Float>>> wxToAndroidMap = new HashMap<>();

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
      wxToAndroidMap = Collections.unmodifiableMap(wxToAndroidMap);
    }

    public String opacity;
    public String backgroundColor;
    public String width;
    public String height;
    public String transform;
    public String transformOrigin;
    private Map<Property<View, Float>, Float> transformMap = new HashMap<>();
    private Pair<Float, Float> pivot;
    private List<PropertyValuesHolder> holders=new LinkedList<>();

    private static Map<Property<View,Float>, Float> parseTransForm(@Nullable String rawTransform, final int width,
                                                final int height,final int viewportW) {
      if (!TextUtils.isEmpty(rawTransform)) {
        FunctionParser<Property<View,Float>, Float> parser = new FunctionParser<>
            (rawTransform, new FunctionParser.Mapper<Property<View,Float>, Float>() {
              @Override
              public Map<Property<View,Float>, Float> map(String functionName, List<String> raw) {
                if (raw != null && !raw.isEmpty()) {
                  if (wxToAndroidMap.containsKey(functionName)) {
                    return convertParam(width, height,viewportW, wxToAndroidMap.get(functionName), raw);
                  }
                }
                return new HashMap<>();
              }

              private Map<Property<View,Float>, Float> convertParam(int width, int height,int viewportW,
                                                      @NonNull List<Property<View,Float>> propertyList,
                                                      @NonNull List<String> rawValue) {
                Map<Property<View,Float>, Float> result = WXDataStructureUtil.newHashMapWithExpectedSize(propertyList.size());
                List<Float> convertedList = new ArrayList<>(propertyList.size());
                if (propertyList.contains(View.ROTATION)) {
                  convertedList.addAll(parseRotationZ(rawValue));
                }else if(propertyList.contains(View.ROTATION_X)) {
                  convertedList.addAll(parseRotationXY(rawValue));
                }else if(propertyList.contains(View.ROTATION_Y)) {
                  convertedList.addAll(parseRotationXY(rawValue));
                }else if (propertyList.contains(View.TRANSLATION_X) ||
                           propertyList.contains(View.TRANSLATION_Y)) {
                  convertedList.addAll(parseTranslation(propertyList, width, height, rawValue,viewportW));
                } else if (propertyList.contains(View.SCALE_X) ||
                           propertyList.contains(View.SCALE_Y)) {
                  convertedList.addAll(parseScale(propertyList.size(), rawValue));
                }
                if (propertyList.size() == convertedList.size()) {
                  for (int i = 0; i < propertyList.size(); i++) {
                    result.put(propertyList.get(i), convertedList.get(i));
                  }
                }
                return result;
              }

              private List<Float> parseScale(int size, @NonNull List<String> rawValue) {
                List<Float> convertedList = new ArrayList<>(rawValue.size() * 2);
                List<Float> rawFloat = new ArrayList<>(rawValue.size());
                for (String item : rawValue) {
                  rawFloat.add(WXUtils.fastGetFloat(item));
                }
                convertedList.addAll(rawFloat);
                if (size != 1 && rawValue.size() == 1) {
                  convertedList.addAll(rawFloat);
                }
                return convertedList;
              }

              private List<Float> parseRotationXY(@NonNull List<String> rawValue) {
                List<Float> intermediate = parseRotationZ(rawValue);
                for (int i = 0; i < intermediate.size(); i++) {
                  intermediate.set(i, -intermediate.get(i));
                }
                return intermediate;
              }

              private @NonNull List<Float> parseRotationZ(@NonNull List<String> rawValue) {
                List<Float> convertedList = new ArrayList<>(1);
                int suffix;
                for (String raw : rawValue) {
                  if ((suffix = raw.lastIndexOf(DEG)) != -1) {
                    convertedList.add(WXUtils.fastGetFloat(raw.substring(0, suffix)));
                  } else {
                    convertedList.add((float) Math.toDegrees(Double.parseDouble(raw)));
                  }
                }
                return convertedList;
              }

              /**
               * As "translate(50%, 25%)" or "translate(25px, 30px)" both are valid,
               * parsing translate is complicated than other method.
               * Add your waste time here if you try to optimize this method like {@link #parseScale(int, List)}
               * Time: 0.5h
               */
              private List<Float> parseTranslation(List<Property<View,Float>> propertyList,
                                                   int width, int height,
                                                   @NonNull List<String> rawValue,int viewportW) {
                List<Float> convertedList = new ArrayList<>(2);
                String first = rawValue.get(0);
                if (propertyList.size() == 1) {
                  parseSingleTranslation(propertyList, width, height, convertedList, first,viewportW);
                } else {
                  parseDoubleTranslation(width, height, rawValue, convertedList, first,viewportW);
                }
                return convertedList;
              }

              private void parseSingleTranslation(List<Property<View,Float>> propertyList, int width, int height,
                                                  List<Float> convertedList, String first,int viewportW) {
                if (propertyList.contains(View.TRANSLATION_X)) {
                  convertedList.add(parsePercentOrPx(first, width,viewportW));
                } else if (propertyList.contains(View.TRANSLATION_Y)) {
                  convertedList.add(parsePercentOrPx(first, height,viewportW));
                }
              }

              private void parseDoubleTranslation(int width, int height,
                                                  @NonNull List<String> rawValue,
                                                  List<Float> convertedList, String first,int viewportW) {
                String second;
                if (rawValue.size() == 1) {
                  second = first;
                } else {
                  second = rawValue.get(1);
                }
                convertedList.add(parsePercentOrPx(first, width,viewportW));
                convertedList.add(parsePercentOrPx(second, height,viewportW));
              }
            });
        return parser.parse();
      }
      return new LinkedHashMap<>();
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

    private static @NonNull Map<Property<View, Float>, Float> createDefaultTransform(){
      Map<Property<View, Float>, Float> defaultMap= new ArrayMap<>(5);
      defaultMap.put(View.TRANSLATION_X, 0f);
      defaultMap.put(View.TRANSLATION_Y, 0f);
      defaultMap.put(View.SCALE_X, 1f);
      defaultMap.put(View.SCALE_Y, 1f);
      defaultMap.put(View.ROTATION, 0f);
      return defaultMap;
    }

    public @Nullable Pair<Float, Float> getPivot() {
      return pivot;
    }

    public void init(@Nullable String transformOrigin,@Nullable String rawTransform,
                     final int width, final int height,int viewportW){
      pivot = parsePivot(transformOrigin,width,height,viewportW);
      transformMap = createDefaultTransform();
      transformMap.putAll(parseTransForm(rawTransform,width,height,viewportW));
      initHolders();
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
  }
}
