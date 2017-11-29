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

import android.app.Activity;
import android.content.Context;
import android.support.annotation.NonNull;
import android.text.Editable;
import android.text.InputFilter;
import android.text.InputType;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.text.method.PasswordTransformationMethod;
import android.util.TypedValue;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.View;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.EditText;
import android.widget.TextView;

import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.annotation.JSMethod;
import com.taobao.weex.bridge.WXBridgeManager;
import com.taobao.weex.common.Constants;
import com.taobao.weex.common.WXThread;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.dom.WXStyle;
import com.taobao.weex.ui.component.helper.SoftKeyboardDetector;
import com.taobao.weex.ui.component.helper.WXTimeInputHelper;
import com.taobao.weex.ui.view.WXEditText;
import com.taobao.weex.utils.WXLogUtils;
import com.taobao.weex.utils.WXResourceUtils;
import com.taobao.weex.utils.WXUtils;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Created by sospartan on 7/11/16.
 */
public abstract class AbstractEditComponent extends WXComponent<WXEditText> {

  private final InputMethodManager mInputMethodManager;
  private String mBeforeText = "";
  private boolean mAutoFocus;
  private String mType = "text";
  private String mMax = null;
  private String mMin = null;
  private String mLastValue = "";
  private int mEditorAction = EditorInfo.IME_ACTION_DONE;
  private String mReturnKeyType = null;
  private List<TextView.OnEditorActionListener> mEditorActionListeners;
  private boolean mListeningKeyboard = false;
  private SoftKeyboardDetector.Unregister mUnregister;
  private boolean mIgnoreNextOnInputEvent = false;

  public AbstractEditComponent(WXSDKInstance instance, WXDomObject dom, WXVContainer parent, boolean isLazy) {
    super(instance, dom, parent, isLazy);
    mInputMethodManager = (InputMethodManager) getContext().getSystemService(Context.INPUT_METHOD_SERVICE);
  }

  @Override
  protected WXEditText initComponentHostView(@NonNull Context context) {
    final WXEditText inputView = new WXEditText(context);
    appleStyleAfterCreated(inputView);
    return inputView;
  }

  @Override
  protected void onHostViewInitialized(WXEditText host) {
    super.onHostViewInitialized(host);
    addFocusChangeListener(new OnFocusChangeListener() {
      @Override
      public void onFocusChange(boolean hasFocus) {
        if (!hasFocus) {
          decideSoftKeyboard();
        }
        setPseudoClassStatus(Constants.PSEUDO.FOCUS,hasFocus);
      }
    });

    addKeyboardListener(host);
  }

  @Override
  protected boolean isConsumeTouch() {
    //EditText always consume touch event except disabled.
    return !isDisabled();
  }

  private void applyOnClickListener() {
    addClickListener(new OnClickListener() {
      @Override
      public void onHostViewClick() {
        switch (mType) {
          case Constants.Value.DATE:
            hideSoftKeyboard();
            if (getParent() != null) {
              getParent().interceptFocus();
            }
            WXTimeInputHelper.pickDate(mMax, mMin, AbstractEditComponent.this);
            break;
          case Constants.Value.TIME:
            hideSoftKeyboard();
            if (getParent() != null) {
              getParent().interceptFocus();
            }
            WXTimeInputHelper.pickTime(AbstractEditComponent.this);
            break;
        }
      }
    });
  }

  protected int getVerticalGravity(){
    return Gravity.CENTER_VERTICAL;
  }

  /**
   * Process view after created.
   *
   * @param editText
   */
  protected void appleStyleAfterCreated(WXEditText editText) {
    String alignStr = (String) getDomObject().getStyles().get(Constants.Name.TEXT_ALIGN);
    int textAlign = getTextAlign(alignStr);
    if (textAlign <= 0) {
      textAlign = Gravity.START;
    }
    editText.setGravity(textAlign | getVerticalGravity());
    int colorInt = WXResourceUtils.getColor("#999999");
    if (colorInt != Integer.MIN_VALUE) {
      editText.setHintTextColor(colorInt);
    }

    editText.setTextSize(TypedValue.COMPLEX_UNIT_PX, WXStyle.getFontSize(getDomObject().getStyles(),getInstance().getInstanceViewPortWidth()));
  }


