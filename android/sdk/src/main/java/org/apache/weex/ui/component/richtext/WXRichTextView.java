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

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.drawable.Drawable;
import android.text.Layout;
import android.text.Selection;
import android.text.Spannable;
import android.text.Spanned;
import android.text.style.ClickableSpan;
import android.view.MotionEvent;
import android.widget.TextView;

import org.apache.weex.ui.component.richtext.span.ImgSpan;
import org.apache.weex.ui.view.WXTextView;

public class WXRichTextView extends WXTextView {

  public WXRichTextView(Context context) {
    super(context);
  }

  @SuppressLint("ClickableViewAccessibility")
  @Override
  public boolean onTouchEvent(MotionEvent event) {
    boolean superResult = super.onTouchEvent(event);
    boolean handled = false;
    if (isEnabled() && getTextLayout() != null && getText() instanceof Spannable) {
      Spannable spannable = (Spannable) getText();
      handled = updateSelection(event, spannable);
    }
    return handled || superResult;
  }

  @Override
  protected boolean verifyDrawable(Drawable who) {
    super.verifyDrawable(who);
    return true;
  }

  @Override
  public void setTextLayout(Layout layout) {
    super.setTextLayout(layout);
    if (layout.getText() instanceof Spanned) {
      Spanned spanned = (Spanned) layout.getText();
      ImgSpan[] imgSpan = spanned.getSpans(0, spanned.length(), ImgSpan.class);
      if (imgSpan != null) {
        for (ImgSpan span : imgSpan) {
          span.setView(this);
        }
      }
    }
  }

  /**
   * Mostly copied from
   * {@link android.text.method.LinkMovementMethod#onTouchEvent(TextView, Spannable, MotionEvent)}.
   */
  private boolean updateSelection(MotionEvent event, Spannable buffer) {
    int action = event.getActionMasked();

    if (action == MotionEvent.ACTION_UP ||
        action == MotionEvent.ACTION_DOWN) {
      int x = (int) event.getX();
      int y = (int) event.getY();

      x -= getPaddingLeft();
      y -= getPaddingTop();

      x += getScrollX();
      y += getScrollY();

      Layout layout = getTextLayout();
      int line = layout.getLineForVertical(y);
      int off = layout.getOffsetForHorizontal(line, x);

      ClickableSpan[] link = buffer.getSpans(off, off, ClickableSpan.class);

      if (link.length != 0) {
        if (action == MotionEvent.ACTION_UP) {
          link[0].onClick(this);
        } else {
          Selection.setSelection(buffer,
                                 buffer.getSpanStart(link[0]),
                                 buffer.getSpanEnd(link[0]));
        }

        return true;
      } else {
        Selection.removeSelection(buffer);
      }
    }

    return false;
  }


}
