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

import android.text.TextUtils;
import android.util.Pair;

import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.dom.action.Actions;
import com.taobao.weex.dom.flex.CSSLayoutContext;
import com.taobao.weex.tracing.Stopwatch;
import com.taobao.weex.tracing.WXTracing;
import com.taobao.weex.ui.IWXRenderTask;
import com.taobao.weex.ui.WXRenderManager;
import com.taobao.weex.ui.animation.WXAnimationBean;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXVContainer;
import com.taobao.weex.utils.WXLogUtils;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedHashSet;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

/**
 * <p>
 * This class is responsible for creating command object of DOM operation and
 * invoking command of corresponding object.
 * </p>
 * <p>
 * In the command design pattern,
 * this class acts as the <strong>invoker </strong>in the command pattern
 * despite that it is also responsible for creating <strong>Command</strong> object.
 * And,{@link IWXRenderTask} works as the <strong>Command</strong>
 * {@link WXDomManager} works as the <strong>Client</strong>
 * {@link WXRenderManager} works as the <strong>Receiver</strong>.
 * </p>
 * <p>
 * There exists one to one correspondence between DOMActionContextImpl and WXInstance,
 * and {@link WXDomManager} is responsible for manage the relation of correspondence.
 * </p>
 */
class DOMActionContextImpl implements DOMActionContext {
  /** package **/ final ConcurrentHashMap<String, WXDomObject> mRegistry;
  private WXDomObject.Consumer mAddDOMConsumer;
  private WXDomObject.Consumer mUnregisterDomConsumer;
  private String mInstanceId;
  private WXRenderManager mWXRenderManager;
  private ArrayList<IWXRenderTask> mNormalTasks;
  private Set <Pair<String, Map<String, Object>>> animations;
  private CSSLayoutContext mLayoutContext;
  private volatile boolean mDirty;
  private boolean mDestroy;
  private Map<String, AddDomInfo> mAddDom = new HashMap<>();

  /**
   * Create an instance of {@link DOMActionContextImpl},
   * One {@link WXSDKInstance} corresponding to one and only one {@link DOMActionContextImpl}.
   * And all the instance of {@link WXDomManager} share the same {@link WXRenderManager}.
   * @param instanceId the id of the {@link WXSDKInstance}.
   *                   One {@link WXSDKInstance} corresponding to one {@link DOMActionContextImpl},
   *                   and vice versa.
   * @param renderManager This acts as the Receiver of the command pattern
   */
  public DOMActionContextImpl(String instanceId, WXRenderManager renderManager) {
    mDestroy = false;
    mInstanceId = instanceId;
    mLayoutContext = new CSSLayoutContext();
    mRegistry = new ConcurrentHashMap<>();
    mNormalTasks = new ArrayList<>();
    animations = new LinkedHashSet<>();
    mWXRenderManager = renderManager;
    mAddDOMConsumer = new AddDOMConsumer(mRegistry);
    mUnregisterDomConsumer = new RemoveElementConsumer(mRegistry);
  }

  @Override
  public String getInstanceId() {
    return mInstanceId;
  }

  @Override
  public WXDomObject.Consumer getAddDOMConsumer(){
    return mAddDOMConsumer;
  }

  @Override
  public WXDomObject.Consumer getRemoveElementConsumer() {
    return mUnregisterDomConsumer;
  }

  @Override
  public WXDomObject.Consumer getApplyStyleConsumer() {
    return ApplyStyleConsumer.getInstance();
  }

  @Override
  public void addDomInfo(String ref, WXComponent component) {
    AddDomInfo addDomInfo = new AddDomInfo();
    addDomInfo.component = component;
    mAddDom.put(ref, addDomInfo);
  }

  /**
   * Destroy current instance, which occurred when {@link WXSDKInstance#destroy()} is called.
   */
  public void destroy() {
    mDestroy = true;
    mRegistry.clear();
    mAddDOMConsumer = null;
    mNormalTasks.clear();
    mAddDom.clear();
    mLayoutContext = null;
    mWXRenderManager = null;
    animations.clear();
  }

  /**
   * Rebuild the component tree.
   * The purpose of this method is moving fixed components to the root component.
   * This method will be called when {@link #batch()} is executed.
   * @param root root dom
   */
  void rebuildingFixedDomTree(WXDomObject root) {
    if (root != null && root.getFixedStyleRefs() != null) {
      int size = root.getFixedStyleRefs().size();
      for (int i = 0; i < size; i++) {
        String fixedRef = root.getFixedStyleRefs().get(i);
        WXDomObject wxDomObject = mRegistry.get(fixedRef);
        if (wxDomObject!=null && wxDomObject.parent != null) {
          wxDomObject.parent.remove(wxDomObject);
          root.add(wxDomObject, -1);
        }
      }
    }

  }

