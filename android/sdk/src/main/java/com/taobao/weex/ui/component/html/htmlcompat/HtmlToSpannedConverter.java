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

import android.content.Context;
import android.graphics.Color;
import android.graphics.Typeface;
import android.text.Editable;
import android.text.Layout;
import android.text.Spannable;
import android.text.SpannableStringBuilder;
import android.text.Spanned;
import android.text.TextUtils;
import android.text.style.AbsoluteSizeSpan;
import android.text.style.AlignmentSpan;
import android.text.style.BackgroundColorSpan;
import android.text.style.ForegroundColorSpan;
import android.text.style.ParagraphStyle;
import android.text.style.RelativeSizeSpan;
import android.text.style.StrikethroughSpan;
import android.text.style.StyleSpan;
import android.text.style.SubscriptSpan;
import android.text.style.SuperscriptSpan;
import android.text.style.TypefaceSpan;
import android.text.style.UnderlineSpan;

import com.taobao.weex.ui.component.html.spans.WxBulletSpan;
import com.taobao.weex.ui.component.html.spans.WxQuoteSpan;

import org.ccil.cowan.tagsoup.Parser;
import org.xml.sax.Attributes;
import org.xml.sax.ContentHandler;
import org.xml.sax.InputSource;
import org.xml.sax.Locator;
import org.xml.sax.SAXException;
import org.xml.sax.XMLReader;

import java.io.IOException;
import java.io.StringReader;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

class HtmlToSpannedConverter implements ContentHandler {
    private static final float[] HEADING_SIZES = {
            1.5f, 1.4f, 1.3f, 1.2f, 1.1f, 1f,
            };
    private final Context mContext;
    private String mSource;
    private final HtmlCompat.SpanCallback mSpanCallback;
    private XMLReader mReader;
    private SpannableStringBuilder mSpannableStringBuilder;
    private HtmlCompat.TagHandler mTagHandler;
    private int mFlags;
    private static Pattern sTextAlignPattern;
    private static Pattern sForegroundColorPattern;
    private static Pattern sBackgroundColorPattern;
    private static Pattern sTextDecorationPattern;
    private static Pattern sTextFontSizePattern;
    /**
     * Name-value mapping of HTML/CSS colors which have different values in {@link Color}.
     */
    private static final Map<String, Integer> sColorMap;

