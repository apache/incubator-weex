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
package com.taobao.weex.ui.view.gesture;

import android.content.Context;
import android.graphics.Point;
import android.graphics.PointF;
import android.graphics.Rect;
import android.os.Looper;
import android.support.annotation.NonNull;
import android.view.GestureDetector;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;
import android.view.ViewGroup;
import android.view.ViewParent;

import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import com.taobao.weex.bridge.EventResult;
import com.taobao.weex.common.Constants;
import com.taobao.weex.dom.WXEvent;
import com.taobao.weex.ui.component.Scrollable;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.view.gesture.WXGestureType.GestureInfo;
import com.taobao.weex.ui.view.gesture.WXGestureType.HighLevelGesture;
import com.taobao.weex.ui.view.gesture.WXGestureType.LowLevelGesture;
import com.taobao.weex.utils.WXLogUtils;
import com.taobao.weex.utils.WXUtils;
import com.taobao.weex.utils.WXViewUtils;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import static com.taobao.weex.common.Constants.Event.STOP_PROPAGATION;
import static com.taobao.weex.common.Constants.Event.STOP_PROPAGATION_RAX;

public class WXGesture extends GestureDetector.SimpleOnGestureListener implements OnTouchListener {

  private final static String TAG = "Gesture";
  private static final int CUR_EVENT = -1;
  public static final String START = "start";
  public static final String MOVE = "move";
  public static final String END = "end";
  public static final String UNKNOWN = "unknown";
  public static final String LEFT = "left";
  public static final String RIGHT = "right";
  public static final String UP = "up";
  public static final String DOWN = "down";
  private WXComponent component;
  private GestureDetector mGestureDetector;
  private Rect globalRect;
  private Point globalOffset;
  private Point globalEventOffset;
  private PointF locEventOffset;
  private PointF locLeftTop;
  private long swipeDownTime = -1;
  private long panDownTime = -1;
  private WXGestureType mPendingPan = null;//event type to notify when action_up or action_cancel
  private int mParentOrientation =-1;
  private boolean mIsPreventMoveEvent = false;
  private boolean mIsTouchEventConsumed = false; //Reset to false when first touch event, set to true when gesture event fired.

  private boolean requestDisallowInterceptTouchEvent = false;

  private boolean shouldBubbleResult = true;
  private int     shouldBubbleInterval = 0; //every times try
  private int     shouldBubbleCallRemainTimes = 0;

  private final List<OnTouchListener> mTouchListeners = new LinkedList<>();

  public WXGesture(WXComponent wxComponent, Context context) {
    this.component = wxComponent;
    globalRect = new Rect();
    globalOffset = new Point();
    globalEventOffset = new Point();
    locEventOffset = new PointF();
    locLeftTop = new PointF();
    mGestureDetector = new GestureDetector(context, this, new GestureHandler());
    Scrollable parentScrollable = wxComponent.getParentScroller();
    if(parentScrollable != null) {
      mParentOrientation = parentScrollable.getOrientation();
    }
    shouldBubbleResult =  WXUtils.getBoolean(wxComponent.getAttrs().get(Constants.Name.SHOULD_STOP_PROPAGATION_INIT_RESULT), true);
    shouldBubbleInterval = WXUtils.getNumberInt(wxComponent.getAttrs().get(Constants.Name.SHOULD_STOP_PROPAGATION_INTERVAL), 0);
  }

  private boolean isParentScrollable() {
    if(component == null) {
      return true;
    }
    Scrollable parentScrollable = component.getParentScroller();
    return parentScrollable == null || parentScrollable.isScrollable();
  }

  private boolean hasSameOrientationWithParent(){
    return (mParentOrientation == Constants.Orientation.HORIZONTAL && component.containsGesture(HighLevelGesture.HORIZONTALPAN))
            || (mParentOrientation == Constants.Orientation.VERTICAL && component.containsGesture(HighLevelGesture.VERTICALPAN));
  }

  public void setPreventMoveEvent(boolean preventMoveEvent) {
    mIsPreventMoveEvent = preventMoveEvent;
  }

  /**
   *
   * @return true if current touch event is already consumed by gesture.
   * Reset to false when next first touch event, set to true when gesture event fired.
   */
  public boolean isTouchEventConsumedByAdvancedGesture(){
    return mIsTouchEventConsumed;
  }


