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

package com.taobao.weex.dom.action;

import android.animation.Animator;
import android.animation.AnimatorListenerAdapter;
import android.animation.ArgbEvaluator;
import android.animation.ObjectAnimator;
import android.animation.PropertyValuesHolder;
import android.graphics.drawable.ColorDrawable;
import android.os.Build;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v4.view.animation.PathInterpolatorCompat;
import android.text.TextUtils;
import android.util.Pair;
import android.view.View;
import android.view.ViewGroup;
import android.view.animation.AccelerateDecelerateInterpolator;
import android.view.animation.AccelerateInterpolator;
import android.view.animation.DecelerateInterpolator;
import android.view.animation.Interpolator;
import android.view.animation.LinearInterpolator;
import com.alibaba.fastjson.JSONObject;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.dom.DOMAction;
import com.taobao.weex.dom.DOMActionContext;
import com.taobao.weex.dom.RenderAction;
import com.taobao.weex.dom.RenderActionContext;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.ui.animation.BackgroundColorProperty;
import com.taobao.weex.ui.animation.HeightProperty;
import com.taobao.weex.ui.animation.WXAnimationBean;
import com.taobao.weex.ui.animation.WXAnimationModule;
import com.taobao.weex.ui.animation.WidthProperty;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.view.border.BorderDrawable;
import com.taobao.weex.utils.SingleFunctionParser;
import com.taobao.weex.utils.WXLogUtils;
import com.taobao.weex.utils.WXResourceUtils;
import com.taobao.weex.utils.WXUtils;
import com.taobao.weex.utils.WXViewUtils;
import java.util.HashMap;
import java.util.List;


class AnimationAction implements DOMAction, RenderAction {

  private final static String TAG = "AnimationAction";
  @NonNull
  private final String ref;

  @Nullable
  private
  final String animation;

  @Nullable
  private
  final String callback;

  @Nullable
  private
  WXAnimationBean mAnimationBean;

  AnimationAction(@NonNull final String ref, @Nullable String animation,
                  @Nullable final String callBack) {
    this.ref = ref;
    this.animation = animation;
    this.callback = callBack;
  }

  AnimationAction(@NonNull String ref, @NonNull WXAnimationBean animationBean) {
    this(ref, animationBean, null);
  }

  AnimationAction(@NonNull String ref, @NonNull WXAnimationBean animationBean,
                  @Nullable final String callBack) {
    this.ref = ref;
    this.mAnimationBean = animationBean;
    this.callback = callBack;
    this.animation = null;
  }

  @Override
  public void executeDom(DOMActionContext context) {
    try {
      WXDomObject domObject;
      if (!context.isDestory() &&
          !TextUtils.isEmpty(animation) &&
          (domObject = context.getDomByRef(ref)) != null) {
        WXAnimationBean animationBean = JSONObject.parseObject(animation, WXAnimationBean.class);
        if (animationBean != null && animationBean.styles != null) {
          int width = (int) domObject.getLayoutWidth();
          int height = (int) domObject.getLayoutHeight();
          animationBean.styles.init(animationBean.styles.transformOrigin,
                                    animationBean.styles.transform, width, height,
                                    context.getInstance().getInstanceViewPortWidth());
          mAnimationBean = animationBean;
          context.postRenderTask(this);
        }
      }
    } catch (RuntimeException e) {
      WXLogUtils.e(TAG, WXLogUtils.getStackTrace(e));
    }
  }

  @Override
  public void executeRender(RenderActionContext context) {
    WXSDKInstance instance;
    if (mAnimationBean != null && (instance = context.getInstance()) != null) {
      startAnimation(instance, context.getComponent(ref));
    }
  }

  private void startAnimation(@NonNull WXSDKInstance instance, @Nullable WXComponent component) {
    if (component != null) {
      if (mAnimationBean != null) {
        component.setNeedLayoutOnAnimation(mAnimationBean.needLayout);
      }
      if (component.getHostView() == null) {
        WXAnimationModule.AnimationHolder holder = new WXAnimationModule.AnimationHolder(mAnimationBean, callback);
        component.postAnimation(holder);
      } else {
        try {
          Animator animator = createAnimator(component.getHostView(), instance
              .getInstanceViewPortWidth());
          if (animator != null) {
            Animator.AnimatorListener animatorCallback = createAnimatorListener(instance, callback);
            if (Build.VERSION.SDK_INT < Build.VERSION_CODES.JELLY_BEAN_MR2 && component
                .isLayerTypeEnabled() ) {
              component.getHostView().setLayerType(View.LAYER_TYPE_HARDWARE, null);
            }
            Interpolator interpolator = createTimeInterpolator();
            if (animatorCallback != null) {
              animator.addListener(animatorCallback);
            }
            if (interpolator != null) {
              animator.setInterpolator(interpolator);
            }
            animator.setDuration(mAnimationBean.duration);
            animator.start();
          }
        } catch (RuntimeException e) {
          WXLogUtils.e(TAG, WXLogUtils.getStackTrace(e));
        }
      }
    }
  }

