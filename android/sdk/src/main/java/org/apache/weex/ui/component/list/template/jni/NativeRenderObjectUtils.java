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
package org.apache.weex.ui.component.list.template.jni;

import org.apache.weex.base.CalledByNative;
import org.apache.weex.ui.component.WXComponent;

/**
 * Created by furture on 2018/5/3.
 */

public class NativeRenderObjectUtils {


    /**
     * getRenderObject
     * */
    public static native long nativeGetRenderObject(String instanceId, String ref);

    /**
     * render object layout
     * */
    public static native void nativeUpdateRenderObjectStyle(long ptr, String key, String value);
    public static native void nativeUpdateRenderObjectAttr(long ptr, String key, String value);

    /**
     * copy render object
     * */
    public static native long nativeCopyRenderObject(long ptr);

    /**
     * layout render object
     * */
    public static native int nativeLayoutRenderObject(long ptr, float width, float height);

    public static native int nativeRenderObjectGetLayoutDirectionFromPathNode(long ptr);
    /**
     * get child length
     * */
    public static native void nativeAddChildRenderObject(long parent, long child);

    /**
     * get component, tranverse child and update component
     * */
    public static native boolean nativeRenderObjectHasNewLayout(long ptr);
    public static native int nativeRenderObjectChildCount(long ptr);
    public static native long nativeRenderObjectGetChild(long ptr, int index);
    public static native void nativeRenderObjectUpdateComponent(long ptr, WXComponent component);
    public static native void nativeRenderObjectChildWaste(long ptr, boolean waster);


    /**
     * component size
     * */
    @CalledByNative
    public static void updateComponentSize(WXComponent component, float top, float bottom, float left, float right, float height, float width){
        component.updateDemission(top, bottom, left, right, height, width);
        component.applyLayoutOnly();
    }

}
