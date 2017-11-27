/**
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
package com.taobao.weex.dom.transition;

import android.animation.Animator;
import android.animation.AnimatorListenerAdapter;
import android.animation.ArgbEvaluator;
import android.animation.ObjectAnimator;
import android.animation.PropertyValuesHolder;
import android.animation.ValueAnimator;
import android.graphics.drawable.ColorDrawable;
import android.os.Handler;
import android.support.v4.util.ArrayMap;
import android.support.v4.util.ArraySet;
import android.support.v4.view.animation.PathInterpolatorCompat;
import android.text.TextUtils;
import android.util.Property;
import android.view.View;
import android.view.animation.Interpolator;

import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.common.Constants;
import com.taobao.weex.dom.DOMActionContext;
import com.taobao.weex.dom.WXDomHandler;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.dom.flex.Spacing;
import com.taobao.weex.ui.animation.BackgroundColorProperty;
import com.taobao.weex.ui.animation.TransformParser;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.utils.SingleFunctionParser;
import com.taobao.weex.utils.WXLogUtils;
import com.taobao.weex.utils.WXResourceUtils;
import com.taobao.weex.utils.WXUtils;
import com.taobao.weex.utils.WXViewUtils;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.regex.Pattern;

import static com.taobao.weex.common.Constants.TimeFunction.CUBIC_BEZIER;
import static com.taobao.weex.common.Constants.TimeFunction.EASE;
import static com.taobao.weex.common.Constants.TimeFunction.EASE_IN;
import static com.taobao.weex.common.Constants.TimeFunction.EASE_IN_OUT;
import static com.taobao.weex.common.Constants.TimeFunction.EASE_OUT;
import static com.taobao.weex.common.Constants.TimeFunction.LINEAR;

/**
 *   transition on dom thread
 *   transition-property: height;
 *   transition-duration: .3s;
 *   transition-delay: .05s;
 *   transition-timing-function: ease-in-out;
 *
 *   Created by furture on 2017/10/18.
 */
public class WXTransition {

    public static final  String TRANSITION_PROPERTY = "transitionProperty";
    public static final  String TRANSITION_DURATION = "transitionDuration";
    public static final  String TRANSITION_DELAY = "transitionDelay";
    public static final  String TRANSITION_TIMING_FUNCTION = "transitionTimingFunction";
    public static final  Pattern PROPERTY_SPLIT_PATTERN = Pattern.compile("\\||,");


    /**
     * layout animation property
     * */
    private static final Set<String> LAYOUT_PROPERTIES = new ArraySet<>();
    static {
        LAYOUT_PROPERTIES.add(Constants.Name.WIDTH);
        LAYOUT_PROPERTIES.add(Constants.Name.HEIGHT);
        LAYOUT_PROPERTIES.add(Constants.Name.MARGIN_TOP);
        LAYOUT_PROPERTIES.add(Constants.Name.MARGIN_BOTTOM);
        LAYOUT_PROPERTIES.add(Constants.Name.MARGIN_LEFT);
        LAYOUT_PROPERTIES.add(Constants.Name.MARGIN_RIGHT);
    }

    /**
     * transform animation property, use android system animaton ability
     * */
    private static final Set<String> TRANSFORM_PROPERTIES = new ArraySet<>();
    static {
        TRANSFORM_PROPERTIES.add(Constants.Name.OPACITY);
        TRANSFORM_PROPERTIES.add(Constants.Name.BACKGROUND_COLOR);
        TRANSFORM_PROPERTIES.add(Constants.Name.TRANSFORM);
    }

    private List<String> properties;
    private Interpolator  interpolator;
    private float  duration;
    private float delay;
    private WXDomObject domObject;
    private Handler handler;
    private ValueAnimator layoutValueAnimator;
    private Map<String, Object> layoutPendingUpdates;
    private ObjectAnimator transformAnimator;
    private Map<String, Object> transformPendingUpdates;
    private Runnable transitionEndEvent;
    private Map<String, Object> targetStyles;
    private Runnable animationRunnable;


    public WXTransition() {
        this.properties = new ArrayList<>(4);
        this.handler = new Handler();
        this.layoutPendingUpdates = new ArrayMap<>();
        this.transformPendingUpdates = new ArrayMap<>();
        this.targetStyles = new ArrayMap<>();
    }

