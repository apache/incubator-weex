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

import android.text.TextUtils;

import com.taobao.weappplus_sdk.BuildConfig;
import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.common.WXConfig;

import junit.framework.TestCase;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.invocation.InvocationOnMock;
import org.mockito.stubbing.Answer;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PowerMockIgnore;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.robolectric.annotation.Config;

import java.util.HashMap;
import java.util.Map;

import static org.mockito.Matchers.any;

/**
 * Created by lixinke on 16/2/24.
 */
@RunWith(PowerMockRunner.class)
@Config(constants = BuildConfig.class, sdk = 19)
@PowerMockIgnore( {"org.mockito.*", "org.robolectric.*", "android.*"})
@PrepareForTest( {WXEnvironment.class, WXViewUtils.class, WXSDKInstance.class, TextUtils.class})
public class WXUtilsTest extends TestCase {

    public static final float TEST_DENSITY = 3.0f;
    public static final int TEST_SCREEN_WIDTH = 1024;
    public static final int TEST_VIEW_PORT = 800;

    @Before
    public void setUp() throws Exception {
        super.setUp();

        Map<String, String> map = new HashMap<>();
        map.put(WXConfig.scale, Float.toString(TEST_DENSITY));
        PowerMockito.mockStatic(WXEnvironment.class);
        PowerMockito.when(WXEnvironment.class, "getConfig").thenReturn(map);

        PowerMockito.mockStatic(WXViewUtils.class);
        PowerMockito.when(WXViewUtils.class, "getScreenWidth").thenReturn(TEST_SCREEN_WIDTH);

        PowerMockito.mockStatic(WXSDKInstance.class);
        PowerMockito.when(WXSDKInstance.class, "getViewPortWidth").thenReturn(TEST_VIEW_PORT);

        PowerMockito.mockStatic(TextUtils.class);
        PowerMockito.when(TextUtils.isEmpty(any(CharSequence.class))).thenAnswer(new Answer<Boolean>() {
            @Override
            public Boolean answer(InvocationOnMock invocation) throws Throwable {
                CharSequence a = (CharSequence) invocation.getArguments()[0];
                return !(a != null && a.length() > 0);
            }
        });
        // also look at @PrepareForTest if add mock of new class
    }

    @Test
    public void testGetFloat() throws Exception {
        float test_float = WXUtils.getFloat("12324.9px");
        assertEquals(12324.9, test_float, 0.01);

        assertEquals(WXUtils.fastGetFloat("1.2345",2), 1.23f);
    }

    @Test
    public void testGetInt() throws Exception {
        int test_int = WXUtils.getInt("23px");
        assertEquals(23, test_int);
    }

    @Test
    public void testGetLong() throws Exception {
        long test_long = WXUtils.getLong("8098px");
        assertEquals(8098, test_long);
    }

    @Test
    public void testGetDouble() throws Exception {
        double test_Double = WXUtils.getDouble("8098.8989px");
        assertEquals(8098.8, test_Double, 0.1);
    }

    @Test
    public void testGetFloatWX() throws Exception {
        Float test_float = WXUtils.getFloatByViewport("100wx", TEST_VIEW_PORT);
        Float want = 100 * TEST_DENSITY * TEST_VIEW_PORT / TEST_SCREEN_WIDTH;
        assertEquals(test_float, want , 0.01);

        test_float = WXUtils.getFloatByViewport("100px", TEST_VIEW_PORT);
        want = 100F;
        assertEquals(test_float, want);

        test_float = WXUtils.getFloatByViewport("100.2", TEST_VIEW_PORT);
        want = 100.2F;
        assertEquals(test_float, want);

        test_float = WXUtils.getFloatByViewport(100.2F, TEST_VIEW_PORT);
        want = 100.2F;
        assertEquals(test_float, want, 0.0001);

        test_float = WXUtils.getFloatByViewport(100.2D, TEST_VIEW_PORT);
        want = 100.2F;
        assertEquals(test_float, want, 0.0001);

        test_float = WXUtils.getFloatByViewport("NaN", TEST_VIEW_PORT);
        want = Float.NaN;
        assertEquals(test_float, want);
    }

    @Test
    public void testGetIntWX() throws Exception {
        Integer test_int = WXUtils.getInt("100wx");
        Integer want = (int)(100 * TEST_DENSITY * 750 / TEST_SCREEN_WIDTH);
        assertEquals(test_int, want);

        test_int = WXUtils.getInt("100px");
        want = 100;
        assertEquals(test_int, want);

        test_int = WXUtils.getInt("100");
        want = 100;
        assertEquals(test_int, want);

        test_int = WXUtils.getInt(100);
        want = 100;
        assertEquals(test_int, want);

        test_int = WXUtils.getInt(100.1);
        want = 0;
        assertEquals(test_int, want); // double can not cast to integer
    }

    @Test
    public void testGetDoubleWX() throws Exception {
        Double test_double = WXUtils.getDouble("100.32wx");
        Double want = (100.32D * TEST_DENSITY * 750 / TEST_SCREEN_WIDTH);
        assertEquals(test_double, want, 0.01);

        test_double = WXUtils.getDouble("100px");
        want = 100D;
        assertEquals(test_double, want, 0.01);

        test_double = WXUtils.getDouble("100");
        want = 100D;
        assertEquals(test_double, want, 0.01);

        test_double = WXUtils.getDouble(100);
        want = 100D;
        assertEquals(test_double, want, 0.01);

        test_double = WXUtils.getDouble(100.1);
        want = 100.1D;
        assertEquals(test_double, want, 0.01);
    }

}
