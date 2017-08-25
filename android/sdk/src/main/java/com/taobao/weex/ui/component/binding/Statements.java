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
package com.taobao.weex.ui.component.binding;

import android.graphics.Color;
import android.support.v4.util.ArrayMap;
import android.text.TextUtils;

import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import com.taobao.weex.dom.WXAttr;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.dom.WXEvent;
import com.taobao.weex.dom.binding.BindingUtils;
import com.taobao.weex.dom.binding.WXStatement;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXComponentFactory;
import com.taobao.weex.ui.component.WXVContainer;
import com.taobao.weex.utils.WXLogUtils;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.Stack;

/**
 * Created by jianbai.gbj on 2017/8/17.
 * simple statement execute, render component for template list
 */
public class Statements {
    /**
     * recursive copy component,
     * */
    public static WXComponent copyComponentTree(WXComponent component){
        WXComponent copy =  copyComponentTree(component, component.getParent());
        return copy;
    }

    /**
     * recursive copy component,
     * */
    private static final WXComponent copyComponentTree(WXComponent source, WXVContainer parent){
        WXDomObject node = (WXDomObject) source.getDomObject();
        WXComponent component = WXComponentFactory.newInstance(source.getInstance(), node, parent);
        if(source instanceof WXVContainer){
            WXVContainer container = (WXVContainer) source;
            WXVContainer childParent = (WXVContainer) component;
            WXDomObject childParentNode = (WXDomObject) childParent.getDomObject();
            int count = container.getChildCount();
            for (int i = 0; i < count; ++i) {
                WXComponent child = container.getChild(i);
                if (child != null) {
                    WXComponent targetChild = copyComponentTree(child,  childParent);
                    childParent.addChild(targetChild);
                    childParentNode.add((WXDomObject) targetChild.getDomObject(), -1);
                }
            }
        }
        return  component;
    }

    /**
     *  @param component component with v-for statement, v-if statement and bind attrs
     *  @param map   execute context
     *  render component in context, the function do the following  work.
     *  execute component's v-for statement, v-if statement in context,
     *  and rebuild component's tree with the statement, v-for reuse component execute by pre render.
     *  if executed, component will be removed, don't remove, just mark it waste;
     *  may be next render it can be used.
     *  after statement has executed, render component's binding attrs in context and bind it to component.
     * */
    public static final void doRender(WXComponent component, Map map){
        try{
            Stack context = new Stack();
            context.push(map);
            doRender(component, context);
            context = null;
        }catch (Exception e){
            WXLogUtils.e("WeexStatementRender", e);
        }
    }


    /**
     *  @param component component with v-for statement, v-if statement and bind attrs
     *  @param context   execute context
     *  render component in context, the function do the following  work.
     *  execute component's v-for statement, v-if statement in context,
     *  and rebuild component's tree with the statement, v-for reuse component execute by pre render.
     *  if executed, component will be removed, don't remove, just mark it waste;
     *  may be next render it can be used.
     *  after statement has executed, render component's binding attrs in context and bind it to component.
     * */
    static final int doRender(WXComponent component, Stack context){
        WXVContainer parent = component.getParent();
        WXDomObject domObject = (WXDomObject) component.getDomObject();
        WXAttr attrs = domObject.getAttrs();
        WXStatement statement =  attrs.getStatement();
        if(statement != null){
            WXDomObject parentDomObject = (WXDomObject) parent.getDomObject();
            JSONObject vfor = (JSONObject) statement.get(WXStatement.WX_FOR);
            String vif = (String) statement.get(WXStatement.WX_IF);
            int renderIndex = parent.indexOf(component);
            // execute v-for content
            if(vfor != null){
                String listKey = vfor.getString(WXStatement.WX_FOR_LIST);
                String indexKey = vfor.getString(WXStatement.WX_FOR_INDEX);
                String itemKey = vfor.getString(WXStatement.WX_FOR_ITEM);
                Object data = ELUtils.getValue(context, listKey);
                if(data instanceof List){
                    List list = (List) data;
                    Map<String, Object> loop = new HashMap<>();
                    context.push(loop);
                    for(int i=0; i<list.size(); i++){
                        loop.put(indexKey, i);
                        loop.put(itemKey, list.get(i));
                        if(!TextUtils.isEmpty(vif)){
                            if(!isIfTrue(vif, context)){
                                continue;
                            }
                        }
                        //find resuable renderNode
                        WXComponent renderNode = null;
                        if(renderIndex < parent.getChildCount()){
                            renderNode = parent.getChild(renderIndex);
                            //check is same statment, if true, it is usabled.
                            if(!isCreateFromNodeStatement(renderNode, component)){
                                renderNode = null;
                            }
                        }
                        //none resuable render node, create node, add to parent, but clear node's statement
                        if(renderNode == null){
                            renderNode = copyComponentTree(component, parent);
                            WXDomObject renderNodeDomObject = (WXDomObject) renderNode.getDomObject();
                            renderNodeDomObject.getAttrs().setStatement(null); // clear node's statement
                            parent.addChild(renderNode, renderIndex);
                            parent.createChildViewAt(renderIndex);
                            parentDomObject.add(renderNodeDomObject, renderIndex);
                            renderNode.applyLayoutAndEvent(component);
                            //FIXME delete
                            renderNode.getHostView().setBackgroundColor(Color.RED);
                        }
                        doRenderBindingAttrs(component, domObject, context);
                        doRenderChildNode(component, context);
                        renderIndex++;
                    }
                    context.pop();
                }
                //after v-for execute, remove component created pre v-for.
                for(;renderIndex<parent.getChildCount(); renderIndex++){
                    WXComponent wasteNode = parent.getChild(renderIndex);
                    if(!isCreateFromNodeStatement(wasteNode, component)){
                        break;
                    }
                    wasteNode.setWaste(true);
                }
                return renderIndex - parent.indexOf(component);
            }

            //execute v-if context
            if(vif != null){
                if(!isIfTrue(vif, context)){
                    component.setWaste(true);
                    return 1;
                }
            }
        }
        doRenderBindingAttrs(component, domObject, context);
        doRenderChildNode(component, context);
        return  1;
    }

