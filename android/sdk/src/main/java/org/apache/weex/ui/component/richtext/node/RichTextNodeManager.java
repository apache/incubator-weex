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
package org.apache.weex.ui.component.richtext.node;

import android.content.Context;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v4.util.ArrayMap;

import com.alibaba.fastjson.JSONObject;
import org.apache.weex.utils.WXLogUtils;

import java.util.Map;

public class RichTextNodeManager {

  private final static Map<String, RichTextNodeCreator>
      registeredTextNodes = new ArrayMap<>();

  static {
    registeredTextNodes.put(SpanNode.NODE_TYPE, new SpanNode.SpanNodeCreator());
    registeredTextNodes.put(ImgNode.NODE_TYPE, new ImgNode.ImgNodeCreator());
    registeredTextNodes.put(ANode.NODE_TYPE, new ANode.ANodeCreator());
  }

  public static void registerTextNode(String text, RichTextNodeCreator type) {
    registeredTextNodes.put(text, type);
  }

  @Nullable
  static RichTextNode createRichTextNode(@NonNull Context context, @NonNull String instanceId,
      @NonNull String componentRef, @Nullable JSONObject jsonObject) {
    RichTextNode instance = null;
    try {
      if (jsonObject != null) {
        instance = registeredTextNodes.get(jsonObject.getString(RichTextNode.TYPE))
            .createRichTextNode(context, instanceId, componentRef);
        instance.parse(context, instanceId, componentRef, jsonObject);
      }
    } catch (Exception e) {
      WXLogUtils.e("Richtext", WXLogUtils.getStackTrace(e));
      instance = null;
    }
    return instance;
  }
  public static RichTextNode createRichTextNode(Context context,String instanceId,String componentRef,String ref,
                                         String nodeType,Map<String,String> styles,Map<String,String> attrs){
    RichTextNode instance;
    try{
      instance = registeredTextNodes.get(nodeType).createRichTextNode(context,instanceId,componentRef,ref,styles,attrs);
    }catch (Exception e){
      WXLogUtils.e("Richtext", WXLogUtils.getStackTrace(e));
      instance = null;
    }
    return instance;
  }
}
