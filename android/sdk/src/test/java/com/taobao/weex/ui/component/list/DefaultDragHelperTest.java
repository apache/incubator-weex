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
package com.taobao.weex.ui.component.list;

import com.taobao.weappplus_sdk.BuildConfig;
import com.taobao.weex.WXSDKInstanceTest;
import com.taobao.weex.dom.TestDomObject;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.dom.WXListDomObject;
import com.taobao.weex.ui.SimpleComponentHolder;
import com.taobao.weex.ui.component.ComponentTest;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXDiv;
import com.taobao.weex.ui.component.WXDivTest;
import com.taobao.weex.ui.component.WXVContainer;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PowerMockIgnore;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;

import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;
import java.util.List;

import static org.junit.Assert.assertEquals;
import static org.mockito.Matchers.anyMap;
import static org.mockito.Matchers.eq;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;

@RunWith(RobolectricTestRunner.class)
@Config(constants = BuildConfig.class, sdk = 19)
@PowerMockIgnore( {"org.mockito.*", "org.robolectric.*", "android.*"})
public class DefaultDragHelperTest {

    WXListComponent listComponent;

    WXCell fakeCell;

    private DragHelper mFakeDragHelper;

    private EventTrigger mockedEventTrigger;

    private WXComponent c1;
    private WXComponent c2;
    private WXComponent c3;

    private List<WXComponent> mFakeDataSource;

    public static WXListComponent create(WXVContainer parent) throws IllegalAccessException, InstantiationException, InvocationTargetException {
        return create(parent,new WXListDomObject());
    }

    public static WXListComponent create(WXVContainer parent, WXDomObject dom) throws IllegalAccessException, InstantiationException, InvocationTargetException {
        return (WXListComponent) new SimpleComponentHolder(WXListComponent.class).createInstance(WXSDKInstanceTest.createInstance(), dom, parent);
    }

    @Before
    public void setUp() throws Exception {
        WXDiv div = WXDivTest.create();
        ComponentTest.create(div);
        listComponent = create(div);
        ComponentTest.create(listComponent);

        fakeCell = new WXCell(WXSDKInstanceTest.createInstance(),new TestDomObject(),null,false);

        c1 = new WXCell(WXSDKInstanceTest.createInstance(),new TestDomObject(),null,false);
        c2 = new WXCell(WXSDKInstanceTest.createInstance(),new TestDomObject(),null,false);
        c3 = new WXCell(WXSDKInstanceTest.createInstance(),new TestDomObject(),null,false);

        mFakeDataSource = new ArrayList<>();
        mFakeDataSource.add(c1);
        mFakeDataSource.add(c2);
        mFakeDataSource.add(c3);

        mockedEventTrigger = mock(EventTrigger.class);

        mFakeDragHelper = new DefaultDragHelper(mFakeDataSource,listComponent.getHostView().getInnerView(), mockedEventTrigger);
    }

    @After
    public void tearDown() throws Exception {
        listComponent.destroy();
    }

    @Test
    public void onDragStart() throws Exception {
        WXComponent c = new WXCell(WXSDKInstanceTest.createInstance(),new TestDomObject(),null,false);
        mFakeDragHelper.onDragStart(c,3);
        verify(mockedEventTrigger).triggerEvent(eq("dragstart"),anyMap());
    }

    @Test
    public void onDragEnd() throws Exception {
        WXComponent c = new WXCell(WXSDKInstanceTest.createInstance(),new TestDomObject(),null,false);
        mFakeDragHelper.onDragEnd(c,1,2);
        verify(mockedEventTrigger).triggerEvent(eq("dragend"),anyMap());
    }

    @Test
    public void onDragging() throws Exception {
        assertEquals(mFakeDataSource.get(0),c1);
        assertEquals(mFakeDataSource.get(1),c2);

        mFakeDragHelper.onDragging(0,1);

        assertEquals(mFakeDataSource.get(0),c2);
        assertEquals(mFakeDataSource.get(1),c1);
    }


}