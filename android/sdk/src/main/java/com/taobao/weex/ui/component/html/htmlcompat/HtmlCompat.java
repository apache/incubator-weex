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
package com.taobao.weex.ui.component.html.htmlcompat;

/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import android.annotation.SuppressLint;
import android.content.ActivityNotFoundException;
import android.content.Context;
import android.graphics.Color;
import android.graphics.Typeface;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.text.Editable;
import android.text.Layout;
import android.text.Spanned;
import android.text.TextUtils;
import android.text.style.AbsoluteSizeSpan;
import android.text.style.AlignmentSpan;
import android.text.style.BackgroundColorSpan;
import android.text.style.BulletSpan;
import android.text.style.CharacterStyle;
import android.text.style.ForegroundColorSpan;
import android.text.style.ImageSpan;
import android.text.style.ParagraphStyle;
import android.text.style.QuoteSpan;
import android.text.style.RelativeSizeSpan;
import android.text.style.StrikethroughSpan;
import android.text.style.StyleSpan;
import android.text.style.SubscriptSpan;
import android.text.style.SuperscriptSpan;
import android.text.style.TypefaceSpan;
import android.text.style.URLSpan;
import android.text.style.UnderlineSpan;
import android.view.View;

import org.ccil.cowan.tagsoup.HTMLSchema;
import org.ccil.cowan.tagsoup.Parser;
import org.xml.sax.Attributes;
import org.xml.sax.XMLReader;

/**
 * This class processes HTML strings into displayable styled text.
 * Not all HTML tags are supported.
 */
@SuppressWarnings({"unused", "WeakerAccess"})
public class HtmlCompat {

    /**
     * Is notified when HTML tags are encountered that the parser does
     * not know how to interpret.
     */
    public interface TagHandler {
        /**
         * This method will be called when the HTML parser encounters
         * a tag that it does not know how to interpret.
         */
        void handleTag(boolean opening, String tag,
                       Attributes attributes, Editable output, XMLReader xmlReader);
    }

    /**
     * Is notified when a new span is created.
     */
    public interface SpanCallback {
        /**
         * This method will be called when a new span is created.
         */
        Object onSpanCreated(String tag, Object span);
    }

    @SuppressLint("ParcelCreator")
    public static class DefensiveURLSpan extends URLSpan {

        public DefensiveURLSpan(String url) {
            super(url);
        }

        @Override
        public void onClick(View widget) {
            try {
                super.onClick(widget);
            } catch (ActivityNotFoundException e) {
                // Ignore
            }
        }

    }

    /**
     * Option for {@link #toHtml(Context, Spanned, int)}: Wrap consecutive lines of text delimited by '\n'
     * inside &lt;p&gt; elements. {@link BulletSpan}s are ignored.
     */
    public static final int TO_HTML_PARAGRAPH_LINES_CONSECUTIVE = 0x00000000;
    /**
     * Option for {@link #toHtml(Context, Spanned, int)}: Wrap each line of text delimited by '\n' inside a
     * &lt;p&gt; or a &lt;li&gt; element. This allows {@link ParagraphStyle}s attached to be
     * encoded as CSS styles within the corresponding &lt;p&gt; or &lt;li&gt; element.
     */
    public static final int TO_HTML_PARAGRAPH_LINES_INDIVIDUAL = 0x00000001;
    /**
     * Flag indicating that texts inside &lt;p&gt; elements will be separated from other texts with
     * one newline character by default.
     */
    public static final int FROM_HTML_SEPARATOR_LINE_BREAK_PARAGRAPH = 0x00000001;
    /**
     * Flag indicating that texts inside &lt;h1&gt;~&lt;h6&gt; elements will be separated from
     * other texts with one newline character by default.
     */
    public static final int FROM_HTML_SEPARATOR_LINE_BREAK_HEADING = 0x00000002;
    /**
     * Flag indicating that texts inside &lt;li&gt; elements will be separated from other texts
     * with one newline character by default.
     */
    public static final int FROM_HTML_SEPARATOR_LINE_BREAK_LIST_ITEM = 0x00000004;
    /**
     * Flag indicating that texts inside &lt;ul&gt; elements will be separated from other texts
     * with one newline character by default.
     */
    public static final int FROM_HTML_SEPARATOR_LINE_BREAK_LIST = 0x00000008;
    /**
     * Flag indicating that texts inside &lt;div&gt; elements will be separated from other texts
     * with one newline character by default.
     */
    public static final int FROM_HTML_SEPARATOR_LINE_BREAK_DIV = 0x00000010;
    /**
     * Flag indicating that texts inside &lt;blockquote&gt; elements will be separated from other
     * texts with one newline character by default.
     */
    public static final int FROM_HTML_SEPARATOR_LINE_BREAK_BLOCKQUOTE = 0x00000020;
    /**
     * Flag indicating that CSS color values should be used instead of those defined in
     * {@link Color}.
     */
    public static final int FROM_HTML_OPTION_USE_CSS_COLORS = 0x00000100;
    /**
     * Flags for {@link #fromHtml(Context, String, int, TagHandler)}: Separate block-level
     * elements with blank lines (two newline characters) in between. This is the legacy behavior
     * prior to N.
     */
    public static final int FROM_HTML_MODE_LEGACY = 0x00000000;
    /**
     * Flags for {@link #fromHtml(Context, String, int, TagHandler)}: Separate block-level
     * elements with line breaks (single newline character) in between. This inverts the
     * {@link Spanned} to HTML string conversion done with the option
     * {@link #TO_HTML_PARAGRAPH_LINES_INDIVIDUAL}.
     */
    public static final int FROM_HTML_MODE_COMPACT =
            FROM_HTML_SEPARATOR_LINE_BREAK_PARAGRAPH
                    | FROM_HTML_SEPARATOR_LINE_BREAK_HEADING
                    | FROM_HTML_SEPARATOR_LINE_BREAK_LIST_ITEM
                    | FROM_HTML_SEPARATOR_LINE_BREAK_LIST
                    | FROM_HTML_SEPARATOR_LINE_BREAK_DIV
                    | FROM_HTML_SEPARATOR_LINE_BREAK_BLOCKQUOTE;
    /**
     * The bit which indicates if lines delimited by '\n' will be grouped into &lt;p&gt; elements.
     */
    private static final int TO_HTML_PARAGRAPH_FLAG = 0x00000001;