    /**
     * bind attrs and doRender next
     * */
    private static void doRenderChildNode(WXComponent component, Stack context){
        if(component instanceof WXVContainer){
            WXVContainer container = (WXVContainer) component;
            for(int k=0; k<container.getChildCount();){
                WXComponent next = container.getChild(k);
                k += doRender(next, context);
            }
        }
    }


    /**
     * check whether render node is create from component node statement.
     * */
    private static boolean isCreateFromNodeStatement(WXComponent renderNode, WXComponent component){
        return (renderNode.getRef() != null && renderNode.getRef().equals(component.getRef()));
    }


    /**
     * render dynamic binding attrs and bind them to component node.
     * */
    private static void doRenderBindingAttrs(WXComponent component, WXDomObject domObject, Stack context){
        component.setWaste(false);
        if(domObject.getAttrs() != null
                && domObject.getAttrs().getBindingAttrs() != null
                && domObject.getAttrs().getBindingAttrs().size() > 0){
            ArrayMap<String, Object> bindAttrs = domObject.getAttrs().getBindingAttrs();
            Map<String, Object> dynamic =  getDynamicAttrs(bindAttrs, context);
            domObject.updateAttr(dynamic);
            component.updateProperties(dynamic);
        }
        WXEvent event = domObject.getEvents();
        if(event == null || event.getEventBindingArgs() == null){
            return;
        }
        Set<Map.Entry<String, Object>> eventBindArgsEntrySet = event.getEventBindingArgs().entrySet();
        for(Map.Entry<String, Object> eventBindArgsEntry : eventBindArgsEntrySet){
             List<Object> values = getBindingEventArgsValue(context, eventBindArgsEntry.getValue());
             if(values != null){
                 event.putEventBindingArgsValue(eventBindArgsEntry.getKey(), values);
             }
        }
    }


    /**
     * @param  bindAttrs  none null,
     * @param  context  context
     * return binding attrs rended value in context
     * */
    public static Map<String, Object> getDynamicAttrs(ArrayMap bindAttrs, Stack context){
        Set<Map.Entry<String, Object>> entrySet = bindAttrs.entrySet();
        Map<String, Object> dynamic = new HashMap<>();
        for(Map.Entry<String, Object> entry : entrySet){
            Object binding = entry.getValue();
            //FIXME remove
            String key = entry.getKey();
            if(key.equals("text")){
                key = "value";
            }
            if(entry.getValue() instanceof  JSONObject && ((JSONObject) binding).containsKey(BindingUtils.BINDING)){
                Object bindingValue = getBindingValue(((JSONObject) binding).getString(BindingUtils.BINDING), context);
                dynamic.put(key, bindingValue);
            }else if(binding instanceof JSONArray){
                JSONArray array = (JSONArray) binding;
                StringBuilder builder = new StringBuilder();
                for(int i=0; i<array.size(); i++){
                    Object value = array.get(i);
                    if(value instanceof  CharSequence){
                        builder.append(value);
                        continue;
                    }
                    if(value instanceof JSONObject && ((JSONObject) value).containsKey(BindingUtils.BINDING)){
                        Object bindingValue = getBindingValue( ((JSONObject) value).getString(BindingUtils.BINDING), context);
                        if(bindingValue == null){
                            bindingValue = "";
                        }
                        builder.append(bindingValue);
                    }
                }
                dynamic.put(key, builder.toString());
            }
        }
        return  dynamic;
    }

