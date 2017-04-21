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
package com.taobao.weex.dom;

import android.graphics.Typeface;

import com.taobao.weex.common.Constants;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import java.util.HashMap;
import java.util.Map;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertThat;
import static org.hamcrest.CoreMatchers.*;


/**
 * Description:
 *
 * Created by rowandjj(chuyi)<br/>
 */

public class WXStyleTest {
    WXStyle style;

    @Before
    public void setUp() throws Exception {
        style = new WXStyle();
    }


    @Test
    public void testBlur() {
        assertEquals(0,style.size());
        assertEquals(null,style.getBlur());
        style.put(Constants.Name.FILTER,"blur(5px)");
        assertEquals("blur(5px)",style.getBlur());

    }

    @Test
    public void testFontWeight(){
        WXStyle none = new WXStyle();
        assertThat(WXStyle.getFontWeight(none), is(Typeface.NORMAL));

        WXStyle normal = new WXStyle();
        normal.put(Constants.Name.FONT_WEIGHT, Constants.Value.NORMAL);
        assertThat(WXStyle.getFontWeight(normal), is(Typeface.NORMAL));

        WXStyle bold = new WXStyle();
        bold.put(Constants.Name.FONT_WEIGHT, Constants.Value.BOLD);
        assertThat(WXStyle.getFontWeight(bold), is(Typeface.BOLD));

        WXStyle illegal = new WXStyle();
        illegal.put(Constants.Name.FONT_WEIGHT, "f");
        assertThat(WXStyle.getFontWeight(illegal), is(Typeface.NORMAL));

        WXStyle number100 = new WXStyle();
        number100.put(Constants.Name.FONT_WEIGHT, "100");
        assertThat(WXStyle.getFontWeight(number100), is(Typeface.NORMAL));

        WXStyle number200 = new WXStyle();
        number200.put(Constants.Name.FONT_WEIGHT, "200");
        assertThat(WXStyle.getFontWeight(number200), is(Typeface.NORMAL));

        WXStyle number300 = new WXStyle();
        number300.put(Constants.Name.FONT_WEIGHT, "300");
        assertThat(WXStyle.getFontWeight(number300), is(Typeface.NORMAL));

        WXStyle number400 = new WXStyle();
        number400.put(Constants.Name.FONT_WEIGHT, "400");
        assertThat(WXStyle.getFontWeight(number400), is(Typeface.NORMAL));

        WXStyle number500 = new WXStyle();
        number500.put(Constants.Name.FONT_WEIGHT, "500");
        assertThat(WXStyle.getFontWeight(number500), is(Typeface.NORMAL));

        WXStyle number600 = new WXStyle();
        number600.put(Constants.Name.FONT_WEIGHT, "600");
        assertThat(WXStyle.getFontWeight(number600), is(Typeface.BOLD));

        WXStyle number700 = new WXStyle();
        number700.put(Constants.Name.FONT_WEIGHT, "700");
        assertThat(WXStyle.getFontWeight(number700), is(Typeface.BOLD));

        WXStyle number800 = new WXStyle();
        number800.put(Constants.Name.FONT_WEIGHT, "800");
        assertThat(WXStyle.getFontWeight(number800), is(Typeface.BOLD));

        WXStyle number900 = new WXStyle();
        number900.put(Constants.Name.FONT_WEIGHT, "900");
        assertThat(WXStyle.getFontWeight(number900), is(Typeface.BOLD));
    }

    @After
    public void tearDown() throws Exception {
        style.clear();
    }

    @Test
    public void testPesudoParsing() throws Exception {
        Map<String,Object> styles = new HashMap<>();
        styles.put("color","#FF0000");
        styles.put("color:active","#008000");
        styles.put("fontSize:active:some_clz",20);
        styles.put("color:active:enabled",15);
        styles.put("color:active:disabled","#008000");
        styles.put("fontSize:active:disabled",30);

        WXStyle style = new WXStyle();
        style.putAll(styles,false);

        assertEquals(style.getPesudoStyles().get(":active").keySet().size(),1);
        assertEquals(style.getPesudoStyles().get(":active:some_clz").get("fontSize"),20);
        assertEquals(style.getPesudoStyles().get(":active:disabled").get("fontSize"),30);
        assertEquals(style.getPesudoStyles().get(":active:disabled").keySet().size(),2);
    }
}
