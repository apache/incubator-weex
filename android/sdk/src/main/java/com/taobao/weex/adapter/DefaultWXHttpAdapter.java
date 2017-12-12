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
package com.taobao.weex.adapter;

import android.net.Uri;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.text.TextUtils;

import com.taobao.weex.common.WXPerformance;
import com.taobao.weex.common.WXRequest;
import com.taobao.weex.common.WXResponse;
import com.taobao.weex.utils.WXLogUtils;
import com.taobao.zcache.zipapp.utils.ZipAppUtils;

import java.io.BufferedReader;
import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;


public class DefaultWXHttpAdapter implements IWXHttpAdapter {

  private static final IEventReporterDelegate DEFAULT_DELEGATE = new NOPEventReportDelegate();
  private ExecutorService mExecutorService;

  @Override
  public void sendRequest(final WXRequest request, final OnHttpListener listener) {
    if (listener != null) {
      listener.onHttpStart();
      final WXResponse response = getResponseByPackageApp(request);
      if (TextUtils.equals("200", response.statusCode)) {
        response.extendParams.put(WXPerformance.CACHE_TYPE, "zcache");
        listener.onHttpFinish(response);
      }
      else {
        execute(new Runnable() {
          @Override
          public void run() {
            IEventReporterDelegate reporter = getEventReporterDelegate();
            try {
              HttpURLConnection connection = openConnection(request, listener);
              reporter.preConnect(connection, request.body);
              Map<String, List<String>> headers = connection.getHeaderFields();
              int responseCode = connection.getResponseCode();
              if (listener != null) {
                listener.onHeadersReceived(responseCode, headers);
              }
              reporter.postConnect();

              response.statusCode = String.valueOf(responseCode);
              if (responseCode >= 200 && responseCode <= 299) {
                InputStream rawStream = connection.getInputStream();
                rawStream = reporter.interpretResponseStream(rawStream);
                response.originalData = readInputStreamAsBytes(rawStream, listener);
              } else {
                response.errorMsg = readInputStream(connection.getErrorStream(), listener);
              }
              response.extendParams.put(WXPerformance.CACHE_TYPE, "none");
              listener.onHttpFinish(response);
            } catch (IOException | IllegalArgumentException e) {
              e.printStackTrace();
              response.statusCode = "-1";
              response.errorCode = "-1";
              response.errorMsg = e.getMessage();
              if (listener != null) {
                listener.onHttpFinish(response);
              }
              if (e instanceof IOException) {
                reporter.httpExchangeFailed((IOException) e);
              }
            }
          }
        });
      }
    }
  }

  private void execute(Runnable runnable){
    if(mExecutorService==null){
      mExecutorService = Executors.newFixedThreadPool(3);
    }
    mExecutorService.execute(runnable);
  }

  /**
   * 通过PackageApp获取JS Bundle的缓存
   *
   * @param request  请求Request
   * @param response response
   * @return 有缓存response.statusCode=200
   */
  private WXResponse getResponseByPackageApp(WXRequest request) {
    WXResponse ret = new WXResponse();
    ret.statusCode = "-1";
    String template="";
    String url = request.url.trim();
    try {
      Uri uri = Uri.parse(url);
      if(uri.getBooleanQueryParameter("wh_weex", false)) {
        String host = uri.getHost();
        String newHost = uri.getHost() + ".local.weex";
        String newUrl = url.replace(host, newHost);
        //TODO Zcache剥离&初始化
        template = ZipAppUtils.getStreamByUrl(newUrl);
      } else {
        //TODO Zcache剥离&初始化
        template = ZipAppUtils.getStreamByUrl(url);
      }
    }catch(Exception e){
      WXLogUtils.e("getResponseByPackageApp error:"+e.getMessage());
    }

    if (!TextUtils.isEmpty(template)) {
      ret.statusCode = "200";
      ret.originalData = template.getBytes();
      ret.extendParams.put("requestType", "packageApp");
      ret.extendParams.put("connectionType", "packageApp");
      return ret;
    }
    return ret;
  }

