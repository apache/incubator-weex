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
package com.taobao.weex.ui.component.helper;

import com.taobao.weappplus_sdk.BuildConfig;
import com.taobao.weex.appfram.pickers.DatePickerImpl;

import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PowerMockIgnore;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.rule.PowerMockRule;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;

import java.lang.reflect.Method;

/**
 * Created by moxun on 16/10/24.
 */

@RunWith(RobolectricTestRunner.class)
@Config(constants = BuildConfig.class, sdk = 19)
@PowerMockIgnore({"org.mockito.*", "org.robolectric.*", "android.*", "org.json.*"})
@PrepareForTest()
public class WXTimeInputHelperTest {
    @Rule
    public PowerMockRule rule = new PowerMockRule();

    @Test
    public void testParseDate() throws Exception{
        Method parseDate = DatePickerImpl.class.getDeclaredMethod("parseDate", String.class);
        parseDate.setAccessible(true);
        parseDate.invoke(null, "");
        parseDate.invoke(null, "test");
        parseDate.invoke(null, "2016-12-11");
        parseDate.invoke(null, "2016-1-1");
        parseDate.invoke(null, "9999-99-99");
    }

    @Test
    public void testParseTime() throws Exception{
        Method parseTime = DatePickerImpl.class.getDeclaredMethod("parseTime", String.class);
        parseTime.setAccessible(true);
        parseTime.invoke(null, "");
        parseTime.invoke(null, "test");
        parseTime.invoke(null, "11:11");
        parseTime.invoke(null, "1:1");
        parseTime.invoke(null, "25:61");
    }
}
