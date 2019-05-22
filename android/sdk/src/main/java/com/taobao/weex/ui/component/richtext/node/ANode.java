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
package com.taobao.weex.ui.component.richtext.node;

import android.content.Context;
import android.text.SpannableStringBuilder;

import com.taobao.weex.ui.component.richtext.span.ASpan;
import com.taobao.weex.ui.component.richtext.span.ItemClickSpan;

class ANode extends RichTextNode {

  static class ANodeCreator implements RichTextNodeCreator<ANode> {

    @Override
    public ANode createRichTextNode(Context context, String instanceId, String componentRef) {
      return new ANode(context, instanceId, componentRef);
    }
  }

  public static final String NODE_TYPE = "a";
  public static final String HREF = "href";

  private ANode(Context context, String instanceId, String componentRef) {
    super(context, instanceId, componentRef);
  }

  @Override
  public String toString() {
    return "";
  }

  @Override
  protected boolean isInternalNode() {
    return true;
  }

  @Override
  protected void updateSpans(SpannableStringBuilder spannableStringBuilder, int level) {
    super.updateSpans(spannableStringBuilder, level);
      if (attr != null && attr.containsKey(RichTextNode.PSEUDO_REF)) {
          ItemClickSpan itemClickSpan = new ItemClickSpan(mInstanceId, mComponentRef, attr.get(RichTextNode.PSEUDO_REF).toString());
          spannableStringBuilder.setSpan(itemClickSpan, 0, spannableStringBuilder.length(),
                  createSpanFlag(level));
      } else if (attr != null && attr.containsKey(HREF)) {
          ASpan aSpan = new ASpan(mInstanceId, attr.get(HREF).toString());
          spannableStringBuilder.setSpan(aSpan, 0, spannableStringBuilder.length(),
                  createSpanFlag(level));
      }
  }
}
