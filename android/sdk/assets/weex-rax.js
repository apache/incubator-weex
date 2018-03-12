/* Prepare Rax Environment APIs 0.4.20, Build 2018-02-26 19:32. */

var global = this; var process = {env:{}};
(function (global, factory) {
	typeof exports === 'object' && typeof module !== 'undefined' ? factory() :
	typeof define === 'function' && define.amd ? define(factory) :
	(factory());
}(this, (function () { 'use strict';

function unwrapExports (x) {
	return x && x.__esModule && Object.prototype.hasOwnProperty.call(x, 'default') ? x['default'] : x;
}

function createCommonjsModule(fn, module) {
	return module = { exports: {} }, fn(module, module.exports), module.exports;
}

var api = createCommonjsModule(function (module) {
module.exports = /******/ (function(modules) { // webpackBootstrap
/******/  // The module cache
/******/  var installedModules = {};
/******/
/******/  // The require function
/******/  function __webpack_require__(moduleId) {
/******/
/******/    // Check if module is in cache
/******/    if(installedModules[moduleId]) {
/******/      return installedModules[moduleId].exports;
/******/    }
/******/    // Create a new module (and put it into the cache)
/******/    var module = installedModules[moduleId] = {
/******/      i: moduleId,
/******/      l: false,
/******/      exports: {}
/******/    };
/******/
/******/    // Execute the module function
/******/    modules[moduleId].call(module.exports, module, module.exports, __webpack_require__);
/******/
/******/    // Flag the module as loaded
/******/    module.l = true;
/******/
/******/    // Return the exports of the module
/******/    return module.exports;
/******/  }
/******/
/******/
/******/  // expose the modules object (__webpack_modules__)
/******/  __webpack_require__.m = modules;
/******/
/******/  // expose the module cache
/******/  __webpack_require__.c = installedModules;
/******/
/******/  // define getter function for harmony exports
/******/  __webpack_require__.d = function(exports, name, getter) {
/******/    if(!__webpack_require__.o(exports, name)) {
/******/      Object.defineProperty(exports, name, {
/******/        configurable: false,
/******/        enumerable: true,
/******/        get: getter
/******/      });
/******/    }
/******/  };
/******/
/******/  // getDefaultExport function for compatibility with non-harmony modules
/******/  __webpack_require__.n = function(module) {
/******/    var getter = module && module.__esModule ?
/******/      function getDefault() { return module['default']; } :
/******/      function getModuleExports() { return module; };
/******/    __webpack_require__.d(getter, 'a', getter);
/******/    return getter;
/******/  };
/******/
/******/  // Object.prototype.hasOwnProperty.call
/******/  __webpack_require__.o = function(object, property) { return Object.prototype.hasOwnProperty.call(object, property); };
/******/
/******/  // __webpack_public_path__
/******/  __webpack_require__.p = "";
/******/
/******/  // Load entry module and return exports
/******/  return __webpack_require__(__webpack_require__.s = 3);
/******/ })
/************************************************************************/
/******/ ([
/* 0 */
/*!***************************************************************************************************************************!*\
  !*** ./packages/weex-rax-framework-api/node_modules/.npminstall/event-target-shim/2.0.0/event-target-shim/lib/commons.js ***!
  \***************************************************************************************************************************/
/*! dynamic exports provided */
/*! all exports used */
/*! ModuleConcatenation bailout: Module is not an ECMAScript module */
/***/ (function(module, exports, __webpack_require__) {

var createUniqueKey = module.exports.createUniqueKey = (
    typeof Symbol !== "undefined" ? Symbol : //eslint-disable-line no-undef
    /* otherwise */ function createUniqueKey(name) {
        return "[[" + name + "_" + Math.random().toFixed(8).slice(2) + "]]"
    }
);

/**
 * Checks whether the given value is a non-null object or not.
 *
 * @param {any} x - The value to be check.
 * @returns {boolean} `true` if the value is a non-null object.
 * @private
 */
var isObject = module.exports.isObject = function isObject(x) {
    return typeof x === "object" && x !== null
};

/**
 * The key of listeners.
 *
 * @type {symbol|string}
 * @private
 */
module.exports.LISTENERS = createUniqueKey("listeners");

/**
 * A value of kind for listeners which are registered in the capturing phase.
 *
 * @type {number}
 * @private
 */
module.exports.CAPTURE = 1;

/**
 * A value of kind for listeners which are registered in the bubbling phase.
 *
 * @type {number}
 * @private
 */
module.exports.BUBBLE = 2;

/**
 * A value of kind for listeners which are registered as an attribute.
 *
 * @type {number}
 * @private
 */
module.exports.ATTRIBUTE = 3;

/**
 * @typedef object ListenerNode
 * @property {function} listener - A listener function.
 * @property {number} kind - The kind of the listener.
 * @property {ListenerNode|null} next - The next node.
 *      If this node is the last, this is `null`.
 */

/**
 * Creates a node of singly linked list for a list of listeners.
 *
 * @param {function} listener - A listener function.
 * @param {number} kind - The kind of the listener.
 * @param {object} [options] - The option object.
 * @param {boolean} [options.once] - The flag to remove the listener at the first call.
 * @param {boolean} [options.passive] - The flag to ignore `event.preventDefault` method.
 * @returns {ListenerNode} The created listener node.
 */
module.exports.newNode = function newNode(listener, kind, options) {
    var obj = isObject(options);

    return {
        listener: listener,
        kind: kind,
        once: obj && Boolean(options.once),
        passive: obj && Boolean(options.passive),
        next: null,
    }
};


/***/ }),
/* 1 */
/*!********************************************************!*\
  !*** ./packages/weex-rax-framework-api/src/emitter.js ***!
  \********************************************************/
/*! dynamic exports provided */
/*! all exports used */
/*! ModuleConcatenation bailout: Module is not an ECMAScript module */
/***/ (function(module, exports, __webpack_require__) {

Object.defineProperty(exports, "__esModule", {
  value: true
});

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) { descriptor.writable = true; } Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) { defineProperties(Constructor.prototype, protoProps); } if (staticProps) { defineProperties(Constructor, staticProps); } return Constructor; }; }();

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

var EventEmitter = function () {
  function EventEmitter() {
    _classCallCheck(this, EventEmitter);

    this._listeners = {};
  }

  /**
   * Adds a listener function to the specified event.
   * @param {String} type
   * @param {Function} listener
   * @param {Boolean} once
   */


  _createClass(EventEmitter, [{
    key: "_addListener",
    value: function _addListener(type, listener, once) {
      this._listeners[type] = this._listeners[type] || [];
      this._listeners[type].push({ listener: listener, once: once });
      return this;
    }

    /**
     * Adds a listener function to the specified event.
     * @param {String} type
     * @param {Function} listener
     * @return {Object} Current instance of EventEmitter for chaining.
     */

  }, {
    key: "on",
    value: function on(type, listener) {
      return this._addListener(type, listener, false);
    }
  }, {
    key: "once",
    value: function once(type, listener) {
      return this._addListener(type, listener, true);
    }

    /**
     * Removes a listener function to the specified event.
     * @param {String} type
     * @param {Function} listener
     * @return {Object} Current instance of EventEmitter for chaining.
     */

  }, {
    key: "off",
    value: function off(type, listener) {
      // alias
      if (!this._listeners[type]) {
        return this;
      }
      if (!this._listeners[type].length) {
        return this;
      }
      if (!listener) {
        delete this._listeners[type];
        return this;
      }
      this._listeners[type] = this._listeners[type].filter(function (_listener) {
        return !(_listener.listener === listener);
      });
      return this;
    }

    /**
     * Emits an specified event.
     * @param {String} type
     * @param {Object} payload
     * @return {Object} Current instance of EventEmitter for chaining.
     */

  }, {
    key: "emit",
    value: function emit(type, payload) {
      var _this = this;

      if (!this._listeners[type]) {
        return this;
      }
      this._listeners[type].forEach(function (_listener) {
        _listener.listener.apply(_this, [payload]);
        if (_listener.once) {
          _this.removeListener(type, _listener.listener);
        }
      });
      return this;
    }
  }]);

  return EventEmitter;
}();

exports.default = EventEmitter;
module.exports = exports["default"];

/***/ }),
/* 2 */
/*!********************************************************************************************************************************!*\
  !*** ./packages/weex-rax-framework-api/node_modules/.npminstall/event-target-shim/2.0.0/event-target-shim/lib/event-target.js ***!
  \********************************************************************************************************************************/
/*! dynamic exports provided */
/*! all exports used */
/*! ModuleConcatenation bailout: Module is not an ECMAScript module */
/***/ (function(module, exports, __webpack_require__) {

var Commons = __webpack_require__(/*! ./commons */ 0);
var CustomEventTarget = __webpack_require__(/*! ./custom-event-target */ 15);
var EventWrapper = __webpack_require__(/*! ./event-wrapper */ 16);
var isObject = Commons.isObject;
var LISTENERS = Commons.LISTENERS;
var CAPTURE = Commons.CAPTURE;
var BUBBLE = Commons.BUBBLE;
var ATTRIBUTE = Commons.ATTRIBUTE;
var newNode = Commons.newNode;
var defineCustomEventTarget = CustomEventTarget.defineCustomEventTarget;
var createEventWrapper = EventWrapper.createEventWrapper;
var STOP_IMMEDIATE_PROPAGATION_FLAG = EventWrapper.STOP_IMMEDIATE_PROPAGATION_FLAG;
var PASSIVE_LISTENER_FLAG = EventWrapper.PASSIVE_LISTENER_FLAG;

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------

/**
 * A flag which shows there is the native `EventTarget` interface object.
 *
 * @type {boolean}
 * @private
 */
var HAS_EVENTTARGET_INTERFACE = (
    typeof window !== "undefined" &&
    typeof window.EventTarget !== "undefined"
);

//------------------------------------------------------------------------------
// Public Interface
//------------------------------------------------------------------------------

/**
 * An implementation for `EventTarget` interface.
 *
 * @constructor
 * @public
 */
var EventTarget = module.exports = function EventTarget() {
    var arguments$1 = arguments;

    if (this instanceof EventTarget) {
        // this[LISTENERS] is a Map.
        // Its key is event type.
        // Its value is ListenerNode object or null.
        //
        // interface ListenerNode {
        //     var listener: Function
        //     var kind: CAPTURE|BUBBLE|ATTRIBUTE
        //     var next: ListenerNode|null
        // }
        Object.defineProperty(this, LISTENERS, {value: Object.create(null)});
    }
    else if (arguments.length === 1 && Array.isArray(arguments[0])) {
        return defineCustomEventTarget(EventTarget, arguments[0])
    }
    else if (arguments.length > 0) {
        var types = Array(arguments.length);
        for (var i = 0; i < arguments.length; ++i) {
            types[i] = arguments$1[i];
        }

        // To use to extend with attribute listener properties.
        // e.g.
        //     class MyCustomObject extends EventTarget("message", "error") {
        //         //...
        //     }
        return defineCustomEventTarget(EventTarget, types)
    }
    else {
        throw new TypeError("Cannot call a class as a function")
    }
};

EventTarget.prototype = Object.create(
    (HAS_EVENTTARGET_INTERFACE ? window.EventTarget : Object).prototype,
    {
        constructor: {
            value: EventTarget,
            writable: true,
            configurable: true,
        },

        addEventListener: {
            value: function addEventListener(type, listener, options) {
                if (listener == null) {
                    return false
                }
                if (typeof listener !== "function" && typeof listener !== "object") {
                    throw new TypeError("\"listener\" is not an object.")
                }

                var capture = isObject(options) ? Boolean(options.capture) : Boolean(options);
                var kind = (capture ? CAPTURE : BUBBLE);
                var node = this[LISTENERS][type];
                if (node == null) {
                    this[LISTENERS][type] = newNode(listener, kind, options);
                    return true
                }

                var prev = null;
                while (node != null) {
                    if (node.listener === listener && node.kind === kind) {
                        // Should ignore a duplicated listener.
                        return false
                    }
                    prev = node;
                    node = node.next;
                }

                prev.next = newNode(listener, kind, options);
                return true
            },
            configurable: true,
            writable: true,
        },

        removeEventListener: {
            value: function removeEventListener(type, listener, options) {
                var this$1 = this;

                if (listener == null) {
                    return false
                }

                var capture = isObject(options) ? Boolean(options.capture) : Boolean(options);
                var kind = (capture ? CAPTURE : BUBBLE);
                var prev = null;
                var node = this[LISTENERS][type];
                while (node != null) {
                    if (node.listener === listener && node.kind === kind) {
                        if (prev == null) {
                            this$1[LISTENERS][type] = node.next;
                        }
                        else {
                            prev.next = node.next;
                        }
                        return true
                    }

                    prev = node;
                    node = node.next;
                }

                return false
            },
            configurable: true,
            writable: true,
        },

        dispatchEvent: {
            value: function dispatchEvent(event) {
                var this$1 = this;

                // If listeners aren't registered, terminate.
                var type = event.type;
                var node = this[LISTENERS][type];
                if (node == null) {
                    return true
                }

                // Since we cannot rewrite several properties, so wrap object.
                var wrapped = createEventWrapper(event, this);

                // This doesn't process capturing phase and bubbling phase.
                // This isn't participating in a tree.
                var prev = null;
                while (node != null) {
                    // Remove this listener if it's once
                    if (node.once) {
                        if (prev == null) {
                            this$1[LISTENERS][type] = node.next;
                        }
                        else {
                            prev.next = node.next;
                        }
                    }
                    else {
                        prev = node;
                    }

                    // Call this listener
                    wrapped[PASSIVE_LISTENER_FLAG] = node.passive;
                    if (typeof node.listener === "function") {
                        node.listener.call(this$1, wrapped);
                    }
                    else if (node.kind !== ATTRIBUTE && typeof node.listener.handleEvent === "function") {
                        node.listener.handleEvent(wrapped);
                    }

                    // Break if `event.stopImmediatePropagation` was called.
                    if (wrapped[STOP_IMMEDIATE_PROPAGATION_FLAG]) {
                        break
                    }

                    node = node.next;
                }

                return !wrapped.defaultPrevented
            },
            configurable: true,
            writable: true,
        },
    }
);


/***/ }),
/* 3 */
/*!******************************************************!*\
  !*** ./packages/weex-rax-framework-api/src/index.js ***!
  \******************************************************/
/*! dynamic exports provided */
/*! all exports used */
/*! ModuleConcatenation bailout: Module is not an ECMAScript module */
/***/ (function(module, exports, __webpack_require__) {

Object.defineProperty(exports, "__esModule", {
  value: true
});

var _typeof = typeof Symbol === "function" && typeof Symbol.iterator === "symbol" ? function (obj) { return typeof obj; } : function (obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; };

exports.injectContext = injectContext;
exports.resetInstanceContext = resetInstanceContext;

var _builtin = __webpack_require__(/*! ./builtin */ 4);

var _emitter = __webpack_require__(/*! ./emitter */ 1);

var _emitter2 = _interopRequireDefault(_emitter);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

var MODULE_NAME_PREFIX = '@weex-module/';
var MODAL_MODULE = MODULE_NAME_PREFIX + 'modal';
var NAVIGATOR_MODULE = MODULE_NAME_PREFIX + 'navigator';
var GLOBAL_EVENT_MODULE = MODULE_NAME_PREFIX + 'globalEvent';
// Instance hub
// const instances = {};
// Bundles hub
var noop = function noop() {};
var weex = {};

function genBuiltinModules(modules, moduleFactories, context) {
  for (var moduleName in moduleFactories) {
    modules[moduleName] = {
      factory: moduleFactories[moduleName].bind(context),
      module: { exports: {} },
      isInitialized: false
    };
  }
  return modules;
}

function injectContext() {
  var instanceContext = new Function('return this')();
  var window = resetInstanceContext(instanceContext);
  for (var key in window) {
    if (typeof instanceContext[key] === 'undefined' && key != '__weex_data__') {
      instanceContext[key] = window[key];
    }
  }
}

/**
 * create a Weex instance
 *
 * @param  {string} instanceId
 * @param  {string} __weex_code__
 * @param  {object} [__weex_options__] {bundleUrl, debug}
 */
// export function createInstance(instanceId, __weex_code__, __weex_options__, __weex_data__, __weex_config__) {
function resetInstanceContext(instanceContext) {
  var instanceId = instanceContext.instanceId,
      document = instanceContext.document,
      bundleUrl = instanceContext.bundleUrl,
      __weex_document__ = instanceContext.__weex_document__,
      __weex_options__ = instanceContext.__weex_options__,
      __weex_data__ = instanceContext.__weex_data__,
      __weex_config__ = instanceContext.__weex_config__;


  weex = __weex_options__.weex;

  // Mark start time
  var responseEnd = Date.now();
  var __weex_env__ = (typeof WXEnvironment === 'undefined' ? 'undefined' : _typeof(WXEnvironment)) === 'object' && WXEnvironment || {};
  // For better performance use built-in promise first
  var shared = __webpack_require__(/*! runtime-shared/dist/shared.function */ 6)();

  var Promise = typeof Promise === 'function' ? Promise : shared.Promise;
  var _Symbol = typeof _Symbol === 'function' ? _Symbol : shared.Symbol;
  var Set = typeof Set === 'function' ? Set : shared.Set;
  var Map = typeof Map === 'function' ? Map : shared.Map;
  var WeakMap = typeof WeakMap === 'function' ? WeakMap : shared.WeakMap;
  var WeakSet = typeof WeakSet === 'function' ? WeakSet : shared.WeakSet;
  var URL = shared.URL,
      URLSearchParams = shared.URLSearchParams,
      FontFace = shared.FontFace,
      matchMedia = shared.matchMedia;
  // let bundleUrl = __weex_options__.bundleUrl || 'about:blank';

  if (!bundleUrl) {
    console.error('Error: Must have bundleUrl option when createInstance, downgrade to "about:blank".');
  } else if (!bundleUrl.split('//')[0]) {
    bundleUrl = 'https:' + bundleUrl;
  }

  var documentURL = new URL(bundleUrl);
  var modules = {};

  // Generate native modules map at instance init
  // genNativeModules(modules, document);
  var __weex_define__ = __webpack_require__(/*! ./define.weex */ 7)(modules);
  var __weex_require__ = __webpack_require__(/*! ./require.weex */ 8)(modules, weex);
  var __weex_downgrade__ = __webpack_require__(/*! ./downgrade.weex */ 9)(__weex_require__);
  // Extend document
  __webpack_require__(/*! ./document.weex */ 11)(__weex_require__, document);

  var location = __webpack_require__(/*! ./location.weex */ 12)(__weex_require__, documentURL);

  var _require = __webpack_require__(/*! ./fetch.weex */ 13)(__weex_require__, Promise),
      fetch = _require.fetch,
      Headers = _require.Headers,
      Request = _require.Request,
      Response = _require.Response;

  var XMLHttpRequest = __webpack_require__(/*! ./xmlhttprequest.weex */ 14)(__weex_require__);
  var WebSocket = __webpack_require__(/*! ./websocket.weex */ 17)(__weex_require__);

  var _require2 = __webpack_require__(/*! ./timer.weex */ 18)(__weex_require__, document),
      setTimeout = _require2.setTimeout,
      clearTimeout = _require2.clearTimeout,
      setInterval = _require2.setInterval,
      clearInterval = _require2.clearInterval,
      requestAnimationFrame = _require2.requestAnimationFrame,
      cancelAnimationFrame = _require2.cancelAnimationFrame;

  var _require3 = __webpack_require__(/*! ./base64.weex */ 19)(),
      atob = _require3.atob,
      btoa = _require3.btoa;

  var performance = __webpack_require__(/*! ./performance.weex */ 20)(responseEnd);

  var _require4 = __webpack_require__(/*! ./event.weex */ 21)(),
      Event = _require4.Event,
      CustomEvent = _require4.CustomEvent;

  var windowEmitter = new _emitter2.default();

  var errorHandler = null;
  function registerErrorHandler() {
    if (registerErrorHandler.once) { return; }

    var globalEvent = __weex_require__(GLOBAL_EVENT_MODULE);
    globalEvent.addEventListener('exception', function (e) {
      // TODO: miss lineno and colno
      // window.onerror = function(messageOrEvent, source, lineno, colno, error) { ... }
      errorHandler(e.exception, e.bundleUrl, 0, 0, new Error(e.exception, e.bundleUrl, 0));
    });

    registerErrorHandler.once = true;
  }

  var window = {
    // ES
    Promise: Promise,
    Symbol: _Symbol,
    Map: Map,
    Set: Set,
    WeakMap: WeakMap,
    WeakSet: WeakSet,
    // W3C: https://www.w3.org/TR/html5/browsers.html#browsing-context-name
    name: '',
    // This read-only property indicates whether the referenced window is closed or not.
    closed: false,
    atob: atob,
    btoa: btoa,
    performance: performance,
    // W3C
    document: document,
    location: location,
    // https://www.w3.org/TR/2009/WD-html5-20090423/browsers.html#dom-navigator
    navigator: {
      product: 'Weex',
      platform: __weex_env__.platform,
      appName: __weex_env__.appName,
      appVersion: __weex_env__.appVersion,
      // Weex/0.12 iOS/9.3 (iPhone7,2) AppName/0.12
      userAgent: 'Weex/' + __weex_env__.weexVersion + ' ' + __weex_env__.platform + '/' + __weex_env__.osVersion + ' (' + __weex_env__.deviceModel + ') ' + __weex_env__.appName + '/' + __weex_env__.appVersion
    },
    // https://drafts.csswg.org/cssom-view/#the-screen-interface
    screen: {
      width: __weex_env__.deviceWidth,
      height: __weex_env__.deviceHeight,
      availWidth: __weex_env__.deviceWidth,
      availHeight: __weex_env__.deviceHeight,
      colorDepth: 24,
      pixelDepth: 24
    },
    devicePixelRatio: __weex_env__.scale,
    fetch: fetch,
    Headers: Headers,
    Response: Response,
    Request: Request,
    XMLHttpRequest: XMLHttpRequest,
    URL: URL,
    URLSearchParams: URLSearchParams,
    FontFace: FontFace,
    WebSocket: WebSocket,
    Event: Event,
    CustomEvent: CustomEvent,
    matchMedia: matchMedia,
    setTimeout: setTimeout,
    clearTimeout: clearTimeout,
    setInterval: setInterval,
    clearInterval: clearInterval,
    requestAnimationFrame: requestAnimationFrame,
    cancelAnimationFrame: cancelAnimationFrame,
    setImmediate: typeof setImmediate === 'function' && setImmediate || function (fn) {
      setTimeout(fn, 0);
    },
    clearImmediate: typeof setImmediate === 'function' && setImmediate || function (id) {
      clearTimeout(id);
    },
    frameworkVersion: null,
    alert: function alert(message) {
      var modal = __weex_require__(MODAL_MODULE);
      modal.alert({
        message: message
      }, function () {});
    },
    open: function open(url) {
      var weexNavigator = __weex_require__(NAVIGATOR_MODULE);
      weexNavigator.push({
        url: url,
        animated: true
      }, noop);
    },
    close: function close() {
      var weexNavigator = __weex_require__(NAVIGATOR_MODULE);
      weexNavigator.close({
        animated: true
      }, noop, noop);
    },
    postMessage: function postMessage(message, targetOrigin) {
      var event = {
        origin: location.origin,
        data: JSON.parse(JSON.stringify(message)),
        type: 'message',
        source: window // FIXME: maybe not export window
      };

      if (typeof BroadcastChannel === 'function') {
        if (targetOrigin == '*') {
          var stack = new BroadcastChannel('message');
          stack.postMessage(event);
        } else {
          var stack = new BroadcastChannel('message' + targetOrigin);
          stack.postMessage(event);
        }
      }
    },
    addEventListener: function addEventListener(type, listener) {
      if (type === 'message') {
        if (typeof BroadcastChannel === 'function') {
          var stack = new BroadcastChannel('message');
          var thisStack = new BroadcastChannel('message' + bundleUrl);
          stack.onmessage = function (e) {
            listener(e.data);
          };
          thisStack.onmessage = function (e) {
            listener(e.data);
          };
        }
      } else {
        windowEmitter.on(type, listener);
      }
    },
    removeEventListener: function removeEventListener(type, listener) {
      windowEmitter.off(type, listener);
    },
    dispatchEvent: function dispatchEvent(e) {
      windowEmitter.emit(e.type, e);
    },
    set onerror(handler) {
      if (typeof handler == 'function') {
        errorHandler = handler;
        registerErrorHandler();
      } else {
        errorHandler = null;
      }
    },
    get onerror() {
      return errorHandler;
    },
    // ModuleJS
    define: __weex_define__,
    require: __weex_require__,
    // Weex
    callNative: function callNative() {},
    __weex_document__: __weex_document__,
    __weex_module_supports__: weex.isRegisteredModule,
    __weex_tag_supports__: weex.isRegisteredComponent,
    __weex_define__: __weex_define__,
    __weex_require__: __weex_require__,
    __weex_downgrade__: __weex_downgrade__,
    __weex_env__: __weex_env__,
    __weex_code__: '',
    __weex_options__: __weex_options__,
    __weex_data__: __weex_data__,
    __weex_config__: __weex_config__
  };

  genBuiltinModules(modules, _builtin.ModuleFactories, window);

  window.self = window.window = window;

  console.log('Rax jsfm init window', typeof window === 'undefined' ? 'undefined' : _typeof(window));
  return window;
}

/***/ }),
/* 4 */
/*!********************************************************!*\
  !*** ./packages/weex-rax-framework-api/src/builtin.js ***!
  \********************************************************/
/*! dynamic exports provided */
/*! all exports used */
/*! ModuleConcatenation bailout: Module is not an ECMAScript module */
/***/ (function(module, exports, __webpack_require__) {

Object.defineProperty(exports, "__esModule", {
  value: true
});
var ModuleFactories = exports.ModuleFactories = {
  'rax': __webpack_require__(/*! rax/dist/rax.factory */ 5)
};

/***/ }),
/* 5 */
/*!******************************************!*\
  !*** ./packages/rax/dist/rax.factory.js ***!
  \******************************************/
/*! dynamic exports provided */
/*! all exports used */
/*! ModuleConcatenation bailout: Module is not an ECMAScript module */
/***/ (function(module, exports) {

module.exports = function(require, exports, module) {
  var __weex_document__ = this["__weex_document__"];
var document = this["document"];
  module.exports = /******/ (function(modules) { // webpackBootstrap
/******/  // The module cache
/******/  var installedModules = {};
/******/
/******/  // The require function
/******/  function __webpack_require__(moduleId) {
/******/
/******/    // Check if module is in cache
/******/    if(installedModules[moduleId]) {
/******/      return installedModules[moduleId].exports;
/******/    }
/******/    // Create a new module (and put it into the cache)
/******/    var module = installedModules[moduleId] = {
/******/      i: moduleId,
/******/      l: false,
/******/      exports: {}
/******/    };
/******/
/******/    // Execute the module function
/******/    modules[moduleId].call(module.exports, module, module.exports, __webpack_require__);
/******/
/******/    // Flag the module as loaded
/******/    module.l = true;
/******/
/******/    // Return the exports of the module
/******/    return module.exports;
/******/  }
/******/
/******/
/******/  // expose the modules object (__webpack_modules__)
/******/  __webpack_require__.m = modules;
/******/
/******/  // expose the module cache
/******/  __webpack_require__.c = installedModules;
/******/
/******/  // define getter function for harmony exports
/******/  __webpack_require__.d = function(exports, name, getter) {
/******/    if(!__webpack_require__.o(exports, name)) {
/******/      Object.defineProperty(exports, name, {
/******/        configurable: false,
/******/        enumerable: true,
/******/        get: getter
/******/      });
/******/    }
/******/  };
/******/
/******/  // getDefaultExport function for compatibility with non-harmony modules
/******/  __webpack_require__.n = function(module) {
/******/    var getter = module && module.__esModule ?
/******/      function getDefault() { return module['default']; } :
/******/      function getModuleExports() { return module; };
/******/    __webpack_require__.d(getter, 'a', getter);
/******/    return getter;
/******/  };
/******/
/******/  // Object.prototype.hasOwnProperty.call
/******/  __webpack_require__.o = function(object, property) { return Object.prototype.hasOwnProperty.call(object, property); };
/******/
/******/  // __webpack_public_path__
/******/  __webpack_require__.p = "";
/******/
/******/  // Load entry module and return exports
/******/  return __webpack_require__(__webpack_require__.s = 17);
/******/ })
/************************************************************************/
/******/ ([
/* 0 */
/***/ (function(module, exports, __webpack_require__) {

Object.defineProperty(exports, "__esModule", {
  value: true
});
/*
 * Stateful things in runtime
 */
exports.default = {
  component: null,
  mountID: 1,
  sandbox: true,
  // Roots
  rootComponents: {},
  rootInstances: {},
  // Inject
  hook: null,
  driver: null,
  monitor: null
};
module.exports = exports["default"];

/***/ }),
/* 1 */
/***/ (function(module, exports, __webpack_require__) {

Object.defineProperty(exports, "__esModule", {
  value: true
});

var _host = __webpack_require__(0);

var _host2 = _interopRequireDefault(_host);

var _element = __webpack_require__(6);

var _unmountComponentAtNode = __webpack_require__(9);

var _unmountComponentAtNode2 = _interopRequireDefault(_unmountComponentAtNode);

var _instantiateComponent = __webpack_require__(2);

var _instantiateComponent2 = _interopRequireDefault(_instantiateComponent);

var _shouldUpdateComponent = __webpack_require__(3);

var _shouldUpdateComponent2 = _interopRequireDefault(_shouldUpdateComponent);

var _root = __webpack_require__(19);

var _root2 = _interopRequireDefault(_root);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

/**
 * Instance manager
 */
var KEY = '$$instance';

exports.default = {
  set: function set(node, instance) {
    if (!node[KEY]) {
      node[KEY] = instance;
      // Record root instance to roots map
      if (instance.rootID) {
        _host2.default.rootInstances[instance.rootID] = instance;
        _host2.default.rootComponents[instance.rootID] = instance._internal;
      }
    }
  },
  get: function get(node) {
    return node[KEY];
  },
  remove: function remove(node) {
    var instance = this.get(node);
    if (instance) {
      node[KEY] = null;
      if (instance.rootID) {
        delete _host2.default.rootComponents[instance.rootID];
        delete _host2.default.rootInstances[instance.rootID];
      }
    }
  },
  mount: function mount(element, container, parentInstance) {
    _host2.default.driver.beforeRender && _host2.default.driver.beforeRender();

    // Real native root node is body
    if (container == null) {
      container = _host2.default.driver.createBody();
    }

    // Get the context from the conceptual parent component.
    var parentContext = void 0;
    if (parentInstance) {
      var parentInternal = parentInstance._internal;
      parentContext = parentInternal._processChildContext(parentInternal._context);
    }

    var prevRootInstance = this.get(container);
    var hasPrevRootInstance = prevRootInstance && prevRootInstance.isRootComponent;

    if (hasPrevRootInstance) {
      var prevRenderedComponent = prevRootInstance.getRenderedComponent();
      var prevElement = prevRenderedComponent._currentElement;
      if ((0, _shouldUpdateComponent2.default)(prevElement, element)) {
        var prevUnmaskedContext = prevRenderedComponent._context;
        prevRenderedComponent.updateComponent(prevElement, element, prevUnmaskedContext, parentContext || prevUnmaskedContext);

        return prevRootInstance;
      } else {
        _host2.default.hook.Reconciler.unmountComponent(prevRootInstance);
        (0, _unmountComponentAtNode2.default)(container);
      }
    }

    var wrappedElement = (0, _element.createElement)(_root2.default, null, element);
    var renderedComponent = (0, _instantiateComponent2.default)(wrappedElement);
    var defaultContext = parentContext || {};
    var rootInstance = renderedComponent.mountComponent(container, null, defaultContext);
    this.set(container, rootInstance);

    // After render callback
    _host2.default.driver.afterRender && _host2.default.driver.afterRender(rootInstance);

    // Devtool render new root hook
    _host2.default.hook.Mount._renderNewRootComponent(rootInstance._internal);

    return rootInstance;
  }
};
module.exports = exports['default'];

/***/ }),
/* 2 */
/***/ (function(module, exports, __webpack_require__) {

Object.defineProperty(exports, "__esModule", {
  value: true
});

var _typeof = typeof Symbol === "function" && typeof Symbol.iterator === "symbol" ? function (obj) { return typeof obj; } : function (obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; };

var _host = __webpack_require__(0);

var _host2 = _interopRequireDefault(_host);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function instantiateComponent(element) {
  var instance = void 0;

  if (element === undefined || element === null || element === false || element === true) {
    instance = new _host2.default.EmptyComponent();
  } else if (Array.isArray(element)) {
    instance = new _host2.default.FragmentComponent(element);
  } else if ((typeof element === 'undefined' ? 'undefined' : _typeof(element)) === 'object' && element.type) {
    // Special case string values
    if (typeof element.type === 'string') {
      instance = new _host2.default.NativeComponent(element);
    } else {
      instance = new _host2.default.CompositeComponent(element);
    }
  } else if (typeof element === 'string' || typeof element === 'number') {
    instance = new _host2.default.TextComponent(element);
  } else {
    throw new Error('Invalid element type: ' + element + '. (keys: ' + Object.keys(element) + ')');
  }

  instance._mountIndex = 0;

  return instance;
}

exports.default = instantiateComponent;
module.exports = exports['default'];

/***/ }),
/* 3 */
/***/ (function(module, exports, __webpack_require__) {

Object.defineProperty(exports, "__esModule", {
  value: true
});

var _typeof = typeof Symbol === "function" && typeof Symbol.iterator === "symbol" ? function (obj) { return typeof obj; } : function (obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; };

function shouldUpdateComponent(prevElement, nextElement) {
  // TODO: prevElement and nextElement could be array
  var prevEmpty = prevElement === null;
  var nextEmpty = nextElement === null;
  if (prevEmpty || nextEmpty) {
    return prevEmpty === nextEmpty;
  }

  var prevType = typeof prevElement === 'undefined' ? 'undefined' : _typeof(prevElement);
  var nextType = typeof nextElement === 'undefined' ? 'undefined' : _typeof(nextElement);
  if (prevType === 'string' || prevType === 'number') {
    return nextType === 'string' || nextType === 'number';
  } else {
    return prevType === 'object' && nextType === 'object' && prevElement.type === nextElement.type && prevElement.key === nextElement.key;
  }
}

exports.default = shouldUpdateComponent;
module.exports = exports['default'];

/***/ }),
/* 4 */
/***/ (function(module, exports, __webpack_require__) {

Object.defineProperty(exports, "__esModule", {
  value: true
});

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) { descriptor.writable = true; } Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) { defineProperties(Constructor.prototype, protoProps); } if (staticProps) { defineProperties(Constructor, staticProps); } return Constructor; }; }();

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

/**
 * Base component class.
 */
var Component = function () {
  function Component(props, context, updater) {
    _classCallCheck(this, Component);

    this.props = props;
    this.context = context;
    this.refs = {};
    this.updater = updater;
  }

  _createClass(Component, [{
    key: "isComponentClass",
    value: function isComponentClass() {}
  }, {
    key: "setState",
    value: function setState(partialState, callback) {
      this.updater.setState(this, partialState, callback);
    }
  }, {
    key: "forceUpdate",
    value: function forceUpdate(callback) {
      this.updater.forceUpdate(this, callback);
    }
  }]);

  return Component;
}();

exports.default = Component;
module.exports = exports["default"];

/***/ }),
/* 5 */
/***/ (function(module, exports, __webpack_require__) {

Object.defineProperty(exports, "__esModule", {
  value: true
});

var _instance = __webpack_require__(1);

var _instance2 = _interopRequireDefault(_instance);

var _host = __webpack_require__(0);

var _host2 = _interopRequireDefault(_host);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

exports.default = {
  ComponentTree: {
    getClosestInstanceFromNode: function getClosestInstanceFromNode(node) {
      return _instance2.default.get(node);
    },
    getNodeFromInstance: function getNodeFromInstance(inst) {
      // inst is an internal instance (but could be a composite)
      while (inst._renderedComponent) {
        inst = inst._renderedComponent;
      }

      if (inst) {
        return inst._nativeNode;
      } else {
        return null;
      }
    }
  },
  Mount: {
    _instancesByReactRootID: _host2.default.rootComponents,

    // Stub - React DevTools expects to find this method and replace it
    // with a wrapper in order to observe new root components being added
    _renderNewRootComponent: function _renderNewRootComponent() {}
  },
  Reconciler: {
    // Stubs - React DevTools expects to find these methods and replace them
    // with wrappers in order to observe components being mounted, updated and
    // unmounted
    mountComponent: function mountComponent() {},
    receiveComponent: function receiveComponent() {},
    unmountComponent: function unmountComponent() {}
  },
  // monitor the info of all components
  monitor: null
};
module.exports = exports['default'];

/***/ }),
/* 6 */
/***/ (function(module, exports, __webpack_require__) {

Object.defineProperty(exports, "__esModule", {
  value: true
});

var _typeof = typeof Symbol === "function" && typeof Symbol.iterator === "symbol" ? function (obj) { return typeof obj; } : function (obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; };

exports.createElement = createElement;
exports.createFactory = createFactory;
exports.cloneElement = cloneElement;
exports.isValidElement = isValidElement;

var _host = __webpack_require__(0);

var _host2 = _interopRequireDefault(_host);

var _flattenChildren = __webpack_require__(7);

var _flattenChildren2 = _interopRequireDefault(_flattenChildren);

var _universalEnv = __webpack_require__(8);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

var RESERVED_PROPS = {
  key: true,
  ref: true
};

function getRenderErrorInfo() {
  if (_host2.default.component) {
    var name = _host2.default.component.getName();
    if (name) {
      return ' Check the render method of `' + name + '`.';
    }
  }
  return '';
}

function Element(type, key, ref, props, owner) {
  if (_universalEnv.isWeex) {
    props = filterProps(type, props);
  }

  return {
    // Built-in properties that belong on the element
    type: type,
    key: key,
    ref: ref,
    props: props,
    // Record the component responsible for creating this element.
    _owner: owner
  };
}

exports.default = Element;


function flattenStyle(style) {
  if (!style) {
    return undefined;
  }

  if (!Array.isArray(style)) {
    return style;
  } else {
    var result = {};
    for (var i = 0; i < style.length; ++i) {
      var computedStyle = flattenStyle(style[i]);
      if (computedStyle) {
        for (var key in computedStyle) {
          result[key] = computedStyle[key];
        }
      }
    }
    return result;
  }
}

// TODO: move to weex-drvier
function filterProps(type, props) {
  // Only for weex text
  if (type === 'text') {
    var children = props.children;
    var value = props.value;

    // Value is first
    if (value == null && children != null) {
      if (Array.isArray(children)) {
        children = children.map(function (val) {
          if (typeof val === 'number' || typeof val === 'string') {
            return val;
          } else {
            return '';
          }
        }).join('');
      } else if (typeof children !== 'number' && typeof children !== 'string') {
        children = '';
      }

      props.value = String(children);
    }

    props.children = null;
  }

  return props;
}

function createElement(type, config, children) {
  var arguments$1 = arguments;

  if (type == null) {
    throw Error('createElement: type should not be null or undefined.' + getRenderErrorInfo());
  }
  // Reserved names are extracted
  var props = {};
  var propName = void 0;
  var key = null;
  var ref = null;

  if (config != null) {
    ref = config.ref === undefined ? null : config.ref;
    key = config.key === undefined ? null : String(config.key);
    // Remaining properties are added to a new props object
    for (propName in config) {
      if (!RESERVED_PROPS[propName]) {
        props[propName] = config[propName];
      }
    }
  }

  var childrenLength = arguments.length - 2;
  if (childrenLength > 0) {
    if (childrenLength === 1 && !Array.isArray(children)) {
      props.children = children;
    } else {
      var childArray = children;
      if (childrenLength > 1) {
        childArray = new Array(childrenLength);
        for (var i = 0; i < childrenLength; i++) {
          childArray[i] = arguments$1[i + 2];
        }
      }
      props.children = (0, _flattenChildren2.default)(childArray);
    }
  }

  // Resolve default props
  if (type && type.defaultProps) {
    var defaultProps = type.defaultProps;
    for (propName in defaultProps) {
      if (props[propName] === undefined) {
        props[propName] = defaultProps[propName];
      }
    }
  }

  if (props.style && (Array.isArray(props.style) || _typeof(props.style) === 'object')) {
    props.style = flattenStyle(props.style);
  }

  return new Element(type, key, ref, props, _host2.default.component);
}

function createFactory(type) {
  var factory = createElement.bind(null, type);
  // Expose the type on the factory and the prototype so that it can be
  // easily accessed on elements. E.g. `<Foo />.type === Foo`.
  // This should not be named `constructor` since this may not be the function
  // that created the element, and it may not even be a constructor.
  factory.type = type;
  return factory;
}

function cloneElement(element, config) {
  var arguments$1 = arguments;

  if (!isValidElement(element)) {
    throw Error('cloneElement: not a valid element.' + getRenderErrorInfo());
  }

  // Original props are copied
  var props = Object.assign({}, element.props);

  // Reserved names are extracted
  var key = element.key;
  var ref = element.ref;

  // Owner will be preserved, unless ref is overridden
  var owner = element._owner;

  if (config) {
    // Should reset ref and owner if has a new ref
    if (config.ref !== undefined) {
      ref = config.ref;
      owner = _host2.default.component;
    }

    if (config.key !== undefined) {
      key = String(config.key);
    }

    // Resolve default props
    var defaultProps = void 0;
    if (element.type && element.type.defaultProps) {
      defaultProps = element.type.defaultProps;
    }
    // Remaining properties override existing props
    var propName = void 0;
    for (propName in config) {
      if (config.hasOwnProperty(propName) && !RESERVED_PROPS.hasOwnProperty(propName)) {
        if (config[propName] === undefined && defaultProps !== undefined) {
          // Resolve default props
          props[propName] = defaultProps[propName];
        } else {
          props[propName] = config[propName];
        }
      }
    }
  }

  for (var _len = arguments.length, children = Array(_len > 2 ? _len - 2 : 0), _key = 2; _key < _len; _key++) {
    children[_key - 2] = arguments$1[_key];
  }

  if (children.length) {
    props.children = (0, _flattenChildren2.default)(children);
  }

  return new Element(element.type, key, ref, props, owner);
}

function isValidElement(object) {
  return (typeof object === 'undefined' ? 'undefined' : _typeof(object)) === 'object' && object !== null && object.type && object.props;
}

/***/ }),
/* 7 */
/***/ (function(module, exports, __webpack_require__) {

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.default = flattenChildren;
function traverseChildren(children, result) {
  if (Array.isArray(children)) {
    for (var i = 0, l = children.length; i < l; i++) {
      traverseChildren(children[i], result);
    }
  } else {
    result.push(children);
  }
}

function flattenChildren(children) {
  if (children == null) {
    return children;
  }
  var result = [];
  traverseChildren(children, result);

  if (result.length === 1) {
    result = result[0];
  }

  return result;
}
module.exports = exports["default"];

/***/ }),
/* 8 */
/***/ (function(module, exports, __webpack_require__) {

var _typeof2 = typeof Symbol === "function" && typeof Symbol.iterator === "symbol" ? function (obj) { return typeof obj; } : function (obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; };

Object.defineProperty(exports, "__esModule", {
  value: true
});

var _typeof = typeof Symbol === "function" && _typeof2(Symbol.iterator) === "symbol" ? function (obj) {
  return typeof obj === "undefined" ? "undefined" : _typeof2(obj);
} : function (obj) {
  return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj === "undefined" ? "undefined" : _typeof2(obj);
};

// https://www.w3.org/TR/html5/webappapis.html#dom-navigator-appcodename
var isWeb = exports.isWeb = (typeof navigator === 'undefined' ? 'undefined' : _typeof(navigator)) === 'object' && (navigator.appCodeName === 'Mozilla' || navigator.product === 'Gecko');
var isNode = exports.isNode = typeof process !== 'undefined' && !!(process.versions && process.versions.node);
var isWeex = exports.isWeex = typeof callNative === 'function';
var isReactNative = exports.isReactNative = typeof __fbBatchedBridgeConfig !== 'undefined';
exports['default'] = module.exports;
exports.default = module.exports;

/***/ }),
/* 9 */
/***/ (function(module, exports, __webpack_require__) {

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.default = unmountComponentAtNode;

var _instance = __webpack_require__(1);

var _instance2 = _interopRequireDefault(_instance);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function unmountComponentAtNode(node) {
  var component = _instance2.default.get(node);

  if (!component) {
    return false;
  }

  _instance2.default.remove(node);
  component._internal.unmountComponent();

  return true;
}
module.exports = exports['default'];

/***/ }),
/* 10 */
/***/ (function(module, exports, __webpack_require__) {

Object.defineProperty(exports, "__esModule", {
  value: true
});

var _inject = __webpack_require__(22);

var _inject2 = _interopRequireDefault(_inject);

var _instance = __webpack_require__(1);

var _instance2 = _interopRequireDefault(_instance);

var _host = __webpack_require__(0);

var _host2 = _interopRequireDefault(_host);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function render(element, container, options, callback) {
  // Compatible with `render(element, container, callback)`
  if (typeof options === 'function') {
    callback = options;
    options = null;
  }

  options = options || {};
  // Init inject
  (0, _inject2.default)(options);

  var rootComponent = _instance2.default.mount(element, container, options.parent);
  var componentInstance = rootComponent.getPublicInstance();

  if (callback) {
    callback.call(componentInstance);
  }

  return componentInstance;
}

exports.default = render;
module.exports = exports['default'];

/***/ }),
/* 11 */
/***/ (function(module, exports, __webpack_require__) {

Object.defineProperty(exports, "__esModule", {
  value: true
});

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) { descriptor.writable = true; } Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) { defineProperties(Constructor.prototype, protoProps); } if (staticProps) { defineProperties(Constructor, staticProps); } return Constructor; }; }();

var _host = __webpack_require__(0);

var _host2 = _interopRequireDefault(_host);

var _ref = __webpack_require__(12);

var _ref2 = _interopRequireDefault(_ref);

var _instantiateComponent = __webpack_require__(2);

var _instantiateComponent2 = _interopRequireDefault(_instantiateComponent);

var _shouldUpdateComponent = __webpack_require__(3);

var _shouldUpdateComponent2 = _interopRequireDefault(_shouldUpdateComponent);

var _getElementKeyName = __webpack_require__(13);

var _getElementKeyName2 = _interopRequireDefault(_getElementKeyName);

var _instance = __webpack_require__(1);

var _instance2 = _interopRequireDefault(_instance);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

var STYLE = 'style';
var CHILDREN = 'children';
var TREE = 'tree';
var EVENT_PREFIX_REGEXP = /on[A-Z]/;

/**
 * Native Component
 */

var NativeComponent = function () {
  function NativeComponent(element) {
    _classCallCheck(this, NativeComponent);

    this._currentElement = element;
  }

  _createClass(NativeComponent, [{
    key: 'mountComponent',
    value: function mountComponent(parent, parentInstance, context, childMounter) {
      // Parent native element
      this._parent = parent;
      this._parentInstance = parentInstance;
      this._context = context;
      this._mountID = _host2.default.mountID++;

      var props = this._currentElement.props;
      var type = this._currentElement.type;
      var instance = {
        _internal: this,
        type: type,
        props: props
      };
      var appendType = props.append; // Default is node

      this._instance = instance;

      // Clone a copy for style diff
      this._prevStyleCopy = Object.assign({}, props.style);

      var nativeNode = this.getNativeNode();

      if (appendType !== TREE) {
        if (childMounter) {
          childMounter(nativeNode, parent);
        } else {
          _host2.default.driver.appendChild(nativeNode, parent);
        }
      }

      if (this._currentElement && this._currentElement.ref) {
        _ref2.default.attach(this._currentElement._owner, this._currentElement.ref, this);
      }

      // Process children
      var children = props.children;
      if (children != null) {
        this.mountChildren(children, context);
      }

      if (appendType === TREE) {
        if (childMounter) {
          childMounter(nativeNode, parent);
        } else {
          _host2.default.driver.appendChild(nativeNode, parent);
        }
      }

      _host2.default.hook.Reconciler.mountComponent(this);

      return instance;
    }
  }, {
    key: 'mountChildren',
    value: function mountChildren(children, context) {
      var _this = this;

      if (!Array.isArray(children)) {
        children = [children];
      }

      var renderedChildren = this._renderedChildren = {};

      var renderedChildrenImage = children.map(function (element, index) {
        var renderedChild = (0, _instantiateComponent2.default)(element);
        var name = (0, _getElementKeyName2.default)(renderedChildren, element, index);
        renderedChildren[name] = renderedChild;
        renderedChild._mountIndex = index;
        // Mount
        var mountImage = renderedChild.mountComponent(_this.getNativeNode(), _this._instance, context, null);
        return mountImage;
      });

      return renderedChildrenImage;
    }
  }, {
    key: 'unmountChildren',
    value: function unmountChildren(notRemoveChild) {
      var renderedChildren = this._renderedChildren;

      if (renderedChildren) {
        for (var name in renderedChildren) {
          var renderedChild = renderedChildren[name];
          renderedChild.unmountComponent(notRemoveChild);
        }
        this._renderedChildren = null;
      }
    }
  }, {
    key: 'unmountComponent',
    value: function unmountComponent(notRemoveChild) {
      if (this._nativeNode) {
        var ref = this._currentElement.ref;
        if (ref) {
          _ref2.default.detach(this._currentElement._owner, ref, this);
        }

        _instance2.default.remove(this._nativeNode);
        if (!notRemoveChild) {
          _host2.default.driver.removeChild(this._nativeNode, this._parent);
        }
        _host2.default.driver.removeAllEventListeners(this._nativeNode);
      }

      this.unmountChildren(notRemoveChild);

      _host2.default.hook.Reconciler.unmountComponent(this);

      this._currentElement = null;
      this._nativeNode = null;
      this._parent = null;
      this._parentInstance = null;
      this._context = null;
      this._instance = null;
      this._prevStyleCopy = null;
    }
  }, {
    key: 'updateComponent',
    value: function updateComponent(prevElement, nextElement, prevContext, nextContext) {
      // Replace current element
      this._currentElement = nextElement;

      _ref2.default.update(prevElement, nextElement, this);

      var prevProps = prevElement.props;
      var nextProps = nextElement.props;

      this.updateProperties(prevProps, nextProps);
      this.updateChildren(nextProps.children, nextContext);

      _host2.default.hook.Reconciler.receiveComponent(this);
    }
  }, {
    key: 'updateProperties',
    value: function updateProperties(prevProps, nextProps) {
      var this$1 = this;

      var propKey = void 0;
      var styleName = void 0;
      var styleUpdates = void 0;
      for (propKey in prevProps) {
        if (propKey === CHILDREN || nextProps.hasOwnProperty(propKey) || !prevProps.hasOwnProperty(propKey) || prevProps[propKey] == null) {
          continue;
        }
        if (propKey === STYLE) {
          var lastStyle = this$1._prevStyleCopy;
          for (styleName in lastStyle) {
            if (lastStyle.hasOwnProperty(styleName)) {
              styleUpdates = styleUpdates || {};
              styleUpdates[styleName] = '';
            }
          }
          this$1._prevStyleCopy = null;
        } else if (EVENT_PREFIX_REGEXP.test(propKey)) {
          if (typeof prevProps[propKey] === 'function') {
            _host2.default.driver.removeEventListener(this$1.getNativeNode(), propKey.slice(2).toLowerCase(), prevProps[propKey]);
          }
        } else {
          _host2.default.driver.removeAttribute(this$1.getNativeNode(), propKey, prevProps[propKey]);
        }
      }

      for (propKey in nextProps) {
        var nextProp = nextProps[propKey];
        var prevProp = propKey === STYLE ? this$1._prevStyleCopy : prevProps != null ? prevProps[propKey] : undefined;
        if (propKey === CHILDREN || !nextProps.hasOwnProperty(propKey) || nextProp === prevProp || nextProp == null && prevProp == null) {
          continue;
        }
        // Update style
        if (propKey === STYLE) {
          if (nextProp) {
            // Clone property
            nextProp = this$1._prevStyleCopy = Object.assign({}, nextProp);
          } else {
            this$1._prevStyleCopy = null;
          }

          if (prevProp != null) {
            // Unset styles on `prevProp` but not on `nextProp`.
            for (styleName in prevProp) {
              if (prevProp.hasOwnProperty(styleName) && (!nextProp || !nextProp.hasOwnProperty(styleName))) {
                styleUpdates = styleUpdates || {};
                styleUpdates[styleName] = '';
              }
            }
            // Update styles that changed since `prevProp`.
            for (styleName in nextProp) {
              if (nextProp.hasOwnProperty(styleName) && prevProp[styleName] !== nextProp[styleName]) {
                styleUpdates = styleUpdates || {};
                styleUpdates[styleName] = nextProp[styleName];
              }
            }
          } else {
            // Assign next prop when prev style is null
            styleUpdates = nextProp;
          }

          // Update event binding
        } else if (EVENT_PREFIX_REGEXP.test(propKey)) {
          var eventName = propKey.slice(2).toLowerCase();

          if (typeof prevProp === 'function') {
            _host2.default.driver.removeEventListener(this$1.getNativeNode(), eventName, prevProp, nextProps);
          }

          if (typeof nextProp === 'function') {
            _host2.default.driver.addEventListener(this$1.getNativeNode(), eventName, nextProp, nextProps);
          }
          // Update other property
        } else {
          if (nextProp != null) {
            _host2.default.driver.setAttribute(this$1.getNativeNode(), propKey, nextProp);
          } else {
            _host2.default.driver.removeAttribute(this$1.getNativeNode(), propKey, prevProps[propKey]);
          }
          
        }
      }

      if (styleUpdates) {
        _host2.default.driver.setStyles(this.getNativeNode(), styleUpdates);
      }
    }
  }, {
    key: 'updateChildren',
    value: function updateChildren(nextChildrenElements, context) {
      var _this2 = this;

      // prev rendered children
      var prevChildren = this._renderedChildren;

      if (nextChildrenElements == null && prevChildren == null) {
        return;
      }

      var nextChildren = {};
      var oldNodes = {};

      if (nextChildrenElements != null) {
        if (!Array.isArray(nextChildrenElements)) {
          nextChildrenElements = [nextChildrenElements];
        }

        // Update next children elements
        for (var index = 0, length = nextChildrenElements.length; index < length; index++) {
          var nextElement = nextChildrenElements[index];
          var name = (0, _getElementKeyName2.default)(nextChildren, nextElement, index);
          var prevChild = prevChildren && prevChildren[name];
          var prevElement = prevChild && prevChild._currentElement;

          if (prevChild != null && (0, _shouldUpdateComponent2.default)(prevElement, nextElement)) {
            // Pass the same context when updating chidren
            prevChild.updateComponent(prevElement, nextElement, context, context);
            nextChildren[name] = prevChild;
          } else {
            // Unmount the prevChild when nextChild is different element type.
            if (prevChild) {
              var oldNativeNode = prevChild.getNativeNode();
              // Delay remove child
              prevChild.unmountComponent(true);
              oldNodes[name] = oldNativeNode;
            }
            // The child must be instantiated before it's mounted.
            nextChildren[name] = (0, _instantiateComponent2.default)(nextElement);
          }
        }
      }

      var firstPrevChild = void 0;
      var delayRemoveFirstPrevChild = void 0;
      // Unmount children that are no longer present.
      if (prevChildren != null) {
        for (var _name in prevChildren) {
          if (!prevChildren.hasOwnProperty(_name)) {
            continue;
          }

          var _prevChild = prevChildren[_name];
          var shouldRemove = !nextChildren[_name];

          // Store old first child ref for append node ahead and maybe delay remove it
          if (!firstPrevChild) {
            firstPrevChild = _prevChild;
            delayRemoveFirstPrevChild = shouldRemove;
          } else if (shouldRemove) {
            _prevChild.unmountComponent();
          }
        }
      }

      if (nextChildren != null) {
        (function () {
          // `nextIndex` will increment for each child in `nextChildren`, but
          // `lastIndex` will be the last index visited in `prevChildren`.
          var lastIndex = 0;
          var nextIndex = 0;
          var lastPlacedNode = null;
          var nextNativeNode = [];

          var _loop = function _loop(_name2) {
            if (!nextChildren.hasOwnProperty(_name2)) {
              return 'continue';
            }

            var nextChild = nextChildren[_name2];
            var prevChild = prevChildren && prevChildren[_name2];

            if (prevChild === nextChild) {
              var prevChildNativeNode = prevChild.getNativeNode();
              // Convert to array type
              if (!Array.isArray(prevChildNativeNode)) {
                prevChildNativeNode = [prevChildNativeNode];
              }

              // If the index of `child` is less than `lastIndex`, then it needs to
              // be moved. Otherwise, we do not need to move it because a child will be
              // inserted or moved before `child`.
              if (prevChild._mountIndex < lastIndex) {
                // Get the last child
                if (Array.isArray(lastPlacedNode)) {
                  lastPlacedNode = lastPlacedNode[lastPlacedNode.length - 1];
                }

                for (var _i = prevChildNativeNode.length - 1; _i >= 0; _i--) {
                  _host2.default.driver.insertAfter(prevChildNativeNode[_i], lastPlacedNode);
                }
              }

              nextNativeNode = nextNativeNode.concat(prevChildNativeNode);

              lastIndex = Math.max(prevChild._mountIndex, lastIndex);
              prevChild._mountIndex = nextIndex;
            } else {
              if (prevChild != null) {
                // Update `lastIndex` before `_mountIndex` gets unset by unmounting.
                lastIndex = Math.max(prevChild._mountIndex, lastIndex);
              }

              var parent = _this2.getNativeNode();
              // Fragment extended native component, so if parent is fragment should get this._parent
              if (Array.isArray(parent)) {
                parent = _this2._parent;
              }

              nextChild.mountComponent(parent, _this2._instance, context, function (newChild, parent) {
                // TODO: Rework the duplicate code
                var oldChild = oldNodes[_name2];
                if (!Array.isArray(newChild)) {
                  newChild = [newChild];
                }

                if (oldChild) {
                  // The oldChild or newChild all maybe fragment
                  if (!Array.isArray(oldChild)) {
                    oldChild = [oldChild];
                  }

                  // If newChild count large then oldChild
                  var lastNewChild = void 0;
                  for (var _i2 = 0; _i2 < newChild.length; _i2++) {
                    var child = newChild[_i2];
                    if (oldChild[_i2]) {
                      _host2.default.driver.replaceChild(child, oldChild[_i2]);
                    } else {
                      _host2.default.driver.insertAfter(child, lastNewChild);
                    }
                    lastNewChild = child;
                  }

                  // If newChild count less then oldChild
                  if (newChild.length < oldChild.length) {
                    for (var _i3 = newChild.length; _i3 < oldChild.length; _i3++) {
                      _host2.default.driver.removeChild(oldChild[_i3]);
                    }
                  }
                } else {
                  // Insert child at a specific index

                  // Get the last child
                  if (Array.isArray(lastPlacedNode)) {
                    lastPlacedNode = lastPlacedNode[lastPlacedNode.length - 1];
                  }

                  var prevFirstNativeNode = void 0;

                  if (firstPrevChild && !lastPlacedNode) {
                    prevFirstNativeNode = firstPrevChild.getNativeNode();
                    if (Array.isArray(prevFirstNativeNode)) {
                      prevFirstNativeNode = prevFirstNativeNode[0];
                    }
                  }

                  for (var _i4 = newChild.length - 1; _i4 >= 0; _i4--) {
                    var _child = newChild[_i4];
                    if (lastPlacedNode) {
                      _host2.default.driver.insertAfter(_child, lastPlacedNode);
                    } else if (prevFirstNativeNode) {
                      _host2.default.driver.insertBefore(_child, prevFirstNativeNode);
                    } else {
                      _host2.default.driver.appendChild(_child, parent);
                    }
                  }
                }

                nextNativeNode = nextNativeNode.concat(newChild);
              });
              nextChild._mountIndex = nextIndex;
            }

            nextIndex++;
            lastPlacedNode = nextChild.getNativeNode();
          };

          for (var _name2 in nextChildren) {
            var _ret2 = _loop(_name2);

            if (_ret2 === 'continue') { continue; }
          }

          // Sync update native refs
          if (Array.isArray(_this2._nativeNode)) {
            // Clear all and push the new array
            _this2._nativeNode.splice(0, _this2._nativeNode.length);
            for (var i = 0; i < nextNativeNode.length; i++) {
              _this2._nativeNode.push(nextNativeNode[i]);
            }
          }
        })();
      }

      if (delayRemoveFirstPrevChild) {
        firstPrevChild.unmountComponent();
      }

      this._renderedChildren = nextChildren;
    }
  }, {
    key: 'getNativeNode',
    value: function getNativeNode() {
      if (this._nativeNode == null) {
        this._nativeNode = _host2.default.driver.createElement(this._instance);
        _instance2.default.set(this._nativeNode, this._instance);
      }

      return this._nativeNode;
    }
  }, {
    key: 'getPublicInstance',
    value: function getPublicInstance() {
      return this.getNativeNode();
    }
  }, {
    key: 'getName',
    value: function getName() {
      return this._currentElement.type;
    }
  }]);

  return NativeComponent;
}();

exports.default = NativeComponent;
module.exports = exports['default'];

/***/ }),
/* 12 */
/***/ (function(module, exports, __webpack_require__) {

Object.defineProperty(exports, "__esModule", {
  value: true
});
/*
 * Ref manager
 */

exports.default = {
  update: function update(prevElement, nextElement, component) {
    var prevRef = prevElement != null && prevElement.ref;
    var nextRef = nextElement != null && nextElement.ref;

    // Update refs in owner component
    if (prevRef !== nextRef) {
      // Detach prev RenderedElement's ref
      prevRef != null && this.detach(prevElement._owner, prevRef, component);
      // Attach next RenderedElement's ref
      nextRef != null && this.attach(nextElement._owner, nextRef, component);
    }
  },
  attach: function attach(ownerComponent, ref, component) {
    if (!ownerComponent) {
      throw new Error('You might be adding a ref to a component that was not created inside a component\'s ' + '`render` method, or you have multiple copies of Rax loaded.');
    }

    var instance = component.getPublicInstance();
    if (typeof ref === 'function') {
      ref(instance);
    } else {
      ownerComponent._instance.refs[ref] = instance;
    }
  },
  detach: function detach(ownerComponent, ref, component) {
    if (typeof ref === 'function') {
      // When the referenced component is unmounted and whenever the ref changes, the old ref will be called with null as an argument.
      ref(null);
    } else {
      // Must match component and ref could detach the ref on owner when A's before ref is B's current ref
      var instance = component.getPublicInstance();
      if (ownerComponent._instance.refs[ref] === instance) {
        delete ownerComponent._instance.refs[ref];
      }
    }
  }
};
module.exports = exports['default'];

/***/ }),
/* 13 */
/***/ (function(module, exports, __webpack_require__) {

Object.defineProperty(exports, "__esModule", {
  value: true
});

exports.default = function (children, element, index) {
  var elementKey = element && element.key;
  var hasKey = typeof elementKey === 'string';
  var defaultName = '.' + index.toString(36);

  if (hasKey) {
    var keyName = '$' + elementKey;
    // Child keys must be unique.
    var keyUnique = children[keyName] === undefined;
    // Only the first child will be used when encountered two children with the same key
    if (!keyUnique) { console.warn('Encountered two children with the same key "' + elementKey + '".'); }

    return keyUnique ? keyName : defaultName;
  } else {
    return defaultName;
  }
};

module.exports = exports['default'];

/***/ }),
/* 14 */
/***/ (function(module, exports, __webpack_require__) {

var _typeof2 = typeof Symbol === "function" && typeof Symbol.iterator === "symbol" ? function (obj) { return typeof obj; } : function (obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; };

Object.defineProperty(exports, "__esModule", {
  value: true
});

var _typeof = typeof Symbol === "function" && _typeof2(Symbol.iterator) === "symbol" ? function (obj) {
  return typeof obj === "undefined" ? "undefined" : _typeof2(obj);
} : function (obj) {
  return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj === "undefined" ? "undefined" : _typeof2(obj);
};

exports.isRem = isRem;
exports.calcRem = calcRem;
exports.getRem = getRem;
exports.setRem = setRem;
exports.isUnitNumber = isUnitNumber;
exports.convertUnit = convertUnit;
/**
 * CSS properties which accept numbers but are not in units of "px".
 */
var UNITLESS_NUMBER_PROPS = {
  animationIterationCount: true,
  borderImageOutset: true,
  borderImageSlice: true,
  borderImageWidth: true,
  boxFlex: true,
  boxFlexGroup: true,
  boxOrdinalGroup: true,
  columnCount: true,
  flex: true,
  flexGrow: true,
  flexPositive: true,
  flexShrink: true,
  flexNegative: true,
  flexOrder: true,
  gridRow: true,
  gridColumn: true,
  fontWeight: true,
  lineClamp: true,
  // We make lineHeight default is px that is diff with w3c spec
  // lineHeight: true,
  opacity: true,
  order: true,
  orphans: true,
  tabSize: true,
  widows: true,
  zIndex: true,
  zoom: true,
  // Weex only
  lines: true
};
var SUFFIX = 'rem';
var REM_REG = /[-+]?\d*\.?\d+rem/g;
var GLOBAL_REM_UNIT = '__global_rem_unit__';
var global = (typeof window === 'undefined' ? 'undefined' : _typeof(window)) === 'object' ? window : (typeof global === 'undefined' ? 'undefined' : _typeof(global)) === 'object' ? global : {};
// Default 1 rem to 1 px
if (global[GLOBAL_REM_UNIT] == null) {
  global[GLOBAL_REM_UNIT] = 1;
}

/**
 * Is string contains rem
 * @param {String} str
 * @returns {Boolean}
 */
function isRem(str) {
  return typeof str === 'string' && str.indexOf(SUFFIX) !== -1;
}

/**
 * Calculate rem to pixels: '1.2rem' => 1.2 * rem
 * @param {String} str
 * @param {Number} rem
 * @returns {number}
 */
function calcRem(str) {
  var remUnit = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : global[GLOBAL_REM_UNIT];

  return str.replace(REM_REG, function (rem) {
    return parseFloat(rem) * remUnit + 'px';
  });
}

function getRem() {
  return global[GLOBAL_REM_UNIT];
}

function setRem(rem) {
  global[GLOBAL_REM_UNIT] = rem;
}

function isUnitNumber(val, prop) {
  return typeof val === 'number' && !UNITLESS_NUMBER_PROPS[prop];
}

function convertUnit(val, prop) {
  var remUnit = arguments.length > 2 && arguments[2] !== undefined ? arguments[2] : global[GLOBAL_REM_UNIT];

  if (prop && isUnitNumber(val, prop)) {
    return val * remUnit + 'px';
  } else if (isRem(val)) {
    return calcRem(val, remUnit);
  }

  return val;
}
exports['default'] = module.exports;
exports.default = module.exports;

/***/ }),
/* 15 */
/***/ (function(module, exports, __webpack_require__) {

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.transformPropsAttrsToStyle = transformPropsAttrsToStyle;
exports.renamePropsAttr = renamePropsAttr;
/**
 * transformPropAttrsToStyle
 *
 * @param {Object} props
 * @param {Array} attrs
 */
function transformPropsAttrsToStyle(props, attrs) {
  props.style = props.style || {};

  attrs.forEach(function (attr) {
    if (props[attr] && !props.style[attr]) {
      props.style[attr] = props[attr];
      delete props[attr];
    }
  });

  return props;
}

/**
 * renamePropsAttr
 *
 * @param {Object} props
 * @param {String} originalAttrName
 * @param {String} newAttrName
 */
function renamePropsAttr(props, originalAttrName, newAttrName) {
  if (props[originalAttrName] && !props[newAttrName]) {
    props[newAttrName] = props[originalAttrName];
    delete props[originalAttrName];
  }

  return props;
}
exports["default"] = module.exports;
exports.default = module.exports;

/***/ }),
/* 16 */
/***/ (function(module, exports, __webpack_require__) {

Object.defineProperty(exports, "__esModule", {
  value: true
});

var _host = __webpack_require__(0);

var _host2 = _interopRequireDefault(_host);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function findDOMNode(instance) {
  if (instance == null) {
    return null;
  }

  // If a native node, weex may not export ownerDocument property
  if (instance.ownerDocument || instance.nodeType) {
    return instance;
  }

  // Native component
  if (instance._nativeNode) {
    return instance._nativeNode;
  }

  if (typeof instance == 'string') {
    return _host2.default.driver.getElementById(instance);
  }

  if (typeof instance.render !== 'function') {
    throw new Error('findDOMNode: find by neither component nor DOM node.');
  }

  // Composite component
  var internal = instance._internal;

  if (internal) {
    while (!internal._nativeNode) {
      internal = internal._renderedComponent;
      // If not mounted
      if (internal == null) {
        return null;
      }
    }
    return internal._nativeNode;
  } else {
    throw new Error('findDOMNode: find on an unmounted component.');
  }
}

exports.default = findDOMNode;
module.exports = exports['default'];

/***/ }),
/* 17 */
/***/ (function(module, exports, __webpack_require__) {

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.Children = exports.version = exports.setNativeProps = exports.findComponentInstance = exports.unmountComponentAtNode = exports.findDOMNode = exports.hydrate = exports.render = exports.PropTypes = exports.PureComponent = exports.Component = exports.createFactory = exports.isValidElement = exports.cloneElement = exports.createElement = undefined;

__webpack_require__(18);

var _element = __webpack_require__(6);

var _component = __webpack_require__(4);

var _component2 = _interopRequireDefault(_component);

var _purecomponent = __webpack_require__(20);

var _purecomponent2 = _interopRequireDefault(_purecomponent);

var _proptypes = __webpack_require__(21);

var _proptypes2 = _interopRequireDefault(_proptypes);

var _render2 = __webpack_require__(10);

var _render3 = _interopRequireDefault(_render2);

var _hydrate2 = __webpack_require__(42);

var _hydrate3 = _interopRequireDefault(_hydrate2);

var _findDOMNode2 = __webpack_require__(16);

var _findDOMNode3 = _interopRequireDefault(_findDOMNode2);

var _unmountComponentAtNode2 = __webpack_require__(9);

var _unmountComponentAtNode3 = _interopRequireDefault(_unmountComponentAtNode2);

var _findComponentInstance2 = __webpack_require__(43);

var _findComponentInstance3 = _interopRequireDefault(_findComponentInstance2);

var _setNativeProps2 = __webpack_require__(44);

var _setNativeProps3 = _interopRequireDefault(_setNativeProps2);

var _version2 = __webpack_require__(45);

var _version3 = _interopRequireDefault(_version2);

var _children = __webpack_require__(46);

var _children2 = _interopRequireDefault(_children);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

exports.createElement = _element.createElement;
exports.cloneElement = _element.cloneElement;
exports.isValidElement = _element.isValidElement;
exports.createFactory = _element.createFactory;
exports.Component = _component2.default;
exports.PureComponent = _purecomponent2.default;
exports.PropTypes = _proptypes2.default;
exports.render = _render3.default;
exports.hydrate = _hydrate3.default;
exports.findDOMNode = _findDOMNode3.default;
exports.unmountComponentAtNode = _unmountComponentAtNode3.default;
exports.findComponentInstance = _findComponentInstance3.default;
exports.setNativeProps = _setNativeProps3.default;
exports.version = _version3.default;
exports.Children = _children2.default;

/***/ }),
/* 18 */
/***/ (function(module, exports, __webpack_require__) {

var _hook = __webpack_require__(5);

var _hook2 = _interopRequireDefault(_hook);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

/* global __REACT_DEVTOOLS_GLOBAL_HOOK__ */
if (typeof __REACT_DEVTOOLS_GLOBAL_HOOK__ !== 'undefined' && typeof __REACT_DEVTOOLS_GLOBAL_HOOK__.inject === 'function') {
  __REACT_DEVTOOLS_GLOBAL_HOOK__.inject(_hook2.default);
}

/***/ }),
/* 19 */
/***/ (function(module, exports, __webpack_require__) {

Object.defineProperty(exports, "__esModule", {
  value: true
});

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) { descriptor.writable = true; } Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) { defineProperties(Constructor.prototype, protoProps); } if (staticProps) { defineProperties(Constructor, staticProps); } return Constructor; }; }();