    static {
        sColorMap = new HashMap<>();
        sColorMap.put("aliceblue", 0xFFF0F8FF);
        sColorMap.put("antiquewhite", 0xFFFAEBD7);
        sColorMap.put("aqua", 0xFF00FFFF);
        sColorMap.put("aquamarine", 0xFF7FFFD4);
        sColorMap.put("azure", 0xFFF0FFFF);
        sColorMap.put("beige", 0xFFF5F5DC);
        sColorMap.put("bisque", 0xFFFFE4C4);
        sColorMap.put("black", 0xFF000000);
        sColorMap.put("blanchedalmond", 0xFFFFEBCD);
        sColorMap.put("blue", 0xFF0000FF);
        sColorMap.put("blueviolet", 0xFF8A2BE2);
        sColorMap.put("brown", 0xFFA52A2A);
        sColorMap.put("burlywood", 0xFFDEB887);
        sColorMap.put("cadetblue", 0xFF5F9EA0);
        sColorMap.put("chartreuse", 0xFF7FFF00);
        sColorMap.put("chocolate", 0xFFD2691E);
        sColorMap.put("coral", 0xFFFF7F50);
        sColorMap.put("cornflowerblue", 0xFF6495ED);
        sColorMap.put("cornsilk", 0xFFFFF8DC);
        sColorMap.put("crimson", 0xFFDC143C);
        sColorMap.put("cyan", 0xFF00FFFF);
        sColorMap.put("darkblue", 0xFF00008B);
        sColorMap.put("darkcyan", 0xFF008B8B);
        sColorMap.put("darkgoldenrod", 0xFFB8860B);
        sColorMap.put("darkgray", 0xFFA9A9A9);
        sColorMap.put("darkgrey", 0xFFA9A9A9);
        sColorMap.put("darkgreen", 0xFF006400);
        sColorMap.put("darkkhaki", 0xFFBDB76B);
        sColorMap.put("darkmagenta", 0xFF8B008B);
        sColorMap.put("darkolivegreen", 0xFF556B2F);
        sColorMap.put("darkorange", 0xFFFF8C00);
        sColorMap.put("darkorchid", 0xFF9932CC);
        sColorMap.put("darkred", 0xFF8B0000);
        sColorMap.put("darksalmon", 0xFFE9967A);
        sColorMap.put("darkseagreen", 0xFF8FBC8F);
        sColorMap.put("darkslateblue", 0xFF483D8B);
        sColorMap.put("darkslategray", 0xFF2F4F4F);
        sColorMap.put("darkslategrey", 0xFF2F4F4F);
        sColorMap.put("darkturquoise", 0xFF00CED1);
        sColorMap.put("darkviolet", 0xFF9400D3);
        sColorMap.put("deeppink", 0xFFFF1493);
        sColorMap.put("deepskyblue", 0xFF00BFFF);
        sColorMap.put("dimgray", 0xFF696969);
        sColorMap.put("dimgrey", 0xFF696969);
        sColorMap.put("dodgerblue", 0xFF1E90FF);
        sColorMap.put("firebrick", 0xFFB22222);
        sColorMap.put("floralwhite", 0xFFFFFAF0);
        sColorMap.put("forestgreen", 0xFF228B22);
        sColorMap.put("fuchsia", 0xFFFF00FF);
        sColorMap.put("gainsboro", 0xFFDCDCDC);
        sColorMap.put("ghostwhite", 0xFFF8F8FF);
        sColorMap.put("gold", 0xFFFFD700);
        sColorMap.put("goldenrod", 0xFFDAA520);
        sColorMap.put("gray", 0xFF808080);
        sColorMap.put("grey", 0xFF808080);
        sColorMap.put("green", 0xFF008000);
        sColorMap.put("greenyellow", 0xFFADFF2F);
        sColorMap.put("honeydew", 0xFFF0FFF0);
        sColorMap.put("hotpink", 0xFFFF69B4);
        sColorMap.put("indianred ", 0xFFCD5C5C);
        sColorMap.put("indigo  ", 0xFF4B0082);
        sColorMap.put("ivory", 0xFFFFFFF0);
        sColorMap.put("khaki", 0xFFF0E68C);
        sColorMap.put("lavender", 0xFFE6E6FA);
        sColorMap.put("lavenderblush", 0xFFFFF0F5);
        sColorMap.put("lawngreen", 0xFF7CFC00);
        sColorMap.put("lemonchiffon", 0xFFFFFACD);
        sColorMap.put("lightblue", 0xFFADD8E6);
        sColorMap.put("lightcoral", 0xFFF08080);
        sColorMap.put("lightcyan", 0xFFE0FFFF);
        sColorMap.put("lightgoldenrodyellow", 0xFFFAFAD2);
        sColorMap.put("lightgray", 0xFFD3D3D3);
        sColorMap.put("lightgrey", 0xFFD3D3D3);
        sColorMap.put("lightgreen", 0xFF90EE90);
        sColorMap.put("lightpink", 0xFFFFB6C1);
        sColorMap.put("lightsalmon", 0xFFFFA07A);
        sColorMap.put("lightseagreen", 0xFF20B2AA);
        sColorMap.put("lightskyblue", 0xFF87CEFA);
        sColorMap.put("lightslategray", 0xFF778899);
        sColorMap.put("lightslategrey", 0xFF778899);
        sColorMap.put("lightsteelblue", 0xFFB0C4DE);
        sColorMap.put("lightyellow", 0xFFFFFFE0);
        sColorMap.put("lime", 0xFF00FF00);
        sColorMap.put("limegreen", 0xFF32CD32);
        sColorMap.put("linen", 0xFFFAF0E6);
        sColorMap.put("magenta", 0xFFFF00FF);
        sColorMap.put("maroon", 0xFF800000);
        sColorMap.put("mediumaquamarine", 0xFF66CDAA);
        sColorMap.put("mediumblue", 0xFF0000CD);
        sColorMap.put("mediumorchid", 0xFFBA55D3);
        sColorMap.put("mediumpurple", 0xFF9370DB);
        sColorMap.put("mediumseagreen", 0xFF3CB371);
        sColorMap.put("mediumslateblue", 0xFF7B68EE);
        sColorMap.put("mediumspringgreen", 0xFF00FA9A);
        sColorMap.put("mediumturquoise", 0xFF48D1CC);
        sColorMap.put("mediumvioletred", 0xFFC71585);
        sColorMap.put("midnightblue", 0xFF191970);
        sColorMap.put("mintcream", 0xFFF5FFFA);
        sColorMap.put("mistyrose", 0xFFFFE4E1);
        sColorMap.put("moccasin", 0xFFFFE4B5);
        sColorMap.put("navajowhite", 0xFFFFDEAD);
        sColorMap.put("navy", 0xFF000080);
        sColorMap.put("oldlace", 0xFFFDF5E6);
        sColorMap.put("olive", 0xFF808000);
        sColorMap.put("olivedrab", 0xFF6B8E23);
        sColorMap.put("orange", 0xFFFFA500);
        sColorMap.put("orangered", 0xFFFF4500);
        sColorMap.put("orchid", 0xFFDA70D6);
        sColorMap.put("palegoldenrod", 0xFFEEE8AA);
        sColorMap.put("palegreen", 0xFF98FB98);
        sColorMap.put("paleturquoise", 0xFFAFEEEE);
        sColorMap.put("palevioletred", 0xFFDB7093);
        sColorMap.put("papayawhip", 0xFFFFEFD5);
        sColorMap.put("peachpuff", 0xFFFFDAB9);
        sColorMap.put("peru", 0xFFCD853F);
        sColorMap.put("pink", 0xFFFFC0CB);
        sColorMap.put("plum", 0xFFDDA0DD);
        sColorMap.put("powderblue", 0xFFB0E0E6);
        sColorMap.put("purple", 0xFF800080);
        sColorMap.put("rebeccapurple", 0xFF663399);
        sColorMap.put("red", 0xFFFF0000);
        sColorMap.put("rosybrown", 0xFFBC8F8F);
        sColorMap.put("royalblue", 0xFF4169E1);
        sColorMap.put("saddlebrown", 0xFF8B4513);
        sColorMap.put("salmon", 0xFFFA8072);
        sColorMap.put("sandybrown", 0xFFF4A460);
        sColorMap.put("seagreen", 0xFF2E8B57);
        sColorMap.put("seashell", 0xFFFFF5EE);
        sColorMap.put("sienna", 0xFFA0522D);
        sColorMap.put("silver", 0xFFC0C0C0);
        sColorMap.put("skyblue", 0xFF87CEEB);
        sColorMap.put("slateblue", 0xFF6A5ACD);
        sColorMap.put("slategray", 0xFF708090);
        sColorMap.put("slategrey", 0xFF708090);
        sColorMap.put("snow", 0xFFFFFAFA);
        sColorMap.put("springgreen", 0xFF00FF7F);
        sColorMap.put("steelblue", 0xFF4682B4);
        sColorMap.put("tan", 0xFFD2B48C);
        sColorMap.put("teal", 0xFF008080);
        sColorMap.put("thistle", 0xFFD8BFD8);
        sColorMap.put("tomato", 0xFFFF6347);
        sColorMap.put("turquoise", 0xFF40E0D0);
        sColorMap.put("violet", 0xFFEE82EE);
        sColorMap.put("wheat", 0xFFF5DEB3);
        sColorMap.put("white", 0xFFFFFFFF);
        sColorMap.put("whitesmoke", 0xFFF5F5F5);
        sColorMap.put("yellow", 0xFFFFFF00);
        sColorMap.put("yellowgreen", 0xFF9ACD32);
    }