    public static List<Object> getBindingEventArgsValue(Stack context, Object bindings){
          List<Object>  params = new ArrayList<>(4);
          if(bindings instanceof  JSONArray){
              JSONArray array = (JSONArray) bindings;
              for(int i=0; i<array.size(); i++){
                  Object value = array.get(i);
                  if(value instanceof  JSONObject && ((JSONObject) value).containsKey(BindingUtils.BINDING)){
                      Object bindingValue = getBindingValue(((JSONObject)value).getString(BindingUtils.BINDING), context);
                      params.add(bindingValue);
                  }else{
                      params.add(value);
                  }
              }


          }else if(bindings instanceof  JSONObject){
              JSONObject binding = (JSONObject) bindings;
               if(binding.containsKey(BindingUtils.BINDING)){
                   Object bindingValue = getBindingValue(binding.getString(BindingUtils.BINDING), context);
                   params.add(bindingValue);
               }else{
                   params.add(bindings);
               }
          }else{
              params.add(bindings.toString());
          }
          return  params;
    }


    /**
     * true ? true : false
     * return binding value statement's supported
     * */
    public static Object getBindingValue(String expression, Stack context){
        int start = expression.indexOf('?');
        int end = expression.indexOf(':');
        if(start >= 0 && end > 0 && start < end){
            String[] parts = splitTwoByOperator(expression, "?");
            String[] values = splitTwoByOperator(parts[1], ":");
            if(isIfTrue(parts[0], context)){
                return getStatementValue(context, values[0]);
            }else {
                return getStatementValue(context, values[1]);
            }
        }
        return  ELUtils.getValue(context, expression);
    }

