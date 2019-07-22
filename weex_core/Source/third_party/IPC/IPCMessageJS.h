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
/*
 * Implementation of the user-space ashmem API for devices, which have our
 * ashmem-enabled kernel. See ashmem-sim.c for the "fake" tmp-based version,
 * used by the simulator.
 */

#ifndef IPCMESSAGEJS_H
#define IPCMESSAGEJS_H

// Message from Platform to Script in ScriptBridge
enum class IPCJSMsg {
    INITFRAMEWORK,
    EXECJSSERVICE,
    TAKEHEAPSNAPSHOT,
    EXECJS,
	CREATEINSTANCE,
    DESTORYINSTANCE,
    EXECJSONINSTANCE,
    EXECJSWITHRESULT,
    EXECJSWITHCALLBACK,
    UPDATEGLOBALCONFIG,
	UpdateInitFrameworkParams,
    EXECTIMERCALLBACK,
	INITAPPFRAMEWORK,
    CREATEAPPCONTEXT,
    EXECJSONAPPWITHRESULT,
    CALLJSONAPPCONTEXT,
    DESTORYAPPCONTEXT,
    SETLOGLEVEL,
};

// Message from Script to Core in ScriptBridge
enum class IPCProxyMsg {
    SETJSFVERSION,
    REPORTEXCEPTION,
    CALLNATIVE,
    CALLNATIVEMODULE,
    CALLNATIVECOMPONENT,
    CALLADDELEMENT,
    SETTIMEOUT,
    NATIVELOG,
    CALLCREATEBODY,
    CALLUPDATEFINISH,
    CALLCREATEFINISH,
    CALLREFRESHFINISH,
    CALLUPDATEATTRS,
    CALLUPDATESTYLE,
    CALLREMOVEELEMENT,
    CALLMOVEELEMENT,
    CALLADDEVENT,
    CALLREMOVEEVENT,
    CALLGCANVASLINK,
	CALLT3DLINK,
    SETINTERVAL,
    CLEARINTERVAL,
	POSTMESSAGE,
  	DISPATCHMESSAGE,
    DISPATCHMESSAGESYNC,
  	ONRECEIVEDRESULT,
    UPDATECOMPONENTDATA,
    HEARTBEAT,
    POSTLOGDETAIL,
};
// Message from Script to Core in ScriptBridge

// Message from Core to Platform in PlatformBridge
enum class IPCMsgFromCoreToPlatform {
	INVOKE_MEASURE_FUNCTION = 100,
	INVOKE_LAYOUT_BEFORE,
	INVOKE_LAYOUT_AFTER,
	SET_JS_VERSION,
	REPORT_EXCEPTION,
	CALL_NATIVE,
	CALL_NATIVE_MODULE,
	CALL_NATIVE_COMPONENT,
	SET_TIMEOUT,
	NATIVE_LOG,
	UPDATE_FINISH,
	REFRESH_FINISH,
	ADD_EVENT,
	REMOVE_EVENT,
	CREATE_BODY,
	ADD_ELEMENT,
	LAYOUT,
	UPDATE_STYLE,
	UPDATE_ATTR,
	CREATE_FINISH,
	RENDER_SUCCESS,
	REMOVE_ELEMENT,
	MOVE_ELEMENT,
	APPEND_TREE_CREATE_FINISH,
	HAS_TRANSITION_PROS,
	POST_MESSAGE,
	DISPATCH_MESSAGE
};

// Message from Platform to Core in PlatformBridge
enum class IPCMsgFromPlatformToCore {
	SET_DEFAULT_HEIGHT_AND_WIDTH_INTO_ROOT_DOM = 100,
	ON_INSTANCE_CLOSE,
	SET_STYLE_WIDTH,
	SET_STYLE_HEIGHT,
	SET_MARGIN,
	SET_PADDING,
	SET_POSITION,
	MARK_DIRTY,
	SET_VIEWPORT_WIDTH,
	FORCE_LAYOUT,
	NOTIFY_LAYOUT,
	GET_FIRST_SCREEN_RENDER_TIME,
	GET_RENDER_FINISH_TIME,
	SET_RENDER_CONTAINER_WRAP_CONTENT,
	BIND_MEASUREMENT_TO_RENDER_OBJECT,
	REGISTER_CORE_ENV,
	GET_RENDER_OBJECT,
	UPDATE_RENDER_OBJECT_STYLE,
	UPDATE_RENDER_OBJECT_ATTR,
	COPY_RENDER_OBJECT,
	SET_MEASURE_FUNCTION_ADAPTER,
	SET_PLATFORM,
	SET_DEVICE_WIDTH_AND_HEIGHT,
	ADD_OPTION,

	INIT_FRAMEWORK,
	INIT_APP_FRAMEWORK,
	CREATE_APP_CONTEXT,
	EXEC_JS_ON_APP_WITH_RESULT,
	CALL_JS_ON_APP_CONTEXT,
	DESTROY_APP_CONTEXT,
	EXEC_JS_SERVICE,
	EXEC_TIME_CALLBACK,
	EXEC_JS,
	EXEC_JS_WITH_RESULT,
	CREATE_INSTANCE,
	EXEC_JS_ON_INSTANCE,
	DESTROY_INSTANCE,
	UPDATE_GLOBAL_CONFIG
};
#endif /* IPCMESSAGEJS_H */