    private static Pattern getTextAlignPattern() {
        if (sTextAlignPattern == null) {
            sTextAlignPattern = Pattern.compile("(?:\\s+|\\A)text-align\\s*:\\s*(\\S*)\\b");
        }
        return sTextAlignPattern;
    }

    private static Pattern getForegroundColorPattern() {
        if (sForegroundColorPattern == null) {
            sForegroundColorPattern = Pattern.compile(
                    "(?:\\s+|\\A)color\\s*:\\s*(\\S*)\\b");
        }
        return sForegroundColorPattern;
    }

    private static Pattern getBackgroundColorPattern() {
        if (sBackgroundColorPattern == null) {
            sBackgroundColorPattern = Pattern.compile(
                    "(?:\\s+|\\A)background(?:-color)?\\s*:\\s*(\\S*)\\b");
        }
        return sBackgroundColorPattern;
    }

    private static Pattern getTextDecorationPattern() {
        if (sTextDecorationPattern == null) {
            sTextDecorationPattern = Pattern.compile(
                    "(?:\\s+|\\A)text-decoration\\s*:\\s*(\\S*)\\b");
        }
        return sTextDecorationPattern;
    }

    private static Pattern getFontSizePattern() {
        if (sTextFontSizePattern == null) {
            sTextFontSizePattern = Pattern.compile(
                    "(?:\\s+|\\A)font-size\\s*:\\s*(\\S*)\\b");
        }
        return sTextFontSizePattern;
    }

