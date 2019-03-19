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
import android.support.v4.view.animation.PathInterpolatorCompat;
import android.text.TextUtils;
import android.util.Property;
import android.view.View;
import android.view.animation.Interpolator;

import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.bridge.WXBridgeManager;
import com.taobao.weex.common.Constants;
//import com.taobao.weex.dom.DOMActionContext;
//import com.taobao.weex.dom.WXDomHandler;
//import com.taobao.weex.dom.WXDomObject;
//import com.taobao.weex.dom.flex.Spacing;
import com.taobao.weex.dom.CSSShorthand;
import com.taobao.weex.ui.animation.BackgroundColorProperty;
import com.taobao.weex.ui.animation.TransformParser;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.utils.SingleFunctionParser;
import com.taobao.weex.utils.WXLogUtils;
import com.taobao.weex.utils.WXResourceUtils;
import com.taobao.weex.utils.WXUtils;
import com.taobao.weex.utils.WXViewUtils;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.atomic.AtomicInteger;
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
    private static final Set<String> LAYOUT_PROPERTIES = new HashSet<>();
    static {
        LAYOUT_PROPERTIES.add(Constants.Name.WIDTH);
        LAYOUT_PROPERTIES.add(Constants.Name.HEIGHT);
        LAYOUT_PROPERTIES.add(Constants.Name.MARGIN_TOP);
        LAYOUT_PROPERTIES.add(Constants.Name.MARGIN_BOTTOM);
        LAYOUT_PROPERTIES.add(Constants.Name.MARGIN_LEFT);
        LAYOUT_PROPERTIES.add(Constants.Name.MARGIN_RIGHT);
        LAYOUT_PROPERTIES.add(Constants.Name.LEFT);
        LAYOUT_PROPERTIES.add(Constants.Name.RIGHT);
        LAYOUT_PROPERTIES.add(Constants.Name.TOP);
        LAYOUT_PROPERTIES.add(Constants.Name.BOTTOM);
        LAYOUT_PROPERTIES.add(Constants.Name.PADDING_LEFT);
        LAYOUT_PROPERTIES.add(Constants.Name.PADDING_RIGHT);
        LAYOUT_PROPERTIES.add(Constants.Name.PADDING_TOP);
        LAYOUT_PROPERTIES.add(Constants.Name.PADDING_BOTTOM);
    }

    /**
     * transform animation property, use android system animaton ability
     * */
    private static final Set<String> TRANSFORM_PROPERTIES = new HashSet<>();
    static {
        TRANSFORM_PROPERTIES.add(Constants.Name.OPACITY);
        TRANSFORM_PROPERTIES.add(Constants.Name.BACKGROUND_COLOR);
        TRANSFORM_PROPERTIES.add(Constants.Name.TRANSFORM);
    }

    private List<String> properties;
    private Interpolator  interpolator;
    private long  duration;
    private long delay;
    private WXComponent mWXComponent;
    private Handler handler;
    private ValueAnimator layoutValueAnimator;
    private Map<String, Object> layoutPendingUpdates;
    private ObjectAnimator transformAnimator;
    private Map<String, Object> transformPendingUpdates;
    private Runnable transitionEndEvent;
    private Map<String, Object> targetStyles;
    private Runnable animationRunnable;

    private Runnable transformAnimationRunnable;
    private volatile AtomicInteger lockToken = new AtomicInteger(0);


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
    public static WXTransition fromMap(Map<String, Object> style, WXComponent component){
        if(style.get(TRANSITION_PROPERTY) == null){
            return  null;
        }
        String propertyString = WXUtils.getString(style.get(TRANSITION_PROPERTY), null);
        if(propertyString == null){
            return null;
        }
        WXTransition transition  = new WXTransition();
        updateTransitionProperties(transition, propertyString);
        if(transition.properties.isEmpty()){
            return  null;
        }
        transition.duration = parseTimeMillis(style, TRANSITION_DURATION, 0);
        transition.delay =  parseTimeMillis(style, TRANSITION_DELAY, 0);
        transition.interpolator = createTimeInterpolator(WXUtils.getString(style.get(TRANSITION_TIMING_FUNCTION), null));
        transition.mWXComponent = component;
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

    public void updateTranstionParams(Map<String, Object> updates){
        if(updates.containsKey(TRANSITION_DELAY)){
            mWXComponent.getStyles().put(TRANSITION_DELAY, updates.remove(TRANSITION_DELAY));
            this.delay = parseTimeMillis(mWXComponent.getStyles(), TRANSITION_DELAY, 0);
        }

        if(updates.containsKey(TRANSITION_TIMING_FUNCTION) && updates.get(TRANSITION_TIMING_FUNCTION) != null){
            mWXComponent.getStyles().put(TRANSITION_TIMING_FUNCTION, updates.remove(TRANSITION_TIMING_FUNCTION));
            this.interpolator = createTimeInterpolator(mWXComponent.getStyles().get(TRANSITION_TIMING_FUNCTION).toString());
        }

        if(updates.containsKey(TRANSITION_DURATION)){
            mWXComponent.getStyles().put(TRANSITION_DURATION, updates.remove(TRANSITION_DURATION));
            this.duration = parseTimeMillis(mWXComponent.getStyles(), TRANSITION_DURATION, 0);
        }

        if(updates.containsKey(TRANSITION_PROPERTY)){
            mWXComponent.getStyles().put(TRANSITION_PROPERTY, updates.remove(TRANSITION_PROPERTY));
            updateTransitionProperties(this, WXUtils.getString(mWXComponent.getStyles().get(TRANSITION_PROPERTY), null));
        }
    }

    /**
     * start transition animation, updates maybe split two different updates,
     * because javascript will send multi update on same transition, we assume that updates in 8ms is one transition
     * */
    public void  startTransition(Map<String, Object> updates){
        synchronized (lockToken){
            final View taregtView = getTargetView();
            if(taregtView == null){
                return;
            }
            final int token = lockToken.incrementAndGet();
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

            int delay = WXUtils.getNumberInt(mWXComponent.getAttrs().get("actionDelay"), 16);
            if(delay > duration){
                delay = (int) duration;
            }
            if(animationRunnable != null) {
                handler.removeCallbacks(animationRunnable);
            }
            animationRunnable = new Runnable() {
                @Override
                public void run() {
                    if(token == lockToken.get()) {
                        doTransitionAnimation(token);
                    }
                    animationRunnable = null;
                }
            };
            if(delay > 0){
                handler.postDelayed(animationRunnable, delay);
            }else{
                animationRunnable.run();
            }
        }
    }

    /**
     * doTransitionAnimation include transform and layout animation.
     * 1. put pre transition updates from target style to dom style
     * 2. do transform animation and layout animation
     * */
    private void  doTransitionAnimation(final  int token){
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
                        mWXComponent.getStyles().put(property, targetValue);
//                        domObject.getStyles().put(property, targetValue);
//                        WXComponent component = getComponent();
//                        if(component != null && component.getDomObject() != null){
//                            component.getDomObject().getStyles().put(property, targetValue);
//                        }
                    }
                }
            }
        }



        if(transitionEndEvent != null){
            taregtView.removeCallbacks(transitionEndEvent);
        }
        if(transitionEndEvent == null && duration > Float.MIN_NORMAL){
            transitionEndEvent = new Runnable(){
                @Override
                public void run() {
                    transitionEndEvent = null;
                    if(duration < Float.MIN_NORMAL){
                        return;
                    }
                    if(mWXComponent != null && mWXComponent.getEvents().contains(Constants.Event.ON_TRANSITION_END)){
                        mWXComponent.fireEvent(Constants.Event.ON_TRANSITION_END);
                    }
                }
            };
        }
        if(transformAnimationRunnable != null) {
            taregtView.removeCallbacks(transformAnimationRunnable);
        }
        transformAnimationRunnable = new Runnable() {
            @Override
            public void run() {
                synchronized (lockToken) {
                    if(token == lockToken.get()) {
                        doPendingTransformAnimation(token);
                    }
                }
            }
        };
        taregtView.post(transformAnimationRunnable);
        doPendingLayoutAnimation();
    }


    /**
     *  transform, opacity, backgroundcolor which not effect layout use android system animation in main thread.
     * */
    private void doPendingTransformAnimation(int token) {
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
            Map<Property<View,Float>, Float>  properties = TransformParser.parseTransForm(mWXComponent.getInstanceId(), transform, (int)mWXComponent.getLayoutWidth(), (int)mWXComponent.getLayoutHeight(), mWXComponent.getViewPortWidth());
            PropertyValuesHolder[]  transformHolders = TransformParser.toHolders(properties);
            for(PropertyValuesHolder holder : transformHolders){
                holders.add(holder);
            }
            synchronized (targetStyles) {
                targetStyles.put(Constants.Name.TRANSFORM, transform);
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
                    taregtView.setLayerType(View.LAYER_TYPE_SOFTWARE, null); //hardware or none has bug on some platform
                }
                break;
                case Constants.Name.BACKGROUND_COLOR:{
                    int fromColor = WXResourceUtils.getColor(WXUtils.getString(mWXComponent.getStyles().getBackgroundColor(), null), 0);
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

        if(token == lockToken.get()) {
            transformPendingUpdates.clear();
        }
        transformAnimator =  ObjectAnimator.ofPropertyValuesHolder(taregtView, holders.toArray(new PropertyValuesHolder[holders.size()]));
        transformAnimator.setDuration((long) duration);
        if((long) delay > 0) {
            transformAnimator.setStartDelay((long) delay);
        }
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
                holder = PropertyValuesHolder.ofFloat(Constants.Name.WIDTH, mWXComponent.getLayoutWidth(),
                        WXViewUtils.getRealPxByWidth(WXUtils.getFloat(value, 0.0f), mWXComponent.getViewPortWidth()));
            }
            break;
            case Constants.Name.HEIGHT:{
                holder = PropertyValuesHolder.ofFloat(Constants.Name.HEIGHT, mWXComponent.getLayoutHeight(),
                        WXViewUtils.getRealPxByWidth(WXUtils.getFloat(value, 0.0f), mWXComponent.getViewPortWidth()));
            }
            break;
            case Constants.Name.MARGIN_TOP:{
                holder = PropertyValuesHolder.ofFloat(Constants.Name.MARGIN_TOP,  mWXComponent.getMargin().get(CSSShorthand.EDGE.TOP),
                        WXViewUtils.getRealPxByWidth(WXUtils.getFloatByViewport(value, mWXComponent.getViewPortWidth()), mWXComponent.getViewPortWidth()));
            }
            break;
            case Constants.Name.MARGIN_LEFT:{
                holder = PropertyValuesHolder.ofFloat(Constants.Name.MARGIN_LEFT,  mWXComponent.getMargin().get(CSSShorthand.EDGE.LEFT),
                        WXViewUtils.getRealPxByWidth(WXUtils.getFloatByViewport(value, mWXComponent.getViewPortWidth()), mWXComponent.getViewPortWidth()));
            }
            break;
            case Constants.Name.MARGIN_RIGHT:{
                holder = PropertyValuesHolder.ofFloat(Constants.Name.MARGIN_RIGHT,  mWXComponent.getMargin().get(CSSShorthand.EDGE.RIGHT),
                        WXViewUtils.getRealPxByWidth(WXUtils.getFloatByViewport(value, mWXComponent.getViewPortWidth()), mWXComponent.getViewPortWidth()));
            }
            break;
            case Constants.Name.MARGIN_BOTTOM:{
                holder = PropertyValuesHolder.ofFloat(Constants.Name.MARGIN_BOTTOM,  mWXComponent.getMargin().get(CSSShorthand.EDGE.BOTTOM),
                        WXViewUtils.getRealPxByWidth(WXUtils.getFloatByViewport(value, mWXComponent.getViewPortWidth()), mWXComponent.getViewPortWidth()));
            }
            break;
            case Constants.Name.LEFT:{
                holder = PropertyValuesHolder.ofFloat(Constants.Name.LEFT,  mWXComponent.getLayoutPosition().getLeft(),
                        WXViewUtils.getRealPxByWidth(WXUtils.getFloatByViewport(value, mWXComponent.getViewPortWidth()), mWXComponent.getViewPortWidth()));
            }
            break;
            case Constants.Name.RIGHT:{
                holder = PropertyValuesHolder.ofFloat(Constants.Name.RIGHT,  mWXComponent.getLayoutPosition().getRight(),
                        WXViewUtils.getRealPxByWidth(WXUtils.getFloatByViewport(value, mWXComponent.getViewPortWidth()), mWXComponent.getViewPortWidth()));
            }
            break;
            case Constants.Name.BOTTOM:{
                holder = PropertyValuesHolder.ofFloat(Constants.Name.BOTTOM,  mWXComponent.getLayoutPosition().getBottom(),
                        WXViewUtils.getRealPxByWidth(WXUtils.getFloatByViewport(value, mWXComponent.getViewPortWidth()), mWXComponent.getViewPortWidth()));
            }
            break;
            case Constants.Name.TOP:{
                holder = PropertyValuesHolder.ofFloat(Constants.Name.TOP,  mWXComponent.getLayoutPosition().getTop(),
                        WXViewUtils.getRealPxByWidth(WXUtils.getFloatByViewport(value, mWXComponent.getViewPortWidth()), mWXComponent.getViewPortWidth()));
            }
            break;
            case Constants.Name.PADDING_TOP:{
                holder = PropertyValuesHolder.ofFloat(Constants.Name.PADDING_TOP,  mWXComponent.getPadding().get(CSSShorthand.EDGE.TOP),
                        WXViewUtils.getRealPxByWidth(WXUtils.getFloatByViewport(value, mWXComponent.getViewPortWidth()), mWXComponent.getViewPortWidth()));
            }
            break;
            case Constants.Name.PADDING_BOTTOM:{
                holder = PropertyValuesHolder.ofFloat(Constants.Name.PADDING_BOTTOM,  mWXComponent.getPadding().get(CSSShorthand.EDGE.BOTTOM),
                        WXViewUtils.getRealPxByWidth(WXUtils.getFloatByViewport(value, mWXComponent.getViewPortWidth()), mWXComponent.getViewPortWidth()));
            }
            break;
            case Constants.Name.PADDING_LEFT:{
                holder = PropertyValuesHolder.ofFloat(Constants.Name.PADDING_LEFT,  mWXComponent.getPadding().get(CSSShorthand.EDGE.LEFT),
                        WXViewUtils.getRealPxByWidth(WXUtils.getFloatByViewport(value, mWXComponent.getViewPortWidth()), mWXComponent.getViewPortWidth()));
            }
            break;
            case Constants.Name.PADDING_RIGHT:{
                holder = PropertyValuesHolder.ofFloat(Constants.Name.PADDING_RIGHT,  mWXComponent.getPadding().get(CSSShorthand.EDGE.RIGHT),
                        WXViewUtils.getRealPxByWidth(WXUtils.getFloatByViewport(value, mWXComponent.getViewPortWidth()), mWXComponent.getViewPortWidth()));
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
            public void onAnimationUpdate(final ValueAnimator animation) {
                PropertyValuesHolder holders[] = animation.getValues();
                for(PropertyValuesHolder holder : holders){

                    final String property =  holder.getPropertyName();
                    asynchronouslyUpdateLayout(mWXComponent, property, (Float) animation.getAnimatedValue(property));
                }
//                WXBridgeManager.getInstance().calculateLayoutPostToJSThread(mWXComponent.getInstanceId(), mWXComponent.getRef(), false);
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

    @SuppressWarnings("unused")
    public static void asynchronouslyUpdateLayout(WXComponent component, final String propertyName, final float propertyValue) {
        if(component == null) {
            return;
        }
        final String ref = component.getRef();
        final String instanceId = component.getInstanceId();
        if(TextUtils.isEmpty(ref) || TextUtils.isEmpty(instanceId)) {
            return;
        }

        WXSDKManager.getInstance().getWXBridgeManager().post(new Runnable() {
            @Override
            public void run() {
                switch (propertyName){
                    case Constants.Name.WIDTH:{
                        WXBridgeManager.getInstance().setStyleWidth(instanceId, ref, propertyValue);
                    }
                    break;
                    case Constants.Name.HEIGHT:{
                        WXBridgeManager.getInstance().setStyleHeight(instanceId, ref, propertyValue);
                    }
                    break;
                    case Constants.Name.MARGIN_TOP:{
                        WXBridgeManager.getInstance().setMargin(instanceId, ref, CSSShorthand.EDGE.TOP, propertyValue);
                    }
                    break;
                    case Constants.Name.MARGIN_LEFT:{
                        WXBridgeManager.getInstance().setMargin(instanceId, ref, CSSShorthand.EDGE.LEFT, propertyValue);
                    }
                    break;
                    case Constants.Name.MARGIN_RIGHT:{
                        WXBridgeManager.getInstance().setMargin(instanceId, ref, CSSShorthand.EDGE.RIGHT, propertyValue);
                    }
                    break;
                    case Constants.Name.MARGIN_BOTTOM:{
                        WXBridgeManager.getInstance().setMargin(instanceId, ref, CSSShorthand.EDGE.BOTTOM, propertyValue);
                    }
                    break;
                    case Constants.Name.LEFT:{
                        WXBridgeManager.getInstance().setPosition(instanceId, ref, CSSShorthand.EDGE.LEFT, propertyValue);
                    }
                    break;
                    case Constants.Name.RIGHT:{
                        WXBridgeManager.getInstance().setPosition(instanceId, ref, CSSShorthand.EDGE.RIGHT, (propertyValue));
                    }
                    break;
                    case Constants.Name.BOTTOM:{
                        WXBridgeManager.getInstance().setPosition(instanceId, ref, CSSShorthand.EDGE.BOTTOM, propertyValue);
                    }
                    break;
                    case Constants.Name.TOP:{
                        WXBridgeManager.getInstance().setPosition(instanceId, ref, CSSShorthand.EDGE.TOP, propertyValue);
                    }
                    break;
                    case Constants.Name.PADDING_TOP:{
                        WXBridgeManager.getInstance().setPadding(instanceId, ref, CSSShorthand.EDGE.TOP, propertyValue);
                    }
                    break;
                    case Constants.Name.PADDING_BOTTOM:{
                        WXBridgeManager.getInstance().setPadding(instanceId, ref, CSSShorthand.EDGE.BOTTOM, propertyValue);
                    }
                    break;
                    case Constants.Name.PADDING_LEFT:{
                        WXBridgeManager.getInstance().setPadding(instanceId, ref, CSSShorthand.EDGE.LEFT, propertyValue);
                    }
                    break;
                    case Constants.Name.PADDING_RIGHT:{
                        WXBridgeManager.getInstance().setPadding(instanceId, ref, CSSShorthand.EDGE.RIGHT, propertyValue);
                    }
                    break;
                    default:
                        break;
                }
            }
        });
    }

    private synchronized void onTransitionAnimationEnd(){
        if(duration > 0){
            if(transitionEndEvent != null){
                View view = getTargetView();
                if(view != null &&  transitionEndEvent != null){
                    view.post(transitionEndEvent);
                }
                transitionEndEvent = null;
            }
        }
        synchronized (targetStyles){
            if(targetStyles.size() > 0){
                for(String property : properties) {
                    if(targetStyles.containsKey(property)){
                        Object targetValue = targetStyles.remove(property);
                        mWXComponent.getStyles().put(property, targetValue);
                    }
                }
                targetStyles.clear();
            }
        }
    }

    private View getTargetView(){
        return null != mWXComponent ? mWXComponent.getHostView() : null;
    }


    /**
     * get time millis
     * */
    private static long parseTimeMillis(Map<String, Object> style, String key, long defaultValue){
        String  duration = WXUtils.getString(style.get(key), null);
        if(duration != null){
            duration = duration.replaceAll("ms", "");
        }
        if(duration != null){
            if(WXEnvironment.isApkDebugable()){
                if(duration.contains("px")){
                    WXLogUtils.w("Transition Duration Unit Only Support ms, " + duration + " is not ms Unit");
                }
            }
            duration = duration.replaceAll("px", "");
        }
        if(TextUtils.isEmpty(duration)){
            return  defaultValue;
        }
        try{
            return (long)Float.parseFloat(duration);
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

    private static  void  updateTransitionProperties(WXTransition transition, String transtionProperty){
        if(transtionProperty == null){
            return;
        }
        transition.properties.clear();
        String[] propertiesArray = PROPERTY_SPLIT_PATTERN.split(transtionProperty);
        for(String property : propertiesArray){
            String trim = property.trim();
            if(TextUtils.isEmpty(trim)){
                continue;
            }
            if(!(LAYOUT_PROPERTIES.contains(trim) || TRANSFORM_PROPERTIES.contains(trim))){
                if(WXEnvironment.isApkDebugable()){
                    WXLogUtils.e("WXTransition Property Not Supported" + trim + " in " + transtionProperty);
                }
                continue;
            }
            transition.properties.add(trim);
        }
    }

    public List<String> getProperties() {
        return properties;
    }
}