  /**
   * Batch the execution of command objects and execute all the command objects created other
   * places, e.g. call {@link IWXRenderTask#execute()}.
   * First, it will rebuild the dom tree and do pre layout staff.
   * Then call {@link com.taobao.weex.dom.flex.CSSNode#calculateLayout(CSSLayoutContext)} to
   * start calculate layout.
   * Next, call {@link ApplyUpdateConsumer} to get changed dom and creating
   * corresponding command object.
   * Finally, walk through the queue, e.g. call {@link IWXRenderTask#execute()} for every task
   * in the queue.
   */
  void batch() {

    if (!mDirty || mDestroy) {
      return;
    }
    long start = System.currentTimeMillis();
    long startNanos = System.nanoTime();
    WXDomObject rootDom = mRegistry.get(WXDomObject.ROOT);
    layout(rootDom);

    if (WXTracing.isAvailable()) {
      WXTracing.TraceEvent batchEvent = WXTracing.newEvent("domBatch", mInstanceId, -1);
      batchEvent.duration = Stopwatch.millisUntilNow(startNanos);
      batchEvent.ts = start;
      batchEvent.ph = "X";
      WXTracing.submit(batchEvent);
    }
    if(WXEnvironment.isApkDebugable()){
       WXLogUtils.d("mInstanceId  " + mInstanceId  + " batch used " + (System.currentTimeMillis() - start));
    }
  }

  void layout(WXDomObject rootDom) {
    if (rootDom == null) {
      return;
    }
    long start0 = System.currentTimeMillis();
    rebuildingFixedDomTree(rootDom);

    rootDom.traverseTree( new WXDomObject.Consumer() {
      @Override
      public void accept(WXDomObject dom) {
        if (!dom.hasUpdate() || mDestroy) {
          return;
        }
        dom.layoutBefore();
      }
    });
    long start = System.currentTimeMillis();

    rootDom.calculateLayout(mLayoutContext);

    WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(mInstanceId);
    if (instance != null) {
      instance.cssLayoutTime(System.currentTimeMillis() - start);
    }

    start = System.currentTimeMillis();
    rootDom.traverseTree( new WXDomObject.Consumer() {
      @Override
      public void accept(WXDomObject dom) {
        if (!dom.hasUpdate() || mDestroy) {
          return;
        }
        dom.layoutAfter();
      }
    }, new ApplyUpdateConsumer());


    if (instance != null) {
      instance.applyUpdateTime(System.currentTimeMillis() - start);
    }

    start = System.currentTimeMillis();
    updateDomObj();
    if (instance != null) {
      instance.updateDomObjTime(System.currentTimeMillis() - start);
    }
    parseAnimation();

    boolean isPreRenderMode = instance != null && instance.isPreRenderMode();
    if(!isPreRenderMode) {
      consumeRenderTasks();
    }
    mAddDom.clear();
    animations.clear();
    mDirty = false;
    if (instance != null) {
      instance.batchTime(System.currentTimeMillis() - start0);
    }
  }

  void consumeRenderTasks() {
    int count = mNormalTasks.size();
    for (int i = 0; i < count && !mDestroy; ++i) {
      mWXRenderManager.runOnThread(mInstanceId, mNormalTasks.get(i));
    }
    mNormalTasks.clear();
  }

  private class ApplyUpdateConsumer implements WXDomObject.Consumer{

    @Override
    public void accept(WXDomObject dom) {
      if (dom.hasUpdate()) {
        dom.markUpdateSeen();
        if (!dom.isYoung()) {
          final WXDomObject copy = dom.clone();
          if (copy == null) {
            return;
          }
          mNormalTasks.add(new IWXRenderTask() {

            @Override
            public void execute() {
              mWXRenderManager.setLayout(mInstanceId, copy.getRef(), copy);
              if(copy.getExtra() != null) {
                mWXRenderManager.setExtra(mInstanceId, copy.getRef(), copy.getExtra());
              }
            }

            @Override
            public String toString() {
              return "setLayout & setExtra";
            }
          });
        }
      }
    }
  }

  private void parseAnimation() {
    for (final Pair<String, Map<String, Object>> pair : animations) {
      if (!TextUtils.isEmpty(pair.first)) {
        final WXAnimationBean animationBean = createAnimationBean(pair.first, pair.second);
        if (animationBean != null) {
          postRenderTask(Actions.getAnimationAction(pair.first, animationBean));
        }
      }
    }
  }

