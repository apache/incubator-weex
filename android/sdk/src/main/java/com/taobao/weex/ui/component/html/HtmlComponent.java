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
package com.taobao.weex.ui.component.html;

import android.support.annotation.IntDef;
import android.support.annotation.StringDef;
import android.text.TextUtils;

import com.taobao.weex.utils.WXLogUtils;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/** Created by Bruce Too On 2019/1/7. At 17:20 */
public class HtmlComponent {
  /**
   * Use default css style to render table or other tags,make it more real one Need replace
   * "placeholder" with your real html tag info.
   *
   * @see {{@link #getHtmlTemplateForTable(String,String)}}
   */
  public static final String HTML_TEMPLATE =
      "<!DOCTYPE html>\n"
          + "<html>\n"
          + " <head> \n"
          + "  <style type=\"text/css\">\n"
          + "  .article-content table {\n"
          + "  width: 100%;\n"
          + "  font-size: 14px;\n"
          + "  text-align: center;\n"
          + "  border-collapse: collapse !important;\n"
          + "  border-top-left-radius: 0.16rem;\n"
          + "  border-top-right-radius: 0.16rem;\n"
          + "  border-bottom-right-radius: 0.16rem;\n"
          + "  border-bottom-left-radius: 0.16rem;\n"
          + "  border-radius: 0.16rem;\n"
          + "  border: 1px solid #ddd;\n"
          + "}\n"
          + ".article-content table img {\n"
          + "  margin: 0;\n"
          + "}\n"
          + ".article-content table,\n"
          + ".article-content td,\n"
          + ".article-content th,\n"
          + ".article-content tr {\n"
          + "  border: 1px solid #ddd;\n"
          + "  padding: .15rem .25rem;\n"
          + "}\n"
          + ".article-content table tr:first-child {\n"
          + "  border-radius: 6px 0 0 0;\n"
          + "}\n"
          + ".article-content table tr:nth-child(odd) {\n"
          + "  background: #f8f8f8;\n"
          + "}\n"
          + ".article-content table tr:nth-child(even) {\n"
          + "  background: #fff;\n"
          + "}\n"
          + ".article-content table tr:last-child {\n"
          + "  border-radius: 0 6px 0 0;\n"
          + "}\n"
          + ".article-content table tr:last-child td:first-child {\n"
          + "  border-radius: 0 0 0 6px;\n"
          + "}\n"
          + ".article-content table tr:last-child td:last-child {\n"
          + "  border-radius: 0 0 6px 0;\n"
          + "}\n"
          + "</style> \n"
          + " </head> \n"
          + " <body> \n"
          + "  <div class=\"article-content\">\n"
          + "    placeholder \n"
          + "  </div>  \n"
          + " </body>\n"
          + "</html>";

  /**
   * The regex of matching
   *
   * <table ....> ** </table>
   *
   * <table> ** </table>
   */
  private static final String TABLE_REGEX = "<(tag|TAG)\\s{0,1}.+?</(tag|TAG)>";
  /**
   * The regex of matching like <img alt="" src="1.jpg"/> <img alt="" src="1.jpg"></img> <img alt=""
   * src="1.jpg">
   */
  private static final String SINGLE_TAG_REGEX = "<(tag|TAG)(.*?)(/>|></(tag|TAG)>|>)";
  /**
   * The regex of matching like src="1.jpg" if match successfully,the final value will in group(3)
   */
  private static final String ATTRIBUTE_REGEX = "(key|KEY)=(\\\"|\\')(.*?)(\\\"|\\')";

  public static final String TABLE = "table";
  public static final String VIDEO = "video";
  public static final String IMAGE = "img";

  @StringDef({TABLE, VIDEO, IMAGE})
  public @interface TAG {}

  public static final int TEXT_VIEW = 1; // for normal tag can be formatted by text view
  public static final int TABLE_VIEW = 2; // for table tag
  public static final int VIDEO_VIEW = 3; // for video tag
  public static final int IMAGE_VIEW = 4; // for img tag

  @IntDef({TEXT_VIEW, TABLE_VIEW, VIDEO_VIEW, IMAGE_VIEW})
  public @interface ViewType {}

  @ViewType public int type;
  public String info;

  private static Map<String, Integer> sSupportTagMap = new HashMap<>();

  /**
   * All support tags parse need be mapped with extra native view,and register here for remarking
   */
  static {
    sSupportTagMap.put(TABLE, TABLE_VIEW);
    sSupportTagMap.put(IMAGE, IMAGE_VIEW);
  }

  private static HtmlComponent getComponent(@ViewType int type, String info) {
    HtmlComponent htmlComponent = new HtmlComponent();
    htmlComponent.type = type;
    htmlComponent.info = info;
    return htmlComponent;
  }

  private static Matcher matcher(String originHtml, @TAG String tagName) {
    Pattern pattern = Pattern.compile(regexByTagName(tagName));
    return pattern.matcher(originHtml);
  }