var _component = __webpack_require__(4);

var _component2 = _interopRequireDefault(_component);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _possibleConstructorReturn(self, call) { if (!self) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return call && (typeof call === "object" || typeof call === "function") ? call : self; }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function, not " + typeof superClass); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } }); if (superClass) { Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass; } }

var rootCounter = 1;

var Root = function (_Component) {
  _inherits(Root, _Component);

  function Root() {
    var arguments$1 = arguments;

    var _ref;

    var _temp, _this, _ret;

    _classCallCheck(this, Root);

    for (var _len = arguments.length, args = Array(_len), _key = 0; _key < _len; _key++) {
      args[_key] = arguments$1[_key];
    }

    return _ret = (_temp = (_this = _possibleConstructorReturn(this, (_ref = Root.__proto__ || Object.getPrototypeOf(Root)).call.apply(_ref, [this].concat(args))), _this), _this.rootID = rootCounter++, _temp), _possibleConstructorReturn(_this, _ret);
  }

  _createClass(Root, [{
    key: 'isRootComponent',
    value: function isRootComponent() {}
  }, {
    key: 'render',
    value: function render() {
      return this.props.children;
    }
  }, {
    key: 'getPublicInstance',
    value: function getPublicInstance() {
      return this.getRenderedComponent().getPublicInstance();
    }
  }, {
    key: 'getRenderedComponent',
    value: function getRenderedComponent() {
      return this._internal._renderedComponent;
    }
  }]);

  return Root;
}(_component2.default);

