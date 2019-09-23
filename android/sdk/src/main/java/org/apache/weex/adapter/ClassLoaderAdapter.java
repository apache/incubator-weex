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
package org.apache.weex.adapter;

import android.content.Context;

import org.apache.weex.WXSDKInstance;
import org.apache.weex.common.WXModule;
import org.apache.weex.ui.component.WXComponent;

/**
 * Created by furture on 2018/2/7.
 * class loader adapter for load auto config class.
 */
public class ClassLoaderAdapter {

    /**
     * context is module class
     * */
    public Class<? extends WXModule> getModuleClass(String name, String className, Context context){
        try {
            return (Class<? extends WXModule>) context.getClassLoader().loadClass(className);
        } catch (ClassNotFoundException e) {
            throw  new RuntimeException(e);
        }
    }

    /**
     * context is instance context
     */
    public Class<? extends WXComponent> getComponentClass(String name, String className, WXSDKInstance instance) {
        try {
            return (Class<? extends WXComponent>) instance.getContext().getClassLoader().loadClass(className);
        } catch (ClassNotFoundException e) {
            throw  new RuntimeException(e);
        }
    }

}