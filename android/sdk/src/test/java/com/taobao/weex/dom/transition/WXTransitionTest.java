/**
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
package com.taobao.weex.dom.transition;

import android.app.Application;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONObject;
import com.taobao.weappplus_sdk.BuildConfig;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.utils.WXViewUtils;

import junit.framework.Assert;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PowerMockIgnore;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;

import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

/**
 * Created by furture on 2017/10/18.
 */
@RunWith(RobolectricTestRunner.class)
@Config(constants = BuildConfig.class, sdk = 19)
@PowerMockIgnore( {"org.mockito.*", "org.robolectric.*", "android.*"})
public class WXTransitionTest extends Application{



    @Test
    public void  testFrom(){
        WXViewUtils.setScreenWidth(750);
        WXDomObject domObject = new WXDomObject();
        domObject.setViewPortWidth(750);
        JSONObject map = JSON.parseObject("{\n" +
                "    \"style\": {\n" +
                "        \"width\": 600, \n" +
                "        \"marginLeft\": 75, \n" +
                "        \"marginTop\": 35, \n" +
                "        \"marginBottom\": 35, \n" +
                "        \"flexDirection\": \"column\", \n" +
                "        \"justifyContent\": \"center\", \n" +
                "        \"borderWidth\": 2, \n" +
                "        \"borderStyle\": \"solid\", \n" +
                "        \"borderColor\": \"rgb(0,180,255)\", \n" +
                "        \"backgroundColor\": \"rgba(0,180,255,0.2)\", \n" +
                "        \"transitionProperty\": \"height\", \n" +
                "        \"transitionDuration\": 300, \n" +
                "        \"transitionDelay\": 50, \n" +
                "        \"transitionTimingFunction\": \"ease-in-out\"\n" +
                "    }\n" +
                "}");
        domObject.parseFromJson(map);
        Assert.assertNotNull("transition success",domObject.getTransition());
        WXTransition transition = domObject.getTransition();
        Map<String, Object> updates = new HashMap();
        updates.put("height", "1000");
        Assert.assertTrue(transition.hasTransitionProperty(updates));
        transition.startTransition(updates);



    }

    @Test
    public void testSplit(){

        Assert.assertTrue(Arrays.equals(new String[]{"height", "width"}, WXTransition.PROPERTY_SPLIT_PATTERN.split("height|width")));
        Assert.assertTrue(Arrays.equals(new String[]{"height", "width"}, WXTransition.PROPERTY_SPLIT_PATTERN.split("height,width")));


        System.out.println(Arrays.toString(WXTransition.PROPERTY_SPLIT_PATTERN.split("height|width")));
        System.out.println(Arrays.toString(WXTransition.PROPERTY_SPLIT_PATTERN.split("height,width")));
    }
}