  /**
   * Update all components' dom info stored in {@link #mAddDom}
   */
  private void updateDomObj() {
    long start = System.currentTimeMillis();
    Iterator<Map.Entry<String, AddDomInfo>> iterator = mAddDom.entrySet().iterator();
    Map.Entry<String, AddDomInfo> entry;
    AddDomInfo value;
    while (iterator.hasNext()) {
      entry = iterator.next();
      value = entry.getValue();
      updateDomObj(value.component);
    }
    if (WXEnvironment.isApkDebugable()) {
      WXLogUtils.d("updateDomObj", "time:" + (System.currentTimeMillis() - start));
    }
  }

  /**
   * Update the specified component's dom and mark it as old.
   * @param component the component to be updated
   */
  private void updateDomObj(WXComponent component) {
    if (component == null) {
      return;
    }
    WXDomObject domObject = mRegistry.get(component.getRef());
    if (domObject == null) {
      return;
    }
    domObject.old();
    component.updateDom(domObject);
    if (component instanceof WXVContainer) {
      WXVContainer container = (WXVContainer) component;
      int count = container.childCount();
      for (int i = 0; i < count; ++i) {
        updateDomObj(container.getChild(i));
      }
    }
  }

  @Override
  public void addAnimationForElement(String ref, Map<String, Object> animMap) {
    animations.add(new Pair<>(ref,animMap));
    mDirty = true;
  }

  @Override
  public void postRenderTask(RenderAction action) {
    mNormalTasks.add(new RenderActionTask(action, mWXRenderManager.getRenderContext(mInstanceId)));
    mDirty = true;
  }

  @Override
  public void registerDOMObject(String ref, WXDomObject obj) {
    mRegistry.put(ref,obj);
  }

  @Override
  public void unregisterDOMObject(String ref) {
    mRegistry.remove(ref);
  }

  @Override
  public void registerComponent(String ref, WXComponent comp) {
    mWXRenderManager.registerComponent(mInstanceId,ref,comp);
  }

  @Override
  public WXComponent getCompByRef(String ref) {
    return mWXRenderManager.getWXComponent(mInstanceId,ref);
  }

  @Override
  public boolean isDestory() {
    return false;
  }

  @Override
  public void markDirty() {
    if(!mDestroy){
      if(!mDirty){
        mDirty = true;
      }
    }
  }

  @Override
  public WXSDKInstance getInstance() {
    return mWXRenderManager.getWXSDKInstance(mInstanceId);
  }

  @Override
  public WXDomObject getDomByRef(String ref) {
    return mRegistry.get(ref);
  }

  private WXAnimationBean createAnimationBean(String ref,Map<String, Object> style){
    if (style != null) {
      try {
        Object transform = style.get(WXDomObject.TRANSFORM);
        if (transform instanceof String && !TextUtils.isEmpty((String) transform)) {
          String transformOrigin = (String) style.get(WXDomObject.TRANSFORM_ORIGIN);
          WXAnimationBean animationBean = new WXAnimationBean();
          WXDomObject domObject = mRegistry.get(ref);
          int width = (int) domObject.getLayoutWidth();
          int height = (int) domObject.getLayoutHeight();
          animationBean.styles = new WXAnimationBean.Style();
          animationBean.styles.init(transformOrigin, (String) transform, width, height,WXSDKManager.getInstanceViewPortWidth(mInstanceId));
          return animationBean;
        }
      }catch (RuntimeException e){
        WXLogUtils.e("", e);
        return null;
      }
    }
    return null;
  }

  private static class RemoveElementConsumer implements WXDomObject.Consumer {
    final ConcurrentHashMap<String, WXDomObject> mRegistry;

    RemoveElementConsumer(ConcurrentHashMap<String, WXDomObject> r) {
      mRegistry = r;
    }


    @Override
    public void accept(WXDomObject dom) {
      mRegistry.remove(dom.getRef());
    }
  }


  private static class AddDOMConsumer implements WXDomObject.Consumer {
    final ConcurrentHashMap<String, WXDomObject> mRegistry;
    AddDOMConsumer(ConcurrentHashMap<String, WXDomObject> r){
      mRegistry = r;
    }

    @Override
    public void accept(WXDomObject dom) {
      //register dom
      dom.young();
      mRegistry.put(dom.getRef(), dom);

      //find fixed node
      WXDomObject rootDom = mRegistry.get(WXDomObject.ROOT);
      if (rootDom != null && dom.isFixed()) {
        rootDom.add2FixedDomList(dom.getRef());
      }
    }
  }

  static class AddDomInfo {

    public WXComponent component;
  }
}