  /**
   * stoppropagation
   * */
  public static boolean isStopPropagation(String type){
    return  Constants.Event.STOP_PROPAGATION.equals(type) || Constants.Event.STOP_PROPAGATION_RAX.equals(type);
  }

  public static boolean hasStopPropagation(WXComponent component){
    WXEvent event = component.getEvents();
    if(event == null){
      return false;
    }
    int size = event.size();
    for (int i=0; i<size; i++) {
      if(i >= event.size()){
        break;
      }
      String type = event.get(i);
      if(isStopPropagation(type)){
        return true;
      }
    }
    return false;
  }

  /**
   * shouldBubbleEvent default true
   * */
  private boolean shouldBubbleTouchEvent(MotionEvent event){
    if(hasStopPropagation(component)){
      if(shouldBubbleInterval > 0 && shouldBubbleCallRemainTimes > 0){
        shouldBubbleCallRemainTimes--;
        return  shouldBubbleResult;
      }
      Map<String, Object> eventMap = createFireEventParam(event, CUR_EVENT, null);
      eventMap.put("type", "touch");
      if(event.getAction() == MotionEvent.ACTION_DOWN){
        eventMap.put("action", START);
      }else if(event.getAction() == MotionEvent.ACTION_CANCEL
              ||  event.getAction() == MotionEvent.ACTION_UP){
        eventMap.put("action", END);
      }else{
        eventMap.put("action", MOVE);
      }

      String name = STOP_PROPAGATION;
      if(!component.getEvents().contains(STOP_PROPAGATION)){
         name = STOP_PROPAGATION_RAX;
      }
      EventResult result = component.fireEventWait(name, eventMap);
      if(result.isSuccess() && result.getResult() != null){
        boolean stopPropagation = WXUtils.getBoolean(result.getResult(), !shouldBubbleResult);
        shouldBubbleResult = !stopPropagation;
      }
      shouldBubbleCallRemainTimes = shouldBubbleInterval;
      return shouldBubbleResult;

    }
    return  true;
  }

  @SuppressWarnings("unused")
  public void addOnTouchListener(OnTouchListener listener) {
    if(listener != null) {
      mTouchListeners.add(listener);
    }
  }

  @SuppressWarnings("unused")
  public boolean removeTouchListener(OnTouchListener listener) {
    if(listener != null) {
      return mTouchListeners.remove(listener);
    }
    return false;
  }

  @Override
  public boolean onTouch(View v, MotionEvent event) {
    if(requestDisallowInterceptTouchEvent){
      requestDisallowInterceptTouchEvent = false;
      return false;
    }
    try {
      boolean result = mGestureDetector.onTouchEvent(event);

      if(mTouchListeners != null && !mTouchListeners.isEmpty()) {
        for(OnTouchListener listener : mTouchListeners) {
          result |= listener.onTouch(v, event);
        }
      }

      switch (event.getActionMasked()) {
        case MotionEvent.ACTION_POINTER_DOWN:
        case MotionEvent.ACTION_DOWN:
          mIsTouchEventConsumed = false;
          /**
           * If component has same scroll orientation with it's parent and it's parent not scrollable
           * , we should disallow parent in DOWN.
           */
          if(hasSameOrientationWithParent() && !isParentScrollable()){
            ViewParent p;
            if ((p = component.getRealView().getParent()) != null) {
              p.requestDisallowInterceptTouchEvent(true);
            }
          }
          result |= handleMotionEvent(LowLevelGesture.ACTION_DOWN, event);
          break;
        case MotionEvent.ACTION_MOVE:
          result |= handleMotionEvent(LowLevelGesture.ACTION_MOVE, event);
          break;
        case MotionEvent.ACTION_UP:
        case MotionEvent.ACTION_POINTER_UP:
          finishDisallowInterceptTouchEvent(v);
          result |= handleMotionEvent(LowLevelGesture.ACTION_UP, event);
          result |= handlePanMotionEvent(event);
          break;
        case MotionEvent.ACTION_CANCEL:
          finishDisallowInterceptTouchEvent(v);
          result |= handleMotionEvent(LowLevelGesture.ACTION_CANCEL, event);
          result |= handlePanMotionEvent(event);
          break;
      }
      if(hasStopPropagation(component)){
        ViewGroup parent = (ViewGroup) v.getParent();
        boolean requestDisallowInterceptTouchEvent = false;
        if(parent != null){
          if(!shouldBubbleTouchEvent(event)){
            requestDisallowInterceptTouchEvent = true;
          }
          parent.requestDisallowInterceptTouchEvent(requestDisallowInterceptTouchEvent);
        }
        if(component.getParent() != null){
          component.getParent().requestDisallowInterceptTouchEvent(requestDisallowInterceptTouchEvent);
        }
        if(mIsTouchEventConsumed && WXUtils.getBoolean(component.getAttrs().get("cancelTouchOnConsume"), false)){//when touch event consumed by one gesture, other component should not consumed
          event.setAction(MotionEvent.ACTION_CANCEL);
        }
      }
      return result;
    } catch (Exception e) {
      WXLogUtils.e("Gesture RunTime Error ", e);
      return false;
    }
  }