exports.default = Root;
module.exports = exports['default'];

/***/ }),
/* 20 */
/***/ (function(module, exports, __webpack_require__) {

Object.defineProperty(exports, "__esModule", {
  value: true
});

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) { descriptor.writable = true; } Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) { defineProperties(Constructor.prototype, protoProps); } if (staticProps) { defineProperties(Constructor, staticProps); } return Constructor; }; }();

var _component = __webpack_require__(4);

var _component2 = _interopRequireDefault(_component);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _possibleConstructorReturn(self, call) { if (!self) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return call && (typeof call === "object" || typeof call === "function") ? call : self; }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function, not " + typeof superClass); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } }); if (superClass) { Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass; } }

/**
 * Pure component class.
 */
var PureComponent = function (_Component) {
  _inherits(PureComponent, _Component);

  function PureComponent(props, context) {
    _classCallCheck(this, PureComponent);

    return _possibleConstructorReturn(this, (PureComponent.__proto__ || Object.getPrototypeOf(PureComponent)).call(this, props, context));
  }

  _createClass(PureComponent, [{
    key: 'isPureComponentClass',
    value: function isPureComponentClass() {}
  }]);

  return PureComponent;
}(_component2.default);

exports.default = PureComponent;
module.exports = exports['default'];

/***/ }),
/* 21 */
/***/ (function(module, exports, __webpack_require__) {

Object.defineProperty(exports, "__esModule", {
  value: true
});
/*
 * Current PropTypes only export some api with react, not validate in runtime.
 */

function createChainableTypeChecker(validate) {
  function checkType(isRequired, props, propName, componentName, location, propFullName) {
    return typeChecker;
  }

  var chainedCheckType = checkType.bind(null, false);
  chainedCheckType.isRequired = checkType.bind(null, true);

  return chainedCheckType;
}

function createTypeChecker(expectedType) {
  function validate(props, propName, componentName, location, propFullName) {
    // Noop
  }
  return createChainableTypeChecker(validate);
}

var typeChecker = createTypeChecker();

exports.default = {
  array: typeChecker,
  bool: typeChecker,
  func: typeChecker,
  number: typeChecker,
  object: typeChecker,
  string: typeChecker,
  symbol: typeChecker,
  element: typeChecker,
  node: typeChecker,
  any: typeChecker,
  arrayOf: typeChecker,
  instanceOf: typeChecker,
  objectOf: typeChecker,
  oneOf: typeChecker,
  oneOfType: typeChecker,
  shape: typeChecker
};
module.exports = exports["default"];

/***/ }),
/* 22 */
/***/ (function(module, exports, __webpack_require__) {

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.default = inject;

var _universalEnv = __webpack_require__(8);

var _host = __webpack_require__(0);

var _host2 = _interopRequireDefault(_host);

var _empty = __webpack_require__(23);

var _empty2 = _interopRequireDefault(_empty);

var _native = __webpack_require__(11);

var _native2 = _interopRequireDefault(_native);

var _text = __webpack_require__(24);

var _text2 = _interopRequireDefault(_text);

var _composite = __webpack_require__(25);

var _composite2 = _interopRequireDefault(_composite);

var _fragment = __webpack_require__(29);

var _fragment2 = _interopRequireDefault(_fragment);

var _driverWeex = __webpack_require__(30);

var _driverWeex2 = _interopRequireDefault(_driverWeex);

var _driverBrowser = __webpack_require__(40);

var _driverBrowser2 = _interopRequireDefault(_driverBrowser);

var _hook = __webpack_require__(5);

var _hook2 = _interopRequireDefault(_hook);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function inject(_ref) {
  var driver = _ref.driver,
      hook = _ref.hook,
      measurer = _ref.measurer,
      deviceWidth = _ref.deviceWidth,
      viewportWidth = _ref.viewportWidth,
      eventRegistry = _ref.eventRegistry,
      bodyType = _ref.bodyType,
      bodyProps = _ref.bodyProps;

  // Inject component class
  _host2.default.EmptyComponent = _empty2.default;
  _host2.default.NativeComponent = _native2.default;
  _host2.default.TextComponent = _text2.default;
  _host2.default.FragmentComponent = _fragment2.default;
  _host2.default.CompositeComponent = _composite2.default;

  // Inject devtool hook
  _host2.default.hook = hook || _hook2.default;

  // Inject performance measurer
  _host2.default.measurer = measurer;

  // Inject render driver
  if (!_host2.default.driver) {
    if (!driver) {
      if (_universalEnv.isWeex) {
        driver = _driverWeex2.default;
      } else if (_universalEnv.isWeb) {
        driver = _driverBrowser2.default;
      } else {
        throw Error('No builtin driver matched');
      }
    }
    _host2.default.driver = driver;
  }

  if (deviceWidth && _host2.default.driver.setDeviceWidth) {
    _host2.default.driver.setDeviceWidth(deviceWidth);
  }

  if (viewportWidth && _host2.default.driver.setViewportWidth) {
    _host2.default.driver.setViewportWidth(viewportWidth);
  }

  if (eventRegistry) {
    _host2.default.driver.eventRegistry = eventRegistry;
  }

  // Body custom type only works in weex
  if (bodyType) {
    _host2.default.driver.bodyType = bodyType;
  }

  if (bodyProps) {
    _host2.default.driver.bodyProps = bodyProps;
  }
}
module.exports = exports['default'];

/***/ }),
/* 23 */
/***/ (function(module, exports, __webpack_require__) {

Object.defineProperty(exports, "__esModule", {
  value: true
});

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) { descriptor.writable = true; } Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) { defineProperties(Constructor.prototype, protoProps); } if (staticProps) { defineProperties(Constructor, staticProps); } return Constructor; }; }();

var _host = __webpack_require__(0);

var _host2 = _interopRequireDefault(_host);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

/**
 * Empty Component
 */
var EmptyComponent = function () {
  function EmptyComponent() {
    _classCallCheck(this, EmptyComponent);

    this._currentElement = null;
  }

  _createClass(EmptyComponent, [{
    key: 'mountComponent',
    value: function mountComponent(parent, parentInstance, context, childMounter) {
      this._parent = parent;
      this._parentInstance = parentInstance;
      this._context = context;

      var instance = {
        _internal: this
      };

      var nativeNode = this.getNativeNode();
      if (childMounter) {
        childMounter(nativeNode, parent);
      } else {
        _host2.default.driver.appendChild(nativeNode, parent);
      }

      return instance;
    }
  }, {
    key: 'unmountComponent',
    value: function unmountComponent(notRemoveChild) {
      if (this._nativeNode && !notRemoveChild) {
        _host2.default.driver.removeChild(this._nativeNode, this._parent);
      }

      this._nativeNode = null;
      this._parent = null;
      this._parentInstance = null;
      this._context = null;
    }
  }, {
    key: 'updateComponent',
    value: function updateComponent() {
      // Noop
    }
  }, {
    key: 'getNativeNode',
    value: function getNativeNode() {
      // Weex native node
      if (this._nativeNode == null) {
        this._nativeNode = _host2.default.driver.createEmpty();
      }

      return this._nativeNode;
    }
  }]);

  return EmptyComponent;
}();

exports.default = EmptyComponent;
module.exports = exports['default'];

/***/ }),
/* 24 */
/***/ (function(module, exports, __webpack_require__) {

Object.defineProperty(exports, "__esModule", {
  value: true
});

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) { descriptor.writable = true; } Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) { defineProperties(Constructor.prototype, protoProps); } if (staticProps) { defineProperties(Constructor, staticProps); } return Constructor; }; }();

var _host = __webpack_require__(0);

var _host2 = _interopRequireDefault(_host);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

/**
 * Text Component
 */
var TextComponent = function () {
  function TextComponent(element) {
    _classCallCheck(this, TextComponent);

    this._currentElement = element;
    this._stringText = String(element);
  }

  _createClass(TextComponent, [{
    key: 'mountComponent',
    value: function mountComponent(parent, parentInstance, context, childMounter) {
      this._parent = parent;
      this._parentInstance = parentInstance;
      this._context = context;
      this._mountID = _host2.default.mountID++;

      // Weex dom operation
      var nativeNode = this.getNativeNode();

      if (childMounter) {
        childMounter(nativeNode, parent);
      } else {
        _host2.default.driver.appendChild(nativeNode, parent);
      }

      var instance = {
        _internal: this
      };

      _host2.default.hook.Reconciler.mountComponent(this);

      return instance;
    }
  }, {
    key: 'unmountComponent',
    value: function unmountComponent(notRemoveChild) {
      if (this._nativeNode && !notRemoveChild) {
        _host2.default.driver.removeChild(this._nativeNode, this._parent);
      }

      _host2.default.hook.Reconciler.unmountComponent(this);

      this._currentElement = null;
      this._nativeNode = null;
      this._parent = null;
      this._parentInstance = null;
      this._context = null;
      this._stringText = null;
    }
  }, {
    key: 'updateComponent',
    value: function updateComponent(prevElement, nextElement, context) {
      // If some text do noting
      if (prevElement !== nextElement) {
        // Replace current element
        this._currentElement = nextElement;
        // Devtool read the latest stringText value
        this._stringText = String(nextElement);
        _host2.default.driver.updateText(this.getNativeNode(), this._stringText);
        _host2.default.hook.Reconciler.receiveComponent(this);
      }
    }
  }, {
    key: 'getNativeNode',
    value: function getNativeNode() {
      if (this._nativeNode == null) {
        this._nativeNode = _host2.default.driver.createText(this._stringText);
      }
      return this._nativeNode;
    }
  }]);

  return TextComponent;
}();

exports.default = TextComponent;
module.exports = exports['default'];

/***/ }),
/* 25 */
/***/ (function(module, exports, __webpack_require__) {

Object.defineProperty(exports, "__esModule", {
  value: true
});

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) { descriptor.writable = true; } Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) { defineProperties(Constructor.prototype, protoProps); } if (staticProps) { defineProperties(Constructor, staticProps); } return Constructor; }; }();

var _stateless = __webpack_require__(26);

var _stateless2 = _interopRequireDefault(_stateless);

var _updater = __webpack_require__(27);

var _updater2 = _interopRequireDefault(_updater);

var _host = __webpack_require__(0);

var _host2 = _interopRequireDefault(_host);

var _ref = __webpack_require__(12);

var _ref2 = _interopRequireDefault(_ref);

var _instantiateComponent = __webpack_require__(2);

var _instantiateComponent2 = _interopRequireDefault(_instantiateComponent);

var _shouldUpdateComponent = __webpack_require__(3);

var _shouldUpdateComponent2 = _interopRequireDefault(_shouldUpdateComponent);

var _shallowEqual = __webpack_require__(28);

var _shallowEqual2 = _interopRequireDefault(_shallowEqual);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function performInSandbox(fn, instance, callback) {
  try {
    return fn();
  } catch (e) {
    if (callback) {
      callback(e);
    } else {
      handleError(instance, e);
    }
  }
}

