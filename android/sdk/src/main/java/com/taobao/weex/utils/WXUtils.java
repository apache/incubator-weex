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

import android.app.ActivityManager;
import android.content.Context;
import android.content.res.Configuration;
import android.os.Looper;
import android.os.SystemClock;
import android.support.annotation.Nullable;
import android.support.v4.util.LruCache;
import android.text.TextUtils;
import com.taobao.weex.WXEnvironment;
import com.taobao.weex.common.Constants;
import com.taobao.weex.common.WXConfig;

public class WXUtils {

  final static LruCache<String, Integer> sCache = new LruCache<>(64);
  public static final char PERCENT = '%';
  private static final int HUNDRED =100;
  /**
   * Tell whether current thread is UI(main) thread.
   * @return true for UI(main) thread
   */
  public static boolean isUiThread() {
    return Thread.currentThread().getId() == Looper.getMainLooper().getThread().getId();
  }

  public static boolean isUndefined(float value) {
    return Float.isNaN(value);
  }


  public static float getFloatByViewport(Object value, int viewport) {
    if (value == null) {
      return Float.NaN;
    }
    String temp = value.toString().trim();
    if (Constants.Name.AUTO.equals(temp)
            || Constants.Name.UNDEFINED.equals(temp)
            || TextUtils.isEmpty(temp)) {
      WXLogUtils.e("Argument Warning ! value is " + temp + "And default Value:" + Float.NaN);
      return Float.NaN;
    }

    if (temp.endsWith("wx")) {
      try {
        return transferWx(temp, viewport);
      } catch (NumberFormatException e) {
        WXLogUtils.e("Argument format error! value is " + value, e);
      } catch (Exception e) {
        WXLogUtils.e("Argument error! value is " + value, e);
      }
    } else if (temp.endsWith("px")) {
      try {
        temp = temp.substring(0, temp.indexOf("px"));
        return Float.parseFloat(temp);
      } catch (NumberFormatException nfe) {
        WXLogUtils.e("Argument format error! value is " + value, nfe);
      } catch (Exception e) {
        WXLogUtils.e("Argument error! value is " + value, e);
      }
    } else {
      try {
        return Float.parseFloat(temp);
      } catch (NumberFormatException nfe) {
        WXLogUtils.e("Argument format error! value is " + value, nfe);
      } catch (Exception e) {
        WXLogUtils.e("Argument error! value is " + value, e);
      }
    }
    return Float.NaN;
  }

  public static float getFloat(Object value) {
    return getFloat(value, Float.NaN);
  }

  public static Float getFloat(Object value, @Nullable Float df) {
    if (value == null) {
      return df;
    }

    String temp = value.toString().trim();
    if(Constants.Name.AUTO.equals(temp)
            || Constants.Name.UNDEFINED.equals(temp)
            || TextUtils.isEmpty(temp)){
      WXLogUtils.e("Argument Warning ! value is " + temp + "And default Value:"+Float.NaN);
      return df;
    }
    if (temp.endsWith("wx")) {
      try {
        return transferWx(temp, 750);
      } catch (NumberFormatException e) {
        WXLogUtils.e("Argument format error! value is " + value, e);
      } catch (Exception e) {
        WXLogUtils.e("Argument error! value is " + value, e);
      }
    }else if (temp.endsWith("px")) {
      try {
        temp = temp.substring(0, temp.indexOf("px"));
        return Float.parseFloat(temp);
      } catch (NumberFormatException nfe) {
        WXLogUtils.e("Argument format error! value is " + value, nfe);
      } catch (Exception e) {
        WXLogUtils.e("Argument error! value is " + value, e);
      }
    }else {
      try {
        return Float.parseFloat(temp);
      } catch (NumberFormatException nfe) {
        WXLogUtils.e("Argument format error! value is " + value, nfe);
      } catch (Exception e) {
        WXLogUtils.e("Argument error! value is " + value, e);
      }
    }
    return df;
  }

  private static float transferWx(String stringWithWXPostfix, int viewport) {
    if(null == stringWithWXPostfix) {
      return 0;
    }
    String temp = stringWithWXPostfix;
    if(stringWithWXPostfix.endsWith("wx")) {
      temp = stringWithWXPostfix.substring(0, stringWithWXPostfix.indexOf("wx"));
    }
    Float f = Float.parseFloat(temp);
    float density = WXEnvironment.sApplication.getResources().getDisplayMetrics().density;
    return density * f * viewport / WXViewUtils.getScreenWidth();
  }