    HtmlToSpannedConverter(Context context, String source,
                           HtmlCompat.TagHandler tagHandler, HtmlCompat.SpanCallback spanCallback,
                           Parser parser, int flags) {
        mContext = context;
        mSource = source;
        mSpannableStringBuilder = new SpannableStringBuilder();
        mTagHandler = tagHandler;
        mSpanCallback = spanCallback;
        mReader = parser;
        mFlags = flags;
    }

    Spanned convert() {
        mReader.setContentHandler(this);
        try {
            mReader.parse(new InputSource(new StringReader(mSource)));
        } catch (IOException e) {
            // We are reading from a string. There should not be IO problems.
            throw new RuntimeException(e);
        } catch (SAXException e) {
            // TagSoup doesn't throw parse exceptions.
            throw new RuntimeException(e);
        }
        // Fix flags and range for paragraph-tagName markup.
        Object[] spans = mSpannableStringBuilder.getSpans(0, mSpannableStringBuilder.length(), ParagraphStyle.class);
        for (Object span : spans) {
            int start = mSpannableStringBuilder.getSpanStart(span);
            int end = mSpannableStringBuilder.getSpanEnd(span);
            // If the last line of the range is blank, back off by one.
            if (end - 2 >= 0) {
                if (mSpannableStringBuilder.charAt(end - 1) == '\n' &&
                        mSpannableStringBuilder.charAt(end - 2) == '\n') {
                    end--;
                }
            }
            if (end == start) {
                mSpannableStringBuilder.removeSpan(span);
            } else {
                mSpannableStringBuilder.setSpan(span, start, end, Spannable.SPAN_PARAGRAPH);
            }
        }
        return mSpannableStringBuilder;
    }

    private void handleStartTag(String tag, Attributes attributes) {
        if (tag.equalsIgnoreCase("br")) {
            // We don't need to handle this. TagSoup will ensure that there's a </br> for each <br>
            // so we can safely emit the linebreaks when we handle the close tag.
        } else if (tag.equalsIgnoreCase("p")) {
            startBlockElement(mSpannableStringBuilder, attributes, getMarginParagraph());
            startCssStyle(mSpannableStringBuilder, attributes);
        } else if (tag.equalsIgnoreCase("ul")) {
            startBlockElement(mSpannableStringBuilder, attributes, getMarginList());
        } else if (tag.equalsIgnoreCase("li")) {
            startLi(mSpannableStringBuilder, attributes);
        } else if (tag.equalsIgnoreCase("div")) {
            startBlockElement(mSpannableStringBuilder, attributes, getMarginDiv());
        } else if (tag.equalsIgnoreCase("span")) {
            startCssStyle(mSpannableStringBuilder, attributes);
        } else if (tag.equalsIgnoreCase("strong")) {
            start(mSpannableStringBuilder, new Bold());
        } else if (tag.equalsIgnoreCase("b")) {
            start(mSpannableStringBuilder, new Bold());
        } else if (tag.equalsIgnoreCase("em")) {
            start(mSpannableStringBuilder, new Italic());
        } else if (tag.equalsIgnoreCase("cite")) {
            start(mSpannableStringBuilder, new Italic());
        } else if (tag.equalsIgnoreCase("dfn")) {
            start(mSpannableStringBuilder, new Italic());
        } else if (tag.equalsIgnoreCase("i")) {
            start(mSpannableStringBuilder, new Italic());
        } else if (tag.equalsIgnoreCase("big")) {
            start(mSpannableStringBuilder, new Big());
        } else if (tag.equalsIgnoreCase("small")) {
            start(mSpannableStringBuilder, new Small());
        } else if (tag.equalsIgnoreCase("font")) {
            startFont(mSpannableStringBuilder, attributes);
        } else if (tag.equalsIgnoreCase("blockquote")) {
            startBlockquote(mSpannableStringBuilder, attributes);
        } else if (tag.equalsIgnoreCase("tt")) {
            start(mSpannableStringBuilder, new Monospace());
        } else if (tag.equalsIgnoreCase("a")) {
            startA(mSpannableStringBuilder, attributes);
        } else if (tag.equalsIgnoreCase("u")) {
            start(mSpannableStringBuilder, new Underline());
        } else if (tag.equalsIgnoreCase("del")) {
            start(mSpannableStringBuilder, new Strikethrough());
        } else if (tag.equalsIgnoreCase("s")) {
            start(mSpannableStringBuilder, new Strikethrough());
        } else if (tag.equalsIgnoreCase("strike")) {
            start(mSpannableStringBuilder, new Strikethrough());
        } else if (tag.equalsIgnoreCase("sup")) {
            start(mSpannableStringBuilder, new Super());
        } else if (tag.equalsIgnoreCase("sub")) {
            start(mSpannableStringBuilder, new Sub());
        } else if (tag.length() == 2 &&
                Character.toLowerCase(tag.charAt(0)) == 'h' &&
                tag.charAt(1) >= '1' && tag.charAt(1) <= '6') {
            startHeading(mSpannableStringBuilder, attributes, tag.charAt(1) - '1');
        }
    }

