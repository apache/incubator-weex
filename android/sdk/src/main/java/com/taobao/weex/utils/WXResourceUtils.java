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
package com.taobao.weex.utils;

import android.graphics.Color;
import android.graphics.LinearGradient;
import android.graphics.Shader;
import android.support.annotation.NonNull;
import android.support.v4.util.LruCache;
import android.util.Pair;
import android.text.TextUtils;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.StringTokenizer;

/**
 * Class for parse color
 */
public class WXResourceUtils {

  private final static Map<String, Integer> colorMap = new HashMap<>();
  private final static int RGB_SIZE = 3;
  private final static int RGBA_SIZE = 4;
  private final static int HEX = 16;
  private final static int COLOR_RANGE = 255;
  private final static String RGB = "rgb";
  private final static String RGBA = "rgba";
  private final static SingleFunctionParser.FlatMapper<Integer> FUNCTIONAL_RGB_MAPPER =
          new SingleFunctionParser.FlatMapper<Integer>() {
            @Override
            public Integer map(String raw) {
              int color = WXUtils.parseUnitOrPercent(raw, COLOR_RANGE);
              if (color < 0) {
                color = 0;
              } else if (color > COLOR_RANGE) {
                color = COLOR_RANGE;
              }
              return color;
            }
          };

  private final static SingleFunctionParser.NonUniformMapper<Number> FUNCTIONAL_RGBA_MAPPER =
          new SingleFunctionParser.NonUniformMapper<Number>() {
            @Override
            public List<Number> map(List<String> raw) {
              List<Number> result = new ArrayList<>(RGBA_SIZE);
              int i, color;
              for (i = 0; i < RGB_SIZE; i++) {
                color = WXUtils.parseUnitOrPercent(raw.get(i), COLOR_RANGE);
                if (color < 0) {
                  color = 0;
                } else if (color > COLOR_RANGE) {
                  color = COLOR_RANGE;
                }
                result.add(color);
              }
              result.add(Float.valueOf(raw.get(i)));
              return result;
            }
          };