  private
  @Nullable
  ObjectAnimator createAnimator(final View target, final int viewPortWidth) {
    if (target == null) {
      return null;
    }
    WXAnimationBean.Style style = mAnimationBean.styles;
    if (style != null) {
      ObjectAnimator animator;
      List<PropertyValuesHolder> holders = style.getHolders();
      if (!TextUtils.isEmpty(style.backgroundColor)) {
        BorderDrawable borderDrawable;
        if ((borderDrawable = WXViewUtils.getBorderDrawable(target)) != null) {
          holders.add(PropertyValuesHolder.ofObject(
              new BackgroundColorProperty(), new ArgbEvaluator(),
              borderDrawable.getColor(),
              WXResourceUtils.getColor(style.backgroundColor)));
        } else if (target.getBackground() instanceof ColorDrawable) {
          holders.add(PropertyValuesHolder.ofObject(
              new BackgroundColorProperty(), new ArgbEvaluator(),
              ((ColorDrawable) target.getBackground()).getColor(),
              WXResourceUtils.getColor(style.backgroundColor)));
        }
      }

      if (target.getLayoutParams() != null &&
          (!TextUtils.isEmpty(style.width) || !TextUtils.isEmpty(style.height))) {
        ViewGroup.LayoutParams layoutParams = target.getLayoutParams();
        if (!TextUtils.isEmpty(style.width)) {
          holders.add(PropertyValuesHolder.ofInt(new WidthProperty(), layoutParams.width,
              (int) WXViewUtils.getRealPxByWidth(WXUtils.getFloat(style.width), viewPortWidth)));
        }
        if (!TextUtils.isEmpty(style.height)) {
          holders.add(PropertyValuesHolder.ofInt(new HeightProperty(), layoutParams.height,
              (int) WXViewUtils.getRealPxByWidth(WXUtils.getFloat(style.height), viewPortWidth)));
        }
      }

      if (style.getPivot() != null) {
        Pair<Float, Float> pair = style.getPivot();
        target.setPivotX(pair.first);
        target.setPivotY(pair.second);
      }
      animator = ObjectAnimator.ofPropertyValuesHolder(
          target, holders.toArray(new PropertyValuesHolder[holders.size()]));
      animator.setStartDelay(mAnimationBean.delay);
      return animator;
    } else {
      return null;
    }
  }

  private
  @Nullable
  Animator.AnimatorListener createAnimatorListener(final WXSDKInstance instance, @Nullable final String callBack) {
    if (!TextUtils.isEmpty(callBack)) {
      return new AnimatorListenerAdapter() {
        @Override
        public void onAnimationEnd(Animator animation) {
          if (instance == null || instance.isDestroy()) {
            WXLogUtils.e("RenderActionContextImpl-onAnimationEnd WXSDKInstance == null NPE or instance is destroyed");
          } else {
            WXSDKManager.getInstance().callback(instance.getInstanceId(),
                                                callBack,
                                                new HashMap<String, Object>());
          }
        }
      };
    } else {
      return null;
    }
  }

  private
  @Nullable
  Interpolator createTimeInterpolator() {
    String interpolator = mAnimationBean.timingFunction;
    if (!TextUtils.isEmpty(interpolator)) {
      switch (interpolator) {
        case WXAnimationBean.EASE_IN:
          return new AccelerateInterpolator();
        case WXAnimationBean.EASE_OUT:
          return new DecelerateInterpolator();
        case WXAnimationBean.EASE_IN_OUT:
          return new AccelerateDecelerateInterpolator();
        case WXAnimationBean.LINEAR:
          return new LinearInterpolator();
        default:
          //Parse cubic-bezier
          try {
            SingleFunctionParser<Float> parser = new SingleFunctionParser<>(
                mAnimationBean.timingFunction,
                new SingleFunctionParser.FlatMapper<Float>() {
                  @Override
                  public Float map(String raw) {
                    return Float.parseFloat(raw);
                  }
                });
            List<Float> params = parser.parse(WXAnimationBean.CUBIC_BEZIER);
            if (params != null && params.size() == WXAnimationBean.NUM_CUBIC_PARAM) {
              return PathInterpolatorCompat.create(
                  params.get(0), params.get(1), params.get(2), params.get(3));
            } else {
              return null;
            }
          } catch (RuntimeException e) {
            return null;
          }
      }
    }
    return null;
  }
}
