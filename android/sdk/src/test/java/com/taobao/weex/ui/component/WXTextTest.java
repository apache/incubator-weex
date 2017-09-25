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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

import com.taobao.weappplus_sdk.BuildConfig;
import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.common.Constants;
import com.taobao.weex.dom.TestDomObject;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.dom.WXTextDomObject;
import com.taobao.weex.dom.flex.Spacing;
import com.taobao.weex.ui.SimpleComponentHolder;
import com.taobao.weex.ui.flat.FlatGUIContext;
import java.util.HashMap;
import java.util.Map;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.RuntimeEnvironment;
import org.robolectric.annotation.Config;

/**
 * Created by gulin on 16/2/4.
 */
@RunWith(RobolectricTestRunner.class)
@Config(constants = BuildConfig.class)
public class WXTextTest {
    private WXText mWXText;
    private WXDiv mParent;
    private WXTextDomObject mDomObject;
    private WXDomObject mParentDomObj;

    @Before
    public void setUp() throws Exception {
        WXEnvironment.sApplication = RuntimeEnvironment.application;
        WXSDKInstance instance = Mockito.mock(WXSDKInstance.class);
        Mockito.when(instance.getContext()).thenReturn(RuntimeEnvironment.application);
        Mockito.when(instance.getFlatUIContext()).thenReturn(new FlatGUIContext());

        mParentDomObj = Mockito.spy(new WXDomObject());
        Mockito.when(mParentDomObj.getPadding()).thenReturn(new Spacing());
        Mockito.when(mParentDomObj.getBorder()).thenReturn(new Spacing());
        Mockito.when(mParentDomObj.clone()).thenReturn(mParentDomObj);
        TestDomObject.setRef(mParentDomObj,WXDomObject.ROOT);

        mDomObject = Mockito.spy(new WXTextDomObject());
        TestDomObject.setRef(mDomObject,"1");
        mDomObject.addEvent(Constants.Event.CLICK);
        Mockito.when(mDomObject.clone()).thenReturn(mDomObject);
        Mockito.when(mDomObject.getPadding()).thenReturn(new Spacing());
        Mockito.when(mDomObject.getBorder()).thenReturn(new Spacing());
        Mockito.when(mDomObject.getMargin()).thenReturn(new Spacing());
        Mockito.when(mDomObject.getLayoutWidth()).thenReturn(100f);
        Mockito.when(mDomObject.getLayoutHeight()).thenReturn(100f);

        mParent = new WXDiv(instance, mParentDomObj, null);
        mParent.createView();
        mWXText = new WXText(instance, mDomObject, mParent);
        mWXText.bindHolder(new SimpleComponentHolder(WXText.class));
        assertNotNull(instance.getContext());
    }

    @Test
    public void testSetProperties() throws Exception {
        mWXText.setProperty(Constants.Name.LINES,null);
        mWXText.setProperty(Constants.Name.FONT_SIZE,null);
        mWXText.setProperty(Constants.Name.FONT_WEIGHT,null);
        mWXText.setProperty(Constants.Name.FONT_STYLE,null);
        mWXText.setProperty(Constants.Name.COLOR,null);
        mWXText.setProperty(Constants.Name.TEXT_DECORATION,null);
        mWXText.setProperty(Constants.Name.FONT_FAMILY,null);
        mWXText.setProperty(Constants.Name.TEXT_ALIGN,null);
        mWXText.setProperty(Constants.Name.TEXT_OVERFLOW,null);
        mWXText.setProperty(Constants.Name.LINE_HEIGHT,null);
    }

    @Test
    public void testCreateView(){
        mWXText.createView();
        assertNotNull(mWXText.getHostView());
    }

    @Test
    public void testSetLayout(){
        testCreateView();
        mWXText.setLayout(mDomObject);
        assertNotNull(mWXText.getHostView().getLayoutParams());
        assertEquals(100, mWXText.getHostView().getLayoutParams().height);
        assertEquals(100, mWXText.getHostView().getLayoutParams().width);
    }

    @Test
    public void testSetPadding(){
        testCreateView();
        mWXText.setPadding(mDomObject.getPadding(), mDomObject.getBorder());
        assertEquals(0, mWXText.getHostView().getPaddingLeft());
        assertEquals(0, mWXText.getHostView().getPaddingTop());
        assertEquals(0, mWXText.getHostView().getPaddingRight());
        assertEquals(0, mWXText.getHostView().getPaddingBottom());
    }

    @Test
    public void testBind(){
        testCreateView();
//        mWXText.bind(null);
        mWXText.applyLayoutAndEvent(mWXText);
        mWXText.bindData(mWXText);

        assertNotNull(mWXText.getHostView().getLayoutParams());
        assertEquals(100, mWXText.getHostView().getLayoutParams().height);
        assertEquals(100, mWXText.getHostView().getLayoutParams().width);

        assertEquals(0, mWXText.getHostView().getPaddingLeft());
        assertEquals(0, mWXText.getHostView().getPaddingTop());
        assertEquals(0, mWXText.getHostView().getPaddingRight());
        assertEquals(0, mWXText.getHostView().getPaddingBottom());
    }

    @Test
    public void testAddEvent(){
        testBind();
        mWXText.addEvent(Constants.Event.CLICK);
        assertTrue(mWXText.getHostView().isEnabled());
        mWXText.getHostView().performClick();
    }

    @Test
    public void testUpdateProperties(){
        testBind();
        Map<String, Object> prop = new HashMap<>();
        prop.put(Constants.Name.DISABLED, "false");
        prop.put(Constants.Name.OPACITY, 0.8f);
        mWXText.updateProperties(prop);
        assertTrue(mWXText.getHostView().isEnabled());
        assertEquals(0.8f, mWXText.getHostView().getAlpha(), 0.001f);

        prop.put(Constants.Name.DISABLED, "true");
        mWXText.updateProperties(prop);
        assertFalse(mWXText.getHostView().isEnabled());
    }
}