  static {
    colorMap.put("aliceblue", 0XFFF0F8FF);
    colorMap.put("antiquewhite", 0XFFFAEBD7);
    colorMap.put("aqua", 0XFF00FFFF);
    colorMap.put("aquamarine", 0XFF7FFFD4);
    colorMap.put("azure", 0XFFF0FFFF);
    colorMap.put("beige", 0XFFF5F5DC);
    colorMap.put("bisque", 0XFFFFE4C4);
    colorMap.put("black", 0XFF000000);
    colorMap.put("blanchedalmond", 0XFFFFEBCD);
    colorMap.put("blue", 0XFF0000FF);
    colorMap.put("blueviolet", 0XFF8A2BE2);
    colorMap.put("brown", 0XFFA52A2A);
    colorMap.put("burlywood", 0XFFDEB887);
    colorMap.put("cadetblue", 0XFF5F9EA0);
    colorMap.put("chartreuse", 0XFF7FFF00);
    colorMap.put("chocolate", 0XFFD2691E);
    colorMap.put("coral", 0XFFFF7F50);
    colorMap.put("cornflowerblue", 0XFF6495ED);
    colorMap.put("cornsilk", 0XFFFFF8DC);
    colorMap.put("crimson", 0XFFDC143C);
    colorMap.put("cyan", 0XFF00FFFF);
    colorMap.put("darkblue", 0XFF00008B);
    colorMap.put("darkcyan", 0XFF008B8B);
    colorMap.put("darkgoldenrod", 0XFFB8860B);
    colorMap.put("darkgray", 0XFFA9A9A9);
    colorMap.put("darkgreen", 0XFF006400);
    colorMap.put("darkkhaki", 0XFFBDB76B);
    colorMap.put("darkmagenta", 0XFF8B008B);
    colorMap.put("darkolivegreen", 0XFF556B2F);
    colorMap.put("darkorange", 0XFFFF8C00);
    colorMap.put("darkorchid", 0XFF9932CC);
    colorMap.put("darkred", 0XFF8B0000);
    colorMap.put("darksalmon", 0XFFE9967A);
    colorMap.put("darkseagreen", 0XFF8FBC8F);
    colorMap.put("darkslateblue", 0XFF483D8B);
    colorMap.put("darkslategray", 0XFF2F4F4F);
    colorMap.put("darkslategrey", 0XFF2F4F4F);
    colorMap.put("darkturquoise", 0XFF00CED1);
    colorMap.put("darkviolet", 0XFF9400D3);
    colorMap.put("deeppink", 0XFFFF1493);
    colorMap.put("deepskyblue", 0XFF00BFFF);
    colorMap.put("dimgray", 0XFF696969);
    colorMap.put("dimgrey", 0XFF696969);
    colorMap.put("dodgerblue", 0XFF1E90FF);
    colorMap.put("firebrick", 0XFFB22222);
    colorMap.put("floralwhite", 0XFFFFFAF0);
    colorMap.put("forestgreen", 0XFF228B22);
    colorMap.put("fuchsia", 0XFFFF00FF);
    colorMap.put("gainsboro", 0XFFDCDCDC);
    colorMap.put("ghostwhite", 0XFFF8F8FF);
    colorMap.put("gold", 0XFFFFD700);
    colorMap.put("goldenrod", 0XFFDAA520);
    colorMap.put("gray", 0XFF808080);
    colorMap.put("grey", 0XFF808080);
    colorMap.put("green", 0XFF008000);
    colorMap.put("greenyellow", 0XFFADFF2F);
    colorMap.put("honeydew", 0XFFF0FFF0);
    colorMap.put("hotpink", 0XFFFF69B4);
    colorMap.put("indianred", 0XFFCD5C5C);
    colorMap.put("indigo", 0XFF4B0082);
    colorMap.put("ivory", 0XFFFFFFF0);
    colorMap.put("khaki", 0XFFF0E68C);
    colorMap.put("lavender", 0XFFE6E6FA);
    colorMap.put("lavenderblush", 0XFFFFF0F5);
    colorMap.put("lawngreen", 0XFF7CFC00);
    colorMap.put("lemonchiffon", 0XFFFFFACD);
    colorMap.put("lightblue", 0XFFADD8E6);
    colorMap.put("lightcoral", 0XFFF08080);
    colorMap.put("lightcyan", 0XFFE0FFFF);
    colorMap.put("lightgoldenrodyellow", 0XFFFAFAD2);
    colorMap.put("lightgray", 0XFFD3D3D3);
    colorMap.put("lightgrey", 0XFFD3D3D3);
    colorMap.put("lightgreen", 0XFF90EE90);
    colorMap.put("lightpink", 0XFFFFB6C1);
    colorMap.put("lightsalmon", 0XFFFFA07A);
    colorMap.put("lightseagreen", 0XFF20B2AA);
    colorMap.put("lightskyblue", 0XFF87CEFA);
    colorMap.put("lightslategray", 0XFF778899);
    colorMap.put("lightslategrey", 0XFF778899);
    colorMap.put("lightsteelblue", 0XFFB0C4DE);
    colorMap.put("lightyellow", 0XFFFFFFE0);
    colorMap.put("lime", 0XFF00FF00);
    colorMap.put("limegreen", 0XFF32CD32);
    colorMap.put("linen", 0XFFFAF0E6);
    colorMap.put("magenta", 0XFFFF00FF);
    colorMap.put("maroon", 0XFF800000);
    colorMap.put("mediumaquamarine", 0XFF66CDAA);
    colorMap.put("mediumblue", 0XFF0000CD);
    colorMap.put("mediumorchid", 0XFFBA55D3);
    colorMap.put("mediumpurple", 0XFF9370DB);
    colorMap.put("mediumseagreen", 0XFF3CB371);
    colorMap.put("mediumslateblue", 0XFF7B68EE);
    colorMap.put("mediumspringgreen", 0XFF00FA9A);
    colorMap.put("mediumturquoise", 0XFF48D1CC);
    colorMap.put("mediumvioletred", 0XFFC71585);
    colorMap.put("midnightblue", 0XFF191970);
    colorMap.put("mintcream", 0XFFF5FFFA);
    colorMap.put("mistyrose", 0XFFFFE4E1);
    colorMap.put("moccasin", 0XFFFFE4B5);
    colorMap.put("navajowhite", 0XFFFFDEAD);
    colorMap.put("navy", 0XFF000080);
    colorMap.put("oldlace", 0XFFFDF5E6);
    colorMap.put("olive", 0XFF808000);
    colorMap.put("olivedrab", 0XFF6B8E23);
    colorMap.put("orange", 0XFFFFA500);
    colorMap.put("orangered", 0XFFFF4500);
    colorMap.put("orchid", 0XFFDA70D6);
    colorMap.put("palegoldenrod", 0XFFEEE8AA);
    colorMap.put("palegreen", 0XFF98FB98);
    colorMap.put("paleturquoise", 0XFFAFEEEE);
    colorMap.put("palevioletred", 0XFFDB7093);
    colorMap.put("papayawhip", 0XFFFFEFD5);
    colorMap.put("peachpuff", 0XFFFFDAB9);
    colorMap.put("peru", 0XFFCD853F);
    colorMap.put("pink", 0XFFFFC0CB);
    colorMap.put("plum", 0XFFDDA0DD);
    colorMap.put("powderblue", 0XFFB0E0E6);
    colorMap.put("purple", 0XFF800080);
    colorMap.put("rebeccapurple", 0XFF663399);
    colorMap.put("red", 0XFFFF0000);
    colorMap.put("rosybrown", 0XFFBC8F8F);
    colorMap.put("royalblue", 0XFF4169E1);
    colorMap.put("saddlebrown", 0XFF8B4513);
    colorMap.put("salmon", 0XFFFA8072);
    colorMap.put("sandybrown", 0XFFF4A460);
    colorMap.put("seagreen", 0XFF2E8B57);
    colorMap.put("seashell", 0XFFFFF5EE);
    colorMap.put("sienna", 0XFFA0522D);
    colorMap.put("silver", 0XFFC0C0C0);
    colorMap.put("skyblue", 0XFF87CEEB);
    colorMap.put("slateblue", 0XFF6A5ACD);
    colorMap.put("slategray", 0XFF708090);
    colorMap.put("slategrey", 0XFF708090);
    colorMap.put("snow", 0XFFFFFAFA);
    colorMap.put("springgreen", 0XFF00FF7F);
    colorMap.put("steelblue", 0XFF4682B4);
    colorMap.put("tan", 0XFFD2B48C);
    colorMap.put("teal", 0XFF008080);
    colorMap.put("thistle", 0XFFD8BFD8);
    colorMap.put("tomato", 0XFFFF6347);
    colorMap.put("turquoise", 0XFF40E0D0);
    colorMap.put("violet", 0XFFEE82EE);
    colorMap.put("wheat", 0XFFF5DEB3);
    colorMap.put("white", 0XFFFFFFFF);
    colorMap.put("whitesmoke", 0XFFF5F5F5);
    colorMap.put("yellow", 0XFFFFFF00);
    colorMap.put("yellowgreen", 0XFF9ACD32);
    colorMap.put("transparent", 0x00000000);
  }

