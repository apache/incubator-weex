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
import android.text.SpannableStringBuilder;

import org.apache.weex.common.Constants;
import org.apache.weex.dom.TextDecorationSpan;
import org.apache.weex.dom.WXStyle;

import java.util.Map;

class SpanNode extends RichTextNode {

  static class SpanNodeCreator implements RichTextNodeCreator<SpanNode> {

    @Override
    public SpanNode createRichTextNode(Context context, String instanceId, String componentRef) {
      return new SpanNode(context, instanceId, componentRef);
    }

    @Override
    public SpanNode createRichTextNode(Context context,String instanceId,String componentRef,String ref,
                                       Map<String,Object> styles, Map<String,Object> attrs){
      return new SpanNode(context,instanceId,componentRef,ref,styles,attrs);
    }
  }

  public static final String NODE_TYPE = "span";

  private SpanNode(Context context, String instanceId, String componentRef) {
    super(context, instanceId, componentRef);
  }
  private SpanNode(Context context, String instanceId, String componentRef, String ref, Map<String,Object> styles,Map<String,Object> attrs) {
    super(context, instanceId, componentRef, ref, styles, attrs);
  }

  @Override
  public String toString() {
    if (attr == null || !attr.containsKey(Constants.Name.VALUE)) {
      return "";
    } else {
      return attr.get(Constants.Name.VALUE).toString();
    }
  }

  @Override
  protected boolean isInternalNode() {
    return true;
  }

  @Override
  protected void updateSpans(SpannableStringBuilder spannableStringBuilder, int level) {
    super.updateSpans(spannableStringBuilder, level);
    spannableStringBuilder.setSpan(new TextDecorationSpan(WXStyle.getTextDecoration(style)), 0,
        spannableStringBuilder.length(), createSpanFlag(level));
  }
}
