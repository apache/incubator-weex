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
package com.taobao.weex.ui.component;

import android.annotation.SuppressLint;
import android.text.TextUtils;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.webkit.WebView;
import android.widget.FrameLayout;
import android.widget.ImageView;

import com.taobao.weex.R;
import com.taobao.weex.IWXRenderListener;
import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.annotation.Component;
import com.taobao.weex.common.Constants;
import com.taobao.weex.common.OnWXScrollListener;
import com.taobao.weex.common.WXErrorCode;
import com.taobao.weex.common.WXRenderStrategy;
import com.taobao.weex.instance.InstanceOnFireEventInterceptor;
import com.taobao.weex.performance.WXInstanceApm;
import com.taobao.weex.ui.action.BasicComponentData;
import com.taobao.weex.utils.WXLogUtils;
import com.taobao.weex.utils.WXUtils;
import com.taobao.weex.utils.WXViewUtils;

import java.util.ArrayDeque;
import java.util.Comparator;
import java.util.Map;
import java.util.PriorityQueue;
import java.util.Queue;

@Component(lazyload = false)
public class WXEmbed extends WXDiv implements WXSDKInstance.OnInstanceVisibleListener,NestedContainer {

  public static final  String STRATEGY_NONE =  "none";
  public static final  String STRATEGY_NORMAL =  "normal";
  public static final  String STRATEGY_HIGH =  "high";


  public static final  String PRIORITY_LOW =  "low";
  public static final  String PRIORITY_NORMAL =  "normal";
  public static final  String PRIORITY_HIGH =  "high";

  public static final String ITEM_ID = "itemId";

  private String src;
  protected WXSDKInstance mNestedInstance;
  private static int ERROR_IMG_WIDTH = (int) WXViewUtils.getRealPxByWidth(270,750);
  private static int ERROR_IMG_HEIGHT = (int) WXViewUtils.getRealPxByWidth(260,750);

  private boolean mIsVisible = true;
  private EmbedRenderListener mListener;
  private EmbedInstanceOnScrollFireEventInterceptor mInstanceOnScrollFireEventInterceptor;


  private String priority = PRIORITY_NORMAL;

  private String strategy = "normal";  //none, normal, high(ignore priority)

  private long hiddenTime;



  public interface EmbedManager {
    WXEmbed getEmbed(String itemId);
    void putEmbed(String itemId,WXEmbed comp);
  }