  public static float fastGetFloat(String raw, int precision){
    if(!TextUtils.isEmpty(raw)){
      boolean positive=true;
      int loc=0;
      if(raw.charAt(0)=='-'){
        positive=false;
        loc++;
      }
      else if(raw.charAt(0)=='+'){
        loc++;
      }

      char digit;
      float result=0;
      while (loc<raw.length() && (digit=raw.charAt(loc))>='0'&&digit<='9'){
        result=result*10+digit-'0';
        loc++;
      }

      if(loc<raw.length()){
        if(raw.charAt(loc)=='.'){
          loc++;
          int remainderLength=10;
          int counter=0;
          while(loc<raw.length() &&
                  counter<precision &&
                  ((digit=raw.charAt(loc))>='0'&& digit<='9')){
            result+=(digit-'0')/(float)remainderLength;
            remainderLength*=10;
            loc++;
            counter++;
          }
        }
        else{
          // throw new NumberFormatException("Illegal separator");
        }
      }

      if(!positive)
        result*=-1;
      return result;
    }
    return 0;
    // throw new NumberFormatException("NullNumber");
  }

  /**
   * Parse string representation of float. This method intend to be faster than
   * {@link Float#parseFloat(String)}, but less accuracy.
   * @param raw
   * @return
   */
  public static float fastGetFloat(String raw){
    return fastGetFloat(raw, Integer.MAX_VALUE);
  }

  public static int parseInt(String value) {
    try {
      if (!TextUtils.isEmpty(value) && !value.contains(".")) {
        return Integer.parseInt(value);
      }
    } catch (NumberFormatException e) {
      if (WXEnvironment.isApkDebugable()) {
        WXLogUtils.e(WXLogUtils.getStackTrace(e));
      }
    }
    return 0;
  }

  public static int parseInt(Object value) {
    return parseInt(String.valueOf(value));
  }

  public static float parseFloat(Object value) {
    return parseFloat(String.valueOf(value));
  }

  public static float parseFloat(String value) {
    try {
      if (!TextUtils.isEmpty(value) && !TextUtils.equals(value, "null")) {
        return Float.parseFloat(value);
      } else {
        if (WXEnvironment.isApkDebugable()) {
          WXLogUtils.e("WXUtils parseFloat illegal value is " + value);
        }
      }
    } catch (NumberFormatException e) {
      if (WXEnvironment.isApkDebugable()) {
        WXLogUtils.e(WXLogUtils.getStackTrace(e));
      }
    }
    return 0;
  }

  public static int getInt(Object value) {
    return getInteger(value, 0);
  }

  public static @Nullable Integer getInteger(@Nullable Object value, @Nullable Integer df) {

    if (value == null) {
      return df;
    }

    String temp = value.toString().trim();
    String key = temp;
    Integer cache = sCache.get(key);
    if (cache != null) {
      return cache;
    } else {
      Integer ret = df;
      String suffix = "";
      if (temp.length() >= 2) {
        suffix = temp.substring(temp.length() - 2, temp.length());
      }

      if (TextUtils.equals("wx", suffix)) {
        if (WXEnvironment.isApkDebugable()) {
          WXLogUtils
              .w("the value of " + value
                  + " use wx unit, which will be not supported soon after.");
        }
        try {
          ret = (int)transferWx(temp, 750);
        } catch (NumberFormatException e) {
          WXLogUtils.e("Argument format error! value is " + value, e);
        } catch (Exception e) {
          WXLogUtils.e("Argument error! value is " + value, e);
        }
      } else if (TextUtils.equals("px", suffix)) {
        try {
          temp = temp.substring(0, temp.length() - 2);
          if (!TextUtils.isEmpty(temp) && temp.contains(".")) {
            ret = (int)WXUtils.parseFloat(temp);
          } else {
            ret = Integer.parseInt(temp);
          }
        } catch (NumberFormatException nfe) {
          WXLogUtils.e("Argument format error! value is " + value, nfe);
        } catch (Exception e) {
          WXLogUtils.e("Argument error! value is " + value, e);
        }
      } else {
        try {
          if (!TextUtils.isEmpty(temp)) {
            if (temp.contains(".")) {
              ret = (int)WXUtils.parseFloat(temp);
            } else {
              ret = Integer.parseInt(temp);
            }
          } else {
            if (WXEnvironment.isApkDebugable()) {
              WXLogUtils.e("Argument value is null, df is" + df);
            }
          }
        } catch (NumberFormatException nfe) {
          WXLogUtils.e("Argument format error! value is " + value, nfe);
        } catch (Exception e) {
          WXLogUtils.e("Argument error! value is " + value, e);
        }
      }
      if (ret != null && !ret.equals(df)) {
        sCache.put(key, ret);
      }
      return ret;
    }
  }

