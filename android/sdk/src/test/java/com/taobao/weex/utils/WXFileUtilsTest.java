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

import com.taobao.weappplus_sdk.BuildConfig;
import java.io.File;

import org.junit.Assert;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PowerMockIgnore;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.rule.PowerMockRule;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.RuntimeEnvironment;
import org.robolectric.annotation.Config;

/**
 * Created by sospartan on 8/2/16.
 */
@RunWith(RobolectricTestRunner.class)
@Config(constants = BuildConfig.class, sdk = 19)
@PowerMockIgnore({ "org.mockito.*", "org.robolectric.*", "android.*" })
@PrepareForTest()
public class WXFileUtilsTest {

  @Rule
  public PowerMockRule rule = new PowerMockRule();

  @Test
  public void testLoadFileContent() throws Exception {
    File folder = new File("build/intermediates/bundles/debug/assets/");
    if(!folder.exists()){
      folder = new File("build/intermediates/bundles/debug/assets/");
      folder.mkdirs();
    }
    File file = new File(folder,"test");
    System.out.println(file.getAbsolutePath());
    if(!file.exists()){
      file.createNewFile();
    }

    WXFileUtils.loadAsset("test", RuntimeEnvironment.application);
  }

  @Test
  public void testSaveFile() throws Exception {
    WXFileUtils.saveFile("build/test","test".getBytes(),RuntimeEnvironment.application);
  }


  @Test
  public void  testMd5(){
    Assert.assertEquals("77963b7a931377ad4ab5ad6a9cd718aa", WXFileUtils.md5("ddd"));
    Assert.assertEquals("d5Y7epMTd61Kta1qnNcYqg==", WXFileUtils.base64Md5("ddd"));
  }
}