    private HtmlCompat() {
    }

    /**
     * Returns displayable styled text from the provided HTML string. Any &lt;img&gt; tags in the
     * HTML will display as a generic replacement image which your program can then go through and
     * replace with real images.
     * <p>
     * <p>This uses TagSoup to handle real HTML, including all of the brokenness found in the wild.
     */
    public static Spanned fromHtml(@NonNull Context context, @NonNull String source, int flags) {
        return fromHtml(context, source, flags, null, null);
    }

    /**
     * Lazy initialization holder for HTML parser. This class will
     * a) be preloaded by the zygote, or b) not loaded until absolutely
     * necessary.
     */
    private static class HtmlParser {
        private static final HTMLSchema schema = new HTMLSchema();
    }

    /**
     * Returns displayable styled text from the provided HTML string. Any &lt;img&gt; tags in the
     * HTML will use the specified to request a representation of the image (use null
     * if you don't want this) and the specified TagHandler to handle unknown tags (specify null if
     * you don't want this).
     * <p>
     * <p>This uses TagSoup to handle real HTML, including all of the brokenness found in the wild.
     */
    public static Spanned fromHtml(@NonNull Context context, @NonNull String source, int flags,
                                   @Nullable TagHandler tagHandler) {
        return fromHtml(context, source, flags, tagHandler, null);
    }

    /**
     * Returns displayable styled text from the provided HTML string. Any &lt;img&gt; tags in the
     * HTML will use the specified to request a representation of the image (use null
     * if you don't want this) and the specified TagHandler to handle unknown tags (specify null if
     * you don't want this).
     * <p>
     * <p>This uses TagSoup to handle real HTML, including all of the brokenness found in the wild.
     */
    public static Spanned fromHtml(@NonNull Context context, @NonNull String source, int flags,
                                   @Nullable TagHandler tagHandler,
                                   @Nullable SpanCallback spanCallback) {
        if (source == null) {
            return null;
        }
        Parser parser = new Parser();
        try {
            parser.setProperty(Parser.schemaProperty, HtmlParser.schema);
        } catch (org.xml.sax.SAXNotRecognizedException | org.xml.sax.SAXNotSupportedException e) {
            // Should not happen.
            throw new RuntimeException(e);
        }
        HtmlToSpannedConverter converter =
                new HtmlToSpannedConverter(context, source, tagHandler, spanCallback, parser, flags);
        return converter.convert();
    }

    /**
     * Returns an HTML representation of the provided Spanned text. A best effort is
     * made to add HTML tags corresponding to spans. Also note that HTML metacharacters
     * (such as "&lt;" and "&amp;") within the input text are escaped.
     *
     * @param text   input text to convert
     * @param option one of {@link #TO_HTML_PARAGRAPH_LINES_CONSECUTIVE} or
     *               {@link #TO_HTML_PARAGRAPH_LINES_INDIVIDUAL}
     * @return string containing input converted to HTML
     */
    public static String toHtml(Context context, Spanned text, int option) {
        StringBuilder out = new StringBuilder();
        withinHtml(context, out, text, option);
        return out.toString();
    }