  /**
   * Looks like no longer usage exists, Mark deprecate first.
   * @param value
   * @return
   */
  @Deprecated
  public static long getLong(Object value) {
    if (value == null) {
      return 0;
    }
    long result = 0;
    String temp = value.toString().trim();
    if (temp.endsWith("wx")) {
      if (WXEnvironment.isApkDebugable()) {
        WXLogUtils.w("the value of " + value + " use wx unit, which will be not supported soon after.");
      }
      try {
        return (long)transferWx(temp, 750);
      } catch (NumberFormatException e) {
        WXLogUtils.e("Argument format error! value is " + value, e);
      } catch (Exception e) {
        WXLogUtils.e("Argument error! value is " + value, e);
      }
    }else if (temp.endsWith("px")) {
      try {
        temp = temp.substring(0, temp.indexOf("px"));
        return Long.parseLong(temp);
      } catch (NumberFormatException nfe) {
        WXLogUtils.e("Argument format error! value is " + value, nfe);
      } catch (Exception e) {
        WXLogUtils.e("Argument error! value is " + value, e);
      }
    }else {
      try {
        return Long.parseLong(temp);
      } catch (NumberFormatException nfe) {
        WXLogUtils.e("Argument format error! value is " + value, nfe);
      } catch (Exception e) {
        WXLogUtils.e("Argument error! value is " + value, e);
      }
    }
    return result;
  }

  /**
   * Looks like no longer usage exists, Mark deprecate first.
   * @param value
   * @return
   */
  @Deprecated
  public static double getDouble(Object value) {
    if (value == null) {
      return 0;
    }
    double result = 0;
    String temp = value.toString().trim();
    if (temp.endsWith("wx")) {
      if (WXEnvironment.isApkDebugable()) {
        WXLogUtils.w("the value of " + value + " use wx unit, which will be not supported soon after.");
      }
      try {
        return transferWx(temp, 750);
      } catch (NumberFormatException e) {
        WXLogUtils.e("Argument format error! value is " + value, e);
      } catch (Exception e) {
        WXLogUtils.e("Argument error! value is " + value, e);
      }
    }else if (temp.endsWith("px")) {
      try {
        temp = temp.substring(0, temp.indexOf("px"));
        return Double.parseDouble(temp);
      } catch (NumberFormatException nfe) {
        WXLogUtils.e("Argument format error! value is " + value, nfe);
      } catch (Exception e) {
        WXLogUtils.e("Argument error! value is " + value, e);
      }
    }else {
      try {
        return Double.parseDouble(temp);
      } catch (NumberFormatException nfe) {
        WXLogUtils.e("Argument format error! value is " + value, nfe);
      } catch (Exception e) {
        WXLogUtils.e("Argument error! value is " + value, e);
      }
    }

    return result;
  }

  @Deprecated
  public static boolean isTabletDevice() {
    try{
      return (WXEnvironment.getApplication().getResources().getConfiguration().screenLayout & Configuration.SCREENLAYOUT_SIZE_MASK) >= Configuration.SCREENLAYOUT_SIZE_LARGE;
    }catch (Exception e){
      //
    }
    return false;
  }

  public static Boolean getBoolean(@Nullable Object value, @Nullable Boolean df) {

    if (value == null)
      return df;
    if (TextUtils.equals("false",value.toString())) {
      return false;
    } else if (TextUtils.equals("true",value.toString())) {
      return true;
    }
    return df;
  }

