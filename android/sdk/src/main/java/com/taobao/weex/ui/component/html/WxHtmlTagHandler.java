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
package com.taobao.weex.ui.component.html;

import android.text.Editable;
import android.text.Spannable;

import com.taobao.weex.ui.component.html.htmlcompat.HtmlCompat;

import org.xml.sax.Attributes;
import org.xml.sax.XMLReader;
/**
 * Created by Bruce Too On 2019/1/9. At 19:31
 */
public class WxHtmlTagHandler implements HtmlCompat.TagHandler {

  private static final String TAG = WxHtmlTagHandler.class.getSimpleName();

  @Override
  public void handleTag(boolean opening, String tag, Attributes attributes, Editable output, XMLReader xmlReader) {
    //TODO  more tag support by textView (nested <li> etc..)
  }

  /** Returns the text contained within a span and deletes it from the output string */
  private CharSequence extractSpanText(Editable output, Class kind) {
    final Object obj = getLast(output, kind);
    // start of the tag
    final int where = output.getSpanStart(obj);
    // end of the tag
    final int len = output.length();

    final CharSequence extractedSpanText = output.subSequence(where, len);
    output.delete(where, len);
    return extractedSpanText;
  }

  /** Get last marked position of a specific tag kind (private class) */
  private static Object getLast(Editable text, Class kind) {
    Object[] objs = text.getSpans(0, text.length(), kind);
    if (objs.length == 0) {
      return null;
    } else {
      for (int i = objs.length; i > 0; i--) {
        if (text.getSpanFlags(objs[i - 1]) == Spannable.SPAN_MARK_MARK) {
          return objs[i - 1];
        }
      }
      return null;
    }
  }
}