  private static String regexByTagName(String tagName) {
    String regex;
    if (tagName.endsWith(TABLE)) {
      regex = TABLE_REGEX.replace("tag", tagName).replace("TAG", tagName.toUpperCase());
    } else {
      regex = SINGLE_TAG_REGEX.replace("tag", tagName).replace("TAG", tagName.toUpperCase());
    }
    return regex;
  }

  private static ArrayList<HtmlComponent> getInnerComponents(
      String originHtml, @TAG String tagName) {
    ArrayList<HtmlComponent> htmlComponents = new ArrayList<>();
    // trim useless symbols in case failed regex
    originHtml = originHtml.replaceAll("[\\t\\n\\r]", "");
    Matcher tagMatcher = matcher(originHtml, tagName);
    String[] tagSplit = originHtml.split(regexByTagName(tagName));
    if (tagSplit.length == 1) {
      htmlComponents.add(HtmlComponent.getComponent(HtmlComponent.TEXT_VIEW, originHtml));
    } else {
      int index = 0;
      while (tagMatcher.find()) {
        htmlComponents.add(HtmlComponent.getComponent(HtmlComponent.TEXT_VIEW, tagSplit[index]));
        htmlComponents.add(
            HtmlComponent.getComponent(sSupportTagMap.get(tagName), tagMatcher.group()));
        WXLogUtils.e(
            "find " + tagName + " -> start:" + tagMatcher.start() + " end:" + tagMatcher.end());
        index++;
      }
      htmlComponents.add(HtmlComponent.getComponent(HtmlComponent.TEXT_VIEW, tagSplit[index]));
    }
    return htmlComponents;
  }

  /**
   * Wrap <table> tag with standard html than contains css style
   * make <table> looks more artistic
   * @param info <table> string
   * @return whole html template
   */
  public static String getHtmlTemplateForTable(String template, String info) {

    String temp;
    if (TextUtils.isEmpty(template) || !template.contains("placeholder")) {
      temp = HTML_TEMPLATE;
      WXLogUtils.e(
          "Custom template must contains 'placeholder' for table,See "
              + "HtmlComponent#HTML_TEMPLATE");
    }else {
        temp = template;
    }
    return temp.replace("placeholder", info);
  }

  /**
   * Pick up html tag's specified attribute's value,looks like: "src" from <img src="http://...." />
   *
   * @param keyName attribute's name
   * @param info specified string hold tags
   * @return the value
   */
  public static String getAttributeValue(String keyName, String info) {
    String regex = ATTRIBUTE_REGEX.replace("key", keyName).replace("KEY", keyName.toUpperCase());
    Matcher matcher = Pattern.compile(regex).matcher(info);
    if (matcher.find()) {
      // the third group with parenthesis
      return matcher.group(3);
    }
    return "";
  }

  /**
   * Parse html tags that need be implemented by android view, we need two steps: 1、custom tag info,
   * remark by {@link TAG} 2、custom native view, remarks by {@link ViewType}
   *
   * @param originHtml the formatted html
   * @param tagNames tags will support
   * @return the {@link HtmlComponent} list use by native view
   */
  public static List<HtmlComponent> parseTags(String originHtml, @TAG String... tagNames) {
    originHtml = originHtml.replaceAll("\\\\", "");
    CopyOnWriteArrayList<HtmlComponent> htmlComponents = new CopyOnWriteArrayList<>();
    // make sure, table tag must be the first one to be parsed, in case <table> can hold
    // any other tags inside.
    List<String> allTagNames = new ArrayList<>(Arrays.asList(tagNames));
    if (allTagNames.size() != 0 && allTagNames.contains(TABLE)) {
      allTagNames.remove(TABLE);
      allTagNames.add(0, TABLE);
    }
    for (String tagName : allTagNames) {
      // check rationality
      if (!sSupportTagMap.containsKey(tagName)) {
        throw new IllegalArgumentException(tagName + " tag is not supported now!");
      }
      if (htmlComponents.size() == 0) { // parse first tag. named <table>..
        htmlComponents.addAll(getInnerComponents(originHtml, tagName));
      } else {
        for (HtmlComponent htmlComponent : htmlComponents) {
          if (htmlComponent.type == TEXT_VIEW) { // only text-view type can be spitted again
            ArrayList<HtmlComponent> innerHtmlComponents =
                getInnerComponents(htmlComponent.info, tagName);
            if (innerHtmlComponents.size() == 1) { // don't find the tag in this text-view info
              // Do Nothing
            } else { // found & replace the old one, with separate multiple components
              int replaceIndex = htmlComponents.indexOf(htmlComponent);
              htmlComponents.remove(replaceIndex);
              htmlComponents.addAll(replaceIndex, innerHtmlComponents);
            }
          }
        }
      }
    }
    return htmlComponents;
  }
}