  private String getPanEventAction(MotionEvent event) {
    switch (event.getAction()) {
      case MotionEvent.ACTION_DOWN:
        return START;
      case MotionEvent.ACTION_MOVE:
        return MOVE;
      case MotionEvent.ACTION_UP:
        return END;
      case MotionEvent.ACTION_CANCEL:
        return END;
      default:
        return UNKNOWN;
    }
  }

  private void finishDisallowInterceptTouchEvent(View v){
    if(v.getParent() != null){
      v.getParent().requestDisallowInterceptTouchEvent(false);
    }
  }

  private boolean handlePanMotionEvent(MotionEvent motionEvent) {
    if (mPendingPan == null) {
      return false;
    }
    String state = null;
    if (mPendingPan == HighLevelGesture.HORIZONTALPAN || mPendingPan == HighLevelGesture.VERTICALPAN) {
      state = getPanEventAction(motionEvent);

    }

    if (component.containsGesture(mPendingPan)) {
      if(mIsPreventMoveEvent && MOVE.equals(state)){
        return true;
      }
      List<Map<String, Object>> list = createMultipleFireEventParam(motionEvent, state);
      for (Map<String, Object> map : list) {
        component.fireEvent(mPendingPan.toString(), map);
      }
      //action is finish, clean pending pan
      if (motionEvent.getAction() == MotionEvent.ACTION_UP || motionEvent.getAction() == MotionEvent.ACTION_CANCEL) {
        mPendingPan = null;
      }
      return true;
    } else {
      return false;
    }
  }

  /**
   *
   * @param WXGestureType possible low-level gesture, defined in {@link com.taobao.weex.common.Constants.Event}
   * @param motionEvent motionEvent, which contains all pointers event in a period of time
   * @return true if this event is handled, otherwise false.
   */
  private boolean handleMotionEvent(WXGestureType WXGestureType, MotionEvent motionEvent) {
    if (component.containsGesture(WXGestureType)) {
      List<Map<String, Object>> list = createMultipleFireEventParam(motionEvent, null);
      for (Map<String, Object> map : list) {
        component.fireEvent(WXGestureType.toString(), map);
      }
      return true;
    } else {
      return false;
    }
  }

  /**
   * Create a list of event for {@link com.taobao.weex.WXSDKInstance#fireEvent(String, String, Map, Map)}.
   * As there is a batch mechanism in MotionEvent, so this method returns a list.
   * @param motionEvent motionEvent, which contains all pointers event in a period of time
   * @return List of Map, which contains touch object.
   */
  private List<Map<String, Object>> createMultipleFireEventParam(MotionEvent motionEvent,String state) {
    List<Map<String, Object>> list = new ArrayList<>(motionEvent.getHistorySize() + 1);
    //list.addAll(getHistoricalEvents(motionEvent));
    list.add(createFireEventParam(motionEvent, CUR_EVENT, state));
    return list;
  }

  /**
   * Get historical event. This is only applied to {@link MotionEvent#ACTION_MOVE}.
   * For other types of motionEvent, historical event is meaningless.
   * @param motionEvent motionEvent, which contains all pointers event in a period of time
   * @return If motionEvent.getActionMasked()!=MotionEvent.ACTION_MOVE,
   * this method will return an empty list.
   * Otherwise this method will return the historical motionEvent, which may also be empty.
   */
  private List<Map<String, Object>> getHistoricalEvents(MotionEvent motionEvent) {
    List<Map<String, Object>> list = new ArrayList<>(motionEvent.getHistorySize());
    if (motionEvent.getActionMasked() == MotionEvent.ACTION_MOVE) {
      Map<String, Object> param;
      for (int i = 0; i < motionEvent.getHistorySize(); i++) {
        param = createFireEventParam(motionEvent, i,null);
        list.add(param);
      }
    }
    return list;
  }

