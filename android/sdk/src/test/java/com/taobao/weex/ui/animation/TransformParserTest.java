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
package com.taobao.weex.ui.animation;

import android.util.Property;
import android.view.View;

import junit.framework.TestCase;

import java.util.Map;

/**
 * Created by furture on 2017/10/24.
 */

public class TransformParserTest extends TestCase {

    public void testParseTransform(){
        Map<Property<View,Float>, Float> transforms =  TransformParser.parseTransForm("rotate(7deg) translate(1, 2)", 100, 100, 750);
        System.out.println(transforms.size());
    }
}
