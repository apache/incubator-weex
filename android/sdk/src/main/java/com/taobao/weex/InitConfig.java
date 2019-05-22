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
package com.taobao.weex;

import android.support.annotation.NonNull;
import com.taobao.weex.adapter.ClassLoaderAdapter;
import com.taobao.weex.adapter.IDrawableLoader;
import com.taobao.weex.adapter.IWXHttpAdapter;
import com.taobao.weex.adapter.IWXImgLoaderAdapter;
import com.taobao.weex.adapter.IWXJSExceptionAdapter;
import com.taobao.weex.adapter.IWXJsFileLoaderAdapter;
import com.taobao.weex.adapter.IWXJscProcessManager;
import com.taobao.weex.adapter.IWXSoLoaderAdapter;
import com.taobao.weex.adapter.IWXUserTrackAdapter;
import com.taobao.weex.adapter.URIAdapter;
import com.taobao.weex.appfram.storage.IWXStorageAdapter;
import com.taobao.weex.appfram.websocket.IWebSocketAdapterFactory;
import com.taobao.weex.performance.IApmGenerator;
import java.util.LinkedList;
import java.util.List;

/**
 * Created by sospartan on 5/31/16.
 */
public class InitConfig {
  private IWXHttpAdapter httpAdapter;
  private IDrawableLoader drawableLoader;
  private IWXImgLoaderAdapter imgAdapter;
  private IWXUserTrackAdapter utAdapter;
  private IWXStorageAdapter storageAdapter;
  private IWXSoLoaderAdapter soLoader;
  private URIAdapter mURIAdapter;
  private IWebSocketAdapterFactory webSocketAdapterFactory;
  private IWXJSExceptionAdapter mJSExceptionAdapter;
  private String framework;
  private ClassLoaderAdapter classLoaderAdapter;
  private IApmGenerator apmGenerater;
  private IWXJsFileLoaderAdapter jsFileLoaderAdapter;
  private IWXJscProcessManager jscProcessManager;
  private List<String> nativeLibraryList;

  public IWXHttpAdapter getHttpAdapter() {
    return httpAdapter;
  }

  public IWXImgLoaderAdapter getImgAdapter() {
    return imgAdapter;
  }

  public IDrawableLoader getDrawableLoader() {
    return drawableLoader;
  }

  public IWXUserTrackAdapter getUtAdapter() {
    return utAdapter;
  }

  public IWXSoLoaderAdapter getIWXSoLoaderAdapter() {
    return soLoader;
  }

  public String getFramework() {
    return framework;
  }

  public IWXStorageAdapter getStorageAdapter() {
    return storageAdapter;
  }

  public URIAdapter getURIAdapter() {
    return mURIAdapter;
  }

  public IWebSocketAdapterFactory getWebSocketAdapterFactory() {
    return webSocketAdapterFactory;
  }

  public ClassLoaderAdapter getClassLoaderAdapter() {
    return classLoaderAdapter;
  }

  public IApmGenerator getApmGenerater() {
    return apmGenerater;
  }

  public IWXJsFileLoaderAdapter getJsFileLoaderAdapter() {
    return jsFileLoaderAdapter;
  }

  public InitConfig setClassLoaderAdapter(ClassLoaderAdapter classLoaderAdapter) {
    this.classLoaderAdapter = classLoaderAdapter;
    return this;
  }

  public IWXJSExceptionAdapter getJSExceptionAdapter() {
    return mJSExceptionAdapter;
  }
  public IWXJscProcessManager getJscProcessManager() {
    return jscProcessManager;
  }

  @NonNull Iterable<String> getNativeLibraryList() {
    if(nativeLibraryList == null){
      nativeLibraryList = new LinkedList<>();
    }
    return nativeLibraryList;
  }

  private InitConfig() {
  }