function handleError(instance, error) {
  var boundary = void 0;

  while (instance) {
    if (typeof instance.componentDidCatch === 'function') {
      boundary = instance;
      break;
    } else if (instance._internal && instance._internal._parentInstance) {
      instance = instance._internal._parentInstance;
    } else {
      break;
    }
  }

  if (boundary) {
    boundary.componentDidCatch(error);
  } else {
    if (_host2.default.sandbox) {
      setTimeout(function () {
        throw error;
      }, 0);
    } else {
      throw error;
    }
  }
}

var CompositeComponent = function () {
  function CompositeComponent(element) {
    _classCallCheck(this, CompositeComponent);

    this._currentElement = element;
  }

  _createClass(CompositeComponent, [{
    key: 'getName',
    value: function getName() {
      var type = this._currentElement.type;
      var constructor = this._instance && this._instance.constructor;
      return type.displayName || constructor && constructor.displayName || type.name || constructor && constructor.name || null;
    }
  }, {
    key: 'mountComponent',
    value: function mountComponent(parent, parentInstance, context, childMounter) {
      this._parent = parent;
      this._parentInstance = parentInstance;
      this._context = context;
      this._mountID = _host2.default.mountID++;
      this._updateCount = 0;

      var Component = this._currentElement.type;
      var publicProps = this._currentElement.props;
      var isClass = Component.prototype;
      var isComponentClass = isClass && Component.prototype.isComponentClass;
      // Class stateless component without state but have lifecycles
      var isStatelessClass = isClass && Component.prototype.render;

      // Context process
      var publicContext = this._processContext(context);

      // Initialize the public class
      var instance = void 0;
      var renderedElement = void 0;

      if (isComponentClass || isStatelessClass) {
        // Component instance
        instance = new Component(publicProps, publicContext, _updater2.default);
      } else if (typeof Component === 'function') {
        // Functional stateless component without state and lifecycles
        instance = new _stateless2.default(Component);
      } else {
        throw new Error('Invalid component type: ' + Component + '. (keys: ' + Object.keys(Component) + ')');
      }

      // These should be set up in the constructor, but as a convenience for
      // simpler class abstractions, we set them up after the fact.
      instance.props = publicProps;
      instance.context = publicContext;
      instance.refs = {};

      // Inject the updater into instance
      instance.updater = _updater2.default;
      instance._internal = this;
      this._instance = instance;

      // Init state, must be set to an object or null
      var initialState = instance.state;
      if (initialState === undefined) {
        // TODO clone the state?
        instance.state = initialState = null;
      }

      var error = null;
      var errorCallback = function errorCallback(e) {
        error = e;
      };

      if (instance.componentWillMount) {
        performInSandbox(function () {
          {
            instance.componentWillMount();
          }
        }, instance, errorCallback);
      }

      if (renderedElement == null) {
        _host2.default.component = this;
        // Process pending state when call setState in componentWillMount
        instance.state = this._processPendingState(publicProps, publicContext);

        performInSandbox(function () {
          {
            renderedElement = instance.render();
          }
        }, instance, errorCallback);

        _host2.default.component = null;
      }

      this._renderedComponent = (0, _instantiateComponent2.default)(renderedElement);
      this._renderedComponent.mountComponent(this._parent, instance, this._processChildContext(context), childMounter);

      if (error) {
        handleError(instance, error);
      }

      if (this._currentElement && this._currentElement.ref) {
        _ref2.default.attach(this._currentElement._owner, this._currentElement.ref, this);
      }

      if (instance.componentDidMount) {
        performInSandbox(function () {
          {
            instance.componentDidMount();
          }
        }, instance);
      }

      _host2.default.hook.Reconciler.mountComponent(this);

      return instance;
    }
  }, {
    key: 'unmountComponent',
    value: function unmountComponent(notRemoveChild) {
      var instance = this._instance;

      if (instance.componentWillUnmount) {
        performInSandbox(function () {
          instance.componentWillUnmount();
        }, instance);
      }

      _host2.default.hook.Reconciler.unmountComponent(this);

      instance._internal = null;

      if (this._renderedComponent != null) {
        var ref = this._currentElement.ref;
        if (ref) {
          _ref2.default.detach(this._currentElement._owner, ref, this);
        }

        this._renderedComponent.unmountComponent(notRemoveChild);
        this._renderedComponent = null;
        this._instance = null;
      }

      this._currentElement = null;
      this._parentInstance = null;

      // Reset pending fields
      // Even if this component is scheduled for another update in ReactUpdates,
      // it would still be ignored because these fields are reset.
      this._pendingStateQueue = null;
      this._pendingForceUpdate = false;

      // These fields do not really need to be reset since this object is no
      // longer accessible.
      this._context = null;
    }

    /**
     * Filters the context object to only contain keys specified in
     * `contextTypes`
     */

  }, {
    key: '_processContext',
    value: function _processContext(context) {
      var Component = this._currentElement.type;
      var contextTypes = Component.contextTypes;
      if (!contextTypes) {
        return {};
      }
      var maskedContext = {};
      for (var contextName in contextTypes) {
        maskedContext[contextName] = context[contextName];
      }
      return maskedContext;
    }
  }, {
    key: '_processChildContext',
    value: function _processChildContext(currentContext) {
      var instance = this._instance;
      var childContext = instance.getChildContext && instance.getChildContext();
      if (childContext) {
        return Object.assign({}, currentContext, childContext);
      }
      return currentContext;
    }
  }, {
    key: '_processPendingState',
    value: function _processPendingState(props, context) {
      var instance = this._instance;
      var queue = this._pendingStateQueue;
      if (!queue) {
        return instance.state;
      }
      // Reset pending queue
      this._pendingStateQueue = null;
      var nextState = Object.assign({}, instance.state);
      for (var i = 0; i < queue.length; i++) {
        var partial = queue[i];
        Object.assign(nextState, typeof partial === 'function' ? partial.call(instance, nextState, props, context) : partial);
      }

      return nextState;
    }
  }, {
    key: 'updateComponent',
    value: function updateComponent(prevElement, nextElement, prevUnmaskedContext, nextUnmaskedContext) {
      var instance = this._instance;

      if (!instance) {
        console.error('Update component \'' + this.getName() + '\' that has already been unmounted (or failed to mount).');
      }

      var willReceive = false;
      var nextContext = void 0;
      var nextProps = void 0;

      // Determine if the context has changed or not
      if (this._context === nextUnmaskedContext) {
        nextContext = instance.context;
      } else {
        nextContext = this._processContext(nextUnmaskedContext);
        willReceive = true;
      }

      // Distinguish between a props update versus a simple state update
      if (prevElement === nextElement) {
        // Skip checking prop types again -- we don't read component.props to avoid
        // warning for DOM component props in this upgrade
        nextProps = nextElement.props;
      } else {
        nextProps = nextElement.props;
        willReceive = true;
      }

      var hasReceived = willReceive && instance.componentWillReceiveProps;

      if (hasReceived) {
        // Calling this.setState() within componentWillReceiveProps will not trigger an additional render.
        this._pendingState = true;
        performInSandbox(function () {
          instance.componentWillReceiveProps(nextProps, nextContext);
        }, instance);
        this._pendingState = false;
      }

      // Update refs
      _ref2.default.update(prevElement, nextElement, this);

      // Shoud update always default
      var shouldUpdate = true;
      var prevProps = instance.props;
      var prevState = instance.state;
      // TODO: could delay execution processPendingState
      var nextState = this._processPendingState(nextProps, nextContext);

      // ShouldComponentUpdate is not called when forceUpdate is used
      if (!this._pendingForceUpdate) {
        if (instance.shouldComponentUpdate) {
          shouldUpdate = performInSandbox(function () {
            return instance.shouldComponentUpdate(nextProps, nextState, nextContext);
          }, instance);
        } else if (instance.isPureComponentClass) {
          shouldUpdate = !(0, _shallowEqual2.default)(prevProps, nextProps) || !(0, _shallowEqual2.default)(prevState, nextState);
        }
      }

      if (shouldUpdate) {
        this._pendingForceUpdate = false;
        // Will set `this.props`, `this.state` and `this.context`.
        var prevContext = instance.context;

        // Cannot use this.setState() in componentWillUpdate.
        // If need to update state in response to a prop change, use componentWillReceiveProps instead.
        performInSandbox(function () {
          if (instance.componentWillUpdate) {
            instance.componentWillUpdate(nextProps, nextState, nextContext);
          }
        }, instance);

        // Replace with next
        this._currentElement = nextElement;
        this._context = nextUnmaskedContext;
        instance.props = nextProps;
        instance.state = nextState;
        instance.context = nextContext;

        this._updateRenderedComponent(nextUnmaskedContext);

        performInSandbox(function () {
          if (instance.componentDidUpdate) {
            instance.componentDidUpdate(prevProps, prevState, prevContext);
          }
        }, instance);

        this._updateCount++;
      } else {
        // If it's determined that a component should not update, we still want
        // to set props and state but we shortcut the rest of the update.
        this._currentElement = nextElement;
        this._context = nextUnmaskedContext;
        instance.props = nextProps;
        instance.state = nextState;
        instance.context = nextContext;
      }

      // Flush setState callbacks set in componentWillReceiveProps
      if (hasReceived) {
        var callbacks = this._pendingCallbacks;
        this._pendingCallbacks = null;
        _updater2.default.runCallbacks(callbacks, instance);
      }

      _host2.default.hook.Reconciler.receiveComponent(this);
    }

    /**
     * Call the component's `render` method and update the DOM accordingly.
     */

  }, {
    key: '_updateRenderedComponent',
    value: function _updateRenderedComponent(context) {
      var prevRenderedComponent = this._renderedComponent;
      var prevRenderedElement = prevRenderedComponent._currentElement;

      var instance = this._instance;
      var nextRenderedElement = void 0;

      _host2.default.component = this;

      performInSandbox(function () {
        {
          nextRenderedElement = instance.render();
        }
      }, instance);

      _host2.default.component = null;

      if ((0, _shouldUpdateComponent2.default)(prevRenderedElement, nextRenderedElement)) {
        prevRenderedComponent.updateComponent(prevRenderedElement, nextRenderedElement, prevRenderedComponent._context, this._processChildContext(context));
        
      } else {
        var oldChild = prevRenderedComponent.getNativeNode();
        prevRenderedComponent.unmountComponent(true);

        this._renderedComponent = (0, _instantiateComponent2.default)(nextRenderedElement);
        this._renderedComponent.mountComponent(this._parent, instance, this._processChildContext(context), function (newChild, parent) {
          // TODO: Duplicate code in native component file
          if (!Array.isArray(newChild)) {
            newChild = [newChild];
          }

          // oldChild or newChild all maybe fragment
          if (!Array.isArray(oldChild)) {
            oldChild = [oldChild];
          }

          // If newChild count large then oldChild
          var lastNewChild = void 0;
          for (var i = 0; i < newChild.length; i++) {
            var child = newChild[i];
            if (oldChild[i]) {
              _host2.default.driver.replaceChild(child, oldChild[i]);
            } else {
              _host2.default.driver.insertAfter(child, lastNewChild);
            }
            lastNewChild = child;
          }

          // If newChild count less then oldChild
          if (newChild.length < oldChild.length) {
            for (var _i = newChild.length; _i < oldChild.length; _i++) {
              _host2.default.driver.removeChild(oldChild[_i]);
            }
          }
        });
      }
    }
  }, {
    key: 'getNativeNode',
    value: function getNativeNode() {
      var renderedComponent = this._renderedComponent;
      if (renderedComponent) {
        return renderedComponent.getNativeNode();
      }
    }
  }, {
    key: 'getPublicInstance',
    value: function getPublicInstance() {
      var instance = this._instance;
      // The Stateless components cannot be given refs
      if (instance instanceof _stateless2.default) {
        return null;
      }
      return instance;
    }
  }]);

  return CompositeComponent;
}();

exports.default = CompositeComponent;
module.exports = exports['default'];

/***/ }),
/* 26 */
/***/ (function(module, exports, __webpack_require__) {

Object.defineProperty(exports, "__esModule", {
  value: true
});

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) { descriptor.writable = true; } Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) { defineProperties(Constructor.prototype, protoProps); } if (staticProps) { defineProperties(Constructor, staticProps); } return Constructor; }; }();

var _host = __webpack_require__(0);

var _host2 = _interopRequireDefault(_host);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

/**
 * Stateless Component Class Wrapper
 */
var StatelessComponent = function () {
  function StatelessComponent(pureRender) {
    _classCallCheck(this, StatelessComponent);

    // A stateless function
    this.pureRender = pureRender;
  }

  _createClass(StatelessComponent, [{
    key: 'render',
    value: function render() {
      return this.pureRender(this.props, this.context);
    }
  }]);

  return StatelessComponent;
}();

exports.default = StatelessComponent;
module.exports = exports['default'];

/***/ }),
/* 27 */
/***/ (function(module, exports, __webpack_require__) {

Object.defineProperty(exports, "__esModule", {
  value: true
});
function enqueueCallback(internal, callback) {
  if (callback) {
    var callbackQueue = internal._pendingCallbacks || (internal._pendingCallbacks = []);
    callbackQueue.push(callback);
  }
}

function enqueueState(internal, partialState) {
  if (partialState) {
    var stateQueue = internal._pendingStateQueue || (internal._pendingStateQueue = []);
    stateQueue.push(partialState);
  }
}

var Updater = {
  setState: function setState(component, partialState, callback) {
    var internal = component._internal;

    if (!internal) {
      return;
    }

    enqueueState(internal, partialState);
    enqueueCallback(internal, callback);

    // pending in componentWillReceiveProps and componentWillMount
    if (!internal._pendingState && internal._renderedComponent) {
      this.runUpdate(component);
    }
  },

  forceUpdate: function forceUpdate(component, callback) {
    var internal = component._internal;

    if (!internal) {
      return;
    }

    internal._pendingForceUpdate = true;

    enqueueCallback(internal, callback);
    // pending in componentWillMount
    if (internal._renderedComponent) {
      this.runUpdate(component);
    }
  },

  runUpdate: function runUpdate(component) {
    var internal = component._internal;

    // If updateComponent happens to enqueue any new updates, we
    // shouldn't execute the callbacks until the next render happens, so
    // stash the callbacks first
    var callbacks = internal._pendingCallbacks;
    internal._pendingCallbacks = null;

    var prevElement = internal._currentElement;
    var prevUnmaskedContext = internal._context;

    if (internal._pendingStateQueue || internal._pendingForceUpdate) {
      internal.updateComponent(prevElement, prevElement, prevUnmaskedContext, prevUnmaskedContext);
    }

    this.runCallbacks(callbacks, component);
  },

  runCallbacks: function runCallbacks(callbacks, context) {
    if (callbacks) {
      for (var i = 0; i < callbacks.length; i++) {
        callbacks[i].call(context);
      }
    }
  }
};

exports.default = Updater;
module.exports = exports["default"];

/***/ }),
/* 28 */
/***/ (function(module, exports, __webpack_require__) {

Object.defineProperty(exports, "__esModule", {
  value: true
});

var _typeof = typeof Symbol === "function" && typeof Symbol.iterator === "symbol" ? function (obj) { return typeof obj; } : function (obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; };

var hasOwnProperty = Object.prototype.hasOwnProperty;

/**
 * inlined Object.is polyfill to avoid requiring consumers ship their own
 * https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Object/is
 */
function is(x, y) {
  // SameValue algorithm
  if (x === y) {
    // Steps 1-5, 7-10
    // Steps 6.b-6.e: +0 != -0
    return x !== 0 || 1 / x === 1 / y;
  } else {
    // Step 6.a: NaN == NaN
    return x !== x && y !== y;
  }
}

/**
 * Performs equality by iterating through keys on an object and returning false
 * when any key has values which are not strictly equal between the arguments.
 * Returns true when the values of all keys are strictly equal.
 */
function shallowEqual(objA, objB) {
  if (is(objA, objB)) {
    return true;
  }

  if ((typeof objA === 'undefined' ? 'undefined' : _typeof(objA)) !== 'object' || objA === null || (typeof objB === 'undefined' ? 'undefined' : _typeof(objB)) !== 'object' || objB === null) {
    return false;
  }

  var keysA = Object.keys(objA);
  var keysB = Object.keys(objB);

  if (keysA.length !== keysB.length) {
    return false;
  }

  // Test for A's keys different from B.
  for (var i = 0; i < keysA.length; i++) {
    if (!hasOwnProperty.call(objB, keysA[i]) || !is(objA[keysA[i]], objB[keysA[i]])) {
      return false;
    }
  }

  return true;
}

exports.default = shallowEqual;
module.exports = exports['default'];

/***/ }),
/* 29 */
/***/ (function(module, exports, __webpack_require__) {

Object.defineProperty(exports, "__esModule", {
  value: true
});

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) { descriptor.writable = true; } Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) { defineProperties(Constructor.prototype, protoProps); } if (staticProps) { defineProperties(Constructor, staticProps); } return Constructor; }; }();

var _host = __webpack_require__(0);

var _host2 = _interopRequireDefault(_host);

var _native = __webpack_require__(11);

var _native2 = _interopRequireDefault(_native);

var _instance = __webpack_require__(1);

var _instance2 = _interopRequireDefault(_instance);

var _instantiateComponent = __webpack_require__(2);

var _instantiateComponent2 = _interopRequireDefault(_instantiateComponent);

var _getElementKeyName = __webpack_require__(13);

var _getElementKeyName2 = _interopRequireDefault(_getElementKeyName);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _possibleConstructorReturn(self, call) { if (!self) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return call && (typeof call === "object" || typeof call === "function") ? call : self; }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function, not " + typeof superClass); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } }); if (superClass) { Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass; } }

/**
 * Fragment Component
 */
var FragmentComponent = function (_NativeComponent) {
  _inherits(FragmentComponent, _NativeComponent);

  function FragmentComponent(element) {
    _classCallCheck(this, FragmentComponent);

    return _possibleConstructorReturn(this, (FragmentComponent.__proto__ || Object.getPrototypeOf(FragmentComponent)).call(this, element));
  }

  _createClass(FragmentComponent, [{
    key: 'mountComponent',
    value: function mountComponent(parent, parentInstance, context, childMounter) {
      // Parent native element
      this._parent = parent;
      this._parentInstance = parentInstance;
      this._context = context;
      this._mountID = _host2.default.mountID++;

      var instance = {
        _internal: this
      };
      this._instance = instance;

      var fragment = this.getNativeNode();
      var children = this._currentElement;

      // Process children
      this.mountChildren(children, context);

      if (childMounter) {
        childMounter(fragment, parent);
      } else {
        var isFragmentParent = Array.isArray(parent);
        for (var i = 0; i < fragment.length; i++) {
          var child = fragment[i];
          // When the parent is also a fragment
          if (isFragmentParent) {
            parent.push(child);
          } else {
            _host2.default.driver.appendChild(child, parent);
          }
        }
      }

      return instance;
    }
  }, {
    key: 'mountChildren',
    value: function mountChildren(children, context) {
      var _this2 = this;

      var renderedChildren = this._renderedChildren = {};
      var fragment = this.getNativeNode();

      var renderedChildrenImage = children.map(function (element, index) {
        var renderedChild = (0, _instantiateComponent2.default)(element);
        var name = (0, _getElementKeyName2.default)(renderedChildren, element, index);
        renderedChildren[name] = renderedChild;
        renderedChild._mountIndex = index;
        // Mount
        var mountImage = renderedChild.mountComponent(_this2._parent, _this2._instance, context, function (nativeNode) {
          if (Array.isArray(nativeNode)) {
            for (var i = 0; i < nativeNode.length; i++) {
              fragment.push(nativeNode[i]);
            }
          } else {
            fragment.push(nativeNode);
          }
        });
        return mountImage;
      });

      return renderedChildrenImage;
    }
  }, {
    key: 'unmountComponent',
    value: function unmountComponent(notRemoveChild) {
      var this$1 = this;

      if (this._nativeNode) {
        _instance2.default.remove(this._nativeNode);
        if (!notRemoveChild) {
          for (var i = 0; i < this._nativeNode.length; i++) {
            _host2.default.driver.removeChild(this$1._nativeNode[i]);
          }
        }
      }

      // Do not need remove child when their parent is removed
      this.unmountChildren(true);

      this._currentElement = null;
      this._nativeNode = null;
      this._parent = null;
      this._parentInstance = null;
      this._context = null;
      this._instance = null;
    }
  }, {
    key: 'updateComponent',
    value: function updateComponent(prevElement, nextElement, prevContext, nextContext) {
      // Replace current element
      this._currentElement = nextElement;
      this.updateChildren(this._currentElement, nextContext);
    }
  }, {
    key: 'getNativeNode',
    value: function getNativeNode() {
      if (this._nativeNode == null) {
        this._nativeNode = [];
      }

      return this._nativeNode;
    }
  }, {
    key: 'getPublicInstance',
    value: function getPublicInstance() {
      return this.getNativeNode();
    }
  }, {
    key: 'getName',
    value: function getName() {
      return 'fragment';
    }
  }]);

  return FragmentComponent;
}(_native2.default);

exports.default = FragmentComponent;
module.exports = exports['default'];