  public static class FailToH5Listener extends ClickToReloadListener {
    @SuppressLint("SetJavaScriptEnabled")
    @Override
    public void onException(NestedContainer comp, String errCode, String msg) {
      //downgrade embed
      if( errCode != null && comp instanceof WXEmbed && errCode.startsWith("1|")) {
        ViewGroup container = comp.getViewContainer();
        WebView webView = new WebView(container.getContext());
        ViewGroup.LayoutParams params = new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT);
        webView.setLayoutParams(params);
        webView.getSettings().setJavaScriptEnabled(true);

        //WebView Remote Code Execution Vulnerability
        webView.removeJavascriptInterface("searchBoxJavaBridge_");
        webView.removeJavascriptInterface("accessibility");
        webView.removeJavascriptInterface("accessibilityTraversal");
        webView.getSettings().setSavePassword(false);

        container.removeAllViews();
        container.addView(webView);
        webView.loadUrl(((WXEmbed) comp).src);
      }else{
        super.onException(comp,errCode,msg);
      }
    }
  }

  /**
   * Default event listener.
   */
  public static class ClickToReloadListener implements OnNestedInstanceEventListener {
    @Override
    public void onException(NestedContainer container, String errCode, String msg) {
      if (TextUtils.equals(errCode, WXErrorCode.
              WX_DEGRAD_ERR_NETWORK_BUNDLE_DOWNLOAD_FAILED.getErrorCode()) && container instanceof WXEmbed) {
        final WXEmbed comp = ((WXEmbed)container);
        final ImageView imageView = new ImageView(comp.getContext());
        imageView.setImageResource(R.drawable.weex_error);
        FrameLayout.LayoutParams layoutParams = new FrameLayout.LayoutParams(ERROR_IMG_WIDTH, ERROR_IMG_HEIGHT);
        layoutParams.gravity = Gravity.CENTER;
        imageView.setLayoutParams(layoutParams);
        imageView.setScaleType(ImageView.ScaleType.FIT_XY);
        imageView.setAdjustViewBounds(true);
        imageView.setOnClickListener(new View.OnClickListener() {
          @Override
          public void onClick(View v) {
            imageView.setOnClickListener(null);
            imageView.setEnabled(false);
            comp.loadContent();
          }
        });
        FrameLayout hostView = comp.getHostView();
        hostView.removeAllViews();
        hostView.addView(imageView);
        WXLogUtils.e("WXEmbed", "NetWork failure :" + errCode + ",\n error message :" + msg);
      }
    }

    @Override
    public boolean onPreCreate(NestedContainer comp, String src) {
      return true;
    }

    @Override
    public String transformUrl(String origin) {
      return origin;
    }

    @Override
    public void onCreated(NestedContainer comp, WXSDKInstance nestedInstance) {

    }
  }

  static class EmbedRenderListener implements IWXRenderListener {
    WXEmbed mComponent;
    OnNestedInstanceEventListener mEventListener;

    EmbedRenderListener(WXEmbed comp) {
      mComponent = comp;
      mEventListener = new ClickToReloadListener();
    }

    @Override
    public void onViewCreated(WXSDKInstance instance, View view) {
      FrameLayout hostView = mComponent.getHostView();
      hostView.removeAllViews();
      hostView.addView(view);
    }

    @Override
    public void onRenderSuccess(WXSDKInstance instance, int width, int height) {

    }

    @Override
    public void onRefreshSuccess(WXSDKInstance instance, int width, int height) {

    }

    @Override
    public void onException(WXSDKInstance instance, String errCode, String msg) {
      if (mEventListener != null) {
        mEventListener.onException(mComponent, errCode, msg);
      }
    }
  }

  @Deprecated
  public WXEmbed(WXSDKInstance instance, WXVContainer parent, String instanceId, boolean isLazy, BasicComponentData basicComponentData) {
    this(instance, parent, basicComponentData);
  }

  public WXEmbed(WXSDKInstance instance, WXVContainer parent, BasicComponentData basicComponentData) {
    super(instance, parent, basicComponentData);
    mListener = new EmbedRenderListener(this);
    mInstanceOnScrollFireEventInterceptor = new EmbedInstanceOnScrollFireEventInterceptor(this);

    ERROR_IMG_WIDTH = (int) WXViewUtils.getRealPxByWidth(270, instance.getInstanceViewPortWidth());
    ERROR_IMG_HEIGHT = (int) WXViewUtils.getRealPxByWidth(260, instance.getInstanceViewPortWidth());
    if (instance instanceof EmbedManager) {
      Object itemId = getAttrs().get(ITEM_ID);
      if (itemId != null) {
        ((EmbedManager) instance).putEmbed(itemId.toString(), this);
      }
    }
    this.priority = WXUtils.getString(getAttrs().get(Constants.Name.PRIORITY), PRIORITY_NORMAL);
    this.strategy = WXUtils.getString(getAttrs().get(Constants.Name.STRATEGY), STRATEGY_NONE);
    instance.getApmForInstance().updateDiffStats(WXInstanceApm.KEY_PAGE_STATS_EMBED_COUNT,1);
  }

  @Override
  public void setOnNestEventListener(OnNestedInstanceEventListener listener){
    mListener.mEventListener = listener;
  }

  @Override
  public ViewGroup getViewContainer() {
    return getHostView();
  }

  @Override
  protected boolean setProperty(String key, Object param) {
    switch (key) {
      case Constants.Name.SRC:
        String src = WXUtils.getString(param, null);
        if (src != null)
          setSrc(src);
        return true;
      case Constants.Name.PRIORITY:
        String priority = WXUtils.getString(param, null);
        if (priority != null) {
          setPriority(priority);
        }
        return true;
    }
    return super.setProperty(key, param);
  }

  @Override
  public void renderNewURL(String url) {
    src = url;
    loadContent();
  }

  @Override
  public void reload() {
    if (!TextUtils.isEmpty(src)) {
      loadContent();
    }
  }

  public String getOriginUrl() {
    return originUrl;
  }

  public void setOriginUrl(String originUrl) {
    this.originUrl = originUrl;
  }

  private String originUrl;


  @Override
  public void addEvent(String type) {
    super.addEvent(type);
    if(Constants.Event.SCROLL_START.equals(type)){
      mInstanceOnScrollFireEventInterceptor.addInterceptEvent(type);
    }else if(Constants.Event.SCROLL_END.equals(type)){
      mInstanceOnScrollFireEventInterceptor.addInterceptEvent(type);
    }else if(Constants.Event.SCROLL.equals(type)){
      mInstanceOnScrollFireEventInterceptor.addInterceptEvent(type);
    }
  }

  @WXComponentProp(name = Constants.Name.SRC)
  public void setSrc(String src) {
    originUrl=src;
    this.src = src;
    if (mNestedInstance != null) {
      mNestedInstance.destroy();
      mNestedInstance = null;
    }
    if (mIsVisible && !TextUtils.isEmpty(this.src)) {
      loadContent();
    }
  }
  public String getSrc() {
    return src;
  }


  @WXComponentProp(name = Constants.Name.PRIORITY)
  public void setPriority(String priority) {
    if(TextUtils.isEmpty(priority)){
      return;
    }
    this.priority = priority;
  }

  /**
   * Load embed content, default behavior is create a nested instance.
   */
  protected void loadContent(){
    if(mNestedInstance != null){
      mNestedInstance.destroy();
    }
    mNestedInstance = createInstance();
    if(mListener != null && mListener.mEventListener != null){
      if(!mListener.mEventListener.onPreCreate(this,src)){
        //cancel render
        mListener.mEventListener.onCreated(this, mNestedInstance);
      }
    }
  }

  private static final int getLevel(WXEmbed embed){
    String priority = embed.priority;
    String strategy = embed.strategy;
    int level  = 5;
    if(!STRATEGY_HIGH.equals(strategy)) {
      if (TextUtils.equals(priority, PRIORITY_LOW)) {
        level = 0;
      } else if (TextUtils.equals(priority, PRIORITY_HIGH)) {
        level = 10;
      }
    }
    return  level;
  }

  private WXSDKInstance createInstance() {
    WXSDKInstance sdkInstance = getInstance().createNestedInstance(this);
    sdkInstance.setParentInstance(getInstance());
    boolean needsAdd = !getAttrs().containsKey("disableInstanceVisibleListener");
    if(needsAdd){ //prevent switch off fire viewappear event twice
        getInstance().addOnInstanceVisibleListener(this);
    }
    sdkInstance.registerRenderListener(mListener);
    mInstanceOnScrollFireEventInterceptor.resetFirstLaterScroller();;
    sdkInstance.addInstanceOnFireEventInterceptor(mInstanceOnScrollFireEventInterceptor);
    sdkInstance.registerOnWXScrollListener(mInstanceOnScrollFireEventInterceptor);

    String url=src;
    if(mListener != null && mListener.mEventListener != null){
      url=mListener.mEventListener.transformUrl(src);
      if(!mListener.mEventListener.onPreCreate(this,src)){
        //cancel render
        return null;
      }
    }

    if(TextUtils.isEmpty(url)){
      mListener.mEventListener.onException(this,
              WXErrorCode.WX_DEGRAD_ERR_BUNDLE_CONTENTTYPE_ERROR.getErrorCode(),
              WXErrorCode.WX_DEGRAD_ERR_BUNDLE_CONTENTTYPE_ERROR.getErrorMsg() + "!!wx embed src url is null"
      );
      return sdkInstance;
    }
    sdkInstance.setContainerInfo(WXInstanceApm.KEY_PAGE_PROPERTIES_INSTANCE_TYPE,"embed");
    sdkInstance.setContainerInfo(WXInstanceApm.KEY_PAGE_PROPERTIES_PARENT_PAGE,getInstance().getWXPerformance().pageName);
    sdkInstance.renderByUrl(url,
            url,
            null, null,
            WXRenderStrategy.APPEND_ASYNC);

    return sdkInstance;
  }

  @Override
  public void setVisibility(String visibility) {
    super.setVisibility(visibility);
    boolean visible = TextUtils.equals(visibility, Constants.Value.VISIBLE);
    if(mIsVisible != visible){

      if (!TextUtils.isEmpty(src) && visible) {
        if (mNestedInstance == null) {
          loadContent();
        } else {
          mNestedInstance.onViewAppear();
        }
      }

      if (!visible) {
        if (mNestedInstance != null) {
          mNestedInstance.onViewDisappear();
        }
      }
      mIsVisible = visible;
      doAutoEmbedMemoryStrategy();
    }
  }

  @Override
  public void destroy() {
    super.destroy();
    destoryNestInstance();
    src = null;
    if (getInstance() != null) {
      getInstance().removeOnInstanceVisibleListener(this);
    }
  }

  private void  doAutoEmbedMemoryStrategy(){
    /**
     * auto manage embed amount in current instance, save memory
     * */
    if(!STRATEGY_NONE.equals(this.strategy)){
      if(!mIsVisible && mNestedInstance != null){
        if(PRIORITY_LOW.equals(this.priority)){
          destoryNestInstance();
        }else{
          if(getInstance().hiddenEmbeds == null){ // low is in front, when priority is same, hidden time pre in first
            getInstance().hiddenEmbeds = new PriorityQueue<>(8, new Comparator<WXEmbed>() {
              @Override
              public int compare(WXEmbed o1, WXEmbed o2) {
                int level =  getLevel(o1) - getLevel(o2);
                if(level != 0){
                  return  level;
                }
                return (int) (o1.hiddenTime - o2.hiddenTime);
              }
            });
          }
          //getInstance().hiddenEmbeds.remove(this);
          if(!getInstance().hiddenEmbeds.contains(this)) {
            this.hiddenTime = System.currentTimeMillis();
            getInstance().hiddenEmbeds.add(this);
          }
          if(getInstance().hiddenEmbeds != null && getInstance().getMaxHiddenEmbedsNum() >= 0){
            while (getInstance().hiddenEmbeds.size() > getInstance().getMaxHiddenEmbedsNum()){
              WXEmbed embed = getInstance().hiddenEmbeds.poll();
              if(embed.mIsVisible){
                continue;
              }
              if(embed != null) {
                embed.destoryNestInstance();
              }
            }
          }
        }
      }
      if(mIsVisible && mNestedInstance != null){
        if(getInstance().hiddenEmbeds != null && getInstance().hiddenEmbeds.contains(this)){
          getInstance().hiddenEmbeds.remove(this);
        }
      }
    }

  }

  @Override
  public void onAppear() {
    //appear event from root instance will not trigger visibility change
    if(mIsVisible && mNestedInstance != null){
      WXComponent comp = mNestedInstance.getRootComponent();
      if(comp != null)
        comp.fireEvent(Constants.Event.VIEWAPPEAR);
    }
  }

  @Override
  public void onDisappear() {
    //appear event from root instance will not trigger visibility change
    if(mIsVisible && mNestedInstance != null){
      WXComponent comp = mNestedInstance.getRootComponent();
      if(comp != null)
        comp.fireEvent(Constants.Event.VIEWDISAPPEAR);
    }
  }

  @Override
  public void onActivityStart() {
    super.onActivityStart();
    if (mNestedInstance != null) {
      mNestedInstance.onActivityStart();
    }
  }

  @Override
  public void onActivityResume() {
    super.onActivityResume();
    if (mNestedInstance != null) {
      mNestedInstance.onActivityResume();
    }
  }

  @Override
  public void onActivityPause() {
    super.onActivityPause();
    if (mNestedInstance != null) {
      mNestedInstance.onActivityPause();
    }
  }

  @Override
  public void onActivityStop() {
    super.onActivityStop();
    if (mNestedInstance != null) {
      mNestedInstance.onActivityStop();
    }
  }

  @Override
  public void onActivityDestroy() {
    super.onActivityDestroy();
    if (mNestedInstance != null) {
      mNestedInstance.onActivityDestroy();
    }
  }

  public void setStrategy(String strategy) {
    this.strategy = strategy;
  }

  private void destoryNestInstance(){
    if(getInstance().hiddenEmbeds != null && getInstance().hiddenEmbeds.contains(this)){
      getInstance().hiddenEmbeds.remove(this);
    }
    if (mNestedInstance != null) {
      mNestedInstance.destroy();
      mNestedInstance = null;
    }
    if(WXEnvironment.isApkDebugable()){
      WXLogUtils.w("WXEmbed destoryNestInstance priority " + priority + " index " + getAttrs().get("index")
              + "  " + hiddenTime  + " embeds size " + (getInstance().hiddenEmbeds == null ?  0 : getInstance().hiddenEmbeds.size())
              + " strategy " + this.strategy);
    }
  }

  @Override
  public void addLayerOverFlowListener(String ref) {
    if (mNestedInstance != null)
      mNestedInstance.addLayerOverFlowListener(getRef());
  }

  @Override
  public void removeLayerOverFlowListener(String ref) {
    if (mNestedInstance != null)
      mNestedInstance.removeLayerOverFlowListener(ref);
  }

  static class EmbedInstanceOnScrollFireEventInterceptor extends InstanceOnFireEventInterceptor implements OnWXScrollListener{

    private WXEmbed mEmbed;
    private WXComponent firstLayerScroller;

    public EmbedInstanceOnScrollFireEventInterceptor(WXEmbed embed) {
      this.mEmbed = embed;
    }

    public void resetFirstLaterScroller(){
      firstLayerScroller = null;
    }

    @Override
    public void onFireEvent(String instanceId, String elementRef, String type, Map<String, Object> params, Map<String, Object> domChanges) {
      if(mEmbed == null
              || mEmbed.mNestedInstance == null
              || !mEmbed.mNestedInstance.getInstanceId().equals(instanceId)){
        return;
      }
      if(firstLayerScroller == null){
          initFirstLayerScroller();
      }
      if(firstLayerScroller == null){
        return;
      }
      if(firstLayerScroller.getRef().equals(elementRef)){
         mEmbed.getInstance().fireEvent(mEmbed.getRef(), type, params, domChanges);
      }
    }


    private void initFirstLayerScroller(){
      if(firstLayerScroller == null){
          firstLayerScroller = findFirstLayerScroller();
          if(firstLayerScroller != null){
            for(String event : getListenEvents()){
              if(!firstLayerScroller.containsEvent(event)){
                  firstLayerScroller.getEvents().add(event);
                  firstLayerScroller.addEvent(event);
              }
            }
          }
      }
    }

    /**
     * get first layer scroller ref
     * */
    private WXComponent findFirstLayerScroller(){
         if(mEmbed.mNestedInstance == null){
           return null;
         }
         WXComponent rootComponent = mEmbed.mNestedInstance.getRootComponent();
        if(rootComponent instanceof  Scrollable){
          return rootComponent;
        }
        Queue<WXComponent> queues = new ArrayDeque<>();
        queues.offer(rootComponent);
        while (!queues.isEmpty()){
             WXComponent component = queues.poll();
             if(component == null){
               break;
             }
             if(component instanceof  Scrollable){
                return  component;
             }
             if(component instanceof WXVContainer){
               WXVContainer container = (WXVContainer) component;
               for(int i=0; i<container.getChildCount(); i++){
                  queues.offer(container.getChild(i));
               }
             }
        }
        return null;
    }


    @Override
    public void onScrolled(View view, int x, int y) {
      if(firstLayerScroller != null){
        return;
      }
      if(getListenEvents().size() > 0){
        initFirstLayerScroller();
      }
    }

    @Override
    public void onScrollStateChanged(View view, int x, int y, int newState) {

    }
  }

}