    /**
     * Returns an HTML escaped representation of the given plain text.
     */
    public static String escapeHtml(CharSequence text) {
        StringBuilder out = new StringBuilder();
        withinStyle(out, text, 0, text.length());
        return out.toString();
    }

    private static void withinHtml(Context context, StringBuilder out, Spanned text, int option) {
        if ((option & TO_HTML_PARAGRAPH_FLAG) == TO_HTML_PARAGRAPH_LINES_CONSECUTIVE) {
            encodeTextAlignmentByDiv(context, out, text, option);
            return;
        }
        withinDiv(context, out, text, 0, text.length(), option);
    }

    private static void encodeTextAlignmentByDiv(Context context, StringBuilder out, Spanned text, int option) {
        int len = text.length();
        int next;
        for (int i = 0; i < len; i = next) {
            next = text.nextSpanTransition(i, len, ParagraphStyle.class);
            ParagraphStyle[] styles = text.getSpans(i, next, ParagraphStyle.class);
            String elements = " ";
            boolean needDiv = false;
            for (ParagraphStyle style : styles) {
                if (style instanceof AlignmentSpan) {
                    Layout.Alignment align =
                            ((AlignmentSpan) style).getAlignment();
                    needDiv = true;
                    if (align == Layout.Alignment.ALIGN_CENTER) {
                        elements = "align=\"center\" " + elements;
                    } else if (align == Layout.Alignment.ALIGN_OPPOSITE) {
                        elements = "align=\"right\" " + elements;
                    } else {
                        elements = "align=\"left\" " + elements;
                    }
                }
            }
            if (needDiv) {
                out.append("<div ").append(elements).append(">");
            }
            withinDiv(context, out, text, i, next, option);
            if (needDiv) {
                out.append("</div>");
            }
        }
    }

    private static void withinDiv(Context context, StringBuilder out, Spanned text,
                                  int start, int end, int option) {
        int next;
        for (int i = start; i < end; i = next) {
            next = text.nextSpanTransition(i, end, QuoteSpan.class);
            QuoteSpan[] quotes = text.getSpans(i, next, QuoteSpan.class);
            for (QuoteSpan quote : quotes) {
                out.append("<blockquote>");
            }
            withinBlockquote(context, out, text, i, next, option);
            for (QuoteSpan quote : quotes) {
                out.append("</blockquote>\n");
            }
        }
    }

    private static String getTextDirection(Spanned text, int start, int end) {
        // FIXME not supported
        int paraDir = Layout.DIR_LEFT_TO_RIGHT;
//        final int len = end - start;
//        final byte[] levels = ArrayUtils.newUnpaddedByteArray(len);
//        final char[] buffer = TextUtils.obtain(len);
//        TextUtils.getChars(text, start, end, buffer, 0);
//        int paraDir = AndroidBidi.bidi(Layout.DIR_REQUEST_DEFAULT_LTR, buffer, levels, len,
//                false /* no extra */);
        switch (paraDir) {
            case Layout.DIR_RIGHT_TO_LEFT:
                return " dir=\"rtl\"";
            case Layout.DIR_LEFT_TO_RIGHT:
            default:
                return " dir=\"ltr\"";
        }
    }

    private static String getTextStyles(Spanned text, int start, int end,
                                        boolean forceNoVerticalMargin, boolean includeTextAlign) {
        String margin = null;
        String textAlign = null;
        if (forceNoVerticalMargin) {
            margin = "margin-top:0; margin-bottom:0;";
        }
        if (includeTextAlign) {
            final AlignmentSpan[] alignmentSpans = text.getSpans(start, end, AlignmentSpan.class);
            // Only use the last AlignmentSpan with flag SPAN_PARAGRAPH
            for (int i = alignmentSpans.length - 1; i >= 0; i--) {
                AlignmentSpan s = alignmentSpans[i];
                if ((text.getSpanFlags(s) & Spanned.SPAN_PARAGRAPH) == Spanned.SPAN_PARAGRAPH) {
                    final Layout.Alignment alignment = s.getAlignment();
                    if (alignment == Layout.Alignment.ALIGN_NORMAL) {
                        textAlign = "text-align:start;";
                    } else if (alignment == Layout.Alignment.ALIGN_CENTER) {
                        textAlign = "text-align:center;";
                    } else if (alignment == Layout.Alignment.ALIGN_OPPOSITE) {
                        textAlign = "text-align:end;";
                    }
                    break;
                }
            }
        }
        if (margin == null && textAlign == null) {
            return "";
        }
        final StringBuilder style = new StringBuilder(" style=\"");
        if (margin != null && textAlign != null) {
            style.append(margin).append(" ").append(textAlign);
        } else if (margin != null) {
            style.append(margin);
        } else if (textAlign != null) {
            style.append(textAlign);
        }
        return style.append("\"").toString();
    }

