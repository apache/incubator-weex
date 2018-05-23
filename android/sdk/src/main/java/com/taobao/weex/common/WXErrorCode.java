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
package com.taobao.weex.common;


public enum WXErrorCode {


  /*
   * environment
   **/
  /**
   * Failure for load so library
   */
  WX_ERR_LOAD_SO("-2001", "load so error",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),

  /**
   * Failure for load JSLib
   */
  WX_ERR_LOAD_JSLIB("-2002", "unzip JSLib error!",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),

  /**
   * So library corrupted
   */
  WX_ERR_BAD_SO("-2003", "so size error, to reload apk so",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),

  /**
   * Failure for extract and copy so from apk
   */
  WX_ERR_COPY_FROM_APK("-2007", "system load so error，copy from APK also error!",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),

  /**
   * jsBridge
   **/
  /**
   * Error for JavaScript function parameter
   */
  WX_ERR_JSFUNC_PARAM("-2009", "JS params error!",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),

  /**
   * Error for parsing JSON object
   */
  WX_ERR_JSON_OBJECT("-2008", "transform JSON->OBJ  error!",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),

  /**
   * Failure for invoking JavaScript function
   */
  WX_ERR_INVOKE_NATIVE("-2012", "Native-> Call ->JS  error!",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),

  /**
   * Failure for executing JavaScript function.
   */
  WX_ERR_JS_EXECUTE("-2013", "JavaScript execute error!",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),


  /*
   * domModule
   **/
  WX_SUCCESS("0", "successful",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),
  /**
   * Failure for createBody
   */
  WX_ERR_DOM_CREATEBODY("-2100", "createBody error",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),
  /**
   * Failure for updateAttrs
   */
  WX_ERR_DOM_UPDATEATTRS("-2101", "updateAttrs error",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),
  /**
   * Failure for updateAttr
   */
  WX_ERR_DOM_UPDATESTYLE("-2102", "updateStyle error",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),
  /**
   * Failure for addElement
   */
  WX_ERR_DOM_ADDELEMENT("-2103", "addElement error",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),
  /**
   * Failure for removeElement
   */
  WX_ERR_DOM_REMOVEELEMENT("-2104", "removeElement error",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),
  /**
   * Failure for moveElement
   */
  WX_ERR_DOM_MOVEELEMENT("-2105", "moveElement error",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),
  /**
   * Failure for addEvent
   */
  WX_ERR_DOM_ADDEVENT("-2106", "addEvent error",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),
  /**
   * Failure for removeEvent
   */
  WX_ERR_DOM_REMOVEEVENT("-2107", "removeEvent error",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),
  /**
   * Failure for invoking createFinish
   */
  WX_ERROR_DOM_CREATEFINISH("-2108", "createFinish error",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),
  /**
   * Failure for invoking refreshFinish
   */
  WX_ERROR_DOM_REFRESHFINISH("-2109", "refreshFinish error",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),
  /**
   * Failure for scrollToElement
   */
  WX_ERR_DOM_SCROLLTO("-2110", "scrollToElement",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),

  /**
   *
   */
  WX_ERR_RELOAD_PAGE("-2111", "reloadPage",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),

  /**
   *
   */
  WX_ERR_JSC_CRASH("-2112", "weexjscCrash",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),

  /**
   *dom actions invalid for native
   */
  WX_ERR_FIRST_DOM_ACTION_EXCEPTION("-2113", "dom action is invalid ",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),

  /**
   * JS Bundle download error
   */

  WX_ERR_JSDOWNLOAD_START("-2201", "js bundle download start",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),

  WX_ERR_JSBUNDLE_DOWNLOAD("-2299", "js bundle download err",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),

  WX_ERR_JSBUNDLE_EMPTY("-2203", "js bundle empty",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),

  WX_ERR_JSDOWNLOAD_END("-2299", "js bundle download end",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),

  // for js framework
  WX_JS_FRAMEWORK_INIT_SUCCESS("-1000", "js framework success",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),

