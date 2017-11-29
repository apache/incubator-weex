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
  WX_ERR_LOAD_SO("-2001", "load so error"),

  /**
   * Failure for load JSLib
   */
  WX_ERR_LOAD_JSLIB("-2002", "unzip JSLib error!"),

  /**
   * So library corrupted
   */
  WX_ERR_BAD_SO("-2003", "so size error, to reload apk so"),

  /**
   * Failure for extract and copy so from apk
   */
  WX_ERR_COPY_FROM_APK("-2007", "system load so error，copy from APK also error!"),

  /**
   * jsBridge
   **/
  /**
   * Error for JavaScript function parameter
   */
  WX_ERR_JSFUNC_PARAM("-2009", "JS params error!"),

  /**
   * Error for parsing JSON object
   */
  WX_ERR_JSON_OBJECT("-2008", "transform JSON->OBJ  error!"),

  /**
   * Failure for invoking JavaScript function
   */
  WX_ERR_INVOKE_NATIVE("-2012", "Native-> Call ->JS  error!"),

  /**
   * Failure for executing JavaScript function.
   */
  WX_ERR_JS_EXECUTE("-2013", "JavaScript execute error!"),


  /*
   * domModule
   **/
  WX_SUCCESS("0", "successful"),
  /**
   * Failure for createBody
   */
  WX_ERR_DOM_CREATEBODY("-2100", "createBody error"),
  /**
   * Failure for updateAttrs
   */
  WX_ERR_DOM_UPDATEATTRS("-2101", "updateAttrs error"),
  /**
   * Failure for updateAttr
   */
  WX_ERR_DOM_UPDATESTYLE("-2102", "updateStyle error"),
  /**
   * Failure for addElement
   */
  WX_ERR_DOM_ADDELEMENT("-2103", "addElement error"),
  /**
   * Failure for removeElement
   */
  WX_ERR_DOM_REMOVEELEMENT("-2104", "removeElement error"),
  /**
   * Failure for moveElement
   */
  WX_ERR_DOM_MOVEELEMENT("-2105", "moveElement error"),
  /**
   * Failure for addEvent
   */
  WX_ERR_DOM_ADDEVENT("-2106", "addEvent error"),
  /**
   * Failure for removeEvent
   */
  WX_ERR_DOM_REMOVEEVENT("-2107", "removeEvent error"),
  /**
   * Failure for invoking createFinish
   */
  WX_ERROR_DOM_CREATEFINISH("-2108", "createFinish error"),
  /**
   * Failure for invoking refreshFinish
   */
  WX_ERROR_DOM_REFRESHFINISH("-2109", "refreshFinish error"),
  /**
   * Failure for scrollToElement
   */
  WX_ERR_DOM_SCROLLTO("-2110", "scrollToElement"),

  /**
   *
   */
  WX_ERR_RELOAD_PAGE("-2111", "reloadPage"),

  /**
   *
   */
  WX_ERR_JSC_CRASH("-2112", "weexjscCrash"),

  /**
   *dom actions invalid for native
   */
  WX_ERR_FIRST_DOM_ACTION_EXCEPTION("-2113", "dom action is invalid "),

  /**
   * JS Bundle download error
   */

  WX_ERR_JSDOWNLOAD_START("-2201", "js bundle download start"),

  WX_ERR_JSBUNDLE_DOWNLOAD("-2299", "js bundle download err"),

  WX_ERR_JSBUNDLE_EMPTY("-2203", "js bundle empty"),

  WX_ERR_JSDOWNLOAD_END("-2299", "js bundle download end"),

  /**
   * JS Framework run error
   */
  WX_ERR_JS_FRAMEWORK("-1002", "js framework error"),

  WX_ERR_JS_REINIT_FRAMEWORK("-1003", "js reinit framework error"),


  /**
   * WX Key Exception Commit RT SDK Init
   */
  WX_KEY_EXCEPTION_SDK_INIT("-9000", "[WX_KEY_EXCEPTION_SDK_INIT]"),
  WX_KEY_EXCEPTION_SDK_INIT_CPU_NOT_SUPPORT("-9001", "[WX_KEY_EXCEPTION_SDK_INIT_CPU_NOT_SUPPORT] for android cpu is x86"),
  WX_KEY_EXCEPTION_SDK_INIT_TABLE_NOT_SUPPORT("-9002", "[WX_KEY_EXCEPTION_SDK_INIT_TABLE_NOT_SUPPORT] for device isTabletDevice"),
  WX_KEY_EXCEPTION_SDK_INIT_JSFM_INIT_FAILED("-9003", "[WX_KEY_EXCEPTION_SDK_INIT_JSFM_INIT_FAILED] for jsfm init failed|detail error is:"),

  /**
   * WX Key Exception Commit RT Register
   */
  WX_KEY_EXCEPTION_INVOKE("-9100", "[WX_KEY_EXCEPTION_INVOKE]"),
  WX_KEY_EXCEPTION_INVOKE_REGISTER_CONTENT_FAILED("-9101", "[WX_KEY_EXCEPTION_INVOKE_REGISTER_CONTENT_FAILED] details"),
  WX_KEY_EXCEPTION_INVOKE_JSSERVICE_EXECUTE("-9102", "[WX_KEY_EXCEPTION_INVOKE_JSSERVICE_EXECUTE] details"),
  WX_KEY_EXCEPTION_INVOKE_REGISTER_MODULES("-9103", "[WX_KEY_EXCEPTION_INVOKE_REGISTER_MODULES] details"),

  /**
   * WX Key Exception Commit Bundle Js Download
   */
  WX_KEY_EXCEPTION_JS_DOWNLOAD("-9200", "[WX_KEY_EXCEPTION_JS_DOWNLOAD]|"),
  WX_KEY_EXCEPTION_JS_DOWNLOAD_FAILED("-9201", "[WX_KEY_EXCEPTION_JS_DOWNLOAD_FAILED] | details"),

  /**
   * WX Key Exception Commit RT DomAction Excute
   */
  WX_KEY_EXCEPTION_DOM("-9300", "[WX_KEY_EXCEPTION_DOM]"),
  WX_KEY_EXCEPTION_DOM_ACTION_FIRST_ACTION("-9301", "[WX_KEY_EXCEPTION dom action exception] details: ACTION_FIRST_ACTION"),
  WX_KEY_EXCEPTION_DOM_CREATE_BODY("-9302", "[WX_KEY_EXCEPTION dom action exception] details:CREATE_BODY"),
  WX_KEY_EXCEPTION_DOM_UPDATE_ATTRS("-9303", "[WX_KEY_EXCEPTION dom action exception] details:UPDATE_ATTRS"),
  WX_KEY_EXCEPTION_DOM_UPDATE_STYLE("-9304", "[WX_KEY_EXCEPTION dom action exception] details:UPDATE_STYLE"),
  WX_KEY_EXCEPTION_DOM_REMOVE_ELEMENT("-9305", "[WX_KEY_EXCEPTION dom action exception] details:REMOVE_ELEMENT"),
  WX_KEY_EXCEPTION_DOM_ADD_ELEMENT("-9306", "[WX_KEY_EXCEPTION dom action exception] details:ADD_ELEMENT"),
  WX_KEY_EXCEPTION_DOM_MOVE_ELEMENT("-9307", "[WX_KEY_EXCEPTION dom action exception] details:MOVE_ELEMENT"),
  WX_KEY_EXCEPTION_DOM_ADD_EVENT("-9308", "[WX_KEY_EXCEPTION dom action exception] details:DOM_ADD_EVENT"),
  WX_KEY_EXCEPTION_DOM_REMOVE_EVENT("-9309", "[WX_KEY_EXCEPTION dom action exception] details:DOM_REMOVE_EVENT"),
  WX_KEY_EXCEPTION_DOM_CREATE_FINISH("-9310", "[WX_KEY_EXCEPTION dom action exception] details:CREATE_FINISH"),
  WX_KEY_EXCEPTION_DOM_REFRESH_FINISH("-9311", "[WX_KEY_EXCEPTION dom action exception] details:REFRESH_FINISH"),
  WX_KEY_EXCEPTION_DOM_ADD_RULE("-9313", "[WX_KEY_EXCEPTION_DOM_ADD_RULE] details"),
  WX_KEY_EXCEPTION_DOM_GET_COMPONENT_RECT("-9314", "[WX_KEY_EXCEPTION_DOM_GET_COMPONENT_RECT] details"),
  WX_KEY_EXCEPTION_DOM_INVOKE_METHOD("-9315", "[WX_KEY_EXCEPTION_DOM_INVOKE_METHOD] details"),
  WX_KEY_EXCEPTION_DOM_ANIMATION("-9316", "[WX_KEY_EXCEPTION_DOM_ANIMATION] details"),
  WX_KEY_EXCEPTION_DOM_MODULEINVO("-9317", "[WX_KEY_EXCEPTION_DOM_MODULEINVO] details"),



  /**
   * WX Key Exception Commit RT JsBridge eg. js excute runtime error
   */
  WX_KEY_EXCEPTION_WXBRIDGE("-9400", "[js excute runtime error] detail js stack \n"),
  WX_KEY_EXCEPTION_WXBRIDGE_EXCEPTION("-9401", "[js excute runtime error] detail js stack \n");

  private String errorCode;
  private String errorMsg;
  private String appendMsg="";
  private String args;

  WXErrorCode(String errorCode, String errorMsg) {
    this.errorCode = errorCode;
    this.errorMsg = errorMsg;
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

  public String getArgs() {
    return args;
  }

  public void setArgs(String args) {
    this.args = args;
  }

}