    private static void withinBlockquote(Context context, StringBuilder out, Spanned text,
                                         int start, int end, int option) {
        if ((option & TO_HTML_PARAGRAPH_FLAG) == TO_HTML_PARAGRAPH_LINES_CONSECUTIVE) {
            withinBlockquoteConsecutive(context, out, text, start, end);
        } else {
            withinBlockquoteIndividual(context, out, text, start, end);
        }
    }

    private static void withinBlockquoteIndividual(Context context, StringBuilder out, Spanned text,
                                                   int start, int end) {
        boolean isInList = false;
        int next;
        for (int i = start; i <= end; i = next) {
            next = TextUtils.indexOf(text, '\n', i, end);
            if (next < 0) {
                next = end;
            }
            if (next == i) {
                if (isInList) {
                    // Current paragraph is no longer a list item; close the previously opened list
                    isInList = false;
                    out.append("</ul>\n");
                }
                out.append("<br>\n");
            } else {
                boolean isListItem = false;
                ParagraphStyle[] paragraphStyles = text.getSpans(i, next, ParagraphStyle.class);
                for (ParagraphStyle paragraphStyle : paragraphStyles) {
                    final int spanFlags = text.getSpanFlags(paragraphStyle);
                    if ((spanFlags & Spanned.SPAN_PARAGRAPH) == Spanned.SPAN_PARAGRAPH
                            && paragraphStyle instanceof BulletSpan) {
                        isListItem = true;
                        break;
                    }
                }
                if (isListItem && !isInList) {
                    // Current paragraph is the first item in a list
                    isInList = true;
                    out.append("<ul")
                       .append(getTextStyles(text, i, next, true, false))
                       .append(">\n");
                }
                if (isInList && !isListItem) {
                    // Current paragraph is no longer a list item; close the previously opened list
                    isInList = false;
                    out.append("</ul>\n");
                }
                String tagType = isListItem ? "li" : "p";
                out.append("<").append(tagType)
                   .append(getTextDirection(text, i, next))
                   .append(getTextStyles(text, i, next, !isListItem, true))
                   .append(">");
                withinParagraph(context, out, text, i, next);
                out.append("</");
                out.append(tagType);
                out.append(">\n");
                if (next == end && isInList) {
                    isInList = false;
                    out.append("</ul>\n");
                }
            }
            next++;
        }
    }

    private static void withinBlockquoteConsecutive(Context context, StringBuilder out, Spanned text,
                                                    int start, int end) {
        out.append("<p").append(getTextDirection(text, start, end)).append(">");
        int next;
        for (int i = start; i < end; i = next) {
            next = TextUtils.indexOf(text, '\n', i, end);
            if (next < 0) {
                next = end;
            }
            int nl = 0;
            while (next < end && text.charAt(next) == '\n') {
                nl++;
                next++;
            }
            withinParagraph(context, out, text, i, next - nl);
            if (nl == 1) {
                out.append("<br>\n");
            } else {
                for (int j = 2; j < nl; j++) {
                    out.append("<br>");
                }
                if (next != end) {
                    /* Paragraph should be closed and reopened */
                    out.append("</p>\n");
                    out.append("<p").append(getTextDirection(text, start, end)).append(">");
                }
            }
        }
        out.append("</p>\n");
    }