  /**
   * Create a map represented touch event at a certain moment.
   * @param motionEvent motionEvent, which contains all pointers event in a period of time
   * @param pos index used to retrieve a certain moment in a period of time.
   * @return touchEvent
   * @see <a href="https://developer.mozilla.org/en-US/docs/Web/API/TouchEvent">touchEvent</a>
   */
  private Map<String, Object> createFireEventParam(MotionEvent motionEvent, int pos, String state) {
    JSONArray jsonArray = new JSONArray(motionEvent.getPointerCount());
    if (motionEvent.getActionMasked() == MotionEvent.ACTION_MOVE) {
      for (int i = 0; i < motionEvent.getPointerCount(); i++) {
        jsonArray.add(createJSONObject(motionEvent, pos, i));
      }
    } else if (isPointerNumChanged(motionEvent)) {
      int pointerIndex = motionEvent.getActionIndex();
      jsonArray.add(createJSONObject(motionEvent, CUR_EVENT, pointerIndex));
    }
    Map<String, Object> map = new HashMap<>();
    map.put(GestureInfo.HISTORICAL_XY, jsonArray);
    if (state != null) {
      map.put(GestureInfo.STATE, state);
    }
    return map;
  }

  /**
   * Tell whether the number of motion event's pointer changed.
   * @param event the current motion event
   * @return true for number of motion event's pointer changed, otherwise false.
   */
  private boolean isPointerNumChanged(MotionEvent event) {
    return event.getActionMasked() == MotionEvent.ACTION_DOWN ||
            event.getActionMasked() == MotionEvent.ACTION_POINTER_DOWN ||
            event.getActionMasked() == MotionEvent.ACTION_UP ||
            event.getActionMasked() == MotionEvent.ACTION_POINTER_UP ||
            event.getActionMasked() == MotionEvent.ACTION_CANCEL;
  }

  /**
   * Tell whether component contains pan gesture
   * @return true for contains pan gesture, otherwise false.
   */
  private boolean containsSimplePan() {
    return component.containsGesture(HighLevelGesture.PAN_START) ||
            component.containsGesture(HighLevelGesture.PAN_MOVE) ||
            component.containsGesture(HighLevelGesture.PAN_END);
  }

  /**
   * Create a touchObject for a pointer at a certain moment.
   * @param motionEvent motionEvent, which contains all pointers event in a period of time
   * @param pos index used to retrieve a certain moment in a period of time.
   * @param pointerIndex pointerIndex
   * @return JSONObject represent a touch event
   * @see <a href="https://developer.mozilla.org/en-US/docs/Web/API/Touch">touch</a>
   */
  private JSONObject createJSONObject(MotionEvent motionEvent, int pos, int pointerIndex) {
    PointF screenXY, pageXY;
    if (pos == CUR_EVENT) {
      pageXY = getEventLocInPageCoordinate(motionEvent, pointerIndex);
      screenXY = getEventLocInScreenCoordinate(motionEvent, pointerIndex);
    } else {
      pageXY = getEventLocInPageCoordinate(motionEvent, pointerIndex, pos);
      screenXY = getEventLocInScreenCoordinate(motionEvent, pointerIndex, pos);
    }
    JSONObject map = createJSONObject(screenXY, pageXY, (float) motionEvent.getPointerId(pointerIndex));
    float force = motionEvent.getPressure();
    if(force > 0 && force < 1) {
      map.put("force", motionEvent.getPressure());
    }
    return map;
  }

  /**
   * Create a touchObject for a pointer at a certain moment.
   * @param screenXY the point of event happened in screen coordinate
   * @param pageXY the point of event happened in page coorindate
   * @param pointerId pointerIndex pointerIndex
   * @return JSONObject represent a touch event
   * @see <a href="https://developer.mozilla.org/en-US/docs/Web/API/Touch">touch</a>
   */
  @NonNull
  private JSONObject createJSONObject(PointF screenXY, PointF pageXY, float pointerId) {
    JSONObject jsonObject = new JSONObject();
    jsonObject.put(GestureInfo.PAGE_X, pageXY.x);
    jsonObject.put(GestureInfo.PAGE_Y, pageXY.y);
    jsonObject.put(GestureInfo.SCREEN_X, screenXY.x);
    jsonObject.put(GestureInfo.SCREEN_Y, screenXY.y);
    jsonObject.put(GestureInfo.POINTER_ID, pointerId);
    return jsonObject;
  }

