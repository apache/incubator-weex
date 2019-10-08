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
package org.apache.weex.ui.component.richtext;


import android.content.Context;
import android.support.annotation.NonNull;
import android.text.Spannable;
import android.text.SpannableStringBuilder;
import android.text.Spanned;
import android.text.SpannedString;
import android.text.TextUtils;

import org.apache.weex.WXSDKInstance;
import org.apache.weex.layout.measurefunc.TextContentBoxMeasurement;
import org.apache.weex.ui.ComponentCreator;
import org.apache.weex.ui.action.BasicComponentData;
import org.apache.weex.ui.component.WXComponent;
import org.apache.weex.ui.component.WXText;
import org.apache.weex.ui.component.WXVContainer;
import org.apache.weex.ui.component.richtext.node.RichTextNode;
import org.apache.weex.ui.component.richtext.node.RichTextNodeManager;

import java.lang.reflect.InvocationTargetException;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

public class WXRichText extends WXText {
  private List<RichTextNode> nodes = new LinkedList<>();

  static class RichTextContentBoxMeasurement extends TextContentBoxMeasurement {

    public RichTextContentBoxMeasurement(WXComponent component) {
      super(component);
    }

    @NonNull
    @Override
    protected Spanned createSpanned(String text) {
      if(!TextUtils.isEmpty(text)) {
        if (mComponent.getInstance() != null & mComponent.getInstance().getUIContext() != null &&
                !TextUtils.isEmpty(mComponent.getInstanceId())) {
          Spannable spannable = RichTextNode.parse(
                  mComponent.getInstance().getUIContext(),
                  mComponent.getInstanceId(),
                  mComponent.getRef(),
                  text);
          updateSpannable(spannable, RichTextNode.createSpanFlag(0));
          return spannable;
        } else {
          return new SpannedString("");
        }
      }
      else {
        Spannable spannable = ((WXRichText)mComponent).toSpan();
        updateSpannable(spannable, RichTextNode.createSpanFlag(0));
        return spannable;
        }
    }
  }

  public static class Creator implements ComponentCreator {

    public WXComponent createInstance(WXSDKInstance instance, WXVContainer parent, BasicComponentData basicComponentData) throws IllegalAccessException, InvocationTargetException, InstantiationException {
      return new WXRichText(instance, parent, basicComponentData);
    }
  }

  public WXRichText(WXSDKInstance instance, WXVContainer parent, BasicComponentData basicComponentData) {
    super(instance, parent, basicComponentData);
    setContentBoxMeasurement(new RichTextContentBoxMeasurement(this));
  }
  public void AddChildNode(String ref, String nodeType,String parentRef, Map<String,String> styles,Map<String,String> attrs) {
    if(getInstance() != null && getInstance().getUIContext() != null && !TextUtils.isEmpty(getInstanceId()) &&
            !TextUtils.isEmpty(ref) && !TextUtils.isEmpty(nodeType)) {
      RichTextNode child = RichTextNodeManager.createRichTextNode(getInstance().getUIContext(), getInstanceId(), getRef(), ref, nodeType, styles, attrs);
      if (TextUtils.isEmpty(parentRef)) {
        nodes.add(child);
      } else {
        RichTextNode parent = findRichNode(parentRef);
        if (parent != null) {
          parent.addChildNode(child);
        }
      }
    }
  }
  private Spannable toSpan(){
    SpannableStringBuilder spannableStringBuilder = new SpannableStringBuilder();
    if(nodes != null && !nodes.isEmpty()){
      for(RichTextNode node:nodes){
        spannableStringBuilder.append(node.toSpan(1));
      }
    }
    return spannableStringBuilder;
  }

  public void removeChildNode(String parentRef,String ref) {
    if (nodes != null && !nodes.isEmpty()) {
      if (parentRef.equals("")) {
        for (RichTextNode node : nodes) {
          if (TextUtils.equals(node.getRef(), ref)) {
            nodes.remove(node);
          }
        }
      } else {
        RichTextNode parent = findRichNode(parentRef);
        if (parent != null) {
          parent.removeChildNode(ref);
        }
      }
    }
  }
  public void updateChildNodeStyles(String ref,Map<String,Object> styles){
    RichTextNode node = findRichNode(ref);
    if(node != null){
      node.updateStyles(styles);
    }
  }
  public void updateChildNodeAttrs(String ref,Map<String,Object> attrs){
    RichTextNode node = findRichNode(ref);
    if(node != null){
      node.updateAttrs(attrs);
    }
  }
  private RichTextNode findRichNode(String ref){
    if(!TextUtils.isEmpty(ref)) {
      RichTextNode theNode;
      if (nodes != null && !nodes.isEmpty()) {
        for (RichTextNode node : nodes) {
          if ((theNode = node.findRichNode(ref)) != null)
            return theNode;
        }
      }
    }
    return null;
  }

  @Override
  protected WXRichTextView initComponentHostView(@NonNull Context context) {
    return new WXRichTextView(context);
  }
}