  public static int getColor(String color) {
    return getColor(color, Integer.MIN_VALUE);
  }

  public static int getColor(String color, int defaultColor) {
    if (TextUtils.isEmpty(color)) {
      return defaultColor;
    }
    color = color.trim(); //remove non visible codes

    Integer cache = WXUtils.sCache.get(color);
    if(cache!=null){
      return cache;
    }
    else {
      int resultColor = defaultColor;
      Pair<Boolean, Integer> result;
      ColorConvertHandler[] handlers = ColorConvertHandler.values();
      for (ColorConvertHandler handler : handlers) {
        try {
          result = handler.handle(color);
          if (result.first) {
            resultColor = result.second;
            WXUtils.sCache.put(color, resultColor);
            break;
          }
        } catch (RuntimeException e) {
          WXLogUtils.v("Color_Parser", WXLogUtils.getStackTrace(e));
        }
      }
      return resultColor;
    }
  }

  /**
   * Assembly gradients
   * @param image gradient values contains direction、colors
   * @param width component width
   * @param height component height
   * @return gradient shader
   */
  public static Shader getShader(String image, float width, float height) {
    List<String> valueList = parseGradientValues(image);
    if (valueList != null && valueList.size() == 3) {
      float[] points = parseGradientDirection(valueList.get(0), width, height);
      Shader shader = new LinearGradient(points[0], points[1],
              points[2], points[3],
              getColor(valueList.get(1), Color.WHITE), getColor(valueList.get(2), Color.WHITE),
              Shader.TileMode.CLAMP);
      return shader;
    }
    return null;
  }

  /**
   * parse gradient values contains direction、colors
   * @param image gradient values
   * @return split values by comma
   */
  @NonNull
  private static List<String> parseGradientValues(String image) {
    if (TextUtils.isEmpty(image)) {
      return null;
    }
    image.trim();
    if(image.startsWith("linear-gradient")){
      String valueStr = image.substring(image.indexOf("(") + 1, image.lastIndexOf(")"));
      StringTokenizer tokenizer = new StringTokenizer(valueStr, ",");
      List<String> values = new ArrayList<>();
      String temp = null;
      while (tokenizer.hasMoreTokens()) {
        String token = tokenizer.nextToken();
        if (token.contains("(")) {
          temp = token + ",";
          continue;
        }
        if (token.contains(")")) {
          temp += token;
          values.add(temp);
          temp = null;
          continue;
        }
        if (temp != null) {
          temp += (token + ",");
          continue;
        }
        values.add(token);
      }
      return values;
    }
    return null;
  }