  /**
   * @see {@link #getEventLocInScreenCoordinate(MotionEvent, int, int)}
   */
  private PointF getEventLocInScreenCoordinate(MotionEvent motionEvent, int pointerIndex) {
    return getEventLocInScreenCoordinate(motionEvent, pointerIndex, CUR_EVENT);
  }

  /**
   * Get event location in Screen's coordinate, e.g. root(global) coordinate.
   * @see <a href="https://developer.mozilla.org/en-US/docs/Web/API/Touch/screenX">screenX</a>
   * @see <a href="https://developer.mozilla.org/en-US/docs/Web/API/Touch/screenY">screenY</a>
   * @param motionEvent the original motionEvent
   * @param pointerIndex pointerIndex
   * @param position if motionEvent.getHistoricalSize()!=0, the is the index of historical event,
   *                 otherwise this is {@link #CUR_EVENT} which indicates historicalSize is zero
   * @return the eventLocation in screen's coordinate
   */
  private PointF getEventLocInScreenCoordinate(MotionEvent motionEvent, int pointerIndex, int position) {
    float eventX, eventY;
    if (position == CUR_EVENT) {
      eventX = motionEvent.getX(pointerIndex);
      eventY = motionEvent.getY(pointerIndex);
    } else {
      eventX = motionEvent.getHistoricalX(pointerIndex, position);
      eventY = motionEvent.getHistoricalY(pointerIndex, position);
    }
    return getEventLocInScreenCoordinate(eventX, eventY);
  }

  /**
   * Get event location in Screen's coordinate, e.g. root(global) coordinate.
   * @param eventX {@link MotionEvent#getX()} or {@link MotionEvent#getHistoricalX(int, int)}
   * @param eventY {@link MotionEvent#getX()} or {@link MotionEvent#getHistoricalX(int, int)}
   * @return the eventLocation in screen's coordinate
   * @see {@link #getEventLocInScreenCoordinate(MotionEvent, int, int)}
   */
  @NonNull
  private PointF getEventLocInScreenCoordinate(float eventX, float eventY) {
    globalRect.set(0, 0, 0, 0);
    globalOffset.set(0, 0);
    globalEventOffset.set((int) eventX, (int) eventY);
    component.getRealView().getGlobalVisibleRect(globalRect, globalOffset);
    globalEventOffset.offset(globalOffset.x, globalOffset.y);
    return new PointF(WXViewUtils.getWebPxByWidth(globalEventOffset.x,component.getInstance().getInstanceViewPortWidth()),
            WXViewUtils.getWebPxByWidth(globalEventOffset.y,component.getInstance().getInstanceViewPortWidth()));
  }

  /**
   * @see {@link #getEventLocInPageCoordinate(MotionEvent, int, int)}
   */
  private PointF getEventLocInPageCoordinate(MotionEvent motionEvent, int pointerIndex) {
    return getEventLocInPageCoordinate(motionEvent, pointerIndex, CUR_EVENT);
  }

  /**
   * Get event's location in Document's (Page) coordinate.
   * @see <a href="https://developer.mozilla.org/en-US/docs/Web/API/Touch/pageX">pageX</a>
   * @see <a href="https://developer.mozilla.org/en-US/docs/Web/API/Touch/pageY">pageY</a>
   * @param motionEvent the original motionEvent
   * @param pointerIndex pointerIndex
   * @param position if motionEvent.getHistoricalSize()!=0, the is the index of historical event,
   *                 otherwise this is {@link #CUR_EVENT} which indicates historicalSize is zero
   * @return the event location in page's coordinate.
   */
  private PointF getEventLocInPageCoordinate(MotionEvent motionEvent, int pointerIndex, int position) {
    float eventX, eventY;
    if (position == CUR_EVENT) {
      eventX = motionEvent.getX(pointerIndex);
      eventY = motionEvent.getY(pointerIndex);
    } else {
      eventX = motionEvent.getHistoricalX(pointerIndex, position);
      eventY = motionEvent.getHistoricalY(pointerIndex, position);
    }
    return getEventLocInPageCoordinate(eventX, eventY);
  }