  WX_JS_FRAMEWORK_REINIT_SUCCESS("-1001", "js framework reinit success",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),
  /**
   * JS Framework run error
   */
  WX_ERR_JS_FRAMEWORK("-1002", "js framework error",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),

  WX_ERR_JS_REINIT_FRAMEWORK("-1003", "js reinit framework error",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),
  /**
   * Single progress init error
   */
  WX_ERR_SINGLE_PROCESS_DLOPEN_FILE_NOT_EXIST("-1004", "so file is not exist",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),

  WX_ERR_SINGLE_PROCESS_DLOPEN_FLAIED("-1005", "dlopen so file failed",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),

  WX_ERR_SINGLE_PROCESS_DLSYM_FAILED("-1006", "find function from so file failed",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),

  WX_ERR_SINGLE_PROCESS_JS_FRAMEWORK("-1007", "js framework  init singleProcess failed",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),

  WX_JS_FRAMEWORK_INIT_MULPROCESS_FAILED("-1008", "js framework init multiProcess failed",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),

  WX_JS_FRAMEWORK_REINIT_MULPROCESS_FAILED("-1009", "js framework reinit multiProcess failed",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),

  WX_JS_FRAMEWORK_INIT_FAILED("-1010", "js framework init  failed",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),

  WX_JS_FRAMEWORK_INIT_SINGLE_PROCESS_SUCCESS("-1011", "js framework init success in single process",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),


  /**
   * WX Key Exception Commit RT SDK Init
   */
  WX_KEY_EXCEPTION_SDK_INIT("-9000", "[WX_KEY_EXCEPTION_SDK_INIT]",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),
  WX_KEY_EXCEPTION_SDK_INIT_CPU_NOT_SUPPORT("-9001", "[WX_KEY_EXCEPTION_SDK_INIT_CPU_NOT_SUPPORT] for android cpu is x86",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),
  WX_KEY_EXCEPTION_SDK_INIT_TABLE_NOT_SUPPORT("-9002", "[WX_KEY_EXCEPTION_SDK_INIT_TABLE_NOT_SUPPORT] for device isTabletDevice",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),
  WX_KEY_EXCEPTION_SDK_INIT_JSFM_INIT_FAILED("-9003", "[WX_KEY_EXCEPTION_SDK_INIT_JSFM_INIT_FAILED] for jsfm init failed|detail error is:",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),

  /**
   * WX Key Exception Commit RT Register
   */
  WX_KEY_EXCEPTION_INVOKE("-9100", "[WX_KEY_EXCEPTION_INVOKE]",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),
  WX_KEY_EXCEPTION_INVOKE_REGISTER_CONTENT_FAILED("-9101", "[WX_KEY_EXCEPTION_INVOKE_REGISTER_CONTENT_FAILED] details",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),
  WX_KEY_EXCEPTION_INVOKE_JSSERVICE_EXECUTE("-9102", "[WX_KEY_EXCEPTION_INVOKE_JSSERVICE_EXECUTE] details",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),
  WX_KEY_EXCEPTION_INVOKE_REGISTER_MODULES("-9103", "[WX_KEY_EXCEPTION_INVOKE_REGISTER_MODULES] details",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),

  /**
   * WX Key Exception Commit Bundle Js Download
   */
  WX_KEY_EXCEPTION_JS_DOWNLOAD("-9200", "[WX_KEY_EXCEPTION_JS_DOWNLOAD]|",ErrorType.NATIVE_ERROR,ErrorGroup.NET),
  WX_KEY_EXCEPTION_JS_DOWNLOAD_FAILED("-9201", "[WX_KEY_EXCEPTION_JS_DOWNLOAD_FAILED] | details",ErrorType.NATIVE_ERROR,ErrorGroup.NET),