  @Override
  public void addEvent(final String type) {
    super.addEvent(type);
    if (getHostView() == null || TextUtils.isEmpty(type)) {
      return;
    }
    final TextView text = getHostView();

    if (type.equals(Constants.Event.CHANGE)) {
      addFocusChangeListener(new OnFocusChangeListener() {
        @Override
        public void onFocusChange(boolean hasFocus) {
          if (hasFocus) {
            mLastValue = text.getText().toString();
          } else {
            CharSequence newValue = text.getText();
            newValue = newValue == null ? "" : newValue;
            if (!newValue.toString().equals(mLastValue)) {
              fireEvent(Constants.Event.CHANGE, newValue.toString());
              mLastValue = text.getText().toString();
            }
          }
        }
      });

      addEditorActionListener(new TextView.OnEditorActionListener() {
        @Override
        public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
          if (actionId == mEditorAction) {
            CharSequence newValue = text.getText();
            newValue = newValue == null ? "" : newValue;
            if (!newValue.toString().equals(mLastValue)) {
              fireEvent(Constants.Event.CHANGE, newValue.toString());
              mLastValue = text.getText().toString();
            }
            if (getParent() != null) {
              getParent().interceptFocus();
            }
            hideSoftKeyboard();
            return true;
          }
          return false;
        }
      });
    } else if (type.equals(Constants.Event.INPUT)) {
      text.addTextChangedListener(new TextWatcher() {
        @Override
        public void beforeTextChanged(CharSequence s, int start, int count, int after) {

        }

        @Override
        public void onTextChanged(CharSequence s, int start, int before, int count) {

          if (mIgnoreNextOnInputEvent) {
            mIgnoreNextOnInputEvent = false;
          }

          if (mBeforeText.equals(s.toString())) {
            return;
          }

          mBeforeText = s.toString();

          if (getDomObject() != null && getDomObject().getAttrs() != null) {
            Object val = getDomObject().getAttrs().get(Constants.Name.VALUE);
            String valString = WXUtils.getString(val, null);
            if (mBeforeText != null && mBeforeText.equals(valString)) {
              return;
            }
          }

          if (!mIgnoreNextOnInputEvent) {
            fireEvent(Constants.Event.INPUT, s.toString());
          }
        }

        @Override
        public void afterTextChanged(Editable s) {

        }
      });
    }