/***/ }),
/* 30 */
/***/ (function(module, exports, __webpack_require__) {

var _typeof2 = typeof Symbol === "function" && typeof Symbol.iterator === "symbol" ? function (obj) { return typeof obj; } : function (obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; };

Object.defineProperty(exports, "__esModule", {
  value: true
});

var _typeof = typeof Symbol === "function" && _typeof2(Symbol.iterator) === "symbol" ? function (obj) {
  return typeof obj === "undefined" ? "undefined" : _typeof2(obj);
} : function (obj) {
  return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj === "undefined" ? "undefined" : _typeof2(obj);
}; /**
    * Weex driver
    */

var _styleUnit = __webpack_require__(14);

var _elements = __webpack_require__(31);

var _elements2 = _interopRequireDefault(_elements);

function _interopRequireDefault(obj) {
  return obj && obj.__esModule ? obj : { default: obj };
}

var STYLE = 'style';
var ID = 'id';
var TEXT = 'text';
var CHILDREN = 'children';
var EVENT_PREFIX_REGEXP = /^on[A-Z]/;
var ARIA_PREFIX_REGEXP = /^aria-/;

var nodeMaps = {};
/* global __weex_document__ */
var document = (typeof __weex_document__ === 'undefined' ? 'undefined' : _typeof(__weex_document__)) === 'object' ? __weex_document__ : (typeof document === 'undefined' ? 'undefined' : _typeof(document)) === 'object' ? document : null;

var Driver = {
  deviceWidth: 750,
  viewportWidth: 750,

  getDeviceWidth: function getDeviceWidth() {
    return this.deviceWidth;
  },
  setDeviceWidth: function setDeviceWidth(width) {
    this.deviceWidth = width;
  },
  getViewportWidth: function getViewportWidth() {
    return this.viewportWidth;
  },
  setViewportWidth: function setViewportWidth(width) {
    this.viewportWidth = width;
  },
  getElementById: function getElementById(id) {
    return nodeMaps[id];
  },
  createBody: function createBody() {
    if (document.body) {
      return document.body;
    }

    var documentElement = document.documentElement;
    var body = document.createBody(Driver.bodyType, Driver.bodyProps);
    documentElement.appendChild(body);

    return body;
  },
  createComment: function createComment(content) {
    return document.createComment(content);
  },
  createEmpty: function createEmpty() {
    return this.createComment(' empty ');
  },
  createText: function createText(text) {
    return Driver.createElement({
      type: TEXT,
      props: {
        value: text
      }
    });
  },
  updateText: function updateText(node, content) {
    this.setAttribute(node, 'value', content);
  },
  createElement: function createElement(component) {
    var htmlElement = _elements2.default[component.type];
    if (htmlElement) {
      component = htmlElement.parse(component);
    }

    var props = component.props;
    var style = {};
    var originStyle = props[STYLE];
    for (var prop in originStyle) {
      style[prop] = (0, _styleUnit.convertUnit)(originStyle[prop], prop);
    }

    var node = document.createElement(component.type, {
      style: style
    });

    this.setNativeProps(node, props, true);

    return node;
  },
  appendChild: function appendChild(node, parent) {
    return parent.appendChild(node);
  },
  removeChild: function removeChild(node, parent) {
    parent = parent || node.parentNode;
    var id = node.attr && node.attr[ID];
    if (id != null) {
      nodeMaps[id] = null;
    }
    return parent.removeChild(node);
  },
  replaceChild: function replaceChild(newChild, oldChild, parent) {
    parent = parent || oldChild.parentNode;
    var previousSibling = oldChild.previousSibling;
    var nextSibling = oldChild.nextSibling;
    this.removeChild(oldChild, parent);

    if (previousSibling) {
      this.insertAfter(newChild, previousSibling, parent);
    } else if (nextSibling) {
      this.insertBefore(newChild, nextSibling, parent);
    } else {
      this.appendChild(newChild, parent);
    }
  },
  insertAfter: function insertAfter(node, after, parent) {
    parent = parent || after.parentNode;
    return parent.insertAfter(node, after);
  },
  insertBefore: function insertBefore(node, before, parent) {
    parent = parent || before.parentNode;
    return parent.insertBefore(node, before);
  },
  addEventListener: function addEventListener(node, eventName, eventHandler, props) {
    var params = props[eventName + 'EventParams'];
    return node.addEvent(eventName, eventHandler, params);
  },
  removeEventListener: function removeEventListener(node, eventName, eventHandler) {
    return node.removeEvent(eventName, eventHandler);
  },
  removeAllEventListeners: function removeAllEventListeners(node) {
    // Noop
  },
  removeAttribute: function removeAttribute(node, propKey, propValue) {
    if (propKey == ID) {
      nodeMaps[propValue] = null;
    }
    // Weex native will crash when pass null value
    return node.setAttr(propKey, undefined, false);
  },
  setAttribute: function setAttribute(node, propKey, propValue) {
    if (propKey == ID) {
      nodeMaps[propValue] = node;
    }

    // Weex only support `ariaLabel` format, convert `aria-label` format to camelcase
    if (ARIA_PREFIX_REGEXP.test(propKey)) {
      propKey = propKey.replace(/\-(\w)/, function (m, p1) {
        return p1.toUpperCase();
      });
    }

    return node.setAttr(propKey, propValue, false);
  },
  setStyles: function setStyles(node, styles) {
    // TODO if more then one style update, call setStyles will be better performance
    for (var key in styles) {
      var val = styles[key];
      val = (0, _styleUnit.convertUnit)(val, key);
      node.setStyle(key, val);
    }
  },
  beforeRender: function beforeRender() {
    // Turn off batched updates
    document.open();

    // Init rem unit
    (0, _styleUnit.setRem)(this.getDeviceWidth() / this.getViewportWidth());
  },
  afterRender: function afterRender() {
    if (document.listener && document.listener.createFinish) {
      document.listener.createFinish();
    }

    // Turn on batched updates
    document.close();
  },
  setNativeProps: function setNativeProps(node, props, skipSetStyles) {
    var this$1 = this;

    for (var prop in props) {
      var value = props[prop];
      if (prop === CHILDREN) {
        continue;
      }

      if (value != null) {
        if (prop === STYLE) {
          if (skipSetStyles) {
            continue;
          }
          this$1.setStyles(node, value);
        } else if (EVENT_PREFIX_REGEXP.test(prop)) {
          var eventName = prop.slice(2).toLowerCase();
          this$1.addEventListener(node, eventName, value, props);
        } else {
          this$1.setAttribute(node, prop, value);
        }
      }
    }
  }
};

exports.default = Driver;
module.exports = exports['default'];

/***/ }),
/* 31 */
/***/ (function(module, exports, __webpack_require__) {

Object.defineProperty(exports, "__esModule", {
  value: true
});

var _img = __webpack_require__(32);

var _img2 = _interopRequireDefault(_img);

var _video = __webpack_require__(33);

var _video2 = _interopRequireDefault(_video);

var _textarea = __webpack_require__(34);

var _textarea2 = _interopRequireDefault(_textarea);

var _span = __webpack_require__(35);

var _span2 = _interopRequireDefault(_span);

var _p = __webpack_require__(36);

var _p2 = _interopRequireDefault(_p);

var _button = __webpack_require__(37);

var _button2 = _interopRequireDefault(_button);

var _heading = __webpack_require__(38);

var _heading2 = _interopRequireDefault(_heading);

var _block = __webpack_require__(39);

var _block2 = _interopRequireDefault(_block);

function _interopRequireDefault(obj) {
  return obj && obj.__esModule ? obj : { default: obj };
}

exports.default = {
  span: _span2.default,
  p: _p2.default,
  img: _img2.default,
  button: _button2.default,
  video: _video2.default,
  textarea: _textarea2.default,
  h1: _heading2.default,
  h2: _heading2.default,
  h3: _heading2.default,
  h4: _heading2.default,
  h5: _heading2.default,
  h6: _heading2.default,
  nav: _block2.default,
  article: _block2.default,
  section: _block2.default,
  // Conflict with weex header tag
  // header: block,
  footer: _block2.default,
  aside: _block2.default,
  main: _block2.default
};
module.exports = exports['default'];

/***/ }),
/* 32 */
/***/ (function(module, exports, __webpack_require__) {

Object.defineProperty(exports, "__esModule", {
  value: true
});

var _parseProps = __webpack_require__(15);

exports.default = {

  /**
   * parse w3c attrs to weex module attrs
   *
   * @param {Object} w3c component data
   * @return {Object} weex component data
   */
  parse: function parse(component) {
    var props = component.props;

    component.type = 'image';

    // modify props
    component.props = (0, _parseProps.transformPropsAttrsToStyle)(props, ['width', 'height']);

    return component;
  }
};
module.exports = exports['default'];

/***/ }),
/* 33 */
/***/ (function(module, exports, __webpack_require__) {

Object.defineProperty(exports, "__esModule", {
  value: true
});

var _parseProps = __webpack_require__(15);

exports.default = {

  /**
   * parse w3c attrs to weex module attrs
   *
   * @param {Object} w3c component data
   * @return {Object} weex component data
   */
  parse: function parse(component) {
    var props = component.props;

    // modify props

    component.props = (0, _parseProps.transformPropsAttrsToStyle)(props, ['width', 'height']);

    return component;
  }
};
module.exports = exports['default'];

/***/ }),
/* 34 */
/***/ (function(module, exports, __webpack_require__) {

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.default = {

  /**
   * parse w3c attrs to weex module attrs
   *
   * @param {Object} w3c component data
   * @return {Object} weex component data
   */
  parse: function parse(component) {
    var props = component.props;

    if (typeof props.children === 'string' && !props.value) {
      props.value = props.children;
      props.children = null;
    }

    return component;
  }
};
module.exports = exports['default'];

/***/ }),
/* 35 */
/***/ (function(module, exports, __webpack_require__) {

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.default = {
  parse: function parse(component) {
    var props = component.props;

    component.type = 'text';

    if (typeof props.children === 'string' && !props.value) {
      props.value = props.children;
      props.children = null;
    }

    return component;
  }
};
module.exports = exports['default'];

/***/ }),
/* 36 */
/***/ (function(module, exports, __webpack_require__) {

var _typeof2 = typeof Symbol === "function" && typeof Symbol.iterator === "symbol" ? function (obj) { return typeof obj; } : function (obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; };

Object.defineProperty(exports, "__esModule", {
  value: true
});

var _typeof = typeof Symbol === "function" && _typeof2(Symbol.iterator) === "symbol" ? function (obj) {
  return typeof obj === "undefined" ? "undefined" : _typeof2(obj);
} : function (obj) {
  return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj === "undefined" ? "undefined" : _typeof2(obj);
};

var _extends = Object.assign || function (target) {
  var arguments$1 = arguments;

  for (var i = 1; i < arguments.length; i++) {
    var source = arguments$1[i];for (var key in source) {
      if (Object.prototype.hasOwnProperty.call(source, key)) {
        target[key] = source[key];
      }
    }
  }return target;
};

var BASE_FONT_SIZE = 28;

var defaultParagraphStyle = {
  fontSize: BASE_FONT_SIZE,
  marginTop: BASE_FONT_SIZE,
  marginBottom: BASE_FONT_SIZE
};

var TypographyElements = {
  u: {
    textDecoration: 'underline'
  },
  s: {
    textDecoration: 'line-through'
  },
  i: {
    fontStyle: 'italic'
  },
  b: {
    fontWeight: 'bold'
  },
  del: {
    textDecoration: 'line-through'
  },
  em: {
    fontStyle: 'italic'
  },
  strong: {
    fontWeight: 'bold'
  },
  big: {
    fontSize: BASE_FONT_SIZE * 1.2
  },
  small: {
    fontSize: BASE_FONT_SIZE * 0.8
  }
};

function transformString(string) {
  return {
    type: 'span',
    attr: {
      value: string
    }
  };
}

function transformChild(child) {
  var type = child.type;
  var props = child.props;
  var style = props.style;
  var nestedChildren = props.children;
  // Alias img tag
  if (type === 'img') {
    type = 'image';
  }

  // Transfrom to span
  if (TypographyElements[type]) {
    style = _extends({}, TypographyElements[type], style);
    type = 'span';
  }

  props.style = null;
  props.children = null;

  var element = {
    type: type,
    style: style,
    attr: props || {}
  };

  if (nestedChildren) {
    if (type === 'span' && typeof nestedChildren === 'string') {
      element.attr.value = nestedChildren;
    } else {
      element.children = transformChildren(nestedChildren);
    }
  }

  return element;
}

function transformChildren(children) {
  var elements = [];
  if (!Array.isArray(children)) {
    children = [children];
  }

  for (var i = 0; i < children.length; i++) {
    var child = children[i];
    if (typeof child === 'string') {
      elements.push(transformString(child));
    } else if ((typeof child === 'undefined' ? 'undefined' : _typeof(child)) === 'object') {
      elements.push(transformChild(child));
    }
  }

  return elements;
}

exports.default = {
  parse: function parse(component) {
    var props = component.props;

    var children = props.children;

    component.type = 'richtext';

    props.style = _extends({}, defaultParagraphStyle, props.style);

    props.value = transformChildren(children);
    props.children = null;

    return component;
  }
};
module.exports = exports['default'];

/***/ }),
/* 37 */
/***/ (function(module, exports, __webpack_require__) {

Object.defineProperty(exports, "__esModule", {
  value: true
});

var _extends = Object.assign || function (target) {
  var arguments$1 = arguments;

  for (var i = 1; i < arguments.length; i++) {
    var source = arguments$1[i];for (var key in source) {
      if (Object.prototype.hasOwnProperty.call(source, key)) {
        target[key] = source[key];
      }
    }
  }return target;
};

/**
 * weex button
 *
 * props: disabled, style
 */

exports.default = {
  parse: function parse(component) {
    var props = component.props;

    component.type = 'text';

    var style = props.style,
        disabled = props.disabled,
        children = props.children;

    var textStyle = _extends({
      textAlign: 'center',
      fontSize: 22,
      paddingTop: 4,
      paddingRight: 12,
      paddingBottom: 6,
      paddingLeft: 12,
      borderWidth: 4,
      borderStyle: 'solid',
      borderColor: '#000000',
      backgroudColor: '#c0c0c0'
    }, style);

    if (disabled) {
      props.onClick = null;
      textStyle = _extends({}, textStyle, {
        color: '#7f7f7f',
        borderColor: '#7f7f7f'
      });
    }

    if (typeof children === 'string') {
      props.value = children;
      props.children = null;
    }

    return component;
  }
};
module.exports = exports['default'];

/***/ }),
/* 38 */
/***/ (function(module, exports, __webpack_require__) {

Object.defineProperty(exports, "__esModule", {
  value: true
});

var _extends = Object.assign || function (target) {
  var arguments$1 = arguments;

  for (var i = 1; i < arguments.length; i++) {
    var source = arguments$1[i];for (var key in source) {
      if (Object.prototype.hasOwnProperty.call(source, key)) {
        target[key] = source[key];
      }
    }
  }return target;
};

var BASE_FONT_SIZE = 28;

function generateHeadingStyle(baseFontSize, fontMultiplier, marginMultiplier) {
  return {
    fontSize: baseFontSize * fontMultiplier,
    marginTop: baseFontSize * fontMultiplier * marginMultiplier,
    marginBottom: baseFontSize * fontMultiplier * marginMultiplier,
    fontWeight: 'bold'
  };
}

var HeadingElements = {
  h1: generateHeadingStyle(BASE_FONT_SIZE, 2, 0.67),
  h2: generateHeadingStyle(BASE_FONT_SIZE, 1.5, 0.83),
  h3: generateHeadingStyle(BASE_FONT_SIZE, 1.17, 1),
  h4: generateHeadingStyle(BASE_FONT_SIZE, 1, 1.33),
  h5: generateHeadingStyle(BASE_FONT_SIZE, 0.83, 1.67),
  h6: generateHeadingStyle(BASE_FONT_SIZE, 0.67, 2.33)
};

exports.default = {
  parse: function parse(component) {
    var type = component.type,
        props = component.props;

    component.type = 'text';
    props.style = _extends({}, HeadingElements[type] || HeadingElements.h6, props.style);

    if (typeof props.children === 'string' && !props.value) {
      props.value = props.children;
      props.children = null;
    }

    return component;
  }
};
module.exports = exports['default'];

/***/ }),
/* 39 */
/***/ (function(module, exports, __webpack_require__) {

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.default = {

  /**
   * parse w3c attrs to weex module attrs
   *
   * @param {Object} w3c component data
   * @return {Object} weex component data
   */
  parse: function parse(component) {
    component.type = 'div';
    return component;
  }
};
module.exports = exports['default'];

/***/ }),
/* 40 */
/***/ (function(module, exports, __webpack_require__) {

Object.defineProperty(exports, "__esModule", {
  value: true
});

var _styleUnit = __webpack_require__(14);

var _flexbox = __webpack_require__(41);

var _flexbox2 = _interopRequireDefault(_flexbox);

function _interopRequireDefault(obj) {
  return obj && obj.__esModule ? obj : { default: obj };
}

/**
 * Web Browser driver
 **/

/* global DEVICE_WIDTH, VIEWPORT_WIDTH */

var DANGEROUSLY_SET_INNER_HTML = 'dangerouslySetInnerHTML';
var CLASS_NAME = 'className';
var CLASS = 'class';
var STYLE = 'style';
var CHILDREN = 'children';
var EVENT_PREFIX_REGEXP = /on[A-Z]/;

var ADD_EVENT = 'addEvent';
var REMOVE_EVENT = 'removeEvent';

var Driver = {

  deviceWidth: typeof DEVICE_WIDTH !== 'undefined' && DEVICE_WIDTH || null,
  viewportWidth: typeof VIEWPORT_WIDTH !== 'undefined' && VIEWPORT_WIDTH || 750,
  eventRegistry: {},

  getDeviceWidth: function getDeviceWidth() {
    return this.deviceWidth || document.documentElement.clientWidth;
  },
  setDeviceWidth: function setDeviceWidth(width) {
    this.deviceWidth = width;
  },
  getViewportWidth: function getViewportWidth() {
    return this.viewportWidth;
  },
  setViewportWidth: function setViewportWidth(width) {
    this.viewportWidth = width;
  },
  getElementById: function getElementById(id) {
    return document.getElementById(id);
  },
  createBody: function createBody() {
    return document.body;
  },
  createComment: function createComment(content) {
    return document.createComment(content);
  },
  createEmpty: function createEmpty() {
    return this.createComment(' empty ');
  },
  createText: function createText(text) {
    return document.createTextNode(text);
  },
  updateText: function updateText(node, text) {
    var textContentAttr = 'textContent' in document ? 'textContent' : 'nodeValue';
    node[textContentAttr] = text;
  },
  createElement: function createElement(component) {
    var node = document.createElement(component.type);
    var props = component.props;

    this.setNativeProps(node, props);

    return node;
  },
  appendChild: function appendChild(node, parent) {
    return parent.appendChild(node);
  },
  removeChild: function removeChild(node, parent) {
    parent = parent || node.parentNode;
    // Maybe has been removed when remove child
    if (parent) {
      parent.removeChild(node);
    }
  },
  replaceChild: function replaceChild(newChild, oldChild, parent) {
    parent = parent || oldChild.parentNode;
    parent.replaceChild(newChild, oldChild);
  },
  insertAfter: function insertAfter(node, after, parent) {
    parent = parent || after.parentNode;
    var nextSibling = after.nextSibling;
    if (nextSibling) {
      parent.insertBefore(node, nextSibling);
    } else {
      parent.appendChild(node);
    }
  },
  insertBefore: function insertBefore(node, before, parent) {
    parent = parent || before.parentNode;
    parent.insertBefore(node, before);
  },
  addEventListener: function addEventListener(node, eventName, eventHandler, props) {
    if (this.eventRegistry[eventName]) {
      return this.eventRegistry[eventName](ADD_EVENT, node, eventName, eventHandler, props);
    } else {
      return node.addEventListener(eventName, eventHandler);
    }
  },
  removeEventListener: function removeEventListener(node, eventName, eventHandler, props) {
    if (this.eventRegistry[eventName]) {
      return this.eventRegistry[eventName](REMOVE_EVENT, node, eventName, eventHandler, props);
    } else {
      return node.removeEventListener(eventName, eventHandler);
    }
  },
  removeAllEventListeners: function removeAllEventListeners(node) {
    // noop
  },
  removeAttribute: function removeAttribute(node, propKey) {
    if (propKey === DANGEROUSLY_SET_INNER_HTML) {
      return node.innerHTML = null;
    }

    if (propKey === CLASS_NAME) {
      propKey = CLASS;
    }

    if (propKey in node) {
      try {
        // Some node property is readonly when in strict mode
        node[propKey] = null;
      } catch (e) {}
    }

    node.removeAttribute(propKey);
  },
  setAttribute: function setAttribute(node, propKey, propValue) {
    if (propKey === DANGEROUSLY_SET_INNER_HTML) {
      return node.innerHTML = propValue.__html;
    }

    if (propKey === CLASS_NAME) {
      propKey = CLASS;
    }

    if (propKey in node) {
      try {
        // Some node property is readonly when in strict mode
        node[propKey] = propValue;
      } catch (e) {
        node.setAttribute(propKey, propValue);
      }
    } else {
      node.setAttribute(propKey, propValue);
    }
  },
  setStyles: function setStyles(node, styles) {
    var tranformedStyles = {};

    for (var prop in styles) {
      var val = styles[prop];
      if (_flexbox2.default.isFlexProp(prop)) {
        _flexbox2.default[prop](val, tranformedStyles);
      } else {
        tranformedStyles[prop] = (0, _styleUnit.convertUnit)(val, prop);
      }
    }

    for (var _prop in tranformedStyles) {
      var transformValue = tranformedStyles[_prop];
      // hack handle compatibility issue
      if (Array.isArray(transformValue)) {
        for (var i = 0; i < transformValue.length; i++) {
          node.style[_prop] = transformValue[i];
        }
      } else {
        node.style[_prop] = transformValue;
      }
    }
  },
  beforeRender: function beforeRender() {
    // Init rem unit
    (0, _styleUnit.setRem)(this.getDeviceWidth() / this.getViewportWidth());
  },
  setNativeProps: function setNativeProps(node, props) {
    var this$1 = this;

    for (var prop in props) {
      var value = props[prop];
      if (prop === CHILDREN) {
        continue;
      }

      if (value != null) {
        if (prop === STYLE) {
          this$1.setStyles(node, value);
        } else if (EVENT_PREFIX_REGEXP.test(prop)) {
          var eventName = prop.slice(2).toLowerCase();
          this$1.addEventListener(node, eventName, value);
        } else {
          this$1.setAttribute(node, prop, value);
        }
      }
    }
  }
};

exports.default = Driver;
module.exports = exports['default'];

/***/ }),
/* 41 */
/***/ (function(module, exports, __webpack_require__) {

Object.defineProperty(exports, "__esModule", {
  value: true
});
var BOX_ALIGN = {
  stretch: 'stretch',
  'flex-start': 'start',
  'flex-end': 'end',
  center: 'center'
};

var BOX_ORIENT = {
  row: 'horizontal',
  column: 'vertical'
};

var BOX_PACK = {
  'flex-start': 'start',
  'flex-end': 'end',
  center: 'center',
  'space-between': 'justify',
  'space-around': 'justify' // Just same as `space-between`
};

var FLEX_PROPS = {
  display: true,
  flex: true,
  alignItems: true,
  alignSelf: true,
  flexDirection: true,
  justifyContent: true,
  flexWrap: true
};

var Flexbox = {
  isFlexProp: function isFlexProp(prop) {
    return FLEX_PROPS[prop];
  },
  display: function display(value) {
    var style = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : {};

    if (value === 'flex') {
      style.display = ['-webkit-box', '-webkit-flex', 'flex'];
    } else {
      style.display = value;
    }

    return style;
  },
  flex: function flex(value) {
    var style = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : {};

    style.webkitBoxFlex = value;
    style.webkitFlex = value;
    style.flex = value;
    return style;
  },
  flexWrap: function flexWrap(value) {
    var style = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : {};

    style.webkitFlexWrap = value;
    style.flexWrap = value;
    return style;
  },
  alignItems: function alignItems(value) {
    var style = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : {};

    style.webkitBoxAlign = BOX_ALIGN[value];
    style.webkitAlignItems = value;
    style.alignItems = value;
    return style;
  },
  alignSelf: function alignSelf(value) {
    var style = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : {};

    style.webkitAlignSelf = value;
    style.alignSelf = value;
    return style;
  },
  flexDirection: function flexDirection(value) {
    var style = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : {};

    style.webkitBoxOrient = BOX_ORIENT[value];
    style.webkitFlexDirection = value;
    style.flexDirection = value;
    return style;
  },
  justifyContent: function justifyContent(value) {
    var style = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : {};

    style.webkitBoxPack = BOX_PACK[value];
    style.webkitJustifyContent = value;
    style.justifyContent = value;
    return style;
  }
};

exports.default = Flexbox;
module.exports = exports['default'];

/***/ }),
/* 42 */
/***/ (function(module, exports, __webpack_require__) {

Object.defineProperty(exports, "__esModule", {
  value: true
});

var _render = __webpack_require__(10);

var _render2 = _interopRequireDefault(_render);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _toConsumableArray(arr) { if (Array.isArray(arr)) { for (var i = 0, arr2 = Array(arr.length); i < arr.length; i++) { arr2[i] = arr[i]; } return arr2; } else { return Array.from(arr); } }

function hydrate(element, container, options, callback) {
  // Handle server rendered element
  if (container.childNodes) {
    // Clone childNodes, Because removeChild will causing change in childNodes length
    var childNodes = [].concat(_toConsumableArray(container.childNodes));

    for (var i = 0; i < childNodes.length; i++) {
      var rootChildNode = childNodes[i];
      container.removeChild(rootChildNode);
    }
  }

  return (0, _render2.default)(element, container, options, callback);
}

exports.default = hydrate;
module.exports = exports['default'];

/***/ }),
/* 43 */
/***/ (function(module, exports, __webpack_require__) {

Object.defineProperty(exports, "__esModule", {
  value: true
});

var _instance = __webpack_require__(1);

var _instance2 = _interopRequireDefault(_instance);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function findComponentInstance(node) {
  if (node == null) {
    return null;
  }
  return _instance2.default.get(node);
}

exports.default = findComponentInstance;
module.exports = exports['default'];

/***/ }),
/* 44 */
/***/ (function(module, exports, __webpack_require__) {

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.default = setNativeProps;

var _host = __webpack_require__(0);

var _host2 = _interopRequireDefault(_host);

var _findDOMNode = __webpack_require__(16);

var _findDOMNode2 = _interopRequireDefault(_findDOMNode);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function setNativeProps(node, props) {
  node = (0, _findDOMNode2.default)(node);
  _host2.default.driver.setNativeProps(node, props);
}
module.exports = exports['default'];

/***/ }),
/* 45 */
/***/ (function(module, exports, __webpack_require__) {

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.default = '0.4.20';
module.exports = exports['default'];

/***/ }),
/* 46 */
/***/ (function(module, exports, __webpack_require__) {

Object.defineProperty(exports, "__esModule", {
  value: true
});

var _flattenChildren = __webpack_require__(7);

var _flattenChildren2 = _interopRequireDefault(_flattenChildren);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function convertChildrenToArray(children) {
  // flatten children
  children = (0, _flattenChildren2.default)(children, []);
  return Array.isArray(children) ? children : [].concat(children);
}

var Children = {
  map: function map(children, fn, ctx) {
    if (children == null) { return null; }
    children = convertChildrenToArray(children);
    return children.map(function (child, index) {
      return fn.call(ctx, child, index);
    });
  },
  forEach: function forEach(children, fn, ctx) {
    if (children == null) { return null; }
    children = convertChildrenToArray(children);
    children.forEach(function (child, index) {
      return fn.call(ctx, child, index);
    });
  },
  count: function count(children) {
    if (children == null) { return 0; }
    return convertChildrenToArray(children).length;
  },
  only: function only(children) {
    // `only` receive rax element child
    // null value is not acceptable
    children = Children.toArray(children);
    if (children.length !== 1) { throw new Error('Children.only: expected to receive a single element child.'); }
    return children[0];
  },
  toArray: function toArray(children) {
    if (children == null) { return []; }
    // `toArray` filter null value
    return convertChildrenToArray(children).filter(function (child) {
      return child !== null;
    });
  }
};

exports.default = Children;
module.exports = exports['default'];

/***/ })
/******/ ]);};


/***/ }),
/* 6 */
/*!*********************************************************!*\
  !*** ./packages/runtime-shared/dist/shared.function.js ***!
  \*********************************************************/