    /**
     * check vif is true in context. vif can contain's
     * operator parse priority：
     * (
     * ||  &&
     * >=  <=
     * >   <
     * ===  !==
     * ==  !=
     * !
     * */
    public static boolean isIfTrue(String vif, Stack context){
        if(vif.indexOf('(') >= 0){
            Stack operator = new Stack();
            int start = vif.indexOf('(');
            int matchIndex = -1;
            for(int i= start; i<vif.length(); i++){
                char ch = vif.charAt(i);
                if(ch == '('){
                    operator.push(i);
                }
                if(ch == ')'){
                    operator.pop();
                    if(operator.size() == 0){
                        matchIndex = i;
                        break;
                    }
                }
            }
            if(matchIndex < 0){
                matchIndex = vif.length();
            }
            StringBuilder builder = new StringBuilder();
            for(int i=0; i<start; i++){
                char ch = vif.charAt(i);
                if(ch == ' '){
                    continue;
                }
                builder.append(ch);
            }
            if(matchIndex > start +1){
                String   bracket = vif.substring(start + 1, matchIndex);
                if(!isEmpty(bracket)){
                    boolean value = isIfTrue(bracket, context);
                    builder.append(value);
                }
            }
            for(int i=matchIndex + 1; i<vif.length(); i++){
                char ch = vif.charAt(i);
                if(ch == ' '){
                    continue;
                }
                builder.append(ch);
            }
            return isIfTrue(builder.toString(), context);
        }else if(vif.indexOf("||") > 0){
            String[] values = splitTwoByOperator(vif, "||");
            if(values == null || values.length == 0){
                return  false;
            }
            if(values.length == 1){
                return isIfTrue(values[0], context);
            }
            boolean left = isIfTrue(values[0], context);
            if(left){
                return true;
            }
            return  isIfTrue(values[1], context);
        }else if(vif.indexOf("&&") > 0){
            String[] values = splitTwoByOperator(vif, "&&");
            if(values == null
                    || values.length == 0){
                return  false;
            }
            if(values.length == 1){
                return isIfTrue(values[0], context);
            }
            boolean left = isIfTrue(values[0], context);
            if(left){
                return isIfTrue(values[1], context);
            }
            return  false;
        }else if(vif.indexOf(">=") > 0){
            String[] values = splitTwoByOperator(vif, ">=");
            if(values == null
                    || values.length == 0
                    || values.length == 1
                    || isEmpty(values[0])
                    || isEmpty(values[1])){
                return  false;
            }
            Object left = getStatementValue(context, values[0]);
            Object right = getStatementValue(context, values[1]);
            return  getNumber(left) >= getNumber(right);
        }else if(vif.indexOf("<=") > 0){
            String[] values = splitTwoByOperator(vif, "<=");
            if(values == null
                    || values.length == 0
                    || values.length == 1
                    || isEmpty(values[0])
                    || isEmpty(values[1])){
                return  false;
            }
            Object left = getStatementValue(context, values[0]);
            Object right = getStatementValue(context, values[1]);
            return  getNumber(left) <= getNumber(right);
        }else if(vif.indexOf(">") > 0){
            String[] values = splitTwoByOperator(vif, ">");
            if(values == null
                    || values.length == 0
                    || values.length == 1
                    || isEmpty(values[0])
                    || isEmpty(values[1])){
                return  false;
            }
            Object left = getStatementValue(context, values[0]);
            Object right = getStatementValue(context, values[1]);
            return  getNumber(left) > getNumber(right);
        }else if(vif.indexOf("<") > 0){
            String[] values = splitTwoByOperator(vif, "<");
            if(values == null
                    || values.length == 0
                    || values.length == 1
                    || isEmpty(values[0])
                    || isEmpty(values[1])){
                return  false;
            }
            Object left = getStatementValue(context, values[0]);
            Object right = getStatementValue(context, values[1]);
            return  getNumber(left) < getNumber(right);
        }else if(vif.indexOf("===") > 0){
            String[] values = splitTwoByOperator(vif, "===");
            return isEquals(values, context);
        }else if(vif.indexOf("!==") > 0){
            String[] values = splitTwoByOperator(vif, "!==");
            return !isEquals(values, context);
        }else if(vif.indexOf("==") > 0){
            String[] values = splitTwoByOperator(vif, "==");
            return isEquals(values, context);
        }else if(vif.indexOf("!=") > 0){
            String[] values = splitTwoByOperator(vif, "!=");
            return !isEquals(values, context);
        }else if(vif.indexOf('!') > 0){
            int index = vif.indexOf('!');
            if(index + 1 >= vif.length()){
                return  false;
            }
            String nextVIf = vif.substring(index + 1, vif.length());
            if(isEmpty(nextVIf)){
                return false;
            }
            return  !isIfTrue(nextVIf, context);
        }else{
            Object value = getStatementValue(context, vif);
            return  isTrue(value);
        }
    }

    /**
     * v-if check left and right values is equals in context
     * */
    private static boolean isEquals(String[] values, Stack context){
        if(values.length == 0){
            return  false;
        }
        String left = values[0];
        if(values.length == 1){
            return  false;
        }
        String right = values[1];
        Object leftValue = getStatementValue(context, left);
        Object rightValue = getStatementValue(context, right);
        System.out.println(left.getClass() + "value " + left  + "  " + rightValue);
        if(leftValue == null){
            if(rightValue == null){
                return  true;
            }
            if(rightValue instanceof  CharSequence){
                if(isEmpty(rightValue.toString())){
                    return  true;
                }
            }
            return  false;
        }
        if(rightValue == null){
            if(isEmpty(leftValue.toString())){
                return  true;
            }
            return  false;
        }
        if(leftValue instanceof CharSequence
                || leftValue instanceof  Number){
            String leftStr = leftValue.toString();
            if(isNumber(leftStr)){
                if(leftStr.indexOf('.') > 0){
                    return  getNumber(leftStr)  == getNumber(rightValue);
                }else{
                    return  ((int)getNumber(leftStr))  == ((int)getNumber(rightValue));
                }
            }
            return  leftValue.toString().trim().equals(rightValue.toString().trim());
        }
        if(rightValue instanceof CharSequence
                || rightValue instanceof  Number){
            String rightStr = leftValue.toString();
            if(isNumber(rightStr)){
                if(rightStr.indexOf('.') > 0){
                    return  getNumber(leftValue)  == getNumber(rightValue);
                }else{
                    return  ((int)getNumber(leftValue))  == ((int)getNumber(rightValue));
                }
            }
            return  leftValue.toString().trim().equals(rightValue.toString().trim());
        }
        return  leftValue.equals(rightValue);
    }