  /**
   * WX Key Exception Commit RT DomAction Excute
   */
  WX_KEY_EXCEPTION_DOM("-9300", "[WX_KEY_EXCEPTION_DOM]",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),
  WX_KEY_EXCEPTION_DOM_ACTION_FIRST_ACTION("-9301", "[WX_KEY_EXCEPTION dom action exception] details: ACTION_FIRST_ACTION",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),
  WX_KEY_EXCEPTION_DOM_CREATE_BODY("-9302", "[WX_KEY_EXCEPTION dom action exception] details:CREATE_BODY",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),
  WX_KEY_EXCEPTION_DOM_UPDATE_ATTRS("-9303", "[WX_KEY_EXCEPTION dom action exception] details:UPDATE_ATTRS",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),
  WX_KEY_EXCEPTION_DOM_UPDATE_STYLE("-9304", "[WX_KEY_EXCEPTION dom action exception] details:UPDATE_STYLE",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),
  WX_KEY_EXCEPTION_DOM_REMOVE_ELEMENT("-9305", "[WX_KEY_EXCEPTION dom action exception] details:REMOVE_ELEMENT",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),
  WX_KEY_EXCEPTION_DOM_ADD_ELEMENT("-9306", "[WX_KEY_EXCEPTION dom action exception] details:ADD_ELEMENT",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),
  WX_KEY_EXCEPTION_DOM_MOVE_ELEMENT("-9307", "[WX_KEY_EXCEPTION dom action exception] details:MOVE_ELEMENT",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),
  WX_KEY_EXCEPTION_DOM_ADD_EVENT("-9308", "[WX_KEY_EXCEPTION dom action exception] details:DOM_ADD_EVENT",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),
  WX_KEY_EXCEPTION_DOM_REMOVE_EVENT("-9309", "[WX_KEY_EXCEPTION dom action exception] details:DOM_REMOVE_EVENT",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),
  WX_KEY_EXCEPTION_DOM_CREATE_FINISH("-9310", "[WX_KEY_EXCEPTION dom action exception] details:CREATE_FINISH",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),
  WX_KEY_EXCEPTION_DOM_REFRESH_FINISH("-9311", "[WX_KEY_EXCEPTION dom action exception] details:REFRESH_FINISH",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),
  WX_KEY_EXCEPTION_DOM_ADD_RULE("-9313", "[WX_KEY_EXCEPTION_DOM_ADD_RULE] details",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),
  WX_KEY_EXCEPTION_DOM_GET_COMPONENT_RECT("-9314", "[WX_KEY_EXCEPTION_DOM_GET_COMPONENT_RECT] details",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),
  WX_KEY_EXCEPTION_DOM_INVOKE_METHOD("-9315", "[WX_KEY_EXCEPTION_DOM_INVOKE_METHOD] details",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),
  WX_KEY_EXCEPTION_DOM_ANIMATION("-9316", "[WX_KEY_EXCEPTION_DOM_ANIMATION] details",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),
  WX_KEY_EXCEPTION_DOM_MODULEINVO("-9317", "[WX_KEY_EXCEPTION_DOM_MODULEINVO] details",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE),


  /**
   * WX Key Exception Commit RT JsBridge eg. js excute runtime error
   */
  WX_KEY_EXCEPTION_WXBRIDGE("-9400", "[js excute runtime error] detail js stack \n",ErrorType.JS_ERROR,ErrorGroup.JS),
  WX_KEY_EXCEPTION_WXBRIDGE_EXCEPTION("-9401", "[js excute runtime error] detail js stack \n",ErrorType.JS_ERROR,ErrorGroup.JS),

  /**
   * renderErrorCode
   */
  WX_RENDER_ERR_JS_CREATE_INSTANCE("-9600", "WX_RENDER_ERR_JS_CREATE_INSTANCE",ErrorType.RENDER_ERROR,ErrorGroup.JS),
  WX_RENDER_ERR_JS_RUNTIME("-9601", "WX_RENDER_ERR for js error",ErrorType.RENDER_ERROR, ErrorGroup.JS),
  WX_RENDER_ERR_LAYER_OVERFLOW("-9602", "WX_RENDER_ERR_LAYER_OVERFLOW", ErrorType.NATIVE_ERROR, ErrorGroup.NATIVE),
  WX_RENDER_ERR_NULL_KEY("-9603", "WX_RENDER_ERR_NULL_KEY", ErrorType.NATIVE_ERROR, ErrorGroup.NATIVE),
  WX_RENDER_ERR_CONTAINER_TYPE("-9611", "WX_RENDER_ERR_CONTAINER_TYPE", ErrorType.JS_ERROR,ErrorGroup.JS),



