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
package org.apache.weex.http;

import static org.apache.weex.http.WXHttpUtil.KEY_USER_AGENT;

import android.net.Uri;
import android.text.TextUtils;

import com.alibaba.fastjson.JSONException;
import com.alibaba.fastjson.JSONObject;
import org.apache.weex.WXEnvironment;
import org.apache.weex.WXSDKInstance;
import org.apache.weex.WXSDKManager;
import org.apache.weex.adapter.IWXHttpAdapter;
import org.apache.weex.adapter.URIAdapter;
import org.apache.weex.annotation.JSMethod;
import org.apache.weex.bridge.JSCallback;
import org.apache.weex.bridge.WXBridgeManager;
import org.apache.weex.common.WXModule;
import org.apache.weex.common.WXRequest;
import org.apache.weex.common.WXResponse;
import org.apache.weex.performance.WXStateRecord;
import org.apache.weex.utils.WXLogUtils;
import java.io.UnsupportedEncodingException;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class WXStreamModule extends WXModule {

  public static final String STATUS_TEXT = "statusText";
  public static final String STATUS = "status";
  final IWXHttpAdapter mAdapter;
  static final Pattern CHARSET_PATTERN = Pattern.compile("charset=([a-z0-9-]+)");

  public WXStreamModule(){
    this(null);
  }
  public WXStreamModule(IWXHttpAdapter adapter){
    mAdapter = adapter;
  }

  /**
   * send HTTP request
   *
   * @param params   {method:POST/GET/PUT/DELETE/HEAD/PATCH,url:http://xxx,header:{key:value},
   *                 body:{key:value}}
   * @param callback formate：handler(err, response)
   */
  @Deprecated
  @JSMethod(uiThread = false)
  public void sendHttp(JSONObject paramsObj, final String callback) {

    String method = paramsObj.getString("method");
    String url = paramsObj.getString("url");
    JSONObject headers = paramsObj.getJSONObject("header");
    String body = paramsObj.getString("body");
    int timeout = paramsObj.getIntValue("timeout");

    if (method != null) method = method.toUpperCase(Locale.ROOT);
    Options.Builder builder = new Options.Builder()
            .setMethod(!"GET".equals(method)
                    &&!"POST".equals(method)
                    &&!"PUT".equals(method)
                    &&!"DELETE".equals(method)
                    &&!"HEAD".equals(method)
                    &&!"PATCH".equals(method)?"GET":method)
            .setUrl(url)
            .setBody(body)
            .setTimeout(timeout);

    extractHeaders(headers,builder);
    sendRequest(builder.createOptions(), new ResponseCallback() {
      @Override
      public void onResponse(WXResponse response, Map<String, String> headers) {
        if(callback != null && mWXSDKInstance != null)
          WXBridgeManager.getInstance().callback(mWXSDKInstance.getInstanceId(), callback,
                  (response == null || response.originalData == null) ? "{}" :
                          readAsString(response.originalData,
                                  headers!=null?getHeader(headers,"Content-Type"):""
                          ));
      }
    }, null, mWXSDKInstance.getInstanceId(), mWXSDKInstance.getBundleUrl());
  }

  /**
   *
   * @param optionsStr request options include:
   *  method: GET 、POST、PUT、DELETE、HEAD、PATCH
   *  headers：object，request header
   *  url:
   *  body: "Any body that you want to add to your request"
   *  type: json、text、jsonp（json）
   * @param callback finished callback,response object:
   *  status：status code
   *  ok：boolean is success，http status200～299
   *  statusText： statusText
   *  data:  option type is json，data is object，not data is string
   *  headers: headers
   *
   * @param progressCallback in progress callback,for download progress and request state,response object:
   *  readyState: number connection status 1 OPENED 2 HEADERS_RECEIVED 3 LOADING
   *  status：status code
   *  length：headers Content-Length
   *  statusText：statusText
   *  headers: headers
   */
  @JSMethod(uiThread = false)
  public void fetch(JSONObject optionsObj , final JSCallback callback, JSCallback progressCallback){
    fetch(optionsObj, callback, progressCallback, mWXSDKInstance.getInstanceId(), mWXSDKInstance.getBundleUrl());
  }

  public void fetch(JSONObject optionsObj , final JSCallback callback, JSCallback progressCallback, final String instanceId, String bundleURL){
    boolean invaildOption = optionsObj==null || optionsObj.getString("url")==null;
    if(invaildOption){
      if(callback != null) {
        Map<String, Object> resp = new HashMap<>();
        resp.put("ok", false);
        resp.put(STATUS_TEXT, Status.ERR_INVALID_REQUEST);
        callback.invoke(resp);
      }
      return;
    }
    String method = optionsObj.getString("method");
    String url = optionsObj.getString("url");
    JSONObject headers = optionsObj.getJSONObject("headers");
    String body = optionsObj.getString("body");
    String type = optionsObj.getString("type");
    int timeout = optionsObj.getIntValue("timeout");

    WXSDKInstance wxsdkInstance = WXSDKManager.getInstance().getSDKInstance(instanceId);
    if (wxsdkInstance != null) {
      if (wxsdkInstance.getStreamNetworkHandler() != null) {
        String localUrl = wxsdkInstance.getStreamNetworkHandler().fetchLocal(url);
        if (!TextUtils.isEmpty(localUrl)) {
          url = localUrl;
        }
      }
    }

    if (method != null) method = method.toUpperCase(Locale.ROOT);
    Options.Builder builder = new Options.Builder()
        .setMethod(!"GET".equals(method)
            &&!"POST".equals(method)
            &&!"PUT".equals(method)
            &&!"DELETE".equals(method)
            &&!"HEAD".equals(method)
            &&!"PATCH".equals(method)?"GET":method)
        .setUrl(url)
        .setBody(body)
        .setType(type)
        .setTimeout(timeout);

    extractHeaders(headers,builder);
    final Options options = builder.createOptions();
    sendRequest(options, new ResponseCallback() {
      @Override
      public void onResponse(WXResponse response, Map<String, String> headers) {
        if(callback != null) {
          Map<String, Object> resp = new HashMap<>();
          if(response == null|| "-1".equals(response.statusCode)){
            resp.put(STATUS,-1);
            resp.put(STATUS_TEXT,Status.ERR_CONNECT_FAILED);
          }else {
            int code = Integer.parseInt(response.statusCode);
            resp.put(STATUS, code);
            resp.put("ok", (code >= 200 && code <= 299));
            if (response.originalData == null) {
              resp.put("data", null);
            } else {
              String respData = readAsString(response.originalData,
                  headers != null ? getHeader(headers, "Content-Type") : ""
              );
              try {
                resp.put("data", parseData(respData, options.getType()));
              } catch (JSONException exception) {
                WXLogUtils.e("", exception);
                resp.put("ok", false);
                resp.put("data","{'err':'Data parse failed!'}");
              }
            }
            resp.put(STATUS_TEXT, Status.getStatusText(response.statusCode));
          }
          resp.put("headers", headers);
          WXStateRecord
              .getInstance().recordAction(instanceId,"stream response code:"+(null!= response?response.statusCode:"null"));
          callback.invoke(resp);
        }
      }
    }, progressCallback, instanceId, bundleURL);
  }

  Object parseData(String data, Options.Type type) throws JSONException{
    if( type == Options.Type.json){
      return JSONObject.parse(data);
    }else if( type == Options.Type.jsonp){
      if(data == null || data.isEmpty()) {
        return new JSONObject();
      }
      int b = data.indexOf("(")+1;
      int e = data.lastIndexOf(")");
      if(b ==0 || b >= e || e <= 0){
        return new JSONObject();
      }

      data = data.substring(b,e);
      return JSONObject.parse(data);
    }else {
      return data;
    }
  }

  static String getHeader(Map<String,String> headers,String key){
    if(headers == null||key == null){
      return null;
    }
    if(headers.containsKey(key)){
      return headers.get(key);
    }else{
      return headers.get(key.toLowerCase(Locale.ROOT));
    }
  }



  static String readAsString(byte[] data,String cType){
    String charset = "utf-8";
    if(cType != null){
      Matcher matcher = CHARSET_PATTERN.matcher(cType.toLowerCase(Locale.ROOT));
      if(matcher.find()){
        charset = matcher.group(1);
      }
    }
    try {
      return new String(data,charset);
    } catch (UnsupportedEncodingException e) {
      WXLogUtils.e("", e);
      return new String(data);
    }
  }


  private void extractHeaders(JSONObject headers, Options.Builder builder){
    //set user-agent
    String UA = WXHttpUtil.assembleUserAgent(WXEnvironment.getApplication(),WXEnvironment.getConfig());
    if(headers != null){
      for (String key : headers.keySet()) {
        if (key.equals(KEY_USER_AGENT)) {
          UA = headers.getString(key);
          continue;
        }
        builder.putHeader(key, headers.getString(key));
      }
    }
    builder.putHeader(KEY_USER_AGENT,UA);
  }


  private void sendRequest(Options options,ResponseCallback callback,JSCallback progressCallback,String instanceId, String bundleURL){
    WXRequest wxRequest = new WXRequest();
    wxRequest.method = options.getMethod();
    wxRequest.url = WXSDKManager.getInstance().getURIAdapter().rewrite(bundleURL, URIAdapter.REQUEST,Uri.parse(options.getUrl())).toString();
    wxRequest.body = options.getBody();
    wxRequest.timeoutMs = options.getTimeout();
    wxRequest.instanceId = instanceId;

    if(options.getHeaders()!=null) {
      if (wxRequest.paramMap == null) {
        wxRequest.paramMap = options.getHeaders();
      } else {
        wxRequest.paramMap.putAll(options.getHeaders());
      }
    }

    IWXHttpAdapter adapter = ( mAdapter==null) ? WXSDKManager.getInstance().getIWXHttpAdapter() : mAdapter;
    if (adapter != null) {
      adapter.sendRequest(wxRequest, new StreamHttpListener(callback,progressCallback));
    }else{
      WXLogUtils.e("WXStreamModule","No HttpAdapter found,request failed.");
    }
  }

  private interface ResponseCallback{
    void onResponse(WXResponse response, Map<String, String> headers);
  }

  private static class StreamHttpListener implements IWXHttpAdapter.OnHttpListener {
    private ResponseCallback mCallback;
    private JSCallback mProgressCallback;
    private Map<String,Object> mResponse = new HashMap<>();
    private Map<String,String> mRespHeaders;

    private StreamHttpListener(ResponseCallback callback,JSCallback progressCallback) {
      mCallback = callback;
      mProgressCallback = progressCallback;
    }


    @Override
    public void onHttpStart() {
      if(mProgressCallback !=null) {
        mResponse.put("readyState",1);//readyState: number 1 OPENED 2 HEADERS_RECEIVED 3 LOADING
        mResponse.put("length",0);
        mProgressCallback.invokeAndKeepAlive(new HashMap<>(mResponse));
      }
    }

    @Override
    public void onHttpUploadProgress(int uploadProgress) {

    }

    @Override
    public void onHeadersReceived(int statusCode,Map<String,List<String>> headers) {
      mResponse.put("readyState", 2);
      mResponse.put("status", statusCode);

      Map<String, String> simpleHeaders = new HashMap<>();
      if (headers != null) {
        Iterator<Map.Entry<String, List<String>>> it = headers.entrySet().iterator();
        while (it.hasNext()) {
          Map.Entry<String, List<String>> entry = it.next();
          if (entry.getValue().size() == 0) {
            continue;
          } else if (entry.getValue().size() == 1)
            simpleHeaders.put(entry.getKey() == null ? "_" : entry.getKey(), entry.getValue().get(0));
          else {
            simpleHeaders.put(entry.getKey() == null ? "_" : entry.getKey(), entry.getValue().toString());
          }
        }
      }

      mResponse.put("headers", simpleHeaders);
      mRespHeaders = simpleHeaders;
      if (mProgressCallback != null) {
        mProgressCallback.invokeAndKeepAlive(new HashMap<>(mResponse));
      }
    }

    @Override
    public void onHttpResponseProgress(int loadedLength) {
      mResponse.put("length",loadedLength);
      if(mProgressCallback!=null){
        mProgressCallback.invokeAndKeepAlive(new HashMap<>(mResponse));
      }

    }

    @Override
    public void onHttpFinish(final WXResponse response) {
      //compatible with old sendhttp
      if(mCallback!=null){
        mCallback.onResponse(response, mRespHeaders);
      }

      if(WXEnvironment.isApkDebugable()){
        WXLogUtils.d("WXStreamModule",response!=null && response.originalData!=null?new String(response.originalData):"response data is NUll!");
      }
    }
  }

}
