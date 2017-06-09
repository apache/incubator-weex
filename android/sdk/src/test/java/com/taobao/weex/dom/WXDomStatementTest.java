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
package com.taobao.weex.dom;

import com.alibaba.fastjson.JSONObject;
import com.taobao.weappplus_sdk.BuildConfig;
import com.taobao.weex.InitConfig;
import com.taobao.weex.WXSDKEngine;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKInstanceTest;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.WXSDKManagerTest;
import com.taobao.weex.bridge.WXBridgeManagerTest;
import com.taobao.weex.dom.action.TestActions;
import com.taobao.weex.ui.WXRenderManager;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PowerMockIgnore;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.RuntimeEnvironment;
import org.robolectric.annotation.Config;
import org.robolectric.shadows.ShadowLooper;

/**
 * Created by sospartan on 8/29/16.
 */
@RunWith(RobolectricTestRunner.class)
@Config(constants = BuildConfig.class, sdk = 19)
@PowerMockIgnore( {"org.mockito.*", "org.robolectric.*", "android.*"})
public class WXDomStatementTest {

  DOMActionContextImpl stmt;
  WXRenderManager rednerManager;
  WXSDKInstance instance;

  @Before
  public void setUp() throws Exception {
    WXSDKEngine.initialize(RuntimeEnvironment.application,new InitConfig.Builder().build());
    ShadowLooper looper = WXBridgeManagerTest.getLooper();
    looper.idle();
    ShadowLooper.runUiThreadTasksIncludingDelayedTasks();
    instance = WXSDKInstanceTest.createInstance();
    rednerManager = new WXRenderManager();
    rednerManager.registerInstance(instance);//
    WXSDKManagerTest.setRenderManager(rednerManager);
    stmt = new DOMActionContextImpl(instance.getInstanceId(),rednerManager );
  }

  @After
  public void tearDown() throws Exception {
    stmt.destroy();
  }

  void createBody(){
    JSONObject body = new JSONObject();
    body.put("type","div");
    body.put("ref",WXDomObject.ROOT);
    WXSDKManager.getInstance().getWXDomManager().executeAction(instance.getInstanceId(), TestActions.body(body),true);
  }

  void addDom(JSONObject obj,String parentRef,int index){
    WXSDKManager.getInstance().getWXDomManager().executeAction(instance.getInstanceId(),TestActions.addDom(obj,parentRef,index),false);
  }

  void removeDom(String ref){
    WXSDKManager.getInstance().getWXDomManager().executeAction(instance.getInstanceId(),TestActions.remove(ref),false);
  }

  void updateAttrs(String ref,JSONObject data){
    WXSDKManager.getInstance().getWXDomManager().executeAction(instance.getInstanceId(),TestActions.updateAttr(ref,data),false);
  }

  void updateStyle(String ref,JSONObject data,boolean byPesudo){
    WXSDKManager.getInstance().getWXDomManager().executeAction(instance.getInstanceId(),TestActions.updateStyle(ref,data,byPesudo),false);
  }

  void moveDom(String ref,String parent,int index){
    WXSDKManager.getInstance().getWXDomManager().executeAction(instance.getInstanceId(),TestActions.moveDom(ref,parent,index),false);
  }

  private void scrollToDom(String s, JSONObject o) {
    WXSDKManager.getInstance().getWXDomManager().executeAction(instance.getInstanceId(),TestActions.scrollTo(s,o),false);
  }

  private void addEvent(String ref,String event){
    WXSDKManager.getInstance().getWXDomManager().executeAction(instance.getInstanceId(),TestActions.addEvent(ref,event),false);
  }

  private void removeEvent(String ref,String event){
    WXSDKManager.getInstance().getWXDomManager().executeAction(instance.getInstanceId(),TestActions.removeEvent(ref,event),false);
  }

  private void updateFinish(){
    WXSDKManager.getInstance().getWXDomManager().executeAction(instance.getInstanceId(),TestActions.updateFinish(),false);
  }

  private void createFinish(){
    WXSDKManager.getInstance().getWXDomManager().executeAction(instance.getInstanceId(),TestActions.createFinish(),false);
  }