/*! dynamic exports provided */
/*! all exports used */
/*! ModuleConcatenation bailout: Module is not an ECMAScript module */
/***/ (function(module, exports) {

module.exports = function() {
  return /******/ (function(modules) { // webpackBootstrap
/******/  // The module cache
/******/  var installedModules = {};
/******/
/******/  // The require function
/******/  function __webpack_require__(moduleId) {
/******/
/******/    // Check if module is in cache
/******/    if(installedModules[moduleId]) {
/******/      return installedModules[moduleId].exports;
/******/    }
/******/    // Create a new module (and put it into the cache)
/******/    var module = installedModules[moduleId] = {
/******/      i: moduleId,
/******/      l: false,
/******/      exports: {}
/******/    };
/******/
/******/    // Execute the module function
/******/    modules[moduleId].call(module.exports, module, module.exports, __webpack_require__);
/******/
/******/    // Flag the module as loaded
/******/    module.l = true;
/******/
/******/    // Return the exports of the module
/******/    return module.exports;
/******/  }
/******/
/******/
/******/  // expose the modules object (__webpack_modules__)
/******/  __webpack_require__.m = modules;
/******/
/******/  // expose the module cache
/******/  __webpack_require__.c = installedModules;
/******/
/******/  // define getter function for harmony exports
/******/  __webpack_require__.d = function(exports, name, getter) {
/******/    if(!__webpack_require__.o(exports, name)) {
/******/      Object.defineProperty(exports, name, {
/******/        configurable: false,
/******/        enumerable: true,
/******/        get: getter
/******/      });
/******/    }
/******/  };
/******/
/******/  // getDefaultExport function for compatibility with non-harmony modules
/******/  __webpack_require__.n = function(module) {
/******/    var getter = module && module.__esModule ?
/******/      function getDefault() { return module['default']; } :
/******/      function getModuleExports() { return module; };
/******/    __webpack_require__.d(getter, 'a', getter);
/******/    return getter;
/******/  };
/******/
/******/  // Object.prototype.hasOwnProperty.call
/******/  __webpack_require__.o = function(object, property) { return Object.prototype.hasOwnProperty.call(object, property); };
/******/
/******/  // __webpack_public_path__
/******/  __webpack_require__.p = "";
/******/
/******/  // Load entry module and return exports
/******/  return __webpack_require__(__webpack_require__.s = 2);
/******/ })
/************************************************************************/
/******/ ([
/* 0 */
/***/ (function(module, exports, __webpack_require__) {

var _typeof = typeof Symbol === "function" && typeof Symbol.iterator === "symbol" ? function (obj) { return typeof obj; } : function (obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; };

/* eslint no-extend-native: "off", new-cap: "off" */

var defineProperties = Object.defineProperties,
    defineProperty = Object.defineProperty,
    SymbolPolyfill,
    HiddenSymbol,
    globalSymbols = Object.create(null);

function isSymbol(x) {
  if (!x) { return false; }
  if ((typeof x === 'undefined' ? 'undefined' : _typeof(x)) === 'symbol') { return true; }
  if (!x.constructor) { return false; }
  if (x.constructor.name !== 'Symbol') { return false; }
  return x[x.constructor.toStringTag] === 'Symbol';
}

function validateSymbol(value) {
  if (!isSymbol(value)) { throw new TypeError(value + ' is not a symbol'); }
  return value;
}

var generateName = function () {
  var created = Object.create(null);
  return function (desc) {
    var postfix = 0,
        name;
    while (created[desc + (postfix || '')]) {
      ++postfix;
    }desc += postfix || '';
    created[desc] = true;
    name = '@@' + desc;
    return name;
  };
}();

// Internal constructor (not one exposed) for creating Symbol instances.
// This one is used to ensure that `someSymbol instanceof Symbol` always return false
HiddenSymbol = function _Symbol(description) {
  if (this instanceof HiddenSymbol) { throw new TypeError('Symbol is not a constructor'); }
  return SymbolPolyfill(description);
};

// Exposed `Symbol` constructor
// (returns instances of HiddenSymbol)
module.exports = SymbolPolyfill = function _Symbol2(description) {
  var symbol;
  if (this instanceof _Symbol2) { throw new TypeError('Symbol is not a constructor'); }
  symbol = Object.create(HiddenSymbol.prototype);
  description = description === undefined ? '' : String(description);
  return defineProperties(symbol, {
    __description__: { value: description },
    __name__: { value: generateName(description) }
  });
};
defineProperties(SymbolPolyfill, {
  for: { value: function value(key) {
      if (globalSymbols[key]) { return globalSymbols[key]; }
      return globalSymbols[key] = SymbolPolyfill(String(key));
    } },
  keyFor: { value: function value(s) {
      var key;
      validateSymbol(s);
      for (key in globalSymbols) {
        if (globalSymbols[key] === s) { return key; }
      }
    } },

  // To ensure proper interoperability with other native functions (e.g. Array.from)
  // fallback to eventual native implementation of given symbol
  hasInstance: { value: SymbolPolyfill('hasInstance') },
  isConcatSpreadable: { value: SymbolPolyfill('isConcatSpreadable') },
  iterator: { value: SymbolPolyfill('iterator') },
  match: { value: SymbolPolyfill('match') },
  replace: { value: SymbolPolyfill('replace') },
  search: { value: SymbolPolyfill('search') },
  species: { value: SymbolPolyfill('species') },
  split: { value: SymbolPolyfill('split') },
  toPrimitive: { value: SymbolPolyfill('toPrimitive') },
  toStringTag: { value: SymbolPolyfill('toStringTag') },
  unscopables: { value: SymbolPolyfill('unscopables') }
});

// Internal tweaks for real symbol producer
defineProperties(HiddenSymbol.prototype, {
  constructor: { value: SymbolPolyfill },
  toString: { value: function value() {
      return this.__name__;
    } }
});

// Proper implementation of methods exposed on Symbol.prototype
// They won't be accessible on produced symbol instances as they derive from HiddenSymbol.prototype
defineProperties(SymbolPolyfill.prototype, {
  toString: { value: function value() {
      return 'Symbol (' + validateSymbol(this).__description__ + ')';
    } },
  valueOf: { value: function value() {
      return validateSymbol(this);
    } }
});

defineProperty(SymbolPolyfill.prototype, SymbolPolyfill.toPrimitive, {
  value: function value() {
    var symbol = validateSymbol(this);
    if ((typeof symbol === 'undefined' ? 'undefined' : _typeof(symbol)) === 'symbol') { return symbol; }
    return symbol.toString();
  }
});
defineProperty(SymbolPolyfill.prototype, SymbolPolyfill.toStringTag, { value: 'Symbol' });

// Proper implementaton of toPrimitive and toStringTag for returned symbol instances
defineProperty(HiddenSymbol.prototype, SymbolPolyfill.toStringTag, { value: SymbolPolyfill.prototype[SymbolPolyfill.toStringTag] });

// Note: It's important to define `toPrimitive` as last one, as some implementations
// implement `toPrimitive` natively without implementing `toStringTag` (or other specified symbols)
// And that may invoke error in definition flow:
// See: https://github.com/medikoo/es6-symbol/issues/13#issuecomment-164146149
defineProperty(HiddenSymbol.prototype, SymbolPolyfill.toPrimitive, { value: SymbolPolyfill.prototype[SymbolPolyfill.toPrimitive] });

/***/ }),
/* 1 */
/***/ (function(module, exports, __webpack_require__) {

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) { descriptor.writable = true; } Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) { defineProperties(Constructor.prototype, protoProps); } if (staticProps) { defineProperties(Constructor, staticProps); } return Constructor; }; }(); // https://github.com/WebReflection/url-search-params


var _symbol = __webpack_require__(0);

var _symbol2 = _interopRequireDefault(_symbol);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

var find = /[!'\(\)~]|%20|%00/g;
var plus = /\+/g;
var replace = {
  '!': '%21',
  "'": '%27',
  '(': '%28',
  ')': '%29',
  '~': '%7E',
  '%20': '+',
  '%00': '\x00'
};
var replacer = function replacer(match) {
  return replace[match];
};
var secret = '__URLSearchParams__';

function encode(str) {
  return encodeURIComponent(str).replace(find, replacer);
}

function decode(str) {
  return decodeURIComponent(str.replace(plus, ' '));
}

var URLSearchParams = function () {
  function URLSearchParams(query) {
    var this$1 = this;

    _classCallCheck(this, URLSearchParams);

    this[secret] = Object.create(null);
    if (!query) { return; }
    if (query.charAt(0) === '?') {
      query = query.slice(1);
    }
    for (var index, value, pairs = (query || '').split('&'), i = 0, length = pairs.length; i < length; i++) {
      value = pairs[i];
      index = value.indexOf('=');
      if (-1 < index) {
        this$1.append(decode(value.slice(0, index)), decode(value.slice(index + 1)));
      } else if (value.length) {
        this$1.append(decode(value), '');
      }
    }
  }

  _createClass(URLSearchParams, [{
    key: 'append',
    value: function append(name, value) {
      var dict = this[secret];
      if (name in dict) {
        dict[name].push('' + value);
      } else {
        dict[name] = ['' + value];
      }
    }
  }, {
    key: 'delete',
    value: function _delete(name) {
      delete this[secret][name];
    }
  }, {
    key: 'get',
    value: function get(name) {
      var dict = this[secret];
      return name in dict ? dict[name][0] : null;
    }
  }, {
    key: 'getAll',
    value: function getAll(name) {
      var dict = this[secret];
      return name in dict ? dict[name].slice(0) : [];
    }
  }, {
    key: 'has',
    value: function has(name) {
      return name in this[secret];
    }
  }, {
    key: 'set',
    value: function set(name, value) {
      this[secret][name] = ['' + value];
    }
  }, {
    key: 'forEach',
    value: function forEach(callback, thisArg) {
      var dict = this[secret];
      Object.getOwnPropertyNames(dict).forEach(function (name) {
        dict[name].forEach(function (value) {
          callback.call(thisArg, value, name, this);
        }, this);
      }, this);
    }
  }, {
    key: 'keys',
    value: function keys() {
      var items = [];
      this.forEach(function (value, name) {
        items.push(name);
      });
      var iterator = {
        next: function next() {
          var value = items.shift();
          return { done: value === undefined, value: value };
        }
      };

      iterator[_symbol2.default.iterator] = function () {
        return iterator;
      };

      return iterator;
    }
  }, {
    key: 'values',
    value: function values() {
      var items = [];
      this.forEach(function (value) {
        items.push(value);
      });
      var iterator = {
        next: function next() {
          var value = items.shift();
          return { done: value === undefined, value: value };
        }
      };

      iterator[_symbol2.default.iterator] = function () {
        return iterator;
      };

      return iterator;
    }
  }, {
    key: 'entries',
    value: function entries() {
      var items = [];
      this.forEach(function (value, name) {
        items.push([name, value]);
      });
      var iterator = {
        next: function next() {
          var value = items.shift();
          return { done: value === undefined, value: value };
        }
      };

      iterator[_symbol2.default.iterator] = function () {
        return iterator;
      };

      return iterator;
    }
  }, {
    key: 'toString',
    value: function toString() {
      var dict = this[secret],
          query = [],
          i,
          key,
          name,
          value;
      for (key in dict) {
        name = encode(key);
        for (i = 0, value = dict[key]; i < value.length; i++) {
          query.push(name + '=' + encode(value[i]));
        }
      }
      return query.join('&');
    }
  }]);

  return URLSearchParams;
}();

URLSearchParams.prototype[_symbol2.default.iterator] = URLSearchParams.prototype.entries;

module.exports = URLSearchParams;

/***/ }),
/* 2 */
/***/ (function(module, exports, __webpack_require__) {

module.exports = {
  get Promise() {
    return __webpack_require__(3);
  },
  get Symbol() {
    return __webpack_require__(0);
  },
  get Map() {
    return __webpack_require__(4);
  },
  get Set() {
    return __webpack_require__(5);
  },
  get WeakMap() {
    return __webpack_require__(6);
  },
  get WeakSet() {
    return __webpack_require__(7);
  },
  get FontFace() {
    return __webpack_require__(8);
  },
  get URL() {
    return __webpack_require__(9);
  },
  get URLSearchParams() {
    return __webpack_require__(1);
  },
  get matchMedia() {
    return __webpack_require__(10);
  }
};

/***/ }),
/* 3 */
/***/ (function(module, exports, __webpack_require__) {

var _typeof = typeof Symbol === "function" && typeof Symbol.iterator === "symbol" ? function (obj) { return typeof obj; } : function (obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; };

/* eslint no-extend-native: "off" */

function noop() {}

// Use polyfill for setImmediate for performance gains
var asap = typeof setImmediate === 'function' && setImmediate || function (fn) {
  if (typeof setTimeout === 'function') {
    setTimeout(fn, 0);
  } else {
    fn();
  }
};

var onUnhandledRejection = function onUnhandledRejection(err) {
  if (typeof console !== 'undefined' && console) {
    console.log('Possible Unhandled Promise Rejection:', err); // eslint-disable-line no-console
  }
};

// Polyfill for Function.prototype.bind
function bind(fn, thisArg) {
  return function () {
    fn.apply(thisArg, arguments);
  };
}

function Promise(fn) {
  if (_typeof(this) !== 'object') { throw new TypeError('Promises must be constructed via new'); }
  if (typeof fn !== 'function') { throw new TypeError('Promise resolver is not a function'); }
  this._state = 0;
  this._handled = false;
  this._value = undefined;
  this._deferreds = [];

  doResolve(fn, this);
}

function handle(self, deferred) {
  while (self._state === 3) {
    self = self._value;
  }
  if (self._state === 0) {
    self._deferreds.push(deferred);
    return;
  }
  self._handled = true;
  asap(function () {
    var cb = self._state === 1 ? deferred.onFulfilled : deferred.onRejected;
    if (cb === null) {
      (self._state === 1 ? resolve : reject)(deferred.promise, self._value);
      return;
    }
    var ret;
    try {
      ret = cb(self._value);
    } catch (e) {
      reject(deferred.promise, e);
      return;
    }
    resolve(deferred.promise, ret);
  });
}

function resolve(self, newValue) {
  try {
    // Promise Resolution Procedure: https://github.com/promises-aplus/promises-spec#the-promise-resolution-procedure
    if (newValue === self) { throw new TypeError('A promise cannot be resolved with itself.'); }
    if (newValue && ((typeof newValue === 'undefined' ? 'undefined' : _typeof(newValue)) === 'object' || typeof newValue === 'function')) {
      var then = newValue.then;
      if (newValue instanceof Promise) {
        self._state = 3;
        self._value = newValue;
        finale(self);
        return;
      } else if (typeof then === 'function') {
        doResolve(bind(then, newValue), self);
        return;
      }
    }
    self._state = 1;
    self._value = newValue;
    finale(self);
  } catch (e) {
    reject(self, e);
  }
}

function reject(self, newValue) {
  self._state = 2;
  self._value = newValue;
  finale(self);
}

function finale(self) {
  if (self._state === 2 && self._deferreds.length === 0) {
    asap(function () {
      if (!self._handled) {
        onUnhandledRejection(self._value);
      }
    });
  }

  for (var i = 0, len = self._deferreds.length; i < len; i++) {
    handle(self, self._deferreds[i]);
  }
  self._deferreds = null;
}

function Handler(onFulfilled, onRejected, promise) {
  this.onFulfilled = typeof onFulfilled === 'function' ? onFulfilled : null;
  this.onRejected = typeof onRejected === 'function' ? onRejected : null;
  this.promise = promise;
}

/**
 * Take a potentially misbehaving resolver function and make sure
 * onFulfilled and onRejected are only called once.
 *
 * Makes no guarantees about asynchrony.
 */
function doResolve(fn, self) {
  var done = false;
  try {
    fn(function (value) {
      if (done) { return; }
      done = true;
      resolve(self, value);
    }, function (reason) {
      if (done) { return; }
      done = true;
      reject(self, reason);
    });
  } catch (ex) {
    if (done) { return; }
    done = true;
    reject(self, ex);
  }
}

Promise.prototype.catch = function (onRejected) {
  return this.then(null, onRejected);
};

Promise.prototype.then = function (onFulfilled, onRejected) {
  var prom = new this.constructor(noop);

  handle(this, new Handler(onFulfilled, onRejected, prom));
  return prom;
};

Promise.all = function (arr) {
  var args = Array.prototype.slice.call(arr);

  return new Promise(function (resolve, reject) {
    if (args.length === 0) { return resolve([]); }
    var remaining = args.length;

    function res(i, val) {
      try {
        if (val && ((typeof val === 'undefined' ? 'undefined' : _typeof(val)) === 'object' || typeof val === 'function')) {
          var then = val.then;
          if (typeof then === 'function') {
            then.call(val, function (val) {
              res(i, val);
            }, reject);
            return;
          }
        }
        args[i] = val;
        if (--remaining === 0) {
          resolve(args);
        }
      } catch (ex) {
        reject(ex);
      }
    }

    for (var i = 0; i < args.length; i++) {
      res(i, args[i]);
    }
  });
};

Promise.resolve = function (value) {
  if (value && (typeof value === 'undefined' ? 'undefined' : _typeof(value)) === 'object' && value.constructor === Promise) {
    return value;
  }

  return new Promise(function (resolve) {
    resolve(value);
  });
};

Promise.reject = function (value) {
  return new Promise(function (resolve, reject) {
    reject(value);
  });
};

Promise.race = function (values) {
  return new Promise(function (resolve, reject) {
    for (var i = 0, len = values.length; i < len; i++) {
      values[i].then(resolve, reject);
    }
  });
};

/**
 * Set the immediate function to execute callbacks
 * @param fn {function} Function to execute
 * @private
 */
Promise._setImmediateFn = function _setImmediateFn(fn) {
  asap = fn;
};

Promise._setUnhandledRejectionFn = function _setUnhandledRejectionFn(fn) {
  onUnhandledRejection = fn;
};

module.exports = Promise;

/***/ }),
/* 4 */
/***/ (function(module, exports, __webpack_require__) {

var _symbol = __webpack_require__(0);

var _symbol2 = _interopRequireDefault(_symbol);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

// Deleted map items mess with iterator pointers, so rather than removing them mark them as deleted. Can't use undefined or null since those both valid keys so use a private symbol.
var undefMarker = (0, _symbol2.default)('undef');
// NaN cannot be found in an array using indexOf, so we encode NaNs using a private symbol.
/* eslint no-extend-native: "off" */
var NaNMarker = (0, _symbol2.default)('NaN');
var ACCESSOR_SUPPORT = true;

function encodeKey(key) {
  // Number.isNaN not extist in iOS 8.x
  return key !== key ? NaNMarker : key;
}

function decodeKey(encodedKey) {
  return encodedKey === NaNMarker ? NaN : encodedKey;
}

function makeIterator(mapInst, getter) {
  var nextIdx = 0;
  var done = false;
  return {
    next: function next() {
      if (nextIdx === mapInst._keys.length) { done = true; }
      if (!done) {
        while (mapInst._keys[nextIdx] === undefMarker) {
          nextIdx++;
        }return { value: getter.call(mapInst, nextIdx++), done: false };
      } else {
        return { value: void 0, done: true };
      }
    }
  };
}

function calcSize(mapInst) {
  var size = 0;
  for (var i = 0, s = mapInst._keys.length; i < s; i++) {
    if (mapInst._keys[i] !== undefMarker) { size++; }
  }
  return size;
}

function hasProtoMethod(instance, method) {
  return typeof instance[method] === 'function';
}

var Map = function Map(data) {
  this._keys = [];
  this._values = [];
  // If `data` is iterable (indicated by presence of a forEach method), pre-populate the map
  if (data && hasProtoMethod(data, 'forEach')) {
    // Fastpath: If `data` is a Map, shortcircuit all following the checks
    if (data instanceof Map ||
    // If `data` is not an instance of Map, it could be because you have a Map from an iframe or a worker or something.
    // Check if  `data` has all the `Map` methods and if so, assume data is another Map
    hasProtoMethod(data, 'clear') && hasProtoMethod(data, 'delete') && hasProtoMethod(data, 'entries') && hasProtoMethod(data, 'forEach') && hasProtoMethod(data, 'get') && hasProtoMethod(data, 'has') && hasProtoMethod(data, 'keys') && hasProtoMethod(data, 'set') && hasProtoMethod(data, 'values')) {
      data.forEach(function (value, key) {
        this.set.apply(this, [key, value]);
      }, this);
    } else {
      data.forEach(function (item) {
        this.set.apply(this, item);
      }, this);
    }
  }

  if (!ACCESSOR_SUPPORT) { this.size = calcSize(this); }
};
Map.prototype = {};

// Some old engines do not support ES5 getters/setters.  Since Map only requires these for the size property, we can fall back to setting the size property statically each time the size of the map changes.
try {
  Object.defineProperty(Map.prototype, 'size', {
    get: function get() {
      return calcSize(this);
    }
  });
} catch (e) {
  ACCESSOR_SUPPORT = false;
}

Map.prototype.get = function (key) {
  var idx = this._keys.indexOf(encodeKey(key));
  return idx !== -1 ? this._values[idx] : undefined;
};
Map.prototype.set = function (key, value) {
  var idx = this._keys.indexOf(encodeKey(key));
  if (idx !== -1) {
    this._values[idx] = value;
  } else {
    this._keys.push(encodeKey(key));
    this._values.push(value);
    if (!ACCESSOR_SUPPORT) { this.size = calcSize(this); }
  }
  return this;
};
Map.prototype.has = function (key) {
  return this._keys.indexOf(encodeKey(key)) !== -1;
};
Map.prototype.delete = function (key) {
  var idx = this._keys.indexOf(encodeKey(key));
  if (idx === -1) { return false; }
  this._keys[idx] = undefMarker;
  this._values[idx] = undefMarker;
  if (!ACCESSOR_SUPPORT) { this.size = calcSize(this); }
  return true;
};
Map.prototype.clear = function () {
  this._keys = this._values = [];
  if (!ACCESSOR_SUPPORT) { this.size = 0; }
};
Map.prototype.values = function () {
  return makeIterator(this, function (i) {
    return this._values[i];
  });
};
Map.prototype.keys = function () {
  return makeIterator(this, function (i) {
    return decodeKey(this._keys[i]);
  });
};
Map.prototype.entries = Map.prototype[_symbol2.default.iterator] = function () {
  return makeIterator(this, function (i) {
    return [decodeKey(this._keys[i]), this._values[i]];
  });
};
Map.prototype.forEach = function (callbackFn, thisArg) {
  var this$1 = this;

  thisArg = thisArg || global;
  var iterator = this.entries();
  var result = iterator.next();
  while (result.done === false) {
    callbackFn.call(thisArg, result.value[1], result.value[0], this$1);
    result = iterator.next();
  }
};

Map.prototype[_symbol2.default.species] = Map;

Object.defineProperty(Map, 'constructor', {
  value: Map
});

try {
  Object.defineProperty(Map, 'length', {
    value: 0
  });
} catch (e) {}

module.exports = Map;

/***/ }),
/* 5 */
/***/ (function(module, exports, __webpack_require__) {

var _symbol = __webpack_require__(0);

var _symbol2 = _interopRequireDefault(_symbol);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

// Deleted map items mess with iterator pointers, so rather than removing them mark them as deleted. Can't use undefined or null since those both valid keys so use a private symbol.
var undefMarker = (0, _symbol2.default)('undef');

// NaN cannot be found in an array using indexOf, so we encode NaNs using a private symbol.
/* eslint no-extend-native: "off" */
var NaNMarker = (0, _symbol2.default)('NaN');

var ACCESSOR_SUPPORT = true;

function encodeVal(data) {
  // Number.isNaN not extist in iOS 8.x
  return data !== data ? NaNMarker : data;
}

function decodeVal(encodedData) {
  return encodedData === NaNMarker ? NaN : encodedData;
}

function makeIterator(setInst, getter) {
  var nextIdx = 0;
  return {
    next: function next() {
      while (setInst._values[nextIdx] === undefMarker) {
        nextIdx++;
      }if (nextIdx === setInst._values.length) {
        return { value: void 0, done: true };
      } else {
        return { value: getter.call(setInst, nextIdx++), done: false };
      }
    }
  };
}

function calcSize(setInst) {
  var size = 0;
  for (var i = 0, s = setInst._values.length; i < s; i++) {
    if (setInst._values[i] !== undefMarker) { size++; }
  }
  return size;
}

var Set = function Set(data) {
  this._values = [];

  // If `data` is iterable (indicated by presence of a forEach method), pre-populate the set
  data && typeof data.forEach === 'function' && data.forEach(function (item) {
    this.add.call(this, item);
  }, this);

  if (!ACCESSOR_SUPPORT) { this.size = calcSize(this); }
};

// Some old engines do not support ES5 getters/setters.  Since Set only requires these for the size property, we can fall back to setting the size property statically each time the size of the set changes.
try {
  Object.defineProperty(Set.prototype, 'size', {
    get: function get() {
      return calcSize(this);
    }
  });
} catch (e) {
  ACCESSOR_SUPPORT = false;
}

Set.prototype.add = function (value) {
  value = encodeVal(value);
  if (this._values.indexOf(value) === -1) {
    this._values.push(value);
    if (!ACCESSOR_SUPPORT) { this.size = calcSize(this); }
  }
  return this;
};
Set.prototype.has = function (value) {
  return this._values.indexOf(encodeVal(value)) !== -1;
};
Set.prototype.delete = function (value) {
  var idx = this._values.indexOf(encodeVal(value));
  if (idx === -1) { return false; }
  this._values[idx] = undefMarker;
  if (!ACCESSOR_SUPPORT) { this.size = calcSize(this); }
  return true;
};
Set.prototype.clear = function () {
  this._values = [];
  if (!ACCESSOR_SUPPORT) { this.size = 0; }
};
Set.prototype.values = Set.prototype.keys = function () {
  return makeIterator(this, function (i) {
    return decodeVal(this._values[i]);
  });
};
Set.prototype.entries = Set.prototype[_symbol2.default.iterator] = function () {
  return makeIterator(this, function (i) {
    return [decodeVal(this._values[i]), decodeVal(this._values[i])];
  });
};
Set.prototype.forEach = function (callbackFn, thisArg) {
  var this$1 = this;

  thisArg = thisArg || global;
  var iterator = this.entries();
  var result = iterator.next();
  while (result.done === false) {
    callbackFn.call(thisArg, result.value[1], result.value[0], this$1);
    result = iterator.next();
  }
};

Set.prototype[_symbol2.default.species] = Set;

Object.defineProperty(Set, 'constructor', {
  value: Set
});

try {
  Object.defineProperty(Set, 'length', {
    value: 0
  });
} catch (e) {}

module.exports = Set;

/***/ }),
/* 6 */
/***/ (function(module, exports, __webpack_require__) {

var _typeof = typeof Symbol === "function" && typeof Symbol.iterator === "symbol" ? function (obj) { return typeof obj; } : function (obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; };

/* eslint no-extend-native: "off" */

var defineProperty = Object.defineProperty;
var counter = Date.now() % 1e9;

var WeakMap = function WeakMap(data) {
  this.name = '__st' + (Math.random() * 1e9 >>> 0) + (counter++ + '__');

  // If data is iterable (indicated by presence of a forEach method), pre-populate the map
  data && data.forEach && data.forEach(function (item) {
    this.set.apply(this, item);
  }, this);
};

WeakMap.prototype.set = function (key, value) {
  if ((typeof key === 'undefined' ? 'undefined' : _typeof(key)) !== 'object' && typeof key !== 'function') { throw new TypeError('Invalid value used as weak map key'); }

  var entry = key[this.name];
  if (entry && entry[0] === key) { entry[1] = value; }else { defineProperty(key, this.name, { value: [key, value], writable: true }); }
  return this;
};

WeakMap.prototype.get = function (key) {
  var entry;
  return (entry = key[this.name]) && entry[0] === key ? entry[1] : undefined;
};

WeakMap.prototype.delete = function (key) {
  var entry = key[this.name];
  if (!entry || entry[0] !== key) { return false; }
  entry[0] = entry[1] = undefined;
  return true;
};

WeakMap.prototype.has = function (key) {
  var entry = key[this.name];
  if (!entry) { return false; }
  return entry[0] === key;
};

module.exports = WeakMap;

/***/ }),
/* 7 */
/***/ (function(module, exports, __webpack_require__) {

var counter = Date.now() % 1e9;

var WeakSet = function WeakSet(data) {
  this.name = '__st' + (Math.random() * 1e9 >>> 0) + (counter++ + '__');
  data && data.forEach && data.forEach(this.add, this);
};

WeakSet.prototype.add = function (obj) {
  var name = this.name;
  if (!obj[name]) { Object.defineProperty(obj, name, { value: true, writable: true }); }
  return this;
};

WeakSet.prototype.delete = function (obj) {
  if (!obj[this.name]) { return false; }
  obj[this.name] = undefined;
  return true;
};

WeakSet.prototype.has = function (obj) {
  return !!obj[this.name];
};

module.exports = WeakSet;

/***/ }),
/* 8 */
/***/ (function(module, exports, __webpack_require__) {

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

var FontFace = function FontFace(family, source) {
  _classCallCheck(this, FontFace);

  this.family = family;
  this.source = source;
};

module.exports = FontFace;

/***/ }),
/* 9 */
/***/ (function(module, exports, __webpack_require__) {

var URLSearchParams = __webpack_require__(1);

var relative = Object.create(null);
relative.ftp = 21;
relative.file = 0;
relative.gopher = 70;
relative.http = 80;
relative.https = 443;
relative.ws = 80;
relative.wss = 443;

var relativePathDotMapping = Object.create(null);
relativePathDotMapping['%2e'] = '.';
relativePathDotMapping['.%2e'] = '..';
relativePathDotMapping['%2e.'] = '..';
relativePathDotMapping['%2e%2e'] = '..';

function isRelativeScheme(scheme) {
  return relative[scheme] !== undefined;
}

function invalid() {
  clear.call(this);
  this._isInvalid = true;
}

function IDNAToASCII(h) {
  if ('' == h) {
    invalid.call(this);
  }
  // XXX
  return h.toLowerCase();
}

function percentEscape(c) {
  var unicode = c.charCodeAt(0);
  if (unicode > 0x20 && unicode < 0x7F &&
  // " # < > ? `
  [0x22, 0x23, 0x3C, 0x3E, 0x3F, 0x60].indexOf(unicode) == -1) {
    return c;
  }
  return encodeURIComponent(c);
}

function percentEscapeQuery(c) {
  // XXX This actually needs to encode c using encoding and then
  // convert the bytes one-by-one.

  var unicode = c.charCodeAt(0);
  if (unicode > 0x20 && unicode < 0x7F &&
  // " # < > ` (do not escape '?')
  [0x22, 0x23, 0x3C, 0x3E, 0x60].indexOf(unicode) == -1) {
    return c;
  }
  return encodeURIComponent(c);
}

var EOF = undefined,
    ALPHA = /[a-zA-Z]/,
    ALPHANUMERIC = /[a-zA-Z0-9\+\-\.]/;

function parse(input, stateOverride, base) {
  var this$1 = this;

  function err(message) {
    errors.push(message);
  }

  var state = stateOverride || 'scheme start',
      cursor = 0,
      buffer = '',
      seenAt = false,
      seenBracket = false,
      errors = [];

  loop: while ((input[cursor - 1] != EOF || cursor == 0) && !this._isInvalid) {
    var c = input[cursor];
    switch (state) {
      case 'scheme start':
        if (c && ALPHA.test(c)) {
          buffer += c.toLowerCase(); // ASCII-safe
          state = 'scheme';
        } else if (!stateOverride) {
          buffer = '';
          state = 'no scheme';
          continue;
        } else {
          err('Invalid scheme.');
          break loop;
        }
        break;

      case 'scheme':
        if (c && ALPHANUMERIC.test(c)) {
          buffer += c.toLowerCase(); // ASCII-safe
        } else if (':' == c) {
          this$1._scheme = buffer;
          buffer = '';
          if (stateOverride) {
            break loop;
          }
          if (isRelativeScheme(this$1._scheme)) {
            this$1._isRelative = true;
          }
          if ('file' == this$1._scheme) {
            state = 'relative';
          } else if (this$1._isRelative && base && base._scheme == this$1._scheme) {
            state = 'relative or authority';
          } else if (this$1._isRelative) {
            state = 'authority first slash';
          } else {
            state = 'scheme data';
          }
        } else if (!stateOverride) {
          buffer = '';
          cursor = 0;
          state = 'no scheme';
          continue;
        } else if (EOF == c) {
          break loop;
        } else {
          err('Code point not allowed in scheme: ' + c);
          break loop;
        }
        break;

      case 'scheme data':
        if ('?' == c) {
          state = 'query';
        } else if ('#' == c) {
          this$1._fragment = '#';
          state = 'fragment';
        } else {
          // XXX error handling
          if (EOF != c && '\t' != c && '\n' != c && '\r' != c) {
            this$1._schemeData += percentEscape(c);
          }
        }
        break;

      case 'no scheme':
        if (!base || !isRelativeScheme(base._scheme)) {
          err('Missing scheme.');
          invalid.call(this$1);
        } else {
          state = 'relative';
          continue;
        }
        break;

      case 'relative or authority':
        if ('/' == c && '/' == input[cursor + 1]) {
          state = 'authority ignore slashes';
        } else {
          err('Expected /, got: ' + c);
          state = 'relative';
          continue;
        }
        break;

      case 'relative':
        this$1._isRelative = true;
        if ('file' != this$1._scheme) { this$1._scheme = base._scheme; }
        if (EOF == c) {
          this$1._host = base._host;
          this$1._port = base._port;
          this$1._path = base._path.slice();
          this$1._query = base._query;
          this$1._username = base._username;
          this$1._password = base._password;
          break loop;
        } else if ('/' == c || '\\' == c) {
          if ('\\' == c) { err('\\ is an invalid code point.'); }
          state = 'relative slash';
        } else if ('?' == c) {
          this$1._host = base._host;
          this$1._port = base._port;
          this$1._path = base._path.slice();
          this$1._query = '?';
          this$1._username = base._username;
          this$1._password = base._password;
          state = 'query';
        } else if ('#' == c) {
          this$1._host = base._host;
          this$1._port = base._port;
          this$1._path = base._path.slice();
          this$1._query = base._query;
          this$1._fragment = '#';
          this$1._username = base._username;
          this$1._password = base._password;
          state = 'fragment';
        } else {
          var nextC = input[cursor + 1];
          var nextNextC = input[cursor + 2];
          if ('file' != this$1._scheme || !ALPHA.test(c) || nextC != ':' && nextC != '|' || EOF != nextNextC && '/' != nextNextC && '\\' != nextNextC && '?' != nextNextC && '#' != nextNextC) {
            this$1._host = base._host;
            this$1._port = base._port;
            this$1._username = base._username;
            this$1._password = base._password;
            this$1._path = base._path.slice();
            this$1._path.pop();
          }
          state = 'relative path';
          continue;
        }
        break;

      case 'relative slash':
        if ('/' == c || '\\' == c) {
          if ('\\' == c) {
            err('\\ is an invalid code point.');
          }
          if ('file' == this$1._scheme) {
            state = 'file host';
          } else {
            state = 'authority ignore slashes';
          }
        } else {
          if ('file' != this$1._scheme) {
            this$1._host = base._host;
            this$1._port = base._port;
            this$1._username = base._username;
            this$1._password = base._password;
          }
          state = 'relative path';
          continue;
        }
        break;

      case 'authority first slash':
        if ('/' == c) {
          state = 'authority second slash';
        } else {
          err("Expected '/', got: " + c);
          state = 'authority ignore slashes';
          continue;
        }
        break;

      case 'authority second slash':
        state = 'authority ignore slashes';
        if ('/' != c) {
          err("Expected '/', got: " + c);
          continue;
        }
        break;

      case 'authority ignore slashes':
        if ('/' != c && '\\' != c) {
          state = 'authority';
          continue;
        } else {
          err('Expected authority, got: ' + c);
        }
        break;

      case 'authority':
        if ('@' == c) {
          if (seenAt) {
            err('@ already seen.');
            buffer += '%40';
          }
          seenAt = true;
          for (var i = 0; i < buffer.length; i++) {
            var cp = buffer[i];
            if ('\t' == cp || '\n' == cp || '\r' == cp) {
              err('Invalid whitespace in authority.');
              continue;
            }
            // XXX check URL code points
            if (':' == cp && null === this$1._password) {
              this$1._password = '';
              continue;
            }
            var tempC = percentEscape(cp);
            null !== this$1._password ? this$1._password += tempC : this$1._username += tempC;
          }
          buffer = '';
        } else if (EOF == c || '/' == c || '\\' == c || '?' == c || '#' == c) {
          cursor -= buffer.length;
          buffer = '';
          state = 'host';
          continue;
        } else {
          buffer += c;
        }
        break;

      case 'file host':
        if (EOF == c || '/' == c || '\\' == c || '?' == c || '#' == c) {
          if (buffer.length == 2 && ALPHA.test(buffer[0]) && (buffer[1] == ':' || buffer[1] == '|')) {
            state = 'relative path';
          } else if (buffer.length == 0) {
            state = 'relative path start';
          } else {
            this$1._host = IDNAToASCII.call(this$1, buffer);
            buffer = '';
            state = 'relative path start';
          }
          continue;
        } else if ('\t' == c || '\n' == c || '\r' == c) {
          err('Invalid whitespace in file host.');
        } else {
          buffer += c;
        }
        break;

      case 'host':
      case 'hostname':
        if (':' == c && !seenBracket) {
          // XXX host parsing
          this$1._host = IDNAToASCII.call(this$1, buffer);
          buffer = '';
          state = 'port';
          if ('hostname' == stateOverride) {
            break loop;
          }
        } else if (EOF == c || '/' == c || '\\' == c || '?' == c || '#' == c) {
          this$1._host = IDNAToASCII.call(this$1, buffer);
          buffer = '';
          state = 'relative path start';
          if (stateOverride) {
            break loop;
          }
          continue;
        } else if ('\t' != c && '\n' != c && '\r' != c) {
          if ('[' == c) {
            seenBracket = true;
          } else if (']' == c) {
            seenBracket = false;
          }
          buffer += c;
        } else {
          err('Invalid code point in host/hostname: ' + c);
        }
        break;

      case 'port':
        if (/[0-9]/.test(c)) {
          buffer += c;
        } else if (EOF == c || '/' == c || '\\' == c || '?' == c || '#' == c || stateOverride) {
          if ('' != buffer) {
            var temp = parseInt(buffer, 10);
            if (temp != relative[this$1._scheme]) {
              this$1._port = temp + '';
            }
            buffer = '';
          }
          if (stateOverride) {
            break loop;
          }
          state = 'relative path start';
          continue;
        } else if ('\t' == c || '\n' == c || '\r' == c) {
          err('Invalid code point in port: ' + c);
        } else {
          invalid.call(this$1);
        }
        break;

      case 'relative path start':
        if ('\\' == c) { err("'\\' not allowed in path."); }
        state = 'relative path';
        if ('/' != c && '\\' != c) {
          continue;
        }
        break;

      case 'relative path':
        if (EOF == c || '/' == c || '\\' == c || !stateOverride && ('?' == c || '#' == c)) {
          if ('\\' == c) {
            err('\\ not allowed in relative path.');
          }
          var tmp;
          if (tmp = relativePathDotMapping[buffer.toLowerCase()]) {
            buffer = tmp;
          }
          if ('..' == buffer) {
            this$1._path.pop();
            if ('/' != c && '\\' != c) {
              this$1._path.push('');
            }
          } else if ('.' == buffer && '/' != c && '\\' != c) {
            this$1._path.push('');
          } else if ('.' != buffer) {
            if ('file' == this$1._scheme && this$1._path.length == 0 && buffer.length == 2 && ALPHA.test(buffer[0]) && buffer[1] == '|') {
              buffer = buffer[0] + ':';
            }
            this$1._path.push(buffer);
          }
          buffer = '';
          if ('?' == c) {
            this$1._query = '?';
            state = 'query';
          } else if ('#' == c) {
            this$1._fragment = '#';
            state = 'fragment';
          }
        } else if ('\t' != c && '\n' != c && '\r' != c) {
          buffer += percentEscape(c);
        }
        break;

      case 'query':
        if (!stateOverride && '#' == c) {
          this$1._fragment = '#';
          state = 'fragment';
        } else if (EOF != c && '\t' != c && '\n' != c && '\r' != c) {
          this$1._query += percentEscapeQuery(c);
        }
        break;

      case 'fragment':
        if (EOF != c && '\t' != c && '\n' != c && '\r' != c) {
          this$1._fragment += c;
        }
        break;
    }

    cursor++;
  }
}

function clear() {
  this._scheme = '';
  this._schemeData = '';
  this._username = '';
  this._password = null;
  this._host = '';
  this._port = '';
  this._path = [];
  this._query = '';
  this._fragment = '';
  this._isInvalid = false;
  this._isRelative = false;
}

// Does not process domain names or IP addresses.
// Does not handle encoding for the query parameter.
function URL(url, base /*  encoding */) {
  if (base !== undefined && !(base instanceof URL)) { base = new URL(String(base)); }

  this._url = url;
  clear.call(this);

  var input = url.replace(/^[ \t\r\n\f]+|[ \t\r\n\f]+$/g, '');
  // encoding = encoding || 'utf-8'

  parse.call(this, input, null, base);
}

URL.prototype = {
  toString: function toString() {
    return this.href;
  },
  get href() {
    if (this._isInvalid) { return this._url; }

    var authority = '';
    if ('' != this._username || null != this._password) {
      authority = this._username + (null != this._password ? ':' + this._password : '') + '@';
    }

    return this.protocol + (this._isRelative ? '//' + authority + this.host : '') + this.pathname + this._query + this._fragment;
  },
  set href(href) {
    clear.call(this);
    parse.call(this, href);
  },

  get protocol() {
    return this._scheme + ':';
  },
  set protocol(protocol) {
    if (this._isInvalid) { return; }
    parse.call(this, protocol + ':', 'scheme start');
  },

  get host() {
    return this._isInvalid ? '' : this._port ? this._host + ':' + this._port : this._host;
  },
  set host(host) {
    if (this._isInvalid || !this._isRelative) { return; }
    parse.call(this, host, 'host');
  },

  get hostname() {
    return this._host;
  },
  set hostname(hostname) {
    if (this._isInvalid || !this._isRelative) { return; }
    parse.call(this, hostname, 'hostname');
  },

  get port() {
    return this._port;
  },
  set port(port) {
    if (this._isInvalid || !this._isRelative) { return; }
    parse.call(this, port, 'port');
  },

  get pathname() {
    return this._isInvalid ? '' : this._isRelative ? '/' + this._path.join('/') : this._schemeData;
  },
  set pathname(pathname) {
    if (this._isInvalid || !this._isRelative) { return; }
    this._path = [];
    parse.call(this, pathname, 'relative path start');
  },

  get search() {
    return this._isInvalid || !this._query || '?' == this._query ? '' : this._query;
  },
  set search(search) {
    if (this._isInvalid || !this._isRelative) { return; }
    this._query = '?';
    if ('?' == search[0]) { search = search.slice(1); }
    parse.call(this, search, 'query');
  },

  get searchParams() {
    return new URLSearchParams(this.search);
  },

  get hash() {
    return this._isInvalid || !this._fragment || '#' == this._fragment ? '' : this._fragment;
  },
  set hash(hash) {
    if (this._isInvalid) { return; }
    this._fragment = '#';
    if ('#' == hash[0]) { hash = hash.slice(1); }
    parse.call(this, hash, 'fragment');
  },

  get origin() {
    var host;
    if (this._isInvalid || !this._scheme) {
      return '';
    }
    // javascript: Gecko returns String(""), WebKit/Blink String("null")
    // Gecko throws error for "data://"
    // data: Gecko returns "", Blink returns "data://", WebKit returns "null"
    // Gecko returns String("") for file: mailto:
    // WebKit/Blink returns String("SCHEME://") for file: mailto:
    switch (this._scheme) {
      case 'data':
      case 'file':
      case 'javascript':
      case 'mailto':
        return 'null';
    }
    host = this.host;
    if (!host) {
      return '';
    }
    return this._scheme + '://' + host;
  }
};

module.exports = URL;

/***/ }),
/* 10 */
/***/ (function(module, exports, __webpack_require__) {

var RE_MEDIA_QUERY = /^(?:(only|not)?\s*([_a-z][_a-z0-9-]*)|(\([^\)]+\)))(?:\s*and\s*(.*))?$/i,
    RE_MQ_EXPRESSION = /^\(\s*([_a-z-][_a-z0-9-]*)\s*(?:\:\s*([^\)]+))?\s*\)$/,
    RE_MQ_FEATURE = /^(?:(min|max)-)?(.+)/;

function _matches(media, values) {
  return _parseQuery(media).some(function (query) {
    var inverse = query.inverse;

    var typeMatch = query.type === 'all' || values.type === query.type;

    if (typeMatch && inverse || !(typeMatch || inverse)) {
      return false;
    }

    var expressionsMatch = query.expressions.every(function (expression) {
      var feature = expression.feature,
          modifier = expression.modifier,
          expValue = expression.value,
          value = values[feature];

      if (!value) {
        return false;
      }

      switch (feature) {
        case 'width':
        case 'height':
          expValue = parseFloat(expValue);
          value = parseFloat(value);
          break;
      }

      switch (modifier) {
        case 'min':
          return value >= expValue;
        case 'max':
          return value <= expValue;
        default:
          return value === expValue;
      }
    });

    return expressionsMatch && !inverse || !expressionsMatch && inverse;
  });
}

function _parseQuery(media) {
  return media.split(',').map(function (query) {
    query = query.trim();

    var captures = query.match(RE_MEDIA_QUERY);

    if (!captures) {
      throw new SyntaxError('Invalid CSS media query: "' + query + '"');
    }

    var modifier = captures[1],
        type = captures[2],
        expressions = ((captures[3] || '') + (captures[4] || '')).trim(),
        parsed = {};

    parsed.inverse = !!modifier && modifier.toLowerCase() === 'not';
    parsed.type = type ? type.toLowerCase() : 'all';

    if (!expressions) {
      parsed.expressions = [];
      return parsed;
    }

    expressions = expressions.match(/\([^\)]+\)/g);

    if (!expressions) {
      throw new SyntaxError('Invalid CSS media query: "' + query + '"');
    }

    parsed.expressions = expressions.map(function (expression) {
      var captures = expression.match(RE_MQ_EXPRESSION);

      if (!captures) {
        throw new SyntaxError('Invalid CSS media query: "' + query + '"');
      }

      var feature = captures[1].toLowerCase().match(RE_MQ_FEATURE);

      return {
        modifier: feature[1],
        feature: feature[2],
        value: captures[2]
      };
    });

    return parsed;
  });
}

function matchMedia(media) {
  var mql = {
    matches: false,
    media: media
  };

  if (media === '') {
    mql.matches = true;
    return mql;
  }

  mql.matches = _matches(media, {
    type: 'screen',
    width: window.screen.width,
    height: window.screen.height
  });

  return mql;
}

module.exports = matchMedia;

/***/ })
/******/ ])};