    private void handleEndTag(String tag) {
        if (tag.equalsIgnoreCase("br")) {
            handleBr(mSpannableStringBuilder);
        } else if (tag.equalsIgnoreCase("p")) {
            endCssStyle(tag, mSpannableStringBuilder);
            endBlockElement(tag, mSpannableStringBuilder);
        } else if (tag.equalsIgnoreCase("ul")) {
            endBlockElement(tag, mSpannableStringBuilder);
        } else if (tag.equalsIgnoreCase("li")) {
            endLi(tag, mSpannableStringBuilder);
        } else if (tag.equalsIgnoreCase("div")) {
            endBlockElement(tag, mSpannableStringBuilder);
        } else if (tag.equalsIgnoreCase("span")) {
            endCssStyle(tag, mSpannableStringBuilder);
        } else if (tag.equalsIgnoreCase("strong")) {
            end(tag, mSpannableStringBuilder, Bold.class, new StyleSpan(Typeface.BOLD));
        } else if (tag.equalsIgnoreCase("b")) {
            end(tag, mSpannableStringBuilder, Bold.class, new StyleSpan(Typeface.BOLD));
        } else if (tag.equalsIgnoreCase("em")) {
            end(tag, mSpannableStringBuilder, Italic.class, new StyleSpan(Typeface.ITALIC));
        } else if (tag.equalsIgnoreCase("cite")) {
            end(tag, mSpannableStringBuilder, Italic.class, new StyleSpan(Typeface.ITALIC));
        } else if (tag.equalsIgnoreCase("dfn")) {
            end(tag, mSpannableStringBuilder, Italic.class, new StyleSpan(Typeface.ITALIC));
        } else if (tag.equalsIgnoreCase("i")) {
            end(tag, mSpannableStringBuilder, Italic.class, new StyleSpan(Typeface.ITALIC));
        } else if (tag.equalsIgnoreCase("big")) {
            end(tag, mSpannableStringBuilder, Big.class, new RelativeSizeSpan(1.25f));
        } else if (tag.equalsIgnoreCase("small")) {
            end(tag, mSpannableStringBuilder, Small.class, new RelativeSizeSpan(0.8f));
        } else if (tag.equalsIgnoreCase("font")) {
            endFont(tag, mSpannableStringBuilder);
        } else if (tag.equalsIgnoreCase("blockquote")) {
            endBlockquote(tag, mSpannableStringBuilder);
        } else if (tag.equalsIgnoreCase("tt")) {
            end(tag, mSpannableStringBuilder, Monospace.class, new TypefaceSpan("monospace"));
        } else if (tag.equalsIgnoreCase("a")) {
            endA(tag, mSpannableStringBuilder);
        } else if (tag.equalsIgnoreCase("u")) {
            end(tag, mSpannableStringBuilder, Underline.class, new UnderlineSpan());
        } else if (tag.equalsIgnoreCase("del")) {
            end(tag, mSpannableStringBuilder, Strikethrough.class, new StrikethroughSpan());
        } else if (tag.equalsIgnoreCase("s")) {
            end(tag, mSpannableStringBuilder, Strikethrough.class, new StrikethroughSpan());
        } else if (tag.equalsIgnoreCase("strike")) {
            end(tag, mSpannableStringBuilder, Strikethrough.class, new StrikethroughSpan());
        } else if (tag.equalsIgnoreCase("sup")) {
            end(tag, mSpannableStringBuilder, Super.class, new SuperscriptSpan());
        } else if (tag.equalsIgnoreCase("sub")) {
            end(tag, mSpannableStringBuilder, Sub.class, new SubscriptSpan());
        } else if (tag.length() == 2 &&
                Character.toLowerCase(tag.charAt(0)) == 'h' &&
                tag.charAt(1) >= '1' && tag.charAt(1) <= '6') {
            endHeading(tag, mSpannableStringBuilder);
        }
    }