    /**
     * create transition from map styles if style contains transitionProperty
     * */
    public static WXTransition fromMap(Map<String, Object> style, WXDomObject domObject){
        if(style.get(TRANSITION_PROPERTY) == null){
            return  null;
        }
        String propertyString = WXUtils.getString(style.get(TRANSITION_PROPERTY), null);
        if(propertyString == null){
            return null;
        }
        WXTransition transition  = new WXTransition();
        String[] propertiesArray = PROPERTY_SPLIT_PATTERN.split(propertyString);
        for(String property : propertiesArray){
            String trim = property.trim();
            if(TextUtils.isEmpty(trim)){
                continue;
            }
            if(!(LAYOUT_PROPERTIES.contains(trim) || TRANSFORM_PROPERTIES.contains(trim))){
                if(WXEnvironment.isApkDebugable()){
                    WXLogUtils.e("WXTransition Property Not Supported" + trim + " in " + propertyString);
                }
                continue;
            }
            transition.properties.add(trim);
        }
        if(transition.properties.isEmpty()){
            return  null;
        }
        transition.duration = parseTimeMillis(style, TRANSITION_DURATION, 1);
        transition.delay =  parseTimeMillis(style, TRANSITION_DELAY, 0);
        transition.interpolator = createTimeInterpolator(WXUtils.getString(style.get(TRANSITION_TIMING_FUNCTION), null));
        transition.domObject = domObject;
        return  transition;
    }


    /**
     * check updates has transition property
     * */
    public boolean hasTransitionProperty(Map<String, Object> styles){
        for(String property : properties){
            if(styles.containsKey(property)){
                return  true;
            }
        }
        return false;
    }


    /**
     * start transition animation, updates maybe split two different updates,
     * because javascript will send multi update on same transition, we assume that updates in 8ms is one transition
     * */
    public void  startTransition(Map<String, Object> updates){
        final View taregtView = getTargetView();
        if(taregtView == null){
            return;
        }
        for(String property : properties){
            if(updates.containsKey(property)){
                Object targetValue = updates.remove(property);
                if(LAYOUT_PROPERTIES.contains(property)) {
                    layoutPendingUpdates.put(property, targetValue);
                }else if(TRANSFORM_PROPERTIES.contains(property)){
                    transformPendingUpdates.put(property, targetValue);
                }
            }
        }
        int delay = WXUtils.getNumberInt(domObject.getAttrs().get("actionDelay"), 16);
        if(animationRunnable != null) {
            handler.removeCallbacks(animationRunnable);
        }
        if(animationRunnable == null){
            animationRunnable = new Runnable() {
                @Override
                public void run() {
                    doTransitionAnimation();
                    animationRunnable = null;
                }
            };
        }
        handler.postDelayed(animationRunnable, delay);
    }

    /**
     * doTransitionAnimation include transform and layout animation.
     * 1. put pre transition updates from target style to dom style
     * 2. do transform animation and layout animation
     * */
    private void  doTransitionAnimation(){
        final View taregtView = getTargetView();
        if(taregtView == null){
            return;
        }
        if(targetStyles.size() > 0){
            for(String property : properties){
                if(!(LAYOUT_PROPERTIES.contains(property) || TRANSFORM_PROPERTIES.contains(property))){
                    continue;
                }
                if(layoutPendingUpdates.containsKey(property)){
                    continue;
                }
                if(transformPendingUpdates.containsKey(property)){
                    continue;
                }
                synchronized (targetStyles){
                    if(targetStyles.containsKey(property)){
                        //reset pre transition style
                        Object targetValue = targetStyles.remove(property);
                        domObject.getStyles().put(property, targetValue);
                        WXComponent component = getComponent();
                        if(component != null
                                && component.getDomObject() != null){
                            component.getDomObject().getStyles().put(property, targetValue);
                        }
                    }
                }
            }
        }



        if(transitionEndEvent != null){
            taregtView.removeCallbacks(transitionEndEvent);
        }
        if(transitionEndEvent == null){
            transitionEndEvent = new Runnable(){
                @Override
                public void run() {
                    transitionEndEvent = null;
                    WXComponent component = getComponent();
                    if(component != null && domObject.getEvents().contains(Constants.Event.ON_TRANSITION_END)){
                        component.fireEvent(Constants.Event.ON_TRANSITION_END);
                    }
                }
            };
        }

        taregtView.post(new Runnable() {
            @Override
            public void run() {
                doPendingTransformAnimation();
            }
        });
        doPendingLayoutAnimation();
    }


