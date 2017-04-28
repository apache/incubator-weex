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

package com.alibaba.weex;

import android.os.Bundle;
import android.support.test.espresso.idling.CountingIdlingResource;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.widget.LinearLayout;

import com.alibaba.weex.commons.adapter.ImageAdapter;
import com.taobao.weex.IWXRenderListener;
import com.taobao.weex.InitConfig;
import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKEngine;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.common.WXRenderStrategy;
import com.taobao.weex.utils.WXFileUtils;

import java.util.HashMap;
import java.util.Map;

public class BenchmarkActivity extends AppCompatActivity implements IWXRenderListener {

  public final static String ROOT = "root";
  private final static String TAG = "WEEX";
  private final static String URL =
      "http://h5.waptest.taobao.com/app/weextc031/build/TC_Monitor_List_WithAppendTree.js";
  public static CountingIdlingResource countingIdlingResource;
  private WXSDKInstance mInstance;
  private LinearLayout root;
  private long startTime;
  private long endTime;
  private long duration;
  private boolean perfStart;
  private boolean perfEnd;
  private boolean isWeex;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    root = new LinearLayout(this);
    root.setLayoutParams(new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT,
                                                    ViewGroup.LayoutParams.MATCH_PARENT));
    root.setOrientation(LinearLayout.VERTICAL);
    root.setContentDescription(ROOT);
    setContentView(root);
    root.getViewTreeObserver().addOnGlobalLayoutListener(new ViewTreeObserver.OnGlobalLayoutListener() {
      @Override
      public void onGlobalLayout() {
        endTime = System.currentTimeMillis();
        Log.v(TAG, "End: " + endTime);
        if (perfStart && (perfEnd||!isWeex)) {
          perfStart = false;
          perfEnd = false;
          duration = (endTime - startTime);
          Log.v(TAG, "OnGlobalLayoutListener: " + getDuration());
        }
      }
    });
    WXEnvironment.isPerf = true;
    WXSDKEngine.addCustomOptions("appName", "WXSample");
    WXSDKEngine.addCustomOptions("appGroup", "WXApp");
    WXSDKEngine.initialize(getApplication(),
                           new InitConfig.Builder().setImgAdapter(new ImageAdapter()).build());
  }

  @Override
  public void onStart() {
    super.onStart();
    if (mInstance != null) {
      mInstance.onActivityStart();
    }
  }

  @Override
  public void onResume() {
    super.onResume();
    if (mInstance != null) {
      mInstance.onActivityResume();
    }
  }

  @Override
  public void onPause() {
    super.onPause();
    if (mInstance != null) {
      mInstance.onActivityPause();
    }
  }

  @Override
  public void onStop() {
    super.onStop();
    if (mInstance != null) {
      mInstance.onActivityStop();
    }
  }

  @Override
  public void onDestroy() {
    super.onDestroy();
    if (mInstance != null) {
      mInstance.onActivityDestroy();
    }
  }

  @Override
  public void onViewCreated(WXSDKInstance instance, View view) {
    if (root != null) {
      root.removeAllViews();
      root.addView(view);
    }
  }

  @Override
  public void onRenderSuccess(WXSDKInstance instance, int width, int height) {
    if (countingIdlingResource != null) {
      countingIdlingResource.decrement();
    }
    perfEnd = true;
  }

  @Override
  public void onRefreshSuccess(WXSDKInstance instance, int width, int height) {

  }

  @Override
  public void onException(WXSDKInstance instance, String errCode, String msg) {

  }

  public WXSDKInstance getWXInstance() {
    return mInstance;
  }

  public void loadWeexPage(final boolean weex) {
    isWeex=weex;
    runOnUiThread(new Runnable() {
      @Override
      public void run() {
        if (weex) {
          if (mInstance != null) {
            mInstance.destroy();
          }
          mInstance = new WXSDKInstance(BenchmarkActivity.this);
          Map<String, Object> options = new HashMap<>();
          options.put(WXSDKInstance.BUNDLE_URL, "file://assets/hello_weex.js");
          mInstance.registerRenderListener(BenchmarkActivity.this);
          if (countingIdlingResource != null) {
            countingIdlingResource.increment();
          }
          perfStart = true;
          Log.v(TAG, "Start: " + startTime);
          startTime = System.currentTimeMillis();
          mInstance.render(TAG,
                           WXFileUtils.loadAsset("hello.js", BenchmarkActivity.this),
                           options,
                           null,
                           root.getWidth(),
                           root.getHeight(),
                           WXRenderStrategy.APPEND_ASYNC);
        } else {
          root.removeAllViews();
          perfStart = true;
          startTime = System.currentTimeMillis();
          View.inflate(BenchmarkActivity.this, R.layout.hello_weex, root);
        }
      }
    });
  }

  public void loadWeexPage() {
    loadWeexPage(URL);
  }

  public void loadWeexPage(final String url) {
    runOnUiThread(new Runnable() {
      @Override
      public void run() {
        if (mInstance != null) {
          mInstance.destroy();
        }
        mInstance = new WXSDKInstance(BenchmarkActivity.this);
        Map<String, Object> options = new HashMap<>();
        options.put(WXSDKInstance.BUNDLE_URL, url);
        mInstance.registerRenderListener(BenchmarkActivity.this);
        if (countingIdlingResource != null) {
          countingIdlingResource.increment();
        }
        mInstance.renderByUrl(
            TAG,
            url,
            options,
            null,
            root.getWidth(),
            root.getHeight(),
            WXRenderStrategy.APPEND_ASYNC);
      }
    });
  }

  public long getDuration() {
    return duration;
  }
}