/***/ }),
/* 7 */
/*!************************************************************!*\
  !*** ./packages/weex-rax-framework-api/src/define.weex.js ***!
  \************************************************************/
/*! dynamic exports provided */
/*! all exports used */
/*! ModuleConcatenation bailout: Module is not an ECMAScript module */
/***/ (function(module, exports, __webpack_require__) {

module.exports = function (modules) {
  function define(name, deps, factory) {
    if (deps instanceof Function) {
      factory = deps;
      deps = [];
    }

    modules[name] = {
      factory: factory,
      deps: deps,
      module: { exports: {} },
      isInitialized: false,
      hasError: false
    };
  }

  return define;
};

/***/ }),
/* 8 */
/*!*************************************************************!*\
  !*** ./packages/weex-rax-framework-api/src/require.weex.js ***!
  \*************************************************************/
/*! dynamic exports provided */
/*! all exports used */
/*! ModuleConcatenation bailout: Module is not an ECMAScript module */
/***/ (function(module, exports, __webpack_require__) {

var MODULE_NAME_PREFIX = '@weex-module/';

module.exports = function (modules, weex) {
  function require(name) {
    var mod = modules[name];

    // if retuire '@weex-module/'
    if (name.split(MODULE_NAME_PREFIX).length > 1) {
      var weexModuleName = name.split(MODULE_NAME_PREFIX)[1];
      if (weex.isRegisteredModule(weexModuleName)) {
        return weex.requireModule(weexModuleName);
      } else {
        throw new Error('Requiring unknown weex module "' + name + '"');
      }
    }

    if (mod && mod.isInitialized) {
      return mod.module.exports;
    }

    if (!mod) {
      throw new Error('Requiring unknown module "' + name + '"');
    }

    if (mod.hasError) {
      throw new Error('Requiring module "' + name + '" which threw an exception');
    }

    try {
      mod.isInitialized = true;
      mod.factory(require, mod.module.exports, mod.module);
    } catch (e) {
      mod.hasError = true;
      mod.isInitialized = false;
      throw e;
    }

    return mod.module.exports;
  }

  return require;
};

/***/ }),
/* 9 */
/*!***************************************************************!*\
  !*** ./packages/weex-rax-framework-api/src/downgrade.weex.js ***!
  \***************************************************************/
/*! dynamic exports provided */
/*! all exports used */
/*! ModuleConcatenation bailout: Module is not an ECMAScript module */
/***/ (function(module, exports, __webpack_require__) {

var _semver = __webpack_require__(/*! ./semver */ 10);

var _semver2 = _interopRequireDefault(_semver);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function normalizeVersion(v) {
  if (v == '*') {
    return v;
  }
  v = typeof v === 'string' ? v : '';
  var split = v.split('.');
  var i = 0;
  var result = [];

  while (i < 3) {
    var s = typeof split[i] === 'string' && split[i] ? split[i] : '0';
    result.push(s);
    i++;
  }

  return result.join('.');
} /* global WXEnvironment */

function getError(key, val, criteria) {
  var result = {
    isDowngrade: true,
    errorType: 1,
    code: 1000
  };
  var getMsg = function getMsg(key, val, criteria) {
    return 'Downgrade[' + key + '] :: deviceInfo ' + val + ' matched criteria ' + criteria;
  };
  var _key = key.toLowerCase();

  if (_key.indexOf('osversion') >= 0) {
    result.code = 1001;
  } else if (_key.indexOf('appversion') >= 0) {
    result.code = 1002;
  } else if (_key.indexOf('weexversion') >= 0) {
    result.code = 1003;
  } else if (_key.indexOf('devicemodel') >= 0) {
    result.code = 1004;
  }

  result.errorMessage = getMsg(key, val, criteria);
  return result;
}

/**
 * config
 *
 * {
 *   ios: {
 *     osVersion: '>1.0.0' or '>=1.0.0' or '<1.0.0' or '<=1.0.0' or '1.0.0'
 *     appVersion: '>1.0.0' or '>=1.0.0' or '<1.0.0' or '<=1.0.0' or '1.0.0'
 *     weexVersion: '>1.0.0' or '>=1.0.0' or '<1.0.0' or '<=1.0.0' or '1.0.0'
 *     deviceModel: ['modelA', 'modelB', ...]
 *   },
 *   android: {
 *     osVersion: '>1.0.0' or '>=1.0.0' or '<1.0.0' or '<=1.0.0' or '1.0.0'
 *     appVersion: '>1.0.0' or '>=1.0.0' or '<1.0.0' or '<=1.0.0' or '1.0.0'
 *     weexVersion: '>1.0.0' or '>=1.0.0' or '<1.0.0' or '<=1.0.0' or '1.0.0'
 *     deviceModel: ['modelA', 'modelB', ...]
 *   }
 * }
 *
 */
function check(config) {
  var result = {
    isDowngrade: false
  };

  var deviceInfo = WXEnvironment;

  var platform = deviceInfo.platform || 'unknow';
  var dPlatform = platform.toLowerCase();
  var cObj = config[dPlatform] || {};

  for (var i in deviceInfo) {
    var key = i;
    var keyLower = key.toLowerCase();
    var val = deviceInfo[i];
    var isVersion = keyLower.indexOf('version') >= 0;
    var isDeviceModel = keyLower.indexOf('devicemodel') >= 0;
    var criteria = cObj[i];

    if (criteria && isVersion) {
      var c = normalizeVersion(criteria);
      var d = normalizeVersion(deviceInfo[i]);

      if (_semver2.default.satisfies(d, c)) {
        result = getError(key, val, criteria);
        break;
      }
    } else if (isDeviceModel) {
      var _criteria = Array.isArray(criteria) ? criteria : [criteria];

      if (_criteria.indexOf(val) >= 0) {
        result = getError(key, val, criteria);
        break;
      }
    }
  }

  return result;
}

module.exports = function (__weex_require__) {
  return function (config) {
    var nativeInstanceWrap = __weex_require__('@weex-module/instanceWrap');
    var result = check(config);
    if (result.isDowngrade) {
      nativeInstanceWrap.error(result.errorType, result.code, result.errorMessage);
      return true;
    }
    return false;
  };
};

/***/ }),
/* 10 */
/*!*******************************************************!*\
  !*** ./packages/weex-rax-framework-api/src/semver.js ***!
  \*******************************************************/
/*! dynamic exports provided */
/*! all exports used */
/*! ModuleConcatenation bailout: Module is not an ECMAScript module */
/***/ (function(module, exports, __webpack_require__) {

Object.defineProperty(exports, "__esModule", {
  value: true
});

var _typeof = typeof Symbol === "function" && typeof Symbol.iterator === "symbol" ? function (obj) { return typeof obj; } : function (obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; };

exports.default = {
  satisfies: function satisfies(left, right) {
    var regex = /(\W+)?([\d|.]+)/;

    if ((typeof left === 'undefined' ? 'undefined' : _typeof(left)) + (typeof right === 'undefined' ? 'undefined' : _typeof(right)) != 'stringstring') { return false; }

    if (right == '*') {
      return true;
    }

    var arr = right.match(regex);
    var a = left.split('.'),
        i = 0,
        b = arr[2].split('.'),
        len = Math.max(a.length, b.length);

    var flag = 0;
    for (var _i = 0; _i < len; _i++) {
      if (a[_i] && !b[_i] && parseInt(a[_i]) > 0 || parseInt(a[_i]) > parseInt(b[_i])) {
        flag = 1;
        break;
      } else if (b[_i] && !a[_i] && parseInt(b[_i]) > 0 || parseInt(a[_i]) < parseInt(b[_i])) {
        flag = -1;
        break;
      }
    }

    switch (arr[1]) {
      case '<':
        if (flag === -1) {
          return true;
        }
        break;
      case '<=':
        if (flag !== 1) {
          return true;
        }
        break;
      case '>':
        if (flag === 1) {
          return true;
        }
        break;
      case '>=':
        if (flag !== -1) {
          return true;
        }
        break;
      default:
        if (flag === 0) {
          return true;
        }
        break;
    }
    return false;
  }
};
module.exports = exports['default'];

/***/ }),
/* 11 */
/*!**************************************************************!*\
  !*** ./packages/weex-rax-framework-api/src/document.weex.js ***!
  \**************************************************************/
/*! dynamic exports provided */
/*! all exports used */
/*! ModuleConcatenation bailout: Module is not an ECMAScript module */
/***/ (function(module, exports, __webpack_require__) {

var _emitter = __webpack_require__(/*! ./emitter */ 1);

var _emitter2 = _interopRequireDefault(_emitter);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

var DOM_MODULE = '@weex-module/dom';
var VISIBLE = 'visible';
var HIDDEN = 'hidden';
var VISIBILITY_CHANGE_EVENT = 'visibilitychange';

function addBodyAppearListener(document) {
  document.body.addEvent('viewappear', function (e) {
    document.visibilityState = VISIBLE;
    e.type = VISIBILITY_CHANGE_EVENT;
    document.dispatchEvent(e);
  });

  document.body.addEvent('viewdisappear', function (e) {
    document.visibilityState = HIDDEN;
    e.type = VISIBILITY_CHANGE_EVENT;
    document.dispatchEvent(e);
  });
}

function removeBodyAppearListener(document) {
  if (document.body) {
    document.body.removeEvent('viewappear');
    document.body.removeEvent('viewdisappear');
  }
}

module.exports = function (__weex_require__, document) {
  // Add w3c events
  var documentEmitter = new _emitter2.default();
  var hasVisibilityEventPending = false;

  // Weex freezed the document maybe throw error
  try {
    document.addEventListener = function (type, listener) {
      if (type === VISIBILITY_CHANGE_EVENT) {
        if (document.body) {
          addBodyAppearListener(document);
        } else {
          hasVisibilityEventPending = true;
        }
      }
      documentEmitter.on(type, listener);
    };

    document.removeEventListener = function (type, listener) {
      if (type === VISIBILITY_CHANGE_EVENT) {
        removeBodyAppearListener(document);
      }
      documentEmitter.off(type, listener);
    };

    document.dispatchEvent = function (e) {
      documentEmitter.emit(e.type, e);
    };

    // FontFace
    document.fonts = {
      add: function add(fontFace) {
        var domModule = __weex_require__(DOM_MODULE);
        domModule.addRule('fontFace', {
          fontFamily: fontFace.family,
          src: fontFace.source // url('uri') : single quotes are required around uri, and double quotes can not work in weex
        });
      }
    };

    // Init visibility state
    document.visibilityState = VISIBLE;

    // Hijack the origin createBody
    var originCreateBody = document.createBody;

    Object.defineProperty(document, 'createBody', {
      value: function value() {
        var body = originCreateBody.apply(document, arguments);

        if (hasVisibilityEventPending) {
          addBodyAppearListener(document);
        }
        return body;
      }
    });
  } catch (e) {
    console.log(e);
  }

  return document;
};

/***/ }),
/* 12 */
/*!**************************************************************!*\
  !*** ./packages/weex-rax-framework-api/src/location.weex.js ***!
  \**************************************************************/
/*! dynamic exports provided */
/*! all exports used */
/*! ModuleConcatenation bailout: Module is not an ECMAScript module */
/***/ (function(module, exports, __webpack_require__) {

var LOCATION_MODULE = '@weex-module/location';
var NAVIGATOR_MODULE = '@weex-module/navigator';

module.exports = function (__weex_require__, location) {
  location.assign = function (url) {
    var weexNavigator = __weex_require__(NAVIGATOR_MODULE);
    weexNavigator.push({
      url: url,
      animated: 'true'
    }, function (e) {
      // noop
    });
  };

  location.replace = function (url) {
    var weexLocation = __weex_require__(LOCATION_MODULE);
    weexLocation.replace(url);
  };

  location.reload = function () {
    var forceReload = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : false;

    var weexLocation = __weex_require__(LOCATION_MODULE);
    weexLocation.reload(forceReload);
  };

  return location;
};

/***/ }),
/* 13 */
/*!***********************************************************!*\
  !*** ./packages/weex-rax-framework-api/src/fetch.weex.js ***!
  \***********************************************************/
/*! dynamic exports provided */
/*! all exports used */
/*! ModuleConcatenation bailout: Module is not an ECMAScript module */
/***/ (function(module, exports, __webpack_require__) {

var STREAM_MODULE = '@weex-module/stream';

module.exports = function (__weex_require__, Promise) {
  function normalizeName(name) {
    if (typeof name !== 'string') {
      name = String(name);
    }
    // FIXME: In spdy the response header has name like ":version" that is invalid
    // if (/[^a-z0-9\-#$%&'*+.\^_`|~]/i.test(name)) {
    //   throw new TypeError('Invalid character in header field name');
    // }
    return name.toLowerCase();
  }

  function normalizeValue(value) {
    if (typeof value !== 'string') {
      value = String(value);
    }
    return value;
  }

  function Headers(headers) {
    this.originHeaders = headers;
    this.map = {};

    if (headers instanceof Headers) {
      headers.forEach(function (value, name) {
        this.append(name, value);
      }, this);
    } else if (headers) {
      Object.getOwnPropertyNames(headers).forEach(function (name) {
        this.append(name, headers[name]);
      }, this);
    }
  }

  Headers.prototype.append = function (name, value) {
    name = normalizeName(name);
    value = normalizeValue(value);
    var oldValue = this.map[name];
    this.map[name] = oldValue ? oldValue + ',' + value : value;
  };

  Headers.prototype.delete = function (name) {
    delete this.map[normalizeName(name)];
  };

  Headers.prototype.get = function (name) {
    name = normalizeName(name);
    return this.has(name) ? this.map[name] : null;
  };

  Headers.prototype.has = function (name) {
    return this.map.hasOwnProperty(normalizeName(name));
  };

  Headers.prototype.set = function (name, value) {
    this.map[normalizeName(name)] = [normalizeValue(value)];
  };

  Headers.prototype.forEach = function (callback, thisArg) {
    var this$1 = this;

    for (var name in this$1.map) {
      if (this$1.map.hasOwnProperty(name)) {
        callback.call(thisArg, this$1.map[name], name, this$1);
      }
    }
  };

  function consumed(body) {
    if (body.bodyUsed) {
      return Promise.reject(new TypeError('Already read'));
    }
    body.bodyUsed = true;
  }

  function Body() {
    this.bodyUsed = false;

    this._initBody = function (body, options) {
      this._bodyInit = body;
      if (typeof body === 'string') {
        this._bodyText = body;
      } else if (!body) {
        this._bodyText = '';
      } else {
        throw new Error('unsupported BodyInit type');
      }
    };

    this.text = function () {
      var rejected = consumed(this);
      return rejected ? rejected : Promise.resolve(this._bodyText);
    };

    this.json = function () {
      return this.text().then(JSON.parse);
    };

    return this;
  }

  // HTTP methods whose capitalization should be normalized
  var methods = ['DELETE', 'GET', 'HEAD', 'OPTIONS', 'POST', 'PUT'];

  function normalizeMethod(method) {
    var upcased = method.toUpperCase();
    return methods.indexOf(upcased) > -1 ? upcased : method;
  }

  function Request(input, options) {
    options = options || {};
    var body = options.body;
    if (Request.prototype.isPrototypeOf(input)) {
      if (input.bodyUsed) {
        throw new TypeError('Already read');
      }
      this.url = input.url;
      this.credentials = input.credentials;
      if (!options.headers) {
        this.headers = new Headers(input.headers);
      }
      this.method = input.method;
      this.mode = input.mode;
      if (!body) {
        body = input._bodyInit;
        input.bodyUsed = true;
      }
    } else {
      this.url = input;
    }

    this.credentials = options.credentials || this.credentials || 'omit';
    if (options.headers || !this.headers) {
      this.headers = new Headers(options.headers);
    }
    this.method = normalizeMethod(options.method || this.method || 'GET');
    this.mode = options.mode || this.mode || null;
    this.referrer = null;

    if ((this.method === 'GET' || this.method === 'HEAD') && body) {
      throw new TypeError('Body not allowed for GET or HEAD requests');
    }
    this._initBody(body, options);
  }

  Request.prototype.clone = function () {
    return new Request(this);
  };

  Body.call(Request.prototype);

  function Response(bodyInit, options) {
    if (!options) {
      options = {};
    }

    this.type = 'default';
    this.status = 'status' in options ? options.status : 200;
    this.ok = this.status >= 200 && this.status < 300;
    this.statusText = 'statusText' in options ? options.statusText : 'OK';
    this.headers = new Headers(options.headers);
    this.url = options.url || '';
    this._initBody(bodyInit, options);
  }

  Body.call(Response.prototype);

  Response.prototype.clone = function () {
    return new Response(this._bodyInit, {
      status: this.status,
      statusText: this.statusText,
      headers: new Headers(this.headers),
      url: this.url
    });
  };

  Response.error = function () {
    var response = new Response(null, { status: 0, statusText: '' });
    response.type = 'error';
    return response;
  };

  var redirectStatuses = [301, 302, 303, 307, 308];

  Response.redirect = function (url, status) {
    if (redirectStatuses.indexOf(status) === -1) {
      throw new RangeError('Invalid status code');
    }

    return new Response(null, { status: status, headers: { location: url } });
  };

  var fetch = function fetch(input, init) {
    return new Promise(function (resolve, reject) {
      var request;
      if (Request.prototype.isPrototypeOf(input) && !init) {
        request = input;
      } else {
        request = new Request(input, init);
      }

      var params = {
        url: request.url,
        method: request.method,
        headers: request.headers && request.headers.originHeaders
      };

      if (typeof request._bodyInit !== 'undefined') {
        params.body = request._bodyInit;
      }

      params.type = init && init.dataType ? init.dataType : 'text';

      var nativeFetch = __weex_require__(STREAM_MODULE).fetch;
      nativeFetch(params, function (response) {
        try {
          typeof response === 'string' && (response = JSON.parse(response));
          var data = typeof response.data === 'string' ? response.data : JSON.stringify(response.data);

          var res = new Response(data, {
            status: response.status,
            statusText: response.statusText,
            headers: response.headers,
            url: request.url
          });
          resolve(res);
        } catch (err) {
          reject(err);
        }
      }, function (progress) {});
    });
  };

  return {
    fetch: fetch,
    Headers: Headers,
    Request: Request,
    Response: Response
  };
};

/***/ }),
/* 14 */
/*!********************************************************************!*\
  !*** ./packages/weex-rax-framework-api/src/xmlhttprequest.weex.js ***!
  \********************************************************************/
