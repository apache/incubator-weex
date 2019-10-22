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
package org.apache.weex.utils;

import android.graphics.Typeface;
import android.net.Uri;
import android.text.TextUtils;
import android.util.Base64;
import org.apache.weex.WXEnvironment;
import org.apache.weex.WXSDKInstance;
import org.apache.weex.adapter.URIAdapter;
import org.apache.weex.common.Constants;
import java.io.File;

public class FontDO {
  private final String mFontFamilyName;
  private String mUrl = "";
  private String mFilePath;
  private int mType = TYPE_NETWORK;
  private Typeface mTypeface;
  private int mState = STATE_INVALID;

  public final static int STATE_INVALID = -1;
  public final static int STATE_INIT = 0;
  public final static int STATE_LOADING = 1;
  public final static int STATE_SUCCESS = 2;
  public final static int STATE_FAILED = 3;

  public final static int TYPE_UNKNOWN = 0;
  public final static int TYPE_NETWORK = 1;
  public final static int TYPE_FILE = 2;
  public final static int TYPE_LOCAL = 3;
  public final static int TYPE_NATIVE = 4;
  public final static int TYPE_BASE64 = 5;


  public FontDO (String fontFamilyName, String src, WXSDKInstance instance) {
    this.mFontFamilyName = fontFamilyName;
    parseSrc(src,instance);
  }

  public FontDO (String fontFamilyName, Typeface typeface) {
    this.mFontFamilyName = fontFamilyName;
    this.mTypeface = typeface;
    this.mType = TYPE_NATIVE;
    this.mState = STATE_SUCCESS;
  }

  public String getFontFamilyName() {
    return mFontFamilyName;
  }

  private void parseSrc(String src, WXSDKInstance instance) {
    src = (src != null )? src.trim() : "";

    if (instance != null) {
      if (instance.getCustomFontNetworkHandler() != null) {
        String localUrl = instance.getCustomFontNetworkHandler().fetchLocal(src);
        if (!TextUtils.isEmpty(localUrl)) {
          src = localUrl;
        }
      }
    }

    if (src.isEmpty()) {
      mState = STATE_INVALID;
      WXLogUtils.e("TypefaceUtil", "font src is empty.");
      return;
    }

    if (src.matches("^url\\((('.*')|(\".*\"))\\)$")) {
      String url = src.substring(5, src.length() - 2);
      Uri uri = Uri.parse(url);
      if( instance != null){
        uri = instance.rewriteUri(uri, URIAdapter.FONT);
      }
      mUrl = uri.toString();
      try {
        String scheme = uri.getScheme();
        if (Constants.Scheme.HTTP.equals(scheme) ||
                Constants.Scheme.HTTPS.equals(scheme)) {
          mType = TYPE_NETWORK;
        } else if (Constants.Scheme.FILE.equals(scheme)) {
          mType = TYPE_FILE;
            /**
             * eg: file://name/A/B.ttf
             * getPath() = "A/B.ttf",but the real absolute path is "/name/A/B.ttf"
             * so use getEncodedSchemeSpecificPart() to replaced = "//name/A/B.ttf"
             */
            mUrl = uri.getEncodedSchemeSpecificPart();
        } else if (Constants.Scheme.LOCAL.equals(scheme)){
          mType = TYPE_LOCAL;
        } else if (Constants.Scheme.DATA.equals(scheme)) {
          long start = System.currentTimeMillis();
          String[] data = mUrl.split(",");
          if (data != null && data.length == 2) {
            String identify = data[0];
            if (!TextUtils.isEmpty(identify) && identify.endsWith("base64")) {
              //Do not check mime type and charset for now
              String base64Data = data[1];
              if (!TextUtils.isEmpty(base64Data)) {
                String md5 = WXFileUtils.md5(base64Data);
                File cacheDir = new File(WXEnvironment.getApplication().getCacheDir(),
                    "font-family");
                if (!cacheDir.exists()) {
                  cacheDir.mkdirs();
                }
                File tmpFile = new File(cacheDir, md5);
                if(!tmpFile.exists()){
                  tmpFile.createNewFile();
                  WXFileUtils.saveFile(tmpFile.getPath(), Base64.decode(base64Data, Base64.DEFAULT), WXEnvironment.getApplication());
                }
                mUrl = tmpFile.getPath();
                mType = TYPE_BASE64;
                WXLogUtils.d("TypefaceUtil", "Parse base64 font cost " + (System.currentTimeMillis() - start) + " ms");
              }
            }
          }
        } else {
          WXLogUtils.e("TypefaceUtil", "Unknown scheme for font url: " + mUrl);
          mType = TYPE_UNKNOWN;
        }
        mState = STATE_INIT;
      } catch (Exception e) {
        mType = STATE_INVALID;
        WXLogUtils.e("TypefaceUtil", "URI.create(mUrl) failed mUrl: " + mUrl+ "\n"+ WXLogUtils.getStackTrace(e));
      }
    } else {
      mUrl = src;
      mState = STATE_INVALID;
    }

    if(WXEnvironment.isApkDebugable()) {
      WXLogUtils.d("TypefaceUtil", "src:" + src + ", mUrl:" + mUrl + ", mType:" + mType);
    }
  }

  public String getUrl() {
    return mUrl;
  }

  public int getType() {
    return mType;
  }

  public Typeface getTypeface() {
    return mTypeface;
  }

  public void setTypeface(Typeface typeface) {
    this.mTypeface = typeface;
  }

  public int getState() {
    return mState;
  }

  public void setState(int state) {
    this.mState = state;
  }

  public String getFilePath() {
    return mFilePath;
  }

  public void setFilePath(String mFilePath) {
    this.mFilePath = mFilePath;
  }
}
