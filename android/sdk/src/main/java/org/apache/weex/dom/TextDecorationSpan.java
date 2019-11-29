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
package org.apache.weex.dom;


import android.support.annotation.NonNull;
import android.text.TextPaint;
import android.text.style.CharacterStyle;
import android.text.style.UpdateAppearance;

import org.apache.weex.ui.component.WXTextDecoration;

public class TextDecorationSpan extends CharacterStyle
    implements UpdateAppearance {

  private final WXTextDecoration mTextDecoration;

  public TextDecorationSpan(@NonNull WXTextDecoration wxTextDecoration) {
    this.mTextDecoration = wxTextDecoration;
  }

  @Override
  public void updateDrawState(TextPaint tp) {
    switch (mTextDecoration) {
      case LINETHROUGH:
        tp.setUnderlineText(false);
        tp.setStrikeThruText(true);
        break;
      case UNDERLINE:
        tp.setUnderlineText(true);
        tp.setStrikeThruText(false);
        break;
      case NONE:
        tp.setUnderlineText(false);
        tp.setStrikeThruText(false);
        break;
    }
  }
}