    /**
     *  transform, opacity, backgroundcolor which not effect layout use android system animation in main thread.
     * */
    private void doPendingTransformAnimation() {
        if(transformAnimator != null){
            transformAnimator.cancel();
            transformAnimator = null;
        }
        if(transformPendingUpdates.size() == 0){
            return;
        }
        final View taregtView = getTargetView();
        if(taregtView == null){
            return;
        }
        List<PropertyValuesHolder> holders = new ArrayList<>(8);
        String transform  = WXUtils.getString(transformPendingUpdates.remove(Constants.Name.TRANSFORM), null);
        if(!TextUtils.isEmpty(transform)){
            Map<Property<View,Float>, Float>  properties = TransformParser.parseTransForm(transform, (int)domObject.getLayoutWidth(), (int)domObject.getLayoutHeight(), domObject.getViewPortWidth());
            PropertyValuesHolder[]  transformHolders = TransformParser.toHolders(properties);
            for(PropertyValuesHolder holder : transformHolders){
                holders.add(holder);
            }
        }

        for(String property : properties){
            if(!TRANSFORM_PROPERTIES.contains(property)){
                continue;
            }
            if(!transformPendingUpdates.containsKey(property)){
                continue;
            }
            Object value = transformPendingUpdates.remove(property);
            synchronized (targetStyles) {
                targetStyles.put(property, value);
            }
            switch (property){
                case Constants.Name.OPACITY:{
                    holders.add(PropertyValuesHolder.ofFloat(View.ALPHA, taregtView.getAlpha(), WXUtils.getFloat(value, 1.0f)));
                }
                break;
                case Constants.Name.BACKGROUND_COLOR:{
                    int fromColor = WXResourceUtils.getColor(WXUtils.getString(domObject.getStyles().getBackgroundColor(), null), 0);
                    int toColor = WXResourceUtils.getColor(WXUtils.getString(value, null), 0);
                    if(WXViewUtils.getBorderDrawable(taregtView) != null){
                        fromColor = WXViewUtils.getBorderDrawable(taregtView).getColor();
                    }else if (taregtView.getBackground() instanceof ColorDrawable) {
                        fromColor = ((ColorDrawable) taregtView.getBackground()).getColor();
                    }
                    holders.add(PropertyValuesHolder.ofObject(new BackgroundColorProperty(), new ArgbEvaluator(), fromColor,toColor));
                }
                break;
                default:break;
            }
        }
        transformPendingUpdates.clear();
        transformAnimator =  ObjectAnimator.ofPropertyValuesHolder(taregtView, holders.toArray(new PropertyValuesHolder[holders.size()]));
        transformAnimator.setDuration((long) duration);
        transformAnimator.setStartDelay((long) delay);
        if(interpolator != null) {
            transformAnimator.setInterpolator(interpolator);
        }
        transformAnimator.addListener(new AnimatorListenerAdapter() {
            boolean  hasCancel = false;
            @Override
            public void onAnimationCancel(Animator animation) {
                super.onAnimationCancel(animation);
                hasCancel = true;
            }

            @Override
            public void onAnimationEnd(Animator animation) {
                if(hasCancel){
                    return;
                }
                super.onAnimationEnd(animation);
                WXTransition.this.onTransitionAnimationEnd();
                if(WXEnvironment.isApkDebugable()){
                    WXLogUtils.d("WXTransition transform onTransitionAnimationEnd " +  domObject.getRef());
                }
            }
        });
        transformAnimator.start();
    }


    public void doPendingLayoutAnimation(){
        if(layoutValueAnimator != null){
            layoutValueAnimator.cancel();
            layoutValueAnimator = null;
        }
        if(layoutPendingUpdates.size() == 0){
            return;
        }
        PropertyValuesHolder[] holders = new PropertyValuesHolder[layoutPendingUpdates.size()];
        int index = 0;
        for(String property : properties){
            if(!LAYOUT_PROPERTIES.contains(property)){
                continue;
            }
            if(layoutPendingUpdates.containsKey(property)){
                Object targetValue = layoutPendingUpdates.remove(property);
                synchronized (targetStyles) {
                    targetStyles.put(property, targetValue);
                }
                holders[index] = createLayoutPropertyValueHolder(property, targetValue);
                index++;
            }
        }
        layoutPendingUpdates.clear();
        doLayoutPropertyValuesHolderAnimation(holders);
    }


