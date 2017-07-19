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
package com.taobao.weex.ui.component;

import static junit.framework.Assert.assertEquals;
import static junit.framework.Assert.assertNotNull;
import static junit.framework.Assert.assertNull;

import android.graphics.Bitmap;
import android.graphics.drawable.Drawable;
import android.view.ViewGroup;
import android.widget.ImageView;
import com.taobao.weappplus_sdk.BuildConfig;
import com.taobao.weex.TestActivity;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKInstanceTest;
import com.taobao.weex.common.Constants;
import com.taobao.weex.common.WXImageSharpen;
import com.taobao.weex.dom.TestDomObject;
import com.taobao.weex.dom.WXAttr;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.ui.view.WXImageView;
import com.taobao.weex.ui.view.border.BorderDrawable;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PowerMockIgnore;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.rule.PowerMockRule;
import org.robolectric.Robolectric;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;

@RunWith(RobolectricTestRunner.class)
@Config(constants = BuildConfig.class, sdk = 19)
@PowerMockIgnore({"org.mockito.*", "org.robolectric.*", "android.*"})
public class WXImageTest {

  @Rule
  public PowerMockRule rule = new PowerMockRule();

  WXImage mWXImage;
  WXSDKInstance mInstance;
  WXDiv mWXDiv;
  WXDomObject mDomObject;

  @Before
  public void setUp() throws Exception {

    mInstance = WXSDKInstanceTest.createInstance();
    mDomObject = new TestDomObject();
    PowerMockito.when(Mockito.spy(mDomObject).clone()).thenReturn(mDomObject);
    mWXDiv = PowerMockito.mock(WXDiv.class);
    mWXImage = new WXImage(mInstance, mDomObject, mWXDiv);

  }


  @Test
  @PrepareForTest(WXImageView.class)
  public void testInitComponentHostView() throws Exception {

    ImageView imageView = mWXImage.initComponentHostView(Robolectric.setupActivity(TestActivity.class));
    assertEquals(imageView.getClass(), WXImageView.class);

  }

  @Test
  @PrepareForTest(WXImageView.class)
  public void testSetBackgroundColor() throws Exception {

    ImageView imageView = mWXImage.initComponentHostView(Robolectric.setupActivity(TestActivity.class));
    mWXImage.mHost = imageView;

    mWXImage.setBackgroundColor("#FFFFFF");

    Drawable drawable = mWXImage.getHostView().getBackground();
    assertEquals(drawable instanceof BorderDrawable, true);
  }


  @Test
  public void testSetProperty() throws Exception {

    ImageView imageView = mWXImage.initComponentHostView(Robolectric.setupActivity(TestActivity.class));
    mWXImage.mHost = imageView;

    mWXImage.setProperty(Constants.Name.RESIZE_MODE, "cover");
    ImageView.ScaleType scaleType = mWXImage.getHostView().getScaleType();
    assertEquals(scaleType, ImageView.ScaleType.CENTER_CROP);


  }


  @Test
  public void testSetResizeMode() throws Exception {

    ImageView imageView = mWXImage.initComponentHostView(Robolectric.setupActivity(TestActivity.class));
    mWXImage.mHost = imageView;

    mWXImage.setResizeMode("cover");
    ImageView.ScaleType scaleType = mWXImage.getHostView().getScaleType();
    assertEquals(scaleType, ImageView.ScaleType.CENTER_CROP);

  }

  @Test
  public void testSetResize() throws Exception {
    ImageView imageView = mWXImage.initComponentHostView(Robolectric.setupActivity(TestActivity.class));
    mWXImage.mHost = imageView;

    mWXImage.setResize("cover");
    ImageView.ScaleType scaleType = mWXImage.getHostView().getScaleType();
    assertEquals(scaleType, ImageView.ScaleType.CENTER_CROP);
  }

  @Test
  public void testSetSrc() throws Exception {
    TestDomObject.setAttribute((WXDomObject)mWXImage.getDomObject(),PowerMockito.mock(WXAttr.class));
    PowerMockito.when(mWXImage.getDomObject().getAttrs().getImageSharpen()).thenReturn(WXImageSharpen.SHARPEN);
    mWXImage.setSrc("");

  }

  @Test
  public void testSetImageBitmap(){
    ImageView imageView = mWXImage.initComponentHostView(Robolectric.setupActivity(TestActivity.class));
    imageView.setLayoutParams(new ViewGroup.LayoutParams(
        ViewGroup.LayoutParams.WRAP_CONTENT,
        ViewGroup.LayoutParams.WRAP_CONTENT));
    imageView.setImageBitmap(null);
    assertNull(imageView.getDrawable());

    imageView.setImageBitmap(Bitmap.createBitmap(100, 100, Bitmap.Config.RGB_565));
    assertNotNull(imageView.getDrawable());
  }
}