  /**
   * Opens an {@link HttpURLConnection} with parameters.
   *
   * @param request
   * @param listener
   * @return an open connection
   * @throws IOException
   */
  private HttpURLConnection openConnection(WXRequest request, OnHttpListener listener) throws IOException {
    URL url = new URL(request.url);
    HttpURLConnection connection = createConnection(url);
    connection.setConnectTimeout(request.timeoutMs);
    connection.setReadTimeout(request.timeoutMs);
    connection.setUseCaches(false);
    connection.setDoInput(true);

    if (request.paramMap != null) {
      Set<String> keySets = request.paramMap.keySet();
      for (String key : keySets) {
        connection.addRequestProperty(key, request.paramMap.get(key));
      }
    }

    if ("POST".equals(request.method) || "PUT".equals(request.method) || "PATCH".equals(request.method)) {
      connection.setRequestMethod(request.method);
      if (request.body != null) {
        if (listener != null) {
          listener.onHttpUploadProgress(0);
        }
        connection.setDoOutput(true);
        DataOutputStream out = new DataOutputStream(connection.getOutputStream());
        //TODO big stream will cause OOM; Progress callback is meaningless
        out.write(request.body.getBytes());
        out.close();
        if (listener != null) {
          listener.onHttpUploadProgress(100);
        }
      }
    } else if (!TextUtils.isEmpty(request.method)) {
      connection.setRequestMethod(request.method);
    } else {
      connection.setRequestMethod("GET");
    }

    return connection;
  }

  private byte[] readInputStreamAsBytes(InputStream inputStream,OnHttpListener listener) throws IOException{
    if(inputStream == null){
      return null;
    }
    ByteArrayOutputStream buffer = new ByteArrayOutputStream();

    int nRead;
    int readCount = 0;
    byte[] data = new byte[2048];

    while ((nRead = inputStream.read(data, 0, data.length)) != -1) {
      buffer.write(data, 0, nRead);
      readCount += nRead;
      if (listener != null) {
        listener.onHttpResponseProgress(readCount);
      }
    }

    buffer.flush();

    return buffer.toByteArray();
  }

  private String readInputStream(InputStream inputStream, OnHttpListener listener) throws IOException {
    if(inputStream == null){
      return null;
    }
    StringBuilder builder = new StringBuilder();
    BufferedReader localBufferedReader = new BufferedReader(new InputStreamReader(inputStream));
    char[] data = new char[2048];
    int len;
    while ((len = localBufferedReader.read(data)) != -1) {
      builder.append(data, 0, len);
      if (listener != null) {
        listener.onHttpResponseProgress(builder.length());
      }
    }
    localBufferedReader.close();
    return builder.toString();
  }

  /**
   * Create an {@link HttpURLConnection} for the specified {@code url}.
   */
  protected HttpURLConnection createConnection(URL url) throws IOException {
    return (HttpURLConnection) url.openConnection();
  }

  public @NonNull IEventReporterDelegate getEventReporterDelegate() {
    return DEFAULT_DELEGATE;
  }

  public interface IEventReporterDelegate {
    void preConnect(HttpURLConnection connection, @Nullable String body);
    void postConnect();
    InputStream interpretResponseStream(@Nullable InputStream inputStream);
    void httpExchangeFailed(IOException e);
  }

  private static class NOPEventReportDelegate implements IEventReporterDelegate {
    @Override
    public void preConnect(HttpURLConnection connection, @Nullable String body) {
      //do nothing
    }

    @Override
    public void postConnect() {
      //do nothing
    }

    @Override
    public InputStream interpretResponseStream(@Nullable InputStream inputStream) {
      return inputStream;
    }

    @Override
    public void httpExchangeFailed(IOException e) {
      //do nothing
    }
  }
}