  public static class Builder{
    IWXHttpAdapter httpAdapter;
    IWXImgLoaderAdapter imgAdapter;
    IDrawableLoader drawableLoader;
    IWXUserTrackAdapter utAdapter;
    IWXStorageAdapter storageAdapter;
    IWXSoLoaderAdapter soLoader;
    URIAdapter mURIAdapter;
    IWXJSExceptionAdapter mJSExceptionAdapter;
    String framework;
    IWebSocketAdapterFactory webSocketAdapterFactory;
    ClassLoaderAdapter classLoaderAdapter;
    IApmGenerator apmGenerater;
    private IWXJsFileLoaderAdapter jsFileLoaderAdapter;
    private List<String> nativeLibraryList = new LinkedList<>();

    public IWXJscProcessManager getJscProcessManager() {
      return jscProcessManager;
    }

    public Builder setJscProcessManager(IWXJscProcessManager jscProcessManager) {
      this.jscProcessManager = jscProcessManager;
      return this;
    }

    IWXJscProcessManager jscProcessManager;

    public Builder(){

    }

    public Builder setHttpAdapter(IWXHttpAdapter httpAdapter) {
      this.httpAdapter = httpAdapter;
      return this;
    }

    public Builder setImgAdapter(IWXImgLoaderAdapter imgAdapter) {
      this.imgAdapter = imgAdapter;
      return this;
    }

    public Builder setDrawableLoader(IDrawableLoader drawableLoader){
      this.drawableLoader=drawableLoader;
      return this;
    }

    public Builder setUtAdapter(IWXUserTrackAdapter utAdapter) {
      this.utAdapter = utAdapter;
      return this;
    }

    public Builder setStorageAdapter(IWXStorageAdapter storageAdapter) {
      this.storageAdapter = storageAdapter;
      return this;
    }

    public Builder setURIAdapter(URIAdapter URIAdapter) {
      mURIAdapter = URIAdapter;
      return this;
    }

    public Builder setJSExceptionAdapter(IWXJSExceptionAdapter JSExceptionAdapter) {
      mJSExceptionAdapter = JSExceptionAdapter;
      return this;
    }

    public Builder setSoLoader(IWXSoLoaderAdapter loader) {
      this.soLoader = loader;
      return this;
    }

    public Builder setFramework(String framework){
      this.framework=framework;
      return this;
    }

    public Builder setWebSocketAdapterFactory(IWebSocketAdapterFactory factory) {
      this.webSocketAdapterFactory = factory;
      return this;
    }

    public Builder setClassLoaderAdapter(ClassLoaderAdapter classLoaderAdapter) {
      this.classLoaderAdapter = classLoaderAdapter;
      return this;
    }

    public Builder setApmGenerater(IApmGenerator apmGenerater){
      this.apmGenerater =apmGenerater;
      return this;
    }

    public Builder setJsFileLoaderAdapter(IWXJsFileLoaderAdapter jsFileLoaderAdapter) {
      this.jsFileLoaderAdapter = jsFileLoaderAdapter;
      return this;
    }

    public Builder addNativeLibrary(String name){
      nativeLibraryList.add(name);
      return this;
    }

    public InitConfig build(){
      InitConfig config =  new InitConfig();
      config.httpAdapter = this.httpAdapter;
      config.imgAdapter = this.imgAdapter;
      config.drawableLoader = this.drawableLoader;
      config.utAdapter = this.utAdapter;
      config.storageAdapter = this.storageAdapter;
      config.soLoader=this.soLoader;
      config.framework=this.framework;
      config.mURIAdapter = this.mURIAdapter;
      config.webSocketAdapterFactory = this.webSocketAdapterFactory;
      config.mJSExceptionAdapter=this.mJSExceptionAdapter;
      config.classLoaderAdapter = this.classLoaderAdapter;
      config.apmGenerater = this.apmGenerater;
      config.jsFileLoaderAdapter = this.jsFileLoaderAdapter;
      config.jscProcessManager = this.jscProcessManager;
      config.nativeLibraryList = this.nativeLibraryList;
      return config;
    }
  }
}
