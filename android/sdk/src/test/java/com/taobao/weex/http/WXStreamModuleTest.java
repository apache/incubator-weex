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

import android.os.Looper;
import android.telecom.Call;

import com.taobao.weex.WXSDKInstanceTest;
import com.taobao.weex.adapter.DefaultWXHttpAdapter;
import com.taobao.weex.adapter.IWXHttpAdapter;
import com.taobao.weex.bridge.JSCallback;
import com.taobao.weex.bridge.WXBridgeManager;
import com.taobao.weex.common.WXRequest;
import com.taobao.weex.common.WXResponse;
import com.taobao.weex.common.WXThread;
import junit.framework.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.invocation.InvocationOnMock;
import org.mockito.stubbing.Answer;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;

import java.util.*;
import java.util.logging.Level;
import java.util.logging.Logger;

import static org.mockito.Mockito.*;
import static junit.framework.Assert.*;


/**
 * Created by sospartan on 5/24/16.
 */
@RunWith(RobolectricTestRunner.class)
@PrepareForTest({WXStreamModule.class, IWXHttpAdapter.class})
@Config(manifest= Config.NONE)
public class WXStreamModuleTest {

  @Before
  public void setup() throws Exception{

  }

  private WXResponse successResponse(){
    WXResponse resp = new WXResponse();
    resp.data = "data";
    resp.statusCode = "200";
    return resp;
  }

  static class Callback implements JSCallback{
     Map<String, Object> mData;

    @Override
    public void invoke(Object data) {
      mData = (Map<String,Object>)data;
    }

    @Override
    public void invokeAndKeepAlive(Object data) {
      mData = (Map<String,Object>)data;
    }
  }


  @Test
  public void testFetchInvaildOptions() throws Exception{
    IWXHttpAdapter adapter = new IWXHttpAdapter() {
      @Override
      public void sendRequest(WXRequest request, OnHttpListener listener) {
        listener.onHttpFinish(successResponse());
      }
    };

    WXStreamModule streamModule = new WXStreamModule(adapter);
    Callback cb = new Callback();
    streamModule.fetch("",cb,null);

    assert   !(boolean)cb.mData.get("ok");
  }

  private WXStreamModule createModule(IWXHttpAdapter adapter){
    WXStreamModule m = new WXStreamModule(adapter);
    m.mWXSDKInstance = WXSDKInstanceTest.createInstance();
    return m;
  }

  @Test
  public void testFetchSuccessFinish() throws Exception{
    IWXHttpAdapter adapter = new IWXHttpAdapter() {
      @Override
      public void sendRequest(WXRequest request, OnHttpListener listener) {
        listener.onHttpFinish(successResponse());
      }
    };

    WXStreamModule streamModule = createModule(adapter);
    Callback cb = new Callback();
    streamModule.fetch("{'url':'http://www.taobao.com'}",cb,null);

    assert   (boolean)cb.mData.get("ok");
  }


  @Test
  public void testFetchHeaderReceived() throws Exception{
    IWXHttpAdapter adapter = new IWXHttpAdapter() {
      @Override
      public void sendRequest(WXRequest request, OnHttpListener listener) {
        Map<String,List<String>> headers = new HashMap<>();
        headers.put("key", Arrays.asList("someval"));
        listener.onHeadersReceived(200,headers);
      }
    };

    WXStreamModule streamModule = createModule(adapter);
    Callback cb = new Callback();
    streamModule.fetch("{'url':'http://www.taobao.com'}",null,cb);

    assert   ((Map<String,String>)cb.mData.get("headers")).get("key").equals("someval");
  }

  @Test
  public void testFetchRequestHttpbinCallback() throws Exception{
    WXStreamModule streamModule = createModule(new DefaultWXHttpAdapter());
    JSCallback progress = mock(JSCallback.class);
    JSCallback finish = mock(JSCallback.class);
    System.out.print("request start "+System.currentTimeMillis());
    streamModule.fetch("{method: 'POST',url: 'http://httpbin.org/post',type:'json'}",finish,progress);
    verify(progress,timeout(10*1000).atLeastOnce()).invokeAndKeepAlive(anyMapOf(String.class, Object.class));
    verify(finish,timeout(10*1000).times(1)).invoke(anyMapOf(String.class, Object.class));
    System.out.print("\nrequest finish"+System.currentTimeMillis());
  }


  @Test
  public void testFetchStatus() throws Exception{
    WXStreamModule streamModule = createModule(new IWXHttpAdapter() {
      @Override
      public void sendRequest(WXRequest request, OnHttpListener listener) {
        WXResponse response = new WXResponse();
        response.statusCode = "-1";
        listener.onHttpFinish(response);
      }
    });
    Callback finish = new Callback();

    streamModule.fetch("",finish,null);
    assertEquals(finish.mData.get(WXStreamModule.STATUS_TEXT),Status.ERR_INVALID_REQUEST);

    streamModule.fetch("{method: 'POST',url: 'http://httpbin.org/post',type:'json'}",finish,null);
    assertEquals(finish.mData.get(WXStreamModule.STATUS_TEXT),Status.ERR_CONNECT_FAILED);

    streamModule = createModule(new IWXHttpAdapter() {
      @Override
      public void sendRequest(WXRequest request, OnHttpListener listener) {
        WXResponse response = new WXResponse();
        response.statusCode = "302";
        listener.onHttpFinish(response);
      }
    });
    streamModule.fetch("{method: 'POST',url: 'http://httpbin.org/post',type:'json'}",finish,null);
    assertEquals(finish.mData.get(WXStreamModule.STATUS),302);
    assertEquals(finish.mData.get(WXStreamModule.STATUS).getClass(),Integer.class);
    assertEquals(finish.mData.get(WXStreamModule.STATUS_TEXT),Status.getStatusText("302"));
  }

}
