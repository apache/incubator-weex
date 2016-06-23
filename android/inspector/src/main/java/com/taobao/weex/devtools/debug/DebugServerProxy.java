package com.taobao.weex.devtools.debug;

import android.text.TextUtils;
import android.util.Log;
import android.widget.Toast;

import com.squareup.okhttp.ws.WebSocket;
import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.devtools.common.LogRedirector;
import com.taobao.weex.devtools.common.Util;
import com.taobao.weex.devtools.inspector.MessageHandlingException;
import com.taobao.weex.devtools.inspector.MethodDispatcher;
import com.taobao.weex.devtools.inspector.MismatchedResponseException;
import com.taobao.weex.devtools.inspector.jsonrpc.JsonRpcException;
import com.taobao.weex.devtools.inspector.jsonrpc.JsonRpcPeer;
import com.taobao.weex.devtools.inspector.jsonrpc.PendingRequest;
import com.taobao.weex.devtools.inspector.jsonrpc.protocol.JsonRpcError;
import com.taobao.weex.devtools.inspector.jsonrpc.protocol.JsonRpcRequest;
import com.taobao.weex.devtools.inspector.jsonrpc.protocol.JsonRpcResponse;
import com.taobao.weex.devtools.inspector.protocol.ChromeDevtoolsDomain;
import com.taobao.weex.devtools.json.ObjectMapper;
import com.taobao.weex.utils.WXLogUtils;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.IOException;

import okio.BufferedSource;

public class DebugServerProxy {
    private static final String TAG = "DebugServerProxy";

    private DebugSocketClient mWebSocketClient;
    private ObjectMapper mObjectMapper = new ObjectMapper();
    private MethodDispatcher mMethodDispatcher;
    private Iterable<ChromeDevtoolsDomain> mDomainModules;
    private JsonRpcPeer mPeer = null;
    private String mRemoteUrl = WXEnvironment.sDebugWsUrl;

    public DebugServerProxy(Iterable<ChromeDevtoolsDomain> domainModules) {
        mWebSocketClient = new DebugSocketClient(this);
        mDomainModules = domainModules;
    }

    public void start() {
        mWebSocketClient.connect(mRemoteUrl,
                new DebugSocketClient.Callback() {

                    @Override
                    public void onSuccess(String response) {
                        mMethodDispatcher = new MethodDispatcher(mObjectMapper, mDomainModules);

                        mPeer = new JsonRpcPeer(mObjectMapper, mWebSocketClient);
                        WXSDKManager.getInstance().postOnUiThread(
                                new Runnable() {

                                    @Override
                                    public void run() {
                                        Toast.makeText(
                                                WXEnvironment.sApplication,
                                                "Inspector Start", Toast.LENGTH_SHORT)
                                                .show();
                                    }
                                }, 0);
                        WXLogUtils.d("connect debugger server success!");
                    }

                    @Override
                    public void onFailure(Throwable cause) {
                        WXLogUtils.d("connect debugger server failure!! " + cause.toString());
                    }

                });
    }


    public void handleMessage(BufferedSource payload, WebSocket.PayloadType type) throws IOException {
        if (type != WebSocket.PayloadType.TEXT) {
            WXLogUtils.w("Websocket received unexpected message with payload of type " + type);
            return;
        }
        try {
            Util.throwIfNull(mPeer);
            try {
                String message = payload.readUtf8();
                if (!TextUtils.isEmpty(message)) {
                    handleRemoteMessage(mPeer, message);
                }
                Log.v(TAG, "onMessage " + message);
            } catch (Exception e) {

            } finally {
                payload.close();
            }

        } catch (IOException e) {
            if (LogRedirector.isLoggable(TAG, Log.VERBOSE)) {
                LogRedirector.v(TAG, "Unexpected I/O exception processing message: " + e);
            }
        }
    }

    private void handleRemoteMessage(JsonRpcPeer peer, String message)
            throws IOException, MessageHandlingException, JSONException {
        // Parse as a generic JSONObject first since we don't know if this is a request or response.
        Log.v(TAG, "handleRemoteMessage : " + message);
        JSONObject messageNode = new JSONObject(message);
        if (messageNode.has("method")) {
            handleRemoteRequest(peer, messageNode);
        } else if (messageNode.has("result")) {
            handleRemoteResponse(peer, messageNode);
        } else {
            throw new MessageHandlingException("Improper JSON-RPC message: " + message);
        }
    }

    private void handleRemoteRequest(JsonRpcPeer peer, JSONObject requestNode)
            throws MessageHandlingException {
        JsonRpcRequest request;
        request = mObjectMapper.convertValue(
                requestNode,
                JsonRpcRequest.class);

        JSONObject result = null;
        JSONObject error = null;
        try {
            result = mMethodDispatcher.dispatch(peer,
                    request.method,
                    request.params);
        } catch (JsonRpcException e) {
            logDispatchException(e);
            error = mObjectMapper.convertValue(e.getErrorMessage(), JSONObject.class);
        }
        if (request.id != null) {
            JsonRpcResponse response = new JsonRpcResponse();
            response.id = request.id;
            response.result = result;
            response.error = error;
            JSONObject jsonObject = mObjectMapper.convertValue(response, JSONObject.class);
            String responseString;
            try {
                responseString = jsonObject.toString();
            } catch (OutOfMemoryError e) {
                // JSONStringer can cause an OOM when the Json to handle is too big.
                response.result = null;
                response.error = mObjectMapper.convertValue(e.getMessage(), JSONObject.class);
                jsonObject = mObjectMapper.convertValue(response, JSONObject.class);
                responseString = jsonObject.toString();
            }
            Log.v(TAG, "handleRemoteRequest : " + responseString);
            peer.getWebSocket().sendText(responseString);
        }
    }

    private void handleRemoteResponse(JsonRpcPeer peer, JSONObject responseNode)
            throws MismatchedResponseException {
        JsonRpcResponse response = mObjectMapper.convertValue(
                responseNode,
                JsonRpcResponse.class);
        PendingRequest pendingRequest = peer.getAndRemovePendingRequest(response.id);
        if (pendingRequest == null) {
            throw new MismatchedResponseException(response.id);
        }
        if (pendingRequest.callback != null) {
            pendingRequest.callback.onResponse(peer, response);
        }
        Log.v(TAG, "handleRemoteResponse : " + responseNode.toString());
    }

    private static void logDispatchException(JsonRpcException e) {
        JsonRpcError errorMessage = e.getErrorMessage();
        switch (errorMessage.code) {
            case METHOD_NOT_FOUND:
                LogRedirector.d(TAG, "Method not implemented: " + errorMessage.message);
                break;
            default:
                LogRedirector.w(TAG, "Error processing remote message", e);
        }
    }
}