/*! dynamic exports provided */
/*! all exports used */
/*! ModuleConcatenation bailout: Module is not an ECMAScript module */
/***/ (function(module, exports, __webpack_require__) {

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) { descriptor.writable = true; } Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) { defineProperties(Constructor.prototype, protoProps); } if (staticProps) { defineProperties(Constructor, staticProps); } return Constructor; }; }();

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _possibleConstructorReturn(self, call) { if (!self) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return call && (typeof call === "object" || typeof call === "function") ? call : self; }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function, not " + typeof superClass); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } }); if (superClass) { Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass; } }

var STREAM_MODULE = '@weex-module/stream';
var eventTarget = __webpack_require__(/*! event-target-shim */ 2);

var UNSENT = 0;
var OPENED = 1;
var HEADERS_RECEIVED = 2;
var LOADING = 3;
var DONE = 4;

var XHR_EVENTS = ['abort', 'error', 'load', 'loadstart', 'progress', 'timeout', 'loadend', 'readystatechange'];

module.exports = function (__weex_require__) {
  var XMLHttpRequest = function (_eventTarget) {
    _inherits(XMLHttpRequest, _eventTarget);

    // EventTarget automatically initializes these to `null`.
    function XMLHttpRequest() {
      _classCallCheck(this, XMLHttpRequest);

      var _this = _possibleConstructorReturn(this, (XMLHttpRequest.__proto__ || Object.getPrototypeOf(XMLHttpRequest)).call(this));

      _this.UNSENT = UNSENT;
      _this.OPENED = OPENED;
      _this.HEADERS_RECEIVED = HEADERS_RECEIVED;
      _this.LOADING = LOADING;
      _this.DONE = DONE;
      _this.readyState = UNSENT;
      _this.status = 0;
      _this.statusText = '';
      _this.timeout = 0;
      _this._aborted = false;
      _this._hasError = false;
      _this._method = null;
      _this._response = '';
      _this._url = null;
      _this._timedOut = false;

      _this._reset();
      return _this;
    }

    _createClass(XMLHttpRequest, [{
      key: '_reset',
      value: function _reset() {
        this.readyState = this.UNSENT;
        this.responseHeaders = undefined;
        this.status = 0;

        this._hasError = false;
        this._headers = {};
        this._response = '';
        this._responseType = '';
        this._sent = false;
        this._lowerCaseResponseHeaders = {};

        this._timedOut = false;
      }
    }, {
      key: 'getAllResponseHeaders',
      value: function getAllResponseHeaders() {
        if (!this.responseHeaders) {
          // according to the spec, return null if no response has been received
          return null;
        }
        var headers = this.responseHeaders || {};
        return Object.keys(headers).map(function (headerName) {
          return headerName + ': ' + headers[headerName];
        }).join('\r\n');
      }
    }, {
      key: 'getResponseHeader',
      value: function getResponseHeader(header) {
        var value = this._lowerCaseResponseHeaders[header.toLowerCase()];
        return value !== undefined ? value : null;
      }
    }, {
      key: 'setRequestHeader',
      value: function setRequestHeader(header, value) {
        if (this.readyState !== this.OPENED) {
          throw new Error('Request has not been opened');
        }
        this._headers[header.toLowerCase()] = String(value);
      }
    }, {
      key: 'open',
      value: function open(method, url, async) {
        /* Other optional arguments are not supported yet */
        if (this.readyState !== this.UNSENT) {
          throw new Error('Cannot open, already sending');
        }
        if (async !== undefined && !async) {
          // async is default
          throw new Error('Synchronous http requests are not supported');
        }
        if (!url) {
          throw new Error('Cannot load an empty url');
        }
        this._method = method.toUpperCase();
        this._url = url;
        this._aborted = false;
        this.setReadyState(this.OPENED);
      }
    }, {
      key: 'send',
      value: function send(data) {
        var _this2 = this;

        if (this.readyState !== this.OPENED) {
          throw new Error('Request has not been opened');
        }
        if (this._sent) {
          throw new Error('Request has already been sent');
        }
        this._sent = true;

        var nativeFetch = __weex_require__(STREAM_MODULE).fetch;

        nativeFetch({
          method: this._method,
          url: this._url,
          headers: this._headers,
          body: data,
          type: 'text'
        }, function (response) {
          try {
            typeof response === 'string' && (response = JSON.parse(response));

            _this2.status = response.status;
            _this2.statusText = response.statusText;
            _this2.setResponseHeaders(response.headers);

            if (response.ok) {
              _this2._response = response.data;
            } else {
              if (_this2._responseType === '' || _this2._responseType === 'text') {
                _this2._response = response.data;
              }
              _this2._hasError = true;
            }
          } catch (err) {}

          _this2.setReadyState(_this2.DONE);
        }, function (progress) {
          _this2.status = progress.status;
          _this2.statusText = progress.statusText;
          _this2.setResponseHeaders(progress.headers);
          _this2.setReadyState(progress.readyState);
        });
      }
    }, {
      key: 'abort',
      value: function abort() {
        this._aborted = true;

        // TODO: Weex native not support abort now

        // only call onreadystatechange if there is something to abort,
        // below logic is per spec
        if (!(this.readyState === this.UNSENT || this.readyState === this.OPENED && !this._sent || this.readyState === this.DONE)) {
          this._reset();
          this.setReadyState(this.DONE);
        }
        // Reset again after, in case modified in handler
        this._reset();
      }
    }, {
      key: 'setResponseHeaders',
      value: function setResponseHeaders(responseHeaders) {
        this.responseHeaders = responseHeaders || null;
        var headers = responseHeaders || {};
        this._lowerCaseResponseHeaders = Object.keys(headers).reduce(function (lcaseHeaders, headerName) {
          lcaseHeaders[headerName.toLowerCase()] = headers[headerName];
          return lcaseHeaders;
        }, {});
      }
    }, {
      key: 'setReadyState',
      value: function setReadyState(newState) {
        this.readyState = newState;
        this.dispatchEvent({ type: 'readystatechange' });
        if (newState === this.DONE) {
          if (this._aborted) {
            this.dispatchEvent({ type: 'abort' });
          } else if (this._hasError) {
            if (this._timedOut) {
              this.dispatchEvent({ type: 'timeout' });
            } else {
              this.dispatchEvent({ type: 'error' });
            }
          } else {
            this.dispatchEvent({ type: 'load' });
          }
          this.dispatchEvent({ type: 'loadend' });
        }
      }
    }, {
      key: 'responseType',
      get: function get() {
        return this._responseType;
      },
      set: function set(responseType) {
        if (this._sent) {
          throw new Error('Failed to set the \'responseType\' property on \'XMLHttpRequest\': The ' + 'response type cannot be set after the request has been sent.');
        }

        this._responseType = responseType;
      }
    }, {
      key: 'responseText',
      get: function get() {
        if (this._responseType !== '' && this._responseType !== 'text') {
          throw new Error("The 'responseText' property is only available if 'responseType' " + ('is set to \'\' or \'text\', but it is \'' + this._responseType + '\'.'));
        }
        if (this.readyState < LOADING) {
          return '';
        }
        return this._response;
      }
    }, {
      key: 'response',
      get: function get() {
        var responseType = this.responseType;

        if (responseType === '' || responseType === 'text') {
          return this.readyState < LOADING || this._hasError ? '' : this._response;
        }

        if (this.readyState !== DONE) {
          return null;
        }
      }
    }]);

    return XMLHttpRequest;
  }(eventTarget.apply(undefined, XHR_EVENTS));

  XMLHttpRequest.UNSENT = UNSENT;
  XMLHttpRequest.OPENED = OPENED;
  XMLHttpRequest.HEADERS_RECEIVED = HEADERS_RECEIVED;
  XMLHttpRequest.LOADING = LOADING;
  XMLHttpRequest.DONE = DONE;


  return XMLHttpRequest;
};

/***/ }),
/* 15 */
/*!***************************************************************************************************************************************!*\
  !*** ./packages/weex-rax-framework-api/node_modules/.npminstall/event-target-shim/2.0.0/event-target-shim/lib/custom-event-target.js ***!
  \***************************************************************************************************************************************/
/*! dynamic exports provided */
/*! all exports used */
/*! ModuleConcatenation bailout: Module is not an ECMAScript module */
/***/ (function(module, exports, __webpack_require__) {

var Commons = __webpack_require__(/*! ./commons */ 0);
var LISTENERS = Commons.LISTENERS;
var ATTRIBUTE = Commons.ATTRIBUTE;
var newNode = Commons.newNode;

//-----------------------------------------------------------------------------
// Helpers
//-----------------------------------------------------------------------------

/**
 * Gets a specified attribute listener from a given EventTarget object.
 *
 * @param {EventTarget} eventTarget - An EventTarget object to get.
 * @param {string} type - An event type to get.
 * @returns {function|null} The found attribute listener.
 */
function getAttributeListener(eventTarget, type) {
    var node = eventTarget[LISTENERS][type];
    while (node != null) {
        if (node.kind === ATTRIBUTE) {
            return node.listener
        }
        node = node.next;
    }
    return null
}

/**
 * Sets a specified attribute listener to a given EventTarget object.
 *
 * @param {EventTarget} eventTarget - An EventTarget object to set.
 * @param {string} type - An event type to set.
 * @param {function|null} listener - A listener to be set.
 * @returns {void}
 */
function setAttributeListener(eventTarget, type, listener) {
    if (typeof listener !== "function" && typeof listener !== "object") {
        listener = null; // eslint-disable-line no-param-reassign
    }

    var prev = null;
    var node = eventTarget[LISTENERS][type];
    while (node != null) {
        if (node.kind === ATTRIBUTE) {
            // Remove old value.
            if (prev == null) {
                eventTarget[LISTENERS][type] = node.next;
            }
            else {
                prev.next = node.next;
            }
        }
        else {
            prev = node;
        }

        node = node.next;
    }

    // Add new value.
    if (listener != null) {
        if (prev == null) {
            eventTarget[LISTENERS][type] = newNode(listener, ATTRIBUTE);
        }
        else {
            prev.next = newNode(listener, ATTRIBUTE);
        }
    }
}

//-----------------------------------------------------------------------------
// Public Interface
//-----------------------------------------------------------------------------

/**
 * Defines an `EventTarget` implementation which has `onfoobar` attributes.
 *
 * @param {EventTarget} EventTargetBase - A base implementation of EventTarget.
 * @param {string[]} types - A list of event types which are defined as attribute listeners.
 * @returns {EventTarget} The defined `EventTarget` implementation which has attribute listeners.
 */
module.exports.defineCustomEventTarget = function(EventTargetBase, types) {
    /**
     * The constructor of custom event target.
     * @constructor
     */
    function EventTarget() {
        EventTargetBase.call(this);
    }

    var descripter = {
        constructor: {
            value: EventTarget,
            configurable: true,
            writable: true,
        },
    };

    types.forEach(function(type) {
        descripter["on" + type] = {
            get: function() {
                return getAttributeListener(this, type)
            },
            set: function(listener) {
                setAttributeListener(this, type, listener);
            },
            configurable: true,
            enumerable: true,
        };
    });

    EventTarget.prototype = Object.create(EventTargetBase.prototype, descripter);

    return EventTarget
};


/***/ }),
/* 16 */
/*!*********************************************************************************************************************************!*\
  !*** ./packages/weex-rax-framework-api/node_modules/.npminstall/event-target-shim/2.0.0/event-target-shim/lib/event-wrapper.js ***!
  \*********************************************************************************************************************************/
/*! dynamic exports provided */
/*! all exports used */
/*! ModuleConcatenation bailout: Module is not an ECMAScript module */
/***/ (function(module, exports, __webpack_require__) {

var createUniqueKey = __webpack_require__(/*! ./commons */ 0).createUniqueKey;

//-----------------------------------------------------------------------------
// Constsnts
//-----------------------------------------------------------------------------

/**
 * The key of the flag which is turned on by `stopImmediatePropagation` method.
 *
 * @type {symbol|string}
 * @private
 */
var STOP_IMMEDIATE_PROPAGATION_FLAG =
    createUniqueKey("stop_immediate_propagation_flag");

/**
 * The key of the flag which is turned on by `preventDefault` method.
 *
 * @type {symbol|string}
 * @private
 */
var CANCELED_FLAG = createUniqueKey("canceled_flag");

/**
 * The key of the flag that it cannot use `preventDefault` method.
 *
 * @type {symbol|string}
 * @private
 */
var PASSIVE_LISTENER_FLAG = createUniqueKey("passive_listener_flag");

/**
 * The key of the original event object.
 *
 * @type {symbol|string}
 * @private
 */
var ORIGINAL_EVENT = createUniqueKey("original_event");

/**
 * Method definitions for the event wrapper.
 *
 * @type {object}
 * @private
 */
var wrapperPrototypeDefinition = Object.freeze({
    stopPropagation: Object.freeze({
        value: function stopPropagation() {
            var e = this[ORIGINAL_EVENT];
            if (typeof e.stopPropagation === "function") {
                e.stopPropagation();
            }
        },
        writable: true,
        configurable: true,
    }),

    stopImmediatePropagation: Object.freeze({
        value: function stopImmediatePropagation() {
            this[STOP_IMMEDIATE_PROPAGATION_FLAG] = true;

            var e = this[ORIGINAL_EVENT];
            if (typeof e.stopImmediatePropagation === "function") {
                e.stopImmediatePropagation();
            }
        },
        writable: true,
        configurable: true,
    }),

    preventDefault: Object.freeze({
        value: function preventDefault() {
            if (this[PASSIVE_LISTENER_FLAG]) {
                return
            }
            if (this.cancelable === true) {
                this[CANCELED_FLAG] = true;
            }

            var e = this[ORIGINAL_EVENT];
            if (typeof e.preventDefault === "function") {
                e.preventDefault();
            }
        },
        writable: true,
        configurable: true,
    }),

    defaultPrevented: Object.freeze({
        get: function defaultPrevented() {
            return this[CANCELED_FLAG]
        },
        enumerable: true,
        configurable: true,
    }),
});

//-----------------------------------------------------------------------------
// Public Interface
//-----------------------------------------------------------------------------

module.exports.STOP_IMMEDIATE_PROPAGATION_FLAG = STOP_IMMEDIATE_PROPAGATION_FLAG;
module.exports.PASSIVE_LISTENER_FLAG = PASSIVE_LISTENER_FLAG;

/**
 * Creates an event wrapper.
 *
 * We cannot modify several properties of `Event` object, so we need to create the wrapper.
 * Plus, this wrapper supports non `Event` objects.
 *
 * @param {Event|{type: string}} event - An original event to create the wrapper.
 * @param {EventTarget} eventTarget - The event target of the event.
 * @returns {Event} The created wrapper. This object is implemented `Event` interface.
 * @private
 */
module.exports.createEventWrapper = function createEventWrapper(event, eventTarget) {
    var timeStamp = (
        typeof event.timeStamp === "number" ? event.timeStamp : Date.now()
    );
    var propertyDefinition = {
        type: {value: event.type, enumerable: true},
        target: {value: eventTarget, enumerable: true},
        currentTarget: {value: eventTarget, enumerable: true},
        eventPhase: {value: 2, enumerable: true},
        bubbles: {value: Boolean(event.bubbles), enumerable: true},
        cancelable: {value: Boolean(event.cancelable), enumerable: true},
        timeStamp: {value: timeStamp, enumerable: true},
        isTrusted: {value: false, enumerable: true},
    };
    propertyDefinition[STOP_IMMEDIATE_PROPAGATION_FLAG] = {value: false, writable: true};
    propertyDefinition[CANCELED_FLAG] = {value: false, writable: true};
    propertyDefinition[PASSIVE_LISTENER_FLAG] = {value: false, writable: true};
    propertyDefinition[ORIGINAL_EVENT] = {value: event};

    // For CustomEvent.
    if (typeof event.detail !== "undefined") {
        propertyDefinition.detail = {value: event.detail, enumerable: true};
    }

    return Object.create(
        Object.create(event, wrapperPrototypeDefinition),
        propertyDefinition
    )
};


/***/ }),
/* 17 */
/*!***************************************************************!*\
  !*** ./packages/weex-rax-framework-api/src/websocket.weex.js ***!
  \***************************************************************/
/*! dynamic exports provided */
/*! all exports used */
/*! ModuleConcatenation bailout: Module is not an ECMAScript module */
/***/ (function(module, exports, __webpack_require__) {

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) { descriptor.writable = true; } Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) { defineProperties(Constructor.prototype, protoProps); } if (staticProps) { defineProperties(Constructor, staticProps); } return Constructor; }; }();

function _possibleConstructorReturn(self, call) { if (!self) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return call && (typeof call === "object" || typeof call === "function") ? call : self; }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function, not " + typeof superClass); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } }); if (superClass) { Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass; } }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

var eventTarget = __webpack_require__(/*! event-target-shim */ 2);

var WEB_SOCKET_MODULE = '@weex-module/webSocket';

var CONNECTING = 0;
var OPEN = 1;
var CLOSING = 2;
var CLOSED = 3;

var WEBSOCKET_EVENTS = ['close', 'error', 'message', 'open'];

/**
 * Event object passed to the `onopen`, `onclose`, `onmessage`, `onerror`
 * callbacks of `WebSocket`.
 *
 * The `type` property is "open", "close", "message", "error" respectively.
 *
 * In case of "message", the `data` property contains the incoming data.
 */

var WebSocketEvent = function WebSocketEvent(type, eventInitDict) {
  _classCallCheck(this, WebSocketEvent);

  this.type = type.toString();
  Object.assign(this, eventInitDict);
};

module.exports = function (__weex_require__) {
  /**
   * Browser-compatible WebSockets implementation.
   *
   * See https://developer.mozilla.org/en-US/docs/Web/API/WebSocket
   * See https://github.com/websockets/ws
   */
  var WebSocket = function (_eventTarget) {
    _inherits(WebSocket, _eventTarget);

    function WebSocket(url, protocols) {
      _classCallCheck(this, WebSocket);

      var _this = _possibleConstructorReturn(this, (WebSocket.__proto__ || Object.getPrototypeOf(WebSocket)).call(this));

      var websocket = __weex_require__(WEB_SOCKET_MODULE);
      // eslint-disable-next-line new-cap
      websocket.WebSocket(url, protocols);
      _this.readyState = CONNECTING;
      _this.websocket = websocket;

      websocket.onmessage(function (ev) {
        _this.dispatchEvent(new WebSocketEvent('message', ev));
      });

      websocket.onopen(function (ev) {
        _this.readyState = OPEN;
        _this.dispatchEvent(new WebSocketEvent('open'));
      });

      websocket.onclose(function (ev) {
        _this.readyState = CLOSED;
        _this.dispatchEvent(new WebSocketEvent('close', {
          code: ev.code,
          reason: ev.reason
        }));
      });

      websocket.onerror(function (ev) {
        _this.dispatchEvent(new WebSocketEvent('error', ev));
      });
      return _this;
    }

    _createClass(WebSocket, [{
      key: 'close',
      value: function close(code, reason) {
        if (this.readyState === CLOSING || this.readyState === CLOSED) {
          return;
        }

        this.readyState = CLOSING;
        this.websocket.close(code, reason);
      }
    }, {
      key: 'send',
      value: function send(data) {
        if (typeof data === 'string') {
          this.websocket.send(data);
          return;
        }

        throw new Error('Unsupported data type');
      }
    }]);

    return WebSocket;
  }(eventTarget(WEBSOCKET_EVENTS));

  WebSocket.CONNECTING = CONNECTING;
  WebSocket.OPEN = OPEN;
  WebSocket.CLOSING = CLOSING;
  WebSocket.CLOSED = CLOSED;


  return WebSocket;
};

/***/ }),
/* 18 */
/*!***********************************************************!*\
  !*** ./packages/weex-rax-framework-api/src/timer.weex.js ***!
  \***********************************************************/
/*! dynamic exports provided */
/*! all exports used */
/*! ModuleConcatenation bailout: Module is not an ECMAScript module */
/***/ (function(module, exports, __webpack_require__) {

var TIMER_MODULE = '@weex-module/timer';

module.exports = function (__weex_require__, document) {
  var setTimeout = function setTimeout(handler, time) {
    var timer = __weex_require__(TIMER_MODULE);
    timer.setTimeout(handler, time);
    return document.taskCenter.callbackManager.lastCallbackId.toString();
  };

  var setInterval = function setInterval(handler, time) {
    var timer = __weex_require__(TIMER_MODULE);
    timer.setInterval(handler, time);
    return document.taskCenter.callbackManager.lastCallbackId.toString();
  };

  var clearTimeout = function clearTimeout(n) {
    var timer = __weex_require__(TIMER_MODULE);
    timer.clearTimeout(n);
  };

  var clearInterval = function clearInterval(n) {
    var timer = __weex_require__(TIMER_MODULE);
    timer.clearInterval(n);
  };

  var requestAnimationFrame = function requestAnimationFrame(callback) {
    var timer = __weex_require__(TIMER_MODULE);
    return timer.setTimeout(callback, 16);
  };

  var cancelAnimationFrame = function cancelAnimationFrame(n) {
    var timer = __weex_require__(TIMER_MODULE);
    timer.clearTimeout(n);
  };

  return {
    setTimeout: setTimeout,
    clearTimeout: clearTimeout,
    setInterval: setInterval,
    clearInterval: clearInterval,
    requestAnimationFrame: requestAnimationFrame,
    cancelAnimationFrame: cancelAnimationFrame
  };
};

/***/ }),
/* 19 */
/*!************************************************************!*\
  !*** ./packages/weex-rax-framework-api/src/base64.weex.js ***!
  \************************************************************/
/*! dynamic exports provided */
/*! all exports used */
/*! ModuleConcatenation bailout: Module is not an ECMAScript module */
/***/ (function(module, exports, __webpack_require__) {

module.exports = function () {
  var base64 = {};
  var chars = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=';

  // encoder
  // [https://gist.github.com/999166] by [https://github.com/nignag]
  base64.btoa = function (input) {
    var str = String(input);
    for (
    // initialize result and counter
    var block, charCode, idx = 0, map = chars, output = '';
    // if the next str index does not exist:
    //   change the mapping table to "="
    //   check if d has no fractional digits
    str.charAt(idx | 0) || (map = '=', idx % 1);
    // "8 - idx % 1 * 8" generates the sequence 2, 4, 6, 8
    output += map.charAt(63 & block >> 8 - idx % 1 * 8)) {
      charCode = str.charCodeAt(idx += 3 / 4);
      if (charCode > 0xFF) {
        throw new Error("'btoa' failed: The string to be encoded contains characters outside of the Latin1 range.");
      }
      block = block << 8 | charCode;
    }
    return output;
  };

  // decoder
  // [https://gist.github.com/1020396] by [https://github.com/atk]
  base64.atob = function (input) {
    var str = String(input).replace(/=+$/, '');
    if (str.length % 4 == 1) {
      throw new Error("'atob' failed: The string to be decoded is not correctly encoded.");
    }
    for (
    // initialize result and counters
    var bc = 0, bs, buffer, idx = 0, output = '';
    // get next character
    buffer = str.charAt(idx++);
    // character found in table? initialize bit storage and add its ascii value;
    ~buffer && (bs = bc % 4 ? bs * 64 + buffer : buffer, bc++ % 4) ? output += String.fromCharCode(255 & bs >> (-2 * bc & 6)) : 0) {
      // try to find character in table (0-63, not found => -1)
      buffer = chars.indexOf(buffer);
    }
    return output;
  };

  return base64;
};

/***/ }),
/* 20 */
/*!*****************************************************************!*\
  !*** ./packages/weex-rax-framework-api/src/performance.weex.js ***!
  \*****************************************************************/
/*! dynamic exports provided */
/*! all exports used */
/*! ModuleConcatenation bailout: Module is not an ECMAScript module */
/***/ (function(module, exports, __webpack_require__) {

function _defineProperty(obj, key, value) { if (key in obj) { Object.defineProperty(obj, key, { value: value, enumerable: true, configurable: true, writable: true }); } else { obj[key] = value; } return obj; }

module.exports = function (responseEnd) {
  var _performance$timing;

  var performance = {};
  // TODO: current can not get navigationStart time
  performance.timing = (_performance$timing = {
    unloadEventStart: 0,
    unloadEventEnd: 0,
    navigationStart: responseEnd,
    redirectStart: 0,
    redirectEnd: 0,
    fetchStart: responseEnd,
    domainLookupStart: responseEnd,
    domainLookupEnd: responseEnd,
    connectStart: responseEnd,
    secureConnectionStart: responseEnd
  }, _defineProperty(_performance$timing, "connectStart", responseEnd), _defineProperty(_performance$timing, "requestStart", responseEnd), _defineProperty(_performance$timing, "responseStart", responseEnd), _defineProperty(_performance$timing, "responseEnd", responseEnd), _defineProperty(_performance$timing, "domLoading", 0), _defineProperty(_performance$timing, "domInteractive", 0), _defineProperty(_performance$timing, "domComplete", 0), _defineProperty(_performance$timing, "domContentLoadedEventStart", 0), _defineProperty(_performance$timing, "domContentLoadedEventEnd", 0), _defineProperty(_performance$timing, "loadEventStart", 0), _defineProperty(_performance$timing, "loadEventEnd", 0), _performance$timing);
  performance.now = function () {
    return Date.now() - performance.timing.navigationStart;
  };

  return performance;
};

/***/ }),
/* 21 */
/*!***********************************************************!*\
  !*** ./packages/weex-rax-framework-api/src/event.weex.js ***!
  \***********************************************************/
/*! dynamic exports provided */
/*! all exports used */
/*! ModuleConcatenation bailout: Module is not an ECMAScript module */
/***/ (function(module, exports, __webpack_require__) {

function _possibleConstructorReturn(self, call) { if (!self) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return call && (typeof call === "object" || typeof call === "function") ? call : self; }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function, not " + typeof superClass); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } }); if (superClass) { Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass; } }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

module.exports = function () {
  var Event = function Event(type) {
    var params = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : {};

    _classCallCheck(this, Event);

    this.type = type;
    this.bubbles = Boolean(params.bubbles);
    this.cancelable = Boolean(params.cancelable);
  };

  var CustomEvent = function (_Event) {
    _inherits(CustomEvent, _Event);

    function CustomEvent(type) {
      var params = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : {};

      _classCallCheck(this, CustomEvent);

      var _this = _possibleConstructorReturn(this, (CustomEvent.__proto__ || Object.getPrototypeOf(CustomEvent)).call(this, type, params));

      _this.detail = params.detail;
      return _this;
    }

    return CustomEvent;
  }(Event);

  return {
    Event: Event,
    CustomEvent: CustomEvent
  };
};

/***/ })
/******/ ]);

});

var RaxAPI = unwrapExports(api);

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

RaxAPI.injectContext();

})));
//# sourceMappingURL=weex-rax-api.js.map