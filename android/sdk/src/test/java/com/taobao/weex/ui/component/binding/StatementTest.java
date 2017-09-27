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
package com.taobao.weex.ui.component.binding;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONObject;
import com.taobao.weappplus_sdk.BuildConfig;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKInstanceTest;
import com.taobao.weex.dom.TestDomObject;
import com.taobao.weex.dom.WXCellDomObject;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.dom.WXEvent;
import com.taobao.weex.dom.WXTextDomObject;
import com.taobao.weex.dom.binding.ELUtils;
import com.taobao.weex.dom.binding.WXStatement;
import com.taobao.weex.dom.flex.Spacing;
import com.taobao.weex.el.parse.ArrayStack;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXComponentFactory;
import com.taobao.weex.ui.component.WXDiv;
import com.taobao.weex.ui.component.WXText;
import com.taobao.weex.ui.component.list.WXCell;

import junit.framework.Assert;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.mockito.invocation.InvocationOnMock;
import org.mockito.stubbing.Answer;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PowerMockIgnore;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.rule.PowerMockRule;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.RuntimeEnvironment;
import org.robolectric.annotation.Config;

import java.util.ArrayList;
import java.util.List;
import java.util.Stack;

/**
 * Created by furture on 2017/8/18.
 */
@RunWith(RobolectricTestRunner.class)
@Config(constants = BuildConfig.class, sdk = 19)
@PowerMockIgnore({ "org.mockito.*", "org.robolectric.*", "android.*" })
@PrepareForTest(WXComponentFactory.class)
public class StatementTest {


    @Rule
    public PowerMockRule rule = new PowerMockRule();

    @Test
    public void testVFor() throws Exception {
        WXCell cell = createVForNode();
        int count = 3;
        Statements.doRender(cell, createContext(count));
        Assert.assertTrue(cell.getChildCount() == 1);
        WXDiv div = (WXDiv) cell.getChild(0);
        Assert.assertEquals(div.getChildCount(), count);
        Assert.assertNotNull(div.getChild(0).getDomObject());
        Assert.assertNotNull(((WXDomObject)div.getChild(0).getDomObject()).getAttrs().getStatement());
        Assert.assertNull(((WXDomObject)div.getChild(1).getDomObject()).getAttrs().getStatement());
        WXComponent childOne = div.getChild(0);
        WXComponent childTwo = div.getChild(1);
        WXComponent childThree = div.getChild(2);

        count = 4;
        Statements.doRender(cell, createContext(count));
        Assert.assertTrue(cell.getChildCount() == 1);
        div = (WXDiv) cell.getChild(0);
        Assert.assertTrue(div.getChildCount() == count);
        Assert.assertSame(childOne, div.getChild(0));
        Assert.assertSame(childTwo, div.getChild(1));
        Assert.assertSame(childThree, div.getChild(2));
        WXComponent childFour = div.getChild(3);

        count = 5;
        Statements.doRender(cell, createContext(count));
        Assert.assertTrue(cell.getChildCount() == 1);
        div = (WXDiv) cell.getChild(0);
        Assert.assertTrue(div.getChildCount() == count);
        Assert.assertSame(childOne, div.getChild(0));
        Assert.assertSame(childTwo, div.getChild(1));
        Assert.assertSame(childThree, div.getChild(2));
        Assert.assertSame(childFour, div.getChild(3));


        count = 3;
        Statements.doRender(cell, createContext(count));
        Assert.assertTrue(cell.getChildCount() == 1);
        div = (WXDiv) cell.getChild(0);
        Assert.assertTrue(div.getChildCount() == 5);
        Assert.assertSame(childOne, div.getChild(0));
        Assert.assertSame(childTwo, div.getChild(1));
        Assert.assertSame(childThree, div.getChild(2));

        for(int i=count; i<5; i++){
            Assert.assertTrue(div.getChild(i).isWaste());
        }
    }




    @Before
    public void setUp() throws Exception {
        WXSDKInstance instance = Mockito.mock(WXSDKInstance.class);
        Mockito.when(instance.getContext()).thenReturn(RuntimeEnvironment.application);
        WXDomObject divDom = new WXDomObject();
        WXDomObject spy = Mockito.spy(divDom);
        Mockito.when(spy.getPadding()).thenReturn(new Spacing());
        Mockito.when(spy.getEvents()).thenReturn(new WXEvent());
        Mockito.when(spy.clone()).thenReturn(divDom);
        TestDomObject.setRef(divDom,"1");

    }


    private WXCell createVForNode() throws Exception {
        WXCell cell = new WXCell(WXSDKInstanceTest.createInstance(),new WXCellDomObject(), null, false);
        final WXDiv div = new WXDiv(WXSDKInstanceTest.createInstance(), new WXDomObject(), cell);
        cell.addChild(div);
        WXText text = new WXText(WXSDKInstanceTest.createInstance(), new WXTextDomObject(), div);
        WXStatement statement = new WXStatement();
        statement.put("[[repeat]]",
                ELUtils.vforBlock(
                        JSON.parse("{\n" +
                                "      '@expression': 'dataList',\n" +
                                "      '@index': 'index',\n" +
                                "      '@alias': 'item'\n" +
                                "    }")));
        WXDomObject domObject = (WXDomObject) text.getDomObject();
        domObject.getAttrs().setStatement(statement);
        div.addChild(text);

        PowerMockito.mockStatic(WXComponentFactory.class, new Answer() {
            @Override
            public Object answer(InvocationOnMock invocation) throws Throwable {
                 WXText renderNode = new WXText(WXSDKInstanceTest.createInstance(), new WXTextDomObject(), div);
                return renderNode;
            }
        });
        return cell;
    }

    private ArrayStack createContext(int count){
        JSONObject data = new JSONObject();
        data.put("item", new JSONObject());
        data.getJSONObject("item").put("name", "hello world");
        List<String> datas = new ArrayList<>();
        for(int i=0; i<count; i++){
            datas.add("hello" + count);
        }
        data.getJSONObject("item").put("list", datas);
        data.put("dataList", datas);
        ArrayStack context = new ArrayStack();
        context.push(data);
        return context;
    }

}
