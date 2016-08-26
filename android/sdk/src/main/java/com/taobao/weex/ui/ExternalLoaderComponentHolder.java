package com.taobao.weex.ui;

import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.bridge.Invoker;
import com.taobao.weex.bridge.MethodInvoker;
import com.taobao.weex.common.Component;
import com.taobao.weex.common.WXRuntimeException;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXComponentProp;
import com.taobao.weex.ui.component.WXVContainer;
import com.taobao.weex.utils.WXLogUtils;

import java.lang.annotation.Annotation;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.HashMap;
import java.util.Map;

/**
 * Created by sospartan on 8/26/16.
 */
public class ExternalLoaderComponentHolder implements IFComponentHolder {
  public static final String TAG = "SimpleComponentHolder";
  private Map<String, Invoker> mMethods;
  private final IExternalComponentGetter mClzGetter;
  private final String mType;
  private Class mClass;


  public ExternalLoaderComponentHolder(String type,IExternalComponentGetter clzGetter) {
    this.mClzGetter = clzGetter;
    mType = type;
  }

  @Override
  public void loadIfNonLazy() {
  }

  private synchronized void generate(){
    Class clz = mClzGetter.getExternalComponentClass(mType);
    mClass = clz;

    mMethods = SimpleComponentHolder.getMethods(clz);
  }



  @Override
  public synchronized WXComponent createInstance(WXSDKInstance instance, WXDomObject node, WXVContainer parent, boolean lazy) throws IllegalAccessException, InvocationTargetException, InstantiationException {
    if(mClass == null){
      mClass = mClzGetter.getExternalComponentClass(mType);
    }
    ComponentCreator creator = new SimpleComponentHolder.ClazzComponentCreator(mClass);
    WXComponent component = creator.createInstance(instance,node,parent,lazy);

    component.bindHolder(this);
    return component;
  }

  @Override
  public synchronized Invoker getMethod(String name){
    if (mMethods == null) {
      generate();
    }

    return mMethods.get(name);
  }
}
