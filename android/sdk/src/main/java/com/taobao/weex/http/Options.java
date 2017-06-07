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
package com.taobao.weex.http;

import java.util.HashMap;
import java.util.Map;
import com.taobao.weex.common.*;

/**
 * Created by sospartan on 5/19/16.
 */
class Options {
  private String method;
  private String url;
  private Map<String, String> headers;
  private String body;
  private Type type = Type.text;
  private int timeout = WXRequest.DEFAULT_TIMEOUT_MS;

  private Options(String method,
                  String url,
                  Map<String, String> headers,
                  String body,
                  Type type,
                  int timeout) {
    this.method = method;
    this.url = url;
    this.headers = headers;
    this.body = body;
    this.type = type;
    if (timeout == 0) {
      timeout = WXRequest.DEFAULT_TIMEOUT_MS;
    }
    this.timeout = timeout;
  }

  public String getMethod() {
    return method;
  }

  public String getUrl() {
    return url;
  }

  public Map<String, String> getHeaders() {
    return headers;
  }

  public String getBody() {
    return body;
  }

  public Type getType() {
    return type;
  }

  public int getTimeout() { return timeout; }

  public enum Type {
    json, text,jsonp
  }

  public static class Builder {
    private String method;
    private String url;
    private Map<String, String> headers = new HashMap<>();
    private String body;
    private Type type;
    private int timeout;

    public Builder setMethod(String method) {
      this.method = method;
      return this;
    }

    public Builder setUrl(String url) {
      this.url = url;
      return this;
    }

    public Builder putHeader(String key,String value){
      this.headers.put(key,value);
      return this;
    }

    public Builder setBody(String body) {
      this.body = body;
      return this;
    }

    /**
     * default text
     * json = jsonp
     * @param type
     * @return
       */
    public Builder setType(String type) {
      if(Type.json.name().equals(type)){
        this.type = Type.json;
      }else if(Type.jsonp.name().equals(type)){
        this.type = Type.jsonp;
      }else{
        this.type = Type.text;
      }
      return this;
    }

    public Builder setType(Type type) {
      this.type = type;
      return this;
    }

    public Builder setTimeout(int timeout) {
      this.timeout = timeout;
      return this;
    }

    public Options createOptions() {
      return new Options(method, url, headers, body, type, timeout);
    }
  }
}
