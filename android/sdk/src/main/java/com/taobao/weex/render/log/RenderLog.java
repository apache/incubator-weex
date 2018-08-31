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
package com.taobao.weex.render.log;

import android.util.Log;

import com.taobao.weex.render.view.DocumentView;

import org.json.JSONObject;

import java.util.Collection;
import java.util.Map;

/**
 * Render Log for recording render command and replay render action command
 * Created by furture on 2018/9/1.
 */
public class RenderLog {

    public static final String RENDER_LOG_TAG  = "WeexRenderLog";

    private static  boolean isLogEnabled = false;

    public static final void openRenderLog(boolean enabled){
        isLogEnabled = enabled;
    }

    public static boolean isRenderLogEnabled(){
        return isLogEnabled;
    }

    public static void actionCreateBody(DocumentView documentView, String ref, Map<String, String> style, Map<String, String> attrs, Collection<String> events) {
       if(isLogEnabled){
           Log.e(RENDER_LOG_TAG, RENDER_LOG_TAG + " " + documentView.hashCode() + "documentActionCreateBody;" + ref + ";"+ toJSON(style) + ";" + toJSON(attrs)
                   + ";" + toJSON(events));
       }
    }

    public static void actionAddElement(DocumentView documentView, String ref, String componentType, String parentRef, int index, Map<String, String> style, Map<String, String> attrs, Collection<String> events){
        if(isLogEnabled){
            Log.e(RENDER_LOG_TAG, RENDER_LOG_TAG + " " + documentView.hashCode() + "documentActionAddElement;" + ref + ";" + componentType + ";" + parentRef + ";" + index + ";"
                    + toJSON(style) + ";" + toJSON(attrs)
                    + ";" + toJSON(events));
        }
    }


    public static void actionUpdateStyles(DocumentView documentView, String ref, Map<String, String> styles) {
        if(isLogEnabled) {
            Log.e(RENDER_LOG_TAG, RENDER_LOG_TAG + " " + documentView.hashCode() + "documentActionUpdateStyle;" + ref + ";" + toJSON(styles));
        }
    }

    public static void actionUpdateAttrs(DocumentView documentView, String ref, Map<String, String> attrs){
        if(isLogEnabled) {
            Log.e(RENDER_LOG_TAG, RENDER_LOG_TAG + " " + documentView.hashCode() + "documentActionUpdateAttrs;" + ref + ";" + toJSON(attrs));
        }
    }


    public static void actionAddEvent(DocumentView documentView, String ref, String event){
        if(isLogEnabled) {
            Log.e(RENDER_LOG_TAG, RENDER_LOG_TAG + " " + documentView.hashCode() + "documentActionAddEvent;" + ref + ";" + event);
        }
    }

    public static void actionRemoveEvent(DocumentView documentView, String ref, String event){
        if(isLogEnabled) {
            Log.e(RENDER_LOG_TAG, RENDER_LOG_TAG + " " + documentView.hashCode() + "documentRemoveEvent;" + ref + ";" + event);
        }
    }


    public static void actionMoveElement(DocumentView documentView, String ref, String parentRef, int index){
        if(isLogEnabled) {
            Log.e(RENDER_LOG_TAG, RENDER_LOG_TAG + " " + documentView.hashCode() + "documentActionMoveElement;" + ref + ";" + parentRef + ";" + index);
        }
    }


    public static void actionRemoveElement(DocumentView documentView, String ref){
        if(isLogEnabled) {
            Log.e(RENDER_LOG_TAG, RENDER_LOG_TAG + " " + documentView.hashCode() + "documentActionRemoveElement;" + ref);
        }
    }

    public static void actionLayoutExecute(DocumentView documentView){
        if(isLogEnabled) {
            Log.e(RENDER_LOG_TAG, RENDER_LOG_TAG + " " + documentView.hashCode() + "documentActionLayoutExecute;" + documentView.getNativeDocument());
        }
    }

    public static void actionInvalidate(DocumentView documentView){
        if(isLogEnabled) {
            Log.e(RENDER_LOG_TAG, RENDER_LOG_TAG + " " + documentView.hashCode() + "documentActionInvalidate;" + documentView.getNativeDocument());
        }
    }

    public static void actionInitOpenGL(DocumentView documentView, int width, int height){
        if(isLogEnabled) {
            Log.e(RENDER_LOG_TAG, RENDER_LOG_TAG + " " + documentView.hashCode() + "documentActionInitOpenGL;" + width +";" + height);
        }
    }

    public static void actionSizeChanged(DocumentView documentView, int width, int height){
        if(isLogEnabled) {
            Log.e(RENDER_LOG_TAG, RENDER_LOG_TAG + " " + documentView.hashCode() + "documentActionSizeChanged;" + width + ";" + height);
        }
    }

    public static void actionDestroyGL(DocumentView documentView){
        if(isLogEnabled) {
            Log.e(RENDER_LOG_TAG, RENDER_LOG_TAG + " " + documentView.hashCode() + "documentActionDestroyGL;");
        }
    }

    public static void actionSwap(DocumentView documentView){
        if(isLogEnabled) {
            Log.e(RENDER_LOG_TAG, RENDER_LOG_TAG + " " + documentView.hashCode() + "documentActionSwap;" + documentView.getNativeDocument());
        }
    }

    public static void actionError(String message) {
        if(isLogEnabled){
            Log.e(RENDER_LOG_TAG, RENDER_LOG_TAG + " error " + message);
        }
    }

    private static final String toJSON(Map<String, String> style){
        if(style == null){
            return "null";
        }
        return new JSONObject(style).toString();
    }

    private static String toJSON(Collection<String> events) {
        if(events == null){
            return "null";
        }
        return new org.json.JSONArray(events).toString();
    }

}