  private void refreshFinish(){
    WXSDKManager.getInstance().getWXDomManager().executeAction(instance.getInstanceId(),TestActions.refreshFinish(),false);
  }

  @Test
  public void testCustomDomObject() throws Exception {
    WXDomObject root = new TestDomObject();
    root.add(new WXListDomObject(),0);
    root.add(new WXScrollerDomObject(),0);
    root.add(new WXTextDomObject(),0);
    root.add(new WXSwitchDomObject(),0);
    root.add(new TextAreaEditTextDomObject(),0);
    stmt.layout(root);
    root.traverseTree(ApplyStyleConsumer.getInstance());
  }

  @Test
  public void testCreateBody() throws Exception {
    createBody();

    stmt.batch();
  }

  @Test
  public void testAddDom() throws Exception {
    createBody();

    JSONObject obj = new JSONObject();
    obj.put("type","div");
    obj.put("ref","100");

    addDom(obj,WXDomObject.ROOT,0);

    stmt.batch();
  }

  @Test
  public void testMoveDom() throws Exception {
    createBody();
    JSONObject obj;
    obj = new JSONObject();
    obj.put("type","div");
    obj.put("ref","100");
    addDom(obj,WXDomObject.ROOT,0);

    obj = new JSONObject();
    obj.put("type","div");
    obj.put("ref","101");
    addDom(obj,WXDomObject.ROOT,0);

    moveDom("100",WXDomObject.ROOT,1);
    stmt.batch();
  }

  @Test
  public void testRemoveDom() throws Exception {
    createBody();
    JSONObject obj;
    obj = new JSONObject();
    obj.put("type","div");
    obj.put("ref","100");
    addDom(obj,WXDomObject.ROOT,0);

    obj = new JSONObject();
    obj.put("type","div");
    obj.put("ref","101");
    addDom(obj,WXDomObject.ROOT,0);

    removeDom("101");

    stmt.batch();
  }

  @Test
  public void testUpdateAttrs() throws Exception {
    createBody();
    JSONObject obj;
    obj = new JSONObject();
    obj.put("type","div");
    obj.put("ref","100");
    addDom(obj,WXDomObject.ROOT,0);


    updateAttrs("100",new JSONObject());
    updateAttrs("100",null);

    stmt.batch();
  }

  @Test
  public void testUpdateStyle() throws Exception {
    createBody();
    JSONObject obj;
    obj = new JSONObject();
    obj.put("type","div");
    obj.put("ref","100");
    addDom(obj,WXDomObject.ROOT,0);

    updateStyle("100",new JSONObject(),false);
    updateStyle("100",null,false);

    stmt.batch();
  }

  @Test
  public void testAddEvent() throws Exception {
    createBody();
    JSONObject obj;
    obj = new JSONObject();
    obj.put("type","div");
    obj.put("ref","100");
    addDom(obj,WXDomObject.ROOT,0);

    addEvent("100","click");
    addEvent("100",null);

    stmt.batch();
  }

  @Test
  public void testRemoveEvent() throws Exception {
    createBody();
    JSONObject obj;
    obj = new JSONObject();
    obj.put("type","div");
    obj.put("ref","100");
    addDom(obj,WXDomObject.ROOT,0);

    removeEvent("100",null);
    addEvent("100","click");
    removeEvent("100","click");

    stmt.batch();
  }

  @Test
  public void testScrollToDom() throws Exception {
    createBody();
    JSONObject obj;
    obj = new JSONObject();
    obj.put("type","div");
    obj.put("ref","100");
    addDom(obj,WXDomObject.ROOT,0);

    scrollToDom("100",null);
    stmt.batch();
  }



  @Test
  public void testCreateFinish() throws Exception {
    createBody();
    createFinish();

    stmt.batch();
  }

  @Test
  public void testRefreshFinish() throws Exception {
    createBody();
    refreshFinish();

    stmt.batch();
  }

  @Test
  public void testUpdateFinish() throws Exception {
    createBody();

    updateFinish();
    stmt.batch();
  }
}