    private static void withinParagraph(Context context, StringBuilder out, Spanned text, int start, int end) {
        int next;
        for (int i = start; i < end; i = next) {
            next = text.nextSpanTransition(i, end, CharacterStyle.class);
            CharacterStyle[] styles = text.getSpans(i, next, CharacterStyle.class);
            for (CharacterStyle style : styles) {
                if (style instanceof StyleSpan) {
                    int s = ((StyleSpan) style).getStyle();
                    if ((s & Typeface.BOLD) != 0) {
                        out.append("<b>");
                    }
                    if ((s & Typeface.ITALIC) != 0) {
                        out.append("<i>");
                    }
                }
                if (style instanceof TypefaceSpan) {
                    String s = ((TypefaceSpan) style).getFamily();
                    if ("monospace".equals(s)) {
                        out.append("<tt>");
                    }
                }
                if (style instanceof SuperscriptSpan) {
                    out.append("<sup>");
                }
                if (style instanceof SubscriptSpan) {
                    out.append("<sub>");
                }
                if (style instanceof UnderlineSpan) {
                    out.append("<u>");
                }
                if (style instanceof StrikethroughSpan) {
                    out.append("<span style=\"text-decoration:line-through;\">");
                }
                if (style instanceof URLSpan) {
                    out.append("<a href=\"");
                    out.append(((URLSpan) style).getURL());
                    out.append("\">");
                }
                if (style instanceof ImageSpan) {
                    out.append("<img src=\"");
                    out.append(((ImageSpan) style).getSource());
                    out.append("\">");
                    // Don't output the dummy character underlying the image.
                    i = next;
                }
                if (style instanceof AbsoluteSizeSpan) {
                    AbsoluteSizeSpan s = ((AbsoluteSizeSpan) style);
                    float sizeDip = s.getSize();
                    if (!s.getDip()) {
                        sizeDip /= context.getResources().getDisplayMetrics().density;
                    }
                    // px in CSS is the equivalance of dip in Android
                    out.append(String.format("<span style=\"font-size:%.0fpx\";>", sizeDip));
                }
                if (style instanceof RelativeSizeSpan) {
                    float sizeEm = ((RelativeSizeSpan) style).getSizeChange();
                    out.append(String.format("<span style=\"font-size:%.2fem;\">", sizeEm));
                }
                if (style instanceof ForegroundColorSpan) {
                    int color = ((ForegroundColorSpan) style).getForegroundColor();
                    out.append(String.format("<span style=\"color:#%06X;\">", 0xFFFFFF & color));
                }
                if (style instanceof BackgroundColorSpan) {
                    int color = ((BackgroundColorSpan) style).getBackgroundColor();
                    out.append(String.format("<span style=\"background-color:#%06X;\">",
                            0xFFFFFF & color));
                }
            }
            withinStyle(out, text, i, next);
            for (int j = styles.length - 1; j >= 0; j--) {
                if (styles[j] instanceof BackgroundColorSpan) {
                    out.append("</span>");
                }
                if (styles[j] instanceof ForegroundColorSpan) {
                    out.append("</span>");
                }
                if (styles[j] instanceof RelativeSizeSpan) {
                    out.append("</span>");
                }
                if (styles[j] instanceof AbsoluteSizeSpan) {
                    out.append("</span>");
                }
                if (styles[j] instanceof URLSpan) {
                    out.append("</a>");
                }
                if (styles[j] instanceof StrikethroughSpan) {
                    out.append("</span>");
                }
                if (styles[j] instanceof UnderlineSpan) {
                    out.append("</u>");
                }
                if (styles[j] instanceof SubscriptSpan) {
                    out.append("</sub>");
                }
                if (styles[j] instanceof SuperscriptSpan) {
                    out.append("</sup>");
                }
                if (styles[j] instanceof TypefaceSpan) {
                    String s = ((TypefaceSpan) styles[j]).getFamily();
                    if (s.equals("monospace")) {
                        out.append("</tt>");
                    }
                }
                if (styles[j] instanceof StyleSpan) {
                    int s = ((StyleSpan) styles[j]).getStyle();
                    if ((s & Typeface.BOLD) != 0) {
                        out.append("</b>");
                    }
                    if ((s & Typeface.ITALIC) != 0) {
                        out.append("</i>");
                    }
                }
            }
        }
    }

    private static void withinStyle(StringBuilder out, CharSequence text,
                                    int start, int end) {
        for (int i = start; i < end; i++) {
            char c = text.charAt(i);
            if (c == '<') {
                out.append("&lt;");
            } else if (c == '>') {
                out.append("&gt;");
            } else if (c == '&') {
                out.append("&amp;");
            } else if (c >= 0xD800 && c <= 0xDFFF) {
                if (c < 0xDC00 && i + 1 < end) {
                    char d = text.charAt(i + 1);
                    if (d >= 0xDC00 && d <= 0xDFFF) {
                        i++;
                        int codepoint = 0x010000 | (int) c - 0xD800 << 10 | (int) d - 0xDC00;
                        out.append("&#").append(codepoint).append(";");
                    }
                }
            } else if (c > 0x7E || c < ' ') {
                out.append("&#").append((int) c).append(";");
            } else if (c == ' ') {
                while (i + 1 < end && text.charAt(i + 1) == ' ') {
                    out.append("&nbsp;");
                    i++;
                }
                out.append(' ');
            } else {
                out.append(c);
            }
        }
    }
}