  public static long getAvailMemory(Context context){
    ActivityManager am = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
    ActivityManager.MemoryInfo mi = new ActivityManager.MemoryInfo();
    am.getMemoryInfo(mi);
    //mi.availMem; 当前系统的可用内存
    //return Formatter.formatFileSize(context, mi.availMem);// 将获取的内存大小规格化
    WXLogUtils.w("app AvailMemory ---->>>"+mi.availMem/(1024*1024));
    return mi.availMem/(1024*1024);
  }

  public static String getString(@Nullable Object value,@Nullable String df) {

    if (value == null)
      return df;

    String originValue;
    if (value instanceof String) {
      originValue = (String) value;
    } else {
      originValue = value.toString();
    }

    return originValue;
  }

  public static int parseUnitOrPercent(String raw, int unit) {
    int suffix;
    if ((suffix = raw.lastIndexOf(WXUtils.PERCENT)) != -1) {
      return parsePercent(raw.substring(0, suffix), unit);
    }
    else {
      return WXUtils.parseInt(raw);
    }
  }

  private static int parsePercent(String raw, int unit){
    return (int)(Float.parseFloat(raw) / HUNDRED * unit);
  }

  /**
   * Get a banner data in JSON format from a bundle content.
   * A bundle banner looks like below:
   * \/*!count
   *  * {
   *  *   version: "0.2.1.20170104-release",
   *  *   create: "20170207171112",
   *  *   git: "banner--772c915",
   *  *   digest: "c709b7f91867e371b24f54d6aeea232a"
   *  * }
   *  !*\/
   *
   * @param content a bundle content
   * @return
   */
  public static String getBundleBanner(String content) {
    final String commentBegin = "/*!";
    final String commentEnd = "!*/";
    final String asteriskRegex = "\\*";
    final String replacement = "";

    int offsetCountBegin = content.indexOf(commentBegin);
    if (offsetCountBegin == -1) {
      return null;
    }
    offsetCountBegin += commentBegin.length();
    int offsetCountEnd = indexLineBreak(content, offsetCountBegin);
    if (offsetCountEnd == -1) {
      return null;
    }
    String countStr = content.substring(offsetCountBegin, offsetCountEnd);
    int count = Integer.parseInt(countStr);

    String commentBody = content.substring(offsetCountEnd + 1, offsetCountEnd + 1 + count);
    int offsetBodyEnd = commentBody.lastIndexOf(commentEnd);
    if (offsetBodyEnd == -1) {
      return null;
    }
    commentBody = commentBody.substring(0, offsetBodyEnd);

    StringBuilder commentBodyBuilder = new StringBuilder();
    String[] items = splitLineBreak(commentBody);

    for (String item : items) {
      commentBodyBuilder.append(item.replaceFirst(asteriskRegex, replacement));
    }

    return commentBodyBuilder.toString();
  }

  private static int indexLineBreak(String str, int fromIndex) {
    final String lineBreakIos = "\r";
    final String lineBreakUnix = "\n";
    final String linebreakWin = "\r\n";

    int index = str.indexOf(lineBreakIos, fromIndex);
    if (index == -1) {
      index = str.indexOf(lineBreakUnix, fromIndex);
    }
    if (index == -1) {
      index = str.indexOf(linebreakWin, fromIndex);
    }

    return index;
  }

  private static String[] splitLineBreak(String str) {
    final String lineBreakIos = "\r";
    final String lineBreakUnix = "\n";
    final String linebreakWin = "\r\n";
    String[] items = str.split(lineBreakIos);

    if (items.length == 1) {
      items = str.split(lineBreakUnix);
    }
    if (items.length == 1) {
      items = str.split(linebreakWin);
    }

    return items;
  }


  
  /**
   * get number
   * */
  public static int getNumberInt(Object value, int defaultValue){
    if(value == null){
      return  defaultValue;
    }
    if(value instanceof  Number){
      return  ((Number) value).intValue();
    }
    try{
      String number = value.toString();
      if(number.indexOf('.') >= 0) {
        return (int) Float.parseFloat(value.toString());
      }else{
        return  Integer.parseInt(number);
      }
    }catch (Exception e){return  defaultValue;}
  }

  private static final long sInterval = System.currentTimeMillis() - SystemClock.uptimeMillis();

  public static long getFixUnixTime(){
    return  sInterval + SystemClock.uptimeMillis();
  }

}