  /**
   * degrade code.
   */
  WX_DEGRAD_ERR("-1000", "degradeToH5|Weex DegradPassivity \n",ErrorType.DEGRAD_ERROR,ErrorGroup.JS),

  /**
   * degrade for instance create failed, once this case occured,detail js stack and other specific
   * cause need track into errmsg.
   */
  WX_DEGRAD_ERR_INSTANCE_CREATE_FAILED("-1001", "degradeToH5|createInstance fail|wx_create_instance_error",ErrorType.DEGRAD_ERROR,ErrorGroup.NATIVE),

  /**
   * degrade for network failed download js bundle.once this case occured,network requist response header
   * and statuscode need track into errmsg.
   */
  WX_DEGRAD_ERR_NETWORK_BUNDLE_DOWNLOAD_FAILED("-1002", "|wx_network_error|js bundle download failed",ErrorType.DEGRAD_ERROR,ErrorGroup.NET),

  /**
   * degrade for network failed for bundlejs is unbroken , once this case occured,network requist response header
   * and statuscode need track into errmsg.
   */
  WX_DEGRAD_ERR_NETWORK_CHECK_CONTENT_LENGTH_FAILED("-1003", "degradeToH5|wx_network_error|js bundle content-length check failed",ErrorType.DEGRAD_ERROR,ErrorGroup.NET),

  /**
   * degrade for Response header Content-Type is null or not "application/javascript".
   * once this case occured,network requist response header and statuscode need track into errmsg.
   */
  WX_DEGRAD_ERR_BUNDLE_CONTENTTYPE_ERROR("-1004", "degradeToH5|wx_user_intercept_error |Content-Type is not application/javascript, " +
          "Weex render template must be javascript, please check your request!",ErrorType.DEGRAD_ERROR,ErrorGroup.NET),

  /**
   * degrade for other reason. such as white screen which block error for some unknown reason.
   * once this case occured,detail msg need track.
   */
  WX_DEGRAD_ERR_OTHER_CAUSE_DEGRADTOH5("-1005", "degradeToH5|for other reason|",ErrorType.DEGRAD_ERROR,ErrorGroup.NATIVE),


  WX_DEGRAD_ERR_INSTANCE_CREATE_FAILED_JS("-1006", "degradeToH5|createInstance fail|wx_create_instance_error",ErrorType.DEGRAD_ERROR,ErrorGroup.JS),



  /**
   * TEST
   */

  WX_ERR_TEST("test", "test",ErrorType.NATIVE_ERROR,ErrorGroup.NATIVE);

  private String errorCode;
  private String errorMsg;
  private String appendMsg="";
  private String args;
  private ErrorType mErrorType;
  private ErrorGroup mErrorGroup;

  WXErrorCode(String errorCode, String errorMsg,ErrorType errorType,ErrorGroup errorGroup) {
    this.errorCode = errorCode;
    this.errorMsg = errorMsg;
    this.mErrorType = errorType;
    this.mErrorGroup = errorGroup;
  }

  public void appendErrMsg(String err) {
    appendMsg=err;
  }

  public String getErrorCode() {
    return this.errorCode;
  }

  public String getErrorMsg() {
    StringBuilder builder=new StringBuilder(errorMsg);
    builder.append(appendMsg);
    return builder.toString();
  }

  public ErrorType getErrorType() {
    return mErrorType;
  }

  public ErrorGroup getErrorGroup() {
    return mErrorGroup;
  }

  public String getArgs() {
    return args;
  }

  public void setArgs(String args) {
    this.args = args;
  }

  public enum ErrorType{
    JS_ERROR,
    NATIVE_ERROR,
    RENDER_ERROR,
    DEGRAD_ERROR
  }

  public enum ErrorGroup{
    JS,
    NATIVE,
    NET
  }
}