  /**
   * Get event's location in Document's (Page) coordinate.
   * @param eventX {@link MotionEvent#getX()} or {@link MotionEvent#getHistoricalX(int, int)}
   * @param eventY {@link MotionEvent#getX()} or {@link MotionEvent#getHistoricalX(int, int)}
   * @return the event location in page's coordinate.
   * @see {@link #getEventLocInPageCoordinate(MotionEvent, int, int)}
   */
  @NonNull
  private PointF getEventLocInPageCoordinate(float eventX, float eventY) {
    locEventOffset.set(eventX, eventY);
    locLeftTop.set(0, 0);
    component.computeVisiblePointInViewCoordinate(locLeftTop);
    locEventOffset.offset(locLeftTop.x, locLeftTop.y);
    return new PointF(WXViewUtils.getWebPxByWidth(locEventOffset.x,component.getInstance().getInstanceViewPortWidth()),
            WXViewUtils.getWebPxByWidth(locEventOffset.y,component.getInstance().getInstanceViewPortWidth()));
  }

  private static class GestureHandler extends android.os.Handler {

    public GestureHandler() {
      super(Looper.getMainLooper());
    }
  }


  /***************** OnGestureListener ****************/

  @Override
  public void onLongPress(MotionEvent e) {
    if (component.containsGesture(HighLevelGesture.LONG_PRESS)) {
      List<Map<String, Object>> list = createMultipleFireEventParam(e,null);
      component.getInstance().fireEvent(
              component.getRef(),
              HighLevelGesture.LONG_PRESS.toString(),
              list.get(list.size() - 1));
      mIsTouchEventConsumed = true;
    }
  }

  /**
   * Gesture priority：horizontalPan & verticalPan > pan > swipe
   */
  @Override
  public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX, float distanceY) {
    boolean result = false;
    if (e1 == null || e2 == null) {
      return false;
    }
    float dx = Math.abs(e2.getX() - e1.getX());
    float dy = Math.abs(e2.getY() - e1.getY());
    WXGestureType possiblePan;
    if (dx > dy) {
      possiblePan = HighLevelGesture.HORIZONTALPAN;
    } else {
      possiblePan = HighLevelGesture.VERTICALPAN;
    }
    if (mPendingPan == HighLevelGesture.HORIZONTALPAN || mPendingPan == HighLevelGesture.VERTICALPAN) {
      //already during directional-pan
      result = handlePanMotionEvent(e2);
    } else if (component.containsGesture(possiblePan)) {
      ViewParent p;
      if ((p = component.getRealView().getParent()) != null) {
        p.requestDisallowInterceptTouchEvent(true);
      }
      if (mPendingPan != null) {
        handleMotionEvent(mPendingPan, e2);//finish pan if exist
      }
      mPendingPan = possiblePan;
      component.fireEvent(possiblePan.toString(), createFireEventParam(e2, CUR_EVENT, START));

      result = true;
    } else if (containsSimplePan()) {
      if (panDownTime != e1.getEventTime()) {
        panDownTime = e1.getEventTime();
        mPendingPan = HighLevelGesture.PAN_END;
        component.fireEvent(HighLevelGesture.PAN_START.toString(),
                createFireEventParam(e1, CUR_EVENT, null));
      } else {
        component.fireEvent(HighLevelGesture.PAN_MOVE.toString(),
                createFireEventParam(e2, CUR_EVENT, null));
      }
      result = true;
    } else if (component.containsGesture(HighLevelGesture.SWIPE)) {
      if (swipeDownTime != e1.getEventTime()) {
        swipeDownTime = e1.getEventTime();
        List<Map<String, Object>> list = createMultipleFireEventParam(e2, null);
        Map<String, Object> param = list.get(list.size() - 1);
        if (Math.abs(distanceX) > Math.abs(distanceY)) {
          param.put(GestureInfo.DIRECTION, distanceX > 0 ? LEFT : RIGHT);
        } else {
          param.put(GestureInfo.DIRECTION, distanceY > 0 ? UP : DOWN);
        }
        component.getInstance().fireEvent(component.getRef(),
                HighLevelGesture.SWIPE.toString(), param);
        result = true;
      }
    }
    mIsTouchEventConsumed = mIsTouchEventConsumed || result;
    return result;
  }

  @Override
  public boolean onDown(MotionEvent e) {
    return true;
  }

  public boolean isRequestDisallowInterceptTouchEvent() {
    return requestDisallowInterceptTouchEvent;
  }

  public void setRequestDisallowInterceptTouchEvent(boolean requestDisallowInterceptTouchEvent) {
    this.requestDisallowInterceptTouchEvent = requestDisallowInterceptTouchEvent;
  }



}