    private int getMarginParagraph() {
        return getMargin(HtmlCompat.FROM_HTML_SEPARATOR_LINE_BREAK_PARAGRAPH);
    }

    private int getMarginHeading() {
        return getMargin(HtmlCompat.FROM_HTML_SEPARATOR_LINE_BREAK_HEADING);
    }

    private int getMarginListItem() {
        return getMargin(HtmlCompat.FROM_HTML_SEPARATOR_LINE_BREAK_LIST_ITEM);
    }

    private int getMarginList() {
        return getMargin(HtmlCompat.FROM_HTML_SEPARATOR_LINE_BREAK_LIST);
    }

    private int getMarginDiv() {
        return getMargin(HtmlCompat.FROM_HTML_SEPARATOR_LINE_BREAK_DIV);
    }

    private int getMarginBlockquote() {
        return getMargin(HtmlCompat.FROM_HTML_SEPARATOR_LINE_BREAK_BLOCKQUOTE);
    }

    /**
     * Returns the minimum number of newline characters needed before and after a given block-level
     * element.
     *
     * @param flag the corresponding option flag defined in {@link HtmlCompat} of a block-level element
     */
    private int getMargin(int flag) {
        if ((flag & mFlags) != 0) {
            return 1;
        }
        return 2;
    }

    private void appendNewlines(Editable text, int minNewline) {
        final int len = text.length();
        if (len == 0) {
            return;
        }
        int existingNewlines = 0;
        for (int i = len - 1; i >= 0 && text.charAt(i) == '\n'; i--) {
            existingNewlines++;
        }
        for (int j = existingNewlines; j < minNewline; j++) {
            text.append("\n");
        }
    }

    private void startBlockElement(Editable text, Attributes attributes, int margin) {
        if (margin > 0) {
            appendNewlines(text, margin);
            start(text, new Newline(margin));
        }
        String style = attributes.getValue("", "style");
        if (style != null) {
            Matcher m = getTextAlignPattern().matcher(style);
            if (m.find()) {
                String alignment = m.group(1);
                if (alignment.equalsIgnoreCase("start")) {
                    start(text, new Alignment(Layout.Alignment.ALIGN_NORMAL));
                } else if (alignment.equalsIgnoreCase("center")) {
                    start(text, new Alignment(Layout.Alignment.ALIGN_CENTER));
                } else if (alignment.equalsIgnoreCase("end")) {
                    start(text, new Alignment(Layout.Alignment.ALIGN_OPPOSITE));
                }
            }
        }
    }

    private void endBlockElement(String tag, Editable text) {
        Newline n = getLast(text, Newline.class);
        if (n != null) {
            appendNewlines(text, n.mNumNewlines);
            text.removeSpan(n);
        }
        Alignment a = getLast(text, Alignment.class);
        if (a != null) {
            setSpanFromMark(tag, text, a, new AlignmentSpan.Standard(a.mAlignment));
        }
    }

    private void handleBr(Editable text) {
        text.append('\n');
    }

    private void startLi(Editable text, Attributes attributes) {
        startBlockElement(text, attributes, getMarginListItem());
        start(text, new Bullet());
        startCssStyle(text, attributes);
    }

    private void endLi(String tag, Editable text) {
        endCssStyle(tag, text);
        endBlockElement(tag, text);
        end(tag, text, Bullet.class, new WxBulletSpan());
    }

    private void startBlockquote(Editable text, Attributes attributes) {
        startBlockElement(text, attributes, getMarginBlockquote());
        start(text, new Blockquote());
    }

    private void endBlockquote(String tag, Editable text) {
        endBlockElement(tag, text);
        end(tag, text, Blockquote.class, new WxQuoteSpan());
    }

    private void startHeading(Editable text, Attributes attributes, int level) {
        startBlockElement(text, attributes, getMarginHeading());
        start(text, new Heading(level));
    }

    private void endHeading(String tag, Editable text) {
        // RelativeSizeSpan and StyleSpan are CharacterStyles
        // Their ranges should not include the newlines at the end
        Heading h = getLast(text, Heading.class);
        if (h != null) {
            setSpanFromMark(tag, text, h, new RelativeSizeSpan(HEADING_SIZES[h.mLevel]),
                    new StyleSpan(Typeface.BOLD));
        }
        endBlockElement(tag, text);
    }