  /**
   * parse gradient direction
   * @param direction gradient direction
   * @param width component width
   * @param height component height
   * @return gradient points
   */
  private static float[] parseGradientDirection(String direction, float width, float height) {
    int x1 = 0, y1 = 1, x2 = 2, y2 = 3;
    float[] points = {0, 0, 0, 0};

    if (!TextUtils.isEmpty(direction)) {
      direction = direction.replaceAll("\\s*", "").toLowerCase();
    }

    switch (direction) {
      //to right
      case "toright":
        points[x2] = width;
        break;
      //to left
      case "toleft":
        points[x1] = width;
        break;
      //to bottom
      case "tobottom":
        points[y2] = height;
        break;
      //to top
      case "totop":
        points[y1] = height;
        break;
      //to bottom right
      case "tobottomright":
        points[x2] = width;
        points[y2] = height;
        break;
      //to top left
      case "totopleft":
        points[x1] = width;
        points[y1] = height;
        break;
    }
    return points;
  }

  public static boolean isNamedColor(String name) {
    return colorMap.containsKey(name);
  }

  enum ColorConvertHandler {
    NAMED_COLOR_HANDLER {
      @Override
      @NonNull Pair<Boolean, Integer> handle(String rawColor) {
        if (colorMap.containsKey(rawColor)) {
          return new Pair<>(Boolean.TRUE, colorMap.get(rawColor));
        } else {
          return new Pair<>(Boolean.FALSE, Color.TRANSPARENT);
        }
      }
    },
    RGB_HANDLER {
      @Override
      @NonNull Pair<Boolean, Integer> handle(String rawColor) {
        if (rawColor.length() == 4) {
          //#eee, #333
          int r, g, b;
          r = Integer.parseInt(rawColor.substring(1, 2), HEX);
          g = Integer.parseInt(rawColor.substring(2, 3), HEX);
          b = Integer.parseInt(rawColor.substring(3, 4), HEX);
          return new Pair<>(Boolean.TRUE, Color.rgb(r + (r << 4), g + (g << 4), b + (b << 4)));
        } else if (rawColor.length() == 7 || rawColor.length() == 9) {
          //#eeeeee, #333333
          return new Pair<>(Boolean.TRUE, Color.parseColor(rawColor));
        } else {
          return new Pair<>(Boolean.FALSE, Color.TRANSPARENT);
        }
      }
    },
    FUNCTIONAL_RGB_HANDLER {
      @Override
      @NonNull Pair<Boolean, Integer> handle(String rawColor) {
        SingleFunctionParser<Integer> functionParser = new SingleFunctionParser<>(rawColor, FUNCTIONAL_RGB_MAPPER);
        List<Integer> rgb = functionParser.parse(RGB);
        if (null != rgb && rgb.size() == RGB_SIZE) {
          return new Pair<>(Boolean.TRUE, Color.rgb(rgb.get(0), rgb.get(1), rgb.get(2)));
        } else {
          return new Pair<>(Boolean.FALSE, Color.TRANSPARENT);
        }
      }
    },

    FUNCTIONAL_RGBA_HANDLER {
      @Override
      @NonNull Pair<Boolean, Integer> handle(String rawColor) {
        SingleFunctionParser<Number> functionParser = new SingleFunctionParser<>(rawColor, FUNCTIONAL_RGBA_MAPPER);
        List<Number> rgba = functionParser.parse(RGBA);
        if (rgba.size() == RGBA_SIZE) {
          return new Pair<>(Boolean.TRUE, Color.argb(
                  parseAlpha(rgba.get(3).floatValue()),
                  rgba.get(0).intValue(),
                  rgba.get(1).intValue(),
                  rgba.get(2).intValue()));
        } else {
          return new Pair<>(Boolean.FALSE, Color.TRANSPARENT);
        }
      }
    };

    /**
     * Parse color to #RRGGBB or #AARRGGBB. The parsing algorithm depends on sub-class.
     *
     * @param rawColor color, maybe functional RGB(RGBA), #RGB, keywords color or transparent
     * @return #RRGGBB or #AARRGGBB
     */
    @NonNull abstract Pair<Boolean, Integer> handle(String rawColor);

    /**
     * Parse alpha gradient of color from range 0-1 to range 0-255
     *
     * @param alpha the alpha value, in range 0-1
     * @return the alpha value, in range 0-255
     */
    private static int parseAlpha(float alpha) {
      return (int) (alpha * COLOR_RANGE);
    }
  }
}