    if (Constants.Event.RETURN.equals(type)) {
      addEditorActionListener(new TextView.OnEditorActionListener() {
        @Override
        public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
          if (actionId == mEditorAction) {
            Map<String, Object> ret = new HashMap<>(2);
            ret.put("returnKeyType", mReturnKeyType);
            ret.put("value", v.getText().toString());
            fireEvent(Constants.Event.RETURN, ret);
            return true;
          }
          return false;
        }
      });
    }

    if (Constants.Event.KEYBOARD.equals(type)) {
      mListeningKeyboard = true;
    }
  }

  private void fireEvent(String event, String value) {
    if (event != null) {
      Map<String, Object> params = new HashMap<>(2);
      params.put("value", value);
      params.put("timeStamp", System.currentTimeMillis());

      Map<String, Object> domChanges = new HashMap<>();
      Map<String, Object> attrsChanges = new HashMap<>();
      attrsChanges.put("value", value);
      domChanges.put("attrs", attrsChanges);

      WXSDKManager.getInstance().fireEvent(getInstanceId(), getDomObject().getRef(), event, params, domChanges);
    }
  }

  public void performOnChange(String value) {
    if (getDomObject() != null && getDomObject().getEvents() != null) {
      String event = getDomObject().getEvents().contains(Constants.Event.CHANGE) ? Constants.Event.CHANGE : null;
      fireEvent(event, value);
    }
  }

  @Override
  protected boolean setProperty(String key, Object param) {
    switch (key) {
      case Constants.Name.PLACEHOLDER:
        String placeholder = WXUtils.getString(param,null);
        if (placeholder != null)
          setPlaceholder(placeholder);
        return true;
      case Constants.Name.PLACEHOLDER_COLOR:
        String placeholder_color = WXUtils.getString(param,null);
        if (placeholder_color != null)
          setPlaceholderColor(placeholder_color);
        return true;
      case Constants.Name.TYPE:
        String input_type = WXUtils.getString(param,null);
        if (input_type != null)
          setType(input_type);
        return true;
      case Constants.Name.AUTOFOCUS:
        Boolean result = WXUtils.getBoolean(param, null);
        if (result != null)
          setAutofocus(result);
        return true;
      case Constants.Name.COLOR:
        String color = WXUtils.getString(param,null);
        if (color != null)
          setColor(color);
        return true;
      case Constants.Name.FONT_SIZE:
        String fontsize = WXUtils.getString(param,null);
        if (fontsize != null)
          setFontSize(fontsize);
        return true;
      case Constants.Name.TEXT_ALIGN:
        String text_align = WXUtils.getString(param,null);
        if (text_align != null)
          setTextAlign(text_align);
        return true;
      case Constants.Name.SINGLELINE:
        Boolean singLineResult = WXUtils.getBoolean(param, null);
        if (singLineResult != null)
          setSingleLine(singLineResult);
        return true;
      case Constants.Name.LINES:
        Integer lines = WXUtils.getInteger(param, null);
        if (lines != null)
          setLines(lines);
        return true;
      case Constants.Name.MAX_LENGTH:
        Integer maxlength = WXUtils.getInteger(param, null);
        if (maxlength != null)
          setMaxLength(maxlength);
        return true;
      case Constants.Name.MAXLENGTH:
        Integer maxLength = WXUtils.getInteger(param, null);
        if (maxLength != null)
          setMaxLength(maxLength);
        return true;
      case Constants.Name.MAX:
        setMax(String.valueOf(param));
        return true;
      case Constants.Name.MIN:
        setMin(String.valueOf(param));
        return true;
      case Constants.Name.RETURN_KEY_TYPE:
        setReturnKeyType(String.valueOf(param));
        return true;
    }
    return super.setProperty(key, param);
  }

  @WXComponentProp(name = Constants.Name.RETURN_KEY_TYPE)
  public void setReturnKeyType(String type) {
    if (getHostView() == null) {
      return;
    }
    mReturnKeyType = type;
    switch (type) {
      case ReturnTypes.DEFAULT:
        mEditorAction = EditorInfo.IME_ACTION_UNSPECIFIED;
        break;
      case ReturnTypes.GO:
        mEditorAction = EditorInfo.IME_ACTION_GO;
        break;
      case ReturnTypes.NEXT:
        mEditorAction = EditorInfo.IME_ACTION_NEXT;
        break;
      case ReturnTypes.SEARCH:
        mEditorAction = EditorInfo.IME_ACTION_SEARCH;
        break;
      case ReturnTypes.SEND:
        mEditorAction = EditorInfo.IME_ACTION_SEND;
        break;
      case ReturnTypes.DONE:
        mEditorAction = EditorInfo.IME_ACTION_DONE;
        break;
      default:
        break;
    }

    //remove focus and hide keyboard first, the ImeOptions will take effect when show keyboard next time
    blur();
    getHostView().setImeOptions(mEditorAction);
  }

  @WXComponentProp(name = Constants.Name.PLACEHOLDER)
  public void setPlaceholder(String placeholder) {
    if (placeholder == null || getHostView() == null) {
      return;
    }
    ((WXEditText) getHostView()).setHint(placeholder);
  }

  @WXComponentProp(name = Constants.Name.PLACEHOLDER_COLOR)
  public void setPlaceholderColor(String color) {
    if (getHostView() != null && !TextUtils.isEmpty(color)) {
      int colorInt = WXResourceUtils.getColor(color);
      if (colorInt != Integer.MIN_VALUE) {
        ((WXEditText) getHostView()).setHintTextColor(colorInt);
      }
    }
  }

  @WXComponentProp(name = Constants.Name.TYPE)
  public void setType(String type) {
    if (type == null || getHostView() == null) {
      return;
    }
    mType = type;
    ((EditText) getHostView()).setRawInputType(getInputType(mType));
    switch (mType) {
      case Constants.Value.DATE:
      case Constants.Value.TIME:
        applyOnClickListener();
        break;
    }
  }

  @WXComponentProp(name = Constants.Name.AUTOFOCUS)
  public void setAutofocus(boolean autofocus) {
    if (getHostView() == null) {
      return;
    }
    mAutoFocus = autofocus;
    EditText inputView = getHostView();
    if (mAutoFocus) {
      inputView.setFocusable(true);
      inputView.requestFocus();
      inputView.setFocusableInTouchMode(true);
      showSoftKeyboard();
    } else {
      hideSoftKeyboard();
    }
  }

  @WXComponentProp(name = Constants.Name.VALUE)
  public void setValue(String value){
    WXEditText view;
    if((view = getHostView()) == null){
      return;
    }

    mIgnoreNextOnInputEvent = true;
    view.setText(value);
    view.setSelection(value == null ? 0 : value.length());
  }

  @WXComponentProp(name = Constants.Name.COLOR)
  public void setColor(String color) {
    if (getHostView() != null && !TextUtils.isEmpty(color)) {
      int colorInt = WXResourceUtils.getColor(color);
      if (colorInt != Integer.MIN_VALUE) {
        getHostView().setTextColor(colorInt);
      }
    }
  }

  @WXComponentProp(name = Constants.Name.FONT_SIZE)
  public void setFontSize(String fontSize) {
    if (getHostView() != null && fontSize != null ) {
      Map<String, Object> map = new HashMap<>(1);
      map.put(Constants.Name.FONT_SIZE, fontSize);
      getHostView().setTextSize(TypedValue.COMPLEX_UNIT_PX, WXStyle.getFontSize(map, getInstance().getInstanceViewPortWidth()));
    }
  }

  @WXComponentProp(name = Constants.Name.TEXT_ALIGN)
  public void setTextAlign(String textAlign) {
    int align = getTextAlign(textAlign);
    if (align > 0) {
      getHostView().setGravity(align | getVerticalGravity());
    }
  }

  @WXComponentProp(name = Constants.Name.SINGLELINE)
  public void setSingleLine(boolean singleLine) {
    if (getHostView() == null) {
      return;
    }
    getHostView().setSingleLine(singleLine);
  }

  @WXComponentProp(name = Constants.Name.LINES)
  public void setLines(int lines) {
    if (getHostView() == null) {
      return;
    }
    getHostView().setLines(lines);
  }

  /**
   * Compatible with both 'max-length' and 'maxlength'
   * @param maxLength
   */
  @WXComponentProp(name = Constants.Name.MAX_LENGTH)
  public void setMaxLength(int maxLength) {
    if (getHostView() == null) {
      return;
    }
    getHostView().setFilters(new InputFilter[]{new InputFilter.LengthFilter(maxLength)});
  }

  /**
   * Compatible with both 'max-length' and 'maxlength'
   * @param maxLength
   */
  @WXComponentProp(name = Constants.Name.MAXLENGTH)
  @Deprecated
  public void setMaxlength(int maxLength) {
    setMaxLength(maxLength);
  }

  private int getInputType(String type) {
    int inputType;
    switch (type) {
      case Constants.Value.TEXT:
        inputType = InputType.TYPE_CLASS_TEXT;
        break;
      case Constants.Value.DATE:
        inputType = InputType.TYPE_NULL;
        getHostView().setFocusable(false);
        break;
      case Constants.Value.DATETIME:
        inputType = InputType.TYPE_CLASS_DATETIME;
        break;
      case Constants.Value.EMAIL:
        inputType = InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_EMAIL_ADDRESS;
        break;
      case Constants.Value.PASSWORD:
        inputType = InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_PASSWORD;
        getHostView().setTransformationMethod(PasswordTransformationMethod.getInstance());
        break;
      case Constants.Value.TEL:
        inputType = InputType.TYPE_CLASS_PHONE;
        break;
      case Constants.Value.TIME:
        inputType = InputType.TYPE_NULL;
        getHostView().setFocusable(false);
        break;
      case Constants.Value.URL:
        inputType = InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_URI;
        break;
      case Constants.Value.NUMBER:
        inputType = InputType.TYPE_CLASS_NUMBER;
        break;
      default:
        inputType = InputType.TYPE_CLASS_TEXT;
    }
    return inputType;
  }

  @WXComponentProp(name = Constants.Name.MAX)
  public void setMax(String max) {
    mMax = max;
  }

  @WXComponentProp(name = Constants.Name.MIN)
  public void setMin(String min) {
    mMin = min;
  }

  private boolean showSoftKeyboard() {
    if (getHostView() == null) {
      return false;
    } else {
      getHostView().postDelayed(WXThread.secure(new Runnable() {
        @Override
        public void run() {
          mInputMethodManager.showSoftInput(getHostView(), InputMethodManager.SHOW_IMPLICIT);
        }
      }), 100);
    }
    return true;
  }

  private void hideSoftKeyboard() {
    if (getHostView() != null) {
      getHostView().postDelayed(WXThread.secure(new Runnable() {
        @Override
        public void run() {
          mInputMethodManager.hideSoftInputFromWindow(getHostView().getWindowToken(), 0);
        }
      }), 16);
    }
  }

  private int getTextAlign(String textAlign) {
    int align = Gravity.START;
    if (TextUtils.isEmpty(textAlign)) {
      return align;
    }
    if (textAlign.equals(Constants.Value.LEFT)) {
      align = Gravity.START;
    } else if (textAlign.equals(Constants.Value.CENTER)) {
      align = Gravity.CENTER;
    } else if (textAlign.equals(Constants.Value.RIGHT)) {
      align = Gravity.END;
    }
    return align;
  }

  @JSMethod
  public void blur() {
    WXEditText host = getHostView();
    if (host != null && host.hasFocus()) {
      if (getParent() != null) {
        getParent().interceptFocus();
      }
      host.clearFocus();
      hideSoftKeyboard();
    }
  }

  @JSMethod
  public void focus() {
    WXEditText host = getHostView();
    if (host != null && !host.hasFocus()) {
      if (getParent() != null) {
        getParent().ignoreFocus();
      }
      host.requestFocus();
      host.setFocusable(true);
      host.setFocusableInTouchMode(true);
      showSoftKeyboard();
    }
  }

  @Override
  protected Object convertEmptyProperty(String propName, Object originalValue) {
    switch (propName) {
      case Constants.Name.FONT_SIZE:
        return WXText.sDEFAULT_SIZE;
      case Constants.Name.COLOR:
        return "black";
    }
    return super.convertEmptyProperty(propName, originalValue);
  }

  private void decideSoftKeyboard() {
    View hostView;
    if ((hostView = getHostView()) != null) {
      final Context context = getContext();
      if (context != null && context instanceof Activity) {
        hostView.postDelayed(WXThread.secure(new Runnable() {
          @Override
          public void run() {
            View currentFocus = ((Activity) context).getCurrentFocus();
            if (!(currentFocus instanceof EditText)) {
              mInputMethodManager.hideSoftInputFromWindow(getHostView().getWindowToken(), 0);
            }
          }
        }), 16);
      }
    }
  }

  @JSMethod
  public void setSelectionRange(int selectionStart, int selectionEnd) {
    EditText hostView;
    if ((hostView = getHostView()) != null) {
      int length = getHostView().length();
      if (selectionStart > length || selectionEnd > length) {
        return;
      }
      focus();
      hostView.setSelection(selectionStart, selectionEnd);
    }
  }

  @JSMethod
  public void getSelectionRange(String callbackId) {
    EditText hostView;
    Map<String, Object> result = new HashMap<>(2);
    if ((hostView = getHostView()) != null) {
      int start = hostView.getSelectionStart();
      int end = hostView.getSelectionEnd();

      if (!hostView.hasFocus()) {
        //The default behavior, same as iOS and web
        start = 0;
        end = 0;
      }

      result.put(Constants.Name.SELECTION_START, start);
      result.put(Constants.Name.SELECTION_END, end);
    }
    WXBridgeManager.getInstance().callback(getInstanceId(), callbackId, result, false);
  }

  protected final void addEditorActionListener(TextView.OnEditorActionListener listener) {
    TextView view;
    if (listener != null && (view = getHostView()) != null) {
      if (mEditorActionListeners == null) {
        mEditorActionListeners = new ArrayList<>();
        view.setOnEditorActionListener(new TextView.OnEditorActionListener() {
          private boolean handled = true;

          @Override
          public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
            for (TextView.OnEditorActionListener l : mEditorActionListeners) {
              if (l != null) {
                handled = handled & l.onEditorAction(v, actionId, event);
              }
            }
            return handled;
          }
        });
      }
      mEditorActionListeners.add(listener);
    }
  }

  private void addKeyboardListener(final WXEditText host) {
    if (host == null) {
      return;
    }
    Context context = host.getContext();
    if (context != null && context instanceof Activity) {
      SoftKeyboardDetector.registerKeyboardEventListener((Activity) context, new SoftKeyboardDetector.OnKeyboardEventListener() {
        @Override
        public void onKeyboardEvent(boolean isShown) {
          if (mListeningKeyboard) {
            Map<String, Object> event = new HashMap<>(1);
            event.put("isShow", isShown);
            fireEvent(Constants.Event.KEYBOARD, event);
          }
          if (!isShown) {
            blur();
          }
        }
      });
    }
  }

  @Override
  public void destroy() {
    super.destroy();
    if (mUnregister != null) {
      try {
        mUnregister.execute();
        mUnregister = null;
      } catch (Throwable throwable) {
        WXLogUtils.w("Unregister throw ", throwable);
      }
    }
  }

  private interface ReturnTypes {
    String DEFAULT = "default";
    String GO = "go";
    String NEXT = "next";
    String SEARCH = "search";
    String SEND = "send";
    String DONE = "done";
  }
}