    private <T> T getLast(Spanned text, Class<T> kind) {
        /*
         * This knows that the last returned object from getSpans()
         * will be the most recently added.
         */
        T[] objs = text.getSpans(0, text.length(), kind);
        if (objs.length == 0) {
            return null;
        } else {
            return objs[objs.length - 1];
        }
    }

    private void setSpanFromMark(String tag, Spannable text, Object mark, Object... spans) {
        int where = text.getSpanStart(mark);
        text.removeSpan(mark);
        int len = text.length();
        if (where != len) {
            for (Object span : spans) {
                if (mSpanCallback != null) {
                    span = mSpanCallback.onSpanCreated(tag, span);
                }
                text.setSpan(span, where, len, Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
            }
        }
    }

    private void start(Editable text, Object mark) {
        int len = text.length();
        text.setSpan(mark, len, len, Spannable.SPAN_INCLUSIVE_EXCLUSIVE);
    }

    private void end(String tag, Editable text, Class kind, Object repl) {
        Object obj = getLast(text, kind);
        if (obj != null) {
            if(repl instanceof WxQuoteSpan){
                setSpanFromMark(tag, text, obj, repl,new ForegroundColorSpan(((WxQuoteSpan) repl).getColor()));
            }else {
                setSpanFromMark(tag, text, obj, repl);
            }
        }
    }

    private void startCssStyle(Editable text, Attributes attributes) {
        String style = attributes.getValue("", "style");
        if (style != null) {
            Matcher m = getForegroundColorPattern().matcher(style);
            if (m.find()) {
                int c = getHtmlColor(m.group(1));
                if (c != -1) {
                    start(text, new Foreground(c | 0xFF000000));
                }
            }
            m = getBackgroundColorPattern().matcher(style);
            if (m.find()) {
                int c = getHtmlColor(m.group(1));
                if (c != -1) {
                    start(text, new Background(c | 0xFF000000));
                }
            }
            m = getTextDecorationPattern().matcher(style);
            if (m.find()) {
                String textDecoration = m.group(1);
                if (textDecoration.equalsIgnoreCase("line-through")) {
                    start(text, new Strikethrough());
                }
            }
            m = getFontSizePattern().matcher(style);
            if (m.find()) {
                String textSizeString = m.group(1);
                if (!TextUtils.isEmpty(textSizeString)) {
                    if (textSizeString.contains("px")) {
                        int textSize = Integer.valueOf(textSizeString.replaceAll("\\D+", ""));
                        textSize *= mContext.getResources().getDisplayMetrics().density;
                        start(text, new AbsoluteSize(textSize));
                    }
                    if (textSizeString.contains("em")) {
                        float textSize = Float.valueOf(textSizeString.replaceAll("\\D+", ""));
                        start(text, new RelativeSize(textSize));
                    }
                }
            }
        }
    }

    private void endCssStyle(String tag, Editable text) {
        Strikethrough s = getLast(text, Strikethrough.class);
        if (s != null) {
            setSpanFromMark(tag, text, s, new StrikethroughSpan());
        }
        Background b = getLast(text, Background.class);
        if (b != null) {
            setSpanFromMark(tag, text, b, new BackgroundColorSpan(b.mBackgroundColor));
        }
        Foreground f = getLast(text, Foreground.class);
        if (f != null) {
            setSpanFromMark(tag, text, f, new ForegroundColorSpan(f.mForegroundColor));
        }
        AbsoluteSize a = getLast(text, AbsoluteSize.class);
        if (a != null) {
            setSpanFromMark(tag, text, a, new AbsoluteSizeSpan(a.getTextSize()));
        }
        RelativeSize r = getLast(text, RelativeSize.class);
        if (r != null) {
            setSpanFromMark(tag, text, r, new RelativeSizeSpan(r.getTextProportion()));
        }
    }

    private void startFont(Editable text, Attributes attributes) {
        String color = attributes.getValue("", "color");
        String face = attributes.getValue("", "face");
        if (!TextUtils.isEmpty(color)) {
            int c = getHtmlColor(color);
            if (c != -1) {
                start(text, new Foreground(c | 0xFF000000));
            }
        }
        if (!TextUtils.isEmpty(face)) {
            start(text, new Font(face));
        }
    }

    private void endFont(String tag, Editable text) {
        Font font = getLast(text, Font.class);
        if (font != null) {
            setSpanFromMark(tag, text, font, new TypefaceSpan(font.mFace));
        }
        Foreground foreground = getLast(text, Foreground.class);
        if (foreground != null) {
            setSpanFromMark(tag, text, foreground,
                    new ForegroundColorSpan(foreground.mForegroundColor));
        }
    }

    private void startA(Editable text, Attributes attributes) {
        String href = attributes.getValue("", "href");
        start(text, new Href(href));
    }

    private void endA(String tag, Editable text) {
        Href h = getLast(text, Href.class);
        if (h != null) {
            if (h.mHref != null) {
                setSpanFromMark(tag, text, h, new HtmlCompat.DefensiveURLSpan((h.mHref)));
            }
        }
    }

    private int getHtmlColor(String color) {
        if ((mFlags & HtmlCompat.FROM_HTML_OPTION_USE_CSS_COLORS)
                == HtmlCompat.FROM_HTML_OPTION_USE_CSS_COLORS) {
            Integer i = sColorMap.get(color.toLowerCase(Locale.US));
            if (i != null) {
                return i;
            }
        }
        return ColorUtils.getHtmlColor(color);
    }

    public void setDocumentLocator(Locator locator) {
    }

    public void startDocument() throws SAXException {
    }

    public void endDocument() throws SAXException {
    }

    public void startPrefixMapping(String prefix, String uri) throws SAXException {
    }

    public void endPrefixMapping(String prefix) throws SAXException {
    }

    public void startElement(String uri, String localName, String qName, Attributes attributes)
            throws SAXException {
        handleStartTag(localName, attributes);
    }

    public void endElement(String uri, String localName, String qName) throws SAXException {
        handleEndTag(localName);
    }

    public void characters(char ch[], int start, int length) throws SAXException {
        StringBuilder sb = new StringBuilder();
        /*
         * Ignore whitespace that immediately follows other whitespace;
         * newlines count as spaces.
         */
        for (int i = 0; i < length; i++) {
            char c = ch[i + start];
            if (c == ' ' || c == '\n') {
                char pred;
                int len = sb.length();
                if (len == 0) {
                    len = mSpannableStringBuilder.length();
                    if (len == 0) {
                        pred = '\n';
                    } else {
                        pred = mSpannableStringBuilder.charAt(len - 1);
                    }
                } else {
                    pred = sb.charAt(len - 1);
                }
                if (pred != ' ' && pred != '\n') {
                    sb.append(' ');
                }
            } else {
                sb.append(c);
            }
        }
        mSpannableStringBuilder.append(sb);
    }

    public void ignorableWhitespace(char ch[], int start, int length) throws SAXException {
    }

    public void processingInstruction(String target, String data) throws SAXException {
    }

    public void skippedEntity(String name) throws SAXException {
    }

    private static class Bold {
    }

    private static class Italic {
    }

    private static class Underline {
    }

    private static class Strikethrough {
    }

    private static class Big {
    }

    private static class Small {
    }

    private static class Monospace {
    }

    private static class Blockquote {
    }

    private static class Super {
    }

    private static class Sub {
    }

    private static class Bullet {
    }

    private static class Font {
        String mFace;

        Font(String face) {
            mFace = face;
        }
    }

    private static class Href {
        String mHref;

        Href(String href) {
            mHref = href;
        }
    }

    private static class Foreground {
        private int mForegroundColor;

        Foreground(int foregroundColor) {
            mForegroundColor = foregroundColor;
        }
    }

    private static class Background {
        private int mBackgroundColor;

        Background(int backgroundColor) {
            mBackgroundColor = backgroundColor;
        }
    }

    private static class Heading {
        private int mLevel;

        Heading(int level) {
            mLevel = level;
        }
    }

    private static class AbsoluteSize {
        private int mTextSize;

        AbsoluteSize(int textSize) {
            mTextSize = textSize;
        }

        public int getTextSize() {
            return mTextSize;
        }
    }

    private static class RelativeSize {
        private float mTextProportion;

        RelativeSize(float textProportion) {
            mTextProportion = textProportion;
        }

        public float getTextProportion() {
            return mTextProportion;
        }
    }

    private static class Newline {
        private int mNumNewlines;

        Newline(int numNewlines) {
            mNumNewlines = numNewlines;
        }
    }

    private static class Alignment {
        private Layout.Alignment mAlignment;

        Alignment(Layout.Alignment alignment) {
            mAlignment = alignment;
        }
    }

}