    /**
     * @param  vif   statement
     * @param operator operator
     * split use first operator index, and only split to two part, or less than two parts
     * */
    public static String[] splitTwoByOperator(String vif, String operator){
        int index = vif.indexOf(operator);
        if(index < 0){
            index = 0;
        }
        String[] split = new String[2];
        if(index > 0){
            split[0] = vif.substring(0, index).trim();
        }else{
            split[0] = "";
        }
        if(index + operator.length() < vif.length()){;
            split[1] = vif.substring(index + operator.length(),  vif.length()).trim();
        }else{
            split[1] = "";
        }
        return  split;
    }

    private static boolean isTrue(Object value){
        if(value == null){
            return  false;
        }
        String bool = value.toString();
        if("false".equals(bool) || "undefined".equals(bool)){
            return  false;
        }
        if(isEmpty(bool)){
            return  false;
        }
        return  true;
    }

    /**
     * check string is empty
     * */
    private static boolean isEmpty(String value){
        if (value == null){
            return  true;
        }
        for(int i=0; i<value.length(); i++){
            if(value.charAt(i) != ' '){
                return  false;
            }
        }
        return true;
    }

    /**
     * get number
     * */
    private static double getNumber(Object value){
        if(value == null){
            return  0;
        }
        if(value instanceof  Number){
            return  ((Number) value).doubleValue();
        }
        try{
            return Double.parseDouble(value.toString());
        }catch (Exception e){return  0;}
    }


    /**
     * @param context  stack
     * @param statement statment
     * check statement is number string add or subplus, if true, return it.
     * if not, execute statement as el expression in context and return value
     * */
    public static Object getStatementValue(Stack<Map> context, String statement){
        if(statement == null){
            return  null;
        }
        statement = statement.trim();
        if(statement.length() == 0){
            return  statement;
        }
        if(statement.indexOf('+') > 0){
            String[] parts = splitTwoByOperator(statement, "+");
            Object partone = getStatementValue(context, parts[0]);
            Object parttwo = getStatementValue(context, parts[1]);
            if(partone != null && parttwo != null){
                if(isNumber(partone.toString().trim())
                        || isNumber(parttwo.toString().trim())){
                    return  getNumber(partone) + getNumber(parttwo);
                }
                return  partone + parttwo.toString();
            }
            return getNumber(partone) + getNumber(parttwo);
        }else if(statement.indexOf('-') > 0){
            String[] parts = splitTwoByOperator(statement, "-");
            return getNumber(getStatementValue(context, parts[0]))-getNumber(getStatementValue(context, parts[1]));
        }else if(statement.indexOf('*') > 0){
            String[] parts = splitTwoByOperator(statement, "*");
            return getNumber(getStatementValue(context, parts[0]))*getNumber(getStatementValue(context, parts[1]));
        }else if(statement.indexOf('/') > 0){
            String[] parts = splitTwoByOperator(statement, "/");
            return getNumber(getStatementValue(context, parts[0]))/getNumber(getStatementValue(context, parts[1]));
        }else if(statement.indexOf('%') > 0){
            String[] parts = splitTwoByOperator(statement, "%");
            return ((int)getNumber(getStatementValue(context, parts[0])))%((int)getNumber(getStatementValue(context, parts[1])));
        }
        if(isNumber(statement)){
            return  statement.trim();
        }
        if(isString(statement)){
            return getString(statement);
        }
        if("true".equals(statement)
                || "false".equals(statement)
                || "undefined".equals(statement)){
            return  statement;
        }
        return  ELUtils.getValue(context, statement);
    }


    /**
     * check expression is string
     * */
    public static boolean isString(String value){
        if(value.indexOf('\'') >= 0 || value.indexOf('"') >= 0){
            return  true;
        }
        return  false;
    }


    /**
     * @param  statement statement may be 'string' or "string"
     * get string from statement
     * */
    public static String getString(String statement){
        int index = statement.indexOf('\'');
        if(index >= 0){
            int end = statement.lastIndexOf('\'');
            if(end == index){
                end = statement.length();
            }
            if(end > index + 1){
                return  statement.substring(index+1, end);
            }
        }
        index = statement.indexOf('"');
        if(index >= 0){
            int end = statement.lastIndexOf('\"');
            if(end == index){
                end = statement.length();
            }
            if(end > index + 1){
                return  statement.substring(index+1, end);
            }
        }
        return  statement;
    }

    /**
     * check if and value is number
     * */
    public static boolean isNumber(String value){
        for(int i=0; i<value.length(); i++){
            char ch = value.charAt(i);
            if(!(Character.isDigit(ch) || ch == '.')){
                return  false;
            }
        }
        return true;
    }
}