    private PropertyValuesHolder createLayoutPropertyValueHolder(String property, Object value){
        PropertyValuesHolder holder = null;
        switch (property){
            case Constants.Name.WIDTH:{
                holder = PropertyValuesHolder.ofFloat(Constants.Name.WIDTH, domObject.getLayoutWidth(),
                        WXViewUtils.getRealPxByWidth(WXUtils.getFloat(value), domObject.getViewPortWidth()));
            }
            break;
            case Constants.Name.HEIGHT:{
                holder = PropertyValuesHolder.ofFloat(Constants.Name.HEIGHT, domObject.getLayoutHeight(),
                        WXViewUtils.getRealPxByWidth(WXUtils.getFloat(value), domObject.getViewPortWidth()));
            }
            break;
            case Constants.Name.MARGIN_TOP:{
                holder = PropertyValuesHolder.ofFloat(Constants.Name.MARGIN_TOP,  domObject.getMargin().get(Spacing.TOP),
                        WXViewUtils.getRealPxByWidth(WXUtils.getFloatByViewport(value, domObject.getViewPortWidth()), domObject.getViewPortWidth()));
            }
            break;
            case Constants.Name.MARGIN_LEFT:{
                holder = PropertyValuesHolder.ofFloat(Constants.Name.MARGIN_LEFT,  domObject.getMargin().get(Spacing.LEFT),
                        WXViewUtils.getRealPxByWidth(WXUtils.getFloatByViewport(value, domObject.getViewPortWidth()), domObject.getViewPortWidth()));
            }
            break;
            case Constants.Name.MARGIN_RIGHT:{
                holder = PropertyValuesHolder.ofFloat(Constants.Name.MARGIN_RIGHT,  domObject.getMargin().get(Spacing.RIGHT),
                        WXViewUtils.getRealPxByWidth(WXUtils.getFloatByViewport(value, domObject.getViewPortWidth()), domObject.getViewPortWidth()));
            }
            break;
            case Constants.Name.MARGIN_BOTTOM:{
                holder = PropertyValuesHolder.ofFloat(Constants.Name.MARGIN_BOTTOM,  domObject.getMargin().get(Spacing.BOTTOM),
                        WXViewUtils.getRealPxByWidth(WXUtils.getFloatByViewport(value, domObject.getViewPortWidth()), domObject.getViewPortWidth()));
            }
            break;
            default:
                break;
        }
        if(holder == null){
            holder  = PropertyValuesHolder.ofFloat(property, 1, 1);
        }
        return  holder;
    }

    private void doLayoutPropertyValuesHolderAnimation(PropertyValuesHolder[] holders){
        layoutValueAnimator = ValueAnimator.ofPropertyValuesHolder(holders);
        layoutValueAnimator.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
            @Override
            public void onAnimationUpdate(ValueAnimator animation) {
                PropertyValuesHolder holders[] = animation.getValues();
                for(PropertyValuesHolder holder : holders){
                    String property =  holder.getPropertyName();
                    switch (property){
                        case Constants.Name.WIDTH:{
                            domObject.setStyleWidth((Float) animation.getAnimatedValue(property));
                        }
                        break;
                        case Constants.Name.HEIGHT:{
                            domObject.setStyleHeight((Float) animation.getAnimatedValue(property));
                        }
                        break;
                        case Constants.Name.MARGIN_TOP:{
                            domObject.setMargin(Spacing.TOP, (Float) animation.getAnimatedValue(property));
                        }
                        break;
                        case Constants.Name.MARGIN_LEFT:{
                            domObject.setMargin(Spacing.LEFT, (Float) animation.getAnimatedValue(property));
                        }
                        break;
                        case Constants.Name.MARGIN_RIGHT:{
                            domObject.setMargin(Spacing.RIGHT, (Float) animation.getAnimatedValue(property));
                        }
                        break;
                        case Constants.Name.MARGIN_BOTTOM:{
                            domObject.setMargin(Spacing.BOTTOM, (Float) animation.getAnimatedValue(property));
                        }
                        break;
                        default:
                            break;
                    }
                }

                DOMActionContext domActionContext = WXSDKManager.getInstance().getWXDomManager().getDomContext(domObject.getDomContext().getInstanceId());
                if(domActionContext == null){
                    return;
                }
                domActionContext.markDirty();
                WXSDKManager.getInstance().getWXDomManager().sendEmptyMessageDelayed(WXDomHandler.MsgType.WX_DOM_TRANSITION_BATCH, 0);
                if(WXEnvironment.isApkDebugable()){
                    WXLogUtils.d("WXTransition send layout batch msg");
                }
            }
        });
        layoutValueAnimator.addListener(new AnimatorListenerAdapter() {

            boolean  hasCancel = false;
            @Override
            public void onAnimationCancel(Animator animation) {
                super.onAnimationCancel(animation);
                hasCancel = true;
            }

            @Override
            public void onAnimationEnd(Animator animation) {
                if(hasCancel){
                    return;
                }
                super.onAnimationEnd(animation);
                if(WXEnvironment.isApkDebugable()){
                    WXLogUtils.d("WXTransition layout onTransitionAnimationEnd " +  domObject.getRef());
                }
                WXTransition.this.onTransitionAnimationEnd();
            }
        });
        if(interpolator != null) {
            layoutValueAnimator.setInterpolator(interpolator);
        }
        layoutValueAnimator.setStartDelay((long) (delay));
        layoutValueAnimator.setDuration((long) (duration));
        layoutValueAnimator.start();
    }

    private synchronized void onTransitionAnimationEnd(){
        if(transitionEndEvent != null){
            View view = getTargetView();
            if(view != null &&  transitionEndEvent != null){
                view.post(transitionEndEvent);
            }
            transitionEndEvent = null;
        }
        synchronized (targetStyles){
            if(targetStyles.size() > 0){
                WXComponent component = getComponent();
                for(String property : properties) {
                    if(targetStyles.containsKey(property)){
                        Object targetValue = targetStyles.remove(property);
                        domObject.getStyles().put(property, targetValue);
                        if(component != null && component.getDomObject() != null){
                            component.getDomObject().getStyles().put(property, targetValue);
                        }
                    }
                }
                targetStyles.clear();
            }
        }
    }

    private WXComponent getComponent(){
        DOMActionContext domActionContext = WXSDKManager.getInstance().getWXDomManager().getDomContext(domObject.getDomContext().getInstanceId());
        if(domActionContext != null){
            WXComponent component = domActionContext.getCompByRef(domObject.getRef());
            return  component;
        }
        return null;
    }


    private View getTargetView(){
        if(domObject.getDomContext() == null){
            return null;
        }
        DOMActionContext domActionContext = WXSDKManager.getInstance().getWXDomManager().getDomContext(domObject.getDomContext().getInstanceId());
        if(domActionContext != null){
            WXComponent component = domActionContext.getCompByRef(domObject.getRef());
            if(component != null && component.getHostView() != null) {
                return  component.getHostView();
            }
        }
        return null;
    }




    /**
     * get time millis
     * */
    private static float parseTimeMillis(Map<String, Object> style, String key, float defaultValue){
        String  duration = WXUtils.getString(style.get(key), null);
        if(duration != null){
            duration = duration.replaceAll("s", "");
        }
        if(TextUtils.isEmpty(duration)){
            return  defaultValue;
        }
        try{
           return Float.parseFloat(duration);
        }catch (NumberFormatException e){
            return  defaultValue;
        }
    }

    /**
     * create interpolcator same with web
     * http://www.w3school.com.cn/cssref/pr_transition-timing-function.asp
     * */
    private static Interpolator createTimeInterpolator(String interpolator) {
        if (!TextUtils.isEmpty(interpolator)) {
            switch (interpolator) {
                case EASE_IN:
                    return PathInterpolatorCompat.create(0.42f,0f, 1f,1f);
                case EASE_OUT:
                    return PathInterpolatorCompat.create(0f,0f, 0.58f,1f);
                case EASE_IN_OUT:
                    return PathInterpolatorCompat.create(0.42f,0f, 0.58f,1f);
                case EASE:
                    return PathInterpolatorCompat.create(0.25f,0.1f, 0.25f,1f);
                case LINEAR:
                    return PathInterpolatorCompat.create(0.0f,0f, 1f,1f);
                default:
                    try {
                        //Parse cubic-bezier
                        SingleFunctionParser<Float> parser = new SingleFunctionParser<>(
                                interpolator,
                                new SingleFunctionParser.FlatMapper<Float>() {
                                    @Override
                                    public Float map(String raw) {
                                        return Float.parseFloat(raw);
                                    }
                                });
                        List<Float> params = parser.parse(CUBIC_BEZIER);
                        if (params != null && params.size() == 4) {
                            return PathInterpolatorCompat.create(
                                    params.get(0), params.get(1), params.get(2), params.get(3));
                        }
                    } catch (RuntimeException e) {
                        if(WXEnvironment.isApkDebugable()) {
                            WXLogUtils.e("WXTransition", e);
                        }
                    }
            }
        }
        return PathInterpolatorCompat.create(0.25f,0.1f, 0.25f,1f);
    }

}
