(this.nativeLog || function(s) {console.log(s)})('START JS FRAMEWORK: 0.15.2-test Build 20160810');
/******/ (function(modules) { // webpackBootstrap
/******/  // The module cache
/******/  var installedModules = {};
/******/
/******/  // The require function
/******/  function __webpack_require__(moduleId) {
/******/
/******/    // Check if module is in cache
/******/    if(installedModules[moduleId])
/******/      return installedModules[moduleId].exports;
/******/
/******/    // Create a new module (and put it into the cache)
/******/    var module = installedModules[moduleId] = {
/******/      exports: {},
/******/      id: moduleId,
/******/      loaded: false
/******/    };
/******/
/******/    // Execute the module function
/******/    modules[moduleId].call(module.exports, module, module.exports, __webpack_require__);
/******/
/******/    // Flag the module as loaded
/******/    module.loaded = true;
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
/******/  // __webpack_public_path__
/******/  __webpack_require__.p = "";
/******/
/******/  // Load entry module and return exports
/******/  return __webpack_require__(0);
/******/ })
/************************************************************************/
/******/ ([
/* 0 */
/***/ function(module, exports, __webpack_require__) {

  /* WEBPACK VAR INJECTION */(function(global) {'use strict';

  __webpack_require__(1);

  /**
   * register methods
   */
  var methods = __webpack_require__(103);
  var _global = global;
  var registerMethods = _global.registerMethods;

  registerMethods(methods);
  /* WEBPACK VAR INJECTION */}.call(exports, (function() { return this; }())))

/***/ },
/* 1 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  __webpack_require__(2);

/***/ },
/* 2 */
/***/ function(module, exports, __webpack_require__) {

  /* WEBPACK VAR INJECTION */(function(global) {'use strict';

  __webpack_require__(3);

  var _runtime = __webpack_require__(76);

  var _runtime2 = _interopRequireDefault(_runtime);

  var _package = __webpack_require__(101);

  var _methods = __webpack_require__(102);

  var methods = _interopRequireWildcard(_methods);

  function _interopRequireWildcard(obj) { if (obj && obj.__esModule) { return obj; } else { var newObj = {}; if (obj != null) { for (var key in obj) { if (Object.prototype.hasOwnProperty.call(obj, key)) newObj[key] = obj[key]; } } newObj.default = obj; return newObj; } }

  function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

  var native = _package.subversion.native;
  var transformer = _package.subversion.transformer;

  // register instance management APIs

  var _loop = function _loop(methodName) {
    global[methodName] = function () {
      var ret = _runtime2.default[methodName].apply(_runtime2.default, arguments);
      if (ret instanceof Error) {
        console.error(ret.toString());
      }
      return ret;
    };
  };

  for (var methodName in _runtime2.default) {
    _loop(methodName);
  }

  // register framework meta info
  global.frameworkVersion = native;
  global.transformerVersion = transformer;

  // register special methods for Weex framework
  global.registerMethods(methods);
  /* WEBPACK VAR INJECTION */}.call(exports, (function() { return this; }())))

/***/ },
/* 3 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  Object.defineProperty(exports, "__esModule", {
    value: true
  });

  var _typeof = typeof Symbol === "function" && typeof Symbol.iterator === "symbol" ? function (obj) { return typeof obj; } : function (obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol ? "symbol" : typeof obj; };

  exports.extend = extend;
  exports.def = def;
  exports.remove = remove;
  exports.hasOwn = hasOwn;
  exports.bind = bind;
  exports.toArray = toArray;
  exports.isObject = isObject;
  exports.isPlainObject = isPlainObject;

  __webpack_require__(4);

  __webpack_require__(5);

  __webpack_require__(69);

  __webpack_require__(70);

  /**
   * Mix properties into target object.
   *
   * @param {Object} to
   * @param {Object} from
   */

  function extend(target) {
    for (var _len = arguments.length, src = Array(_len > 1 ? _len - 1 : 0), _key = 1; _key < _len; _key++) {
      src[_key - 1] = arguments[_key];
    }

    if (typeof Object.assign === 'function') {
      Object.assign.apply(Object, [target].concat(src));
    } else {
      var first = src.shift();
      for (var key in first) {
        target[key] = first[key];
      }
      if (src.length) {
        extend.apply(undefined, [target].concat(src));
      }
    }
    return target;
  }

  /**
   * Define a property.
   *
   * @param {Object} obj
   * @param {String} key
   * @param {*} val
   * @param {Boolean} [enumerable]
   */

  function def(obj, key, val, enumerable) {
    Object.defineProperty(obj, key, {
      value: val,
      enumerable: !!enumerable,
      writable: true,
      configurable: true
    });
  }

  /**
   * Remove an item from an array
   *
   * @param {Array} arr
   * @param {*} item
   */

  function remove(arr, item) {
    if (arr.length) {
      var index = arr.indexOf(item);
      if (index > -1) {
        return arr.splice(index, 1);
      }
    }
  }

  /**
   * Check whether the object has the property.
   *
   * @param {Object} obj
   * @param {String} key
   * @return {Boolean}
   */

  var hasOwnProperty = Object.prototype.hasOwnProperty;
  function hasOwn(obj, key) {
    return hasOwnProperty.call(obj, key);
  }

  /**
   * Simple bind, faster than native
   *
   * @param {Function} fn
   * @param {Object} ctx
   * @return {Function}
   */

  function bind(fn, ctx) {
    return function (a) {
      var l = arguments.length;
      return l ? l > 1 ? fn.apply(ctx, arguments) : fn.call(ctx, a) : fn.call(ctx);
    };
  }

  /**
   * Convert an Array-like object to a real Array.
   *
   * @param {Array-like} list
   * @param {Number} [start] - start index
   * @return {Array}
   */

  function toArray(list, start) {
    start = start || 0;
    var i = list.length - start;
    var ret = new Array(i);
    while (i--) {
      ret[i] = list[i + start];
    }
    return ret;
  }

  /**
   * Quick object check - this is primarily used to tell
   * Objects from primitive values when we know the value
   * is a JSON-compliant type.
   *
   * @param {*} obj
   * @return {Boolean}
   */

  function isObject(obj) {
    return obj !== null && (typeof obj === 'undefined' ? 'undefined' : _typeof(obj)) === 'object';
  }

  /**
   * Strict object type check. Only returns true
   * for plain JavaScript objects.
   *
   * @param {*} obj
   * @return {Boolean}
   */

  var toString = Object.prototype.toString;
  var OBJECT_STRING = '[object Object]';
  function isPlainObject(obj) {
    return toString.call(obj) === OBJECT_STRING;
  }

/***/ },
/* 4 */
/***/ function(module, exports) {

  /* WEBPACK VAR INJECTION */(function(global) {'use strict';

  /**
   * @fileOverview
   * Polyfill `setTimeout` on Android V8 using native method
   * `setTimeoutNative(callbackId, time)` and JS method
   * `setTimeoutCallback(callbackId)`.
   * This polyfill is only used in virtual-DOM diff & flush agorithm. Not
   * accessed by JS Bundle code (The timer APIs polyfill for JS Bundle is in
   * `html5/default/app/ctrl.js`).
   */

  var _global = global;
  var setTimeout = _global.setTimeout;
  var setTimeoutNative = _global.setTimeoutNative;

  /* istanbul ignore if */

  if (typeof setTimeout === 'undefined' && typeof setTimeoutNative === 'function') {
    (function () {
      var timeoutMap = {};
      var timeoutId = 0;

      global.setTimeout = function (cb, time) {
        timeoutMap[++timeoutId] = cb;
        setTimeoutNative(timeoutId.toString(), time);
      };

      global.setTimeoutCallback = function (id) {
        if (typeof timeoutMap[id] === 'function') {
          timeoutMap[id]();
          delete timeoutMap[id];
        }
      };
    })();
  }
  /* WEBPACK VAR INJECTION */}.call(exports, (function() { return this; }())))

/***/ },
/* 5 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  // fix Promise Problem on JSContext of iOS7~8
  // @see https://bugs.webkit.org/show_bug.cgi?id=135866
  // global.Promise = null
  __webpack_require__(6);
  __webpack_require__(26);
  __webpack_require__(52);
  __webpack_require__(56);

/***/ },
/* 6 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';
  // 19.1.3.6 Object.prototype.toString()

  var classof = __webpack_require__(7),
      test = {};
  test[__webpack_require__(9)('toStringTag')] = 'z';
  if (test + '' != '[object z]') {
    __webpack_require__(13)(Object.prototype, 'toString', function toString() {
      return '[object ' + classof(this) + ']';
    }, true);
  }

/***/ },
/* 7 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  // getting tag from 19.1.3.6 Object.prototype.toString()
  var cof = __webpack_require__(8),
      TAG = __webpack_require__(9)('toStringTag')
  // ES3 wrong here
  ,
      ARG = cof(function () {
    return arguments;
  }()) == 'Arguments';

  // fallback for IE11 Script Access Denied error
  var tryGet = function tryGet(it, key) {
    try {
      return it[key];
    } catch (e) {/* empty */}
  };

  module.exports = function (it) {
    var O, T, B;
    return it === undefined ? 'Undefined' : it === null ? 'Null'
    // @@toStringTag case
    : typeof (T = tryGet(O = Object(it), TAG)) == 'string' ? T
    // builtinTag case
    : ARG ? cof(O)
    // ES3 arguments fallback
    : (B = cof(O)) == 'Object' && typeof O.callee == 'function' ? 'Arguments' : B;
  };

/***/ },
/* 8 */
/***/ function(module, exports) {

  "use strict";

  var toString = {}.toString;

  module.exports = function (it) {
    return toString.call(it).slice(8, -1);
  };

/***/ },
/* 9 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  var store = __webpack_require__(10)('wks'),
      uid = __webpack_require__(12),
      _Symbol = __webpack_require__(11).Symbol,
      USE_SYMBOL = typeof _Symbol == 'function';

  var $exports = module.exports = function (name) {
    return store[name] || (store[name] = USE_SYMBOL && _Symbol[name] || (USE_SYMBOL ? _Symbol : uid)('Symbol.' + name));
  };

  $exports.store = store;

/***/ },
/* 10 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  var global = __webpack_require__(11),
      SHARED = '__core-js_shared__',
      store = global[SHARED] || (global[SHARED] = {});
  module.exports = function (key) {
    return store[key] || (store[key] = {});
  };

/***/ },
/* 11 */
/***/ function(module, exports) {

  'use strict';

  // https://github.com/zloirock/core-js/issues/86#issuecomment-115759028
  var global = module.exports = typeof window != 'undefined' && window.Math == Math ? window : typeof self != 'undefined' && self.Math == Math ? self : Function('return this')();
  if (typeof __g == 'number') __g = global; // eslint-disable-line no-undef

/***/ },
/* 12 */
/***/ function(module, exports) {

  'use strict';

  var id = 0,
      px = Math.random();
  module.exports = function (key) {
    return 'Symbol('.concat(key === undefined ? '' : key, ')_', (++id + px).toString(36));
  };

/***/ },
/* 13 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  var global = __webpack_require__(11),
      hide = __webpack_require__(14),
      has = __webpack_require__(24),
      SRC = __webpack_require__(12)('src'),
      TO_STRING = 'toString',
      $toString = Function[TO_STRING],
      TPL = ('' + $toString).split(TO_STRING);

  __webpack_require__(25).inspectSource = function (it) {
    return $toString.call(it);
  };

  (module.exports = function (O, key, val, safe) {
    var isFunction = typeof val == 'function';
    if (isFunction) has(val, 'name') || hide(val, 'name', key);
    if (O[key] === val) return;
    if (isFunction) has(val, SRC) || hide(val, SRC, O[key] ? '' + O[key] : TPL.join(String(key)));
    if (O === global) {
      O[key] = val;
    } else {
      if (!safe) {
        delete O[key];
        hide(O, key, val);
      } else {
        if (O[key]) O[key] = val;else hide(O, key, val);
      }
    }
    // add fake Function#toString for correct work wrapped methods / constructors with methods like LoDash isNative
  })(Function.prototype, TO_STRING, function toString() {
    return typeof this == 'function' && this[SRC] || $toString.call(this);
  });

/***/ },
/* 14 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  var dP = __webpack_require__(15),
      createDesc = __webpack_require__(23);
  module.exports = __webpack_require__(19) ? function (object, key, value) {
    return dP.f(object, key, createDesc(1, value));
  } : function (object, key, value) {
    object[key] = value;
    return object;
  };

/***/ },
/* 15 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  var anObject = __webpack_require__(16),
      IE8_DOM_DEFINE = __webpack_require__(18),
      toPrimitive = __webpack_require__(22),
      dP = Object.defineProperty;

  exports.f = __webpack_require__(19) ? Object.defineProperty : function defineProperty(O, P, Attributes) {
    anObject(O);
    P = toPrimitive(P, true);
    anObject(Attributes);
    if (IE8_DOM_DEFINE) try {
      return dP(O, P, Attributes);
    } catch (e) {/* empty */}
    if ('get' in Attributes || 'set' in Attributes) throw TypeError('Accessors not supported!');
    if ('value' in Attributes) O[P] = Attributes.value;
    return O;
  };

/***/ },
/* 16 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  var isObject = __webpack_require__(17);
  module.exports = function (it) {
    if (!isObject(it)) throw TypeError(it + ' is not an object!');
    return it;
  };

/***/ },
/* 17 */
/***/ function(module, exports) {

  'use strict';

  var _typeof = typeof Symbol === "function" && typeof Symbol.iterator === "symbol" ? function (obj) { return typeof obj; } : function (obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol ? "symbol" : typeof obj; };

  module.exports = function (it) {
    return (typeof it === 'undefined' ? 'undefined' : _typeof(it)) === 'object' ? it !== null : typeof it === 'function';
  };

/***/ },
/* 18 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  module.exports = !__webpack_require__(19) && !__webpack_require__(20)(function () {
    return Object.defineProperty(__webpack_require__(21)('div'), 'a', { get: function get() {
        return 7;
      } }).a != 7;
  });

/***/ },
/* 19 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  // Thank's IE8 for his funny defineProperty
  module.exports = !__webpack_require__(20)(function () {
    return Object.defineProperty({}, 'a', { get: function get() {
        return 7;
      } }).a != 7;
  });

/***/ },
/* 20 */
/***/ function(module, exports) {

  "use strict";

  module.exports = function (exec) {
    try {
      return !!exec();
    } catch (e) {
      return true;
    }
  };

/***/ },
/* 21 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  var isObject = __webpack_require__(17),
      document = __webpack_require__(11).document
  // in old IE typeof document.createElement is 'object'
  ,
      is = isObject(document) && isObject(document.createElement);
  module.exports = function (it) {
    return is ? document.createElement(it) : {};
  };

/***/ },
/* 22 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  // 7.1.1 ToPrimitive(input [, PreferredType])
  var isObject = __webpack_require__(17);
  // instead of the ES6 spec version, we didn't implement @@toPrimitive case
  // and the second argument - flag - preferred type is a string
  module.exports = function (it, S) {
    if (!isObject(it)) return it;
    var fn, val;
    if (S && typeof (fn = it.toString) == 'function' && !isObject(val = fn.call(it))) return val;
    if (typeof (fn = it.valueOf) == 'function' && !isObject(val = fn.call(it))) return val;
    if (!S && typeof (fn = it.toString) == 'function' && !isObject(val = fn.call(it))) return val;
    throw TypeError("Can't convert object to primitive value");
  };

/***/ },
/* 23 */
/***/ function(module, exports) {

  "use strict";

  module.exports = function (bitmap, value) {
    return {
      enumerable: !(bitmap & 1),
      configurable: !(bitmap & 2),
      writable: !(bitmap & 4),
      value: value
    };
  };

/***/ },
/* 24 */
/***/ function(module, exports) {

  "use strict";

  var hasOwnProperty = {}.hasOwnProperty;
  module.exports = function (it, key) {
    return hasOwnProperty.call(it, key);
  };

/***/ },
/* 25 */
/***/ function(module, exports) {

  'use strict';

  var core = module.exports = { version: '2.4.0' };
  if (typeof __e == 'number') __e = core; // eslint-disable-line no-undef

/***/ },
/* 26 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  var $at = __webpack_require__(27)(true);

  // 21.1.3.27 String.prototype[@@iterator]()
  __webpack_require__(30)(String, 'String', function (iterated) {
    this._t = String(iterated); // target
    this._i = 0; // next index
    // 21.1.5.2.1 %StringIteratorPrototype%.next()
  }, function () {
    var O = this._t,
        index = this._i,
        point;
    if (index >= O.length) return { value: undefined, done: true };
    point = $at(O, index);
    this._i += point.length;
    return { value: point, done: false };
  });

/***/ },
/* 27 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  var toInteger = __webpack_require__(28),
      defined = __webpack_require__(29);
  // true  -> String#at
  // false -> String#codePointAt
  module.exports = function (TO_STRING) {
    return function (that, pos) {
      var s = String(defined(that)),
          i = toInteger(pos),
          l = s.length,
          a,
          b;
      if (i < 0 || i >= l) return TO_STRING ? '' : undefined;
      a = s.charCodeAt(i);
      return a < 0xd800 || a > 0xdbff || i + 1 === l || (b = s.charCodeAt(i + 1)) < 0xdc00 || b > 0xdfff ? TO_STRING ? s.charAt(i) : a : TO_STRING ? s.slice(i, i + 2) : (a - 0xd800 << 10) + (b - 0xdc00) + 0x10000;
    };
  };

/***/ },
/* 28 */
/***/ function(module, exports) {

  "use strict";

  // 7.1.4 ToInteger
  var ceil = Math.ceil,
      floor = Math.floor;
  module.exports = function (it) {
    return isNaN(it = +it) ? 0 : (it > 0 ? floor : ceil)(it);
  };

/***/ },
/* 29 */
/***/ function(module, exports) {

  "use strict";

  // 7.2.1 RequireObjectCoercible(argument)
  module.exports = function (it) {
    if (it == undefined) throw TypeError("Can't call method on  " + it);
    return it;
  };

/***/ },
/* 30 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  var LIBRARY = __webpack_require__(31),
      $export = __webpack_require__(32),
      redefine = __webpack_require__(13),
      hide = __webpack_require__(14),
      has = __webpack_require__(24),
      Iterators = __webpack_require__(35),
      $iterCreate = __webpack_require__(36),
      setToStringTag = __webpack_require__(49),
      getPrototypeOf = __webpack_require__(50),
      ITERATOR = __webpack_require__(9)('iterator'),
      BUGGY = !([].keys && 'next' in [].keys()) // Safari has buggy iterators w/o `next`
  ,
      FF_ITERATOR = '@@iterator',
      KEYS = 'keys',
      VALUES = 'values';

  var returnThis = function returnThis() {
    return this;
  };

  module.exports = function (Base, NAME, Constructor, next, DEFAULT, IS_SET, FORCED) {
    $iterCreate(Constructor, NAME, next);
    var getMethod = function getMethod(kind) {
      if (!BUGGY && kind in proto) return proto[kind];
      switch (kind) {
        case KEYS:
          return function keys() {
            return new Constructor(this, kind);
          };
        case VALUES:
          return function values() {
            return new Constructor(this, kind);
          };
      }return function entries() {
        return new Constructor(this, kind);
      };
    };
    var TAG = NAME + ' Iterator',
        DEF_VALUES = DEFAULT == VALUES,
        VALUES_BUG = false,
        proto = Base.prototype,
        $native = proto[ITERATOR] || proto[FF_ITERATOR] || DEFAULT && proto[DEFAULT],
        $default = $native || getMethod(DEFAULT),
        $entries = DEFAULT ? !DEF_VALUES ? $default : getMethod('entries') : undefined,
        $anyNative = NAME == 'Array' ? proto.entries || $native : $native,
        methods,
        key,
        IteratorPrototype;
    // Fix native
    if ($anyNative) {
      IteratorPrototype = getPrototypeOf($anyNative.call(new Base()));
      if (IteratorPrototype !== Object.prototype) {
        // Set @@toStringTag to native iterators
        setToStringTag(IteratorPrototype, TAG, true);
        // fix for some old engines
        if (!LIBRARY && !has(IteratorPrototype, ITERATOR)) hide(IteratorPrototype, ITERATOR, returnThis);
      }
    }
    // fix Array#{values, @@iterator}.name in V8 / FF
    if (DEF_VALUES && $native && $native.name !== VALUES) {
      VALUES_BUG = true;
      $default = function values() {
        return $native.call(this);
      };
    }
    // Define iterator
    if ((!LIBRARY || FORCED) && (BUGGY || VALUES_BUG || !proto[ITERATOR])) {
      hide(proto, ITERATOR, $default);
    }
    // Plug for library
    Iterators[NAME] = $default;
    Iterators[TAG] = returnThis;
    if (DEFAULT) {
      methods = {
        values: DEF_VALUES ? $default : getMethod(VALUES),
        keys: IS_SET ? $default : getMethod(KEYS),
        entries: $entries
      };
      if (FORCED) for (key in methods) {
        if (!(key in proto)) redefine(proto, key, methods[key]);
      } else $export($export.P + $export.F * (BUGGY || VALUES_BUG), NAME, methods);
    }
    return methods;
  };

/***/ },
/* 31 */
/***/ function(module, exports) {

  "use strict";

  module.exports = false;

/***/ },
/* 32 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  var global = __webpack_require__(11),
      core = __webpack_require__(25),
      hide = __webpack_require__(14),
      redefine = __webpack_require__(13),
      ctx = __webpack_require__(33),
      PROTOTYPE = 'prototype';

  var $export = function $export(type, name, source) {
    var IS_FORCED = type & $export.F,
        IS_GLOBAL = type & $export.G,
        IS_STATIC = type & $export.S,
        IS_PROTO = type & $export.P,
        IS_BIND = type & $export.B,
        target = IS_GLOBAL ? global : IS_STATIC ? global[name] || (global[name] = {}) : (global[name] || {})[PROTOTYPE],
        exports = IS_GLOBAL ? core : core[name] || (core[name] = {}),
        expProto = exports[PROTOTYPE] || (exports[PROTOTYPE] = {}),
        key,
        own,
        out,
        exp;
    if (IS_GLOBAL) source = name;
    for (key in source) {
      // contains in native
      own = !IS_FORCED && target && target[key] !== undefined;
      // export native or passed
      out = (own ? target : source)[key];
      // bind timers to global for call from export context
      exp = IS_BIND && own ? ctx(out, global) : IS_PROTO && typeof out == 'function' ? ctx(Function.call, out) : out;
      // extend global
      if (target) redefine(target, key, out, type & $export.U);
      // export
      if (exports[key] != out) hide(exports, key, exp);
      if (IS_PROTO && expProto[key] != out) expProto[key] = out;
    }
  };
  global.core = core;
  // type bitmap
  $export.F = 1; // forced
  $export.G = 2; // global
  $export.S = 4; // static
  $export.P = 8; // proto
  $export.B = 16; // bind
  $export.W = 32; // wrap
  $export.U = 64; // safe
  $export.R = 128; // real proto method for `library`
  module.exports = $export;

/***/ },
/* 33 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  // optional / simple context binding
  var aFunction = __webpack_require__(34);
  module.exports = function (fn, that, length) {
    aFunction(fn);
    if (that === undefined) return fn;
    switch (length) {
      case 1:
        return function (a) {
          return fn.call(that, a);
        };
      case 2:
        return function (a, b) {
          return fn.call(that, a, b);
        };
      case 3:
        return function (a, b, c) {
          return fn.call(that, a, b, c);
        };
    }
    return function () /* ...args */{
      return fn.apply(that, arguments);
    };
  };

/***/ },
/* 34 */
/***/ function(module, exports) {

  'use strict';

  module.exports = function (it) {
    if (typeof it != 'function') throw TypeError(it + ' is not a function!');
    return it;
  };

/***/ },
/* 35 */
/***/ function(module, exports) {

  "use strict";

  module.exports = {};

/***/ },
/* 36 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  var create = __webpack_require__(37),
      descriptor = __webpack_require__(23),
      setToStringTag = __webpack_require__(49),
      IteratorPrototype = {};

  // 25.1.2.1.1 %IteratorPrototype%[@@iterator]()
  __webpack_require__(14)(IteratorPrototype, __webpack_require__(9)('iterator'), function () {
    return this;
  });

  module.exports = function (Constructor, NAME, next) {
    Constructor.prototype = create(IteratorPrototype, { next: descriptor(1, next) });
    setToStringTag(Constructor, NAME + ' Iterator');
  };

/***/ },
/* 37 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  // 19.1.2.2 / 15.2.3.5 Object.create(O [, Properties])
  var anObject = __webpack_require__(16),
      dPs = __webpack_require__(38),
      enumBugKeys = __webpack_require__(47),
      IE_PROTO = __webpack_require__(46)('IE_PROTO'),
      Empty = function Empty() {/* empty */},
      PROTOTYPE = 'prototype';

  // Create object with fake `null` prototype: use iframe Object with cleared prototype
  var _createDict = function createDict() {
    // Thrash, waste and sodomy: IE GC bug
    var iframe = __webpack_require__(21)('iframe'),
        i = enumBugKeys.length,
        lt = '<',
        gt = '>',
        iframeDocument;
    iframe.style.display = 'none';
    __webpack_require__(48).appendChild(iframe);
    iframe.src = 'javascript:'; // eslint-disable-line no-script-url
    // createDict = iframe.contentWindow.Object;
    // html.removeChild(iframe);
    iframeDocument = iframe.contentWindow.document;
    iframeDocument.open();
    iframeDocument.write(lt + 'script' + gt + 'document.F=Object' + lt + '/script' + gt);
    iframeDocument.close();
    _createDict = iframeDocument.F;
    while (i--) {
      delete _createDict[PROTOTYPE][enumBugKeys[i]];
    }return _createDict();
  };

  module.exports = Object.create || function create(O, Properties) {
    var result;
    if (O !== null) {
      Empty[PROTOTYPE] = anObject(O);
      result = new Empty();
      Empty[PROTOTYPE] = null;
      // add "__proto__" for Object.getPrototypeOf polyfill
      result[IE_PROTO] = O;
    } else result = _createDict();
    return Properties === undefined ? result : dPs(result, Properties);
  };

/***/ },
/* 38 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  var dP = __webpack_require__(15),
      anObject = __webpack_require__(16),
      getKeys = __webpack_require__(39);

  module.exports = __webpack_require__(19) ? Object.defineProperties : function defineProperties(O, Properties) {
    anObject(O);
    var keys = getKeys(Properties),
        length = keys.length,
        i = 0,
        P;
    while (length > i) {
      dP.f(O, P = keys[i++], Properties[P]);
    }return O;
  };

/***/ },
/* 39 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  // 19.1.2.14 / 15.2.3.14 Object.keys(O)
  var $keys = __webpack_require__(40),
      enumBugKeys = __webpack_require__(47);

  module.exports = Object.keys || function keys(O) {
    return $keys(O, enumBugKeys);
  };

/***/ },
/* 40 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  var has = __webpack_require__(24),
      toIObject = __webpack_require__(41),
      arrayIndexOf = __webpack_require__(43)(false),
      IE_PROTO = __webpack_require__(46)('IE_PROTO');

  module.exports = function (object, names) {
    var O = toIObject(object),
        i = 0,
        result = [],
        key;
    for (key in O) {
      if (key != IE_PROTO) has(O, key) && result.push(key);
    } // Don't enum bug & hidden keys
    while (names.length > i) {
      if (has(O, key = names[i++])) {
        ~arrayIndexOf(result, key) || result.push(key);
      }
    }return result;
  };

/***/ },
/* 41 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  // to indexed object, toObject with fallback for non-array-like ES3 strings
  var IObject = __webpack_require__(42),
      defined = __webpack_require__(29);
  module.exports = function (it) {
    return IObject(defined(it));
  };

/***/ },
/* 42 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  // fallback for non-array-like ES3 and non-enumerable old V8 strings
  var cof = __webpack_require__(8);
  module.exports = Object('z').propertyIsEnumerable(0) ? Object : function (it) {
    return cof(it) == 'String' ? it.split('') : Object(it);
  };

/***/ },
/* 43 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  // false -> Array#indexOf
  // true  -> Array#includes
  var toIObject = __webpack_require__(41),
      toLength = __webpack_require__(44),
      toIndex = __webpack_require__(45);
  module.exports = function (IS_INCLUDES) {
    return function ($this, el, fromIndex) {
      var O = toIObject($this),
          length = toLength(O.length),
          index = toIndex(fromIndex, length),
          value;
      // Array#includes uses SameValueZero equality algorithm
      if (IS_INCLUDES && el != el) while (length > index) {
        value = O[index++];
        if (value != value) return true;
        // Array#toIndex ignores holes, Array#includes - not
      } else for (; length > index; index++) {
        if (IS_INCLUDES || index in O) {
          if (O[index] === el) return IS_INCLUDES || index || 0;
        }
      }return !IS_INCLUDES && -1;
    };
  };

/***/ },
/* 44 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  // 7.1.15 ToLength
  var toInteger = __webpack_require__(28),
      min = Math.min;
  module.exports = function (it) {
    return it > 0 ? min(toInteger(it), 0x1fffffffffffff) : 0; // pow(2, 53) - 1 == 9007199254740991
  };

/***/ },
/* 45 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  var toInteger = __webpack_require__(28),
      max = Math.max,
      min = Math.min;
  module.exports = function (index, length) {
    index = toInteger(index);
    return index < 0 ? max(index + length, 0) : min(index, length);
  };

/***/ },
/* 46 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  var shared = __webpack_require__(10)('keys'),
      uid = __webpack_require__(12);
  module.exports = function (key) {
    return shared[key] || (shared[key] = uid(key));
  };

/***/ },
/* 47 */
/***/ function(module, exports) {

  'use strict';

  // IE 8- don't enum bug keys
  module.exports = 'constructor,hasOwnProperty,isPrototypeOf,propertyIsEnumerable,toLocaleString,toString,valueOf'.split(',');

/***/ },
/* 48 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  module.exports = __webpack_require__(11).document && document.documentElement;

/***/ },
/* 49 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  var def = __webpack_require__(15).f,
      has = __webpack_require__(24),
      TAG = __webpack_require__(9)('toStringTag');

  module.exports = function (it, tag, stat) {
    if (it && !has(it = stat ? it : it.prototype, TAG)) def(it, TAG, { configurable: true, value: tag });
  };

/***/ },
/* 50 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  // 19.1.2.9 / 15.2.3.2 Object.getPrototypeOf(O)
  var has = __webpack_require__(24),
      toObject = __webpack_require__(51),
      IE_PROTO = __webpack_require__(46)('IE_PROTO'),
      ObjectProto = Object.prototype;

  module.exports = Object.getPrototypeOf || function (O) {
    O = toObject(O);
    if (has(O, IE_PROTO)) return O[IE_PROTO];
    if (typeof O.constructor == 'function' && O instanceof O.constructor) {
      return O.constructor.prototype;
    }return O instanceof Object ? ObjectProto : null;
  };

/***/ },
/* 51 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  // 7.1.13 ToObject(argument)
  var defined = __webpack_require__(29);
  module.exports = function (it) {
    return Object(defined(it));
  };

/***/ },
/* 52 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  var $iterators = __webpack_require__(53),
      redefine = __webpack_require__(13),
      global = __webpack_require__(11),
      hide = __webpack_require__(14),
      Iterators = __webpack_require__(35),
      wks = __webpack_require__(9),
      ITERATOR = wks('iterator'),
      TO_STRING_TAG = wks('toStringTag'),
      ArrayValues = Iterators.Array;

  for (var collections = ['NodeList', 'DOMTokenList', 'MediaList', 'StyleSheetList', 'CSSRuleList'], i = 0; i < 5; i++) {
    var NAME = collections[i],
        Collection = global[NAME],
        proto = Collection && Collection.prototype,
        key;
    if (proto) {
      if (!proto[ITERATOR]) hide(proto, ITERATOR, ArrayValues);
      if (!proto[TO_STRING_TAG]) hide(proto, TO_STRING_TAG, NAME);
      Iterators[NAME] = ArrayValues;
      for (key in $iterators) {
        if (!proto[key]) redefine(proto, key, $iterators[key], true);
      }
    }
  }

/***/ },
/* 53 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  var addToUnscopables = __webpack_require__(54),
      step = __webpack_require__(55),
      Iterators = __webpack_require__(35),
      toIObject = __webpack_require__(41);

  // 22.1.3.4 Array.prototype.entries()
  // 22.1.3.13 Array.prototype.keys()
  // 22.1.3.29 Array.prototype.values()
  // 22.1.3.30 Array.prototype[@@iterator]()
  module.exports = __webpack_require__(30)(Array, 'Array', function (iterated, kind) {
    this._t = toIObject(iterated); // target
    this._i = 0; // next index
    this._k = kind; // kind
    // 22.1.5.2.1 %ArrayIteratorPrototype%.next()
  }, function () {
    var O = this._t,
        kind = this._k,
        index = this._i++;
    if (!O || index >= O.length) {
      this._t = undefined;
      return step(1);
    }
    if (kind == 'keys') return step(0, index);
    if (kind == 'values') return step(0, O[index]);
    return step(0, [index, O[index]]);
  }, 'values');

  // argumentsList[@@iterator] is %ArrayProto_values% (9.4.4.6, 9.4.4.7)
  Iterators.Arguments = Iterators.Array;

  addToUnscopables('keys');
  addToUnscopables('values');
  addToUnscopables('entries');

/***/ },
/* 54 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  // 22.1.3.31 Array.prototype[@@unscopables]
  var UNSCOPABLES = __webpack_require__(9)('unscopables'),
      ArrayProto = Array.prototype;
  if (ArrayProto[UNSCOPABLES] == undefined) __webpack_require__(14)(ArrayProto, UNSCOPABLES, {});
  module.exports = function (key) {
    ArrayProto[UNSCOPABLES][key] = true;
  };

/***/ },
/* 55 */
/***/ function(module, exports) {

  "use strict";

  module.exports = function (done, value) {
    return { value: value, done: !!done };
  };

/***/ },
/* 56 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  var LIBRARY = __webpack_require__(31),
      global = __webpack_require__(11),
      ctx = __webpack_require__(33),
      classof = __webpack_require__(7),
      $export = __webpack_require__(32),
      isObject = __webpack_require__(17),
      aFunction = __webpack_require__(34),
      anInstance = __webpack_require__(57),
      forOf = __webpack_require__(58),
      speciesConstructor = __webpack_require__(62),
      task = __webpack_require__(63).set,
      microtask = __webpack_require__(65)(),
      PROMISE = 'Promise',
      TypeError = global.TypeError,
      process = global.process,
      $Promise = global[PROMISE],
      process = global.process,
      isNode = classof(process) == 'process',
      empty = function empty() {/* empty */},
      Internal,
      GenericPromiseCapability,
      Wrapper;

  var USE_NATIVE = !!function () {
    try {
      // correct subclassing with @@species support
      var promise = $Promise.resolve(1),
          FakePromise = (promise.constructor = {})[__webpack_require__(9)('species')] = function (exec) {
        exec(empty, empty);
      };
      // unhandled rejections tracking support, NodeJS Promise without it fails @@species test
      return (isNode || typeof PromiseRejectionEvent == 'function') && promise.then(empty) instanceof FakePromise;
    } catch (e) {/* empty */}
  }();

  // helpers
  var sameConstructor = function sameConstructor(a, b) {
    // with library wrapper special case
    return a === b || a === $Promise && b === Wrapper;
  };
  var isThenable = function isThenable(it) {
    var then;
    return isObject(it) && typeof (then = it.then) == 'function' ? then : false;
  };
  var newPromiseCapability = function newPromiseCapability(C) {
    return sameConstructor($Promise, C) ? new PromiseCapability(C) : new GenericPromiseCapability(C);
  };
  var PromiseCapability = GenericPromiseCapability = function GenericPromiseCapability(C) {
    var resolve, reject;
    this.promise = new C(function ($$resolve, $$reject) {
      if (resolve !== undefined || reject !== undefined) throw TypeError('Bad Promise constructor');
      resolve = $$resolve;
      reject = $$reject;
    });
    this.resolve = aFunction(resolve);
    this.reject = aFunction(reject);
  };
  var perform = function perform(exec) {
    try {
      exec();
    } catch (e) {
      return { error: e };
    }
  };
  var notify = function notify(promise, isReject) {
    if (promise._n) return;
    promise._n = true;
    var chain = promise._c;
    microtask(function () {
      var value = promise._v,
          ok = promise._s == 1,
          i = 0;
      var run = function run(reaction) {
        var handler = ok ? reaction.ok : reaction.fail,
            resolve = reaction.resolve,
            reject = reaction.reject,
            domain = reaction.domain,
            result,
            then;
        try {
          if (handler) {
            if (!ok) {
              if (promise._h == 2) onHandleUnhandled(promise);
              promise._h = 1;
            }
            if (handler === true) result = value;else {
              if (domain) domain.enter();
              result = handler(value);
              if (domain) domain.exit();
            }
            if (result === reaction.promise) {
              reject(TypeError('Promise-chain cycle'));
            } else if (then = isThenable(result)) {
              then.call(result, resolve, reject);
            } else resolve(result);
          } else reject(value);
        } catch (e) {
          reject(e);
        }
      };
      while (chain.length > i) {
        run(chain[i++]);
      } // variable length - can't use forEach
      promise._c = [];
      promise._n = false;
      if (isReject && !promise._h) onUnhandled(promise);
    });
  };
  var onUnhandled = function onUnhandled(promise) {
    task.call(global, function () {
      var value = promise._v,
          abrupt,
          handler,
          console;
      if (isUnhandled(promise)) {
        abrupt = perform(function () {
          if (isNode) {
            process.emit('unhandledRejection', value, promise);
          } else if (handler = global.onunhandledrejection) {
            handler({ promise: promise, reason: value });
          } else if ((console = global.console) && console.error) {
            console.error('Unhandled promise rejection', value);
          }
        });
        // Browsers should not trigger `rejectionHandled` event if it was handled here, NodeJS - should
        promise._h = isNode || isUnhandled(promise) ? 2 : 1;
      }promise._a = undefined;
      if (abrupt) throw abrupt.error;
    });
  };
  var isUnhandled = function isUnhandled(promise) {
    if (promise._h == 1) return false;
    var chain = promise._a || promise._c,
        i = 0,
        reaction;
    while (chain.length > i) {
      reaction = chain[i++];
      if (reaction.fail || !isUnhandled(reaction.promise)) return false;
    }return true;
  };
  var onHandleUnhandled = function onHandleUnhandled(promise) {
    task.call(global, function () {
      var handler;
      if (isNode) {
        process.emit('rejectionHandled', promise);
      } else if (handler = global.onrejectionhandled) {
        handler({ promise: promise, reason: promise._v });
      }
    });
  };
  var $reject = function $reject(value) {
    var promise = this;
    if (promise._d) return;
    promise._d = true;
    promise = promise._w || promise; // unwrap
    promise._v = value;
    promise._s = 2;
    if (!promise._a) promise._a = promise._c.slice();
    notify(promise, true);
  };
  var $resolve = function $resolve(value) {
    var promise = this,
        then;
    if (promise._d) return;
    promise._d = true;
    promise = promise._w || promise; // unwrap
    try {
      if (promise === value) throw TypeError("Promise can't be resolved itself");
      if (then = isThenable(value)) {
        microtask(function () {
          var wrapper = { _w: promise, _d: false }; // wrap
          try {
            then.call(value, ctx($resolve, wrapper, 1), ctx($reject, wrapper, 1));
          } catch (e) {
            $reject.call(wrapper, e);
          }
        });
      } else {
        promise._v = value;
        promise._s = 1;
        notify(promise, false);
      }
    } catch (e) {
      $reject.call({ _w: promise, _d: false }, e); // wrap
    }
  };

  // constructor polyfill
  if (!USE_NATIVE) {
    // 25.4.3.1 Promise(executor)
    $Promise = function Promise(executor) {
      anInstance(this, $Promise, PROMISE, '_h');
      aFunction(executor);
      Internal.call(this);
      try {
        executor(ctx($resolve, this, 1), ctx($reject, this, 1));
      } catch (err) {
        $reject.call(this, err);
      }
    };
    Internal = function Promise(executor) {
      this._c = []; // <- awaiting reactions
      this._a = undefined; // <- checked in isUnhandled reactions
      this._s = 0; // <- state
      this._d = false; // <- done
      this._v = undefined; // <- value
      this._h = 0; // <- rejection state, 0 - default, 1 - handled, 2 - unhandled
      this._n = false; // <- notify
    };
    Internal.prototype = __webpack_require__(66)($Promise.prototype, {
      // 25.4.5.3 Promise.prototype.then(onFulfilled, onRejected)
      then: function then(onFulfilled, onRejected) {
        var reaction = newPromiseCapability(speciesConstructor(this, $Promise));
        reaction.ok = typeof onFulfilled == 'function' ? onFulfilled : true;
        reaction.fail = typeof onRejected == 'function' && onRejected;
        reaction.domain = isNode ? process.domain : undefined;
        this._c.push(reaction);
        if (this._a) this._a.push(reaction);
        if (this._s) notify(this, false);
        return reaction.promise;
      },
      // 25.4.5.1 Promise.prototype.catch(onRejected)
      'catch': function _catch(onRejected) {
        return this.then(undefined, onRejected);
      }
    });
    PromiseCapability = function PromiseCapability() {
      var promise = new Internal();
      this.promise = promise;
      this.resolve = ctx($resolve, promise, 1);
      this.reject = ctx($reject, promise, 1);
    };
  }

  $export($export.G + $export.W + $export.F * !USE_NATIVE, { Promise: $Promise });
  __webpack_require__(49)($Promise, PROMISE);
  __webpack_require__(67)(PROMISE);
  Wrapper = __webpack_require__(25)[PROMISE];

  // statics
  $export($export.S + $export.F * !USE_NATIVE, PROMISE, {
    // 25.4.4.5 Promise.reject(r)
    reject: function reject(r) {
      var capability = newPromiseCapability(this),
          $$reject = capability.reject;
      $$reject(r);
      return capability.promise;
    }
  });
  $export($export.S + $export.F * (LIBRARY || !USE_NATIVE), PROMISE, {
    // 25.4.4.6 Promise.resolve(x)
    resolve: function resolve(x) {
      // instanceof instead of internal slot check because we should fix it without replacement native Promise core
      if (x instanceof $Promise && sameConstructor(x.constructor, this)) return x;
      var capability = newPromiseCapability(this),
          $$resolve = capability.resolve;
      $$resolve(x);
      return capability.promise;
    }
  });
  $export($export.S + $export.F * !(USE_NATIVE && __webpack_require__(68)(function (iter) {
    $Promise.all(iter)['catch'](empty);
  })), PROMISE, {
    // 25.4.4.1 Promise.all(iterable)
    all: function all(iterable) {
      var C = this,
          capability = newPromiseCapability(C),
          resolve = capability.resolve,
          reject = capability.reject;
      var abrupt = perform(function () {
        var values = [],
            index = 0,
            remaining = 1;
        forOf(iterable, false, function (promise) {
          var $index = index++,
              alreadyCalled = false;
          values.push(undefined);
          remaining++;
          C.resolve(promise).then(function (value) {
            if (alreadyCalled) return;
            alreadyCalled = true;
            values[$index] = value;
            --remaining || resolve(values);
          }, reject);
        });
        --remaining || resolve(values);
      });
      if (abrupt) reject(abrupt.error);
      return capability.promise;
    },
    // 25.4.4.4 Promise.race(iterable)
    race: function race(iterable) {
      var C = this,
          capability = newPromiseCapability(C),
          reject = capability.reject;
      var abrupt = perform(function () {
        forOf(iterable, false, function (promise) {
          C.resolve(promise).then(capability.resolve, reject);
        });
      });
      if (abrupt) reject(abrupt.error);
      return capability.promise;
    }
  });

/***/ },
/* 57 */
/***/ function(module, exports) {

  'use strict';

  module.exports = function (it, Constructor, name, forbiddenField) {
    if (!(it instanceof Constructor) || forbiddenField !== undefined && forbiddenField in it) {
      throw TypeError(name + ': incorrect invocation!');
    }return it;
  };

/***/ },
/* 58 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  var ctx = __webpack_require__(33),
      call = __webpack_require__(59),
      isArrayIter = __webpack_require__(60),
      anObject = __webpack_require__(16),
      toLength = __webpack_require__(44),
      getIterFn = __webpack_require__(61),
      BREAK = {},
      RETURN = {};
  var _exports = module.exports = function (iterable, entries, fn, that, ITERATOR) {
    var iterFn = ITERATOR ? function () {
      return iterable;
    } : getIterFn(iterable),
        f = ctx(fn, that, entries ? 2 : 1),
        index = 0,
        length,
        step,
        iterator,
        result;
    if (typeof iterFn != 'function') throw TypeError(iterable + ' is not iterable!');
    // fast case for arrays with default iterator
    if (isArrayIter(iterFn)) for (length = toLength(iterable.length); length > index; index++) {
      result = entries ? f(anObject(step = iterable[index])[0], step[1]) : f(iterable[index]);
      if (result === BREAK || result === RETURN) return result;
    } else for (iterator = iterFn.call(iterable); !(step = iterator.next()).done;) {
      result = call(iterator, f, step.value, entries);
      if (result === BREAK || result === RETURN) return result;
    }
  };
  _exports.BREAK = BREAK;
  _exports.RETURN = RETURN;

/***/ },
/* 59 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  // call something on iterator step with safe closing on error
  var anObject = __webpack_require__(16);
  module.exports = function (iterator, fn, value, entries) {
    try {
      return entries ? fn(anObject(value)[0], value[1]) : fn(value);
      // 7.4.6 IteratorClose(iterator, completion)
    } catch (e) {
      var ret = iterator['return'];
      if (ret !== undefined) anObject(ret.call(iterator));
      throw e;
    }
  };

/***/ },
/* 60 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  // check on default Array iterator
  var Iterators = __webpack_require__(35),
      ITERATOR = __webpack_require__(9)('iterator'),
      ArrayProto = Array.prototype;

  module.exports = function (it) {
    return it !== undefined && (Iterators.Array === it || ArrayProto[ITERATOR] === it);
  };

/***/ },
/* 61 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  var classof = __webpack_require__(7),
      ITERATOR = __webpack_require__(9)('iterator'),
      Iterators = __webpack_require__(35);
  module.exports = __webpack_require__(25).getIteratorMethod = function (it) {
    if (it != undefined) return it[ITERATOR] || it['@@iterator'] || Iterators[classof(it)];
  };

/***/ },
/* 62 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  // 7.3.20 SpeciesConstructor(O, defaultConstructor)
  var anObject = __webpack_require__(16),
      aFunction = __webpack_require__(34),
      SPECIES = __webpack_require__(9)('species');
  module.exports = function (O, D) {
    var C = anObject(O).constructor,
        S;
    return C === undefined || (S = anObject(C)[SPECIES]) == undefined ? D : aFunction(S);
  };

/***/ },
/* 63 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  var ctx = __webpack_require__(33),
      invoke = __webpack_require__(64),
      html = __webpack_require__(48),
      cel = __webpack_require__(21),
      global = __webpack_require__(11),
      process = global.process,
      setTask = global.setImmediate,
      clearTask = global.clearImmediate,
      MessageChannel = global.MessageChannel,
      counter = 0,
      queue = {},
      ONREADYSTATECHANGE = 'onreadystatechange',
      defer,
      channel,
      port;
  var run = function run() {
    var id = +this;
    if (queue.hasOwnProperty(id)) {
      var fn = queue[id];
      delete queue[id];
      fn();
    }
  };
  var listener = function listener(event) {
    run.call(event.data);
  };
  // Node.js 0.9+ & IE10+ has setImmediate, otherwise:
  if (!setTask || !clearTask) {
    setTask = function setImmediate(fn) {
      var args = [],
          i = 1;
      while (arguments.length > i) {
        args.push(arguments[i++]);
      }queue[++counter] = function () {
        invoke(typeof fn == 'function' ? fn : Function(fn), args);
      };
      defer(counter);
      return counter;
    };
    clearTask = function clearImmediate(id) {
      delete queue[id];
    };
    // Node.js 0.8-
    if (__webpack_require__(8)(process) == 'process') {
      defer = function defer(id) {
        process.nextTick(ctx(run, id, 1));
      };
      // Browsers with MessageChannel, includes WebWorkers
    } else if (MessageChannel) {
      channel = new MessageChannel();
      port = channel.port2;
      channel.port1.onmessage = listener;
      defer = ctx(port.postMessage, port, 1);
      // Browsers with postMessage, skip WebWorkers
      // IE8 has postMessage, but it's sync & typeof its postMessage is 'object'
    } else if (global.addEventListener && typeof postMessage == 'function' && !global.importScripts) {
      defer = function defer(id) {
        global.postMessage(id + '', '*');
      };
      global.addEventListener('message', listener, false);
      // IE8-
    } else if (ONREADYSTATECHANGE in cel('script')) {
      defer = function defer(id) {
        html.appendChild(cel('script'))[ONREADYSTATECHANGE] = function () {
          html.removeChild(this);
          run.call(id);
        };
      };
      // Rest old browsers
    } else {
      defer = function defer(id) {
        setTimeout(ctx(run, id, 1), 0);
      };
    }
  }
  module.exports = {
    set: setTask,
    clear: clearTask
  };

/***/ },
/* 64 */
/***/ function(module, exports) {

  "use strict";

  // fast apply, http://jsperf.lnkit.com/fast-apply/5
  module.exports = function (fn, args, that) {
                    var un = that === undefined;
                    switch (args.length) {
                                      case 0:
                                                        return un ? fn() : fn.call(that);
                                      case 1:
                                                        return un ? fn(args[0]) : fn.call(that, args[0]);
                                      case 2:
                                                        return un ? fn(args[0], args[1]) : fn.call(that, args[0], args[1]);
                                      case 3:
                                                        return un ? fn(args[0], args[1], args[2]) : fn.call(that, args[0], args[1], args[2]);
                                      case 4:
                                                        return un ? fn(args[0], args[1], args[2], args[3]) : fn.call(that, args[0], args[1], args[2], args[3]);
                    }return fn.apply(that, args);
  };

/***/ },
/* 65 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  var global = __webpack_require__(11),
      macrotask = __webpack_require__(63).set,
      Observer = global.MutationObserver || global.WebKitMutationObserver,
      process = global.process,
      Promise = global.Promise,
      isNode = __webpack_require__(8)(process) == 'process';

  module.exports = function () {
    var head, last, notify;

    var flush = function flush() {
      var parent, fn;
      if (isNode && (parent = process.domain)) parent.exit();
      while (head) {
        fn = head.fn;
        head = head.next;
        try {
          fn();
        } catch (e) {
          if (head) notify();else last = undefined;
          throw e;
        }
      }last = undefined;
      if (parent) parent.enter();
    };

    // Node.js
    if (isNode) {
      notify = function notify() {
        process.nextTick(flush);
      };
      // browsers with MutationObserver
    } else if (Observer) {
      var toggle = true,
          node = document.createTextNode('');
      new Observer(flush).observe(node, { characterData: true }); // eslint-disable-line no-new
      notify = function notify() {
        node.data = toggle = !toggle;
      };
      // environments with maybe non-completely correct, but existent Promise
    } else if (Promise && Promise.resolve) {
      var promise = Promise.resolve();
      notify = function notify() {
        promise.then(flush);
      };
      // for other environments - macrotask based on:
      // - setImmediate
      // - MessageChannel
      // - window.postMessag
      // - onreadystatechange
      // - setTimeout
    } else {
      notify = function notify() {
        // strange IE + webpack dev server bug - use .call(global)
        macrotask.call(global, flush);
      };
    }

    return function (fn) {
      var task = { fn: fn, next: undefined };
      if (last) last.next = task;
      if (!head) {
        head = task;
        notify();
      }last = task;
    };
  };

/***/ },
/* 66 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  var redefine = __webpack_require__(13);
  module.exports = function (target, src, safe) {
    for (var key in src) {
      redefine(target, key, src[key], safe);
    }return target;
  };

/***/ },
/* 67 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  var global = __webpack_require__(11),
      dP = __webpack_require__(15),
      DESCRIPTORS = __webpack_require__(19),
      SPECIES = __webpack_require__(9)('species');

  module.exports = function (KEY) {
    var C = global[KEY];
    if (DESCRIPTORS && C && !C[SPECIES]) dP.f(C, SPECIES, {
      configurable: true,
      get: function get() {
        return this;
      }
    });
  };

/***/ },
/* 68 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  var ITERATOR = __webpack_require__(9)('iterator'),
      SAFE_CLOSING = false;

  try {
    var riter = [7][ITERATOR]();
    riter['return'] = function () {
      SAFE_CLOSING = true;
    };
    Array.from(riter, function () {
      throw 2;
    });
  } catch (e) {/* empty */}

  module.exports = function (exec, skipClosing) {
    if (!skipClosing && !SAFE_CLOSING) return false;
    var safe = false;
    try {
      var arr = [7],
          iter = arr[ITERATOR]();
      iter.next = function () {
        return { done: safe = true };
      };
      arr[ITERATOR] = function () {
        return iter;
      };
      exec(arr);
    } catch (e) {/* empty */}
    return safe;
  };

/***/ },
/* 69 */
/***/ function(module, exports) {

  /* WEBPACK VAR INJECTION */(function(global) {'use strict';

  function _toConsumableArray(arr) { if (Array.isArray(arr)) { for (var i = 0, arr2 = Array(arr.length); i < arr.length; i++) { arr2[i] = arr[i]; } return arr2; } else { return Array.from(arr); } }

  /**
   * @fileOverview
   * This file will hack `console` methods by `WXEnvironment.logLevel`.
   * So we can control how many and which messages will be sent by change the log level.
   * Additionally in native platform the message content must be primitive values and
   * using `nativeLog(...args, logLevelMark)` so we create a new `console` object in
   * global add a format process for its methods.
   */

  var _global = global;
  var console = _global.console;
  var nativeLog = _global.nativeLog;

  var LEVELS = ['error', 'warn', 'info', 'log', 'debug'];
  var levelMap = {};

  generateLevelMap();

  /* istanbul ignore if */
  if (typeof console === 'undefined' || // Android
  global.WXEnvironment && global.WXEnvironment.platform === 'iOS' // iOS
  ) {
      global.console = {
        debug: function debug() {
          for (var _len = arguments.length, args = Array(_len), _key = 0; _key < _len; _key++) {
            args[_key] = arguments[_key];
          }

          if (checkLevel('debug')) {
            nativeLog.apply(undefined, _toConsumableArray(format(args)).concat(['__DEBUG']));
          }
        },
        log: function log() {
          for (var _len2 = arguments.length, args = Array(_len2), _key2 = 0; _key2 < _len2; _key2++) {
            args[_key2] = arguments[_key2];
          }

          if (checkLevel('log')) {
            nativeLog.apply(undefined, _toConsumableArray(format(args)).concat(['__LOG']));
          }
        },
        info: function info() {
          for (var _len3 = arguments.length, args = Array(_len3), _key3 = 0; _key3 < _len3; _key3++) {
            args[_key3] = arguments[_key3];
          }

          if (checkLevel('info')) {
            nativeLog.apply(undefined, _toConsumableArray(format(args)).concat(['__INFO']));
          }
        },
        warn: function warn() {
          for (var _len4 = arguments.length, args = Array(_len4), _key4 = 0; _key4 < _len4; _key4++) {
            args[_key4] = arguments[_key4];
          }

          if (checkLevel('warn')) {
            nativeLog.apply(undefined, _toConsumableArray(format(args)).concat(['__WARN']));
          }
        },
        error: function error() {
          for (var _len5 = arguments.length, args = Array(_len5), _key5 = 0; _key5 < _len5; _key5++) {
            args[_key5] = arguments[_key5];
          }

          if (checkLevel('error')) {
            nativeLog.apply(undefined, _toConsumableArray(format(args)).concat(['__ERROR']));
          }
        }
      };
    } else {
    // HTML5
    var debug = console.debug;
    var log = console.log;
    var info = console.info;
    var warn = console.warn;
    var error = console.error;

    console.__ori__ = { debug: debug, log: log, info: info, warn: warn, error: error };
    console.debug = function () {
      for (var _len6 = arguments.length, args = Array(_len6), _key6 = 0; _key6 < _len6; _key6++) {
        args[_key6] = arguments[_key6];
      }

      if (checkLevel('debug')) {
        console.__ori__.debug.apply(console, args);
      }
    };
    console.log = function () {
      for (var _len7 = arguments.length, args = Array(_len7), _key7 = 0; _key7 < _len7; _key7++) {
        args[_key7] = arguments[_key7];
      }

      if (checkLevel('log')) {
        console.__ori__.log.apply(console, args);
      }
    };
    console.info = function () {
      for (var _len8 = arguments.length, args = Array(_len8), _key8 = 0; _key8 < _len8; _key8++) {
        args[_key8] = arguments[_key8];
      }

      if (checkLevel('info')) {
        console.__ori__.info.apply(console, args);
      }
    };
    console.warn = function () {
      for (var _len9 = arguments.length, args = Array(_len9), _key9 = 0; _key9 < _len9; _key9++) {
        args[_key9] = arguments[_key9];
      }

      if (checkLevel('warn')) {
        console.__ori__.warn.apply(console, args);
      }
    };
    console.error = function () {
      for (var _len10 = arguments.length, args = Array(_len10), _key10 = 0; _key10 < _len10; _key10++) {
        args[_key10] = arguments[_key10];
      }

      if (checkLevel('error')) {
        console.__ori__.error.apply(console, args);
      }
    };
  }

  /**
   * Generate map for which types of message will be sent in a certain message level
   * as the order of LEVELS.
   */
  function generateLevelMap() {
    LEVELS.forEach(function (level) {
      var levelIndex = LEVELS.indexOf(level);
      levelMap[level] = {};
      LEVELS.forEach(function (type) {
        var typeIndex = LEVELS.indexOf(type);
        if (typeIndex <= levelIndex) {
          levelMap[level][type] = true;
        }
      });
    });
  }

  /**
   * Check if a certain type of message will be sent in current log level of env.
   * @param  {string} type
   * @return {boolean}
   */
  function checkLevel(type) {
    var logLevel = global.WXEnvironment && global.WXEnvironment.logLevel || 'log';
    return levelMap[logLevel] && levelMap[logLevel][type];
  }

  /**
   * Convert all log arguments into primitive values.
   * @param  {array} args
   * @return {array}
   */
  function format(args) {
    return args.map(function (v) {
      var type = Object.prototype.toString.call(v);
      if (type.toLowerCase() === '[object object]') {
        v = JSON.stringify(v);
      } else {
        v = String(v);
      }
      return v;
    });
  }
  /* WEBPACK VAR INJECTION */}.call(exports, (function() { return this; }())))

/***/ },
/* 70 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  __webpack_require__(71);

/***/ },
/* 71 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  __webpack_require__(72);
  module.exports = __webpack_require__(25).Object.assign;

/***/ },
/* 72 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  // 19.1.3.1 Object.assign(target, source)
  var $export = __webpack_require__(32);

  $export($export.S + $export.F, 'Object', { assign: __webpack_require__(73) });

/***/ },
/* 73 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';
  // 19.1.2.1 Object.assign(target, source, ...)

  var getKeys = __webpack_require__(39),
      gOPS = __webpack_require__(74),
      pIE = __webpack_require__(75),
      toObject = __webpack_require__(51),
      IObject = __webpack_require__(42),
      $assign = Object.assign;

  // should work with symbols and should have deterministic property order (V8 bug)
  module.exports = !$assign || __webpack_require__(20)(function () {
    var A = {},
        B = {},
        S = Symbol(),
        K = 'abcdefghijklmnopqrst';
    A[S] = 7;
    K.split('').forEach(function (k) {
      B[k] = k;
    });
    return $assign({}, A)[S] != 7 || Object.keys($assign({}, B)).join('') != K;
  }) ? function assign(target, source) {
    // eslint-disable-line no-unused-vars
    var T = toObject(target),
        aLen = arguments.length,
        index = 1,
        getSymbols = gOPS.f,
        isEnum = pIE.f;
    while (aLen > index) {
      var S = IObject(arguments[index++]),
          keys = getSymbols ? getKeys(S).concat(getSymbols(S)) : getKeys(S),
          length = keys.length,
          j = 0,
          key;
      while (length > j) {
        if (isEnum.call(S, key = keys[j++])) T[key] = S[key];
      }
    }return T;
  } : $assign;

/***/ },
/* 74 */
/***/ function(module, exports) {

  "use strict";

  exports.f = Object.getOwnPropertySymbols;

/***/ },
/* 75 */
/***/ function(module, exports) {

  "use strict";

  exports.f = {}.propertyIsEnumerable;

/***/ },
/* 76 */
/***/ function(module, exports, __webpack_require__) {

  /* WEBPACK VAR INJECTION */(function(global) {'use strict';

  Object.defineProperty(exports, "__esModule", {
    value: true
  });
  exports.prepareInstance = prepareInstance;

  var _config = __webpack_require__(77);

  var _config2 = _interopRequireDefault(_config);

  var _vdom = __webpack_require__(99);

  function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

  /**
   * @fileOverview
   * Register framework(s) in JS runtime. Weex supply two layers for 3rd-party
   * framework(s): one is the instance management layer, another is the
   * virtual-DOM layer.
   */

  // This config is generated by `npm run build:config`.
  // It will collect all `runtime/framework-*.js` files and import each of them.
  // The filename is also the framework name.
  var config = {
    Document: _vdom.Document, Element: _vdom.Element, Comment: _vdom.Comment,
    sendTasks: function sendTasks() {
      var _global;

      return (_global = global).callNative.apply(_global, arguments);
    }
  };

  // Init each framework by `init` method and `config` which contains three
  // virtual-DOM Class: `Document`, `Element` & `Comment`, and a JS bridge method:
  // `sendTasks(...args)`.
  for (var name in _config2.default) {
    var framework = _config2.default[name];
    framework.init(config);
  }

  var versionRegExp = /^\/\/ *(\{[^\}]*\}) *\r?\n/;

  /**
   * Detect a JS Bundle code and make sure which framework it's based to. Each JS
   * Bundle should make sure that it starts with a line of JSON comment and is
   * more that one line.
   * @param  {string} code
   * @return {object}
   */
  function checkVersion(code) {
    var info = void 0;
    var result = versionRegExp.exec(code);
    if (result) {
      try {
        info = JSON.parse(result[1]);
      } catch (e) {}
    }
    return info;
  }

  var instanceMap = {};

  global.WXInstanceMap = instanceMap;

  /**
   * Prepare env for an new instance instead create it directly. This mode could
   * avoid running `new Function` during create an instance.
   * @param  {string} id
   * @param  {string} type
   * @param  {object} config
   * @param  {object} data
   */
  function prepareInstance(id, type, config, data) {
    type = type || 'Weex';
    var framework = _config2.default[type];
    if (framework && framework.prepareInstance) {
      instanceMap[id] = framework.prepareInstance(id, config, data);
      instanceMap[id].framework = type;
    } else {
      instanceMap[id] = {};
    }
  }

  /**
   * Check which framework a certain JS Bundle code based to. And create instance
   * by this framework.
   * @param {string} id
   * @param {string} code
   * @param {object} config
   * @param {object} data
   */
  function createInstance(id, code, config, data) {
    var info = instanceMap[id];
    if (!info) {
      info = checkVersion(code) || {};
      if (!_config2.default[info.framework]) {
        info.framework = 'Weex';
      }
      instanceMap[id] = info;
      config = config || {};
      config.bundleVersion = info.version;
      console.debug('[JS Framework] create an ' + info.framework + '@' + config.bundleVersion + ' instance from ' + config.bundleVersion);
      return _config2.default[info.framework].createInstance(id, code, config, data);
    }
    return new Error('invalid instance id "' + id + '"');
  }

  var methods = {
    prepareInstance: prepareInstance,
    createInstance: createInstance
  };

  /**
   * Register methods which init each frameworks.
   * @param {string} methodName
   */
  function genInit(methodName) {
    methods[methodName] = function () {
      for (var _name in _config2.default) {
        var _framework = _config2.default[_name];
        if (_framework && _framework[methodName]) {
          _framework[methodName].apply(_framework, arguments);
        }
      }
    };
  }

  // @todo: The method `registerMethods` will be re-designed or removed later.
  ['registerComponents', 'registerModules', 'registerMethods'].forEach(genInit);

  /**
   * Register methods which will be called for each instance.
   * @param {string} methodName
   */
  function genInstance(methodName) {
    methods[methodName] = function () {
      var id = arguments.length <= 0 ? undefined : arguments[0];
      var info = instanceMap[id];
      if (methodName === 'destroyInstance') {
        delete instanceMap[id];
      }
      if (info && _config2.default[info.framework]) {
        var _frameworks$info$fram;

        return (_frameworks$info$fram = _config2.default[info.framework])[methodName].apply(_frameworks$info$fram, arguments);
      }
      return new Error('invalid instance id "' + id + '"');
    };
  }

  ['destroyInstance', 'refreshInstance', 'receiveTasks', 'getRoot'].forEach(genInstance);

  /**
   * Adapt some legacy method(s) which will be called for each instance. These
   * methods should be deprecated and removed later.
   * @param {string} methodName
   * @param {string} nativeMethodName
   */
  function adaptInstance(methodName, nativeMethodName) {
    methods[nativeMethodName] = function () {
      var id = arguments.length <= 0 ? undefined : arguments[0];
      var info = instanceMap[id];
      if (info && _config2.default[info.framework]) {
        var _frameworks$info$fram2;

        return (_frameworks$info$fram2 = _config2.default[info.framework])[methodName].apply(_frameworks$info$fram2, arguments);
      }
      return new Error('invalid instance id "' + id + '"');
    };
  }

  adaptInstance('receiveTasks', 'callJS');

  exports.default = methods;
  /* WEBPACK VAR INJECTION */}.call(exports, (function() { return this; }())))

/***/ },
/* 77 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  Object.defineProperty(exports, "__esModule", {
    value: true
  });

  var _default = __webpack_require__(78);

  var Weex = _interopRequireWildcard(_default);

  function _interopRequireWildcard(obj) { if (obj && obj.__esModule) { return obj; } else { var newObj = {}; if (obj != null) { for (var key in obj) { if (Object.prototype.hasOwnProperty.call(obj, key)) newObj[key] = obj[key]; } } newObj.default = obj; return newObj; } }

  exports.default = {
    Weex: Weex
  }; // built by npm run build:config

/***/ },
/* 78 */
/***/ function(module, exports, __webpack_require__) {

  /* WEBPACK VAR INJECTION */(function(global) {'use strict';

  Object.defineProperty(exports, "__esModule", {
    value: true
  });

  var _typeof = typeof Symbol === "function" && typeof Symbol.iterator === "symbol" ? function (obj) { return typeof obj; } : function (obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol ? "symbol" : typeof obj; }; /**
                                                                                                                                                                                                                                                     * @fileOverview Weex framework entry.
                                                                                                                                                                                                                                                     */

  exports.init = init;
  exports.prepareInstance = prepareInstance;
  exports.createInstance = createInstance;
  exports.refreshInstance = refreshInstance;
  exports.destroyInstance = destroyInstance;
  exports.registerComponents = registerComponents;
  exports.registerModules = registerModules;
  exports.registerMethods = registerMethods;
  exports.getRoot = getRoot;
  exports.receiveTasks = receiveTasks;

  var _config = __webpack_require__(79);

  var _config2 = _interopRequireDefault(_config);

  var _app = __webpack_require__(80);

  var _app2 = _interopRequireDefault(_app);

  var _ctrl = __webpack_require__(83);

  var _register = __webpack_require__(82);

  var _vm = __webpack_require__(88);

  var _vm2 = _interopRequireDefault(_vm);

  function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

  function _toConsumableArray(arr) { if (Array.isArray(arr)) { for (var i = 0, arr2 = Array(arr.length); i < arr.length; i++) { arr2[i] = arr[i]; } return arr2; } else { return Array.from(arr); } }

  var nativeComponentMap = _config2.default.nativeComponentMap;

  var instanceMap = {};

  /**
   * Init config informations for Weex framework
   * @param  {object} cfg
   */
  function init(cfg) {
    _config2.default.Document = cfg.Document;
    _config2.default.Element = cfg.Element;
    _config2.default.Comment = cfg.Comment;
    _config2.default.sendTasks = cfg.sendTasks;
  }

  /**
   * Prepare a Weex instance and return the environment.
   *
   * @param  {string} id
   * @param  {object} options
   * @param  {object} data
   * @return {object}
   */
  function prepareInstance(id, options, data) {
    var instance = instanceMap[id];
    options = options || {};
    var result = void 0;
    if (!instance) {
      instance = new _app2.default(id, options);
      instanceMap[id] = instance;
      result = (0, _ctrl.init)(instance, '', data);
    }
    return result || {};
  }

  /**
   * Create a Weex instance.
   *
   * @param  {string} id
   * @param  {string} code
   * @param  {object} [options] option `HAS_LOG` enable print log
   * @param  {object} [data]
   */
  function createInstance(id, code, options, data) {
    var instance = instanceMap[id];
    options = options || {};
    var result = void 0;
    if (!instance) {
      instance = new _app2.default(id, options);
      instanceMap[id] = instance;
      result = (0, _ctrl.init)(instance, code, data);
    } else {
      result = new Error('invalid instance id "' + id + '"');
    }
    return result;
  }

  /**
   * Refresh a Weex instance with data.
   *
   * @param  {string} id
   * @param  {object} data
   */
  function refreshInstance(id, data) {
    var instance = instanceMap[id];
    var result = void 0;
    if (instance) {
      result = (0, _ctrl.refresh)(instance, data);
    } else {
      result = new Error('invalid instance id "' + id + '"');
    }
    return result;
  }

  /**
   * Destroy a Weex instance.
   * @param  {string} id
   */
  function destroyInstance(id) {
    var instance = instanceMap[id];
    if (!instance) {
      return new Error('invalid instance id "' + id + '"');
    }
    (0, _ctrl.destroy)(instance);
    delete instanceMap[id];
    return instanceMap;
  }

  /**
   * Register the name of each native component.
   * @param  {array} components array of name
   */
  function registerComponents(components) {
    if (Array.isArray(components)) {
      components.forEach(function register(name) {
        /* istanbul ignore if */
        if (!name) {
          return;
        }
        if (typeof name === 'string') {
          nativeComponentMap[name] = true;
        } else if ((typeof name === 'undefined' ? 'undefined' : _typeof(name)) === 'object' && typeof name.type === 'string') {
          nativeComponentMap[name.type] = name;
        }
      });
    }
  }

  /**
   * Register the name and methods of each module.
   * @param  {object} modules a object of modules
   */
  function registerModules(modules) {
    if ((typeof modules === 'undefined' ? 'undefined' : _typeof(modules)) === 'object') {
      (0, _register.initModules)(modules);
    }
  }

  /**
   * Register the name and methods of each api.
   * @param  {object} apis a object of apis
   */
  function registerMethods(methods) {
    if ((typeof methods === 'undefined' ? 'undefined' : _typeof(methods)) === 'object') {
      (0, _register.initMethods)(_vm2.default, methods);
    }
  }

  // @todo: Hack for this framework only. Will be re-designed or removed later.
  global.registerMethods = registerMethods;

  /**
   * Get a whole element tree of an instance for debugging.
   * @param  {string} id
   * @return {object} a virtual dom tree
   */
  function getRoot(id) {
    var instance = instanceMap[id];
    var result = void 0;
    if (instance) {
      result = (0, _ctrl.getRootElement)(instance);
    } else {
      result = new Error('invalid instance id "' + id + '"');
    }
    return result;
  }

  var jsHandlers = {
    fireEvent: function fireEvent(id) {
      for (var _len = arguments.length, args = Array(_len > 1 ? _len - 1 : 0), _key = 1; _key < _len; _key++) {
        args[_key - 1] = arguments[_key];
      }

      return _ctrl.fireEvent.apply(undefined, [instanceMap[id]].concat(args));
    },
    callback: function callback(id) {
      for (var _len2 = arguments.length, args = Array(_len2 > 1 ? _len2 - 1 : 0), _key2 = 1; _key2 < _len2; _key2++) {
        args[_key2 - 1] = arguments[_key2];
      }

      return _ctrl.callback.apply(undefined, [instanceMap[id]].concat(args));
    }
  };

  /**
   * Accept calls from native (event or callback).
   *
   * @param  {string} id
   * @param  {array} tasks list with `method` and `args`
   */
  function receiveTasks(id, tasks) {
    var instance = instanceMap[id];
    if (instance && Array.isArray(tasks)) {
      var _ret = function () {
        var results = [];
        tasks.forEach(function (task) {
          var handler = jsHandlers[task.method];
          var args = [].concat(_toConsumableArray(task.args));
          if (typeof handler === 'function') {
            args.unshift(id);
            results.push(handler.apply(undefined, _toConsumableArray(args)));
          }
        });
        return {
          v: results
        };
      }();

      if ((typeof _ret === 'undefined' ? 'undefined' : _typeof(_ret)) === "object") return _ret.v;
    }
    return new Error('invalid instance id "' + id + '" or tasks');
  }
  /* WEBPACK VAR INJECTION */}.call(exports, (function() { return this; }())))

/***/ },
/* 79 */
/***/ function(module, exports) {

  'use strict';

  Object.defineProperty(exports, "__esModule", {
    value: true
  });
  // @todo: It should be registered by native from `registerComponents()`.
  exports.default = {
    nativeComponentMap: {
      text: true,
      image: true,
      container: true,
      slider: {
        type: 'slider',
        append: 'tree'
      },
      cell: {
        type: 'cell',
        append: 'tree'
      }
    }
  };

/***/ },
/* 80 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  Object.defineProperty(exports, "__esModule", {
    value: true
  });
  exports.default = App;

  var _differ = __webpack_require__(81);

  var _differ2 = _interopRequireDefault(_differ);

  var _config = __webpack_require__(79);

  var _config2 = _interopRequireDefault(_config);

  var _register = __webpack_require__(82);

  var _ctrl = __webpack_require__(83);

  function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

  /**
   * App constructor for Weex framework.
   * @param {string} id
   * @param {object} options
   */
  /**
   * @fileOverview
   * Weex instance constructor & definition
   */

  function App(id, options) {
    this.id = id;
    this.options = options || {};
    this.vm = null;
    this.customComponentMap = {};
    this.callbacks = {};
    this.doc = new _config2.default.Document(id, this.options.bundleUrl);
    this.differ = new _differ2.default(id);
    this.uid = 0;
  }

  /**
   * @deprecated
   */
  App.prototype.requireModule = function (name) {
    return (0, _register.requireModule)(this, name);
  };

  /**
   * @deprecated
   */
  App.prototype.updateActions = function () {
    (0, _ctrl.updateActions)(this);
  };

  /**
   * @deprecated
   */
  App.prototype.callTasks = function (tasks) {
    (0, _ctrl.callTasks)(this, tasks);
  };

/***/ },
/* 81 */
/***/ function(module, exports) {

  'use strict';

  Object.defineProperty(exports, "__esModule", {
    value: true
  });

  var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) defineProperties(Constructor.prototype, protoProps); if (staticProps) defineProperties(Constructor, staticProps); return Constructor; }; }();

  function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

  var Differ = function () {
    function Differ(id) {
      _classCallCheck(this, Differ);

      this.id = id;
      this.map = [];
      this.hooks = [];
    }

    _createClass(Differ, [{
      key: 'isEmpty',
      value: function isEmpty() {
        return this.map.length === 0;
      }
    }, {
      key: 'append',
      value: function append(type, depth, ref, handler) {
        var _this = this;

        if (!this.hasTimer) {
          this.hasTimer = true;
          setTimeout(function () {
            _this.hasTimer = false;
            _this.flush(true);
          }, 0);
        }
        var map = this.map;
        if (!map[depth]) {
          map[depth] = {};
        }
        var group = map[depth];
        if (!group[type]) {
          group[type] = {};
        }
        if (type === 'element') {
          if (!group[type][ref]) {
            group[type][ref] = [];
          }
          group[type][ref].push(handler);
        } else {
          group[type][ref] = handler;
        }
      }
    }, {
      key: 'flush',
      value: function flush(isTimeout) {
        var map = this.map.slice();
        this.map.length = 0;
        map.forEach(function (group) {
          callTypeMap(group, 'repeat');
          callTypeMap(group, 'shown');
          callTypeList(group, 'element');
        });

        var hooks = this.hooks.slice();
        this.hooks.length = 0;
        hooks.forEach(function (fn) {
          fn();
        });

        if (!this.isEmpty()) {
          this.flush();
        }
      }
    }, {
      key: 'then',
      value: function then(fn) {
        this.hooks.push(fn);
      }
    }]);

    return Differ;
  }();

  exports.default = Differ;


  function callTypeMap(group, type) {
    var map = group[type];
    for (var ref in map) {
      map[ref]();
    }
  }

  function callTypeList(group, type) {
    var map = group[type];
    for (var ref in map) {
      var list = map[ref];
      list.forEach(function (handler) {
        handler();
      });
    }
  }

/***/ },
/* 82 */
/***/ function(module, exports) {

  'use strict';

  Object.defineProperty(exports, "__esModule", {
    value: true
  });
  exports.getModule = getModule;
  exports.clearModules = clearModules;
  exports.initModules = initModules;
  exports.initMethods = initMethods;
  exports.requireModule = requireModule;
  exports.requireCustomComponent = requireCustomComponent;
  exports.registerCustomComponent = registerCustomComponent;
  var nativeModules = {};

  // for testing

  /**
   * for testing
   */
  function getModule(moduleName) {
    return nativeModules[moduleName];
  }

  /**
   * for testing
   */
  function clearModules() {
    nativeModules = {};
  }

  // for framework

  /**
   * init modules for an app instance
   * the second param determines whether to replace an existed method
   */
  function initModules(modules, ifReplace) {
    var _loop = function _loop(moduleName) {
      // init `modules[moduleName][]`
      var methods = nativeModules[moduleName];
      if (!methods) {
        methods = {};
        nativeModules[moduleName] = methods;
      }

      // push each non-existed new method
      modules[moduleName].forEach(function (method) {
        if (typeof method === 'string') {
          method = {
            name: method
          };
        }

        if (!methods[method.name] || ifReplace) {
          methods[method.name] = method;
        }
      });
    };

    for (var moduleName in modules) {
      _loop(moduleName);
    }
  }

  /**
   * init app methods
   */
  function initMethods(Vm, apis) {
    var p = Vm.prototype;

    for (var apiName in apis) {
      if (!p.hasOwnProperty(apiName)) {
        p[apiName] = apis[apiName];
      }
    }
  }

  // for app

  /**
   * get a module of methods for an app instance
   */
  function requireModule(app, name) {
    var methods = nativeModules[name];
    var target = {};

    var _loop2 = function _loop2(methodName) {
      target[methodName] = function () {
        for (var _len = arguments.length, args = Array(_len), _key = 0; _key < _len; _key++) {
          args[_key] = arguments[_key];
        }

        return app.callTasks({
          module: name,
          method: methodName,
          args: args
        });
      };
    };

    for (var methodName in methods) {
      _loop2(methodName);
    }
    return target;
  }

  /**
   * get a custom component options
   */
  function requireCustomComponent(app, name) {
    var customComponentMap = app.customComponentMap;

    return customComponentMap[name];
  }

  /**
   * register a custom component options
   */
  function registerCustomComponent(app, name, def) {
    var customComponentMap = app.customComponentMap;


    if (customComponentMap[name]) {
      console.error('[JS Framework] define a component(' + name + ') that already exists');
      return;
    }

    customComponentMap[name] = def;
  }

/***/ },
/* 83 */
/***/ function(module, exports, __webpack_require__) {

  /* WEBPACK VAR INJECTION */(function(global) {'use strict';

  Object.defineProperty(exports, "__esModule", {
    value: true
  });

  var _typeof = typeof Symbol === "function" && typeof Symbol.iterator === "symbol" ? function (obj) { return typeof obj; } : function (obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol ? "symbol" : typeof obj; };

  exports.init = init;
  exports.refresh = refresh;
  exports.destroy = destroy;
  exports.getRootElement = getRootElement;
  exports.fireEvent = fireEvent;
  exports.callback = callback;
  exports.updateActions = updateActions;
  exports.callTasks = callTasks;

  var _util = __webpack_require__(84);

  var _config = __webpack_require__(79);

  var _config2 = _interopRequireDefault(_config);

  var _bundle = __webpack_require__(85);

  function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

  function _toConsumableArray(arr) { if (Array.isArray(arr)) { for (var i = 0, arr2 = Array(arr.length); i < arr.length; i++) { arr2[i] = arr[i]; } return arr2; } else { return Array.from(arr); } } /**
                                                                                                                                                                                                       * @fileOverview
                                                                                                                                                                                                       * instance controls from native
                                                                                                                                                                                                       *
                                                                                                                                                                                                       * - init bundle
                                                                                                                                                                                                       * - fire event
                                                                                                                                                                                                       * - callback
                                                                                                                                                                                                       * - refresh
                                                                                                                                                                                                       * - destroy
                                                                                                                                                                                                       *
                                                                                                                                                                                                       * corresponded with the API of instance manager (framework.js)
                                                                                                                                                                                                       */

  /**
   * Init an app by run code with data.
   * In native renderer if the code is empty then we just prepare an instance
   * env only for run the real code later.
   * @param  {object} app
   * @param  {string} code
   * @param  {object} data
   */
  function init(app, code, data) {
    console.debug('[JS Framework] Intialize an instance with:\n', data);
    var result = void 0;

    // prepare app env methods
    var bundleDefine = function bundleDefine() {
      for (var _len = arguments.length, args = Array(_len), _key = 0; _key < _len; _key++) {
        args[_key] = arguments[_key];
      }

      return _bundle.defineFn.apply(undefined, [app].concat(args));
    };
    var bundleBootstrap = function bundleBootstrap(name, config, _data) {
      result = (0, _bundle.bootstrap)(app, name, config, _data || data);
      updateActions(app);
      app.doc.listener.createFinish();
      console.debug('[JS Framework] After intialized an instance(' + app.id + ')');
    };
    var bundleRegister = function bundleRegister() {
      for (var _len2 = arguments.length, args = Array(_len2), _key2 = 0; _key2 < _len2; _key2++) {
        args[_key2] = arguments[_key2];
      }

      return _bundle.register.apply(undefined, [app].concat(args));
    };
    var bundleRender = function bundleRender(name, _data) {
      result = (0, _bundle.bootstrap)(app, name, {}, _data);
    };
    var bundleRequire = function bundleRequire(name) {
      return function (_data) {
        result = (0, _bundle.bootstrap)(app, name, {}, _data);
      };
    };
    var bundleDocument = app.doc;
    var bundleRequireModule = function bundleRequireModule(name) {
      return app.requireModule((0, _util.removeWeexPrefix)(name));
    };

    // prepare code
    var functionBody = void 0;
    /* istanbul ignore if */
    if (typeof code === 'function') {
      // `function () {...}` -> `{...}`
      // not very strict
      functionBody = code.toString().substr(12);
    } else if (code) {
      functionBody = code.toString();
    }

    // run code and get result
    var _global = global;
    var WXEnvironment = _global.WXEnvironment;

    if (WXEnvironment && WXEnvironment.platform !== 'Web') {
      var _ret = function () {
        // timer APIs polyfill in native
        var timer = app.requireModule('timer');
        var timerAPIs = {
          setTimeout: function setTimeout() {
            for (var _len3 = arguments.length, args = Array(_len3), _key3 = 0; _key3 < _len3; _key3++) {
              args[_key3] = arguments[_key3];
            }

            var handler = function handler() {
              args[0].apply(args, _toConsumableArray(args.slice(2)));
            };
            timer.setTimeout(handler, args[1]);
            return app.uid.toString();
          },
          setInterval: function setInterval() {
            for (var _len4 = arguments.length, args = Array(_len4), _key4 = 0; _key4 < _len4; _key4++) {
              args[_key4] = arguments[_key4];
            }

            var handler = function handler() {
              args[0].apply(args, _toConsumableArray(args.slice(2)));
            };
            timer.setInterval(handler, args[1]);
            return app.uid.toString();
          },
          clearTimeout: function clearTimeout(n) {
            timer.clearTimeout(n);
          },
          clearInterval: function clearInterval(n) {
            timer.clearInterval(n);
          }
        };

        // Special: if the code is empty that means we only need to prepare an
        // environment. If so we just return a enviorment object.
        if (!code) {
          return {
            v: {
              define: bundleDefine,
              require: bundleRequire,
              document: bundleDocument,
              bootstrap: bundleBootstrap,
              register: bundleRegister,
              render: bundleRender,
              __weex_define__: bundleDefine,
              __weex_bootstrap__: bundleBootstrap,
              __weex_document__: bundleDocument,
              __weex_require__: bundleRequireModule,
              setTimeout: timerAPIs.setTimeout,
              setInterval: timerAPIs.setInterval,
              clearTimeout: timerAPIs.clearTimeout,
              clearInterval: timerAPIs.clearInterval
            }
          };
        }

        var fn = new Function('define', 'require', 'document', 'bootstrap', 'register', 'render', '__weex_define__', // alias for define
        '__weex_bootstrap__', // alias for bootstrap
        '__weex_document__', // alias for bootstrap
        '__weex_require__', 'setTimeout', 'setInterval', 'clearTimeout', 'clearInterval', functionBody);

        fn(bundleDefine, bundleRequire, bundleDocument, bundleBootstrap, bundleRegister, bundleRender, bundleDefine, bundleBootstrap, bundleDocument, bundleRequireModule, timerAPIs.setTimeout, timerAPIs.setInterval, timerAPIs.clearTimeout, timerAPIs.clearInterval);
      }();

      if ((typeof _ret === 'undefined' ? 'undefined' : _typeof(_ret)) === "object") return _ret.v;
    } else {
      var _fn = new Function('define', 'require', 'document', 'bootstrap', 'register', 'render', '__weex_define__', // alias for define
      '__weex_bootstrap__', // alias for bootstrap
      '__weex_document__', // alias for bootstrap
      '__weex_require__', functionBody);

      _fn(bundleDefine, bundleRequire, bundleDocument, bundleBootstrap, bundleRegister, bundleRender, bundleDefine, bundleBootstrap, bundleDocument, bundleRequireModule);
    }

    return result;
  }

  /**
   * Refresh an app with data to its root component options.
   * @param  {object} app
   * @param  {any}    data
   */
  function refresh(app, data) {
    console.debug('[JS Framework] Refresh with', data, 'in instance[' + app.id + ']');
    var vm = app.vm;
    if (vm && data) {
      app.doc.close();
      if (typeof vm.refreshData === 'function') {
        vm.refreshData(data);
      } else {
        (0, _util.extend)(vm, data);
      }
      updateActions(app);
      app.doc.listener.refreshFinish();
      app.doc.open();
      return;
    }
    return new Error('invalid data "' + data + '"');
  }

  /**
   * Destroy an app.
   * @param  {object} app
   */
  function destroy(app) {
    console.debug('[JS Framework] Destory an instance(' + app.id + ')');

    app.id = '';
    app.options = null;
    app.blocks = null;
    app.vm = null;
    app.doc = null;
    app.customComponentMap = null;
    app.callbacks = null;
  }

  /**
   * Get a JSON object to describe the document body.
   * @param  {object} app
   * @return {object}
   */
  function getRootElement(app) {
    var doc = app.doc || {};
    var body = doc.body || {};
    return body.toJSON ? body.toJSON() : {};
  }

  /**
   * Fire an event from renderer. The event has type, an event object and an
   * element ref. If the event comes with some virtual-DOM changes, it should
   * have one more parameter to describe the changes.
   * @param  {object} app
   * @param  {string} ref
   * @param  {type}   type
   * @param  {object} e
   * @param  {object} domChanges
   */
  function fireEvent(app, ref, type, e, domChanges) {
    console.debug('[JS Framework] Fire a "' + type + '" event on an element(' + ref + ') in instance(' + app.id + ')');
    if (Array.isArray(ref)) {
      ref.some(function (ref) {
        return app.fireEvent(ref, type, e) !== false;
      });
      return;
    }
    var el = app.doc.getRef(ref);
    if (el) {
      app.doc.close();
      var result = app.doc.fireEvent(el, type, e, domChanges);
      updateActions(app);
      app.doc.listener.updateFinish();
      app.doc.open();
      return result;
    }
    return new Error('invalid element reference "' + ref + '"');
  }

  /**
   * Make a callback for a certain app.
   * @param  {object}   app
   * @param  {number}   callbackId
   * @param  {any}      data
   * @param  {boolean}  ifKeepAlive
   */
  function callback(app, callbackId, data, ifKeepAlive) {
    console.debug('[JS Framework] Invoke a callback(' + callbackId + ') with', data, 'in instance(' + app.id + ')');
    var callback = app.callbacks[callbackId];
    if (typeof callback === 'function') {
      app.doc.close();
      callback(data);
      if (typeof ifKeepAlive === 'undefined' || ifKeepAlive === false) {
        app.callbacks[callbackId] = undefined;
      }
      updateActions(app);
      app.doc.listener.updateFinish();
      app.doc.open();
      return;
    }
    return new Error('invalid callback id "' + callbackId + '"');
  }

  /**
   * Collect all virtual-DOM mutations together and send them to renderer.
   * @param  {object} app
   */
  function updateActions(app) {
    app.differ.flush();
    var tasks = [];
    if (app.doc && app.doc.listener && app.doc.listener.updates.length) {
      tasks.push.apply(tasks, _toConsumableArray(app.doc.listener.updates));
      app.doc.listener.updates = [];
    }
    if (tasks.length) {
      return callTasks(app, tasks);
    }
  }

  /**
   * Call all tasks from an app to renderer (native).
   * @param  {object} app
   * @param  {array}  tasks
   */
  function callTasks(app, tasks) {
    if ((0, _util.typof)(tasks) !== 'array') {
      tasks = [tasks];
    }

    tasks.forEach(function (task) {
      task.args = task.args.map(function (arg) {
        return normalize(arg, app);
      });
    });

    return _config2.default.sendTasks(app.id, tasks, '-1');
  }

  /**
   * Normalize a value. Specially, if the value is a function, then generate a function id
   * and save it to `app.callbacks`, at last return the function id.
   * @param  {any}        v
   * @param  {object}     app
   * @return {primitive}
   */
  function normalize(v, app) {
    var type = (0, _util.typof)(v);

    switch (type) {
      case 'undefined':
      case 'null':
        return '';
      case 'regexp':
        return v.toString();
      case 'date':
        return v.toISOString();
      case 'number':
      case 'string':
      case 'boolean':
      case 'array':
      case 'object':
        if (v instanceof _config2.default.Element) {
          return v.ref;
        }
        return v;
      case 'function':
        app.callbacks[++app.uid] = v;
        return app.uid.toString();
      default:
        return JSON.stringify(v);
    }
  }
  /* WEBPACK VAR INJECTION */}.call(exports, (function() { return this; }())))

/***/ },
/* 84 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  Object.defineProperty(exports, "__esModule", {
    value: true
  });

  var _shared = __webpack_require__(3);

  Object.defineProperty(exports, 'extend', {
    enumerable: true,
    get: function get() {
      return _shared.extend;
    }
  });
  Object.defineProperty(exports, 'def', {
    enumerable: true,
    get: function get() {
      return _shared.def;
    }
  });
  Object.defineProperty(exports, 'remove', {
    enumerable: true,
    get: function get() {
      return _shared.remove;
    }
  });
  Object.defineProperty(exports, 'hasOwn', {
    enumerable: true,
    get: function get() {
      return _shared.hasOwn;
    }
  });
  Object.defineProperty(exports, 'bind', {
    enumerable: true,
    get: function get() {
      return _shared.bind;
    }
  });
  Object.defineProperty(exports, 'toArray', {
    enumerable: true,
    get: function get() {
      return _shared.toArray;
    }
  });
  Object.defineProperty(exports, 'isObject', {
    enumerable: true,
    get: function get() {
      return _shared.isObject;
    }
  });
  Object.defineProperty(exports, 'isPlainObject', {
    enumerable: true,
    get: function get() {
      return _shared.isPlainObject;
    }
  });
  exports.isReserved = isReserved;
  exports.cached = cached;
  exports.typof = typof;
  exports.removeWeexPrefix = removeWeexPrefix;
  exports.removeJSSurfix = removeJSSurfix;


  /**
   * Check if a string starts with $ or _
   *
   * @param {String} str
   * @return {Boolean}
   */

  function isReserved(str) {
    var c = (str + '').charCodeAt(0);
    return c === 0x24 || c === 0x5F;
  }

  // can we use __proto__?
  var hasProto = exports.hasProto = '__proto__' in {};

  var _Set = void 0;
  /* istanbul ignore if */
  if (typeof Set !== 'undefined') {
    // use native Set when available.
    console.log('native set');
    exports._Set = _Set = Set;
  } else {
    // a non-standard Set polyfill that only works with primitive keys.
    exports._Set = _Set = function _Set() {
      this.set = Object.create(null);
    };
    _Set.prototype.has = function (key) {
      return this.set[key] !== undefined;
    };
    _Set.prototype.add = function (key) {
      this.set[key] = 1;
    };
    _Set.prototype.clear = function () {
      this.set = Object.create(null);
    };
  }

  exports._Set = _Set;

  /**
   * Create a cached version of a pure function.
   *
   * @param {Function} fn
   * @return {Function}
   */

  function cached(fn) {
    var cache = Object.create(null);
    return function cachedFn(str) {
      var hit = cache[str];
      return hit || (cache[str] = fn(str));
    };
  }

  /**
   * Camelize a hyphen-delmited string.
   *
   * @param {String} str
   * @return {String}
   */

  var camelizeRE = /-(\w)/g;
  var camelize = exports.camelize = cached(function (str) {
    return str.replace(camelizeRE, toUpper);
  });

  function toUpper(_, c) {
    return c ? c.toUpperCase() : '';
  }

  /**
   * Hyphenate a camelCase string.
   *
   * @param {String} str
   * @return {String}
   */

  var hyphenateRE = /([a-z\d])([A-Z])/g;
  var hyphenate = exports.hyphenate = cached(function (str) {
    return str.replace(hyphenateRE, '$1-$2').toLowerCase();
  });

  function typof(v) {
    var s = Object.prototype.toString.call(v);
    return s.substring(8, s.length - 1).toLowerCase();
  }

  // weex name rules

  var WEEX_COMPONENT_REG = /^@weex-component\//;
  var WEEX_MODULE_REG = /^@weex-module\//;
  var NORMAL_MODULE_REG = /^\.{1,2}\//;
  var JS_SURFIX_REG = /\.js$/;

  var isWeexComponent = exports.isWeexComponent = function isWeexComponent(name) {
    return !!name.match(WEEX_COMPONENT_REG);
  };
  var isWeexModule = exports.isWeexModule = function isWeexModule(name) {
    return !!name.match(WEEX_MODULE_REG);
  };
  var isNormalModule = exports.isNormalModule = function isNormalModule(name) {
    return !!name.match(NORMAL_MODULE_REG);
  };
  var isNpmModule = exports.isNpmModule = function isNpmModule(name) {
    return !isWeexComponent(name) && !isWeexModule(name) && !isNormalModule(name);
  };

  function removeWeexPrefix(str) {
    var result = str.replace(WEEX_COMPONENT_REG, '').replace(WEEX_MODULE_REG, '');
    return result;
  }

  function removeJSSurfix(str) {
    return str.replace(JS_SURFIX_REG, '');
  }

/***/ },
/* 85 */
/***/ function(module, exports, __webpack_require__) {

  /* WEBPACK VAR INJECTION */(function(global) {'use strict';

  Object.defineProperty(exports, "__esModule", {
    value: true
  });
  exports.clearCommonModules = clearCommonModules;
  exports.defineFn = defineFn;
  exports.bootstrap = bootstrap;
  exports.register = register;

  var _semver = __webpack_require__(86);

  var _semver2 = _interopRequireDefault(_semver);

  var _util = __webpack_require__(84);

  var _vm = __webpack_require__(88);

  var _vm2 = _interopRequireDefault(_vm);

  var _register = __webpack_require__(82);

  var _downgrade = __webpack_require__(98);

  var downgrade = _interopRequireWildcard(_downgrade);

  function _interopRequireWildcard(obj) { if (obj && obj.__esModule) { return obj; } else { var newObj = {}; if (obj != null) { for (var key in obj) { if (Object.prototype.hasOwnProperty.call(obj, key)) newObj[key] = obj[key]; } } newObj.default = obj; return newObj; } }

  function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

  function _defineProperty(obj, key, value) { if (key in obj) { Object.defineProperty(obj, key, { value: value, enumerable: true, configurable: true, writable: true }); } else { obj[key] = value; } return obj; } /**
                                                                                                                                                                                                                     * @fileOverview
                                                                                                                                                                                                                     * api that invoked by js bundle code
                                                                                                                                                                                                                     *
                                                                                                                                                                                                                     * - define(name, factory): define a new composed component type
                                                                                                                                                                                                                     * - bootstrap(type, config, data): require a certain type &
                                                                                                                                                                                                                     *         render with (optional) data
                                                                                                                                                                                                                     *
                                                                                                                                                                                                                     * deprecated:
                                                                                                                                                                                                                     * - register(type, options): register a new composed component type
                                                                                                                                                                                                                     * - render(type, data): render by a certain type with (optional) data
                                                                                                                                                                                                                     * - require(type)(data): require a type then render with data
                                                                                                                                                                                                                     */

  /**
   * @deprecated
   *
   * common modules are shared to all instances
   * it's very dangerous
   */
  var commonModules = {};

  /**
   * @deprecated
   */
  function clearCommonModules() {
    commonModules = {};
  }

  /**
   * define(name, factory) for primary usage
   * or
   * define(name, deps, factory) for compatibility
   * Notice: DO NOT use function define() {},
   * it will cause error after builded by webpack
   */
  function defineFn(app, name) {
    console.debug('[JS Framework] define a component ' + name);

    // adapt args:
    // 1. name, deps[], factory()
    // 2. name, factory()
    // 3. name, definition{}
    var factory = void 0,
        definition = void 0;
    if ((arguments.length <= 2 ? 0 : arguments.length - 2) > 1) {
      definition = arguments.length <= 3 ? undefined : arguments[3];
    } else {
      definition = arguments.length <= 2 ? undefined : arguments[2];
    }
    if (typeof definition === 'function') {
      factory = definition;
      definition = null;
    }

    // resolve definition from factory
    if (factory) {
      var r = function r(name) {
        if ((0, _util.isWeexComponent)(name)) {
          var cleanName = (0, _util.removeWeexPrefix)(name);
          return (0, _register.requireCustomComponent)(app, cleanName);
        }
        if ((0, _util.isWeexModule)(name)) {
          var _cleanName = (0, _util.removeWeexPrefix)(name);
          return app.requireModule(_cleanName);
        }
        if ((0, _util.isNormalModule)(name) || (0, _util.isNpmModule)(name)) {
          var _cleanName2 = (0, _util.removeJSSurfix)(name);
          return commonModules[_cleanName2];
        }
      };
      var m = { exports: {} };
      factory(r, m.exports, m);
      definition = m.exports;
    }

    // apply definition
    if ((0, _util.isWeexComponent)(name)) {
      var cleanName = (0, _util.removeWeexPrefix)(name);
      (0, _register.registerCustomComponent)(app, cleanName, definition);
    } else if ((0, _util.isWeexModule)(name)) {
      var _cleanName3 = (0, _util.removeWeexPrefix)(name);
      (0, _register.initModules)(_defineProperty({}, _cleanName3, definition));
    } else if ((0, _util.isNormalModule)(name)) {
      var _cleanName4 = (0, _util.removeJSSurfix)(name);
      commonModules[_cleanName4] = definition;
    } else if ((0, _util.isNpmModule)(name)) {
      var _cleanName5 = (0, _util.removeJSSurfix)(name);
      if (definition.template || definition.style || definition.methods) {
        // downgrade to old define method (define('componentName', factory))
        // the exports contain one key of template, style or methods
        // but it has risk!!!
        (0, _register.registerCustomComponent)(app, _cleanName5, definition);
      } else {
        commonModules[_cleanName5] = definition;
      }
    }
  }

  /**
   * bootstrap app from a certain custom component with config & data
   */
  function bootstrap(app, name, config, data) {
    console.debug('[JS Framework] bootstrap for ' + name);

    // 1. validate custom component name first
    var cleanName = void 0;
    if ((0, _util.isWeexComponent)(name)) {
      cleanName = (0, _util.removeWeexPrefix)(name);
    } else if ((0, _util.isNpmModule)(name)) {
      cleanName = (0, _util.removeJSSurfix)(name);
      // check if define by old 'define' method
      /* istanbul ignore if */
      if (!(0, _register.requireCustomComponent)(app, cleanName)) {
        return new Error('It\'s not a component: ' + name);
      }
    } else {
      return new Error('Wrong component name: ' + name);
    }

    // 2. validate configuration
    config = (0, _util.isPlainObject)(config) ? config : {};
    // 2.1 transformer version check
    if (typeof config.transformerVersion === 'string' && typeof global.transformerVersion === 'string' && !_semver2.default.satisfies(config.transformerVersion, global.transformerVersion)) {
      return new Error('JS Bundle version: ' + config.transformerVersion + ' ' + ('not compatible with ' + global.transformerVersion));
    }
    // 2.2 downgrade version check
    var downgradeResult = downgrade.check(config.downgrade);
    /* istanbul ignore if */
    if (downgradeResult.isDowngrade) {
      app.callTasks([{
        module: 'instanceWrap',
        method: 'error',
        args: [downgradeResult.errorType, downgradeResult.code, downgradeResult.errorMessage]
      }]);
      return new Error('Downgrade[' + downgradeResult.code + ']: ' + downgradeResult.errorMessage);
    }

    // 3. create a new Vm with custom component name and data
    app.vm = new _vm2.default(cleanName, null, { _app: app }, null, data);
  }

  /**
   * @deprecated
   */
  function register(app, type, options) {
    console.warn('[JS Framework] Register is deprecated, please install lastest transformer.');
    (0, _register.registerCustomComponent)(app, type, options);
  }
  /* WEBPACK VAR INJECTION */}.call(exports, (function() { return this; }())))

/***/ },
/* 86 */
/***/ function(module, exports, __webpack_require__) {

  /* WEBPACK VAR INJECTION */(function(process) {'use strict';

  var _typeof = typeof Symbol === "function" && typeof Symbol.iterator === "symbol" ? function (obj) { return typeof obj; } : function (obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol ? "symbol" : typeof obj; };

  exports = module.exports = SemVer;

  // The debug function is excluded entirely from the minified version.
  /* nomin */var debug;
  /* nomin */if ((typeof process === 'undefined' ? 'undefined' : _typeof(process)) === 'object' &&
  /* nomin */process.env &&
  /* nomin */process.env.NODE_DEBUG &&
  /* nomin *//\bsemver\b/i.test(process.env.NODE_DEBUG))
    /* nomin */debug = function debug() {
      /* nomin */var args = Array.prototype.slice.call(arguments, 0);
      /* nomin */args.unshift('SEMVER');
      /* nomin */console.log.apply(console, args);
      /* nomin */
    };
    /* nomin */else
    /* nomin */debug = function debug() {};

  // Note: this is the semver.org version of the spec that it implements
  // Not necessarily the package version of this code.
  exports.SEMVER_SPEC_VERSION = '2.0.0';

  var MAX_LENGTH = 256;
  var MAX_SAFE_INTEGER = Number.MAX_SAFE_INTEGER || 9007199254740991;

  // The actual regexps go on exports.re
  var re = exports.re = [];
  var src = exports.src = [];
  var R = 0;

  // The following Regular Expressions can be used for tokenizing,
  // validating, and parsing SemVer version strings.

  // ## Numeric Identifier
  // A single `0`, or a non-zero digit followed by zero or more digits.

  var NUMERICIDENTIFIER = R++;
  src[NUMERICIDENTIFIER] = '0|[1-9]\\d*';
  var NUMERICIDENTIFIERLOOSE = R++;
  src[NUMERICIDENTIFIERLOOSE] = '[0-9]+';

  // ## Non-numeric Identifier
  // Zero or more digits, followed by a letter or hyphen, and then zero or
  // more letters, digits, or hyphens.

  var NONNUMERICIDENTIFIER = R++;
  src[NONNUMERICIDENTIFIER] = '\\d*[a-zA-Z-][a-zA-Z0-9-]*';

  // ## Main Version
  // Three dot-separated numeric identifiers.

  var MAINVERSION = R++;
  src[MAINVERSION] = '(' + src[NUMERICIDENTIFIER] + ')\\.' + '(' + src[NUMERICIDENTIFIER] + ')\\.' + '(' + src[NUMERICIDENTIFIER] + ')';

  var MAINVERSIONLOOSE = R++;
  src[MAINVERSIONLOOSE] = '(' + src[NUMERICIDENTIFIERLOOSE] + ')\\.' + '(' + src[NUMERICIDENTIFIERLOOSE] + ')\\.' + '(' + src[NUMERICIDENTIFIERLOOSE] + ')';

  // ## Pre-release Version Identifier
  // A numeric identifier, or a non-numeric identifier.

  var PRERELEASEIDENTIFIER = R++;
  src[PRERELEASEIDENTIFIER] = '(?:' + src[NUMERICIDENTIFIER] + '|' + src[NONNUMERICIDENTIFIER] + ')';

  var PRERELEASEIDENTIFIERLOOSE = R++;
  src[PRERELEASEIDENTIFIERLOOSE] = '(?:' + src[NUMERICIDENTIFIERLOOSE] + '|' + src[NONNUMERICIDENTIFIER] + ')';

  // ## Pre-release Version
  // Hyphen, followed by one or more dot-separated pre-release version
  // identifiers.

  var PRERELEASE = R++;
  src[PRERELEASE] = '(?:-(' + src[PRERELEASEIDENTIFIER] + '(?:\\.' + src[PRERELEASEIDENTIFIER] + ')*))';

  var PRERELEASELOOSE = R++;
  src[PRERELEASELOOSE] = '(?:-?(' + src[PRERELEASEIDENTIFIERLOOSE] + '(?:\\.' + src[PRERELEASEIDENTIFIERLOOSE] + ')*))';

  // ## Build Metadata Identifier
  // Any combination of digits, letters, or hyphens.

  var BUILDIDENTIFIER = R++;
  src[BUILDIDENTIFIER] = '[0-9A-Za-z-]+';

  // ## Build Metadata
  // Plus sign, followed by one or more period-separated build metadata
  // identifiers.

  var BUILD = R++;
  src[BUILD] = '(?:\\+(' + src[BUILDIDENTIFIER] + '(?:\\.' + src[BUILDIDENTIFIER] + ')*))';

  // ## Full Version String
  // A main version, followed optionally by a pre-release version and
  // build metadata.

  // Note that the only major, minor, patch, and pre-release sections of
  // the version string are capturing groups.  The build metadata is not a
  // capturing group, because it should not ever be used in version
  // comparison.

  var FULL = R++;
  var FULLPLAIN = 'v?' + src[MAINVERSION] + src[PRERELEASE] + '?' + src[BUILD] + '?';

  src[FULL] = '^' + FULLPLAIN + '$';

  // like full, but allows v1.2.3 and =1.2.3, which people do sometimes.
  // also, 1.0.0alpha1 (prerelease without the hyphen) which is pretty
  // common in the npm registry.
  var LOOSEPLAIN = '[v=\\s]*' + src[MAINVERSIONLOOSE] + src[PRERELEASELOOSE] + '?' + src[BUILD] + '?';

  var LOOSE = R++;
  src[LOOSE] = '^' + LOOSEPLAIN + '$';

  var GTLT = R++;
  src[GTLT] = '((?:<|>)?=?)';

  // Something like "2.*" or "1.2.x".
  // Note that "x.x" is a valid xRange identifer, meaning "any version"
  // Only the first item is strictly required.
  var XRANGEIDENTIFIERLOOSE = R++;
  src[XRANGEIDENTIFIERLOOSE] = src[NUMERICIDENTIFIERLOOSE] + '|x|X|\\*';
  var XRANGEIDENTIFIER = R++;
  src[XRANGEIDENTIFIER] = src[NUMERICIDENTIFIER] + '|x|X|\\*';

  var XRANGEPLAIN = R++;
  src[XRANGEPLAIN] = '[v=\\s]*(' + src[XRANGEIDENTIFIER] + ')' + '(?:\\.(' + src[XRANGEIDENTIFIER] + ')' + '(?:\\.(' + src[XRANGEIDENTIFIER] + ')' + '(?:' + src[PRERELEASE] + ')?' + src[BUILD] + '?' + ')?)?';

  var XRANGEPLAINLOOSE = R++;
  src[XRANGEPLAINLOOSE] = '[v=\\s]*(' + src[XRANGEIDENTIFIERLOOSE] + ')' + '(?:\\.(' + src[XRANGEIDENTIFIERLOOSE] + ')' + '(?:\\.(' + src[XRANGEIDENTIFIERLOOSE] + ')' + '(?:' + src[PRERELEASELOOSE] + ')?' + src[BUILD] + '?' + ')?)?';

  var XRANGE = R++;
  src[XRANGE] = '^' + src[GTLT] + '\\s*' + src[XRANGEPLAIN] + '$';
  var XRANGELOOSE = R++;
  src[XRANGELOOSE] = '^' + src[GTLT] + '\\s*' + src[XRANGEPLAINLOOSE] + '$';

  // Tilde ranges.
  // Meaning is "reasonably at or greater than"
  var LONETILDE = R++;
  src[LONETILDE] = '(?:~>?)';

  var TILDETRIM = R++;
  src[TILDETRIM] = '(\\s*)' + src[LONETILDE] + '\\s+';
  re[TILDETRIM] = new RegExp(src[TILDETRIM], 'g');
  var tildeTrimReplace = '$1~';

  var TILDE = R++;
  src[TILDE] = '^' + src[LONETILDE] + src[XRANGEPLAIN] + '$';
  var TILDELOOSE = R++;
  src[TILDELOOSE] = '^' + src[LONETILDE] + src[XRANGEPLAINLOOSE] + '$';

  // Caret ranges.
  // Meaning is "at least and backwards compatible with"
  var LONECARET = R++;
  src[LONECARET] = '(?:\\^)';

  var CARETTRIM = R++;
  src[CARETTRIM] = '(\\s*)' + src[LONECARET] + '\\s+';
  re[CARETTRIM] = new RegExp(src[CARETTRIM], 'g');
  var caretTrimReplace = '$1^';

  var CARET = R++;
  src[CARET] = '^' + src[LONECARET] + src[XRANGEPLAIN] + '$';
  var CARETLOOSE = R++;
  src[CARETLOOSE] = '^' + src[LONECARET] + src[XRANGEPLAINLOOSE] + '$';

  // A simple gt/lt/eq thing, or just "" to indicate "any version"
  var COMPARATORLOOSE = R++;
  src[COMPARATORLOOSE] = '^' + src[GTLT] + '\\s*(' + LOOSEPLAIN + ')$|^$';
  var COMPARATOR = R++;
  src[COMPARATOR] = '^' + src[GTLT] + '\\s*(' + FULLPLAIN + ')$|^$';

  // An expression to strip any whitespace between the gtlt and the thing
  // it modifies, so that `> 1.2.3` ==> `>1.2.3`
  var COMPARATORTRIM = R++;
  src[COMPARATORTRIM] = '(\\s*)' + src[GTLT] + '\\s*(' + LOOSEPLAIN + '|' + src[XRANGEPLAIN] + ')';

  // this one has to use the /g flag
  re[COMPARATORTRIM] = new RegExp(src[COMPARATORTRIM], 'g');
  var comparatorTrimReplace = '$1$2$3';

  // Something like `1.2.3 - 1.2.4`
  // Note that these all use the loose form, because they'll be
  // checked against either the strict or loose comparator form
  // later.
  var HYPHENRANGE = R++;
  src[HYPHENRANGE] = '^\\s*(' + src[XRANGEPLAIN] + ')' + '\\s+-\\s+' + '(' + src[XRANGEPLAIN] + ')' + '\\s*$';

  var HYPHENRANGELOOSE = R++;
  src[HYPHENRANGELOOSE] = '^\\s*(' + src[XRANGEPLAINLOOSE] + ')' + '\\s+-\\s+' + '(' + src[XRANGEPLAINLOOSE] + ')' + '\\s*$';

  // Star ranges basically just allow anything at all.
  var STAR = R++;
  src[STAR] = '(<|>)?=?\\s*\\*';

  // Compile to actual regexp objects.
  // All are flag-free, unless they were created above with a flag.
  for (var i = 0; i < R; i++) {
    debug(i, src[i]);
    if (!re[i]) re[i] = new RegExp(src[i]);
  }

  exports.parse = parse;
  function parse(version, loose) {
    if (version instanceof SemVer) return version;

    if (typeof version !== 'string') return null;

    if (version.length > MAX_LENGTH) return null;

    var r = loose ? re[LOOSE] : re[FULL];
    if (!r.test(version)) return null;

    try {
      return new SemVer(version, loose);
    } catch (er) {
      return null;
    }
  }

  exports.valid = valid;
  function valid(version, loose) {
    var v = parse(version, loose);
    return v ? v.version : null;
  }

  exports.clean = clean;
  function clean(version, loose) {
    var s = parse(version.trim().replace(/^[=v]+/, ''), loose);
    return s ? s.version : null;
  }

  exports.SemVer = SemVer;

  function SemVer(version, loose) {
    if (version instanceof SemVer) {
      if (version.loose === loose) return version;else version = version.version;
    } else if (typeof version !== 'string') {
      throw new TypeError('Invalid Version: ' + version);
    }

    if (version.length > MAX_LENGTH) throw new TypeError('version is longer than ' + MAX_LENGTH + ' characters');

    if (!(this instanceof SemVer)) return new SemVer(version, loose);

    debug('SemVer', version, loose);
    this.loose = loose;
    var m = version.trim().match(loose ? re[LOOSE] : re[FULL]);

    if (!m) throw new TypeError('Invalid Version: ' + version);

    this.raw = version;

    // these are actually numbers
    this.major = +m[1];
    this.minor = +m[2];
    this.patch = +m[3];

    if (this.major > MAX_SAFE_INTEGER || this.major < 0) throw new TypeError('Invalid major version');

    if (this.minor > MAX_SAFE_INTEGER || this.minor < 0) throw new TypeError('Invalid minor version');

    if (this.patch > MAX_SAFE_INTEGER || this.patch < 0) throw new TypeError('Invalid patch version');

    // numberify any prerelease numeric ids
    if (!m[4]) this.prerelease = [];else this.prerelease = m[4].split('.').map(function (id) {
      if (/^[0-9]+$/.test(id)) {
        var num = +id;
        if (num >= 0 && num < MAX_SAFE_INTEGER) return num;
      }
      return id;
    });

    this.build = m[5] ? m[5].split('.') : [];
    this.format();
  }

  SemVer.prototype.format = function () {
    this.version = this.major + '.' + this.minor + '.' + this.patch;
    if (this.prerelease.length) this.version += '-' + this.prerelease.join('.');
    return this.version;
  };

  SemVer.prototype.toString = function () {
    return this.version;
  };

  SemVer.prototype.compare = function (other) {
    debug('SemVer.compare', this.version, this.loose, other);
    if (!(other instanceof SemVer)) other = new SemVer(other, this.loose);

    return this.compareMain(other) || this.comparePre(other);
  };

  SemVer.prototype.compareMain = function (other) {
    if (!(other instanceof SemVer)) other = new SemVer(other, this.loose);

    return compareIdentifiers(this.major, other.major) || compareIdentifiers(this.minor, other.minor) || compareIdentifiers(this.patch, other.patch);
  };

  SemVer.prototype.comparePre = function (other) {
    if (!(other instanceof SemVer)) other = new SemVer(other, this.loose);

    // NOT having a prerelease is > having one
    if (this.prerelease.length && !other.prerelease.length) return -1;else if (!this.prerelease.length && other.prerelease.length) return 1;else if (!this.prerelease.length && !other.prerelease.length) return 0;

    var i = 0;
    do {
      var a = this.prerelease[i];
      var b = other.prerelease[i];
      debug('prerelease compare', i, a, b);
      if (a === undefined && b === undefined) return 0;else if (b === undefined) return 1;else if (a === undefined) return -1;else if (a === b) continue;else return compareIdentifiers(a, b);
    } while (++i);
  };

  // preminor will bump the version up to the next minor release, and immediately
  // down to pre-release. premajor and prepatch work the same way.
  SemVer.prototype.inc = function (release, identifier) {
    switch (release) {
      case 'premajor':
        this.prerelease.length = 0;
        this.patch = 0;
        this.minor = 0;
        this.major++;
        this.inc('pre', identifier);
        break;
      case 'preminor':
        this.prerelease.length = 0;
        this.patch = 0;
        this.minor++;
        this.inc('pre', identifier);
        break;
      case 'prepatch':
        // If this is already a prerelease, it will bump to the next version
        // drop any prereleases that might already exist, since they are not
        // relevant at this point.
        this.prerelease.length = 0;
        this.inc('patch', identifier);
        this.inc('pre', identifier);
        break;
      // If the input is a non-prerelease version, this acts the same as
      // prepatch.
      case 'prerelease':
        if (this.prerelease.length === 0) this.inc('patch', identifier);
        this.inc('pre', identifier);
        break;

      case 'major':
        // If this is a pre-major version, bump up to the same major version.
        // Otherwise increment major.
        // 1.0.0-5 bumps to 1.0.0
        // 1.1.0 bumps to 2.0.0
        if (this.minor !== 0 || this.patch !== 0 || this.prerelease.length === 0) this.major++;
        this.minor = 0;
        this.patch = 0;
        this.prerelease = [];
        break;
      case 'minor':
        // If this is a pre-minor version, bump up to the same minor version.
        // Otherwise increment minor.
        // 1.2.0-5 bumps to 1.2.0
        // 1.2.1 bumps to 1.3.0
        if (this.patch !== 0 || this.prerelease.length === 0) this.minor++;
        this.patch = 0;
        this.prerelease = [];
        break;
      case 'patch':
        // If this is not a pre-release version, it will increment the patch.
        // If it is a pre-release it will bump up to the same patch version.
        // 1.2.0-5 patches to 1.2.0
        // 1.2.0 patches to 1.2.1
        if (this.prerelease.length === 0) this.patch++;
        this.prerelease = [];
        break;
      // This probably shouldn't be used publicly.
      // 1.0.0 "pre" would become 1.0.0-0 which is the wrong direction.
      case 'pre':
        if (this.prerelease.length === 0) this.prerelease = [0];else {
          var i = this.prerelease.length;
          while (--i >= 0) {
            if (typeof this.prerelease[i] === 'number') {
              this.prerelease[i]++;
              i = -2;
            }
          }
          if (i === -1) // didn't increment anything
            this.prerelease.push(0);
        }
        if (identifier) {
          // 1.2.0-beta.1 bumps to 1.2.0-beta.2,
          // 1.2.0-beta.fooblz or 1.2.0-beta bumps to 1.2.0-beta.0
          if (this.prerelease[0] === identifier) {
            if (isNaN(this.prerelease[1])) this.prerelease = [identifier, 0];
          } else this.prerelease = [identifier, 0];
        }
        break;

      default:
        throw new Error('invalid increment argument: ' + release);
    }
    this.format();
    this.raw = this.version;
    return this;
  };

  exports.inc = inc;
  function inc(version, release, loose, identifier) {
    if (typeof loose === 'string') {
      identifier = loose;
      loose = undefined;
    }

    try {
      return new SemVer(version, loose).inc(release, identifier).version;
    } catch (er) {
      return null;
    }
  }

  exports.diff = diff;
  function diff(version1, version2) {
    if (eq(version1, version2)) {
      return null;
    } else {
      var v1 = parse(version1);
      var v2 = parse(version2);
      if (v1.prerelease.length || v2.prerelease.length) {
        for (var key in v1) {
          if (key === 'major' || key === 'minor' || key === 'patch') {
            if (v1[key] !== v2[key]) {
              return 'pre' + key;
            }
          }
        }
        return 'prerelease';
      }
      for (var key in v1) {
        if (key === 'major' || key === 'minor' || key === 'patch') {
          if (v1[key] !== v2[key]) {
            return key;
          }
        }
      }
    }
  }

  exports.compareIdentifiers = compareIdentifiers;

  var numeric = /^[0-9]+$/;
  function compareIdentifiers(a, b) {
    var anum = numeric.test(a);
    var bnum = numeric.test(b);

    if (anum && bnum) {
      a = +a;
      b = +b;
    }

    return anum && !bnum ? -1 : bnum && !anum ? 1 : a < b ? -1 : a > b ? 1 : 0;
  }

  exports.rcompareIdentifiers = rcompareIdentifiers;
  function rcompareIdentifiers(a, b) {
    return compareIdentifiers(b, a);
  }

  exports.major = major;
  function major(a, loose) {
    return new SemVer(a, loose).major;
  }

  exports.minor = minor;
  function minor(a, loose) {
    return new SemVer(a, loose).minor;
  }

  exports.patch = patch;
  function patch(a, loose) {
    return new SemVer(a, loose).patch;
  }

  exports.compare = compare;
  function compare(a, b, loose) {
    return new SemVer(a, loose).compare(b);
  }

  exports.compareLoose = compareLoose;
  function compareLoose(a, b) {
    return compare(a, b, true);
  }

  exports.rcompare = rcompare;
  function rcompare(a, b, loose) {
    return compare(b, a, loose);
  }

  exports.sort = sort;
  function sort(list, loose) {
    return list.sort(function (a, b) {
      return exports.compare(a, b, loose);
    });
  }

  exports.rsort = rsort;
  function rsort(list, loose) {
    return list.sort(function (a, b) {
      return exports.rcompare(a, b, loose);
    });
  }

  exports.gt = gt;
  function gt(a, b, loose) {
    return compare(a, b, loose) > 0;
  }

  exports.lt = lt;
  function lt(a, b, loose) {
    return compare(a, b, loose) < 0;
  }

  exports.eq = eq;
  function eq(a, b, loose) {
    return compare(a, b, loose) === 0;
  }

  exports.neq = neq;
  function neq(a, b, loose) {
    return compare(a, b, loose) !== 0;
  }

  exports.gte = gte;
  function gte(a, b, loose) {
    return compare(a, b, loose) >= 0;
  }

  exports.lte = lte;
  function lte(a, b, loose) {
    return compare(a, b, loose) <= 0;
  }

  exports.cmp = cmp;
  function cmp(a, op, b, loose) {
    var ret;
    switch (op) {
      case '===':
        if ((typeof a === 'undefined' ? 'undefined' : _typeof(a)) === 'object') a = a.version;
        if ((typeof b === 'undefined' ? 'undefined' : _typeof(b)) === 'object') b = b.version;
        ret = a === b;
        break;
      case '!==':
        if ((typeof a === 'undefined' ? 'undefined' : _typeof(a)) === 'object') a = a.version;
        if ((typeof b === 'undefined' ? 'undefined' : _typeof(b)) === 'object') b = b.version;
        ret = a !== b;
        break;
      case '':case '=':case '==':
        ret = eq(a, b, loose);break;
      case '!=':
        ret = neq(a, b, loose);break;
      case '>':
        ret = gt(a, b, loose);break;
      case '>=':
        ret = gte(a, b, loose);break;
      case '<':
        ret = lt(a, b, loose);break;
      case '<=':
        ret = lte(a, b, loose);break;
      default:
        throw new TypeError('Invalid operator: ' + op);
    }
    return ret;
  }

  exports.Comparator = Comparator;
  function Comparator(comp, loose) {
    if (comp instanceof Comparator) {
      if (comp.loose === loose) return comp;else comp = comp.value;
    }

    if (!(this instanceof Comparator)) return new Comparator(comp, loose);

    debug('comparator', comp, loose);
    this.loose = loose;
    this.parse(comp);

    if (this.semver === ANY) this.value = '';else this.value = this.operator + this.semver.version;

    debug('comp', this);
  }

  var ANY = {};
  Comparator.prototype.parse = function (comp) {
    var r = this.loose ? re[COMPARATORLOOSE] : re[COMPARATOR];
    var m = comp.match(r);

    if (!m) throw new TypeError('Invalid comparator: ' + comp);

    this.operator = m[1];
    if (this.operator === '=') this.operator = '';

    // if it literally is just '>' or '' then allow anything.
    if (!m[2]) this.semver = ANY;else this.semver = new SemVer(m[2], this.loose);
  };

  Comparator.prototype.toString = function () {
    return this.value;
  };

  Comparator.prototype.test = function (version) {
    debug('Comparator.test', version, this.loose);

    if (this.semver === ANY) return true;

    if (typeof version === 'string') version = new SemVer(version, this.loose);

    return cmp(version, this.operator, this.semver, this.loose);
  };

  exports.Range = Range;
  function Range(range, loose) {
    if (range instanceof Range && range.loose === loose) return range;

    if (!(this instanceof Range)) return new Range(range, loose);

    this.loose = loose;

    // First, split based on boolean or ||
    this.raw = range;
    this.set = range.split(/\s*\|\|\s*/).map(function (range) {
      return this.parseRange(range.trim());
    }, this).filter(function (c) {
      // throw out any that are not relevant for whatever reason
      return c.length;
    });

    if (!this.set.length) {
      throw new TypeError('Invalid SemVer Range: ' + range);
    }

    this.format();
  }

  Range.prototype.format = function () {
    this.range = this.set.map(function (comps) {
      return comps.join(' ').trim();
    }).join('||').trim();
    return this.range;
  };

  Range.prototype.toString = function () {
    return this.range;
  };

  Range.prototype.parseRange = function (range) {
    var loose = this.loose;
    range = range.trim();
    debug('range', range, loose);
    // `1.2.3 - 1.2.4` => `>=1.2.3 <=1.2.4`
    var hr = loose ? re[HYPHENRANGELOOSE] : re[HYPHENRANGE];
    range = range.replace(hr, hyphenReplace);
    debug('hyphen replace', range);
    // `> 1.2.3 < 1.2.5` => `>1.2.3 <1.2.5`
    range = range.replace(re[COMPARATORTRIM], comparatorTrimReplace);
    debug('comparator trim', range, re[COMPARATORTRIM]);

    // `~ 1.2.3` => `~1.2.3`
    range = range.replace(re[TILDETRIM], tildeTrimReplace);

    // `^ 1.2.3` => `^1.2.3`
    range = range.replace(re[CARETTRIM], caretTrimReplace);

    // normalize spaces
    range = range.split(/\s+/).join(' ');

    // At this point, the range is completely trimmed and
    // ready to be split into comparators.

    var compRe = loose ? re[COMPARATORLOOSE] : re[COMPARATOR];
    var set = range.split(' ').map(function (comp) {
      return parseComparator(comp, loose);
    }).join(' ').split(/\s+/);
    if (this.loose) {
      // in loose mode, throw out any that are not valid comparators
      set = set.filter(function (comp) {
        return !!comp.match(compRe);
      });
    }
    set = set.map(function (comp) {
      return new Comparator(comp, loose);
    });

    return set;
  };

  // Mostly just for testing and legacy API reasons
  exports.toComparators = toComparators;
  function toComparators(range, loose) {
    return new Range(range, loose).set.map(function (comp) {
      return comp.map(function (c) {
        return c.value;
      }).join(' ').trim().split(' ');
    });
  }

  // comprised of xranges, tildes, stars, and gtlt's at this point.
  // already replaced the hyphen ranges
  // turn into a set of JUST comparators.
  function parseComparator(comp, loose) {
    debug('comp', comp);
    comp = replaceCarets(comp, loose);
    debug('caret', comp);
    comp = replaceTildes(comp, loose);
    debug('tildes', comp);
    comp = replaceXRanges(comp, loose);
    debug('xrange', comp);
    comp = replaceStars(comp, loose);
    debug('stars', comp);
    return comp;
  }

  function isX(id) {
    return !id || id.toLowerCase() === 'x' || id === '*';
  }

  // ~, ~> --> * (any, kinda silly)
  // ~2, ~2.x, ~2.x.x, ~>2, ~>2.x ~>2.x.x --> >=2.0.0 <3.0.0
  // ~2.0, ~2.0.x, ~>2.0, ~>2.0.x --> >=2.0.0 <2.1.0
  // ~1.2, ~1.2.x, ~>1.2, ~>1.2.x --> >=1.2.0 <1.3.0
  // ~1.2.3, ~>1.2.3 --> >=1.2.3 <1.3.0
  // ~1.2.0, ~>1.2.0 --> >=1.2.0 <1.3.0
  function replaceTildes(comp, loose) {
    return comp.trim().split(/\s+/).map(function (comp) {
      return replaceTilde(comp, loose);
    }).join(' ');
  }

  function replaceTilde(comp, loose) {
    var r = loose ? re[TILDELOOSE] : re[TILDE];
    return comp.replace(r, function (_, M, m, p, pr) {
      debug('tilde', comp, _, M, m, p, pr);
      var ret;

      if (isX(M)) ret = '';else if (isX(m)) ret = '>=' + M + '.0.0 <' + (+M + 1) + '.0.0';else if (isX(p))
        // ~1.2 == >=1.2.0 <1.3.0
        ret = '>=' + M + '.' + m + '.0 <' + M + '.' + (+m + 1) + '.0';else if (pr) {
        debug('replaceTilde pr', pr);
        if (pr.charAt(0) !== '-') pr = '-' + pr;
        ret = '>=' + M + '.' + m + '.' + p + pr + ' <' + M + '.' + (+m + 1) + '.0';
      } else
        // ~1.2.3 == >=1.2.3 <1.3.0
        ret = '>=' + M + '.' + m + '.' + p + ' <' + M + '.' + (+m + 1) + '.0';

      debug('tilde return', ret);
      return ret;
    });
  }

  // ^ --> * (any, kinda silly)
  // ^2, ^2.x, ^2.x.x --> >=2.0.0 <3.0.0
  // ^2.0, ^2.0.x --> >=2.0.0 <3.0.0
  // ^1.2, ^1.2.x --> >=1.2.0 <2.0.0
  // ^1.2.3 --> >=1.2.3 <2.0.0
  // ^1.2.0 --> >=1.2.0 <2.0.0
  function replaceCarets(comp, loose) {
    return comp.trim().split(/\s+/).map(function (comp) {
      return replaceCaret(comp, loose);
    }).join(' ');
  }

  function replaceCaret(comp, loose) {
    debug('caret', comp, loose);
    var r = loose ? re[CARETLOOSE] : re[CARET];
    return comp.replace(r, function (_, M, m, p, pr) {
      debug('caret', comp, _, M, m, p, pr);
      var ret;

      if (isX(M)) ret = '';else if (isX(m)) ret = '>=' + M + '.0.0 <' + (+M + 1) + '.0.0';else if (isX(p)) {
        if (M === '0') ret = '>=' + M + '.' + m + '.0 <' + M + '.' + (+m + 1) + '.0';else ret = '>=' + M + '.' + m + '.0 <' + (+M + 1) + '.0.0';
      } else if (pr) {
        debug('replaceCaret pr', pr);
        if (pr.charAt(0) !== '-') pr = '-' + pr;
        if (M === '0') {
          if (m === '0') ret = '>=' + M + '.' + m + '.' + p + pr + ' <' + M + '.' + m + '.' + (+p + 1);else ret = '>=' + M + '.' + m + '.' + p + pr + ' <' + M + '.' + (+m + 1) + '.0';
        } else ret = '>=' + M + '.' + m + '.' + p + pr + ' <' + (+M + 1) + '.0.0';
      } else {
        debug('no pr');
        if (M === '0') {
          if (m === '0') ret = '>=' + M + '.' + m + '.' + p + ' <' + M + '.' + m + '.' + (+p + 1);else ret = '>=' + M + '.' + m + '.' + p + ' <' + M + '.' + (+m + 1) + '.0';
        } else ret = '>=' + M + '.' + m + '.' + p + ' <' + (+M + 1) + '.0.0';
      }

      debug('caret return', ret);
      return ret;
    });
  }

  function replaceXRanges(comp, loose) {
    debug('replaceXRanges', comp, loose);
    return comp.split(/\s+/).map(function (comp) {
      return replaceXRange(comp, loose);
    }).join(' ');
  }

  function replaceXRange(comp, loose) {
    comp = comp.trim();
    var r = loose ? re[XRANGELOOSE] : re[XRANGE];
    return comp.replace(r, function (ret, gtlt, M, m, p, pr) {
      debug('xRange', comp, ret, gtlt, M, m, p, pr);
      var xM = isX(M);
      var xm = xM || isX(m);
      var xp = xm || isX(p);
      var anyX = xp;

      if (gtlt === '=' && anyX) gtlt = '';

      if (xM) {
        if (gtlt === '>' || gtlt === '<') {
          // nothing is allowed
          ret = '<0.0.0';
        } else {
          // nothing is forbidden
          ret = '*';
        }
      } else if (gtlt && anyX) {
        // replace X with 0
        if (xm) m = 0;
        if (xp) p = 0;

        if (gtlt === '>') {
          // >1 => >=2.0.0
          // >1.2 => >=1.3.0
          // >1.2.3 => >= 1.2.4
          gtlt = '>=';
          if (xm) {
            M = +M + 1;
            m = 0;
            p = 0;
          } else if (xp) {
            m = +m + 1;
            p = 0;
          }
        } else if (gtlt === '<=') {
          // <=0.7.x is actually <0.8.0, since any 0.7.x should
          // pass.  Similarly, <=7.x is actually <8.0.0, etc.
          gtlt = '<';
          if (xm) M = +M + 1;else m = +m + 1;
        }

        ret = gtlt + M + '.' + m + '.' + p;
      } else if (xm) {
        ret = '>=' + M + '.0.0 <' + (+M + 1) + '.0.0';
      } else if (xp) {
        ret = '>=' + M + '.' + m + '.0 <' + M + '.' + (+m + 1) + '.0';
      }

      debug('xRange return', ret);

      return ret;
    });
  }

  // Because * is AND-ed with everything else in the comparator,
  // and '' means "any version", just remove the *s entirely.
  function replaceStars(comp, loose) {
    debug('replaceStars', comp, loose);
    // Looseness is ignored here.  star is always as loose as it gets!
    return comp.trim().replace(re[STAR], '');
  }

  // This function is passed to string.replace(re[HYPHENRANGE])
  // M, m, patch, prerelease, build
  // 1.2 - 3.4.5 => >=1.2.0 <=3.4.5
  // 1.2.3 - 3.4 => >=1.2.0 <3.5.0 Any 3.4.x will do
  // 1.2 - 3.4 => >=1.2.0 <3.5.0
  function hyphenReplace($0, from, fM, fm, fp, fpr, fb, to, tM, tm, tp, tpr, tb) {

    if (isX(fM)) from = '';else if (isX(fm)) from = '>=' + fM + '.0.0';else if (isX(fp)) from = '>=' + fM + '.' + fm + '.0';else from = '>=' + from;

    if (isX(tM)) to = '';else if (isX(tm)) to = '<' + (+tM + 1) + '.0.0';else if (isX(tp)) to = '<' + tM + '.' + (+tm + 1) + '.0';else if (tpr) to = '<=' + tM + '.' + tm + '.' + tp + '-' + tpr;else to = '<=' + to;

    return (from + ' ' + to).trim();
  }

  // if ANY of the sets match ALL of its comparators, then pass
  Range.prototype.test = function (version) {
    if (!version) return false;

    if (typeof version === 'string') version = new SemVer(version, this.loose);

    for (var i = 0; i < this.set.length; i++) {
      if (testSet(this.set[i], version)) return true;
    }
    return false;
  };

  function testSet(set, version) {
    for (var i = 0; i < set.length; i++) {
      if (!set[i].test(version)) return false;
    }

    if (version.prerelease.length) {
      // Find the set of versions that are allowed to have prereleases
      // For example, ^1.2.3-pr.1 desugars to >=1.2.3-pr.1 <2.0.0
      // That should allow `1.2.3-pr.2` to pass.
      // However, `1.2.4-alpha.notready` should NOT be allowed,
      // even though it's within the range set by the comparators.
      for (var i = 0; i < set.length; i++) {
        debug(set[i].semver);
        if (set[i].semver === ANY) continue;

        if (set[i].semver.prerelease.length > 0) {
          var allowed = set[i].semver;
          if (allowed.major === version.major && allowed.minor === version.minor && allowed.patch === version.patch) return true;
        }
      }

      // Version has a -pre, but it's not one of the ones we like.
      return false;
    }

    return true;
  }

  exports.satisfies = satisfies;
  function satisfies(version, range, loose) {
    try {
      range = new Range(range, loose);
    } catch (er) {
      return false;
    }
    return range.test(version);
  }

  exports.maxSatisfying = maxSatisfying;
  function maxSatisfying(versions, range, loose) {
    return versions.filter(function (version) {
      return satisfies(version, range, loose);
    }).sort(function (a, b) {
      return rcompare(a, b, loose);
    })[0] || null;
  }

  exports.minSatisfying = minSatisfying;
  function minSatisfying(versions, range, loose) {
    return versions.filter(function (version) {
      return satisfies(version, range, loose);
    }).sort(function (a, b) {
      return compare(a, b, loose);
    })[0] || null;
  }

  exports.validRange = validRange;
  function validRange(range, loose) {
    try {
      // Return '*' instead of '' so that truthiness works.
      // This will throw if it's invalid anyway
      return new Range(range, loose).range || '*';
    } catch (er) {
      return null;
    }
  }

  // Determine if version is less than all the versions possible in the range
  exports.ltr = ltr;
  function ltr(version, range, loose) {
    return outside(version, range, '<', loose);
  }

  // Determine if version is greater than all the versions possible in the range.
  exports.gtr = gtr;
  function gtr(version, range, loose) {
    return outside(version, range, '>', loose);
  }

  exports.outside = outside;
  function outside(version, range, hilo, loose) {
    version = new SemVer(version, loose);
    range = new Range(range, loose);

    var gtfn, ltefn, ltfn, comp, ecomp;
    switch (hilo) {
      case '>':
        gtfn = gt;
        ltefn = lte;
        ltfn = lt;
        comp = '>';
        ecomp = '>=';
        break;
      case '<':
        gtfn = lt;
        ltefn = gte;
        ltfn = gt;
        comp = '<';
        ecomp = '<=';
        break;
      default:
        throw new TypeError('Must provide a hilo val of "<" or ">"');
    }

    // If it satisifes the range it is not outside
    if (satisfies(version, range, loose)) {
      return false;
    }

    // From now on, variable terms are as if we're in "gtr" mode.
    // but note that everything is flipped for the "ltr" function.

    for (var i = 0; i < range.set.length; ++i) {
      var comparators = range.set[i];

      var high = null;
      var low = null;

      comparators.forEach(function (comparator) {
        if (comparator.semver === ANY) {
          comparator = new Comparator('>=0.0.0');
        }
        high = high || comparator;
        low = low || comparator;
        if (gtfn(comparator.semver, high.semver, loose)) {
          high = comparator;
        } else if (ltfn(comparator.semver, low.semver, loose)) {
          low = comparator;
        }
      });

      // If the edge version comparator has a operator then our version
      // isn't outside it
      if (high.operator === comp || high.operator === ecomp) {
        return false;
      }

      // If the lowest version comparator has an operator and our version
      // is less than it then it isn't higher than the range
      if ((!low.operator || low.operator === comp) && ltefn(version, low.semver)) {
        return false;
      } else if (low.operator === ecomp && ltfn(version, low.semver)) {
        return false;
      }
    }
    return true;
  }

  exports.prerelease = prerelease;
  function prerelease(version, loose) {
    var parsed = parse(version, loose);
    return parsed && parsed.prerelease.length ? parsed.prerelease : null;
  }
  /* WEBPACK VAR INJECTION */}.call(exports, __webpack_require__(87)))

/***/ },
/* 87 */
/***/ function(module, exports) {

  'use strict';

  // shim for using process in browser

  var process = module.exports = {};

  // cached from whatever global is present so that test runners that stub it
  // don't break things.  But we need to wrap it in a try catch in case it is
  // wrapped in strict mode code which doesn't define any globals.  It's inside a
  // function because try/catches deoptimize in certain engines.

  var cachedSetTimeout;
  var cachedClearTimeout;

  (function () {
      try {
          cachedSetTimeout = setTimeout;
      } catch (e) {
          cachedSetTimeout = function cachedSetTimeout() {
              throw new Error('setTimeout is not defined');
          };
      }
      try {
          cachedClearTimeout = clearTimeout;
      } catch (e) {
          cachedClearTimeout = function cachedClearTimeout() {
              throw new Error('clearTimeout is not defined');
          };
      }
  })();
  var queue = [];
  var draining = false;
  var currentQueue;
  var queueIndex = -1;

  function cleanUpNextTick() {
      if (!draining || !currentQueue) {
          return;
      }
      draining = false;
      if (currentQueue.length) {
          queue = currentQueue.concat(queue);
      } else {
          queueIndex = -1;
      }
      if (queue.length) {
          drainQueue();
      }
  }

  function drainQueue() {
      if (draining) {
          return;
      }
      var timeout = cachedSetTimeout(cleanUpNextTick);
      draining = true;

      var len = queue.length;
      while (len) {
          currentQueue = queue;
          queue = [];
          while (++queueIndex < len) {
              if (currentQueue) {
                  currentQueue[queueIndex].run();
              }
          }
          queueIndex = -1;
          len = queue.length;
      }
      currentQueue = null;
      draining = false;
      cachedClearTimeout(timeout);
  }

  process.nextTick = function (fun) {
      var args = new Array(arguments.length - 1);
      if (arguments.length > 1) {
          for (var i = 1; i < arguments.length; i++) {
              args[i - 1] = arguments[i];
          }
      }
      queue.push(new Item(fun, args));
      if (queue.length === 1 && !draining) {
          cachedSetTimeout(drainQueue, 0);
      }
  };

  // v8 likes predictible objects
  function Item(fun, array) {
      this.fun = fun;
      this.array = array;
  }
  Item.prototype.run = function () {
      this.fun.apply(null, this.array);
  };
  process.title = 'browser';
  process.browser = true;
  process.env = {};
  process.argv = [];
  process.version = ''; // empty string to avoid regexp issues
  process.versions = {};

  function noop() {}

  process.on = noop;
  process.addListener = noop;
  process.once = noop;
  process.off = noop;
  process.removeListener = noop;
  process.removeAllListeners = noop;
  process.emit = noop;

  process.binding = function (name) {
      throw new Error('process.binding is not supported');
  };

  process.cwd = function () {
      return '/';
  };
  process.chdir = function (dir) {
      throw new Error('process.chdir is not supported');
  };
  process.umask = function () {
      return 0;
  };

/***/ },
/* 88 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  Object.defineProperty(exports, "__esModule", {
    value: true
  });
  exports.default = Vm;

  var _util = __webpack_require__(84);

  var _state = __webpack_require__(89);

  var _compiler = __webpack_require__(94);

  var _events = __webpack_require__(97);

  /**
   * ViewModel constructor
   *
   * @param {string} type
   * @param {object} options    component options
   * @param {object} parentVm   which contains _app
   * @param {object} parentEl   root element or frag block
   * @param {object} mergedData external data
   * @param {object} externalEvents external events
   */
  /**
   * @fileOverview
   * ViewModel Constructor & definition
   */

  function Vm(type, options, parentVm, parentEl, mergedData, externalEvents) {
    this._parent = parentVm._realParent ? parentVm._realParent : parentVm;
    this._app = parentVm._app;
    parentVm._childrenVms && parentVm._childrenVms.push(this);

    if (!options) {
      options = this._app.customComponentMap[type] || {};
    }
    var data = options.data || {};

    this._options = options;
    this._methods = options.methods || {};
    this._computed = options.computed || {};
    this._css = options.style || {};
    this._ids = {};
    this._vmEvents = {};
    this._childrenVms = [];
    this._type = type;

    // bind events and lifecycles
    (0, _events.initEvents)(this, externalEvents);

    console.debug('[JS Framework] "init" lifecycle in Vm(' + this._type + ')');
    this.$emit('hook:init');
    this._inited = true;

    // proxy data and methods
    // observe data and add this to vms
    this._data = typeof data === 'function' ? data() : data;
    if (mergedData) {
      (0, _util.extend)(this._data, mergedData);
    }
    (0, _state.initState)(this);

    console.debug('[JS Framework] "created" lifecycle in Vm(' + this._type + ')');
    this.$emit('hook:created');
    this._created = true;

    // backward old ready entry
    if (options.methods && options.methods.ready) {
      console.warn('"exports.methods.ready" is deprecated, ' + 'please use "exports.created" instead');
      options.methods.ready.call(this);
    }

    // if no parentElement then specify the documentElement
    this._parentEl = parentEl || this._app.doc.documentElement;
    (0, _compiler.build)(this);
  }

  (0, _events.mixinEvents)(Vm.prototype);

/***/ },
/* 89 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  Object.defineProperty(exports, "__esModule", {
    value: true
  });
  exports.initState = initState;
  exports.initData = initData;
  exports.initComputed = initComputed;
  exports.initMethods = initMethods;

  var _watcher = __webpack_require__(90);

  var _watcher2 = _interopRequireDefault(_watcher);

  var _dep = __webpack_require__(91);

  var _dep2 = _interopRequireDefault(_dep);

  var _observer = __webpack_require__(92);

  var _util = __webpack_require__(84);

  function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

  /* eslint-disable */

  function initState(vm) {
    vm._watchers = [];
    initData(vm);
    initComputed(vm);
    initMethods(vm);
  }

  function initData(vm) {
    var data = vm._data;

    if (!(0, _util.isPlainObject)(data)) {
      data = {};
    }
    // proxy data on instance
    var keys = Object.keys(data);
    var i = keys.length;
    while (i--) {
      (0, _observer.proxy)(vm, keys[i]);
    }
    // observe data
    (0, _observer.observe)(data, vm);
  }

  function noop() {}

  function initComputed(vm) {
    var computed = vm._computed;
    if (computed) {
      for (var key in computed) {
        var userDef = computed[key];
        var def = {
          enumerable: true,
          configurable: true
        };
        if (typeof userDef === 'function') {
          def.get = makeComputedGetter(userDef, vm);
          def.set = noop;
        } else {
          def.get = userDef.get ? userDef.cache !== false ? makeComputedGetter(userDef.get, vm) : (0, _util.bind)(userDef.get, vm) : noop;
          def.set = userDef.set ? (0, _util.bind)(userDef.set, vm) : noop;
        }
        Object.defineProperty(vm, key, def);
      }
    }
  }

  function makeComputedGetter(getter, owner) {
    var watcher = new _watcher2.default(owner, getter, null, {
      lazy: true
    });
    return function computedGetter() {
      if (watcher.dirty) {
        watcher.evaluate();
      }
      if (_dep2.default.target) {
        watcher.depend();
      }
      return watcher.value;
    };
  }

  function initMethods(vm) {
    var methods = vm._methods;
    if (methods) {
      for (var key in methods) {
        vm[key] = methods[key];
      }
    }
  }

/***/ },
/* 90 */
/***/ function(module, exports, __webpack_require__) {

  /* WEBPACK VAR INJECTION */(function(process) {'use strict';

  Object.defineProperty(exports, "__esModule", {
    value: true
  });
  exports.default = Watcher;

  var _dep = __webpack_require__(91);

  var _dep2 = _interopRequireDefault(_dep);

  var _util = __webpack_require__(84);

  function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

  /* eslint-disable */

  var uid = 0;
  // import { pushWatcher } from './batcher'

  var prevTarget = void 0;

  /**
   * A watcher parses an expression, collects dependencies,
   * and fires callback when the expression value changes.
   * This is used for both the $watch() api and directives.
   *
   * @param {Vue} vm
   * @param {String|Function} expOrFn
   * @param {Function} cb
   * @param {Object} options
   *                 - {Array} filters
   *                 - {Boolean} twoWay
   *                 - {Boolean} deep
   *                 - {Boolean} user
   *                 - {Boolean} sync
   *                 - {Boolean} lazy
   *                 - {Function} [preProcess]
   *                 - {Function} [postProcess]
   * @constructor
   */

  function Watcher(vm, expOrFn, cb, options) {
    // mix in options
    if (options) {
      (0, _util.extend)(this, options);
    }
    var isFn = typeof expOrFn === 'function';
    this.vm = vm;
    vm._watchers.push(this);
    this.expression = expOrFn;
    this.cb = cb;
    this.id = ++uid; // uid for batching
    this.active = true;
    this.dirty = this.lazy; // for lazy watchers
    this.deps = [];
    this.newDeps = [];
    this.depIds = new _util._Set();
    this.newDepIds = new _util._Set();
    // parse expression for getter
    if (isFn) {
      this.getter = expOrFn;
    } else {
      this.getter = (0, _util.parsePath)(expOrFn);
      if (!this.getter) {
        this.getter = function () {};
        process.env.NODE_ENV !== 'production' && (0, _util.warn)('Failed watching path: ' + expOrFn + 'Watcher only accepts simple dot-delimited paths. ' + 'For full control, use a function instead.', vm);
      }
    }
    this.value = this.lazy ? undefined : this.get();
    // state for avoiding false triggers for deep and Array
    // watchers during vm._digest()
    this.queued = this.shallow = false;
  }

  /**
   * Evaluate the getter, and re-collect dependencies.
   */

  Watcher.prototype.get = function () {
    this.beforeGet();
    var value = this.getter.call(this.vm, this.vm);
    // "touch" every property so they are all tracked as
    // dependencies for deep watching
    if (this.deep) {
      traverse(value);
    }
    this.afterGet();
    return value;
  };

  /**
   * Prepare for dependency collection.
   */

  Watcher.prototype.beforeGet = function () {
    prevTarget = _dep2.default.target;
    _dep2.default.target = this;
  };

  /**
   * Add a dependency to this directive.
   *
   * @param {Dep} dep
   */

  Watcher.prototype.addDep = function (dep) {
    var id = dep.id;
    if (!this.newDepIds.has(id)) {
      this.newDepIds.add(id);
      this.newDeps.push(dep);
      if (!this.depIds.has(id)) {
        dep.addSub(this);
      }
    }
  };

  /**
   * Clean up for dependency collection.
   */

  Watcher.prototype.afterGet = function () {
    _dep2.default.target = prevTarget;
    var i = this.deps.length;
    while (i--) {
      var dep = this.deps[i];
      if (!this.newDepIds.has(dep.id)) {
        dep.removeSub(this);
      }
    }
    var tmp = this.depIds;
    this.depIds = this.newDepIds;
    this.newDepIds = tmp;
    this.newDepIds.clear();
    tmp = this.deps;
    this.deps = this.newDeps;
    this.newDeps = tmp;
    this.newDeps.length = 0;
  };

  /**
   * Subscriber interface.
   * Will be called when a dependency changes.
   *
   * @param {Boolean} shallow
   */

  Watcher.prototype.update = function (shallow) {
    if (this.lazy) {
      this.dirty = true;
    } else {
      this.run();
    }
    // } else if (this.sync) {
    //   this.run()
    // } else {
    //   // if queued, only overwrite shallow with non-shallow,
    //   // but not the other way around.
    //   this.shallow = this.queued
    //     ? shallow
    //       ? this.shallow
    //       : false
    //     : !!shallow
    //   this.queued = true
    //   pushWatcher(this)
    // }
  };

  /**
   * Batcher job interface.
   * Will be called by the batcher.
   */

  Watcher.prototype.run = function () {
    if (this.active) {
      var value = this.get();
      if (value !== this.value ||
      // Deep watchers and watchers on Object/Arrays should fire even
      // when the value is the same, because the value may
      // have mutated; but only do so if this is a
      // non-shallow update (caused by a vm digest).
      ((0, _util.isObject)(value) || this.deep) && !this.shallow) {
        // set new value
        var oldValue = this.value;
        this.value = value;
        this.cb.call(this.vm, value, oldValue);
      }
      this.queued = this.shallow = false;
    }
  };

  /**
   * Evaluate the value of the watcher.
   * This only gets called for lazy watchers.
   */

  Watcher.prototype.evaluate = function () {
    // avoid overwriting another watcher that is being
    // collected.
    var current = _dep2.default.target;
    this.value = this.get();
    this.dirty = false;
    _dep2.default.target = current;
  };

  /**
   * Depend on all deps collected by this watcher.
   */

  Watcher.prototype.depend = function () {
    var i = this.deps.length;
    while (i--) {
      this.deps[i].depend();
    }
  };

  /**
   * Remove self from all dependencies' subcriber list.
   */

  Watcher.prototype.teardown = function () {
    if (this.active) {
      // remove self from vm's watcher list
      // this is a somewhat expensive operation so we skip it
      // if the vm is being destroyed or is performing a v-for
      // re-render (the watcher list is then filtered by v-for).
      if (!this.vm._isBeingDestroyed && !this.vm._vForRemoving) {
        (0, _util.remove)(this.vm._watchers, this);
      }
      var i = this.deps.length;
      while (i--) {
        this.deps[i].removeSub(this);
      }
      this.active = false;
      this.vm = this.cb = this.value = null;
    }
  };

  /**
   * Recrusively traverse an object to evoke all converted
   * getters, so that every nested property inside the object
   * is collected as a "deep" dependency.
   *
   * @param {*} val
   * @param {Set} seen
   */

  var seenObjects = new _util._Set();
  function traverse(val, seen) {
    var i = void 0,
        keys = void 0,
        isA = void 0,
        isO = void 0;
    if (!seen) {
      seen = seenObjects;
      seen.clear();
    }
    isA = Array.isArray(val);
    isO = (0, _util.isObject)(val);
    if (isA || isO) {
      if (val.__ob__) {
        var depId = val.__ob__.dep.id;
        if (seen.has(depId)) {
          return;
        } else {
          seen.add(depId);
        }
      }
      if (isA) {
        i = val.length;
        while (i--) {
          traverse(val[i], seen);
        }
      } else if (isO) {
        keys = Object.keys(val);
        i = keys.length;
        while (i--) {
          traverse(val[keys[i]], seen);
        }
      }
    }
  }
  /* WEBPACK VAR INJECTION */}.call(exports, __webpack_require__(87)))

/***/ },
/* 91 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  Object.defineProperty(exports, "__esModule", {
    value: true
  });
  exports.default = Dep;

  var _util = __webpack_require__(84);

  var uid = 0;

  /**
   * A dep is an observable that can have multiple
   * directives subscribing to it.
   *
   * @constructor
   */

  /* eslint-disable */

  function Dep() {
    this.id = uid++;
    this.subs = [];
  }

  // the current target watcher being evaluated.
  // this is globally unique because there could be only one
  // watcher being evaluated at any time.
  Dep.target = null;

  /**
   * Add a directive subscriber.
   *
   * @param {Directive} sub
   */

  Dep.prototype.addSub = function (sub) {
    this.subs.push(sub);
  };

  /**
   * Remove a directive subscriber.
   *
   * @param {Directive} sub
   */

  Dep.prototype.removeSub = function (sub) {
    (0, _util.remove)(this.subs, sub);
  };

  /**
   * Add self as a dependency to the target watcher.
   */

  Dep.prototype.depend = function () {
    Dep.target.addDep(this);
  };

  /**
   * Notify all subscribers of a new value.
   */

  Dep.prototype.notify = function () {
    // stablize the subscriber list first
    var subs = this.subs.slice();
    for (var i = 0, l = subs.length; i < l; i++) {
      subs[i].update();
    }
  };

/***/ },
/* 92 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  Object.defineProperty(exports, "__esModule", {
    value: true
  });
  exports.Observer = Observer;
  exports.observe = observe;
  exports.defineReactive = defineReactive;
  exports.set = set;
  exports.del = del;
  exports.proxy = proxy;
  exports.unproxy = unproxy;

  var _dep = __webpack_require__(91);

  var _dep2 = _interopRequireDefault(_dep);

  var _array = __webpack_require__(93);

  var _util = __webpack_require__(84);

  function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

  var arrayKeys = Object.getOwnPropertyNames(_array.arrayMethods);

  /**
   * Observer class that are attached to each observed
   * object. Once attached, the observer converts target
   * object's property keys into getter/setters that
   * collect dependencies and dispatches updates.
   *
   * @param {Array|Object} value
   * @constructor
   */

  /* eslint-disable */

  function Observer(value) {
    this.value = value;
    this.dep = new _dep2.default();
    (0, _util.def)(value, '__ob__', this);
    if (Array.isArray(value)) {
      var augment = _util.hasProto ? protoAugment : copyAugment;
      augment(value, _array.arrayMethods, arrayKeys);
      this.observeArray(value);
    } else {
      this.walk(value);
    }
  }

  // Instance methods

  /**
   * Walk through each property and convert them into
   * getter/setters. This method should only be called when
   * value type is Object.
   *
   * @param {Object} obj
   */

  Observer.prototype.walk = function (obj) {
    for (var key in obj) {
      this.convert(key, obj[key]);
    }
  };

  /**
   * Observe a list of Array items.
   *
   * @param {Array} items
   */

  Observer.prototype.observeArray = function (items) {
    for (var i = 0, l = items.length; i < l; i++) {
      observe(items[i]);
    }
  };

  /**
   * Convert a property into getter/setter so we can emit
   * the events when the property is accessed/changed.
   *
   * @param {String} key
   * @param {*} val
   */

  Observer.prototype.convert = function (key, val) {
    defineReactive(this.value, key, val);
  };

  /**
   * Add an owner vm, so that when $set/$delete mutations
   * happen we can notify owner vms to proxy the keys and
   * digest the watchers. This is only called when the object
   * is observed as an instance's root $data.
   *
   * @param {Vue} vm
   */

  Observer.prototype.addVm = function (vm) {
    (this.vms || (this.vms = [])).push(vm);
  };

  /**
   * Remove an owner vm. This is called when the object is
   * swapped out as an instance's $data object.
   *
   * @param {Vue} vm
   */

  Observer.prototype.removeVm = function (vm) {
    (0, _util.remove)(this.vms, vm);
  };

  // helpers

  /**
   * Augment an target Object or Array by intercepting
   * the prototype chain using __proto__
   *
   * @param {Object|Array} target
   * @param {Object} src
   */

  function protoAugment(target, src) {
    /* eslint-disable no-proto */
    target.__proto__ = src;
    /* eslint-enable no-proto */
  }

  /**
   * Augment an target Object or Array by defining
   * hidden properties.
   *
   * @param {Object|Array} target
   * @param {Object} proto
   */

  function copyAugment(target, src, keys) {
    for (var i = 0, l = keys.length; i < l; i++) {
      var key = keys[i];
      (0, _util.def)(target, key, src[key]);
    }
  }

  /**
   * Attempt to create an observer instance for a value,
   * returns the new observer if successfully observed,
   * or the existing observer if the value already has one.
   *
   * @param {*} value
   * @param {Vue} [vm]
   * @return {Observer|undefined}
   * @static
   */

  function observe(value, vm) {
    if (!(0, _util.isObject)(value)) {
      return;
    }
    var ob = void 0;
    if ((0, _util.hasOwn)(value, '__ob__') && value.__ob__ instanceof Observer) {
      ob = value.__ob__;
    } else if ((Array.isArray(value) || (0, _util.isPlainObject)(value)) && Object.isExtensible(value) && !value._isVue) {
      ob = new Observer(value);
    }
    if (ob && vm) {
      ob.addVm(vm);
    }
    return ob;
  }

  /**
   * Define a reactive property on an Object.
   *
   * @param {Object} obj
   * @param {String} key
   * @param {*} val
   */

  function defineReactive(obj, key, val) {
    var dep = new _dep2.default();

    var property = Object.getOwnPropertyDescriptor(obj, key);
    if (property && property.configurable === false) {
      return;
    }

    // cater for pre-defined getter/setters
    var getter = property && property.get;
    var setter = property && property.set;

    var childOb = observe(val);
    Object.defineProperty(obj, key, {
      enumerable: true,
      configurable: true,
      get: function reactiveGetter() {
        var value = getter ? getter.call(obj) : val;
        if (_dep2.default.target) {
          dep.depend();
          if (childOb) {
            childOb.dep.depend();
          }
          if (Array.isArray(value)) {
            for (var e, i = 0, l = value.length; i < l; i++) {
              e = value[i];
              e && e.__ob__ && e.__ob__.dep.depend();
            }
          }
        }
        return value;
      },
      set: function reactiveSetter(newVal) {
        var value = getter ? getter.call(obj) : val;
        if (newVal === value) {
          return;
        }
        if (setter) {
          setter.call(obj, newVal);
        } else {
          val = newVal;
        }
        childOb = observe(newVal);
        dep.notify();
      }
    });
  }

  /**
   * Set a property on an object. Adds the new property and
   * triggers change notification if the property doesn't
   * already exist.
   *
   * @param {Object} obj
   * @param {String} key
   * @param {*} val
   * @public
   */

  function set(obj, key, val) {
    if (Array.isArray(obj)) {
      return obj.splice(key, 1, val);
    }
    if ((0, _util.hasOwn)(obj, key)) {
      obj[key] = val;
      return;
    }
    if (obj._isVue) {
      set(obj._data, key, val);
      return;
    }
    var ob = obj.__ob__;
    if (!ob) {
      obj[key] = val;
      return;
    }
    ob.convert(key, val);
    ob.dep.notify();
    if (ob.vms) {
      var i = ob.vms.length;
      while (i--) {
        var vm = ob.vms[i];
        proxy(vm, key);
        vm.$forceUpdate();
      }
    }
    return val;
  }

  /**
   * Delete a property and trigger change if necessary.
   *
   * @param {Object} obj
   * @param {String} key
   */

  function del(obj, key) {
    if (!(0, _util.hasOwn)(obj, key)) {
      return;
    }
    delete obj[key];
    var ob = obj.__ob__;

    if (!ob) {
      if (obj._isVue) {
        delete obj._data[key];
        obj.$forceUpdate();
      }
      return;
    }
    ob.dep.notify();
    if (ob.vms) {
      var i = ob.vms.length;
      while (i--) {
        var vm = ob.vms[i];
        unproxy(vm, key);
        vm.$forceUpdate();
      }
    }
  }

  var KEY_WORDS = ['$index', '$value', '$event'];
  function proxy(vm, key) {
    if (KEY_WORDS.indexOf(key) > -1 || !(0, _util.isReserved)(key)) {
      Object.defineProperty(vm, key, {
        configurable: true,
        enumerable: true,
        get: function proxyGetter() {
          return vm._data[key];
        },
        set: function proxySetter(val) {
          vm._data[key] = val;
        }
      });
    }
  }

  function unproxy(vm, key) {
    if (!(0, _util.isReserved)(key)) {
      delete vm[key];
    }
  }

/***/ },
/* 93 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  Object.defineProperty(exports, "__esModule", {
    value: true
  });
  exports.arrayMethods = undefined;

  var _util = __webpack_require__(84);

  var arrayProto = Array.prototype; /* eslint-disable */

  var arrayMethods = exports.arrayMethods = Object.create(arrayProto)

  /**
   * Intercept mutating methods and emit events
   */

  ;['push', 'pop', 'shift', 'unshift', 'splice', 'sort', 'reverse'].forEach(function (method) {
    // cache original method
    var original = arrayProto[method];
    (0, _util.def)(arrayMethods, method, function mutator() {
      // avoid leaking arguments:
      // http://jsperf.com/closure-with-arguments
      var i = arguments.length;
      var args = new Array(i);
      while (i--) {
        args[i] = arguments[i];
      }
      var result = original.apply(this, args);
      var ob = this.__ob__;
      var inserted = void 0;
      switch (method) {
        case 'push':
          inserted = args;
          break;
        case 'unshift':
          inserted = args;
          break;
        case 'splice':
          inserted = args.slice(2);
          break;
      }
      if (inserted) ob.observeArray(inserted);
      // notify change
      ob.dep.notify();
      return result;
    });
  });

  /**
   * Swap the element at the given index with a new value
   * and emits corresponding event.
   *
   * @param {Number} index
   * @param {*} val
   * @return {*} - replaced element
   */

  (0, _util.def)(arrayProto, '$set', function $set(index, val) {
    if (index >= this.length) {
      this.length = index + 1;
    }
    return this.splice(index, 1, val)[0];
  });

  /**
   * Convenience method to remove the element at given index.
   *
   * @param {Number} index
   * @param {*} val
   */

  (0, _util.def)(arrayProto, '$remove', function $remove(index) {
    /* istanbul ignore if */
    if (!this.length) return;
    if (typeof index !== 'number') {
      index = this.indexOf(index);
    }
    if (index > -1) {
      this.splice(index, 1);
    }
  });

/***/ },
/* 94 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  Object.defineProperty(exports, "__esModule", {
    value: true
  });

  var _typeof = typeof Symbol === "function" && typeof Symbol.iterator === "symbol" ? function (obj) { return typeof obj; } : function (obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol ? "symbol" : typeof obj; }; /**
                                                                                                                                                                                                                                                     * @fileOverview
                                                                                                                                                                                                                                                     * ViewModel template parser & data-binding process
                                                                                                                                                                                                                                                     */

  exports.build = build;

  var _util = __webpack_require__(84);

  var _state = __webpack_require__(89);

  var _directive = __webpack_require__(95);

  var _domHelper = __webpack_require__(96);

  /**
   * build()
   *   compile(template, parentNode)
   *     if (type is content) create contentNode
   *     else if (dirs have v-for) foreach -> create context
   *       -> compile(templateWithoutFor, parentNode): diff(list) onchange
   *     else if (dirs have v-if) assert
   *       -> compile(templateWithoutIf, parentNode): toggle(shown) onchange
   *     else if (type is dynamic)
   *       -> compile(templateWithoutDynamicType, parentNode): watch(type) onchange
   *     else if (type is custom)
   *       addChildVm(vm, parentVm)
   *       build(externalDirs)
   *       foreach childNodes -> compile(childNode, template)
   *     else if (type is native)
   *       set(dirs): update(id/attr/style/class) onchange
   *       append(template, parentNode)
   *       foreach childNodes -> compile(childNode, template)
   */
  function build(vm) {
    var opt = vm._options || {};
    var template = opt.template || {};

    if (opt.replace) {
      if (template.children && template.children.length === 1) {
        compile(vm, template.children[0], vm._parentEl);
      } else {
        compile(vm, template.children, vm._parentEl);
      }
    } else {
      compile(vm, template, vm._parentEl);
    }

    console.debug('[JS Framework] "ready" lifecycle in Vm(' + vm._type + ')');
    vm.$emit('hook:ready');
    vm._ready = true;
  }

  /**
   * Generate elements by child or children and append to parent elements.
   * Root element info would be merged if has. The first argument may be an array
   * if the root element with options.replace has not only one child.
   *
   * @param {object|array} target
   * @param {object}       dest
   * @param {object}       meta
   */
  function compile(vm, target, dest, meta) {
    var app = vm._app || {};

    if (app.lastSignal === -1) {
      return;
    }

    if (targetIsFragment(target)) {
      compileFragment(vm, target, dest, meta);
      return;
    }
    meta = meta || {};
    if (targetIsContent(target)) {
      console.debug('[JS Framework] compile "content" block by', target);
      vm._content = (0, _domHelper.createBlock)(vm, dest);
      return;
    }

    if (targetNeedCheckRepeat(target, meta)) {
      console.debug('[JS Framework] compile "repeat" logic by', target);
      compileRepeat(vm, target, dest);
      return;
    }
    if (targetNeedCheckShown(target, meta)) {
      console.debug('[JS Framework] compile "if" logic by', target);
      compileShown(vm, target, dest, meta);
      return;
    }
    var typeGetter = meta.type || target.type;
    if (targetNeedCheckType(typeGetter, meta)) {
      compileType(vm, target, dest, typeGetter, meta);
      return;
    }
    var type = typeGetter;
    var component = targetIsComposed(vm, target, type);
    if (component) {
      console.debug('[JS Framework] compile composed component by', target);
      compileCustomComponent(vm, component, target, dest, type, meta);
      return;
    }
    console.debug('[JS Framework] compile native component by', target);
    compileNativeComponent(vm, target, dest, type);
  }

  /**
   * Check if target is a fragment (an array).
   *
   * @param  {object}  target
   * @return {boolean}
   */
  function targetIsFragment(target) {
    return Array.isArray(target);
  }

  /**
   * Check if target type is content/slot.
   *
   * @param  {object}  target
   * @return {boolean}
   */
  function targetIsContent(target) {
    return target.type === 'content' || target.type === 'slot';
  }

  /**
   * Check if target need to compile by a list.
   *
   * @param  {object}  target
   * @param  {object}  meta
   * @return {boolean}
   */
  function targetNeedCheckRepeat(target, meta) {
    return !meta.hasOwnProperty('repeat') && target.repeat;
  }

  /**
   * Check if target need to compile by a boolean value.
   *
   * @param  {object}  target
   * @param  {object}  meta
   * @return {boolean}
   */
  function targetNeedCheckShown(target, meta) {
    return !meta.hasOwnProperty('shown') && target.shown;
  }

  /**
   * Check if target need to compile by a dynamic type.
   *
   * @param  {string|function} typeGetter
   * @param  {object}          meta
   * @return {boolean}
   */
  function targetNeedCheckType(typeGetter, meta) {
    return typeof typeGetter === 'function' && !meta.hasOwnProperty('type');
  }

  /**
   * Check if this kind of component is composed.
   *
   * @param  {string}  type
   * @return {boolean}
   */
  function targetIsComposed(vm, target, type) {
    var component = void 0;
    if (vm._app && vm._app.customComponentMap) {
      component = vm._app.customComponentMap[type];
    }
    if (vm._options && vm._options.components) {
      component = vm._options.components[type];
    }
    if (target.component) {
      component = component || {};
    }
    return component;
  }

  /**
   * Compile a list of targets.
   *
   * @param {object} target
   * @param {object} dest
   * @param {object} meta
   */
  function compileFragment(vm, target, dest, meta) {
    var fragBlock = (0, _domHelper.createBlock)(vm, dest);
    target.forEach(function (child) {
      compile(vm, child, fragBlock, meta);
    });
  }

  /**
   * Compile a target with repeat directive.
   *
   * @param {object} target
   * @param {object} dest
   */
  function compileRepeat(vm, target, dest) {
    var repeat = target.repeat;
    var oldStyle = typeof repeat === 'function';
    var getter = repeat.getter || repeat.expression || repeat;
    if (typeof getter !== 'function') {
      getter = function getter() {
        return [];
      };
    }
    var key = repeat.key || '$index';
    var value = repeat.value || '$value';
    var trackBy = repeat.trackBy || target.trackBy || target.attr && target.attr.trackBy;

    var fragBlock = (0, _domHelper.createBlock)(vm, dest);
    fragBlock.children = [];
    fragBlock.data = [];
    fragBlock.vms = [];

    bindRepeat(vm, target, fragBlock, { getter: getter, key: key, value: value, trackBy: trackBy, oldStyle: oldStyle });
  }

  /**
   * Compile a target with if directive.
   *
   * @param {object} target
   * @param {object} dest
   * @param {object} meta
   */
  function compileShown(vm, target, dest, meta) {
    var newMeta = { shown: true };
    var fragBlock = (0, _domHelper.createBlock)(vm, dest);

    if (dest.element && dest.children) {
      dest.children.push(fragBlock);
    }

    if (meta.repeat) {
      newMeta.repeat = meta.repeat;
    }

    bindShown(vm, target, fragBlock, newMeta);
  }

  /**
   * Compile a target with dynamic component type.
   *
   * @param {object}   target
   * @param {object}   dest
   * @param {function} typeGetter
   */
  function compileType(vm, target, dest, typeGetter, meta) {
    var type = typeGetter.call(vm);
    var newMeta = (0, _util.extend)({ type: type }, meta);
    var fragBlock = (0, _domHelper.createBlock)(vm, dest);

    if (dest.element && dest.children) {
      dest.children.push(fragBlock);
    }

    (0, _directive.watch)(vm, typeGetter, function (value) {
      var newMeta = (0, _util.extend)({ type: value }, meta);
      (0, _domHelper.removeTarget)(vm, fragBlock, true);
      compile(vm, target, fragBlock, newMeta);
    });

    compile(vm, target, fragBlock, newMeta);
  }

  /**
   * Compile a composed component.
   *
   * @param {object} target
   * @param {object} dest
   * @param {string} type
   */
  function compileCustomComponent(vm, component, target, dest, type, meta) {
    var Ctor = vm.constructor;
    var subVm = new Ctor(type, component, vm, dest, undefined, {
      'hook:init': function hookInit() {
        (0, _directive.setId)(vm, null, target.id, this);
        // bind template earlier because of lifecycle issues
        this._externalBinding = {
          parent: vm,
          template: target
        };
      },
      'hook:created': function hookCreated() {
        (0, _directive.bindSubVm)(vm, this, target, meta.repeat);
      },
      'hook:ready': function hookReady() {
        if (this._content) {
          compileChildren(vm, target, this._content);
        }
      }
    });
    (0, _directive.bindSubVmAfterInitialized)(vm, subVm, target);
  }

  /**
   * Generate element from template and attach to the dest if needed.
   * The time to attach depends on whether the mode status is node or tree.
   *
   * @param {object} template
   * @param {object} dest
   * @param {string} type
   */
  function compileNativeComponent(vm, template, dest, type) {
    (0, _directive.applyNaitveComponentOptions)(template);

    var element = void 0;
    if (dest.ref === '_documentElement') {
      // if its parent is documentElement then it's a body
      console.debug('[JS Framework] compile to create body for ' + type);
      element = (0, _domHelper.createBody)(vm, type);
    } else {
      console.debug('[JS Framework] compile to create element for ' + type);
      element = (0, _domHelper.createElement)(vm, type);
    }

    if (!vm._rootEl) {
      vm._rootEl = element;
      // bind event earlier because of lifecycle issues
      var binding = vm._externalBinding || {};
      var target = binding.template;
      var parentVm = binding.parent;
      if (target && target.events && parentVm && element) {
        for (var _type in target.events) {
          var handler = parentVm[target.events[_type]];
          if (handler) {
            element.addEvent(_type, (0, _util.bind)(handler, parentVm));
          }
        }
      }
    }

    (0, _directive.bindElement)(vm, element, template);

    if (template.attr && template.attr.append) {
      // backward, append prop in attr
      template.append = template.attr.append;
    }

    if (template.append) {
      // give the append attribute for ios adaptation
      element.attr = element.attr || {};
      element.attr.append = template.append;
    }

    var treeMode = template.append === 'tree';
    var app = vm._app || {};
    if (app.lastSignal !== -1 && !treeMode) {
      console.debug('[JS Framework] compile to append single node for', element);
      app.lastSignal = (0, _domHelper.attachTarget)(vm, element, dest);
    }
    if (app.lastSignal !== -1) {
      compileChildren(vm, template, element);
    }
    if (app.lastSignal !== -1 && treeMode) {
      console.debug('[JS Framework] compile to append whole tree for', element);
      app.lastSignal = (0, _domHelper.attachTarget)(vm, element, dest);
    }
  }

  /**
   * Set all children to a certain parent element.
   *
   * @param {object} template
   * @param {object} dest
   */
  function compileChildren(vm, template, dest) {
    var app = vm._app || {};
    var children = template.children;
    if (children && children.length) {
      children.every(function (child) {
        compile(vm, child, dest);
        return app.lastSignal !== -1;
      });
    }
  }

  /**
   * Watch the list update and refresh the changes.
   *
   * @param {object} target
   * @param {object} fragBlock {vms, data, children}
   * @param {object} info      {getter, key, value, trackBy, oldStyle}
   */
  function bindRepeat(vm, target, fragBlock, info) {
    var vms = fragBlock.vms;
    var children = fragBlock.children;
    var getter = info.getter;
    var trackBy = info.trackBy;
    var oldStyle = info.oldStyle;

    var keyName = info.key;
    var valueName = info.value;

    function compileItem(item, index, context) {
      var mergedData = void 0;
      if (oldStyle) {
        mergedData = item;
        if ((typeof item === 'undefined' ? 'undefined' : _typeof(item)) === 'object') {
          mergedData[keyName] = index;
          if (!mergedData.hasOwnProperty('INDEX')) {
            Object.defineProperty(mergedData, 'INDEX', {
              value: function value() {
                console.warn('[JS Framework] "INDEX" in repeat is deprecated, ' + 'please use "$index" instead');
              }
            });
          }
        }
      } else {
        mergedData = {};
        mergedData[keyName] = index;
        mergedData[valueName] = item;
      }
      var newContext = mergeContext(context, mergedData);
      vms.push(newContext);
      compile(newContext, target, fragBlock, { repeat: item });
    }

    var list = watchBlock(vm, fragBlock, getter, 'repeat', function (data) {
      console.debug('[JS Framework] the "repeat" item has changed', data);
      if (!fragBlock) {
        return;
      }

      var oldChildren = children.slice();
      var oldVms = vms.slice();
      var oldData = fragBlock.data.slice();
      // 1. collect all new refs track by
      var trackMap = {};
      var reusedMap = {};
      data.forEach(function (item, index) {
        var key = trackBy ? item[trackBy] : oldStyle ? item[keyName] : index;
        /* istanbul ignore if */
        if (key == null || key === '') {
          return;
        }
        trackMap[key] = item;
      });

      // 2. remove unused element foreach old item
      var reusedList = [];
      oldData.forEach(function (item, index) {
        var key = trackBy ? item[trackBy] : oldStyle ? item[keyName] : index;
        if (trackMap.hasOwnProperty(key)) {
          reusedMap[key] = {
            item: item, index: index, key: key,
            target: oldChildren[index],
            vm: oldVms[index]
          };
          reusedList.push(item);
        } else {
          (0, _domHelper.removeTarget)(vm, oldChildren[index]);
        }
      });

      // 3. create new element foreach new item
      children.length = 0;
      vms.length = 0;
      fragBlock.data = data.slice();
      fragBlock.updateMark = fragBlock.start;

      data.forEach(function (item, index) {
        var key = trackBy ? item[trackBy] : oldStyle ? item[keyName] : index;
        var reused = reusedMap[key];
        if (reused) {
          if (reused.item === reusedList[0]) {
            reusedList.shift();
          } else {
            reusedList.$remove(reused.item);
            (0, _domHelper.moveTarget)(vm, reused.target, fragBlock.updateMark, true);
          }
          children.push(reused.target);
          vms.push(reused.vm);
          if (oldStyle) {
            reused.vm = item;
          } else {
            reused.vm[valueName] = item;
          }
          reused.vm[keyName] = index;
          fragBlock.updateMark = reused.target;
        } else {
          compileItem(item, index, vm);
        }
      });

      delete fragBlock.updateMark;
    });

    fragBlock.data = list.slice(0);
    list.forEach(function (item, index) {
      compileItem(item, index, vm);
    });
  }

  /**
   * Watch the display update and add/remove the element.
   *
   * @param  {object} target
   * @param  {object} fragBlock
   * @param  {object} context
   */
  function bindShown(vm, target, fragBlock, meta) {
    var display = watchBlock(vm, fragBlock, target.shown, 'shown', function (display) {
      console.debug('[JS Framework] the "if" item was changed', display);

      if (!fragBlock || !!fragBlock.display === !!display) {
        return;
      }
      fragBlock.display = !!display;
      if (display) {
        compile(vm, target, fragBlock, meta);
      } else {
        (0, _domHelper.removeTarget)(vm, fragBlock, true);
      }
    });

    fragBlock.display = !!display;
    if (display) {
      compile(vm, target, fragBlock, meta);
    }
  }

  /**
   * Watch calc value changes and append certain type action to differ.
   * It is used for if or repeat data-binding generator.
   *
   * @param  {object}   fragBlock
   * @param  {function} calc
   * @param  {string}   type
   * @param  {function} handler
   * @return {any}      init value of calc
   */
  function watchBlock(vm, fragBlock, calc, type, handler) {
    var differ = vm && vm._app && vm._app.differ;
    var config = {};
    var depth = (fragBlock.element.depth || 0) + 1;

    return (0, _directive.watch)(vm, calc, function (value) {
      config.latestValue = value;
      if (differ && !config.recorded) {
        differ.append(type, depth, fragBlock.blockId, function () {
          var latestValue = config.latestValue;
          handler(latestValue);
          config.recorded = false;
          config.latestValue = undefined;
        });
      }
      config.recorded = true;
    });
  }

  /**
   * Clone a context and merge certain data.
   *
   * @param  {object} mergedData
   * @return {object}
   */
  function mergeContext(context, mergedData) {
    var newContext = Object.create(context);
    newContext._data = mergedData;
    (0, _state.initData)(newContext);
    (0, _state.initComputed)(newContext);
    newContext._realParent = context;
    return newContext;
  }

/***/ },
/* 95 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  Object.defineProperty(exports, "__esModule", {
    value: true
  });

  var _typeof = typeof Symbol === "function" && typeof Symbol.iterator === "symbol" ? function (obj) { return typeof obj; } : function (obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol ? "symbol" : typeof obj; }; /**
                                                                                                                                                                                                                                                     * @fileOverview
                                                                                                                                                                                                                                                     * Directive Parser
                                                                                                                                                                                                                                                     */

  exports.applyNaitveComponentOptions = applyNaitveComponentOptions;
  exports.bindElement = bindElement;
  exports.bindSubVm = bindSubVm;
  exports.bindSubVmAfterInitialized = bindSubVmAfterInitialized;
  exports.setId = setId;
  exports.watch = watch;

  var _util = __webpack_require__(84);

  var _watcher = __webpack_require__(90);

  var _watcher2 = _interopRequireDefault(_watcher);

  var _config = __webpack_require__(79);

  var _config2 = _interopRequireDefault(_config);

  function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

  var nativeComponentMap = _config2.default.nativeComponentMap;


  var SETTERS = {
    attr: 'setAttr',
    style: 'setStyle',
    event: 'addEvent'
  };

  /**
   * apply the native component's options(specified by template.type)
   * to the template
   */
  function applyNaitveComponentOptions(template) {
    var type = template.type;

    var options = nativeComponentMap[type];

    if ((typeof options === 'undefined' ? 'undefined' : _typeof(options)) === 'object') {
      for (var key in options) {
        if (template[key] == null) {
          template[key] = options[key];
        } else if ((0, _util.typof)(template[key]) === 'object' && (0, _util.typof)(options[key]) === 'object') {
          for (var subkey in options[key]) {
            if (template[key][subkey] == null) {
              template[key][subkey] = options[key][subkey];
            }
          }
        }
      }
    }
  }

  /**
   * bind all id, attr, classnames, style, events to an element
   */
  function bindElement(vm, el, template) {
    setId(vm, el, template.id, vm);
    setAttr(vm, el, template.attr);
    setClass(vm, el, template.classList);
    setStyle(vm, el, template.style);
    bindEvents(vm, el, template.events);
  }

  /**
   * bind all props to sub vm and bind all style, events to the root element
   * of the sub vm if it doesn't have a replaced multi-node fragment
   */
  function bindSubVm(vm, subVm, template, repeatItem) {
    subVm = subVm || {};
    template = template || {};

    var options = subVm._options || {};

    // bind props
    var props = options.props;

    if (Array.isArray(props)) {
      props = props.reduce(function (result, value) {
        result[value] = true;
        return result;
      }, {});
    }

    mergeProps(repeatItem, props, vm, subVm);
    mergeProps(template.attr, props, vm, subVm);
  }

  /**
   * merge class and styles from vm to sub vm.
   */
  function bindSubVmAfterInitialized(vm, subVm, template) {
    mergeClassStyle(template.classList, vm, subVm);
    mergeStyle(template.style, vm, subVm);
  }

  /**
   * Bind props from vm to sub vm and watch their updates.
   */
  function mergeProps(target, props, vm, subVm) {
    if (!target) {
      return;
    }

    var _loop = function _loop(key) {
      if (!props || props[key]) {
        var value = target[key];
        if (typeof value === 'function') {
          var returnValue = watch(vm, value, function (v) {
            subVm[key] = v;
          });
          subVm[key] = returnValue;
        } else {
          subVm[key] = value;
        }
      }
    };

    for (var key in target) {
      _loop(key);
    }
  }

  /**
   * Bind style from vm to sub vm and watch their updates.
   */
  function mergeStyle(target, vm, subVm) {
    var _loop2 = function _loop2(key) {
      var value = target[key];
      if (typeof value === 'function') {
        var returnValue = watch(vm, value, function (v) {
          if (subVm._rootEl) {
            subVm._rootEl.setStyle(key, v);
          }
        });
        subVm._rootEl.setStyle(key, returnValue);
      } else {
        if (subVm._rootEl) {
          subVm._rootEl.setStyle(key, value);
        }
      }
    };

    for (var key in target) {
      _loop2(key);
    }
  }

  /**
   * Bind class & style from vm to sub vm and watch their updates.
   */
  function mergeClassStyle(target, vm, subVm) {
    var css = vm._options && vm._options.style || {};

    /* istanbul ignore if */
    if (!subVm._rootEl) {
      return;
    }

    if (typeof target === 'function') {
      var _value = watch(vm, target, function (v) {
        setClassStyle(subVm._rootEl, css, v);
      });
      setClassStyle(subVm._rootEl, css, _value);
    } else if (target != null) {
      setClassStyle(subVm._rootEl, css, target);
    }
  }

  /**
   * bind id to an element
   * each id is unique in a whole vm
   */
  function setId(vm, el, id, target) {
    var map = Object.create(null);

    Object.defineProperties(map, {
      vm: {
        value: target,
        writable: false,
        configurable: false
      },
      el: {
        get: function get() {
          return el || target._rootEl;
        },
        configurable: false
      }
    });

    if (typeof id === 'function') {
      var handler = id;
      id = handler.call(vm);
      if (id) {
        vm._ids[id] = map;
      }
      watch(vm, handler, function (newId) {
        if (newId) {
          vm._ids[newId] = map;
        }
      });
    } else if (id && typeof id === 'string') {
      vm._ids[id] = map;
    }
  }

  /**
   * bind attr to an element
   */
  function setAttr(vm, el, attr) {
    bindDir(vm, el, 'attr', attr);
  }

  function setClassStyle(el, css, classList) {
    var classStyle = {};
    var length = classList.length;

    for (var i = 0; i < length; i++) {
      var style = css[classList[i]];
      if (style) {
        for (var key in style) {
          classStyle[key] = style[key];
        }
      }
    }
    el.setClassStyle(classStyle);
  }

  /**
   * bind classnames to an element
   */
  function setClass(vm, el, classList) {
    if (typeof classList !== 'function' && !Array.isArray(classList)) {
      return;
    }
    if (Array.isArray(classList) && !classList.length) {
      el.setClassStyle({});
      return;
    }

    var style = vm._options && vm._options.style || {};
    if (typeof classList === 'function') {
      var _value2 = watch(vm, classList, function (v) {
        setClassStyle(el, style, v);
      });
      setClassStyle(el, style, _value2);
    } else {
      setClassStyle(el, style, classList);
    }
  }

  /**
   * bind style to an element
   */
  function setStyle(vm, el, style) {
    bindDir(vm, el, 'style', style);
  }

  /**
   * add an event type and handler to an element and generate a dom update
   */
  function setEvent(vm, el, type, handler) {
    el.addEvent(type, (0, _util.bind)(handler, vm));
  }

  /**
   * add all events of an element
   */
  function bindEvents(vm, el, events) {
    if (!events) {
      return;
    }
    var keys = Object.keys(events);
    var i = keys.length;
    while (i--) {
      var key = keys[i];
      var handler = events[key];
      if (typeof handler === 'string') {
        handler = vm[handler];
        /* istanbul ignore if */
        if (!handler) {
          console.debug('[JS Framework] The method "' + handler + '" is not defined.');
        }
      }
      setEvent(vm, el, key, handler);
    }
  }

  /**
   * set a series of members as a kind of an element
   * for example: style, attr, ...
   * if the value is a function then bind the data changes
   */
  function bindDir(vm, el, name, data) {
    if (!data) {
      return;
    }
    var keys = Object.keys(data);
    var i = keys.length;
    while (i--) {
      var key = keys[i];
      var _value3 = data[key];
      if (typeof _value3 === 'function') {
        bindKey(vm, el, name, key, _value3);
      } else {
        el[SETTERS[name]](key, _value3);
      }
    }
  }

  /**
   * bind data changes to a certain key to a name series in an element
   */
  function bindKey(vm, el, name, key, calc) {
    var methodName = SETTERS[name];
    // watch the calc, and returns a value by calc.call()
    var value = watch(vm, calc, function (value) {
      function handler() {
        el[methodName](key, value);
      }
      var differ = vm && vm._app && vm._app.differ;
      if (differ) {
        differ.append('element', el.depth, el.ref, handler);
      } else {
        handler();
      }
    });

    el[methodName](key, value);
  }

  /**
   * watch a calc function and callback if the calc value changes
   */
  function watch(vm, calc, callback) {
    var watcher = new _watcher2.default(vm, calc, function (value, oldValue) {
      /* istanbul ignore if */
      if ((typeof value === 'undefined' ? 'undefined' : _typeof(value)) !== 'object' && value === oldValue) {
        return;
      }
      callback(value);
    });

    return watcher.value;
  }

/***/ },
/* 96 */
/***/ function(module, exports) {

  'use strict';

  Object.defineProperty(exports, "__esModule", {
    value: true
  });

  var _typeof = typeof Symbol === "function" && typeof Symbol.iterator === "symbol" ? function (obj) { return typeof obj; } : function (obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol ? "symbol" : typeof obj; };

  exports.createBody = createBody;
  exports.createElement = createElement;
  exports.createBlock = createBlock;
  exports.attachTarget = attachTarget;
  exports.moveTarget = moveTarget;
  exports.removeTarget = removeTarget;
  /**
   * @fileOverview Document & Element Helpers.
   *
   * required:
   * Document#: createElement, createComment, getRef
   * Element#: appendChild, insertBefore, removeChild, nextSibling
   */

  /**
   * Create a body by type
   * Using this._app.doc
   *
   * @param  {string} type
   */
  function createBody(vm, type) {
    var doc = vm._app.doc;
    return doc.createBody(type);
  }

  /**
   * Create an element by type
   * Using this._app.doc
   *
   * @param  {string} type
   */
  function createElement(vm, type) {
    var doc = vm._app.doc;
    return doc.createElement(type);
  }

  /**
   * Create and return a frag block for an element.
   * The frag block has a starter, ender and the element itself.
   *
   * @param  {object} element
   */
  function createBlock(vm, element) {
    var start = createBlockStart(vm);
    var end = createBlockEnd(vm);
    var blockId = lastestBlockId++;
    if (element.element) {
      var updateMark = element.updateMark;
      if (updateMark) {
        if (updateMark.element) {
          updateMark = updateMark.end;
        }
        element.element.insertAfter(end, updateMark);
        element.element.insertAfter(start, updateMark);
        element.updateMark = end;
      } else {
        element.element.insertBefore(start, element.end);
        element.element.insertBefore(end, element.end);
      }
      element = element.element;
    } else {
      element.appendChild(start);
      element.appendChild(end);
    }
    return { start: start, end: end, element: element, blockId: blockId };
  }

  var lastestBlockId = 1;

  /**
   * Create and return a block starter.
   * Using this._app.doc
   */
  function createBlockStart(vm) {
    var doc = vm._app.doc;
    var anchor = doc.createComment('start');
    return anchor;
  }

  /**
   * Create and return a block ender.
   * Using this._app.doc
   */
  function createBlockEnd(vm) {
    var doc = vm._app.doc;
    var anchor = doc.createComment('end');
    return anchor;
  }

  /**
   * Attach target to a certain dest using appendChild by default.
   * If the dest is a frag block then insert before the ender.
   * If the target is a frag block then attach the starter and ender in order.
   *
   * @param  {object} target
   * @param  {object} dest
   */
  function attachTarget(vm, target, dest) {
    if (dest.element) {
      var before = dest.end;
      var after = dest.updateMark;
      // push new target for watch list update later
      if (dest.children) {
        dest.children.push(target);
      }
      // for check repeat case
      if (after) {
        var signal = moveTarget(vm, target, after);
        dest.updateMark = target.element ? target.end : target;
        return signal;
      } else if (target.element) {
        dest.element.insertBefore(target.start, before);
        dest.element.insertBefore(target.end, before);
      } else {
        return dest.element.insertBefore(target, before);
      }
    } else {
      if (target.element) {
        dest.appendChild(target.start);
        dest.appendChild(target.end);
      } else {
        return dest.appendChild(target);
      }
    }
  }

  /**
   * Move target before a certain element. The target maybe block or element.
   *
   * @param  {object} target
   * @param  {object} before
   */
  function moveTarget(vm, target, after) {
    if (target.element) {
      return moveBlock(target, after);
    }
    return moveElement(target, after);
  }

  /**
   * Move element before a certain element.
   *
   * @param  {object} element
   * @param  {object} before
   */
  function moveElement(element, after) {
    var parent = after.parentNode;
    if (parent) {
      return parent.insertAfter(element, after);
    }
  }

  /**
   * Move all elements of the block before a certain element.
   *
   * @param  {object} fragBlock
   * @param  {object} before
   */
  function moveBlock(fragBlock, after) {
    var parent = after.parentNode;

    if (parent) {
      var _ret = function () {
        var el = fragBlock.start;
        var signal = void 0;
        var group = [el];

        while (el && el !== fragBlock.end) {
          el = el.nextSibling;
          group.push(el);
        }

        var temp = after;
        group.every(function (el) {
          signal = parent.insertAfter(el, temp);
          temp = el;
          return signal !== -1;
        });

        return {
          v: signal
        };
      }();

      if ((typeof _ret === 'undefined' ? 'undefined' : _typeof(_ret)) === "object") return _ret.v;
    }
  }

  /**
   * Remove target from DOM tree.
   * If the target is a frag block then call _removeBlock
   *
   * @param  {object} target
   */
  function removeTarget(vm, target) {
    var preserveBlock = arguments.length <= 2 || arguments[2] === undefined ? false : arguments[2];

    if (target.element) {
      removeBlock(target, preserveBlock);
    } else {
      removeElement(target);
    }
  }

  /**
   * Remove a certain element.
   * Using this._app.doc
   *
   * @param  {object} target
   */
  function removeElement(target) {
    var parent = target.parentNode;

    if (parent) {
      parent.removeChild(target);
    }
  }

  /**
   * Remove a frag block.
   * The second param decides whether the block self should be removed too.
   *
   * @param  {object}  fragBlock
   * @param  {Boolean} preserveBlock=false
   */
  function removeBlock(fragBlock) {
    var preserveBlock = arguments.length <= 1 || arguments[1] === undefined ? false : arguments[1];

    var result = [];
    var el = fragBlock.start.nextSibling;

    while (el && el !== fragBlock.end) {
      result.push(el);
      el = el.nextSibling;
    }

    if (!preserveBlock) {
      removeElement(fragBlock.start);
    }
    result.forEach(function (el) {
      removeElement(el);
    });
    if (!preserveBlock) {
      removeElement(fragBlock.end);
    }
  }

/***/ },
/* 97 */
/***/ function(module, exports) {

  'use strict';

  Object.defineProperty(exports, "__esModule", {
    value: true
  });
  exports.$emit = $emit;
  exports.$dispatch = $dispatch;
  exports.$broadcast = $broadcast;
  exports.$on = $on;
  exports.$off = $off;
  exports.initEvents = initEvents;
  exports.mixinEvents = mixinEvents;
  /**
   * @fileOverview
   * Everything about component event which includes event object, event listener,
   * event emitter and lifecycle hooks.
   */

  /**
   * Event object definition. An event object has `type`, `timestamp` and
   * `detail` from which a component emit. The event object could be dispatched to
   * parents or broadcasted to children except `this.stop()` is called.
   * @param {string} type
   * @param {any}    detail
   */
  function Evt(type, detail) {
    if (detail instanceof Evt) {
      return detail;
    }

    this.timestamp = Date.now();
    this.detail = detail;
    this.type = type;

    var shouldStop = false;

    /**
     * stop dispatch and broadcast
     */
    this.stop = function () {
      shouldStop = true;
    };

    /**
     * check if it can't be dispatched or broadcasted
     */
    this.hasStopped = function () {
      return shouldStop;
    };
  }

  /**
   * Emit an event but not broadcast down or dispatch up.
   * @param  {string} type
   * @param  {any}    detail
   */
  function $emit(type, detail) {
    var _this = this;

    var events = this._vmEvents;
    var handlerList = events[type];
    if (handlerList) {
      (function () {
        var evt = new Evt(type, detail);
        handlerList.forEach(function (handler) {
          handler.call(_this, evt);
        });
      })();
    }
  }

  /**
   * Emit an event and dispatch it up.
   * @param  {string} type
   * @param  {any}    detail
   */
  function $dispatch(type, detail) {
    var evt = new Evt(type, detail);
    this.$emit(type, evt);

    if (!evt.hasStopped() && this._parent && this._parent.$dispatch) {
      this._parent.$dispatch(type, evt);
    }
  }

  /**
   * Emit an event and broadcast it down.
   * @param  {string} type
   * @param  {any}    detail
   */
  function $broadcast(type, detail) {
    var evt = new Evt(type, detail);
    this.$emit(type, evt);

    if (!evt.hasStopped() && this._childrenVms) {
      this._childrenVms.forEach(function (subVm) {
        subVm.$broadcast(type, evt);
      });
    }
  }

  /**
   * Add event listener.
   * @param  {string}   type
   * @param  {function} handler
   */
  function $on(type, handler) {
    if (!type || typeof handler !== 'function') {
      return;
    }
    var events = this._vmEvents;
    var handlerList = events[type] || [];
    handlerList.push(handler);
    events[type] = handlerList;

    // fixed old version lifecycle design
    if (type === 'hook:ready' && this._ready) {
      this.$emit('hook:ready');
    }
  }

  /**
   * Remove event listener.
   * @param  {string}   type
   * @param  {function} handler
   */
  function $off(type, handler) {
    if (!type) {
      return;
    }
    var events = this._vmEvents;
    if (!handler) {
      delete events[type];
      return;
    }
    var handlerList = events[type];
    if (!handlerList) {
      return;
    }
    handlerList.$remove(handler);
  }

  var LIFE_CYCLE_TYPES = ['init', 'created', 'ready'];

  /**
   * Init events:
   * 1. listen `events` in component options & `externalEvents`.
   * 2. bind lifecycle hooks.
   * @param  {Vm}     vm
   * @param  {object} externalEvents
   */
  function initEvents(vm, externalEvents) {
    var options = vm._options || {};
    var events = options.events || {};
    for (var type1 in events) {
      vm.$on(type1, events[type1]);
    }
    for (var type2 in externalEvents) {
      vm.$on(type2, externalEvents[type2]);
    }
    LIFE_CYCLE_TYPES.forEach(function (type) {
      vm.$on('hook:' + type, options[type]);
    });
  }

  /**
   * Bind event related methods to ViewModel instance.
   * @param  {Vm} vm
   */
  function mixinEvents(vm) {
    vm.$emit = $emit;
    vm.$dispatch = $dispatch;
    vm.$broadcast = $broadcast;
    vm.$on = $on;
    vm.$off = $off;
  }

/***/ },
/* 98 */
/***/ function(module, exports, __webpack_require__) {

  /* WEBPACK VAR INJECTION */(function(global) {'use strict';

  Object.defineProperty(exports, "__esModule", {
    value: true
  });
  exports.normalizeVersion = normalizeVersion;
  exports.getError = getError;
  exports.check = check;

  var _semver = __webpack_require__(86);

  var _semver2 = _interopRequireDefault(_semver);

  var _util = __webpack_require__(84);

  function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

  /**
   * Normalize a version string.
   * @param  {String} Version. ie: 1, 1.0, 1.0.0
   * @return {String} Version
   */
  function normalizeVersion(v) {
    var isValid = _semver2.default.valid(v);
    if (isValid) {
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
  }

  /**
   * Get informations from different error key. Like:
   * - code
   * - errorMessage
   * - errorType
   * - isDowngrade
   * @param  {string} key
   * @param  {string} val
   * @param  {string} criteria
   * @return {object}
   */
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

    result.errorMessage = getMsg(key, val, criteria);

    if (_key.indexOf('osversion') >= 0) {
      result.code = 1001;
    } else if (_key.indexOf('appversion') >= 0) {
      result.code = 1002;
    } else if (_key.indexOf('weexversion') >= 0) {
      result.code = 1003;
    } else if (_key.indexOf('devicemodel') >= 0) {
      result.code = 1004;
    }

    return result;
  }

  /**
   * WEEX framework input(deviceInfo)
   * {
   *   platform: 'iOS' or 'android'
   *   osVersion: '1.0.0' or '1.0' or '1'
   *   appVersion: '1.0.0' or '1.0' or '1'
   *   weexVersion: '1.0.0' or '1.0' or '1'
   *   dDeviceModel: 'MODEL_NAME'
   * }
   *
   * downgrade config(config)
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
   *
   * @param  {object} deviceInfo Weex SDK framework input
   * @param  {object} config     user input
   * @return {Object}            { isDowngrade: true/false, errorMessage... }
   */
  function check(config, deviceInfo) {
    deviceInfo = deviceInfo || global.WXEnvironment;
    deviceInfo = (0, _util.isPlainObject)(deviceInfo) ? deviceInfo : {};

    var result = {
      isDowngrade: false // defautl is pass
    };

    if ((0, _util.typof)(config) === 'function') {
      var customDowngrade = config.call(this, deviceInfo, {
        semver: _semver2.default,
        normalizeVersion: this.normalizeVersion
      });

      customDowngrade = !!customDowngrade;

      result = customDowngrade ? this.getError('custom', '', 'custom params') : result;
    } else {
      config = (0, _util.isPlainObject)(config) ? config : {};

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
          var c = this.normalizeVersion(criteria);
          var d = this.normalizeVersion(deviceInfo[i]);

          if (_semver2.default.satisfies(d, c)) {
            result = this.getError(key, val, criteria);
            break;
          }
        } else if (isDeviceModel) {
          var _criteria = (0, _util.typof)(criteria) === 'array' ? criteria : [criteria];
          if (_criteria.indexOf(val) >= 0) {
            result = this.getError(key, val, criteria);
            break;
          }
        }
      }
    }

    return result;
  }
  /* WEBPACK VAR INJECTION */}.call(exports, (function() { return this; }())))

/***/ },
/* 99 */
/***/ function(module, exports, __webpack_require__) {

  'use strict';

  Object.defineProperty(exports, "__esModule", {
    value: true
  });
  exports.instanceMap = undefined;
  exports.Document = Document;
  exports.Node = Node;
  exports.Element = Element;
  exports.Comment = Comment;

  var _listener4 = __webpack_require__(100);

  var _listener5 = _interopRequireDefault(_listener4);

  var _shared = __webpack_require__(3);

  function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

  /**
   * @fileOverview
   * A simple virtual dom implementation
   */

  var DEFAULT_TAG_NAME = 'div';

  var instanceMap = exports.instanceMap = {};
  var nextNodeRef = 1;

  function Document(id, url, handler) {
    id = id ? id.toString() : '';
    this.id = id;
    this.URL = url;

    instanceMap[id] = this;
    this.nodeMap = {};
    this.listener = new _listener5.default(id, handler || genCallTasks(id));
    this.createDocumentElement();
  }

  function genCallTasks(id) {
    return function (tasks) {
      if (!Array.isArray(tasks)) {
        tasks = [tasks];
      }
      return callNative(id, tasks, '-1');
    };
  }

  Document.prototype.destroy = function () {
    delete this.listener;
    delete this.nodeMap;
    delete instanceMap[this.id];
  };

  Document.prototype.open = function () {
    this.listener.batched = false;
  };

  Document.prototype.close = function () {
    this.listener.batched = true;
  };

  Document.prototype.createDocumentElement = function () {
    var _this = this;

    if (!this.documentElement) {
      var el = new Element('document');
      el.docId = this.id;
      el.ownerDocument = this;
      el.role = 'documentElement';
      el.depth = 0;
      el.ref = '_documentElement';
      this.nodeMap._documentElement = el;
      this.documentElement = el;
      el.appendChild = function (node) {
        appendBody(_this, node);
      };
      el.insertBefore = function (node, before) {
        appendBody(_this, node, before);
      };
    }

    return this.documentElement;
  };

  function appendBody(doc, node, before) {
    var documentElement = doc.documentElement;


    if (documentElement.pureChildren.length > 0 || node.parentNode) {
      return;
    }
    var children = documentElement.children;
    var beforeIndex = children.indexOf(before);
    if (beforeIndex < 0) {
      children.push(node);
    } else {
      children.splice(beforeIndex, 0, node);
    }

    if (node.nodeType === 1) {
      if (node.role === 'body') {
        node.docId = doc.id;
        node.ownerDocument = doc;
        node.parentNode = documentElement;
      } else {
        node.children.forEach(function (child) {
          child.parentNode = node;
        });
        setBody(doc, node);
        node.docId = doc.id;
        node.ownerDocument = doc;
        linkParent(node, documentElement);
        delete doc.nodeMap[node.nodeId];
      }
      documentElement.pureChildren.push(node);
      doc.listener.createBody(node);
    } else {
      node.parentNode = documentElement;
      doc.nodeMap[node.ref] = node;
    }
  }

  function setBody(doc, el) {
    el.role = 'body';
    el.depth = 1;
    delete doc.nodeMap[el.nodeId];
    el.ref = '_root';
    doc.nodeMap._root = el;
    doc.body = el;
  }

  Document.prototype.createBody = function (type, props) {
    if (!this.body) {
      var el = new Element(type, props);
      setBody(this, el);
    }

    return this.body;
  };

  Document.prototype.createElement = function (tagName, props) {
    return new Element(tagName, props);
  };

  Document.prototype.createComment = function (text) {
    return new Comment(text);
  };

  Document.prototype.fireEvent = function (el, type, e, domChanges) {
    if (!el) {
      return;
    }
    e = e || {};
    e.type = type;
    e.target = el;
    e.timestamp = Date.now();
    if (domChanges) {
      updateElement(el, domChanges);
    }
    return el.fireEvent(type, e);
  };

  Document.prototype.getRef = function (ref) {
    return this.nodeMap[ref];
  };

  function updateElement(el, changes) {
    var attrs = changes.attrs || {};
    for (var name in attrs) {
      el.setAttr(name, attrs[name], true);
    }
    var style = changes.style || {};
    for (var _name in style) {
      el.setStyle(_name, style[_name], true);
    }
  }

  function Node() {
    this.nodeId = (nextNodeRef++).toString();
    this.ref = this.nodeId;
    this.children = [];
    this.pureChildren = [];
    this.parentNode = null;
    this.nextSibling = null;
    this.previousSibling = null;
  }

  Node.prototype.destroy = function () {
    var doc = instanceMap[this.docId];
    if (doc) {
      delete this.docId;
      delete doc.nodeMap[this.nodeId];
    }
    this.children.forEach(function (child) {
      child.destroy();
    });
  };

  function Element() {
    var type = arguments.length <= 0 || arguments[0] === undefined ? DEFAULT_TAG_NAME : arguments[0];
    var props = arguments[1];

    props = props || {};
    this.nodeType = 1;
    this.nodeId = (nextNodeRef++).toString();
    this.ref = this.nodeId;
    this.type = type;
    this.attr = props.attr || {};
    this.classStyle = props.classStyle || {};
    this.style = props.style || {};
    this.event = {};
    this.children = [];
    this.pureChildren = [];
  }

  Element.prototype = new Node();

  Element.prototype.appendChild = function (node) {
    if (node.parentNode && node.parentNode !== this) {
      return;
    }
    if (!node.parentNode) {
      linkParent(node, this);
      insertIndex(node, this.children, this.children.length, true);
      if (this.docId) {
        registerNode(this.docId, node);
      }
      if (node.nodeType === 1) {
        insertIndex(node, this.pureChildren, this.pureChildren.length);
        if (this.docId) {
          var listener = instanceMap[this.docId].listener;
          return listener.addElement(node, this.ref, -1);
        }
      }
    } else {
      moveIndex(node, this.children, this.children.length, true);
      if (node.nodeType === 1) {
        var index = moveIndex(node, this.pureChildren, this.pureChildren.length);
        if (this.docId && index >= 0) {
          var _listener = instanceMap[this.docId].listener;
          return _listener.moveElement(node.ref, this.ref, index);
        }
      }
    }
  };

  Element.prototype.insertBefore = function (node, before) {
    if (node.parentNode && node.parentNode !== this) {
      return;
    }
    if (node === before || node.nextSibling === before) {
      return;
    }
    if (!node.parentNode) {
      linkParent(node, this);
      insertIndex(node, this.children, this.children.indexOf(before), true);
      if (this.docId) {
        registerNode(this.docId, node);
      }
      if (node.nodeType === 1) {
        var pureBefore = nextElement(before);
        var index = insertIndex(node, this.pureChildren, pureBefore ? this.pureChildren.indexOf(pureBefore) : this.pureChildren.length);
        if (this.docId) {
          var listener = instanceMap[this.docId].listener;
          return listener.addElement(node, this.ref, index);
        }
      }
    } else {
      moveIndex(node, this.children, this.children.indexOf(before), true);
      if (node.nodeType === 1) {
        var _pureBefore = nextElement(before);
        var _index = moveIndex(node, this.pureChildren, _pureBefore ? this.pureChildren.indexOf(_pureBefore) : this.pureChildren.length);
        if (this.docId && _index >= 0) {
          var _listener2 = instanceMap[this.docId].listener;
          return _listener2.moveElement(node.ref, this.ref, _index);
        }
      }
    }
  };

  Element.prototype.insertAfter = function (node, after) {
    if (node.parentNode && node.parentNode !== this) {
      return;
    }
    if (node === after || node.previousSibling === after) {
      return;
    }
    if (!node.parentNode) {
      linkParent(node, this);
      insertIndex(node, this.children, this.children.indexOf(after) + 1, true);
      if (this.docId) {
        registerNode(this.docId, node);
      }
      if (node.nodeType === 1) {
        var index = insertIndex(node, this.pureChildren, this.pureChildren.indexOf(previousElement(after)) + 1);
        if (this.docId) {
          var listener = instanceMap[this.docId].listener;
          return listener.addElement(node, this.ref, index);
        }
      }
    } else {
      moveIndex(node, this.children, this.children.indexOf(after) + 1, true);
      if (node.nodeType === 1) {
        var _index2 = moveIndex(node, this.pureChildren, this.pureChildren.indexOf(previousElement(after)) + 1);
        if (this.docId && _index2 >= 0) {
          var _listener3 = instanceMap[this.docId].listener;
          return _listener3.moveElement(node.ref, this.ref, _index2);
        }
      }
    }
  };

  Element.prototype.removeChild = function (node, preserved) {
    if (node.parentNode) {
      removeIndex(node, this.children, true);
      if (node.nodeType === 1) {
        removeIndex(node, this.pureChildren);
        if (this.docId) {
          var listener = instanceMap[this.docId].listener;
          listener.removeElement(node.ref);
        }
      }
    }
    if (!preserved) {
      node.destroy();
    }
  };

  Element.prototype.clear = function () {
    var _this2 = this;

    if (this.docId) {
      (function () {
        var listener = instanceMap[_this2.docId].listener;
        _this2.pureChildren.forEach(function (node) {
          listener.removeElement(node.ref);
        });
      })();
    }
    this.children.forEach(function (node) {
      node.destroy();
    });
    this.children.length = 0;
    this.pureChildren.length = 0;
  };

  function nextElement(node) {
    while (node) {
      if (node.nodeType === 1) {
        return node;
      }
      node = node.nextSibling;
    }
  }

  function previousElement(node) {
    while (node) {
      if (node.nodeType === 1) {
        return node;
      }
      node = node.previousSibling;
    }
  }

  function linkParent(node, parent) {
    node.parentNode = parent;
    if (parent.docId) {
      node.docId = parent.docId;
      node.ownerDocument = parent.ownerDocument;
      node.ownerDocument.nodeMap[node.nodeId] = node;
      node.depth = parent.depth + 1;
    }
    node.children.forEach(function (child) {
      linkParent(child, node);
    });
  }

  function registerNode(docId, node) {
    var doc = instanceMap[docId];
    doc.nodeMap[node.nodeId] = node;
  }

  function insertIndex(target, list, newIndex, changeSibling) {
    if (newIndex < 0) {
      newIndex = 0;
    }
    var before = list[newIndex - 1];
    var after = list[newIndex];
    list.splice(newIndex, 0, target);
    if (changeSibling) {
      before && (before.nextSibling = target);
      target.previousSibling = before;
      target.nextSibling = after;
      after && (after.previousSibling = target);
    }
    return newIndex;
  }

  function moveIndex(target, list, newIndex, changeSibling) {
    var index = list.indexOf(target);
    if (index < 0) {
      return -1;
    }
    if (changeSibling) {
      var before = list[index - 1];
      var after = list[index + 1];
      before && (before.nextSibling = after);
      after && (after.previousSibling = before);
    }
    list.splice(index, 1);
    var newIndexAfter = newIndex;
    if (index <= newIndex) {
      newIndexAfter = newIndex - 1;
    }
    var beforeNew = list[newIndexAfter - 1];
    var afterNew = list[newIndexAfter];
    list.splice(newIndexAfter, 0, target);
    if (changeSibling) {
      beforeNew && (beforeNew.nextSibling = target);
      target.previousSibling = beforeNew;
      target.nextSibling = afterNew;
      afterNew && (afterNew.previousSibling = target);
    }
    if (index === newIndexAfter) {
      return -1;
    }
    return newIndex;
  }

  function removeIndex(target, list, changeSibling) {
    var index = list.indexOf(target);
    if (index < 0) {
      return;
    }
    if (changeSibling) {
      var before = list[index - 1];
      var after = list[index + 1];
      before && (before.nextSibling = after);
      after && (after.previousSibling = before);
    }
    list.splice(index, 1);
  }

  Element.prototype.setAttr = function (key, value, silent) {
    if (this.attr[key] === value) {
      return;
    }
    this.attr[key] = value;
    if (!silent && this.docId) {
      var listener = instanceMap[this.docId].listener;
      listener.setAttr(this.ref, key, value);
    }
  };

  Element.prototype.setStyle = function (key, value, silent) {
    if (this.style[key] === value) {
      return;
    }
    this.style[key] = value;
    if (!silent && this.docId) {
      var listener = instanceMap[this.docId].listener;
      listener.setStyle(this.ref, key, value);
    }
  };

  Element.prototype.setClassStyle = function (classStyle) {
    this.classStyle = classStyle;
    if (this.docId) {
      var listener = instanceMap[this.docId].listener;
      listener.setStyles(this.ref, this.toStyle());
    }
  };

  Element.prototype.addEvent = function (type, handler) {
    if (!this.event[type]) {
      this.event[type] = handler;
      if (this.docId) {
        var listener = instanceMap[this.docId].listener;
        listener.addEvent(this.ref, type);
      }
    }
  };

  Element.prototype.removeEvent = function (type) {
    if (this.event[type]) {
      delete this.event[type];
      if (this.docId) {
        var listener = instanceMap[this.docId].listener;
        listener.removeEvent(this.ref, type);
      }
    }
  };

  Element.prototype.fireEvent = function (type, e) {
    var handler = this.event[type];
    if (handler) {
      return handler.call(this, e);
    }
  };

  Element.prototype.toStyle = function () {
    return (0, _shared.extend)({}, this.classStyle, this.style);
  };

  Element.prototype.toJSON = function () {
    var result = {
      ref: this.ref.toString(),
      type: this.type,
      attr: this.attr,
      style: this.toStyle()
    };
    var event = Object.keys(this.event);
    if (event.length) {
      result.event = event;
    }
    if (this.pureChildren.length) {
      result.children = this.pureChildren.map(function (child) {
        return child.toJSON();
      });
    }
    return result;
  };

  Element.prototype.toString = function () {
    return '<' + this.type + ' attr=' + JSON.stringify(this.attr) + ' style=' + JSON.stringify(this.toStyle()) + '>' + this.pureChildren.map(function (child) {
      return child.toString();
    }).join('') + '</' + this.type + '>';
  };

  function Comment(value) {
    this.nodeType = 8;
    this.nodeId = (nextNodeRef++).toString();
    this.ref = this.nodeId;
    this.type = 'comment';
    this.value = value;
    this.children = [];
    this.pureChildren = [];
  }

  Comment.prototype = new Node();

  Comment.prototype.toString = function () {
    return '<!-- ' + this.value + ' -->';
  };

/***/ },
/* 100 */
/***/ function(module, exports) {

  'use strict';

  Object.defineProperty(exports, "__esModule", {
    value: true
  });
  exports.default = Listener;
  exports.createAction = createAction;
  function Listener(id, handler) {
    this.id = id;
    this.batched = false;
    this.updates = [];
    if (typeof handler === 'function') {
      this.handler = handler;
    }
  }

  Listener.prototype.createFinish = function (callback) {
    var handler = this.handler;
    return handler([createAction('createFinish', [])], callback);
  };

  Listener.prototype.updateFinish = function (callback) {
    var handler = this.handler;
    return handler([createAction('updateFinish', [])], callback);
  };

  Listener.prototype.refreshFinish = function (callback) {
    var handler = this.handler;
    return handler([createAction('refreshFinish', [])], callback);
  };

  Listener.prototype.createBody = function (element) {
    var body = element.toJSON();
    var children = body.children;
    delete body.children;
    var actions = [createAction('createBody', [body])];
    if (children) {
      actions.push.apply(actions, children.map(function (child) {
        return createAction('addElement', [body.ref, child, -1]);
      }));
    }
    return this.addActions(actions);
  };

  Listener.prototype.addElement = function (element, ref, index) {
    if (!(index >= 0)) {
      index = -1;
    }
    return this.addActions(createAction('addElement', [ref, element.toJSON(), index]));
  };

  Listener.prototype.removeElement = function (ref) {
    if (Array.isArray(ref)) {
      var actions = ref.map(function (r) {
        return createAction('removeElement', [r]);
      });
      return this.addActions(actions);
    }
    return this.addActions(createAction('removeElement', [ref]));
  };

  Listener.prototype.moveElement = function (targetRef, parentRef, index) {
    return this.addActions(createAction('moveElement', [targetRef, parentRef, index]));
  };

  Listener.prototype.setAttr = function (ref, key, value) {
    var result = {};
    result[key] = value;
    return this.addActions(createAction('updateAttrs', [ref, result]));
  };

  Listener.prototype.setStyle = function (ref, key, value) {
    var result = {};
    result[key] = value;
    return this.addActions(createAction('updateStyle', [ref, result]));
  };

  Listener.prototype.setStyles = function (ref, style) {
    return this.addActions(createAction('updateStyle', [ref, style]));
  };

  Listener.prototype.addEvent = function (ref, type) {
    return this.addActions(createAction('addEvent', [ref, type]));
  };

  Listener.prototype.removeEvent = function (ref, type) {
    return this.addActions(createAction('removeEvent', [ref, type]));
  };

  Listener.prototype.handler = function (actions, cb) {
    return cb && cb();
  };

  Listener.prototype.addActions = function (actions) {
    var updates = this.updates;
    var handler = this.handler;

    if (!Array.isArray(actions)) {
      actions = [actions];
    }

    if (this.batched) {
      updates.push.apply(updates, actions);
    } else {
      return handler(actions);
    }
  };

  function createAction(name, args) {
    return { module: 'dom', method: name, args: args };
  }

/***/ },
/* 101 */
/***/ function(module, exports) {

  module.exports = {
    "name": "weex-js-framework",
    "version": "0.15.2-test",
    "subversion": {
      "framework": "0.15.2-test",
      "transformer": ">=0.1.5 <0.4"
    },
    "description": "Weex JS Framework",
    "keywords": [
      "weex",
      "mvvm",
      "javascript",
      "html5"
    ],
    "homepage": "https://alibaba.github.io/weex",
    "bugs": {
      "url": "https://github.com/alibaba/weex/issues"
    },
    "license": "Apache-2.0",
    "author": "Jinjiang <zhaojinjiang@me.com>",
    "maintainers": [
      "terrykingcha <terrykingcha@gmail.com>",
      "IskenHuang <iskenhuang@gmail.com>",
      "yuanyan <yuanyan.cao@gmail.com>"
    ],
    "main": "index.js",
    "repository": {
      "type": "git",
      "url": "git@github.com:alibaba/weex.git"
    },
    "scripts": {
      "test": "echo \"Error: no test specified\" && exit 1"
    },
    "dependencies": {
      "core-js": "^2.4.0",
      "semver": "^5.1.0"
    }
  };

/***/ },
/* 102 */
/***/ function(module, exports, __webpack_require__) {

  /* WEBPACK VAR INJECTION */(function(global) {'use strict';

  Object.defineProperty(exports, "__esModule", {
    value: true
  });
  exports.$ = $;
  exports.$el = $el;
  exports.$vm = $vm;
  exports.$renderThen = $renderThen;
  exports.$scrollTo = $scrollTo;
  exports.$transition = $transition;
  exports.$getConfig = $getConfig;
  exports.$sendHttp = $sendHttp;
  exports.$openURL = $openURL;
  exports.$setTitle = $setTitle;
  exports.$call = $call;

  var _util = __webpack_require__(84);

  /**
   * ==========================================================
   * common
   * ==========================================================
   */

  /**
   * @deprecated use $vm instead
   * find the vm by id
   * Note: there is only one id in whole component
   * @param  {string} id
   * @return {Vm}
   */
  function $(id) {
    console.warn('[JS Framework] Vm#$ is deprecated, please use Vm#$vm instead');
    var info = this._ids[id];
    if (info) {
      return info.vm;
    }
  }

  /**
   * find the element by id
   * Note: there is only one id in whole component
   * @param  {string} id
   * @return {Element}
   */
  /**
   * @fileOverview The api for invoking with "$" prefix
   */
  function $el(id) {
    var info = this._ids[id];
    if (info) {
      return info.el;
    }
  }

  /**
   * find the vm of the custom component by id
   * Note: there is only one id in whole component
   * @param  {string} id
   * @return {Vm}
   */
  function $vm(id) {
    var info = this._ids[id];
    if (info) {
      return info.vm;
    }
  }

  /**
   * Fire when differ rendering finished
   *
   * @param  {Function} fn
   */
  function $renderThen(fn) {
    var app = this._app;
    var differ = app.differ;
    return differ.then(function () {
      fn();
    });
  }

  /**
   * scroll an element specified by id into view,
   * moreover specify a number of offset optionally
   * @param  {string} id
   * @param  {number} offset
   */
  function $scrollTo(id, offset) {
    console.warn('[JS Framework] Vm#$scrollTo is deprecated, ' + 'please use "require(\'@weex-module/dom\')' + '.scrollTo(el, options)" instead');
    var el = this.$el(id);
    if (el) {
      var dom = this._app.requireModule('dom');
      dom.scrollToElement(el.ref, { offset: offset });
    }
  }

  /**
   * perform transition animation on an element specified by id
   * @param  {string}   id
   * @param  {object}   options
   * @param  {object}   options.styles
   * @param  {object}   options.duration(ms)
   * @param  {object}   [options.timingFunction]
   * @param  {object}   [options.delay=0(ms)]
   * @param  {Function} callback
   */
  function $transition(id, options, callback) {
    var _this = this;

    var el = this.$el(id);
    if (el && options && options.styles) {
      var animation = this._app.requireModule('animation');
      animation.transition(el.ref, options, function () {
        _this._setStyle(el, options.styles);
        callback && callback.apply(undefined, arguments);
      });
    }
  }

  /**
   * get some config
   * @return {object} some config for app instance
   * @property {string} bundleUrl
   * @property {boolean} debug
   * @property {object} env
   * @property {string} env.weexVersion(ex. 1.0.0)
   * @property {string} env.appName(ex. TB/TM)
   * @property {string} env.appVersion(ex. 5.0.0)
   * @property {string} env.platform(ex. iOS/Android)
   * @property {string} env.osVersion(ex. 7.0.0)
   * @property {string} env.deviceModel **native only**
   * @property {number} env.[deviceWidth=750]
   * @property {number} env.deviceHeight
   */
  function $getConfig(callback) {
    var config = (0, _util.extend)({
      env: global.WXEnvironment || {}
    }, this._app.options);
    if ((0, _util.typof)(callback) === 'function') {
      console.warn('[JS Framework] the callback of Vm#$getConfig(callback) is deprecated, ' + 'this api now can directly RETURN config info.');
      callback(config);
    }
    return config;
  }

  /**
   * @deprecated
   * request network via http protocol
   * @param  {object}   params
   * @param  {Function} callback
   */
  function $sendHttp(params, callback) {
    console.warn('[JS Framework] Vm#$sendHttp is deprecated, ' + 'please use "require(\'@weex-module/stream\')' + '.sendHttp(params, callback)" instead');
    var stream = this._app.requireModule('stream');
    stream.sendHttp(params, callback);
  }

  /**
   * @deprecated
   * open a url
   * @param  {string} url
   */
  function $openURL(url) {
    console.warn('[JS Framework] Vm#$openURL is deprecated, ' + 'please use "require(\'@weex-module/event\')' + '.openURL(url)" instead');
    var event = this._app.requireModule('event');
    event.openURL(url);
  }

  /**
   * @deprecated
   * set a title for page
   * @param  {string} title
   */
  function $setTitle(title) {
    console.warn('[JS Framework] Vm#$setTitle is deprecated, ' + 'please use "require(\'@weex-module/pageInfo\')' + '.setTitle(title)" instead');
    var pageInfo = this._app.requireModule('pageInfo');
    pageInfo.setTitle(title);
  }

  /**
   * @deprecated use "require('@weex-module/moduleName') instead"
   * invoke a native method by specifing the name of module and method
   * @param  {string} moduleName
   * @param  {string} methodName
   * @param  {...*} the rest arguments
   */
  function $call(moduleName, methodName) {
    console.warn('[JS Framework] Vm#$call is deprecated, ' + 'please use "require(\'@weex-module/moduleName\')" instead');
    var module = this._app.requireModule(moduleName);
    if (module && module[methodName]) {
      for (var _len = arguments.length, args = Array(_len > 2 ? _len - 2 : 0), _key = 2; _key < _len; _key++) {
        args[_key - 2] = arguments[_key];
      }

      module[methodName].apply(module, args);
    }
  }
  /* WEBPACK VAR INJECTION */}.call(exports, (function() { return this; }())))

/***/ },
/* 103 */
/***/ function(module, exports) {

  'use strict';

  Object.defineProperty(exports, "__esModule", {
    value: true
  });
  exports.$userTrack = $userTrack;
  exports.$sendMtop = $sendMtop;
  exports.$callWindvane = $callWindvane;
  exports.$setSpm = $setSpm;
  exports.$getUserInfo = $getUserInfo;
  exports.$login = $login;
  exports.$logout = $logout;
  /**
   * @fileOverview The api for invoking with "$" prefix
   */

  /**
   * ==========================================================
   * private for ali
   * ==========================================================
   */

  /**
   * invoke user-track on Taobao Moblie
   * @param {string} type：enter, click, expose
   * @param {string} name
   * @param {string} comName
   * @param {object} param
  */
  function $userTrack(type, name, comName, param) {
    console.warn('[JS Framework] Vm#$userTrack is deprecated, ' + 'please use "require(\'@weex-module/userTrack\')' + '.commit(type, name, comName, param)" instead');
    var userTrack = this._app.requireModule('userTrack');
    userTrack.commit(type, name, comName, param);
  }

  /**
   * request a restfull api via the mtop gateway
   * @param  {object}   params
   * @param  {Function} callback
   */
  function $sendMtop(params, callback) {
    console.warn('[JS Framework] Vm#$sendMtop is deprecated, ' + 'please use "require(\'@weex-module/stream\')' + '.sendMtop(params, callback)" instead');
    /* istanbul ignore else */
    if (typeof window === 'undefined') {
      // in native，use windvane
      var windvane = this._app.requireModule('windvane');
      windvane.call({
        class: 'MtopWVPlugin',
        method: 'send',
        data: params
      }, callback);
    } else {
      // in web brwoser，use stream.sendMtop
      var stream = this._app.requireModule('stream');
      stream.sendMtop(params, callback);
    }
  }

  /**
   * request a native api via windvane protocol
   * @param  {object}   params
   * @param  {Function} callback
   */
  function $callWindvane(params, callback) {
    console.warn('[JS Framework] Vm#$callWindvane is deprecated, ' + 'please use "require(\'@weex-module/windvane\')' + '.call(params, callback)" instead');
    var windvane = this._app.requireModule('windvane');
    windvane.call(params, callback);
  }

  /**
   * set spm for the page
   * @param  {string} a
   * @param  {string} b
   */
  function $setSpm(a, b) {
    console.warn('[JS Framework] Vm#$setSpm is deprecated, ' + 'please use "require(\'@weex-module/pageInfo\')' + '.setSpm(a, b)" instead');
    var pageInfo = this._app.requireModule('pageInfo');
    pageInfo.setSpm(a, b);
  }

  /**
   * get the information of the current logined user
   * @param  {Function} callback
   */
  function $getUserInfo(callback) {
    console.warn('[JS Framework] Vm#$getUserInfo is deprecated, ' + 'please use "require(\'@weex-module/user\')' + '.getUserInfo(callback)" instead');
    var user = this._app.requireModule('user');
    user.getUserInfo(callback);
  }

  /**
   * perform login
   * @param  {Function} callback
   */
  function $login(callback) {
    console.warn('[JS Framework] Vm#$login is deprecated, ' + 'please use "require(\'@weex-module/user\')' + '.login(callback)" instead');
    var user = this._app.requireModule('user');
    user.login(callback);
  }

  /**
   * perform logout
   * @param  {Function} callback
   */
  function $logout(callback) {
    console.warn('[JS Framework] Vm#$logout is deprecated, ' + 'please use "require(\'@weex-module/user\')' + '.logout(callback)" instead');
    var user = this._app.requireModule('user');
    user.logout(callback);
  }

/***/ }
/******/ ]);
//# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJzb3VyY2VzIjpbIndlYnBhY2s6Ly8vd2VicGFjay9ib290c3RyYXAgZWZmZTcwMzJmNmNhYjI3YmM2NjEiLCJ3ZWJwYWNrOi8vLy4vaW5kZXguanMiLCJ3ZWJwYWNrOi8vLy9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9pbmRleC5qcyIsIndlYnBhY2s6Ly8vL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL3NyYy9uYXRpdmUvaW5kZXguanMiLCJ3ZWJwYWNrOi8vLy9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9zcmMvc2hhcmVkL2luZGV4LmpzIiwid2VicGFjazovLy8vVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvc3JjL3NoYXJlZC9zZXRUaW1lb3V0LmpzIiwid2VicGFjazovLy8vVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvc3JjL3NoYXJlZC9wcm9taXNlLmpzIiwid2VicGFjazovLy8vVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvfi9jb3JlLWpzL21vZHVsZXMvZXM2Lm9iamVjdC50by1zdHJpbmcuanMiLCJ3ZWJwYWNrOi8vLy9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9fY2xhc3NvZi5qcyIsIndlYnBhY2s6Ly8vL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL34vY29yZS1qcy9tb2R1bGVzL19jb2YuanMiLCJ3ZWJwYWNrOi8vLy9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9fd2tzLmpzIiwid2VicGFjazovLy8vVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvfi9jb3JlLWpzL21vZHVsZXMvX3NoYXJlZC5qcyIsIndlYnBhY2s6Ly8vL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL34vY29yZS1qcy9tb2R1bGVzL19nbG9iYWwuanMiLCJ3ZWJwYWNrOi8vLy9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9fdWlkLmpzIiwid2VicGFjazovLy8vVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvfi9jb3JlLWpzL21vZHVsZXMvX3JlZGVmaW5lLmpzIiwid2VicGFjazovLy8vVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvfi9jb3JlLWpzL21vZHVsZXMvX2hpZGUuanMiLCJ3ZWJwYWNrOi8vLy9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9fb2JqZWN0LWRwLmpzIiwid2VicGFjazovLy8vVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvfi9jb3JlLWpzL21vZHVsZXMvX2FuLW9iamVjdC5qcyIsIndlYnBhY2s6Ly8vL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL34vY29yZS1qcy9tb2R1bGVzL19pcy1vYmplY3QuanMiLCJ3ZWJwYWNrOi8vLy9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9faWU4LWRvbS1kZWZpbmUuanMiLCJ3ZWJwYWNrOi8vLy9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9fZGVzY3JpcHRvcnMuanMiLCJ3ZWJwYWNrOi8vLy9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9fZmFpbHMuanMiLCJ3ZWJwYWNrOi8vLy9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9fZG9tLWNyZWF0ZS5qcyIsIndlYnBhY2s6Ly8vL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL34vY29yZS1qcy9tb2R1bGVzL190by1wcmltaXRpdmUuanMiLCJ3ZWJwYWNrOi8vLy9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9fcHJvcGVydHktZGVzYy5qcyIsIndlYnBhY2s6Ly8vL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL34vY29yZS1qcy9tb2R1bGVzL19oYXMuanMiLCJ3ZWJwYWNrOi8vLy9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9fY29yZS5qcyIsIndlYnBhY2s6Ly8vL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL34vY29yZS1qcy9tb2R1bGVzL2VzNi5zdHJpbmcuaXRlcmF0b3IuanMiLCJ3ZWJwYWNrOi8vLy9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9fc3RyaW5nLWF0LmpzIiwid2VicGFjazovLy8vVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvfi9jb3JlLWpzL21vZHVsZXMvX3RvLWludGVnZXIuanMiLCJ3ZWJwYWNrOi8vLy9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9fZGVmaW5lZC5qcyIsIndlYnBhY2s6Ly8vL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL34vY29yZS1qcy9tb2R1bGVzL19pdGVyLWRlZmluZS5qcyIsIndlYnBhY2s6Ly8vL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL34vY29yZS1qcy9tb2R1bGVzL19saWJyYXJ5LmpzIiwid2VicGFjazovLy8vVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvfi9jb3JlLWpzL21vZHVsZXMvX2V4cG9ydC5qcyIsIndlYnBhY2s6Ly8vL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL34vY29yZS1qcy9tb2R1bGVzL19jdHguanMiLCJ3ZWJwYWNrOi8vLy9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9fYS1mdW5jdGlvbi5qcyIsIndlYnBhY2s6Ly8vL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL34vY29yZS1qcy9tb2R1bGVzL19pdGVyYXRvcnMuanMiLCJ3ZWJwYWNrOi8vLy9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9faXRlci1jcmVhdGUuanMiLCJ3ZWJwYWNrOi8vLy9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9fb2JqZWN0LWNyZWF0ZS5qcyIsIndlYnBhY2s6Ly8vL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL34vY29yZS1qcy9tb2R1bGVzL19vYmplY3QtZHBzLmpzIiwid2VicGFjazovLy8vVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvfi9jb3JlLWpzL21vZHVsZXMvX29iamVjdC1rZXlzLmpzIiwid2VicGFjazovLy8vVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvfi9jb3JlLWpzL21vZHVsZXMvX29iamVjdC1rZXlzLWludGVybmFsLmpzIiwid2VicGFjazovLy8vVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvfi9jb3JlLWpzL21vZHVsZXMvX3RvLWlvYmplY3QuanMiLCJ3ZWJwYWNrOi8vLy9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9faW9iamVjdC5qcyIsIndlYnBhY2s6Ly8vL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL34vY29yZS1qcy9tb2R1bGVzL19hcnJheS1pbmNsdWRlcy5qcyIsIndlYnBhY2s6Ly8vL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL34vY29yZS1qcy9tb2R1bGVzL190by1sZW5ndGguanMiLCJ3ZWJwYWNrOi8vLy9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9fdG8taW5kZXguanMiLCJ3ZWJwYWNrOi8vLy9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9fc2hhcmVkLWtleS5qcyIsIndlYnBhY2s6Ly8vL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL34vY29yZS1qcy9tb2R1bGVzL19lbnVtLWJ1Zy1rZXlzLmpzIiwid2VicGFjazovLy8vVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvfi9jb3JlLWpzL21vZHVsZXMvX2h0bWwuanMiLCJ3ZWJwYWNrOi8vLy9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9fc2V0LXRvLXN0cmluZy10YWcuanMiLCJ3ZWJwYWNrOi8vLy9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9fb2JqZWN0LWdwby5qcyIsIndlYnBhY2s6Ly8vL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL34vY29yZS1qcy9tb2R1bGVzL190by1vYmplY3QuanMiLCJ3ZWJwYWNrOi8vLy9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy93ZWIuZG9tLml0ZXJhYmxlLmpzIiwid2VicGFjazovLy8vVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvfi9jb3JlLWpzL21vZHVsZXMvZXM2LmFycmF5Lml0ZXJhdG9yLmpzIiwid2VicGFjazovLy8vVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvfi9jb3JlLWpzL21vZHVsZXMvX2FkZC10by11bnNjb3BhYmxlcy5qcyIsIndlYnBhY2s6Ly8vL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL34vY29yZS1qcy9tb2R1bGVzL19pdGVyLXN0ZXAuanMiLCJ3ZWJwYWNrOi8vLy9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9lczYucHJvbWlzZS5qcyIsIndlYnBhY2s6Ly8vL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL34vY29yZS1qcy9tb2R1bGVzL19hbi1pbnN0YW5jZS5qcyIsIndlYnBhY2s6Ly8vL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL34vY29yZS1qcy9tb2R1bGVzL19mb3Itb2YuanMiLCJ3ZWJwYWNrOi8vLy9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9faXRlci1jYWxsLmpzIiwid2VicGFjazovLy8vVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvfi9jb3JlLWpzL21vZHVsZXMvX2lzLWFycmF5LWl0ZXIuanMiLCJ3ZWJwYWNrOi8vLy9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9jb3JlLmdldC1pdGVyYXRvci1tZXRob2QuanMiLCJ3ZWJwYWNrOi8vLy9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9fc3BlY2llcy1jb25zdHJ1Y3Rvci5qcyIsIndlYnBhY2s6Ly8vL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL34vY29yZS1qcy9tb2R1bGVzL190YXNrLmpzIiwid2VicGFjazovLy8vVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvfi9jb3JlLWpzL21vZHVsZXMvX2ludm9rZS5qcyIsIndlYnBhY2s6Ly8vL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL34vY29yZS1qcy9tb2R1bGVzL19taWNyb3Rhc2suanMiLCJ3ZWJwYWNrOi8vLy9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9fcmVkZWZpbmUtYWxsLmpzIiwid2VicGFjazovLy8vVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvfi9jb3JlLWpzL21vZHVsZXMvX3NldC1zcGVjaWVzLmpzIiwid2VicGFjazovLy8vVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvfi9jb3JlLWpzL21vZHVsZXMvX2l0ZXItZGV0ZWN0LmpzIiwid2VicGFjazovLy8vVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvc3JjL3NoYXJlZC9jb25zb2xlLmpzIiwid2VicGFjazovLy8vVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvc3JjL3NoYXJlZC9vYmplY3RBc3NpZ24uanMiLCJ3ZWJwYWNrOi8vLy9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvZm4vb2JqZWN0L2Fzc2lnbi5qcyIsIndlYnBhY2s6Ly8vL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL34vY29yZS1qcy9tb2R1bGVzL2VzNi5vYmplY3QuYXNzaWduLmpzIiwid2VicGFjazovLy8vVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvfi9jb3JlLWpzL21vZHVsZXMvX29iamVjdC1hc3NpZ24uanMiLCJ3ZWJwYWNrOi8vLy9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9fb2JqZWN0LWdvcHMuanMiLCJ3ZWJwYWNrOi8vLy9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9fb2JqZWN0LXBpZS5qcyIsIndlYnBhY2s6Ly8vL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL3NyYy9ydW50aW1lL2luZGV4LmpzIiwid2VicGFjazovLy8vVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvc3JjL3J1bnRpbWUvY29uZmlnLmpzIiwid2VicGFjazovLy8vVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvc3JjL2RlZmF1bHQvaW5kZXguanMiLCJ3ZWJwYWNrOi8vLy9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9zcmMvZGVmYXVsdC9jb25maWcuanMiLCJ3ZWJwYWNrOi8vLy9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9zcmMvZGVmYXVsdC9hcHAvaW5kZXguanMiLCJ3ZWJwYWNrOi8vLy9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9zcmMvZGVmYXVsdC9hcHAvZGlmZmVyLmpzIiwid2VicGFjazovLy8vVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvc3JjL2RlZmF1bHQvYXBwL3JlZ2lzdGVyLmpzIiwid2VicGFjazovLy8vVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvc3JjL2RlZmF1bHQvYXBwL2N0cmwuanMiLCJ3ZWJwYWNrOi8vLy9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9zcmMvZGVmYXVsdC91dGlsL2luZGV4LmpzIiwid2VicGFjazovLy8vVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvc3JjL2RlZmF1bHQvYXBwL2J1bmRsZS5qcyIsIndlYnBhY2s6Ly8vL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL34vc2VtdmVyL3NlbXZlci5qcyIsIndlYnBhY2s6Ly8vKHdlYnBhY2spL34vbm9kZS1saWJzLWJyb3dzZXIvfi9wcm9jZXNzL2Jyb3dzZXIuanMiLCJ3ZWJwYWNrOi8vLy9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9zcmMvZGVmYXVsdC92bS9pbmRleC5qcyIsIndlYnBhY2s6Ly8vL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL3NyYy9kZWZhdWx0L2NvcmUvc3RhdGUuanMiLCJ3ZWJwYWNrOi8vLy9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9zcmMvZGVmYXVsdC9jb3JlL3dhdGNoZXIuanMiLCJ3ZWJwYWNrOi8vLy9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9zcmMvZGVmYXVsdC9jb3JlL2RlcC5qcyIsIndlYnBhY2s6Ly8vL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL3NyYy9kZWZhdWx0L2NvcmUvb2JzZXJ2ZXIuanMiLCJ3ZWJwYWNrOi8vLy9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9zcmMvZGVmYXVsdC9jb3JlL2FycmF5LmpzIiwid2VicGFjazovLy8vVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvc3JjL2RlZmF1bHQvdm0vY29tcGlsZXIuanMiLCJ3ZWJwYWNrOi8vLy9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9zcmMvZGVmYXVsdC92bS9kaXJlY3RpdmUuanMiLCJ3ZWJwYWNrOi8vLy9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9zcmMvZGVmYXVsdC92bS9kb20taGVscGVyLmpzIiwid2VicGFjazovLy8vVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvc3JjL2RlZmF1bHQvdm0vZXZlbnRzLmpzIiwid2VicGFjazovLy8vVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvc3JjL2RlZmF1bHQvYXBwL2Rvd25ncmFkZS5qcyIsIndlYnBhY2s6Ly8vL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL3NyYy92ZG9tL2luZGV4LmpzIiwid2VicGFjazovLy8vVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvc3JjL3Zkb20vbGlzdGVuZXIuanMiLCJ3ZWJwYWNrOi8vLy9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9wYWNrYWdlLmpzb24iLCJ3ZWJwYWNrOi8vLy9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9zcmMvZGVmYXVsdC9hcGkvbWV0aG9kcy5qcyIsIndlYnBhY2s6Ly8vLi9saWIvYXBpL21ldGhvZHMuanMiXSwibmFtZXMiOltdLCJtYXBwaW5ncyI6Ijs7QUFBQTtBQUNBOztBQUVBO0FBQ0E7O0FBRUE7QUFDQTtBQUNBOztBQUVBO0FBQ0E7QUFDQSx1QkFBZTtBQUNmO0FBQ0E7QUFDQTs7QUFFQTtBQUNBOztBQUVBO0FBQ0E7O0FBRUE7QUFDQTtBQUNBOzs7QUFHQTtBQUNBOztBQUVBO0FBQ0E7O0FBRUE7QUFDQTs7QUFFQTtBQUNBOzs7Ozs7Ozs7QUN0Q0E7O0FBRUE7OztBQUdBLEtBQU0sVUFBVSxvQkFBUSxHQUFSLENBQWhCO2VBQzBCLE07S0FBbkIsZSxXQUFBLGU7O0FBQ1AsaUJBQWdCLE9BQWhCLEU7Ozs7Ozs7OztBQ1BBLHdCOzs7Ozs7OztBQ0FBOztBQUNBOzs7O0FBQ0E7O0FBQ0E7O0tBQVksTzs7Ozs7O0tBRUosTSx1QkFBQSxNO0tBQVEsVyx1QkFBQSxXOztBQUVoQjs7NEJBQ1csVTtBQUNULFVBQU8sVUFBUCxJQUFxQixZQUFhO0FBQ2hDLFNBQU0sTUFBTSxrQkFBUSxVQUFSLHFDQUFaO0FBQ0EsU0FBSSxlQUFlLEtBQW5CLEVBQTBCO0FBQ3hCLGVBQVEsS0FBUixDQUFjLElBQUksUUFBSixFQUFkO0FBQ0Q7QUFDRCxZQUFPLEdBQVA7QUFDRCxJQU5EOzs7QUFERixNQUFLLElBQU0sVUFBWCx1QkFBa0M7QUFBQSxTQUF2QixVQUF1QjtBQVFqQzs7QUFFRDtBQUNBLFFBQU8sZ0JBQVAsR0FBMEIsTUFBMUI7QUFDQSxRQUFPLGtCQUFQLEdBQTRCLFdBQTVCOztBQUVBO0FBQ0EsUUFBTyxlQUFQLENBQXVCLE9BQXZCLEU7Ozs7Ozs7Ozs7Ozs7OztTQ1hnQixNLEdBQUEsTTtTQXlCQSxHLEdBQUEsRztTQWdCQSxNLEdBQUEsTTtTQWtCQSxNLEdBQUEsTTtTQVlBLEksR0FBQSxJO1NBbUJBLE8sR0FBQSxPO1NBbUJBLFEsR0FBQSxRO1NBY0EsYSxHQUFBLGE7O0FBdkloQjs7QUFDQTs7QUFDQTs7QUFDQTs7QUFFQTs7Ozs7OztBQU9PLFVBQVMsTUFBVCxDQUFpQixNQUFqQixFQUFpQztBQUFBLHFDQUFMLEdBQUs7QUFBTCxRQUFLO0FBQUE7O0FBQ3RDLE9BQUksT0FBTyxPQUFPLE1BQWQsS0FBeUIsVUFBN0IsRUFBeUM7QUFDdkMsWUFBTyxNQUFQLGdCQUFjLE1BQWQsU0FBeUIsR0FBekI7QUFDRCxJQUZELE1BR0s7QUFDSCxTQUFNLFFBQVEsSUFBSSxLQUFKLEVBQWQ7QUFDQSxVQUFLLElBQU0sR0FBWCxJQUFrQixLQUFsQixFQUF5QjtBQUN2QixjQUFPLEdBQVAsSUFBYyxNQUFNLEdBQU4sQ0FBZDtBQUNEO0FBQ0QsU0FBSSxJQUFJLE1BQVIsRUFBZ0I7QUFDZCxnQ0FBTyxNQUFQLFNBQWtCLEdBQWxCO0FBQ0Q7QUFDRjtBQUNELFVBQU8sTUFBUDtBQUNEOztBQUVEOzs7Ozs7Ozs7QUFTTyxVQUFTLEdBQVQsQ0FBYyxHQUFkLEVBQW1CLEdBQW5CLEVBQXdCLEdBQXhCLEVBQTZCLFVBQTdCLEVBQXlDO0FBQzlDLFVBQU8sY0FBUCxDQUFzQixHQUF0QixFQUEyQixHQUEzQixFQUFnQztBQUM5QixZQUFPLEdBRHVCO0FBRTlCLGlCQUFZLENBQUMsQ0FBQyxVQUZnQjtBQUc5QixlQUFVLElBSG9CO0FBSTlCLG1CQUFjO0FBSmdCLElBQWhDO0FBTUQ7O0FBRUQ7Ozs7Ozs7QUFPTyxVQUFTLE1BQVQsQ0FBaUIsR0FBakIsRUFBc0IsSUFBdEIsRUFBNEI7QUFDakMsT0FBSSxJQUFJLE1BQVIsRUFBZ0I7QUFDZCxTQUFNLFFBQVEsSUFBSSxPQUFKLENBQVksSUFBWixDQUFkO0FBQ0EsU0FBSSxRQUFRLENBQUMsQ0FBYixFQUFnQjtBQUNkLGNBQU8sSUFBSSxNQUFKLENBQVcsS0FBWCxFQUFrQixDQUFsQixDQUFQO0FBQ0Q7QUFDRjtBQUNGOztBQUVEOzs7Ozs7OztBQVFBLEtBQU0saUJBQWlCLE9BQU8sU0FBUCxDQUFpQixjQUF4QztBQUNPLFVBQVMsTUFBVCxDQUFpQixHQUFqQixFQUFzQixHQUF0QixFQUEyQjtBQUNoQyxVQUFPLGVBQWUsSUFBZixDQUFvQixHQUFwQixFQUF5QixHQUF6QixDQUFQO0FBQ0Q7O0FBRUQ7Ozs7Ozs7O0FBUU8sVUFBUyxJQUFULENBQWUsRUFBZixFQUFtQixHQUFuQixFQUF3QjtBQUM3QixVQUFPLFVBQVUsQ0FBVixFQUFhO0FBQ2xCLFNBQU0sSUFBSSxVQUFVLE1BQXBCO0FBQ0EsWUFBTyxJQUNILElBQUksQ0FBSixHQUNFLEdBQUcsS0FBSCxDQUFTLEdBQVQsRUFBYyxTQUFkLENBREYsR0FFRSxHQUFHLElBQUgsQ0FBUSxHQUFSLEVBQWEsQ0FBYixDQUhDLEdBSUgsR0FBRyxJQUFILENBQVEsR0FBUixDQUpKO0FBS0QsSUFQRDtBQVFEOztBQUVEOzs7Ozs7OztBQVFPLFVBQVMsT0FBVCxDQUFrQixJQUFsQixFQUF3QixLQUF4QixFQUErQjtBQUNwQyxXQUFRLFNBQVMsQ0FBakI7QUFDQSxPQUFJLElBQUksS0FBSyxNQUFMLEdBQWMsS0FBdEI7QUFDQSxPQUFNLE1BQU0sSUFBSSxLQUFKLENBQVUsQ0FBVixDQUFaO0FBQ0EsVUFBTyxHQUFQLEVBQVk7QUFDVixTQUFJLENBQUosSUFBUyxLQUFLLElBQUksS0FBVCxDQUFUO0FBQ0Q7QUFDRCxVQUFPLEdBQVA7QUFDRDs7QUFFRDs7Ozs7Ozs7O0FBU08sVUFBUyxRQUFULENBQW1CLEdBQW5CLEVBQXdCO0FBQzdCLFVBQU8sUUFBUSxJQUFSLElBQWdCLFFBQU8sR0FBUCx5Q0FBTyxHQUFQLE9BQWUsUUFBdEM7QUFDRDs7QUFFRDs7Ozs7Ozs7QUFRQSxLQUFNLFdBQVcsT0FBTyxTQUFQLENBQWlCLFFBQWxDO0FBQ0EsS0FBTSxnQkFBZ0IsaUJBQXRCO0FBQ08sVUFBUyxhQUFULENBQXdCLEdBQXhCLEVBQTZCO0FBQ2xDLFVBQU8sU0FBUyxJQUFULENBQWMsR0FBZCxNQUF1QixhQUE5QjtBQUNELEU7Ozs7Ozs7O0FDeklEOzs7Ozs7Ozs7O2VBYUksTTtLQUZGLFUsV0FBQSxVO0tBQ0EsZ0IsV0FBQSxnQjs7QUFHRjs7QUFDQSxLQUFJLE9BQU8sVUFBUCxLQUFzQixXQUF0QixJQUNGLE9BQU8sZ0JBQVAsS0FBNEIsVUFEOUIsRUFDMEM7QUFBQTtBQUN4QyxTQUFNLGFBQWEsRUFBbkI7QUFDQSxTQUFJLFlBQVksQ0FBaEI7O0FBRUEsWUFBTyxVQUFQLEdBQW9CLFVBQUMsRUFBRCxFQUFLLElBQUwsRUFBYztBQUNoQyxrQkFBVyxFQUFFLFNBQWIsSUFBMEIsRUFBMUI7QUFDQSx3QkFBaUIsVUFBVSxRQUFWLEVBQWpCLEVBQXVDLElBQXZDO0FBQ0QsTUFIRDs7QUFLQSxZQUFPLGtCQUFQLEdBQTRCLFVBQUMsRUFBRCxFQUFRO0FBQ2xDLFdBQUksT0FBTyxXQUFXLEVBQVgsQ0FBUCxLQUEwQixVQUE5QixFQUEwQztBQUN4QyxvQkFBVyxFQUFYO0FBQ0EsZ0JBQU8sV0FBVyxFQUFYLENBQVA7QUFDRDtBQUNGLE1BTEQ7QUFUd0M7QUFlekMsRTs7Ozs7Ozs7O0FDaENEO0FBQ0E7QUFDQTtBQUNBLHFCQUFRLENBQVI7QUFDQSxxQkFBUSxFQUFSO0FBQ0EscUJBQVEsRUFBUjtBQUNBLHFCQUFRLEVBQVIsRTs7Ozs7O0FDTkE7QUFDQTs7QUFDQSxLQUFJLFVBQVUsb0JBQVEsQ0FBUixDQUFkO0FBQUEsS0FDSSxPQUFVLEVBRGQ7QUFFQSxNQUFLLG9CQUFRLENBQVIsRUFBa0IsYUFBbEIsQ0FBTCxJQUF5QyxHQUF6QztBQUNBLEtBQUcsT0FBTyxFQUFQLElBQWEsWUFBaEIsRUFBNkI7QUFDM0IsdUJBQVEsRUFBUixFQUF1QixPQUFPLFNBQTlCLEVBQXlDLFVBQXpDLEVBQXFELFNBQVMsUUFBVCxHQUFtQjtBQUN0RSxZQUFPLGFBQWEsUUFBUSxJQUFSLENBQWIsR0FBNkIsR0FBcEM7QUFDRCxJQUZELEVBRUcsSUFGSDtBQUdELEU7Ozs7Ozs7O0FDVEQ7QUFDQSxLQUFJLE1BQU0sb0JBQVEsQ0FBUixDQUFWO0FBQUEsS0FDSSxNQUFNLG9CQUFRLENBQVIsRUFBa0IsYUFBbEI7QUFDUjtBQUZGO0FBQUEsS0FHSSxNQUFNLElBQUksWUFBVTtBQUFFLFVBQU8sU0FBUDtBQUFtQixFQUEvQixFQUFKLEtBQTBDLFdBSHBEOztBQUtBO0FBQ0EsS0FBSSxTQUFTLFNBQVQsTUFBUyxDQUFTLEVBQVQsRUFBYSxHQUFiLEVBQWlCO0FBQzVCLE9BQUk7QUFDRixZQUFPLEdBQUcsR0FBSCxDQUFQO0FBQ0QsSUFGRCxDQUVFLE9BQU0sQ0FBTixFQUFRLENBQUUsV0FBYTtBQUMxQixFQUpEOztBQU1BLFFBQU8sT0FBUCxHQUFpQixVQUFTLEVBQVQsRUFBWTtBQUMzQixPQUFJLENBQUosRUFBTyxDQUFQLEVBQVUsQ0FBVjtBQUNBLFVBQU8sT0FBTyxTQUFQLEdBQW1CLFdBQW5CLEdBQWlDLE9BQU8sSUFBUCxHQUFjO0FBQ3BEO0FBRHNDLEtBRXBDLFFBQVEsSUFBSSxPQUFPLElBQUksT0FBTyxFQUFQLENBQVgsRUFBdUIsR0FBdkIsQ0FBWixLQUE0QyxRQUE1QyxHQUF1RDtBQUN6RDtBQURFLEtBRUEsTUFBTSxJQUFJLENBQUo7QUFDUjtBQURFLEtBRUEsQ0FBQyxJQUFJLElBQUksQ0FBSixDQUFMLEtBQWdCLFFBQWhCLElBQTRCLE9BQU8sRUFBRSxNQUFULElBQW1CLFVBQS9DLEdBQTRELFdBQTVELEdBQTBFLENBTjlFO0FBT0QsRUFURCxDOzs7Ozs7OztBQ2JBLEtBQUksV0FBVyxHQUFHLFFBQWxCOztBQUVBLFFBQU8sT0FBUCxHQUFpQixVQUFTLEVBQVQsRUFBWTtBQUMzQixVQUFPLFNBQVMsSUFBVCxDQUFjLEVBQWQsRUFBa0IsS0FBbEIsQ0FBd0IsQ0FBeEIsRUFBMkIsQ0FBQyxDQUE1QixDQUFQO0FBQ0QsRUFGRCxDOzs7Ozs7OztBQ0ZBLEtBQUksUUFBYSxvQkFBUSxFQUFSLEVBQXFCLEtBQXJCLENBQWpCO0FBQUEsS0FDSSxNQUFhLG9CQUFRLEVBQVIsQ0FEakI7QUFBQSxLQUVJLFVBQWEsb0JBQVEsRUFBUixFQUFxQixNQUZ0QztBQUFBLEtBR0ksYUFBYSxPQUFPLE9BQVAsSUFBaUIsVUFIbEM7O0FBS0EsS0FBSSxXQUFXLE9BQU8sT0FBUCxHQUFpQixVQUFTLElBQVQsRUFBYztBQUM1QyxVQUFPLE1BQU0sSUFBTixNQUFnQixNQUFNLElBQU4sSUFDckIsY0FBYyxRQUFPLElBQVAsQ0FBZCxJQUE4QixDQUFDLGFBQWEsT0FBYixHQUFzQixHQUF2QixFQUE0QixZQUFZLElBQXhDLENBRHpCLENBQVA7QUFFRCxFQUhEOztBQUtBLFVBQVMsS0FBVCxHQUFpQixLQUFqQixDOzs7Ozs7OztBQ1ZBLEtBQUksU0FBUyxvQkFBUSxFQUFSLENBQWI7QUFBQSxLQUNJLFNBQVMsb0JBRGI7QUFBQSxLQUVJLFFBQVMsT0FBTyxNQUFQLE1BQW1CLE9BQU8sTUFBUCxJQUFpQixFQUFwQyxDQUZiO0FBR0EsUUFBTyxPQUFQLEdBQWlCLFVBQVMsR0FBVCxFQUFhO0FBQzVCLFVBQU8sTUFBTSxHQUFOLE1BQWUsTUFBTSxHQUFOLElBQWEsRUFBNUIsQ0FBUDtBQUNELEVBRkQsQzs7Ozs7Ozs7QUNIQTtBQUNBLEtBQUksU0FBUyxPQUFPLE9BQVAsR0FBaUIsT0FBTyxNQUFQLElBQWlCLFdBQWpCLElBQWdDLE9BQU8sSUFBUCxJQUFlLElBQS9DLEdBQzFCLE1BRDBCLEdBQ2pCLE9BQU8sSUFBUCxJQUFlLFdBQWYsSUFBOEIsS0FBSyxJQUFMLElBQWEsSUFBM0MsR0FBa0QsSUFBbEQsR0FBeUQsU0FBUyxhQUFULEdBRHRFO0FBRUEsS0FBRyxPQUFPLEdBQVAsSUFBYyxRQUFqQixFQUEwQixNQUFNLE1BQU4sQyxDQUFjLCtCOzs7Ozs7OztBQ0h4QyxLQUFJLEtBQUssQ0FBVDtBQUFBLEtBQ0ksS0FBSyxLQUFLLE1BQUwsRUFEVDtBQUVBLFFBQU8sT0FBUCxHQUFpQixVQUFTLEdBQVQsRUFBYTtBQUM1QixVQUFPLFVBQVUsTUFBVixDQUFpQixRQUFRLFNBQVIsR0FBb0IsRUFBcEIsR0FBeUIsR0FBMUMsRUFBK0MsSUFBL0MsRUFBcUQsQ0FBQyxFQUFFLEVBQUYsR0FBTyxFQUFSLEVBQVksUUFBWixDQUFxQixFQUFyQixDQUFyRCxDQUFQO0FBQ0QsRUFGRCxDOzs7Ozs7OztBQ0ZBLEtBQUksU0FBWSxvQkFBUSxFQUFSLENBQWhCO0FBQUEsS0FDSSxPQUFZLG9CQUFRLEVBQVIsQ0FEaEI7QUFBQSxLQUVJLE1BQVksb0JBQVEsRUFBUixDQUZoQjtBQUFBLEtBR0ksTUFBWSxvQkFBUSxFQUFSLEVBQWtCLEtBQWxCLENBSGhCO0FBQUEsS0FJSSxZQUFZLFVBSmhCO0FBQUEsS0FLSSxZQUFZLFNBQVMsU0FBVCxDQUxoQjtBQUFBLEtBTUksTUFBWSxDQUFDLEtBQUssU0FBTixFQUFpQixLQUFqQixDQUF1QixTQUF2QixDQU5oQjs7QUFRQSxxQkFBUSxFQUFSLEVBQW1CLGFBQW5CLEdBQW1DLFVBQVMsRUFBVCxFQUFZO0FBQzdDLFVBQU8sVUFBVSxJQUFWLENBQWUsRUFBZixDQUFQO0FBQ0QsRUFGRDs7QUFJQSxFQUFDLE9BQU8sT0FBUCxHQUFpQixVQUFTLENBQVQsRUFBWSxHQUFaLEVBQWlCLEdBQWpCLEVBQXNCLElBQXRCLEVBQTJCO0FBQzNDLE9BQUksYUFBYSxPQUFPLEdBQVAsSUFBYyxVQUEvQjtBQUNBLE9BQUcsVUFBSCxFQUFjLElBQUksR0FBSixFQUFTLE1BQVQsS0FBb0IsS0FBSyxHQUFMLEVBQVUsTUFBVixFQUFrQixHQUFsQixDQUFwQjtBQUNkLE9BQUcsRUFBRSxHQUFGLE1BQVcsR0FBZCxFQUFrQjtBQUNsQixPQUFHLFVBQUgsRUFBYyxJQUFJLEdBQUosRUFBUyxHQUFULEtBQWlCLEtBQUssR0FBTCxFQUFVLEdBQVYsRUFBZSxFQUFFLEdBQUYsSUFBUyxLQUFLLEVBQUUsR0FBRixDQUFkLEdBQXVCLElBQUksSUFBSixDQUFTLE9BQU8sR0FBUCxDQUFULENBQXRDLENBQWpCO0FBQ2QsT0FBRyxNQUFNLE1BQVQsRUFBZ0I7QUFDZCxPQUFFLEdBQUYsSUFBUyxHQUFUO0FBQ0QsSUFGRCxNQUVPO0FBQ0wsU0FBRyxDQUFDLElBQUosRUFBUztBQUNQLGNBQU8sRUFBRSxHQUFGLENBQVA7QUFDQSxZQUFLLENBQUwsRUFBUSxHQUFSLEVBQWEsR0FBYjtBQUNELE1BSEQsTUFHTztBQUNMLFdBQUcsRUFBRSxHQUFGLENBQUgsRUFBVSxFQUFFLEdBQUYsSUFBUyxHQUFULENBQVYsS0FDSyxLQUFLLENBQUwsRUFBUSxHQUFSLEVBQWEsR0FBYjtBQUNOO0FBQ0Y7QUFDSDtBQUNDLEVBakJELEVBaUJHLFNBQVMsU0FqQlosRUFpQnVCLFNBakJ2QixFQWlCa0MsU0FBUyxRQUFULEdBQW1CO0FBQ25ELFVBQU8sT0FBTyxJQUFQLElBQWUsVUFBZixJQUE2QixLQUFLLEdBQUwsQ0FBN0IsSUFBMEMsVUFBVSxJQUFWLENBQWUsSUFBZixDQUFqRDtBQUNELEVBbkJELEU7Ozs7Ozs7O0FDWkEsS0FBSSxLQUFhLG9CQUFRLEVBQVIsQ0FBakI7QUFBQSxLQUNJLGFBQWEsb0JBQVEsRUFBUixDQURqQjtBQUVBLFFBQU8sT0FBUCxHQUFpQixvQkFBUSxFQUFSLElBQTRCLFVBQVMsTUFBVCxFQUFpQixHQUFqQixFQUFzQixLQUF0QixFQUE0QjtBQUN2RSxVQUFPLEdBQUcsQ0FBSCxDQUFLLE1BQUwsRUFBYSxHQUFiLEVBQWtCLFdBQVcsQ0FBWCxFQUFjLEtBQWQsQ0FBbEIsQ0FBUDtBQUNELEVBRmdCLEdBRWIsVUFBUyxNQUFULEVBQWlCLEdBQWpCLEVBQXNCLEtBQXRCLEVBQTRCO0FBQzlCLFVBQU8sR0FBUCxJQUFjLEtBQWQ7QUFDQSxVQUFPLE1BQVA7QUFDRCxFQUxELEM7Ozs7Ozs7O0FDRkEsS0FBSSxXQUFpQixvQkFBUSxFQUFSLENBQXJCO0FBQUEsS0FDSSxpQkFBaUIsb0JBQVEsRUFBUixDQURyQjtBQUFBLEtBRUksY0FBaUIsb0JBQVEsRUFBUixDQUZyQjtBQUFBLEtBR0ksS0FBaUIsT0FBTyxjQUg1Qjs7QUFLQSxTQUFRLENBQVIsR0FBWSxvQkFBUSxFQUFSLElBQTRCLE9BQU8sY0FBbkMsR0FBb0QsU0FBUyxjQUFULENBQXdCLENBQXhCLEVBQTJCLENBQTNCLEVBQThCLFVBQTlCLEVBQXlDO0FBQ3ZHLFlBQVMsQ0FBVDtBQUNBLE9BQUksWUFBWSxDQUFaLEVBQWUsSUFBZixDQUFKO0FBQ0EsWUFBUyxVQUFUO0FBQ0EsT0FBRyxjQUFILEVBQWtCLElBQUk7QUFDcEIsWUFBTyxHQUFHLENBQUgsRUFBTSxDQUFOLEVBQVMsVUFBVCxDQUFQO0FBQ0QsSUFGaUIsQ0FFaEIsT0FBTSxDQUFOLEVBQVEsQ0FBRSxXQUFhO0FBQ3pCLE9BQUcsU0FBUyxVQUFULElBQXVCLFNBQVMsVUFBbkMsRUFBOEMsTUFBTSxVQUFVLDBCQUFWLENBQU47QUFDOUMsT0FBRyxXQUFXLFVBQWQsRUFBeUIsRUFBRSxDQUFGLElBQU8sV0FBVyxLQUFsQjtBQUN6QixVQUFPLENBQVA7QUFDRCxFQVZELEM7Ozs7Ozs7O0FDTEEsS0FBSSxXQUFXLG9CQUFRLEVBQVIsQ0FBZjtBQUNBLFFBQU8sT0FBUCxHQUFpQixVQUFTLEVBQVQsRUFBWTtBQUMzQixPQUFHLENBQUMsU0FBUyxFQUFULENBQUosRUFBaUIsTUFBTSxVQUFVLEtBQUssb0JBQWYsQ0FBTjtBQUNqQixVQUFPLEVBQVA7QUFDRCxFQUhELEM7Ozs7Ozs7Ozs7QUNEQSxRQUFPLE9BQVAsR0FBaUIsVUFBUyxFQUFULEVBQVk7QUFDM0IsVUFBTyxRQUFPLEVBQVAseUNBQU8sRUFBUCxPQUFjLFFBQWQsR0FBeUIsT0FBTyxJQUFoQyxHQUF1QyxPQUFPLEVBQVAsS0FBYyxVQUE1RDtBQUNELEVBRkQsQzs7Ozs7Ozs7QUNBQSxRQUFPLE9BQVAsR0FBaUIsQ0FBQyxvQkFBUSxFQUFSLENBQUQsSUFBOEIsQ0FBQyxvQkFBUSxFQUFSLEVBQW9CLFlBQVU7QUFDNUUsVUFBTyxPQUFPLGNBQVAsQ0FBc0Isb0JBQVEsRUFBUixFQUF5QixLQUF6QixDQUF0QixFQUF1RCxHQUF2RCxFQUE0RCxFQUFDLEtBQUssZUFBVTtBQUFFLGNBQU8sQ0FBUDtBQUFXLE1BQTdCLEVBQTVELEVBQTRGLENBQTVGLElBQWlHLENBQXhHO0FBQ0QsRUFGK0MsQ0FBaEQsQzs7Ozs7Ozs7QUNBQTtBQUNBLFFBQU8sT0FBUCxHQUFpQixDQUFDLG9CQUFRLEVBQVIsRUFBb0IsWUFBVTtBQUM5QyxVQUFPLE9BQU8sY0FBUCxDQUFzQixFQUF0QixFQUEwQixHQUExQixFQUErQixFQUFDLEtBQUssZUFBVTtBQUFFLGNBQU8sQ0FBUDtBQUFXLE1BQTdCLEVBQS9CLEVBQStELENBQS9ELElBQW9FLENBQTNFO0FBQ0QsRUFGaUIsQ0FBbEIsQzs7Ozs7Ozs7QUNEQSxRQUFPLE9BQVAsR0FBaUIsVUFBUyxJQUFULEVBQWM7QUFDN0IsT0FBSTtBQUNGLFlBQU8sQ0FBQyxDQUFDLE1BQVQ7QUFDRCxJQUZELENBRUUsT0FBTSxDQUFOLEVBQVE7QUFDUixZQUFPLElBQVA7QUFDRDtBQUNGLEVBTkQsQzs7Ozs7Ozs7QUNBQSxLQUFJLFdBQVcsb0JBQVEsRUFBUixDQUFmO0FBQUEsS0FDSSxXQUFXLG9CQUFRLEVBQVIsRUFBcUI7QUFDbEM7QUFGRjtBQUFBLEtBR0ksS0FBSyxTQUFTLFFBQVQsS0FBc0IsU0FBUyxTQUFTLGFBQWxCLENBSC9CO0FBSUEsUUFBTyxPQUFQLEdBQWlCLFVBQVMsRUFBVCxFQUFZO0FBQzNCLFVBQU8sS0FBSyxTQUFTLGFBQVQsQ0FBdUIsRUFBdkIsQ0FBTCxHQUFrQyxFQUF6QztBQUNELEVBRkQsQzs7Ozs7Ozs7QUNKQTtBQUNBLEtBQUksV0FBVyxvQkFBUSxFQUFSLENBQWY7QUFDQTtBQUNBO0FBQ0EsUUFBTyxPQUFQLEdBQWlCLFVBQVMsRUFBVCxFQUFhLENBQWIsRUFBZTtBQUM5QixPQUFHLENBQUMsU0FBUyxFQUFULENBQUosRUFBaUIsT0FBTyxFQUFQO0FBQ2pCLE9BQUksRUFBSixFQUFRLEdBQVI7QUFDQSxPQUFHLEtBQUssUUFBUSxLQUFLLEdBQUcsUUFBaEIsS0FBNkIsVUFBbEMsSUFBZ0QsQ0FBQyxTQUFTLE1BQU0sR0FBRyxJQUFILENBQVEsRUFBUixDQUFmLENBQXBELEVBQWdGLE9BQU8sR0FBUDtBQUNoRixPQUFHLFFBQVEsS0FBSyxHQUFHLE9BQWhCLEtBQTRCLFVBQTVCLElBQTBDLENBQUMsU0FBUyxNQUFNLEdBQUcsSUFBSCxDQUFRLEVBQVIsQ0FBZixDQUE5QyxFQUEwRSxPQUFPLEdBQVA7QUFDMUUsT0FBRyxDQUFDLENBQUQsSUFBTSxRQUFRLEtBQUssR0FBRyxRQUFoQixLQUE2QixVQUFuQyxJQUFpRCxDQUFDLFNBQVMsTUFBTSxHQUFHLElBQUgsQ0FBUSxFQUFSLENBQWYsQ0FBckQsRUFBaUYsT0FBTyxHQUFQO0FBQ2pGLFNBQU0sVUFBVSx5Q0FBVixDQUFOO0FBQ0QsRUFQRCxDOzs7Ozs7OztBQ0pBLFFBQU8sT0FBUCxHQUFpQixVQUFTLE1BQVQsRUFBaUIsS0FBakIsRUFBdUI7QUFDdEMsVUFBTztBQUNMLGlCQUFjLEVBQUUsU0FBUyxDQUFYLENBRFQ7QUFFTCxtQkFBYyxFQUFFLFNBQVMsQ0FBWCxDQUZUO0FBR0wsZUFBYyxFQUFFLFNBQVMsQ0FBWCxDQUhUO0FBSUwsWUFBYztBQUpULElBQVA7QUFNRCxFQVBELEM7Ozs7Ozs7O0FDQUEsS0FBSSxpQkFBaUIsR0FBRyxjQUF4QjtBQUNBLFFBQU8sT0FBUCxHQUFpQixVQUFTLEVBQVQsRUFBYSxHQUFiLEVBQWlCO0FBQ2hDLFVBQU8sZUFBZSxJQUFmLENBQW9CLEVBQXBCLEVBQXdCLEdBQXhCLENBQVA7QUFDRCxFQUZELEM7Ozs7Ozs7O0FDREEsS0FBSSxPQUFPLE9BQU8sT0FBUCxHQUFpQixFQUFDLFNBQVMsT0FBVixFQUE1QjtBQUNBLEtBQUcsT0FBTyxHQUFQLElBQWMsUUFBakIsRUFBMEIsTUFBTSxJQUFOLEMsQ0FBWSwrQjs7Ozs7O0FDRHRDOztBQUNBLEtBQUksTUFBTyxvQkFBUSxFQUFSLEVBQXdCLElBQXhCLENBQVg7O0FBRUE7QUFDQSxxQkFBUSxFQUFSLEVBQTBCLE1BQTFCLEVBQWtDLFFBQWxDLEVBQTRDLFVBQVMsUUFBVCxFQUFrQjtBQUM1RCxRQUFLLEVBQUwsR0FBVSxPQUFPLFFBQVAsQ0FBVixDQUQ0RCxDQUNoQztBQUM1QixRQUFLLEVBQUwsR0FBVSxDQUFWLENBRjRELENBRWhDO0FBQzlCO0FBQ0MsRUFKRCxFQUlHLFlBQVU7QUFDWCxPQUFJLElBQVEsS0FBSyxFQUFqQjtBQUFBLE9BQ0ksUUFBUSxLQUFLLEVBRGpCO0FBQUEsT0FFSSxLQUZKO0FBR0EsT0FBRyxTQUFTLEVBQUUsTUFBZCxFQUFxQixPQUFPLEVBQUMsT0FBTyxTQUFSLEVBQW1CLE1BQU0sSUFBekIsRUFBUDtBQUNyQixXQUFRLElBQUksQ0FBSixFQUFPLEtBQVAsQ0FBUjtBQUNBLFFBQUssRUFBTCxJQUFXLE1BQU0sTUFBakI7QUFDQSxVQUFPLEVBQUMsT0FBTyxLQUFSLEVBQWUsTUFBTSxLQUFyQixFQUFQO0FBQ0QsRUFaRCxFOzs7Ozs7OztBQ0pBLEtBQUksWUFBWSxvQkFBUSxFQUFSLENBQWhCO0FBQUEsS0FDSSxVQUFZLG9CQUFRLEVBQVIsQ0FEaEI7QUFFQTtBQUNBO0FBQ0EsUUFBTyxPQUFQLEdBQWlCLFVBQVMsU0FBVCxFQUFtQjtBQUNsQyxVQUFPLFVBQVMsSUFBVCxFQUFlLEdBQWYsRUFBbUI7QUFDeEIsU0FBSSxJQUFJLE9BQU8sUUFBUSxJQUFSLENBQVAsQ0FBUjtBQUFBLFNBQ0ksSUFBSSxVQUFVLEdBQVYsQ0FEUjtBQUFBLFNBRUksSUFBSSxFQUFFLE1BRlY7QUFBQSxTQUdJLENBSEo7QUFBQSxTQUdPLENBSFA7QUFJQSxTQUFHLElBQUksQ0FBSixJQUFTLEtBQUssQ0FBakIsRUFBbUIsT0FBTyxZQUFZLEVBQVosR0FBaUIsU0FBeEI7QUFDbkIsU0FBSSxFQUFFLFVBQUYsQ0FBYSxDQUFiLENBQUo7QUFDQSxZQUFPLElBQUksTUFBSixJQUFjLElBQUksTUFBbEIsSUFBNEIsSUFBSSxDQUFKLEtBQVUsQ0FBdEMsSUFBMkMsQ0FBQyxJQUFJLEVBQUUsVUFBRixDQUFhLElBQUksQ0FBakIsQ0FBTCxJQUE0QixNQUF2RSxJQUFpRixJQUFJLE1BQXJGLEdBQ0gsWUFBWSxFQUFFLE1BQUYsQ0FBUyxDQUFULENBQVosR0FBMEIsQ0FEdkIsR0FFSCxZQUFZLEVBQUUsS0FBRixDQUFRLENBQVIsRUFBVyxJQUFJLENBQWYsQ0FBWixHQUFnQyxDQUFDLElBQUksTUFBSixJQUFjLEVBQWYsS0FBc0IsSUFBSSxNQUExQixJQUFvQyxPQUZ4RTtBQUdELElBVkQ7QUFXRCxFQVpELEM7Ozs7Ozs7O0FDSkE7QUFDQSxLQUFJLE9BQVEsS0FBSyxJQUFqQjtBQUFBLEtBQ0ksUUFBUSxLQUFLLEtBRGpCO0FBRUEsUUFBTyxPQUFQLEdBQWlCLFVBQVMsRUFBVCxFQUFZO0FBQzNCLFVBQU8sTUFBTSxLQUFLLENBQUMsRUFBWixJQUFrQixDQUFsQixHQUFzQixDQUFDLEtBQUssQ0FBTCxHQUFTLEtBQVQsR0FBaUIsSUFBbEIsRUFBd0IsRUFBeEIsQ0FBN0I7QUFDRCxFQUZELEM7Ozs7Ozs7O0FDSEE7QUFDQSxRQUFPLE9BQVAsR0FBaUIsVUFBUyxFQUFULEVBQVk7QUFDM0IsT0FBRyxNQUFNLFNBQVQsRUFBbUIsTUFBTSxVQUFVLDJCQUEyQixFQUFyQyxDQUFOO0FBQ25CLFVBQU8sRUFBUDtBQUNELEVBSEQsQzs7Ozs7O0FDREE7O0FBQ0EsS0FBSSxVQUFpQixvQkFBUSxFQUFSLENBQXJCO0FBQUEsS0FDSSxVQUFpQixvQkFBUSxFQUFSLENBRHJCO0FBQUEsS0FFSSxXQUFpQixvQkFBUSxFQUFSLENBRnJCO0FBQUEsS0FHSSxPQUFpQixvQkFBUSxFQUFSLENBSHJCO0FBQUEsS0FJSSxNQUFpQixvQkFBUSxFQUFSLENBSnJCO0FBQUEsS0FLSSxZQUFpQixvQkFBUSxFQUFSLENBTHJCO0FBQUEsS0FNSSxjQUFpQixvQkFBUSxFQUFSLENBTnJCO0FBQUEsS0FPSSxpQkFBaUIsb0JBQVEsRUFBUixDQVByQjtBQUFBLEtBUUksaUJBQWlCLG9CQUFRLEVBQVIsQ0FSckI7QUFBQSxLQVNJLFdBQWlCLG9CQUFRLENBQVIsRUFBa0IsVUFBbEIsQ0FUckI7QUFBQSxLQVVJLFFBQWlCLEVBQUUsR0FBRyxJQUFILElBQVcsVUFBVSxHQUFHLElBQUgsRUFBdkIsQ0FWckIsQ0FVdUQ7QUFWdkQ7QUFBQSxLQVdJLGNBQWlCLFlBWHJCO0FBQUEsS0FZSSxPQUFpQixNQVpyQjtBQUFBLEtBYUksU0FBaUIsUUFickI7O0FBZUEsS0FBSSxhQUFhLFNBQWIsVUFBYSxHQUFVO0FBQUUsVUFBTyxJQUFQO0FBQWMsRUFBM0M7O0FBRUEsUUFBTyxPQUFQLEdBQWlCLFVBQVMsSUFBVCxFQUFlLElBQWYsRUFBcUIsV0FBckIsRUFBa0MsSUFBbEMsRUFBd0MsT0FBeEMsRUFBaUQsTUFBakQsRUFBeUQsTUFBekQsRUFBZ0U7QUFDL0UsZUFBWSxXQUFaLEVBQXlCLElBQXpCLEVBQStCLElBQS9CO0FBQ0EsT0FBSSxZQUFZLFNBQVosU0FBWSxDQUFTLElBQVQsRUFBYztBQUM1QixTQUFHLENBQUMsS0FBRCxJQUFVLFFBQVEsS0FBckIsRUFBMkIsT0FBTyxNQUFNLElBQU4sQ0FBUDtBQUMzQixhQUFPLElBQVA7QUFDRSxZQUFLLElBQUw7QUFBVyxnQkFBTyxTQUFTLElBQVQsR0FBZTtBQUFFLGtCQUFPLElBQUksV0FBSixDQUFnQixJQUFoQixFQUFzQixJQUF0QixDQUFQO0FBQXFDLFVBQTdEO0FBQ1gsWUFBSyxNQUFMO0FBQWEsZ0JBQU8sU0FBUyxNQUFULEdBQWlCO0FBQUUsa0JBQU8sSUFBSSxXQUFKLENBQWdCLElBQWhCLEVBQXNCLElBQXRCLENBQVA7QUFBcUMsVUFBL0Q7QUFGZixNQUdFLE9BQU8sU0FBUyxPQUFULEdBQWtCO0FBQUUsY0FBTyxJQUFJLFdBQUosQ0FBZ0IsSUFBaEIsRUFBc0IsSUFBdEIsQ0FBUDtBQUFxQyxNQUFoRTtBQUNILElBTkQ7QUFPQSxPQUFJLE1BQWEsT0FBTyxXQUF4QjtBQUFBLE9BQ0ksYUFBYSxXQUFXLE1BRDVCO0FBQUEsT0FFSSxhQUFhLEtBRmpCO0FBQUEsT0FHSSxRQUFhLEtBQUssU0FIdEI7QUFBQSxPQUlJLFVBQWEsTUFBTSxRQUFOLEtBQW1CLE1BQU0sV0FBTixDQUFuQixJQUF5QyxXQUFXLE1BQU0sT0FBTixDQUpyRTtBQUFBLE9BS0ksV0FBYSxXQUFXLFVBQVUsT0FBVixDQUw1QjtBQUFBLE9BTUksV0FBYSxVQUFVLENBQUMsVUFBRCxHQUFjLFFBQWQsR0FBeUIsVUFBVSxTQUFWLENBQW5DLEdBQTBELFNBTjNFO0FBQUEsT0FPSSxhQUFhLFFBQVEsT0FBUixHQUFrQixNQUFNLE9BQU4sSUFBaUIsT0FBbkMsR0FBNkMsT0FQOUQ7QUFBQSxPQVFJLE9BUko7QUFBQSxPQVFhLEdBUmI7QUFBQSxPQVFrQixpQkFSbEI7QUFTQTtBQUNBLE9BQUcsVUFBSCxFQUFjO0FBQ1oseUJBQW9CLGVBQWUsV0FBVyxJQUFYLENBQWdCLElBQUksSUFBSixFQUFoQixDQUFmLENBQXBCO0FBQ0EsU0FBRyxzQkFBc0IsT0FBTyxTQUFoQyxFQUEwQztBQUN4QztBQUNBLHNCQUFlLGlCQUFmLEVBQWtDLEdBQWxDLEVBQXVDLElBQXZDO0FBQ0E7QUFDQSxXQUFHLENBQUMsT0FBRCxJQUFZLENBQUMsSUFBSSxpQkFBSixFQUF1QixRQUF2QixDQUFoQixFQUFpRCxLQUFLLGlCQUFMLEVBQXdCLFFBQXhCLEVBQWtDLFVBQWxDO0FBQ2xEO0FBQ0Y7QUFDRDtBQUNBLE9BQUcsY0FBYyxPQUFkLElBQXlCLFFBQVEsSUFBUixLQUFpQixNQUE3QyxFQUFvRDtBQUNsRCxrQkFBYSxJQUFiO0FBQ0EsZ0JBQVcsU0FBUyxNQUFULEdBQWlCO0FBQUUsY0FBTyxRQUFRLElBQVIsQ0FBYSxJQUFiLENBQVA7QUFBNEIsTUFBMUQ7QUFDRDtBQUNEO0FBQ0EsT0FBRyxDQUFDLENBQUMsT0FBRCxJQUFZLE1BQWIsTUFBeUIsU0FBUyxVQUFULElBQXVCLENBQUMsTUFBTSxRQUFOLENBQWpELENBQUgsRUFBcUU7QUFDbkUsVUFBSyxLQUFMLEVBQVksUUFBWixFQUFzQixRQUF0QjtBQUNEO0FBQ0Q7QUFDQSxhQUFVLElBQVYsSUFBa0IsUUFBbEI7QUFDQSxhQUFVLEdBQVYsSUFBa0IsVUFBbEI7QUFDQSxPQUFHLE9BQUgsRUFBVztBQUNULGVBQVU7QUFDUixlQUFTLGFBQWEsUUFBYixHQUF3QixVQUFVLE1BQVYsQ0FEekI7QUFFUixhQUFTLFNBQWEsUUFBYixHQUF3QixVQUFVLElBQVYsQ0FGekI7QUFHUixnQkFBUztBQUhELE1BQVY7QUFLQSxTQUFHLE1BQUgsRUFBVSxLQUFJLEdBQUosSUFBVyxPQUFYLEVBQW1CO0FBQzNCLFdBQUcsRUFBRSxPQUFPLEtBQVQsQ0FBSCxFQUFtQixTQUFTLEtBQVQsRUFBZ0IsR0FBaEIsRUFBcUIsUUFBUSxHQUFSLENBQXJCO0FBQ3BCLE1BRkQsTUFFTyxRQUFRLFFBQVEsQ0FBUixHQUFZLFFBQVEsQ0FBUixJQUFhLFNBQVMsVUFBdEIsQ0FBcEIsRUFBdUQsSUFBdkQsRUFBNkQsT0FBN0Q7QUFDUjtBQUNELFVBQU8sT0FBUDtBQUNELEVBbkRELEM7Ozs7Ozs7O0FDbEJBLFFBQU8sT0FBUCxHQUFpQixLQUFqQixDOzs7Ozs7OztBQ0FBLEtBQUksU0FBWSxvQkFBUSxFQUFSLENBQWhCO0FBQUEsS0FDSSxPQUFZLG9CQUFRLEVBQVIsQ0FEaEI7QUFBQSxLQUVJLE9BQVksb0JBQVEsRUFBUixDQUZoQjtBQUFBLEtBR0ksV0FBWSxvQkFBUSxFQUFSLENBSGhCO0FBQUEsS0FJSSxNQUFZLG9CQUFRLEVBQVIsQ0FKaEI7QUFBQSxLQUtJLFlBQVksV0FMaEI7O0FBT0EsS0FBSSxVQUFVLFNBQVYsT0FBVSxDQUFTLElBQVQsRUFBZSxJQUFmLEVBQXFCLE1BQXJCLEVBQTRCO0FBQ3hDLE9BQUksWUFBWSxPQUFPLFFBQVEsQ0FBL0I7QUFBQSxPQUNJLFlBQVksT0FBTyxRQUFRLENBRC9CO0FBQUEsT0FFSSxZQUFZLE9BQU8sUUFBUSxDQUYvQjtBQUFBLE9BR0ksV0FBWSxPQUFPLFFBQVEsQ0FIL0I7QUFBQSxPQUlJLFVBQVksT0FBTyxRQUFRLENBSi9CO0FBQUEsT0FLSSxTQUFZLFlBQVksTUFBWixHQUFxQixZQUFZLE9BQU8sSUFBUCxNQUFpQixPQUFPLElBQVAsSUFBZSxFQUFoQyxDQUFaLEdBQWtELENBQUMsT0FBTyxJQUFQLEtBQWdCLEVBQWpCLEVBQXFCLFNBQXJCLENBTHZGO0FBQUEsT0FNSSxVQUFZLFlBQVksSUFBWixHQUFtQixLQUFLLElBQUwsTUFBZSxLQUFLLElBQUwsSUFBYSxFQUE1QixDQU5uQztBQUFBLE9BT0ksV0FBWSxRQUFRLFNBQVIsTUFBdUIsUUFBUSxTQUFSLElBQXFCLEVBQTVDLENBUGhCO0FBQUEsT0FRSSxHQVJKO0FBQUEsT0FRUyxHQVJUO0FBQUEsT0FRYyxHQVJkO0FBQUEsT0FRbUIsR0FSbkI7QUFTQSxPQUFHLFNBQUgsRUFBYSxTQUFTLElBQVQ7QUFDYixRQUFJLEdBQUosSUFBVyxNQUFYLEVBQWtCO0FBQ2hCO0FBQ0EsV0FBTSxDQUFDLFNBQUQsSUFBYyxNQUFkLElBQXdCLE9BQU8sR0FBUCxNQUFnQixTQUE5QztBQUNBO0FBQ0EsV0FBTSxDQUFDLE1BQU0sTUFBTixHQUFlLE1BQWhCLEVBQXdCLEdBQXhCLENBQU47QUFDQTtBQUNBLFdBQU0sV0FBVyxHQUFYLEdBQWlCLElBQUksR0FBSixFQUFTLE1BQVQsQ0FBakIsR0FBb0MsWUFBWSxPQUFPLEdBQVAsSUFBYyxVQUExQixHQUF1QyxJQUFJLFNBQVMsSUFBYixFQUFtQixHQUFuQixDQUF2QyxHQUFpRSxHQUEzRztBQUNBO0FBQ0EsU0FBRyxNQUFILEVBQVUsU0FBUyxNQUFULEVBQWlCLEdBQWpCLEVBQXNCLEdBQXRCLEVBQTJCLE9BQU8sUUFBUSxDQUExQztBQUNWO0FBQ0EsU0FBRyxRQUFRLEdBQVIsS0FBZ0IsR0FBbkIsRUFBdUIsS0FBSyxPQUFMLEVBQWMsR0FBZCxFQUFtQixHQUFuQjtBQUN2QixTQUFHLFlBQVksU0FBUyxHQUFULEtBQWlCLEdBQWhDLEVBQW9DLFNBQVMsR0FBVCxJQUFnQixHQUFoQjtBQUNyQztBQUNGLEVBeEJEO0FBeUJBLFFBQU8sSUFBUCxHQUFjLElBQWQ7QUFDQTtBQUNBLFNBQVEsQ0FBUixHQUFZLENBQVosQyxDQUFpQjtBQUNqQixTQUFRLENBQVIsR0FBWSxDQUFaLEMsQ0FBaUI7QUFDakIsU0FBUSxDQUFSLEdBQVksQ0FBWixDLENBQWlCO0FBQ2pCLFNBQVEsQ0FBUixHQUFZLENBQVosQyxDQUFpQjtBQUNqQixTQUFRLENBQVIsR0FBWSxFQUFaLEMsQ0FBaUI7QUFDakIsU0FBUSxDQUFSLEdBQVksRUFBWixDLENBQWlCO0FBQ2pCLFNBQVEsQ0FBUixHQUFZLEVBQVosQyxDQUFpQjtBQUNqQixTQUFRLENBQVIsR0FBWSxHQUFaLEMsQ0FBaUI7QUFDakIsUUFBTyxPQUFQLEdBQWlCLE9BQWpCLEM7Ozs7Ozs7O0FDMUNBO0FBQ0EsS0FBSSxZQUFZLG9CQUFRLEVBQVIsQ0FBaEI7QUFDQSxRQUFPLE9BQVAsR0FBaUIsVUFBUyxFQUFULEVBQWEsSUFBYixFQUFtQixNQUFuQixFQUEwQjtBQUN6QyxhQUFVLEVBQVY7QUFDQSxPQUFHLFNBQVMsU0FBWixFQUFzQixPQUFPLEVBQVA7QUFDdEIsV0FBTyxNQUFQO0FBQ0UsVUFBSyxDQUFMO0FBQVEsY0FBTyxVQUFTLENBQVQsRUFBVztBQUN4QixnQkFBTyxHQUFHLElBQUgsQ0FBUSxJQUFSLEVBQWMsQ0FBZCxDQUFQO0FBQ0QsUUFGTztBQUdSLFVBQUssQ0FBTDtBQUFRLGNBQU8sVUFBUyxDQUFULEVBQVksQ0FBWixFQUFjO0FBQzNCLGdCQUFPLEdBQUcsSUFBSCxDQUFRLElBQVIsRUFBYyxDQUFkLEVBQWlCLENBQWpCLENBQVA7QUFDRCxRQUZPO0FBR1IsVUFBSyxDQUFMO0FBQVEsY0FBTyxVQUFTLENBQVQsRUFBWSxDQUFaLEVBQWUsQ0FBZixFQUFpQjtBQUM5QixnQkFBTyxHQUFHLElBQUgsQ0FBUSxJQUFSLEVBQWMsQ0FBZCxFQUFpQixDQUFqQixFQUFvQixDQUFwQixDQUFQO0FBQ0QsUUFGTztBQVBWO0FBV0EsVUFBTyxZQUFTLGFBQWM7QUFDNUIsWUFBTyxHQUFHLEtBQUgsQ0FBUyxJQUFULEVBQWUsU0FBZixDQUFQO0FBQ0QsSUFGRDtBQUdELEVBakJELEM7Ozs7Ozs7O0FDRkEsUUFBTyxPQUFQLEdBQWlCLFVBQVMsRUFBVCxFQUFZO0FBQzNCLE9BQUcsT0FBTyxFQUFQLElBQWEsVUFBaEIsRUFBMkIsTUFBTSxVQUFVLEtBQUsscUJBQWYsQ0FBTjtBQUMzQixVQUFPLEVBQVA7QUFDRCxFQUhELEM7Ozs7Ozs7O0FDQUEsUUFBTyxPQUFQLEdBQWlCLEVBQWpCLEM7Ozs7OztBQ0FBOztBQUNBLEtBQUksU0FBaUIsb0JBQVEsRUFBUixDQUFyQjtBQUFBLEtBQ0ksYUFBaUIsb0JBQVEsRUFBUixDQURyQjtBQUFBLEtBRUksaUJBQWlCLG9CQUFRLEVBQVIsQ0FGckI7QUFBQSxLQUdJLG9CQUFvQixFQUh4Qjs7QUFLQTtBQUNBLHFCQUFRLEVBQVIsRUFBbUIsaUJBQW5CLEVBQXNDLG9CQUFRLENBQVIsRUFBa0IsVUFBbEIsQ0FBdEMsRUFBcUUsWUFBVTtBQUFFLFVBQU8sSUFBUDtBQUFjLEVBQS9GOztBQUVBLFFBQU8sT0FBUCxHQUFpQixVQUFTLFdBQVQsRUFBc0IsSUFBdEIsRUFBNEIsSUFBNUIsRUFBaUM7QUFDaEQsZUFBWSxTQUFaLEdBQXdCLE9BQU8saUJBQVAsRUFBMEIsRUFBQyxNQUFNLFdBQVcsQ0FBWCxFQUFjLElBQWQsQ0FBUCxFQUExQixDQUF4QjtBQUNBLGtCQUFlLFdBQWYsRUFBNEIsT0FBTyxXQUFuQztBQUNELEVBSEQsQzs7Ozs7Ozs7QUNUQTtBQUNBLEtBQUksV0FBYyxvQkFBUSxFQUFSLENBQWxCO0FBQUEsS0FDSSxNQUFjLG9CQUFRLEVBQVIsQ0FEbEI7QUFBQSxLQUVJLGNBQWMsb0JBQVEsRUFBUixDQUZsQjtBQUFBLEtBR0ksV0FBYyxvQkFBUSxFQUFSLEVBQXlCLFVBQXpCLENBSGxCO0FBQUEsS0FJSSxRQUFjLFNBQWQsS0FBYyxHQUFVLENBQUUsV0FBYSxDQUozQztBQUFBLEtBS0ksWUFBYyxXQUxsQjs7QUFPQTtBQUNBLEtBQUksY0FBYSxzQkFBVTtBQUN6QjtBQUNBLE9BQUksU0FBUyxvQkFBUSxFQUFSLEVBQXlCLFFBQXpCLENBQWI7QUFBQSxPQUNJLElBQVMsWUFBWSxNQUR6QjtBQUFBLE9BRUksS0FBUyxHQUZiO0FBQUEsT0FHSSxLQUFTLEdBSGI7QUFBQSxPQUlJLGNBSko7QUFLQSxVQUFPLEtBQVAsQ0FBYSxPQUFiLEdBQXVCLE1BQXZCO0FBQ0EsdUJBQVEsRUFBUixFQUFtQixXQUFuQixDQUErQixNQUEvQjtBQUNBLFVBQU8sR0FBUCxHQUFhLGFBQWIsQ0FUeUIsQ0FTRztBQUM1QjtBQUNBO0FBQ0Esb0JBQWlCLE9BQU8sYUFBUCxDQUFxQixRQUF0QztBQUNBLGtCQUFlLElBQWY7QUFDQSxrQkFBZSxLQUFmLENBQXFCLEtBQUssUUFBTCxHQUFnQixFQUFoQixHQUFxQixtQkFBckIsR0FBMkMsRUFBM0MsR0FBZ0QsU0FBaEQsR0FBNEQsRUFBakY7QUFDQSxrQkFBZSxLQUFmO0FBQ0EsaUJBQWEsZUFBZSxDQUE1QjtBQUNBLFVBQU0sR0FBTjtBQUFVLFlBQU8sWUFBVyxTQUFYLEVBQXNCLFlBQVksQ0FBWixDQUF0QixDQUFQO0FBQVYsSUFDQSxPQUFPLGFBQVA7QUFDRCxFQW5CRDs7QUFxQkEsUUFBTyxPQUFQLEdBQWlCLE9BQU8sTUFBUCxJQUFpQixTQUFTLE1BQVQsQ0FBZ0IsQ0FBaEIsRUFBbUIsVUFBbkIsRUFBOEI7QUFDOUQsT0FBSSxNQUFKO0FBQ0EsT0FBRyxNQUFNLElBQVQsRUFBYztBQUNaLFdBQU0sU0FBTixJQUFtQixTQUFTLENBQVQsQ0FBbkI7QUFDQSxjQUFTLElBQUksS0FBSixFQUFUO0FBQ0EsV0FBTSxTQUFOLElBQW1CLElBQW5CO0FBQ0E7QUFDQSxZQUFPLFFBQVAsSUFBbUIsQ0FBbkI7QUFDRCxJQU5ELE1BTU8sU0FBUyxhQUFUO0FBQ1AsVUFBTyxlQUFlLFNBQWYsR0FBMkIsTUFBM0IsR0FBb0MsSUFBSSxNQUFKLEVBQVksVUFBWixDQUEzQztBQUNELEVBVkQsQzs7Ozs7Ozs7QUM5QkEsS0FBSSxLQUFXLG9CQUFRLEVBQVIsQ0FBZjtBQUFBLEtBQ0ksV0FBVyxvQkFBUSxFQUFSLENBRGY7QUFBQSxLQUVJLFVBQVcsb0JBQVEsRUFBUixDQUZmOztBQUlBLFFBQU8sT0FBUCxHQUFpQixvQkFBUSxFQUFSLElBQTRCLE9BQU8sZ0JBQW5DLEdBQXNELFNBQVMsZ0JBQVQsQ0FBMEIsQ0FBMUIsRUFBNkIsVUFBN0IsRUFBd0M7QUFDN0csWUFBUyxDQUFUO0FBQ0EsT0FBSSxPQUFTLFFBQVEsVUFBUixDQUFiO0FBQUEsT0FDSSxTQUFTLEtBQUssTUFEbEI7QUFBQSxPQUVJLElBQUksQ0FGUjtBQUFBLE9BR0ksQ0FISjtBQUlBLFVBQU0sU0FBUyxDQUFmO0FBQWlCLFFBQUcsQ0FBSCxDQUFLLENBQUwsRUFBUSxJQUFJLEtBQUssR0FBTCxDQUFaLEVBQXVCLFdBQVcsQ0FBWCxDQUF2QjtBQUFqQixJQUNBLE9BQU8sQ0FBUDtBQUNELEVBUkQsQzs7Ozs7Ozs7QUNKQTtBQUNBLEtBQUksUUFBYyxvQkFBUSxFQUFSLENBQWxCO0FBQUEsS0FDSSxjQUFjLG9CQUFRLEVBQVIsQ0FEbEI7O0FBR0EsUUFBTyxPQUFQLEdBQWlCLE9BQU8sSUFBUCxJQUFlLFNBQVMsSUFBVCxDQUFjLENBQWQsRUFBZ0I7QUFDOUMsVUFBTyxNQUFNLENBQU4sRUFBUyxXQUFULENBQVA7QUFDRCxFQUZELEM7Ozs7Ozs7O0FDSkEsS0FBSSxNQUFlLG9CQUFRLEVBQVIsQ0FBbkI7QUFBQSxLQUNJLFlBQWUsb0JBQVEsRUFBUixDQURuQjtBQUFBLEtBRUksZUFBZSxvQkFBUSxFQUFSLEVBQTZCLEtBQTdCLENBRm5CO0FBQUEsS0FHSSxXQUFlLG9CQUFRLEVBQVIsRUFBeUIsVUFBekIsQ0FIbkI7O0FBS0EsUUFBTyxPQUFQLEdBQWlCLFVBQVMsTUFBVCxFQUFpQixLQUFqQixFQUF1QjtBQUN0QyxPQUFJLElBQVMsVUFBVSxNQUFWLENBQWI7QUFBQSxPQUNJLElBQVMsQ0FEYjtBQUFBLE9BRUksU0FBUyxFQUZiO0FBQUEsT0FHSSxHQUhKO0FBSUEsUUFBSSxHQUFKLElBQVcsQ0FBWDtBQUFhLFNBQUcsT0FBTyxRQUFWLEVBQW1CLElBQUksQ0FBSixFQUFPLEdBQVAsS0FBZSxPQUFPLElBQVAsQ0FBWSxHQUFaLENBQWY7QUFBaEMsSUFMc0MsQ0FNdEM7QUFDQSxVQUFNLE1BQU0sTUFBTixHQUFlLENBQXJCO0FBQXVCLFNBQUcsSUFBSSxDQUFKLEVBQU8sTUFBTSxNQUFNLEdBQU4sQ0FBYixDQUFILEVBQTRCO0FBQ2pELFFBQUMsYUFBYSxNQUFiLEVBQXFCLEdBQXJCLENBQUQsSUFBOEIsT0FBTyxJQUFQLENBQVksR0FBWixDQUE5QjtBQUNEO0FBRkQsSUFHQSxPQUFPLE1BQVA7QUFDRCxFQVhELEM7Ozs7Ozs7O0FDTEE7QUFDQSxLQUFJLFVBQVUsb0JBQVEsRUFBUixDQUFkO0FBQUEsS0FDSSxVQUFVLG9CQUFRLEVBQVIsQ0FEZDtBQUVBLFFBQU8sT0FBUCxHQUFpQixVQUFTLEVBQVQsRUFBWTtBQUMzQixVQUFPLFFBQVEsUUFBUSxFQUFSLENBQVIsQ0FBUDtBQUNELEVBRkQsQzs7Ozs7Ozs7QUNIQTtBQUNBLEtBQUksTUFBTSxvQkFBUSxDQUFSLENBQVY7QUFDQSxRQUFPLE9BQVAsR0FBaUIsT0FBTyxHQUFQLEVBQVksb0JBQVosQ0FBaUMsQ0FBakMsSUFBc0MsTUFBdEMsR0FBK0MsVUFBUyxFQUFULEVBQVk7QUFDMUUsVUFBTyxJQUFJLEVBQUosS0FBVyxRQUFYLEdBQXNCLEdBQUcsS0FBSCxDQUFTLEVBQVQsQ0FBdEIsR0FBcUMsT0FBTyxFQUFQLENBQTVDO0FBQ0QsRUFGRCxDOzs7Ozs7OztBQ0ZBO0FBQ0E7QUFDQSxLQUFJLFlBQVksb0JBQVEsRUFBUixDQUFoQjtBQUFBLEtBQ0ksV0FBWSxvQkFBUSxFQUFSLENBRGhCO0FBQUEsS0FFSSxVQUFZLG9CQUFRLEVBQVIsQ0FGaEI7QUFHQSxRQUFPLE9BQVAsR0FBaUIsVUFBUyxXQUFULEVBQXFCO0FBQ3BDLFVBQU8sVUFBUyxLQUFULEVBQWdCLEVBQWhCLEVBQW9CLFNBQXBCLEVBQThCO0FBQ25DLFNBQUksSUFBUyxVQUFVLEtBQVYsQ0FBYjtBQUFBLFNBQ0ksU0FBUyxTQUFTLEVBQUUsTUFBWCxDQURiO0FBQUEsU0FFSSxRQUFTLFFBQVEsU0FBUixFQUFtQixNQUFuQixDQUZiO0FBQUEsU0FHSSxLQUhKO0FBSUE7QUFDQSxTQUFHLGVBQWUsTUFBTSxFQUF4QixFQUEyQixPQUFNLFNBQVMsS0FBZixFQUFxQjtBQUM5QyxlQUFRLEVBQUUsT0FBRixDQUFSO0FBQ0EsV0FBRyxTQUFTLEtBQVosRUFBa0IsT0FBTyxJQUFQO0FBQ3BCO0FBQ0MsTUFKRCxNQUlPLE9BQUssU0FBUyxLQUFkLEVBQXFCLE9BQXJCO0FBQTZCLFdBQUcsZUFBZSxTQUFTLENBQTNCLEVBQTZCO0FBQy9ELGFBQUcsRUFBRSxLQUFGLE1BQWEsRUFBaEIsRUFBbUIsT0FBTyxlQUFlLEtBQWYsSUFBd0IsQ0FBL0I7QUFDcEI7QUFGTSxNQUVMLE9BQU8sQ0FBQyxXQUFELElBQWdCLENBQUMsQ0FBeEI7QUFDSCxJQWJEO0FBY0QsRUFmRCxDOzs7Ozs7OztBQ0xBO0FBQ0EsS0FBSSxZQUFZLG9CQUFRLEVBQVIsQ0FBaEI7QUFBQSxLQUNJLE1BQVksS0FBSyxHQURyQjtBQUVBLFFBQU8sT0FBUCxHQUFpQixVQUFTLEVBQVQsRUFBWTtBQUMzQixVQUFPLEtBQUssQ0FBTCxHQUFTLElBQUksVUFBVSxFQUFWLENBQUosRUFBbUIsZ0JBQW5CLENBQVQsR0FBZ0QsQ0FBdkQsQ0FEMkIsQ0FDK0I7QUFDM0QsRUFGRCxDOzs7Ozs7OztBQ0hBLEtBQUksWUFBWSxvQkFBUSxFQUFSLENBQWhCO0FBQUEsS0FDSSxNQUFZLEtBQUssR0FEckI7QUFBQSxLQUVJLE1BQVksS0FBSyxHQUZyQjtBQUdBLFFBQU8sT0FBUCxHQUFpQixVQUFTLEtBQVQsRUFBZ0IsTUFBaEIsRUFBdUI7QUFDdEMsV0FBUSxVQUFVLEtBQVYsQ0FBUjtBQUNBLFVBQU8sUUFBUSxDQUFSLEdBQVksSUFBSSxRQUFRLE1BQVosRUFBb0IsQ0FBcEIsQ0FBWixHQUFxQyxJQUFJLEtBQUosRUFBVyxNQUFYLENBQTVDO0FBQ0QsRUFIRCxDOzs7Ozs7OztBQ0hBLEtBQUksU0FBUyxvQkFBUSxFQUFSLEVBQXFCLE1BQXJCLENBQWI7QUFBQSxLQUNJLE1BQVMsb0JBQVEsRUFBUixDQURiO0FBRUEsUUFBTyxPQUFQLEdBQWlCLFVBQVMsR0FBVCxFQUFhO0FBQzVCLFVBQU8sT0FBTyxHQUFQLE1BQWdCLE9BQU8sR0FBUCxJQUFjLElBQUksR0FBSixDQUE5QixDQUFQO0FBQ0QsRUFGRCxDOzs7Ozs7OztBQ0ZBO0FBQ0EsUUFBTyxPQUFQLEdBQ0UsK0ZBRGUsQ0FFZixLQUZlLENBRVQsR0FGUyxDQUFqQixDOzs7Ozs7OztBQ0RBLFFBQU8sT0FBUCxHQUFpQixvQkFBUSxFQUFSLEVBQXFCLFFBQXJCLElBQWlDLFNBQVMsZUFBM0QsQzs7Ozs7Ozs7QUNBQSxLQUFJLE1BQU0sb0JBQVEsRUFBUixFQUF3QixDQUFsQztBQUFBLEtBQ0ksTUFBTSxvQkFBUSxFQUFSLENBRFY7QUFBQSxLQUVJLE1BQU0sb0JBQVEsQ0FBUixFQUFrQixhQUFsQixDQUZWOztBQUlBLFFBQU8sT0FBUCxHQUFpQixVQUFTLEVBQVQsRUFBYSxHQUFiLEVBQWtCLElBQWxCLEVBQXVCO0FBQ3RDLE9BQUcsTUFBTSxDQUFDLElBQUksS0FBSyxPQUFPLEVBQVAsR0FBWSxHQUFHLFNBQXhCLEVBQW1DLEdBQW5DLENBQVYsRUFBa0QsSUFBSSxFQUFKLEVBQVEsR0FBUixFQUFhLEVBQUMsY0FBYyxJQUFmLEVBQXFCLE9BQU8sR0FBNUIsRUFBYjtBQUNuRCxFQUZELEM7Ozs7Ozs7O0FDSkE7QUFDQSxLQUFJLE1BQWMsb0JBQVEsRUFBUixDQUFsQjtBQUFBLEtBQ0ksV0FBYyxvQkFBUSxFQUFSLENBRGxCO0FBQUEsS0FFSSxXQUFjLG9CQUFRLEVBQVIsRUFBeUIsVUFBekIsQ0FGbEI7QUFBQSxLQUdJLGNBQWMsT0FBTyxTQUh6Qjs7QUFLQSxRQUFPLE9BQVAsR0FBaUIsT0FBTyxjQUFQLElBQXlCLFVBQVMsQ0FBVCxFQUFXO0FBQ25ELE9BQUksU0FBUyxDQUFULENBQUo7QUFDQSxPQUFHLElBQUksQ0FBSixFQUFPLFFBQVAsQ0FBSCxFQUFvQixPQUFPLEVBQUUsUUFBRixDQUFQO0FBQ3BCLE9BQUcsT0FBTyxFQUFFLFdBQVQsSUFBd0IsVUFBeEIsSUFBc0MsYUFBYSxFQUFFLFdBQXhELEVBQW9FO0FBQ2xFLFlBQU8sRUFBRSxXQUFGLENBQWMsU0FBckI7QUFDRCxJQUFDLE9BQU8sYUFBYSxNQUFiLEdBQXNCLFdBQXRCLEdBQW9DLElBQTNDO0FBQ0gsRUFORCxDOzs7Ozs7OztBQ05BO0FBQ0EsS0FBSSxVQUFVLG9CQUFRLEVBQVIsQ0FBZDtBQUNBLFFBQU8sT0FBUCxHQUFpQixVQUFTLEVBQVQsRUFBWTtBQUMzQixVQUFPLE9BQU8sUUFBUSxFQUFSLENBQVAsQ0FBUDtBQUNELEVBRkQsQzs7Ozs7Ozs7QUNGQSxLQUFJLGFBQWdCLG9CQUFRLEVBQVIsQ0FBcEI7QUFBQSxLQUNJLFdBQWdCLG9CQUFRLEVBQVIsQ0FEcEI7QUFBQSxLQUVJLFNBQWdCLG9CQUFRLEVBQVIsQ0FGcEI7QUFBQSxLQUdJLE9BQWdCLG9CQUFRLEVBQVIsQ0FIcEI7QUFBQSxLQUlJLFlBQWdCLG9CQUFRLEVBQVIsQ0FKcEI7QUFBQSxLQUtJLE1BQWdCLG9CQUFRLENBQVIsQ0FMcEI7QUFBQSxLQU1JLFdBQWdCLElBQUksVUFBSixDQU5wQjtBQUFBLEtBT0ksZ0JBQWdCLElBQUksYUFBSixDQVBwQjtBQUFBLEtBUUksY0FBZ0IsVUFBVSxLQVI5Qjs7QUFVQSxNQUFJLElBQUksY0FBYyxDQUFDLFVBQUQsRUFBYSxjQUFiLEVBQTZCLFdBQTdCLEVBQTBDLGdCQUExQyxFQUE0RCxhQUE1RCxDQUFsQixFQUE4RixJQUFJLENBQXRHLEVBQXlHLElBQUksQ0FBN0csRUFBZ0gsR0FBaEgsRUFBb0g7QUFDbEgsT0FBSSxPQUFhLFlBQVksQ0FBWixDQUFqQjtBQUFBLE9BQ0ksYUFBYSxPQUFPLElBQVAsQ0FEakI7QUFBQSxPQUVJLFFBQWEsY0FBYyxXQUFXLFNBRjFDO0FBQUEsT0FHSSxHQUhKO0FBSUEsT0FBRyxLQUFILEVBQVM7QUFDUCxTQUFHLENBQUMsTUFBTSxRQUFOLENBQUosRUFBb0IsS0FBSyxLQUFMLEVBQVksUUFBWixFQUFzQixXQUF0QjtBQUNwQixTQUFHLENBQUMsTUFBTSxhQUFOLENBQUosRUFBeUIsS0FBSyxLQUFMLEVBQVksYUFBWixFQUEyQixJQUEzQjtBQUN6QixlQUFVLElBQVYsSUFBa0IsV0FBbEI7QUFDQSxVQUFJLEdBQUosSUFBVyxVQUFYO0FBQXNCLFdBQUcsQ0FBQyxNQUFNLEdBQU4sQ0FBSixFQUFlLFNBQVMsS0FBVCxFQUFnQixHQUFoQixFQUFxQixXQUFXLEdBQVgsQ0FBckIsRUFBc0MsSUFBdEM7QUFBckM7QUFDRDtBQUNGLEU7Ozs7OztBQ3JCRDs7QUFDQSxLQUFJLG1CQUFtQixvQkFBUSxFQUFSLENBQXZCO0FBQUEsS0FDSSxPQUFtQixvQkFBUSxFQUFSLENBRHZCO0FBQUEsS0FFSSxZQUFtQixvQkFBUSxFQUFSLENBRnZCO0FBQUEsS0FHSSxZQUFtQixvQkFBUSxFQUFSLENBSHZCOztBQUtBO0FBQ0E7QUFDQTtBQUNBO0FBQ0EsUUFBTyxPQUFQLEdBQWlCLG9CQUFRLEVBQVIsRUFBMEIsS0FBMUIsRUFBaUMsT0FBakMsRUFBMEMsVUFBUyxRQUFULEVBQW1CLElBQW5CLEVBQXdCO0FBQ2pGLFFBQUssRUFBTCxHQUFVLFVBQVUsUUFBVixDQUFWLENBRGlGLENBQ2xEO0FBQy9CLFFBQUssRUFBTCxHQUFVLENBQVYsQ0FGaUYsQ0FFbEQ7QUFDL0IsUUFBSyxFQUFMLEdBQVUsSUFBVixDQUhpRixDQUdsRDtBQUNqQztBQUNDLEVBTGdCLEVBS2QsWUFBVTtBQUNYLE9BQUksSUFBUSxLQUFLLEVBQWpCO0FBQUEsT0FDSSxPQUFRLEtBQUssRUFEakI7QUFBQSxPQUVJLFFBQVEsS0FBSyxFQUFMLEVBRlo7QUFHQSxPQUFHLENBQUMsQ0FBRCxJQUFNLFNBQVMsRUFBRSxNQUFwQixFQUEyQjtBQUN6QixVQUFLLEVBQUwsR0FBVSxTQUFWO0FBQ0EsWUFBTyxLQUFLLENBQUwsQ0FBUDtBQUNEO0FBQ0QsT0FBRyxRQUFRLE1BQVgsRUFBb0IsT0FBTyxLQUFLLENBQUwsRUFBUSxLQUFSLENBQVA7QUFDcEIsT0FBRyxRQUFRLFFBQVgsRUFBb0IsT0FBTyxLQUFLLENBQUwsRUFBUSxFQUFFLEtBQUYsQ0FBUixDQUFQO0FBQ3BCLFVBQU8sS0FBSyxDQUFMLEVBQVEsQ0FBQyxLQUFELEVBQVEsRUFBRSxLQUFGLENBQVIsQ0FBUixDQUFQO0FBQ0QsRUFoQmdCLEVBZ0JkLFFBaEJjLENBQWpCOztBQWtCQTtBQUNBLFdBQVUsU0FBVixHQUFzQixVQUFVLEtBQWhDOztBQUVBLGtCQUFpQixNQUFqQjtBQUNBLGtCQUFpQixRQUFqQjtBQUNBLGtCQUFpQixTQUFqQixFOzs7Ozs7OztBQ2pDQTtBQUNBLEtBQUksY0FBYyxvQkFBUSxDQUFSLEVBQWtCLGFBQWxCLENBQWxCO0FBQUEsS0FDSSxhQUFjLE1BQU0sU0FEeEI7QUFFQSxLQUFHLFdBQVcsV0FBWCxLQUEyQixTQUE5QixFQUF3QyxvQkFBUSxFQUFSLEVBQW1CLFVBQW5CLEVBQStCLFdBQS9CLEVBQTRDLEVBQTVDO0FBQ3hDLFFBQU8sT0FBUCxHQUFpQixVQUFTLEdBQVQsRUFBYTtBQUM1QixjQUFXLFdBQVgsRUFBd0IsR0FBeEIsSUFBK0IsSUFBL0I7QUFDRCxFQUZELEM7Ozs7Ozs7O0FDSkEsUUFBTyxPQUFQLEdBQWlCLFVBQVMsSUFBVCxFQUFlLEtBQWYsRUFBcUI7QUFDcEMsVUFBTyxFQUFDLE9BQU8sS0FBUixFQUFlLE1BQU0sQ0FBQyxDQUFDLElBQXZCLEVBQVA7QUFDRCxFQUZELEM7Ozs7OztBQ0FBOztBQUNBLEtBQUksVUFBcUIsb0JBQVEsRUFBUixDQUF6QjtBQUFBLEtBQ0ksU0FBcUIsb0JBQVEsRUFBUixDQUR6QjtBQUFBLEtBRUksTUFBcUIsb0JBQVEsRUFBUixDQUZ6QjtBQUFBLEtBR0ksVUFBcUIsb0JBQVEsQ0FBUixDQUh6QjtBQUFBLEtBSUksVUFBcUIsb0JBQVEsRUFBUixDQUp6QjtBQUFBLEtBS0ksV0FBcUIsb0JBQVEsRUFBUixDQUx6QjtBQUFBLEtBTUksWUFBcUIsb0JBQVEsRUFBUixDQU56QjtBQUFBLEtBT0ksYUFBcUIsb0JBQVEsRUFBUixDQVB6QjtBQUFBLEtBUUksUUFBcUIsb0JBQVEsRUFBUixDQVJ6QjtBQUFBLEtBU0kscUJBQXFCLG9CQUFRLEVBQVIsQ0FUekI7QUFBQSxLQVVJLE9BQXFCLG9CQUFRLEVBQVIsRUFBbUIsR0FWNUM7QUFBQSxLQVdJLFlBQXFCLG9CQUFRLEVBQVIsR0FYekI7QUFBQSxLQVlJLFVBQXFCLFNBWnpCO0FBQUEsS0FhSSxZQUFxQixPQUFPLFNBYmhDO0FBQUEsS0FjSSxVQUFxQixPQUFPLE9BZGhDO0FBQUEsS0FlSSxXQUFxQixPQUFPLE9BQVAsQ0FmekI7QUFBQSxLQWdCSSxVQUFxQixPQUFPLE9BaEJoQztBQUFBLEtBaUJJLFNBQXFCLFFBQVEsT0FBUixLQUFvQixTQWpCN0M7QUFBQSxLQWtCSSxRQUFxQixTQUFyQixLQUFxQixHQUFVLENBQUUsV0FBYSxDQWxCbEQ7QUFBQSxLQW1CSSxRQW5CSjtBQUFBLEtBbUJjLHdCQW5CZDtBQUFBLEtBbUJ3QyxPQW5CeEM7O0FBcUJBLEtBQUksYUFBYSxDQUFDLENBQUMsWUFBVTtBQUMzQixPQUFJO0FBQ0Y7QUFDQSxTQUFJLFVBQWMsU0FBUyxPQUFULENBQWlCLENBQWpCLENBQWxCO0FBQUEsU0FDSSxjQUFjLENBQUMsUUFBUSxXQUFSLEdBQXNCLEVBQXZCLEVBQTJCLG9CQUFRLENBQVIsRUFBa0IsU0FBbEIsQ0FBM0IsSUFBMkQsVUFBUyxJQUFULEVBQWM7QUFBRSxZQUFLLEtBQUwsRUFBWSxLQUFaO0FBQXFCLE1BRGxIO0FBRUE7QUFDQSxZQUFPLENBQUMsVUFBVSxPQUFPLHFCQUFQLElBQWdDLFVBQTNDLEtBQTBELFFBQVEsSUFBUixDQUFhLEtBQWIsYUFBK0IsV0FBaEc7QUFDRCxJQU5ELENBTUUsT0FBTSxDQUFOLEVBQVEsQ0FBRSxXQUFhO0FBQzFCLEVBUmtCLEVBQW5COztBQVVBO0FBQ0EsS0FBSSxrQkFBa0IsU0FBbEIsZUFBa0IsQ0FBUyxDQUFULEVBQVksQ0FBWixFQUFjO0FBQ2xDO0FBQ0EsVUFBTyxNQUFNLENBQU4sSUFBVyxNQUFNLFFBQU4sSUFBa0IsTUFBTSxPQUExQztBQUNELEVBSEQ7QUFJQSxLQUFJLGFBQWEsU0FBYixVQUFhLENBQVMsRUFBVCxFQUFZO0FBQzNCLE9BQUksSUFBSjtBQUNBLFVBQU8sU0FBUyxFQUFULEtBQWdCLFFBQVEsT0FBTyxHQUFHLElBQWxCLEtBQTJCLFVBQTNDLEdBQXdELElBQXhELEdBQStELEtBQXRFO0FBQ0QsRUFIRDtBQUlBLEtBQUksdUJBQXVCLFNBQXZCLG9CQUF1QixDQUFTLENBQVQsRUFBVztBQUNwQyxVQUFPLGdCQUFnQixRQUFoQixFQUEwQixDQUExQixJQUNILElBQUksaUJBQUosQ0FBc0IsQ0FBdEIsQ0FERyxHQUVILElBQUksd0JBQUosQ0FBNkIsQ0FBN0IsQ0FGSjtBQUdELEVBSkQ7QUFLQSxLQUFJLG9CQUFvQiwyQkFBMkIsa0NBQVMsQ0FBVCxFQUFXO0FBQzVELE9BQUksT0FBSixFQUFhLE1BQWI7QUFDQSxRQUFLLE9BQUwsR0FBZSxJQUFJLENBQUosQ0FBTSxVQUFTLFNBQVQsRUFBb0IsUUFBcEIsRUFBNkI7QUFDaEQsU0FBRyxZQUFZLFNBQVosSUFBeUIsV0FBVyxTQUF2QyxFQUFpRCxNQUFNLFVBQVUseUJBQVYsQ0FBTjtBQUNqRCxlQUFVLFNBQVY7QUFDQSxjQUFVLFFBQVY7QUFDRCxJQUpjLENBQWY7QUFLQSxRQUFLLE9BQUwsR0FBZSxVQUFVLE9BQVYsQ0FBZjtBQUNBLFFBQUssTUFBTCxHQUFlLFVBQVUsTUFBVixDQUFmO0FBQ0QsRUFURDtBQVVBLEtBQUksVUFBVSxTQUFWLE9BQVUsQ0FBUyxJQUFULEVBQWM7QUFDMUIsT0FBSTtBQUNGO0FBQ0QsSUFGRCxDQUVFLE9BQU0sQ0FBTixFQUFRO0FBQ1IsWUFBTyxFQUFDLE9BQU8sQ0FBUixFQUFQO0FBQ0Q7QUFDRixFQU5EO0FBT0EsS0FBSSxTQUFTLFNBQVQsTUFBUyxDQUFTLE9BQVQsRUFBa0IsUUFBbEIsRUFBMkI7QUFDdEMsT0FBRyxRQUFRLEVBQVgsRUFBYztBQUNkLFdBQVEsRUFBUixHQUFhLElBQWI7QUFDQSxPQUFJLFFBQVEsUUFBUSxFQUFwQjtBQUNBLGFBQVUsWUFBVTtBQUNsQixTQUFJLFFBQVEsUUFBUSxFQUFwQjtBQUFBLFNBQ0ksS0FBUSxRQUFRLEVBQVIsSUFBYyxDQUQxQjtBQUFBLFNBRUksSUFBUSxDQUZaO0FBR0EsU0FBSSxNQUFNLFNBQU4sR0FBTSxDQUFTLFFBQVQsRUFBa0I7QUFDMUIsV0FBSSxVQUFVLEtBQUssU0FBUyxFQUFkLEdBQW1CLFNBQVMsSUFBMUM7QUFBQSxXQUNJLFVBQVUsU0FBUyxPQUR2QjtBQUFBLFdBRUksU0FBVSxTQUFTLE1BRnZCO0FBQUEsV0FHSSxTQUFVLFNBQVMsTUFIdkI7QUFBQSxXQUlJLE1BSko7QUFBQSxXQUlZLElBSlo7QUFLQSxXQUFJO0FBQ0YsYUFBRyxPQUFILEVBQVc7QUFDVCxlQUFHLENBQUMsRUFBSixFQUFPO0FBQ0wsaUJBQUcsUUFBUSxFQUFSLElBQWMsQ0FBakIsRUFBbUIsa0JBQWtCLE9BQWxCO0FBQ25CLHFCQUFRLEVBQVIsR0FBYSxDQUFiO0FBQ0Q7QUFDRCxlQUFHLFlBQVksSUFBZixFQUFvQixTQUFTLEtBQVQsQ0FBcEIsS0FDSztBQUNILGlCQUFHLE1BQUgsRUFBVSxPQUFPLEtBQVA7QUFDVixzQkFBUyxRQUFRLEtBQVIsQ0FBVDtBQUNBLGlCQUFHLE1BQUgsRUFBVSxPQUFPLElBQVA7QUFDWDtBQUNELGVBQUcsV0FBVyxTQUFTLE9BQXZCLEVBQStCO0FBQzdCLG9CQUFPLFVBQVUscUJBQVYsQ0FBUDtBQUNELFlBRkQsTUFFTyxJQUFHLE9BQU8sV0FBVyxNQUFYLENBQVYsRUFBNkI7QUFDbEMsa0JBQUssSUFBTCxDQUFVLE1BQVYsRUFBa0IsT0FBbEIsRUFBMkIsTUFBM0I7QUFDRCxZQUZNLE1BRUEsUUFBUSxNQUFSO0FBQ1IsVUFoQkQsTUFnQk8sT0FBTyxLQUFQO0FBQ1IsUUFsQkQsQ0FrQkUsT0FBTSxDQUFOLEVBQVE7QUFDUixnQkFBTyxDQUFQO0FBQ0Q7QUFDRixNQTNCRDtBQTRCQSxZQUFNLE1BQU0sTUFBTixHQUFlLENBQXJCO0FBQXVCLFdBQUksTUFBTSxHQUFOLENBQUo7QUFBdkIsTUFoQ2tCLENBZ0NzQjtBQUN4QyxhQUFRLEVBQVIsR0FBYSxFQUFiO0FBQ0EsYUFBUSxFQUFSLEdBQWEsS0FBYjtBQUNBLFNBQUcsWUFBWSxDQUFDLFFBQVEsRUFBeEIsRUFBMkIsWUFBWSxPQUFaO0FBQzVCLElBcENEO0FBcUNELEVBekNEO0FBMENBLEtBQUksY0FBYyxTQUFkLFdBQWMsQ0FBUyxPQUFULEVBQWlCO0FBQ2pDLFFBQUssSUFBTCxDQUFVLE1BQVYsRUFBa0IsWUFBVTtBQUMxQixTQUFJLFFBQVEsUUFBUSxFQUFwQjtBQUFBLFNBQ0ksTUFESjtBQUFBLFNBQ1ksT0FEWjtBQUFBLFNBQ3FCLE9BRHJCO0FBRUEsU0FBRyxZQUFZLE9BQVosQ0FBSCxFQUF3QjtBQUN0QixnQkFBUyxRQUFRLFlBQVU7QUFDekIsYUFBRyxNQUFILEVBQVU7QUFDUixtQkFBUSxJQUFSLENBQWEsb0JBQWIsRUFBbUMsS0FBbkMsRUFBMEMsT0FBMUM7QUFDRCxVQUZELE1BRU8sSUFBRyxVQUFVLE9BQU8sb0JBQXBCLEVBQXlDO0FBQzlDLG1CQUFRLEVBQUMsU0FBUyxPQUFWLEVBQW1CLFFBQVEsS0FBM0IsRUFBUjtBQUNELFVBRk0sTUFFQSxJQUFHLENBQUMsVUFBVSxPQUFPLE9BQWxCLEtBQThCLFFBQVEsS0FBekMsRUFBK0M7QUFDcEQsbUJBQVEsS0FBUixDQUFjLDZCQUFkLEVBQTZDLEtBQTdDO0FBQ0Q7QUFDRixRQVJRLENBQVQ7QUFTQTtBQUNBLGVBQVEsRUFBUixHQUFhLFVBQVUsWUFBWSxPQUFaLENBQVYsR0FBaUMsQ0FBakMsR0FBcUMsQ0FBbEQ7QUFDRCxNQUFDLFFBQVEsRUFBUixHQUFhLFNBQWI7QUFDRixTQUFHLE1BQUgsRUFBVSxNQUFNLE9BQU8sS0FBYjtBQUNYLElBakJEO0FBa0JELEVBbkJEO0FBb0JBLEtBQUksY0FBYyxTQUFkLFdBQWMsQ0FBUyxPQUFULEVBQWlCO0FBQ2pDLE9BQUcsUUFBUSxFQUFSLElBQWMsQ0FBakIsRUFBbUIsT0FBTyxLQUFQO0FBQ25CLE9BQUksUUFBUSxRQUFRLEVBQVIsSUFBYyxRQUFRLEVBQWxDO0FBQUEsT0FDSSxJQUFRLENBRFo7QUFBQSxPQUVJLFFBRko7QUFHQSxVQUFNLE1BQU0sTUFBTixHQUFlLENBQXJCLEVBQXVCO0FBQ3JCLGdCQUFXLE1BQU0sR0FBTixDQUFYO0FBQ0EsU0FBRyxTQUFTLElBQVQsSUFBaUIsQ0FBQyxZQUFZLFNBQVMsT0FBckIsQ0FBckIsRUFBbUQsT0FBTyxLQUFQO0FBQ3BELElBQUMsT0FBTyxJQUFQO0FBQ0gsRUFURDtBQVVBLEtBQUksb0JBQW9CLFNBQXBCLGlCQUFvQixDQUFTLE9BQVQsRUFBaUI7QUFDdkMsUUFBSyxJQUFMLENBQVUsTUFBVixFQUFrQixZQUFVO0FBQzFCLFNBQUksT0FBSjtBQUNBLFNBQUcsTUFBSCxFQUFVO0FBQ1IsZUFBUSxJQUFSLENBQWEsa0JBQWIsRUFBaUMsT0FBakM7QUFDRCxNQUZELE1BRU8sSUFBRyxVQUFVLE9BQU8sa0JBQXBCLEVBQXVDO0FBQzVDLGVBQVEsRUFBQyxTQUFTLE9BQVYsRUFBbUIsUUFBUSxRQUFRLEVBQW5DLEVBQVI7QUFDRDtBQUNGLElBUEQ7QUFRRCxFQVREO0FBVUEsS0FBSSxVQUFVLFNBQVYsT0FBVSxDQUFTLEtBQVQsRUFBZTtBQUMzQixPQUFJLFVBQVUsSUFBZDtBQUNBLE9BQUcsUUFBUSxFQUFYLEVBQWM7QUFDZCxXQUFRLEVBQVIsR0FBYSxJQUFiO0FBQ0EsYUFBVSxRQUFRLEVBQVIsSUFBYyxPQUF4QixDQUoyQixDQUlNO0FBQ2pDLFdBQVEsRUFBUixHQUFhLEtBQWI7QUFDQSxXQUFRLEVBQVIsR0FBYSxDQUFiO0FBQ0EsT0FBRyxDQUFDLFFBQVEsRUFBWixFQUFlLFFBQVEsRUFBUixHQUFhLFFBQVEsRUFBUixDQUFXLEtBQVgsRUFBYjtBQUNmLFVBQU8sT0FBUCxFQUFnQixJQUFoQjtBQUNELEVBVEQ7QUFVQSxLQUFJLFdBQVcsU0FBWCxRQUFXLENBQVMsS0FBVCxFQUFlO0FBQzVCLE9BQUksVUFBVSxJQUFkO0FBQUEsT0FDSSxJQURKO0FBRUEsT0FBRyxRQUFRLEVBQVgsRUFBYztBQUNkLFdBQVEsRUFBUixHQUFhLElBQWI7QUFDQSxhQUFVLFFBQVEsRUFBUixJQUFjLE9BQXhCLENBTDRCLENBS0s7QUFDakMsT0FBSTtBQUNGLFNBQUcsWUFBWSxLQUFmLEVBQXFCLE1BQU0sVUFBVSxrQ0FBVixDQUFOO0FBQ3JCLFNBQUcsT0FBTyxXQUFXLEtBQVgsQ0FBVixFQUE0QjtBQUMxQixpQkFBVSxZQUFVO0FBQ2xCLGFBQUksVUFBVSxFQUFDLElBQUksT0FBTCxFQUFjLElBQUksS0FBbEIsRUFBZCxDQURrQixDQUNzQjtBQUN4QyxhQUFJO0FBQ0YsZ0JBQUssSUFBTCxDQUFVLEtBQVYsRUFBaUIsSUFBSSxRQUFKLEVBQWMsT0FBZCxFQUF1QixDQUF2QixDQUFqQixFQUE0QyxJQUFJLE9BQUosRUFBYSxPQUFiLEVBQXNCLENBQXRCLENBQTVDO0FBQ0QsVUFGRCxDQUVFLE9BQU0sQ0FBTixFQUFRO0FBQ1IsbUJBQVEsSUFBUixDQUFhLE9BQWIsRUFBc0IsQ0FBdEI7QUFDRDtBQUNGLFFBUEQ7QUFRRCxNQVRELE1BU087QUFDTCxlQUFRLEVBQVIsR0FBYSxLQUFiO0FBQ0EsZUFBUSxFQUFSLEdBQWEsQ0FBYjtBQUNBLGNBQU8sT0FBUCxFQUFnQixLQUFoQjtBQUNEO0FBQ0YsSUFoQkQsQ0FnQkUsT0FBTSxDQUFOLEVBQVE7QUFDUixhQUFRLElBQVIsQ0FBYSxFQUFDLElBQUksT0FBTCxFQUFjLElBQUksS0FBbEIsRUFBYixFQUF1QyxDQUF2QyxFQURRLENBQ21DO0FBQzVDO0FBQ0YsRUF6QkQ7O0FBMkJBO0FBQ0EsS0FBRyxDQUFDLFVBQUosRUFBZTtBQUNiO0FBQ0EsY0FBVyxTQUFTLE9BQVQsQ0FBaUIsUUFBakIsRUFBMEI7QUFDbkMsZ0JBQVcsSUFBWCxFQUFpQixRQUFqQixFQUEyQixPQUEzQixFQUFvQyxJQUFwQztBQUNBLGVBQVUsUUFBVjtBQUNBLGNBQVMsSUFBVCxDQUFjLElBQWQ7QUFDQSxTQUFJO0FBQ0YsZ0JBQVMsSUFBSSxRQUFKLEVBQWMsSUFBZCxFQUFvQixDQUFwQixDQUFULEVBQWlDLElBQUksT0FBSixFQUFhLElBQWIsRUFBbUIsQ0FBbkIsQ0FBakM7QUFDRCxNQUZELENBRUUsT0FBTSxHQUFOLEVBQVU7QUFDVixlQUFRLElBQVIsQ0FBYSxJQUFiLEVBQW1CLEdBQW5CO0FBQ0Q7QUFDRixJQVREO0FBVUEsY0FBVyxTQUFTLE9BQVQsQ0FBaUIsUUFBakIsRUFBMEI7QUFDbkMsVUFBSyxFQUFMLEdBQVUsRUFBVixDQURtQyxDQUNUO0FBQzFCLFVBQUssRUFBTCxHQUFVLFNBQVYsQ0FGbUMsQ0FFVDtBQUMxQixVQUFLLEVBQUwsR0FBVSxDQUFWLENBSG1DLENBR1Q7QUFDMUIsVUFBSyxFQUFMLEdBQVUsS0FBVixDQUptQyxDQUlUO0FBQzFCLFVBQUssRUFBTCxHQUFVLFNBQVYsQ0FMbUMsQ0FLVDtBQUMxQixVQUFLLEVBQUwsR0FBVSxDQUFWLENBTm1DLENBTVQ7QUFDMUIsVUFBSyxFQUFMLEdBQVUsS0FBVixDQVBtQyxDQU9UO0FBQzNCLElBUkQ7QUFTQSxZQUFTLFNBQVQsR0FBcUIsb0JBQVEsRUFBUixFQUEyQixTQUFTLFNBQXBDLEVBQStDO0FBQ2xFO0FBQ0EsV0FBTSxTQUFTLElBQVQsQ0FBYyxXQUFkLEVBQTJCLFVBQTNCLEVBQXNDO0FBQzFDLFdBQUksV0FBYyxxQkFBcUIsbUJBQW1CLElBQW5CLEVBQXlCLFFBQXpCLENBQXJCLENBQWxCO0FBQ0EsZ0JBQVMsRUFBVCxHQUFrQixPQUFPLFdBQVAsSUFBc0IsVUFBdEIsR0FBbUMsV0FBbkMsR0FBaUQsSUFBbkU7QUFDQSxnQkFBUyxJQUFULEdBQWtCLE9BQU8sVUFBUCxJQUFxQixVQUFyQixJQUFtQyxVQUFyRDtBQUNBLGdCQUFTLE1BQVQsR0FBa0IsU0FBUyxRQUFRLE1BQWpCLEdBQTBCLFNBQTVDO0FBQ0EsWUFBSyxFQUFMLENBQVEsSUFBUixDQUFhLFFBQWI7QUFDQSxXQUFHLEtBQUssRUFBUixFQUFXLEtBQUssRUFBTCxDQUFRLElBQVIsQ0FBYSxRQUFiO0FBQ1gsV0FBRyxLQUFLLEVBQVIsRUFBVyxPQUFPLElBQVAsRUFBYSxLQUFiO0FBQ1gsY0FBTyxTQUFTLE9BQWhCO0FBQ0QsTUFYaUU7QUFZbEU7QUFDQSxjQUFTLGdCQUFTLFVBQVQsRUFBb0I7QUFDM0IsY0FBTyxLQUFLLElBQUwsQ0FBVSxTQUFWLEVBQXFCLFVBQXJCLENBQVA7QUFDRDtBQWZpRSxJQUEvQyxDQUFyQjtBQWlCQSx1QkFBb0IsNkJBQVU7QUFDNUIsU0FBSSxVQUFXLElBQUksUUFBSixFQUFmO0FBQ0EsVUFBSyxPQUFMLEdBQWUsT0FBZjtBQUNBLFVBQUssT0FBTCxHQUFlLElBQUksUUFBSixFQUFjLE9BQWQsRUFBdUIsQ0FBdkIsQ0FBZjtBQUNBLFVBQUssTUFBTCxHQUFlLElBQUksT0FBSixFQUFhLE9BQWIsRUFBc0IsQ0FBdEIsQ0FBZjtBQUNELElBTEQ7QUFNRDs7QUFFRCxTQUFRLFFBQVEsQ0FBUixHQUFZLFFBQVEsQ0FBcEIsR0FBd0IsUUFBUSxDQUFSLEdBQVksQ0FBQyxVQUE3QyxFQUF5RCxFQUFDLFNBQVMsUUFBVixFQUF6RDtBQUNBLHFCQUFRLEVBQVIsRUFBZ0MsUUFBaEMsRUFBMEMsT0FBMUM7QUFDQSxxQkFBUSxFQUFSLEVBQTBCLE9BQTFCO0FBQ0EsV0FBVSxvQkFBUSxFQUFSLEVBQW1CLE9BQW5CLENBQVY7O0FBRUE7QUFDQSxTQUFRLFFBQVEsQ0FBUixHQUFZLFFBQVEsQ0FBUixHQUFZLENBQUMsVUFBakMsRUFBNkMsT0FBN0MsRUFBc0Q7QUFDcEQ7QUFDQSxXQUFRLFNBQVMsTUFBVCxDQUFnQixDQUFoQixFQUFrQjtBQUN4QixTQUFJLGFBQWEscUJBQXFCLElBQXJCLENBQWpCO0FBQUEsU0FDSSxXQUFhLFdBQVcsTUFENUI7QUFFQSxjQUFTLENBQVQ7QUFDQSxZQUFPLFdBQVcsT0FBbEI7QUFDRDtBQVBtRCxFQUF0RDtBQVNBLFNBQVEsUUFBUSxDQUFSLEdBQVksUUFBUSxDQUFSLElBQWEsV0FBVyxDQUFDLFVBQXpCLENBQXBCLEVBQTBELE9BQTFELEVBQW1FO0FBQ2pFO0FBQ0EsWUFBUyxTQUFTLE9BQVQsQ0FBaUIsQ0FBakIsRUFBbUI7QUFDMUI7QUFDQSxTQUFHLGFBQWEsUUFBYixJQUF5QixnQkFBZ0IsRUFBRSxXQUFsQixFQUErQixJQUEvQixDQUE1QixFQUFpRSxPQUFPLENBQVA7QUFDakUsU0FBSSxhQUFhLHFCQUFxQixJQUFyQixDQUFqQjtBQUFBLFNBQ0ksWUFBYSxXQUFXLE9BRDVCO0FBRUEsZUFBVSxDQUFWO0FBQ0EsWUFBTyxXQUFXLE9BQWxCO0FBQ0Q7QUFUZ0UsRUFBbkU7QUFXQSxTQUFRLFFBQVEsQ0FBUixHQUFZLFFBQVEsQ0FBUixHQUFZLEVBQUUsY0FBYyxvQkFBUSxFQUFSLEVBQTBCLFVBQVMsSUFBVCxFQUFjO0FBQ3RGLFlBQVMsR0FBVCxDQUFhLElBQWIsRUFBbUIsT0FBbkIsRUFBNEIsS0FBNUI7QUFDRCxFQUYrQyxDQUFoQixDQUFoQyxFQUVLLE9BRkwsRUFFYztBQUNaO0FBQ0EsUUFBSyxTQUFTLEdBQVQsQ0FBYSxRQUFiLEVBQXNCO0FBQ3pCLFNBQUksSUFBYSxJQUFqQjtBQUFBLFNBQ0ksYUFBYSxxQkFBcUIsQ0FBckIsQ0FEakI7QUFBQSxTQUVJLFVBQWEsV0FBVyxPQUY1QjtBQUFBLFNBR0ksU0FBYSxXQUFXLE1BSDVCO0FBSUEsU0FBSSxTQUFTLFFBQVEsWUFBVTtBQUM3QixXQUFJLFNBQVksRUFBaEI7QUFBQSxXQUNJLFFBQVksQ0FEaEI7QUFBQSxXQUVJLFlBQVksQ0FGaEI7QUFHQSxhQUFNLFFBQU4sRUFBZ0IsS0FBaEIsRUFBdUIsVUFBUyxPQUFULEVBQWlCO0FBQ3RDLGFBQUksU0FBZ0IsT0FBcEI7QUFBQSxhQUNJLGdCQUFnQixLQURwQjtBQUVBLGdCQUFPLElBQVAsQ0FBWSxTQUFaO0FBQ0E7QUFDQSxXQUFFLE9BQUYsQ0FBVSxPQUFWLEVBQW1CLElBQW5CLENBQXdCLFVBQVMsS0FBVCxFQUFlO0FBQ3JDLGVBQUcsYUFBSCxFQUFpQjtBQUNqQiwyQkFBaUIsSUFBakI7QUFDQSxrQkFBTyxNQUFQLElBQWlCLEtBQWpCO0FBQ0EsYUFBRSxTQUFGLElBQWUsUUFBUSxNQUFSLENBQWY7QUFDRCxVQUxELEVBS0csTUFMSDtBQU1ELFFBWEQ7QUFZQSxTQUFFLFNBQUYsSUFBZSxRQUFRLE1BQVIsQ0FBZjtBQUNELE1BakJZLENBQWI7QUFrQkEsU0FBRyxNQUFILEVBQVUsT0FBTyxPQUFPLEtBQWQ7QUFDVixZQUFPLFdBQVcsT0FBbEI7QUFDRCxJQTNCVztBQTRCWjtBQUNBLFNBQU0sU0FBUyxJQUFULENBQWMsUUFBZCxFQUF1QjtBQUMzQixTQUFJLElBQWEsSUFBakI7QUFBQSxTQUNJLGFBQWEscUJBQXFCLENBQXJCLENBRGpCO0FBQUEsU0FFSSxTQUFhLFdBQVcsTUFGNUI7QUFHQSxTQUFJLFNBQVMsUUFBUSxZQUFVO0FBQzdCLGFBQU0sUUFBTixFQUFnQixLQUFoQixFQUF1QixVQUFTLE9BQVQsRUFBaUI7QUFDdEMsV0FBRSxPQUFGLENBQVUsT0FBVixFQUFtQixJQUFuQixDQUF3QixXQUFXLE9BQW5DLEVBQTRDLE1BQTVDO0FBQ0QsUUFGRDtBQUdELE1BSlksQ0FBYjtBQUtBLFNBQUcsTUFBSCxFQUFVLE9BQU8sT0FBTyxLQUFkO0FBQ1YsWUFBTyxXQUFXLE9BQWxCO0FBQ0Q7QUF4Q1csRUFGZCxFOzs7Ozs7OztBQy9QQSxRQUFPLE9BQVAsR0FBaUIsVUFBUyxFQUFULEVBQWEsV0FBYixFQUEwQixJQUExQixFQUFnQyxjQUFoQyxFQUErQztBQUM5RCxPQUFHLEVBQUUsY0FBYyxXQUFoQixLQUFpQyxtQkFBbUIsU0FBbkIsSUFBZ0Msa0JBQWtCLEVBQXRGLEVBQTBGO0FBQ3hGLFdBQU0sVUFBVSxPQUFPLHlCQUFqQixDQUFOO0FBQ0QsSUFBQyxPQUFPLEVBQVA7QUFDSCxFQUpELEM7Ozs7Ozs7O0FDQUEsS0FBSSxNQUFjLG9CQUFRLEVBQVIsQ0FBbEI7QUFBQSxLQUNJLE9BQWMsb0JBQVEsRUFBUixDQURsQjtBQUFBLEtBRUksY0FBYyxvQkFBUSxFQUFSLENBRmxCO0FBQUEsS0FHSSxXQUFjLG9CQUFRLEVBQVIsQ0FIbEI7QUFBQSxLQUlJLFdBQWMsb0JBQVEsRUFBUixDQUpsQjtBQUFBLEtBS0ksWUFBYyxvQkFBUSxFQUFSLENBTGxCO0FBQUEsS0FNSSxRQUFjLEVBTmxCO0FBQUEsS0FPSSxTQUFjLEVBUGxCO0FBUUEsS0FBSSxXQUFVLE9BQU8sT0FBUCxHQUFpQixVQUFTLFFBQVQsRUFBbUIsT0FBbkIsRUFBNEIsRUFBNUIsRUFBZ0MsSUFBaEMsRUFBc0MsUUFBdEMsRUFBK0M7QUFDNUUsT0FBSSxTQUFTLFdBQVcsWUFBVTtBQUFFLFlBQU8sUUFBUDtBQUFrQixJQUF6QyxHQUE0QyxVQUFVLFFBQVYsQ0FBekQ7QUFBQSxPQUNJLElBQVMsSUFBSSxFQUFKLEVBQVEsSUFBUixFQUFjLFVBQVUsQ0FBVixHQUFjLENBQTVCLENBRGI7QUFBQSxPQUVJLFFBQVMsQ0FGYjtBQUFBLE9BR0ksTUFISjtBQUFBLE9BR1ksSUFIWjtBQUFBLE9BR2tCLFFBSGxCO0FBQUEsT0FHNEIsTUFINUI7QUFJQSxPQUFHLE9BQU8sTUFBUCxJQUFpQixVQUFwQixFQUErQixNQUFNLFVBQVUsV0FBVyxtQkFBckIsQ0FBTjtBQUMvQjtBQUNBLE9BQUcsWUFBWSxNQUFaLENBQUgsRUFBdUIsS0FBSSxTQUFTLFNBQVMsU0FBUyxNQUFsQixDQUFiLEVBQXdDLFNBQVMsS0FBakQsRUFBd0QsT0FBeEQsRUFBZ0U7QUFDckYsY0FBUyxVQUFVLEVBQUUsU0FBUyxPQUFPLFNBQVMsS0FBVCxDQUFoQixFQUFpQyxDQUFqQyxDQUFGLEVBQXVDLEtBQUssQ0FBTCxDQUF2QyxDQUFWLEdBQTRELEVBQUUsU0FBUyxLQUFULENBQUYsQ0FBckU7QUFDQSxTQUFHLFdBQVcsS0FBWCxJQUFvQixXQUFXLE1BQWxDLEVBQXlDLE9BQU8sTUFBUDtBQUMxQyxJQUhELE1BR08sS0FBSSxXQUFXLE9BQU8sSUFBUCxDQUFZLFFBQVosQ0FBZixFQUFzQyxDQUFDLENBQUMsT0FBTyxTQUFTLElBQVQsRUFBUixFQUF5QixJQUFoRSxHQUF1RTtBQUM1RSxjQUFTLEtBQUssUUFBTCxFQUFlLENBQWYsRUFBa0IsS0FBSyxLQUF2QixFQUE4QixPQUE5QixDQUFUO0FBQ0EsU0FBRyxXQUFXLEtBQVgsSUFBb0IsV0FBVyxNQUFsQyxFQUF5QyxPQUFPLE1BQVA7QUFDMUM7QUFDRixFQWREO0FBZUEsVUFBUSxLQUFSLEdBQWlCLEtBQWpCO0FBQ0EsVUFBUSxNQUFSLEdBQWlCLE1BQWpCLEM7Ozs7Ozs7O0FDeEJBO0FBQ0EsS0FBSSxXQUFXLG9CQUFRLEVBQVIsQ0FBZjtBQUNBLFFBQU8sT0FBUCxHQUFpQixVQUFTLFFBQVQsRUFBbUIsRUFBbkIsRUFBdUIsS0FBdkIsRUFBOEIsT0FBOUIsRUFBc0M7QUFDckQsT0FBSTtBQUNGLFlBQU8sVUFBVSxHQUFHLFNBQVMsS0FBVCxFQUFnQixDQUFoQixDQUFILEVBQXVCLE1BQU0sQ0FBTixDQUF2QixDQUFWLEdBQTZDLEdBQUcsS0FBSCxDQUFwRDtBQUNGO0FBQ0MsSUFIRCxDQUdFLE9BQU0sQ0FBTixFQUFRO0FBQ1IsU0FBSSxNQUFNLFNBQVMsUUFBVCxDQUFWO0FBQ0EsU0FBRyxRQUFRLFNBQVgsRUFBcUIsU0FBUyxJQUFJLElBQUosQ0FBUyxRQUFULENBQVQ7QUFDckIsV0FBTSxDQUFOO0FBQ0Q7QUFDRixFQVRELEM7Ozs7Ozs7O0FDRkE7QUFDQSxLQUFJLFlBQWEsb0JBQVEsRUFBUixDQUFqQjtBQUFBLEtBQ0ksV0FBYSxvQkFBUSxDQUFSLEVBQWtCLFVBQWxCLENBRGpCO0FBQUEsS0FFSSxhQUFhLE1BQU0sU0FGdkI7O0FBSUEsUUFBTyxPQUFQLEdBQWlCLFVBQVMsRUFBVCxFQUFZO0FBQzNCLFVBQU8sT0FBTyxTQUFQLEtBQXFCLFVBQVUsS0FBVixLQUFvQixFQUFwQixJQUEwQixXQUFXLFFBQVgsTUFBeUIsRUFBeEUsQ0FBUDtBQUNELEVBRkQsQzs7Ozs7Ozs7QUNMQSxLQUFJLFVBQVksb0JBQVEsQ0FBUixDQUFoQjtBQUFBLEtBQ0ksV0FBWSxvQkFBUSxDQUFSLEVBQWtCLFVBQWxCLENBRGhCO0FBQUEsS0FFSSxZQUFZLG9CQUFRLEVBQVIsQ0FGaEI7QUFHQSxRQUFPLE9BQVAsR0FBaUIsb0JBQVEsRUFBUixFQUFtQixpQkFBbkIsR0FBdUMsVUFBUyxFQUFULEVBQVk7QUFDbEUsT0FBRyxNQUFNLFNBQVQsRUFBbUIsT0FBTyxHQUFHLFFBQUgsS0FDckIsR0FBRyxZQUFILENBRHFCLElBRXJCLFVBQVUsUUFBUSxFQUFSLENBQVYsQ0FGYztBQUdwQixFQUpELEM7Ozs7Ozs7O0FDSEE7QUFDQSxLQUFJLFdBQVksb0JBQVEsRUFBUixDQUFoQjtBQUFBLEtBQ0ksWUFBWSxvQkFBUSxFQUFSLENBRGhCO0FBQUEsS0FFSSxVQUFZLG9CQUFRLENBQVIsRUFBa0IsU0FBbEIsQ0FGaEI7QUFHQSxRQUFPLE9BQVAsR0FBaUIsVUFBUyxDQUFULEVBQVksQ0FBWixFQUFjO0FBQzdCLE9BQUksSUFBSSxTQUFTLENBQVQsRUFBWSxXQUFwQjtBQUFBLE9BQWlDLENBQWpDO0FBQ0EsVUFBTyxNQUFNLFNBQU4sSUFBbUIsQ0FBQyxJQUFJLFNBQVMsQ0FBVCxFQUFZLE9BQVosQ0FBTCxLQUE4QixTQUFqRCxHQUE2RCxDQUE3RCxHQUFpRSxVQUFVLENBQVYsQ0FBeEU7QUFDRCxFQUhELEM7Ozs7Ozs7O0FDSkEsS0FBSSxNQUFxQixvQkFBUSxFQUFSLENBQXpCO0FBQUEsS0FDSSxTQUFxQixvQkFBUSxFQUFSLENBRHpCO0FBQUEsS0FFSSxPQUFxQixvQkFBUSxFQUFSLENBRnpCO0FBQUEsS0FHSSxNQUFxQixvQkFBUSxFQUFSLENBSHpCO0FBQUEsS0FJSSxTQUFxQixvQkFBUSxFQUFSLENBSnpCO0FBQUEsS0FLSSxVQUFxQixPQUFPLE9BTGhDO0FBQUEsS0FNSSxVQUFxQixPQUFPLFlBTmhDO0FBQUEsS0FPSSxZQUFxQixPQUFPLGNBUGhDO0FBQUEsS0FRSSxpQkFBcUIsT0FBTyxjQVJoQztBQUFBLEtBU0ksVUFBcUIsQ0FUekI7QUFBQSxLQVVJLFFBQXFCLEVBVnpCO0FBQUEsS0FXSSxxQkFBcUIsb0JBWHpCO0FBQUEsS0FZSSxLQVpKO0FBQUEsS0FZVyxPQVpYO0FBQUEsS0FZb0IsSUFacEI7QUFhQSxLQUFJLE1BQU0sU0FBTixHQUFNLEdBQVU7QUFDbEIsT0FBSSxLQUFLLENBQUMsSUFBVjtBQUNBLE9BQUcsTUFBTSxjQUFOLENBQXFCLEVBQXJCLENBQUgsRUFBNEI7QUFDMUIsU0FBSSxLQUFLLE1BQU0sRUFBTixDQUFUO0FBQ0EsWUFBTyxNQUFNLEVBQU4sQ0FBUDtBQUNBO0FBQ0Q7QUFDRixFQVBEO0FBUUEsS0FBSSxXQUFXLFNBQVgsUUFBVyxDQUFTLEtBQVQsRUFBZTtBQUM1QixPQUFJLElBQUosQ0FBUyxNQUFNLElBQWY7QUFDRCxFQUZEO0FBR0E7QUFDQSxLQUFHLENBQUMsT0FBRCxJQUFZLENBQUMsU0FBaEIsRUFBMEI7QUFDeEIsYUFBVSxTQUFTLFlBQVQsQ0FBc0IsRUFBdEIsRUFBeUI7QUFDakMsU0FBSSxPQUFPLEVBQVg7QUFBQSxTQUFlLElBQUksQ0FBbkI7QUFDQSxZQUFNLFVBQVUsTUFBVixHQUFtQixDQUF6QjtBQUEyQixZQUFLLElBQUwsQ0FBVSxVQUFVLEdBQVYsQ0FBVjtBQUEzQixNQUNBLE1BQU0sRUFBRSxPQUFSLElBQW1CLFlBQVU7QUFDM0IsY0FBTyxPQUFPLEVBQVAsSUFBYSxVQUFiLEdBQTBCLEVBQTFCLEdBQStCLFNBQVMsRUFBVCxDQUF0QyxFQUFvRCxJQUFwRDtBQUNELE1BRkQ7QUFHQSxXQUFNLE9BQU47QUFDQSxZQUFPLE9BQVA7QUFDRCxJQVJEO0FBU0EsZUFBWSxTQUFTLGNBQVQsQ0FBd0IsRUFBeEIsRUFBMkI7QUFDckMsWUFBTyxNQUFNLEVBQU4sQ0FBUDtBQUNELElBRkQ7QUFHQTtBQUNBLE9BQUcsb0JBQVEsQ0FBUixFQUFrQixPQUFsQixLQUE4QixTQUFqQyxFQUEyQztBQUN6QyxhQUFRLGVBQVMsRUFBVCxFQUFZO0FBQ2xCLGVBQVEsUUFBUixDQUFpQixJQUFJLEdBQUosRUFBUyxFQUFULEVBQWEsQ0FBYixDQUFqQjtBQUNELE1BRkQ7QUFHRjtBQUNDLElBTEQsTUFLTyxJQUFHLGNBQUgsRUFBa0I7QUFDdkIsZUFBVSxJQUFJLGNBQUosRUFBVjtBQUNBLFlBQVUsUUFBUSxLQUFsQjtBQUNBLGFBQVEsS0FBUixDQUFjLFNBQWQsR0FBMEIsUUFBMUI7QUFDQSxhQUFRLElBQUksS0FBSyxXQUFULEVBQXNCLElBQXRCLEVBQTRCLENBQTVCLENBQVI7QUFDRjtBQUNBO0FBQ0MsSUFQTSxNQU9BLElBQUcsT0FBTyxnQkFBUCxJQUEyQixPQUFPLFdBQVAsSUFBc0IsVUFBakQsSUFBK0QsQ0FBQyxPQUFPLGFBQTFFLEVBQXdGO0FBQzdGLGFBQVEsZUFBUyxFQUFULEVBQVk7QUFDbEIsY0FBTyxXQUFQLENBQW1CLEtBQUssRUFBeEIsRUFBNEIsR0FBNUI7QUFDRCxNQUZEO0FBR0EsWUFBTyxnQkFBUCxDQUF3QixTQUF4QixFQUFtQyxRQUFuQyxFQUE2QyxLQUE3QztBQUNGO0FBQ0MsSUFOTSxNQU1BLElBQUcsc0JBQXNCLElBQUksUUFBSixDQUF6QixFQUF1QztBQUM1QyxhQUFRLGVBQVMsRUFBVCxFQUFZO0FBQ2xCLFlBQUssV0FBTCxDQUFpQixJQUFJLFFBQUosQ0FBakIsRUFBZ0Msa0JBQWhDLElBQXNELFlBQVU7QUFDOUQsY0FBSyxXQUFMLENBQWlCLElBQWpCO0FBQ0EsYUFBSSxJQUFKLENBQVMsRUFBVDtBQUNELFFBSEQ7QUFJRCxNQUxEO0FBTUY7QUFDQyxJQVJNLE1BUUE7QUFDTCxhQUFRLGVBQVMsRUFBVCxFQUFZO0FBQ2xCLGtCQUFXLElBQUksR0FBSixFQUFTLEVBQVQsRUFBYSxDQUFiLENBQVgsRUFBNEIsQ0FBNUI7QUFDRCxNQUZEO0FBR0Q7QUFDRjtBQUNELFFBQU8sT0FBUCxHQUFpQjtBQUNmLFFBQU8sT0FEUTtBQUVmLFVBQU87QUFGUSxFQUFqQixDOzs7Ozs7OztBQ3ZFQTtBQUNBLFFBQU8sT0FBUCxHQUFpQixVQUFTLEVBQVQsRUFBYSxJQUFiLEVBQW1CLElBQW5CLEVBQXdCO0FBQ3ZDLHVCQUFJLEtBQUssU0FBUyxTQUFsQjtBQUNBLDJCQUFPLEtBQUssTUFBWjtBQUNFLDBDQUFLLENBQUw7QUFBUSw4REFBTyxLQUFLLElBQUwsR0FDSyxHQUFHLElBQUgsQ0FBUSxJQUFSLENBRFo7QUFFUiwwQ0FBSyxDQUFMO0FBQVEsOERBQU8sS0FBSyxHQUFHLEtBQUssQ0FBTCxDQUFILENBQUwsR0FDSyxHQUFHLElBQUgsQ0FBUSxJQUFSLEVBQWMsS0FBSyxDQUFMLENBQWQsQ0FEWjtBQUVSLDBDQUFLLENBQUw7QUFBUSw4REFBTyxLQUFLLEdBQUcsS0FBSyxDQUFMLENBQUgsRUFBWSxLQUFLLENBQUwsQ0FBWixDQUFMLEdBQ0ssR0FBRyxJQUFILENBQVEsSUFBUixFQUFjLEtBQUssQ0FBTCxDQUFkLEVBQXVCLEtBQUssQ0FBTCxDQUF2QixDQURaO0FBRVIsMENBQUssQ0FBTDtBQUFRLDhEQUFPLEtBQUssR0FBRyxLQUFLLENBQUwsQ0FBSCxFQUFZLEtBQUssQ0FBTCxDQUFaLEVBQXFCLEtBQUssQ0FBTCxDQUFyQixDQUFMLEdBQ0ssR0FBRyxJQUFILENBQVEsSUFBUixFQUFjLEtBQUssQ0FBTCxDQUFkLEVBQXVCLEtBQUssQ0FBTCxDQUF2QixFQUFnQyxLQUFLLENBQUwsQ0FBaEMsQ0FEWjtBQUVSLDBDQUFLLENBQUw7QUFBUSw4REFBTyxLQUFLLEdBQUcsS0FBSyxDQUFMLENBQUgsRUFBWSxLQUFLLENBQUwsQ0FBWixFQUFxQixLQUFLLENBQUwsQ0FBckIsRUFBOEIsS0FBSyxDQUFMLENBQTlCLENBQUwsR0FDSyxHQUFHLElBQUgsQ0FBUSxJQUFSLEVBQWMsS0FBSyxDQUFMLENBQWQsRUFBdUIsS0FBSyxDQUFMLENBQXZCLEVBQWdDLEtBQUssQ0FBTCxDQUFoQyxFQUF5QyxLQUFLLENBQUwsQ0FBekMsQ0FEWjtBQVRWLG9CQVdFLE9BQW9CLEdBQUcsS0FBSCxDQUFTLElBQVQsRUFBZSxJQUFmLENBQXBCO0FBQ0gsRUFkRCxDOzs7Ozs7OztBQ0RBLEtBQUksU0FBWSxvQkFBUSxFQUFSLENBQWhCO0FBQUEsS0FDSSxZQUFZLG9CQUFRLEVBQVIsRUFBbUIsR0FEbkM7QUFBQSxLQUVJLFdBQVksT0FBTyxnQkFBUCxJQUEyQixPQUFPLHNCQUZsRDtBQUFBLEtBR0ksVUFBWSxPQUFPLE9BSHZCO0FBQUEsS0FJSSxVQUFZLE9BQU8sT0FKdkI7QUFBQSxLQUtJLFNBQVksb0JBQVEsQ0FBUixFQUFrQixPQUFsQixLQUE4QixTQUw5Qzs7QUFPQSxRQUFPLE9BQVAsR0FBaUIsWUFBVTtBQUN6QixPQUFJLElBQUosRUFBVSxJQUFWLEVBQWdCLE1BQWhCOztBQUVBLE9BQUksUUFBUSxTQUFSLEtBQVEsR0FBVTtBQUNwQixTQUFJLE1BQUosRUFBWSxFQUFaO0FBQ0EsU0FBRyxXQUFXLFNBQVMsUUFBUSxNQUE1QixDQUFILEVBQXVDLE9BQU8sSUFBUDtBQUN2QyxZQUFNLElBQU4sRUFBVztBQUNULFlBQU8sS0FBSyxFQUFaO0FBQ0EsY0FBTyxLQUFLLElBQVo7QUFDQSxXQUFJO0FBQ0Y7QUFDRCxRQUZELENBRUUsT0FBTSxDQUFOLEVBQVE7QUFDUixhQUFHLElBQUgsRUFBUSxTQUFSLEtBQ0ssT0FBTyxTQUFQO0FBQ0wsZUFBTSxDQUFOO0FBQ0Q7QUFDRixNQUFDLE9BQU8sU0FBUDtBQUNGLFNBQUcsTUFBSCxFQUFVLE9BQU8sS0FBUDtBQUNYLElBZkQ7O0FBaUJBO0FBQ0EsT0FBRyxNQUFILEVBQVU7QUFDUixjQUFTLGtCQUFVO0FBQ2pCLGVBQVEsUUFBUixDQUFpQixLQUFqQjtBQUNELE1BRkQ7QUFHRjtBQUNDLElBTEQsTUFLTyxJQUFHLFFBQUgsRUFBWTtBQUNqQixTQUFJLFNBQVMsSUFBYjtBQUFBLFNBQ0ksT0FBUyxTQUFTLGNBQVQsQ0FBd0IsRUFBeEIsQ0FEYjtBQUVBLFNBQUksUUFBSixDQUFhLEtBQWIsRUFBb0IsT0FBcEIsQ0FBNEIsSUFBNUIsRUFBa0MsRUFBQyxlQUFlLElBQWhCLEVBQWxDLEVBSGlCLENBR3lDO0FBQzFELGNBQVMsa0JBQVU7QUFDakIsWUFBSyxJQUFMLEdBQVksU0FBUyxDQUFDLE1BQXRCO0FBQ0QsTUFGRDtBQUdGO0FBQ0MsSUFSTSxNQVFBLElBQUcsV0FBVyxRQUFRLE9BQXRCLEVBQThCO0FBQ25DLFNBQUksVUFBVSxRQUFRLE9BQVIsRUFBZDtBQUNBLGNBQVMsa0JBQVU7QUFDakIsZUFBUSxJQUFSLENBQWEsS0FBYjtBQUNELE1BRkQ7QUFHRjtBQUNBO0FBQ0E7QUFDQTtBQUNBO0FBQ0E7QUFDQyxJQVhNLE1BV0E7QUFDTCxjQUFTLGtCQUFVO0FBQ2pCO0FBQ0EsaUJBQVUsSUFBVixDQUFlLE1BQWYsRUFBdUIsS0FBdkI7QUFDRCxNQUhEO0FBSUQ7O0FBRUQsVUFBTyxVQUFTLEVBQVQsRUFBWTtBQUNqQixTQUFJLE9BQU8sRUFBQyxJQUFJLEVBQUwsRUFBUyxNQUFNLFNBQWYsRUFBWDtBQUNBLFNBQUcsSUFBSCxFQUFRLEtBQUssSUFBTCxHQUFZLElBQVo7QUFDUixTQUFHLENBQUMsSUFBSixFQUFTO0FBQ1AsY0FBTyxJQUFQO0FBQ0E7QUFDRCxNQUFDLE9BQU8sSUFBUDtBQUNILElBUEQ7QUFRRCxFQTVERCxDOzs7Ozs7OztBQ1BBLEtBQUksV0FBVyxvQkFBUSxFQUFSLENBQWY7QUFDQSxRQUFPLE9BQVAsR0FBaUIsVUFBUyxNQUFULEVBQWlCLEdBQWpCLEVBQXNCLElBQXRCLEVBQTJCO0FBQzFDLFFBQUksSUFBSSxHQUFSLElBQWUsR0FBZjtBQUFtQixjQUFTLE1BQVQsRUFBaUIsR0FBakIsRUFBc0IsSUFBSSxHQUFKLENBQXRCLEVBQWdDLElBQWhDO0FBQW5CLElBQ0EsT0FBTyxNQUFQO0FBQ0QsRUFIRCxDOzs7Ozs7QUNEQTs7QUFDQSxLQUFJLFNBQWMsb0JBQVEsRUFBUixDQUFsQjtBQUFBLEtBQ0ksS0FBYyxvQkFBUSxFQUFSLENBRGxCO0FBQUEsS0FFSSxjQUFjLG9CQUFRLEVBQVIsQ0FGbEI7QUFBQSxLQUdJLFVBQWMsb0JBQVEsQ0FBUixFQUFrQixTQUFsQixDQUhsQjs7QUFLQSxRQUFPLE9BQVAsR0FBaUIsVUFBUyxHQUFULEVBQWE7QUFDNUIsT0FBSSxJQUFJLE9BQU8sR0FBUCxDQUFSO0FBQ0EsT0FBRyxlQUFlLENBQWYsSUFBb0IsQ0FBQyxFQUFFLE9BQUYsQ0FBeEIsRUFBbUMsR0FBRyxDQUFILENBQUssQ0FBTCxFQUFRLE9BQVIsRUFBaUI7QUFDbEQsbUJBQWMsSUFEb0M7QUFFbEQsVUFBSyxlQUFVO0FBQUUsY0FBTyxJQUFQO0FBQWM7QUFGbUIsSUFBakI7QUFJcEMsRUFORCxDOzs7Ozs7OztBQ05BLEtBQUksV0FBZSxvQkFBUSxDQUFSLEVBQWtCLFVBQWxCLENBQW5CO0FBQUEsS0FDSSxlQUFlLEtBRG5COztBQUdBLEtBQUk7QUFDRixPQUFJLFFBQVEsQ0FBQyxDQUFELEVBQUksUUFBSixHQUFaO0FBQ0EsU0FBTSxRQUFOLElBQWtCLFlBQVU7QUFBRSxvQkFBZSxJQUFmO0FBQXNCLElBQXBEO0FBQ0EsU0FBTSxJQUFOLENBQVcsS0FBWCxFQUFrQixZQUFVO0FBQUUsV0FBTSxDQUFOO0FBQVUsSUFBeEM7QUFDRCxFQUpELENBSUUsT0FBTSxDQUFOLEVBQVEsQ0FBRSxXQUFhOztBQUV6QixRQUFPLE9BQVAsR0FBaUIsVUFBUyxJQUFULEVBQWUsV0FBZixFQUEyQjtBQUMxQyxPQUFHLENBQUMsV0FBRCxJQUFnQixDQUFDLFlBQXBCLEVBQWlDLE9BQU8sS0FBUDtBQUNqQyxPQUFJLE9BQU8sS0FBWDtBQUNBLE9BQUk7QUFDRixTQUFJLE1BQU8sQ0FBQyxDQUFELENBQVg7QUFBQSxTQUNJLE9BQU8sSUFBSSxRQUFKLEdBRFg7QUFFQSxVQUFLLElBQUwsR0FBWSxZQUFVO0FBQUUsY0FBTyxFQUFDLE1BQU0sT0FBTyxJQUFkLEVBQVA7QUFBNkIsTUFBckQ7QUFDQSxTQUFJLFFBQUosSUFBZ0IsWUFBVTtBQUFFLGNBQU8sSUFBUDtBQUFjLE1BQTFDO0FBQ0EsVUFBSyxHQUFMO0FBQ0QsSUFORCxDQU1FLE9BQU0sQ0FBTixFQUFRLENBQUUsV0FBYTtBQUN6QixVQUFPLElBQVA7QUFDRCxFQVhELEM7Ozs7Ozs7Ozs7QUNUQTs7Ozs7Ozs7O2VBUytCLE07S0FBdkIsTyxXQUFBLE87S0FBUyxTLFdBQUEsUzs7QUFDakIsS0FBTSxTQUFTLENBQUMsT0FBRCxFQUFVLE1BQVYsRUFBa0IsTUFBbEIsRUFBMEIsS0FBMUIsRUFBaUMsT0FBakMsQ0FBZjtBQUNBLEtBQU0sV0FBVyxFQUFqQjs7QUFFQTs7QUFFQTtBQUNBLEtBQ0UsT0FBTyxPQUFQLEtBQW1CLFdBQW5CLElBQWtDO0FBQ2pDLFFBQU8sYUFBUCxJQUF3QixPQUFPLGFBQVAsQ0FBcUIsUUFBckIsS0FBa0MsS0FGN0QsQ0FFb0U7QUFGcEUsR0FHRTtBQUNBLFlBQU8sT0FBUCxHQUFpQjtBQUNmLGNBQU8saUJBQWE7QUFBQSwyQ0FBVCxJQUFTO0FBQVQsZUFBUztBQUFBOztBQUNsQixhQUFJLFdBQVcsT0FBWCxDQUFKLEVBQXlCO0FBQUUseURBQWEsT0FBTyxJQUFQLENBQWIsVUFBMkIsU0FBM0I7QUFBdUM7QUFDbkUsUUFIYztBQUlmLFlBQUssZUFBYTtBQUFBLDRDQUFULElBQVM7QUFBVCxlQUFTO0FBQUE7O0FBQ2hCLGFBQUksV0FBVyxLQUFYLENBQUosRUFBdUI7QUFBRSx5REFBYSxPQUFPLElBQVAsQ0FBYixVQUEyQixPQUEzQjtBQUFxQztBQUMvRCxRQU5jO0FBT2YsYUFBTSxnQkFBYTtBQUFBLDRDQUFULElBQVM7QUFBVCxlQUFTO0FBQUE7O0FBQ2pCLGFBQUksV0FBVyxNQUFYLENBQUosRUFBd0I7QUFBRSx5REFBYSxPQUFPLElBQVAsQ0FBYixVQUEyQixRQUEzQjtBQUFzQztBQUNqRSxRQVRjO0FBVWYsYUFBTSxnQkFBYTtBQUFBLDRDQUFULElBQVM7QUFBVCxlQUFTO0FBQUE7O0FBQ2pCLGFBQUksV0FBVyxNQUFYLENBQUosRUFBd0I7QUFBRSx5REFBYSxPQUFPLElBQVAsQ0FBYixVQUEyQixRQUEzQjtBQUFzQztBQUNqRSxRQVpjO0FBYWYsY0FBTyxpQkFBYTtBQUFBLDRDQUFULElBQVM7QUFBVCxlQUFTO0FBQUE7O0FBQ2xCLGFBQUksV0FBVyxPQUFYLENBQUosRUFBeUI7QUFBRSx5REFBYSxPQUFPLElBQVAsQ0FBYixVQUEyQixTQUEzQjtBQUF1QztBQUNuRTtBQWZjLE1BQWpCO0FBaUJELElBckJELE1Bc0JLO0FBQUU7QUFBRixPQUNLLEtBREwsR0FDdUMsT0FEdkMsQ0FDSyxLQURMO0FBQUEsT0FDWSxHQURaLEdBQ3VDLE9BRHZDLENBQ1ksR0FEWjtBQUFBLE9BQ2lCLElBRGpCLEdBQ3VDLE9BRHZDLENBQ2lCLElBRGpCO0FBQUEsT0FDdUIsSUFEdkIsR0FDdUMsT0FEdkMsQ0FDdUIsSUFEdkI7QUFBQSxPQUM2QixLQUQ3QixHQUN1QyxPQUR2QyxDQUM2QixLQUQ3Qjs7QUFFSCxXQUFRLE9BQVIsR0FBa0IsRUFBRSxZQUFGLEVBQVMsUUFBVCxFQUFjLFVBQWQsRUFBb0IsVUFBcEIsRUFBMEIsWUFBMUIsRUFBbEI7QUFDQSxXQUFRLEtBQVIsR0FBZ0IsWUFBYTtBQUFBLHdDQUFULElBQVM7QUFBVCxXQUFTO0FBQUE7O0FBQzNCLFNBQUksV0FBVyxPQUFYLENBQUosRUFBeUI7QUFBRSxlQUFRLE9BQVIsQ0FBZ0IsS0FBaEIsQ0FBc0IsS0FBdEIsQ0FBNEIsT0FBNUIsRUFBcUMsSUFBckM7QUFBNEM7QUFDeEUsSUFGRDtBQUdBLFdBQVEsR0FBUixHQUFjLFlBQWE7QUFBQSx3Q0FBVCxJQUFTO0FBQVQsV0FBUztBQUFBOztBQUN6QixTQUFJLFdBQVcsS0FBWCxDQUFKLEVBQXVCO0FBQUUsZUFBUSxPQUFSLENBQWdCLEdBQWhCLENBQW9CLEtBQXBCLENBQTBCLE9BQTFCLEVBQW1DLElBQW5DO0FBQTBDO0FBQ3BFLElBRkQ7QUFHQSxXQUFRLElBQVIsR0FBZSxZQUFhO0FBQUEsd0NBQVQsSUFBUztBQUFULFdBQVM7QUFBQTs7QUFDMUIsU0FBSSxXQUFXLE1BQVgsQ0FBSixFQUF3QjtBQUFFLGVBQVEsT0FBUixDQUFnQixJQUFoQixDQUFxQixLQUFyQixDQUEyQixPQUEzQixFQUFvQyxJQUFwQztBQUEyQztBQUN0RSxJQUZEO0FBR0EsV0FBUSxJQUFSLEdBQWUsWUFBYTtBQUFBLHdDQUFULElBQVM7QUFBVCxXQUFTO0FBQUE7O0FBQzFCLFNBQUksV0FBVyxNQUFYLENBQUosRUFBd0I7QUFBRSxlQUFRLE9BQVIsQ0FBZ0IsSUFBaEIsQ0FBcUIsS0FBckIsQ0FBMkIsT0FBM0IsRUFBb0MsSUFBcEM7QUFBMkM7QUFDdEUsSUFGRDtBQUdBLFdBQVEsS0FBUixHQUFnQixZQUFhO0FBQUEseUNBQVQsSUFBUztBQUFULFdBQVM7QUFBQTs7QUFDM0IsU0FBSSxXQUFXLE9BQVgsQ0FBSixFQUF5QjtBQUFFLGVBQVEsT0FBUixDQUFnQixLQUFoQixDQUFzQixLQUF0QixDQUE0QixPQUE1QixFQUFxQyxJQUFyQztBQUE0QztBQUN4RSxJQUZEO0FBR0Q7O0FBRUQ7Ozs7QUFJQSxVQUFTLGdCQUFULEdBQTZCO0FBQzNCLFVBQU8sT0FBUCxDQUFlLGlCQUFTO0FBQ3RCLFNBQU0sYUFBYSxPQUFPLE9BQVAsQ0FBZSxLQUFmLENBQW5CO0FBQ0EsY0FBUyxLQUFULElBQWtCLEVBQWxCO0FBQ0EsWUFBTyxPQUFQLENBQWUsZ0JBQVE7QUFDckIsV0FBTSxZQUFZLE9BQU8sT0FBUCxDQUFlLElBQWYsQ0FBbEI7QUFDQSxXQUFJLGFBQWEsVUFBakIsRUFBNkI7QUFDM0Isa0JBQVMsS0FBVCxFQUFnQixJQUFoQixJQUF3QixJQUF4QjtBQUNEO0FBQ0YsTUFMRDtBQU1ELElBVEQ7QUFVRDs7QUFFRDs7Ozs7QUFLQSxVQUFTLFVBQVQsQ0FBcUIsSUFBckIsRUFBMkI7QUFDekIsT0FBTSxXQUFZLE9BQU8sYUFBUCxJQUF3QixPQUFPLGFBQVAsQ0FBcUIsUUFBOUMsSUFBMkQsS0FBNUU7QUFDQSxVQUFPLFNBQVMsUUFBVCxLQUFzQixTQUFTLFFBQVQsRUFBbUIsSUFBbkIsQ0FBN0I7QUFDRDs7QUFFRDs7Ozs7QUFLQSxVQUFTLE1BQVQsQ0FBaUIsSUFBakIsRUFBdUI7QUFDckIsVUFBTyxLQUFLLEdBQUwsQ0FBUyxVQUFDLENBQUQsRUFBTztBQUNyQixTQUFNLE9BQU8sT0FBTyxTQUFQLENBQWlCLFFBQWpCLENBQTBCLElBQTFCLENBQStCLENBQS9CLENBQWI7QUFDQSxTQUFJLEtBQUssV0FBTCxPQUF1QixpQkFBM0IsRUFBOEM7QUFDNUMsV0FBSSxLQUFLLFNBQUwsQ0FBZSxDQUFmLENBQUo7QUFDRCxNQUZELE1BR0s7QUFDSCxXQUFJLE9BQU8sQ0FBUCxDQUFKO0FBQ0Q7QUFDRCxZQUFPLENBQVA7QUFDRCxJQVRNLENBQVA7QUFVRCxFOzs7Ozs7Ozs7QUNyR0QseUI7Ozs7Ozs7O0FDQUEscUJBQVEsRUFBUjtBQUNBLFFBQU8sT0FBUCxHQUFpQixvQkFBUSxFQUFSLEVBQStCLE1BQS9CLENBQXNDLE1BQXZELEM7Ozs7Ozs7O0FDREE7QUFDQSxLQUFJLFVBQVUsb0JBQVEsRUFBUixDQUFkOztBQUVBLFNBQVEsUUFBUSxDQUFSLEdBQVksUUFBUSxDQUE1QixFQUErQixRQUEvQixFQUF5QyxFQUFDLFFBQVEsb0JBQVEsRUFBUixDQUFULEVBQXpDLEU7Ozs7OztBQ0hBO0FBQ0E7O0FBQ0EsS0FBSSxVQUFXLG9CQUFRLEVBQVIsQ0FBZjtBQUFBLEtBQ0ksT0FBVyxvQkFBUSxFQUFSLENBRGY7QUFBQSxLQUVJLE1BQVcsb0JBQVEsRUFBUixDQUZmO0FBQUEsS0FHSSxXQUFXLG9CQUFRLEVBQVIsQ0FIZjtBQUFBLEtBSUksVUFBVyxvQkFBUSxFQUFSLENBSmY7QUFBQSxLQUtJLFVBQVcsT0FBTyxNQUx0Qjs7QUFPQTtBQUNBLFFBQU8sT0FBUCxHQUFpQixDQUFDLE9BQUQsSUFBWSxvQkFBUSxFQUFSLEVBQW9CLFlBQVU7QUFDekQsT0FBSSxJQUFJLEVBQVI7QUFBQSxPQUNJLElBQUksRUFEUjtBQUFBLE9BRUksSUFBSSxRQUZSO0FBQUEsT0FHSSxJQUFJLHNCQUhSO0FBSUEsS0FBRSxDQUFGLElBQU8sQ0FBUDtBQUNBLEtBQUUsS0FBRixDQUFRLEVBQVIsRUFBWSxPQUFaLENBQW9CLFVBQVMsQ0FBVCxFQUFXO0FBQUUsT0FBRSxDQUFGLElBQU8sQ0FBUDtBQUFXLElBQTVDO0FBQ0EsVUFBTyxRQUFRLEVBQVIsRUFBWSxDQUFaLEVBQWUsQ0FBZixLQUFxQixDQUFyQixJQUEwQixPQUFPLElBQVAsQ0FBWSxRQUFRLEVBQVIsRUFBWSxDQUFaLENBQVosRUFBNEIsSUFBNUIsQ0FBaUMsRUFBakMsS0FBd0MsQ0FBekU7QUFDRCxFQVI0QixDQUFaLEdBUVosU0FBUyxNQUFULENBQWdCLE1BQWhCLEVBQXdCLE1BQXhCLEVBQStCO0FBQUU7QUFDcEMsT0FBSSxJQUFRLFNBQVMsTUFBVCxDQUFaO0FBQUEsT0FDSSxPQUFRLFVBQVUsTUFEdEI7QUFBQSxPQUVJLFFBQVEsQ0FGWjtBQUFBLE9BR0ksYUFBYSxLQUFLLENBSHRCO0FBQUEsT0FJSSxTQUFhLElBQUksQ0FKckI7QUFLQSxVQUFNLE9BQU8sS0FBYixFQUFtQjtBQUNqQixTQUFJLElBQVMsUUFBUSxVQUFVLE9BQVYsQ0FBUixDQUFiO0FBQUEsU0FDSSxPQUFTLGFBQWEsUUFBUSxDQUFSLEVBQVcsTUFBWCxDQUFrQixXQUFXLENBQVgsQ0FBbEIsQ0FBYixHQUFnRCxRQUFRLENBQVIsQ0FEN0Q7QUFBQSxTQUVJLFNBQVMsS0FBSyxNQUZsQjtBQUFBLFNBR0ksSUFBUyxDQUhiO0FBQUEsU0FJSSxHQUpKO0FBS0EsWUFBTSxTQUFTLENBQWY7QUFBaUIsV0FBRyxPQUFPLElBQVAsQ0FBWSxDQUFaLEVBQWUsTUFBTSxLQUFLLEdBQUwsQ0FBckIsQ0FBSCxFQUFtQyxFQUFFLEdBQUYsSUFBUyxFQUFFLEdBQUYsQ0FBVDtBQUFwRDtBQUNELElBQUMsT0FBTyxDQUFQO0FBQ0gsRUF0QmdCLEdBc0JiLE9BdEJKLEM7Ozs7Ozs7O0FDVkEsU0FBUSxDQUFSLEdBQVksT0FBTyxxQkFBbkIsQzs7Ozs7Ozs7QUNBQSxTQUFRLENBQVIsR0FBWSxHQUFHLG9CQUFmLEM7Ozs7Ozs7Ozs7O1NDOERnQixlLEdBQUEsZTs7QUFwRGhCOzs7O0FBRUE7Ozs7QUFaQTs7Ozs7OztBQU9BO0FBQ0E7QUFDQTtBQUtBLEtBQU0sU0FBUztBQUNiLDJCQURhLEVBQ0gsc0JBREcsRUFDTSxzQkFETjtBQUViLFlBRmEsdUJBRU87QUFBQTs7QUFDbEIsWUFBTyxtQkFBTyxVQUFQLDBCQUFQO0FBQ0Q7QUFKWSxFQUFmOztBQU9BO0FBQ0E7QUFDQTtBQUNBLE1BQUssSUFBTSxJQUFYLHNCQUErQjtBQUM3QixPQUFNLFlBQVksaUJBQVcsSUFBWCxDQUFsQjtBQUNBLGFBQVUsSUFBVixDQUFlLE1BQWY7QUFDRDs7QUFFRCxLQUFNLGdCQUFnQiw0QkFBdEI7O0FBRUE7Ozs7Ozs7QUFPQSxVQUFTLFlBQVQsQ0FBdUIsSUFBdkIsRUFBNkI7QUFDM0IsT0FBSSxhQUFKO0FBQ0EsT0FBTSxTQUFTLGNBQWMsSUFBZCxDQUFtQixJQUFuQixDQUFmO0FBQ0EsT0FBSSxNQUFKLEVBQVk7QUFDVixTQUFJO0FBQ0YsY0FBTyxLQUFLLEtBQUwsQ0FBVyxPQUFPLENBQVAsQ0FBWCxDQUFQO0FBQ0QsTUFGRCxDQUdBLE9BQU8sQ0FBUCxFQUFVLENBQUU7QUFDYjtBQUNELFVBQU8sSUFBUDtBQUNEOztBQUVELEtBQU0sY0FBYyxFQUFwQjs7QUFFQSxRQUFPLGFBQVAsR0FBdUIsV0FBdkI7O0FBRUE7Ozs7Ozs7O0FBUU8sVUFBUyxlQUFULENBQTBCLEVBQTFCLEVBQThCLElBQTlCLEVBQW9DLE1BQXBDLEVBQTRDLElBQTVDLEVBQWtEO0FBQ3ZELFVBQU8sUUFBUSxNQUFmO0FBQ0EsT0FBTSxZQUFZLGlCQUFXLElBQVgsQ0FBbEI7QUFDQSxPQUFJLGFBQWEsVUFBVSxlQUEzQixFQUE0QztBQUMxQyxpQkFBWSxFQUFaLElBQWtCLFVBQVUsZUFBVixDQUEwQixFQUExQixFQUE4QixNQUE5QixFQUFzQyxJQUF0QyxDQUFsQjtBQUNBLGlCQUFZLEVBQVosRUFBZ0IsU0FBaEIsR0FBNEIsSUFBNUI7QUFDRCxJQUhELE1BSUs7QUFDSCxpQkFBWSxFQUFaLElBQWtCLEVBQWxCO0FBQ0Q7QUFDRjs7QUFFRDs7Ozs7Ozs7QUFRQSxVQUFTLGNBQVQsQ0FBeUIsRUFBekIsRUFBNkIsSUFBN0IsRUFBbUMsTUFBbkMsRUFBMkMsSUFBM0MsRUFBaUQ7QUFDL0MsT0FBSSxPQUFPLFlBQVksRUFBWixDQUFYO0FBQ0EsT0FBSSxDQUFDLElBQUwsRUFBVztBQUNULFlBQU8sYUFBYSxJQUFiLEtBQXNCLEVBQTdCO0FBQ0EsU0FBSSxDQUFDLGlCQUFXLEtBQUssU0FBaEIsQ0FBTCxFQUFpQztBQUMvQixZQUFLLFNBQUwsR0FBaUIsTUFBakI7QUFDRDtBQUNELGlCQUFZLEVBQVosSUFBa0IsSUFBbEI7QUFDQSxjQUFTLFVBQVUsRUFBbkI7QUFDQSxZQUFPLGFBQVAsR0FBdUIsS0FBSyxPQUE1QjtBQUNBLGFBQVEsS0FBUiwrQkFBMEMsS0FBSyxTQUEvQyxTQUE0RCxPQUFPLGFBQW5FLHVCQUFrRyxPQUFPLGFBQXpHO0FBQ0EsWUFBTyxpQkFBVyxLQUFLLFNBQWhCLEVBQTJCLGNBQTNCLENBQTBDLEVBQTFDLEVBQThDLElBQTlDLEVBQW9ELE1BQXBELEVBQTRELElBQTVELENBQVA7QUFDRDtBQUNELFVBQU8sSUFBSSxLQUFKLDJCQUFrQyxFQUFsQyxPQUFQO0FBQ0Q7O0FBRUQsS0FBTSxVQUFVO0FBQ2QsbUNBRGM7QUFFZDtBQUZjLEVBQWhCOztBQUtBOzs7O0FBSUEsVUFBUyxPQUFULENBQWtCLFVBQWxCLEVBQThCO0FBQzVCLFdBQVEsVUFBUixJQUFzQixZQUFtQjtBQUN2QyxVQUFLLElBQU0sS0FBWCxzQkFBK0I7QUFDN0IsV0FBTSxhQUFZLGlCQUFXLEtBQVgsQ0FBbEI7QUFDQSxXQUFJLGNBQWEsV0FBVSxVQUFWLENBQWpCLEVBQXdDO0FBQ3RDLG9CQUFVLFVBQVY7QUFDRDtBQUNGO0FBQ0YsSUFQRDtBQVFEOztBQUVEO0FBQ0EsRUFBQyxvQkFBRCxFQUF1QixpQkFBdkIsRUFBMEMsaUJBQTFDLEVBQTZELE9BQTdELENBQXFFLE9BQXJFOztBQUVBOzs7O0FBSUEsVUFBUyxXQUFULENBQXNCLFVBQXRCLEVBQWtDO0FBQ2hDLFdBQVEsVUFBUixJQUFzQixZQUFtQjtBQUN2QyxTQUFNLHFEQUFOO0FBQ0EsU0FBTSxPQUFPLFlBQVksRUFBWixDQUFiO0FBQ0EsU0FBSSxlQUFlLGlCQUFuQixFQUFzQztBQUNwQyxjQUFPLFlBQVksRUFBWixDQUFQO0FBQ0Q7QUFDRCxTQUFJLFFBQVEsaUJBQVcsS0FBSyxTQUFoQixDQUFaLEVBQXdDO0FBQUE7O0FBQ3RDLGNBQU8sMENBQVcsS0FBSyxTQUFoQixHQUEyQixVQUEzQix5Q0FBUDtBQUNEO0FBQ0QsWUFBTyxJQUFJLEtBQUosMkJBQWtDLEVBQWxDLE9BQVA7QUFDRCxJQVZEO0FBV0Q7O0FBRUQsRUFBQyxpQkFBRCxFQUFvQixpQkFBcEIsRUFBdUMsY0FBdkMsRUFBdUQsU0FBdkQsRUFBa0UsT0FBbEUsQ0FBMEUsV0FBMUU7O0FBRUE7Ozs7OztBQU1BLFVBQVMsYUFBVCxDQUF3QixVQUF4QixFQUFvQyxnQkFBcEMsRUFBc0Q7QUFDcEQsV0FBUSxnQkFBUixJQUE0QixZQUFtQjtBQUM3QyxTQUFNLHFEQUFOO0FBQ0EsU0FBTSxPQUFPLFlBQVksRUFBWixDQUFiO0FBQ0EsU0FBSSxRQUFRLGlCQUFXLEtBQUssU0FBaEIsQ0FBWixFQUF3QztBQUFBOztBQUN0QyxjQUFPLDJDQUFXLEtBQUssU0FBaEIsR0FBMkIsVUFBM0IsMENBQVA7QUFDRDtBQUNELFlBQU8sSUFBSSxLQUFKLDJCQUFrQyxFQUFsQyxPQUFQO0FBQ0QsSUFQRDtBQVFEOztBQUVELGVBQWMsY0FBZCxFQUE4QixRQUE5Qjs7bUJBRWUsTzs7Ozs7Ozs7Ozs7OztBQzlKZjs7S0FBWSxJOzs7O21CQUVHO0FBQ2I7QUFEYSxFLEVBSmYsZ0M7Ozs7Ozs7Ozs7OzttUENBQTs7OztTQTZCZ0IsSSxHQUFBLEk7U0FlQSxlLEdBQUEsZTtTQW9CQSxjLEdBQUEsYztTQXFCQSxlLEdBQUEsZTtTQWdCQSxlLEdBQUEsZTtTQWNBLGtCLEdBQUEsa0I7U0FxQkEsZSxHQUFBLGU7U0FVQSxlLEdBQUEsZTtTQWNBLE8sR0FBQSxPO1NBMkJBLFksR0FBQSxZOztBQXZMaEI7Ozs7QUFDQTs7OztBQUNBOztBQVFBOztBQUlBOzs7Ozs7OztLQUdFLGtCLG9CQUFBLGtCOztBQUVGLEtBQU0sY0FBYyxFQUFwQjs7QUFFQTs7OztBQUlPLFVBQVMsSUFBVCxDQUFlLEdBQWYsRUFBb0I7QUFDekIsb0JBQU8sUUFBUCxHQUFrQixJQUFJLFFBQXRCO0FBQ0Esb0JBQU8sT0FBUCxHQUFpQixJQUFJLE9BQXJCO0FBQ0Esb0JBQU8sT0FBUCxHQUFpQixJQUFJLE9BQXJCO0FBQ0Esb0JBQU8sU0FBUCxHQUFtQixJQUFJLFNBQXZCO0FBQ0Q7O0FBRUQ7Ozs7Ozs7O0FBUU8sVUFBUyxlQUFULENBQTBCLEVBQTFCLEVBQThCLE9BQTlCLEVBQXVDLElBQXZDLEVBQTZDO0FBQ2xELE9BQUksV0FBVyxZQUFZLEVBQVosQ0FBZjtBQUNBLGFBQVUsV0FBVyxFQUFyQjtBQUNBLE9BQUksZUFBSjtBQUNBLE9BQUksQ0FBQyxRQUFMLEVBQWU7QUFDYixnQkFBVyxrQkFBUSxFQUFSLEVBQVksT0FBWixDQUFYO0FBQ0EsaUJBQVksRUFBWixJQUFrQixRQUFsQjtBQUNBLGNBQVMsZ0JBQVEsUUFBUixFQUFrQixFQUFsQixFQUFzQixJQUF0QixDQUFUO0FBQ0Q7QUFDRCxVQUFPLFVBQVUsRUFBakI7QUFDRDs7QUFFRDs7Ozs7Ozs7QUFRTyxVQUFTLGNBQVQsQ0FBeUIsRUFBekIsRUFBNkIsSUFBN0IsRUFBbUMsT0FBbkMsRUFBNEMsSUFBNUMsRUFBa0Q7QUFDdkQsT0FBSSxXQUFXLFlBQVksRUFBWixDQUFmO0FBQ0EsYUFBVSxXQUFXLEVBQXJCO0FBQ0EsT0FBSSxlQUFKO0FBQ0EsT0FBSSxDQUFDLFFBQUwsRUFBZTtBQUNiLGdCQUFXLGtCQUFRLEVBQVIsRUFBWSxPQUFaLENBQVg7QUFDQSxpQkFBWSxFQUFaLElBQWtCLFFBQWxCO0FBQ0EsY0FBUyxnQkFBUSxRQUFSLEVBQWtCLElBQWxCLEVBQXdCLElBQXhCLENBQVQ7QUFDRCxJQUpELE1BS0s7QUFDSCxjQUFTLElBQUksS0FBSiwyQkFBa0MsRUFBbEMsT0FBVDtBQUNEO0FBQ0QsVUFBTyxNQUFQO0FBQ0Q7O0FBRUQ7Ozs7OztBQU1PLFVBQVMsZUFBVCxDQUEwQixFQUExQixFQUE4QixJQUE5QixFQUFvQztBQUN6QyxPQUFNLFdBQVcsWUFBWSxFQUFaLENBQWpCO0FBQ0EsT0FBSSxlQUFKO0FBQ0EsT0FBSSxRQUFKLEVBQWM7QUFDWixjQUFTLG1CQUFRLFFBQVIsRUFBa0IsSUFBbEIsQ0FBVDtBQUNELElBRkQsTUFHSztBQUNILGNBQVMsSUFBSSxLQUFKLDJCQUFrQyxFQUFsQyxPQUFUO0FBQ0Q7QUFDRCxVQUFPLE1BQVA7QUFDRDs7QUFFRDs7OztBQUlPLFVBQVMsZUFBVCxDQUEwQixFQUExQixFQUE4QjtBQUNuQyxPQUFNLFdBQVcsWUFBWSxFQUFaLENBQWpCO0FBQ0EsT0FBSSxDQUFDLFFBQUwsRUFBZTtBQUNiLFlBQU8sSUFBSSxLQUFKLDJCQUFrQyxFQUFsQyxPQUFQO0FBQ0Q7QUFDRCxzQkFBUSxRQUFSO0FBQ0EsVUFBTyxZQUFZLEVBQVosQ0FBUDtBQUNBLFVBQU8sV0FBUDtBQUNEOztBQUVEOzs7O0FBSU8sVUFBUyxrQkFBVCxDQUE2QixVQUE3QixFQUF5QztBQUM5QyxPQUFJLE1BQU0sT0FBTixDQUFjLFVBQWQsQ0FBSixFQUErQjtBQUM3QixnQkFBVyxPQUFYLENBQW1CLFNBQVMsUUFBVCxDQUFtQixJQUFuQixFQUF5QjtBQUMxQztBQUNBLFdBQUksQ0FBQyxJQUFMLEVBQVc7QUFDVDtBQUNEO0FBQ0QsV0FBSSxPQUFPLElBQVAsS0FBZ0IsUUFBcEIsRUFBOEI7QUFDNUIsNEJBQW1CLElBQW5CLElBQTJCLElBQTNCO0FBQ0QsUUFGRCxNQUdLLElBQUksUUFBTyxJQUFQLHlDQUFPLElBQVAsT0FBZ0IsUUFBaEIsSUFBNEIsT0FBTyxLQUFLLElBQVosS0FBcUIsUUFBckQsRUFBK0Q7QUFDbEUsNEJBQW1CLEtBQUssSUFBeEIsSUFBZ0MsSUFBaEM7QUFDRDtBQUNGLE1BWEQ7QUFZRDtBQUNGOztBQUVEOzs7O0FBSU8sVUFBUyxlQUFULENBQTBCLE9BQTFCLEVBQW1DO0FBQ3hDLE9BQUksUUFBTyxPQUFQLHlDQUFPLE9BQVAsT0FBbUIsUUFBdkIsRUFBaUM7QUFDL0IsZ0NBQVksT0FBWjtBQUNEO0FBQ0Y7O0FBRUQ7Ozs7QUFJTyxVQUFTLGVBQVQsQ0FBMEIsT0FBMUIsRUFBbUM7QUFDeEMsT0FBSSxRQUFPLE9BQVAseUNBQU8sT0FBUCxPQUFtQixRQUF2QixFQUFpQztBQUMvQiw4Q0FBZ0IsT0FBaEI7QUFDRDtBQUNGOztBQUVEO0FBQ0EsUUFBTyxlQUFQLEdBQXlCLGVBQXpCOztBQUVBOzs7OztBQUtPLFVBQVMsT0FBVCxDQUFrQixFQUFsQixFQUFzQjtBQUMzQixPQUFNLFdBQVcsWUFBWSxFQUFaLENBQWpCO0FBQ0EsT0FBSSxlQUFKO0FBQ0EsT0FBSSxRQUFKLEVBQWM7QUFDWixjQUFTLDBCQUFlLFFBQWYsQ0FBVDtBQUNELElBRkQsTUFHSztBQUNILGNBQVMsSUFBSSxLQUFKLDJCQUFrQyxFQUFsQyxPQUFUO0FBQ0Q7QUFDRCxVQUFPLE1BQVA7QUFDRDs7QUFFRCxLQUFNLGFBQWE7QUFDakIsY0FBVyxtQkFBQyxFQUFELEVBQWlCO0FBQUEsdUNBQVQsSUFBUztBQUFULFdBQVM7QUFBQTs7QUFDMUIsWUFBTyxrQ0FBVSxZQUFZLEVBQVosQ0FBVixTQUE4QixJQUE5QixFQUFQO0FBQ0QsSUFIZ0I7QUFJakIsYUFBVSxrQkFBQyxFQUFELEVBQWlCO0FBQUEsd0NBQVQsSUFBUztBQUFULFdBQVM7QUFBQTs7QUFDekIsWUFBTyxpQ0FBUyxZQUFZLEVBQVosQ0FBVCxTQUE2QixJQUE3QixFQUFQO0FBQ0Q7QUFOZ0IsRUFBbkI7O0FBU0E7Ozs7OztBQU1PLFVBQVMsWUFBVCxDQUF1QixFQUF2QixFQUEyQixLQUEzQixFQUFrQztBQUN2QyxPQUFNLFdBQVcsWUFBWSxFQUFaLENBQWpCO0FBQ0EsT0FBSSxZQUFZLE1BQU0sT0FBTixDQUFjLEtBQWQsQ0FBaEIsRUFBc0M7QUFBQTtBQUNwQyxXQUFNLFVBQVUsRUFBaEI7QUFDQSxhQUFNLE9BQU4sQ0FBYyxVQUFDLElBQUQsRUFBVTtBQUN0QixhQUFNLFVBQVUsV0FBVyxLQUFLLE1BQWhCLENBQWhCO0FBQ0EsYUFBTSxvQ0FBVyxLQUFLLElBQWhCLEVBQU47QUFDQSxhQUFJLE9BQU8sT0FBUCxLQUFtQixVQUF2QixFQUFtQztBQUNqQyxnQkFBSyxPQUFMLENBQWEsRUFBYjtBQUNBLG1CQUFRLElBQVIsQ0FBYSw0Q0FBVyxJQUFYLEVBQWI7QUFDRDtBQUNGLFFBUEQ7QUFRQTtBQUFBLFlBQU87QUFBUDtBQVZvQzs7QUFBQTtBQVdyQztBQUNELFVBQU8sSUFBSSxLQUFKLDJCQUFrQyxFQUFsQyxnQkFBUDtBQUNELEU7Ozs7Ozs7Ozs7OztBQzFNRDttQkFDZTtBQUNiLHVCQUFvQjtBQUNsQixXQUFNLElBRFk7QUFFbEIsWUFBTyxJQUZXO0FBR2xCLGdCQUFXLElBSE87QUFJbEIsYUFBUTtBQUNOLGFBQU0sUUFEQTtBQUVOLGVBQVE7QUFGRixNQUpVO0FBUWxCLFdBQU07QUFDSixhQUFNLE1BREY7QUFFSixlQUFRO0FBRko7QUFSWTtBQURQLEU7Ozs7Ozs7Ozs7O21CQ2VTLEc7O0FBWHhCOzs7O0FBRUE7Ozs7QUFDQTs7QUFDQTs7OztBQUVBOzs7OztBQVhBOzs7OztBQWdCZSxVQUFTLEdBQVQsQ0FBYyxFQUFkLEVBQWtCLE9BQWxCLEVBQTJCO0FBQ3hDLFFBQUssRUFBTCxHQUFVLEVBQVY7QUFDQSxRQUFLLE9BQUwsR0FBZSxXQUFXLEVBQTFCO0FBQ0EsUUFBSyxFQUFMLEdBQVUsSUFBVjtBQUNBLFFBQUssa0JBQUwsR0FBMEIsRUFBMUI7QUFDQSxRQUFLLFNBQUwsR0FBaUIsRUFBakI7QUFDQSxRQUFLLEdBQUwsR0FBVyxJQUFJLGlCQUFTLFFBQWIsQ0FBc0IsRUFBdEIsRUFBMEIsS0FBSyxPQUFMLENBQWEsU0FBdkMsQ0FBWDtBQUNBLFFBQUssTUFBTCxHQUFjLHFCQUFXLEVBQVgsQ0FBZDtBQUNBLFFBQUssR0FBTCxHQUFXLENBQVg7QUFDRDs7QUFFRDs7O0FBR0EsS0FBSSxTQUFKLENBQWMsYUFBZCxHQUE4QixVQUFVLElBQVYsRUFBZ0I7QUFDNUMsVUFBTyw2QkFBYyxJQUFkLEVBQW9CLElBQXBCLENBQVA7QUFDRCxFQUZEOztBQUlBOzs7QUFHQSxLQUFJLFNBQUosQ0FBYyxhQUFkLEdBQThCLFlBQVk7QUFDeEMsNEJBQWMsSUFBZDtBQUNELEVBRkQ7O0FBSUE7OztBQUdBLEtBQUksU0FBSixDQUFjLFNBQWQsR0FBMEIsVUFBVSxLQUFWLEVBQWlCO0FBQ3pDLHdCQUFVLElBQVYsRUFBZ0IsS0FBaEI7QUFDRCxFQUZELEM7Ozs7Ozs7Ozs7Ozs7Ozs7S0M1Q3FCLE07QUFDbkIsbUJBQWEsRUFBYixFQUFpQjtBQUFBOztBQUNmLFVBQUssRUFBTCxHQUFVLEVBQVY7QUFDQSxVQUFLLEdBQUwsR0FBVyxFQUFYO0FBQ0EsVUFBSyxLQUFMLEdBQWEsRUFBYjtBQUNEOzs7OytCQUNVO0FBQ1QsY0FBTyxLQUFLLEdBQUwsQ0FBUyxNQUFULEtBQW9CLENBQTNCO0FBQ0Q7Ozs0QkFDTyxJLEVBQU0sSyxFQUFPLEcsRUFBSyxPLEVBQVM7QUFBQTs7QUFDakMsV0FBSSxDQUFDLEtBQUssUUFBVixFQUFvQjtBQUNsQixjQUFLLFFBQUwsR0FBZ0IsSUFBaEI7QUFDQSxvQkFBVyxZQUFNO0FBQ2YsaUJBQUssUUFBTCxHQUFnQixLQUFoQjtBQUNBLGlCQUFLLEtBQUwsQ0FBVyxJQUFYO0FBQ0QsVUFIRCxFQUdHLENBSEg7QUFJRDtBQUNELFdBQU0sTUFBTSxLQUFLLEdBQWpCO0FBQ0EsV0FBSSxDQUFDLElBQUksS0FBSixDQUFMLEVBQWlCO0FBQ2YsYUFBSSxLQUFKLElBQWEsRUFBYjtBQUNEO0FBQ0QsV0FBTSxRQUFRLElBQUksS0FBSixDQUFkO0FBQ0EsV0FBSSxDQUFDLE1BQU0sSUFBTixDQUFMLEVBQWtCO0FBQ2hCLGVBQU0sSUFBTixJQUFjLEVBQWQ7QUFDRDtBQUNELFdBQUksU0FBUyxTQUFiLEVBQXdCO0FBQ3RCLGFBQUksQ0FBQyxNQUFNLElBQU4sRUFBWSxHQUFaLENBQUwsRUFBdUI7QUFDckIsaUJBQU0sSUFBTixFQUFZLEdBQVosSUFBbUIsRUFBbkI7QUFDRDtBQUNELGVBQU0sSUFBTixFQUFZLEdBQVosRUFBaUIsSUFBakIsQ0FBc0IsT0FBdEI7QUFDRCxRQUxELE1BTUs7QUFDSCxlQUFNLElBQU4sRUFBWSxHQUFaLElBQW1CLE9BQW5CO0FBQ0Q7QUFDRjs7OzJCQUNNLFMsRUFBVztBQUNoQixXQUFNLE1BQU0sS0FBSyxHQUFMLENBQVMsS0FBVCxFQUFaO0FBQ0EsWUFBSyxHQUFMLENBQVMsTUFBVCxHQUFrQixDQUFsQjtBQUNBLFdBQUksT0FBSixDQUFZLFVBQUMsS0FBRCxFQUFXO0FBQ3JCLHFCQUFZLEtBQVosRUFBbUIsUUFBbkI7QUFDQSxxQkFBWSxLQUFaLEVBQW1CLE9BQW5CO0FBQ0Esc0JBQWEsS0FBYixFQUFvQixTQUFwQjtBQUNELFFBSkQ7O0FBTUEsV0FBTSxRQUFRLEtBQUssS0FBTCxDQUFXLEtBQVgsRUFBZDtBQUNBLFlBQUssS0FBTCxDQUFXLE1BQVgsR0FBb0IsQ0FBcEI7QUFDQSxhQUFNLE9BQU4sQ0FBYyxVQUFDLEVBQUQsRUFBUTtBQUNwQjtBQUNELFFBRkQ7O0FBSUEsV0FBSSxDQUFDLEtBQUssT0FBTCxFQUFMLEVBQXFCO0FBQ25CLGNBQUssS0FBTDtBQUNEO0FBQ0Y7OzswQkFDSyxFLEVBQUk7QUFDUixZQUFLLEtBQUwsQ0FBVyxJQUFYLENBQWdCLEVBQWhCO0FBQ0Q7Ozs7OzttQkF4RGtCLE07OztBQTJEckIsVUFBUyxXQUFULENBQXNCLEtBQXRCLEVBQTZCLElBQTdCLEVBQW1DO0FBQ2pDLE9BQU0sTUFBTSxNQUFNLElBQU4sQ0FBWjtBQUNBLFFBQUssSUFBTSxHQUFYLElBQWtCLEdBQWxCLEVBQXVCO0FBQ3JCLFNBQUksR0FBSjtBQUNEO0FBQ0Y7O0FBRUQsVUFBUyxZQUFULENBQXVCLEtBQXZCLEVBQThCLElBQTlCLEVBQW9DO0FBQ2xDLE9BQU0sTUFBTSxNQUFNLElBQU4sQ0FBWjtBQUNBLFFBQUssSUFBTSxHQUFYLElBQWtCLEdBQWxCLEVBQXVCO0FBQ3JCLFNBQU0sT0FBTyxJQUFJLEdBQUosQ0FBYjtBQUNBLFVBQUssT0FBTCxDQUFhLFVBQUMsT0FBRCxFQUFhO0FBQUU7QUFBVyxNQUF2QztBQUNEO0FBQ0YsRTs7Ozs7Ozs7Ozs7U0NqRWUsUyxHQUFBLFM7U0FPQSxZLEdBQUEsWTtTQVVBLFcsR0FBQSxXO1NBMkJBLFcsR0FBQSxXO1NBZUEsYSxHQUFBLGE7U0FnQkEsc0IsR0FBQSxzQjtTQVFBLHVCLEdBQUEsdUI7QUExRmhCLEtBQUksZ0JBQWdCLEVBQXBCOztBQUVBOztBQUVBOzs7QUFHTyxVQUFTLFNBQVQsQ0FBb0IsVUFBcEIsRUFBZ0M7QUFDckMsVUFBTyxjQUFjLFVBQWQsQ0FBUDtBQUNEOztBQUVEOzs7QUFHTyxVQUFTLFlBQVQsR0FBeUI7QUFDOUIsbUJBQWdCLEVBQWhCO0FBQ0Q7O0FBRUQ7O0FBRUE7Ozs7QUFJTyxVQUFTLFdBQVQsQ0FBc0IsT0FBdEIsRUFBK0IsU0FBL0IsRUFBMEM7QUFBQSw4QkFDcEMsVUFEb0M7QUFFN0M7QUFDQSxTQUFJLFVBQVUsY0FBYyxVQUFkLENBQWQ7QUFDQSxTQUFJLENBQUMsT0FBTCxFQUFjO0FBQ1osaUJBQVUsRUFBVjtBQUNBLHFCQUFjLFVBQWQsSUFBNEIsT0FBNUI7QUFDRDs7QUFFRDtBQUNBLGFBQVEsVUFBUixFQUFvQixPQUFwQixDQUE0QixVQUFVLE1BQVYsRUFBa0I7QUFDNUMsV0FBSSxPQUFPLE1BQVAsS0FBa0IsUUFBdEIsRUFBZ0M7QUFDOUIsa0JBQVM7QUFDUCxpQkFBTTtBQURDLFVBQVQ7QUFHRDs7QUFFRCxXQUFJLENBQUMsUUFBUSxPQUFPLElBQWYsQ0FBRCxJQUF5QixTQUE3QixFQUF3QztBQUN0QyxpQkFBUSxPQUFPLElBQWYsSUFBdUIsTUFBdkI7QUFDRDtBQUNGLE1BVkQ7QUFWNkM7O0FBQy9DLFFBQUssSUFBTSxVQUFYLElBQXlCLE9BQXpCLEVBQWtDO0FBQUEsV0FBdkIsVUFBdUI7QUFvQmpDO0FBQ0Y7O0FBRUQ7OztBQUdPLFVBQVMsV0FBVCxDQUFzQixFQUF0QixFQUEwQixJQUExQixFQUFnQztBQUNyQyxPQUFNLElBQUksR0FBRyxTQUFiOztBQUVBLFFBQUssSUFBTSxPQUFYLElBQXNCLElBQXRCLEVBQTRCO0FBQzFCLFNBQUksQ0FBQyxFQUFFLGNBQUYsQ0FBaUIsT0FBakIsQ0FBTCxFQUFnQztBQUM5QixTQUFFLE9BQUYsSUFBYSxLQUFLLE9BQUwsQ0FBYjtBQUNEO0FBQ0Y7QUFDRjs7QUFFRDs7QUFFQTs7O0FBR08sVUFBUyxhQUFULENBQXdCLEdBQXhCLEVBQTZCLElBQTdCLEVBQW1DO0FBQ3hDLE9BQU0sVUFBVSxjQUFjLElBQWQsQ0FBaEI7QUFDQSxPQUFNLFNBQVMsRUFBZjs7QUFGd0MsZ0NBRzdCLFVBSDZCO0FBSXRDLFlBQU8sVUFBUCxJQUFxQjtBQUFBLHlDQUFJLElBQUo7QUFBSSxhQUFKO0FBQUE7O0FBQUEsY0FBYSxJQUFJLFNBQUosQ0FBYztBQUM5QyxpQkFBUSxJQURzQztBQUU5QyxpQkFBUSxVQUZzQztBQUc5QyxlQUFNO0FBSHdDLFFBQWQsQ0FBYjtBQUFBLE1BQXJCO0FBSnNDOztBQUd4QyxRQUFLLElBQU0sVUFBWCxJQUF5QixPQUF6QixFQUFrQztBQUFBLFlBQXZCLFVBQXVCO0FBTWpDO0FBQ0QsVUFBTyxNQUFQO0FBQ0Q7O0FBRUQ7OztBQUdPLFVBQVMsc0JBQVQsQ0FBaUMsR0FBakMsRUFBc0MsSUFBdEMsRUFBNEM7QUFBQSxPQUN6QyxrQkFEeUMsR0FDbEIsR0FEa0IsQ0FDekMsa0JBRHlDOztBQUVqRCxVQUFPLG1CQUFtQixJQUFuQixDQUFQO0FBQ0Q7O0FBRUQ7OztBQUdPLFVBQVMsdUJBQVQsQ0FBa0MsR0FBbEMsRUFBdUMsSUFBdkMsRUFBNkMsR0FBN0MsRUFBa0Q7QUFBQSxPQUMvQyxrQkFEK0MsR0FDeEIsR0FEd0IsQ0FDL0Msa0JBRCtDOzs7QUFHdkQsT0FBSSxtQkFBbUIsSUFBbkIsQ0FBSixFQUE4QjtBQUM1QixhQUFRLEtBQVIsd0NBQW1ELElBQW5EO0FBQ0E7QUFDRDs7QUFFRCxzQkFBbUIsSUFBbkIsSUFBMkIsR0FBM0I7QUFDRCxFOzs7Ozs7Ozs7Ozs7OztTQ3RFZSxJLEdBQUEsSTtTQXlKQSxPLEdBQUEsTztTQXdCQSxPLEdBQUEsTztTQWlCQSxjLEdBQUEsYztTQWdCQSxTLEdBQUEsUztTQTJCQSxRLEdBQUEsUTtTQXNCQSxhLEdBQUEsYTtTQWlCQSxTLEdBQUEsUzs7QUFwU2hCOztBQUNBOzs7O0FBQ0E7Ozs7cU1BZkE7Ozs7Ozs7Ozs7Ozs7QUFxQkE7Ozs7Ozs7O0FBUU8sVUFBUyxJQUFULENBQWUsR0FBZixFQUFvQixJQUFwQixFQUEwQixJQUExQixFQUFnQztBQUNyQyxXQUFRLEtBQVIsQ0FBYyw4Q0FBZCxFQUE4RCxJQUE5RDtBQUNBLE9BQUksZUFBSjs7QUFFQTtBQUNBLE9BQU0sZUFBZSxTQUFmLFlBQWU7QUFBQSx1Q0FBSSxJQUFKO0FBQUksV0FBSjtBQUFBOztBQUFBLFlBQWEsbUNBQVMsR0FBVCxTQUFpQixJQUFqQixFQUFiO0FBQUEsSUFBckI7QUFDQSxPQUFNLGtCQUFrQixTQUFsQixlQUFrQixDQUFDLElBQUQsRUFBTyxNQUFQLEVBQWUsS0FBZixFQUF5QjtBQUMvQyxjQUFTLHVCQUFVLEdBQVYsRUFBZSxJQUFmLEVBQXFCLE1BQXJCLEVBQTZCLFNBQVMsSUFBdEMsQ0FBVDtBQUNBLG1CQUFjLEdBQWQ7QUFDQSxTQUFJLEdBQUosQ0FBUSxRQUFSLENBQWlCLFlBQWpCO0FBQ0EsYUFBUSxLQUFSLGtEQUE2RCxJQUFJLEVBQWpFO0FBQ0QsSUFMRDtBQU1BLE9BQU0saUJBQWlCLFNBQWpCLGNBQWlCO0FBQUEsd0NBQUksSUFBSjtBQUFJLFdBQUo7QUFBQTs7QUFBQSxZQUFhLG1DQUFTLEdBQVQsU0FBaUIsSUFBakIsRUFBYjtBQUFBLElBQXZCO0FBQ0EsT0FBTSxlQUFlLFNBQWYsWUFBZSxDQUFDLElBQUQsRUFBTyxLQUFQLEVBQWlCO0FBQ3BDLGNBQVMsdUJBQVUsR0FBVixFQUFlLElBQWYsRUFBcUIsRUFBckIsRUFBeUIsS0FBekIsQ0FBVDtBQUNELElBRkQ7QUFHQSxPQUFNLGdCQUFnQixTQUFoQixhQUFnQjtBQUFBLFlBQVEsaUJBQVM7QUFDckMsZ0JBQVMsdUJBQVUsR0FBVixFQUFlLElBQWYsRUFBcUIsRUFBckIsRUFBeUIsS0FBekIsQ0FBVDtBQUNELE1BRnFCO0FBQUEsSUFBdEI7QUFHQSxPQUFNLGlCQUFpQixJQUFJLEdBQTNCO0FBQ0EsT0FBTSxzQkFBc0IsU0FBdEIsbUJBQXNCO0FBQUEsWUFBUSxJQUFJLGFBQUosQ0FBa0IsNEJBQWlCLElBQWpCLENBQWxCLENBQVI7QUFBQSxJQUE1Qjs7QUFFQTtBQUNBLE9BQUkscUJBQUo7QUFDQTtBQUNBLE9BQUksT0FBTyxJQUFQLEtBQWdCLFVBQXBCLEVBQWdDO0FBQzlCO0FBQ0E7QUFDQSxvQkFBZSxLQUFLLFFBQUwsR0FBZ0IsTUFBaEIsQ0FBdUIsRUFBdkIsQ0FBZjtBQUNELElBSkQsTUFLSyxJQUFJLElBQUosRUFBVTtBQUNiLG9CQUFlLEtBQUssUUFBTCxFQUFmO0FBQ0Q7O0FBRUQ7QUFsQ3FDLGlCQW1DWCxNQW5DVztBQUFBLE9BbUM3QixhQW5DNkIsV0FtQzdCLGFBbkM2Qjs7QUFvQ3JDLE9BQUksaUJBQWlCLGNBQWMsUUFBZCxLQUEyQixLQUFoRCxFQUF1RDtBQUFBO0FBQ3JEO0FBQ0EsV0FBTSxRQUFRLElBQUksYUFBSixDQUFrQixPQUFsQixDQUFkO0FBQ0EsV0FBTSxZQUFZO0FBQ2hCLHFCQUFZLHNCQUFhO0FBQUEsOENBQVQsSUFBUztBQUFULGlCQUFTO0FBQUE7O0FBQ3ZCLGVBQU0sVUFBVSxTQUFWLE9BQVUsR0FBWTtBQUMxQixrQkFBSyxDQUFMLGlDQUFXLEtBQUssS0FBTCxDQUFXLENBQVgsQ0FBWDtBQUNELFlBRkQ7QUFHQSxpQkFBTSxVQUFOLENBQWlCLE9BQWpCLEVBQTBCLEtBQUssQ0FBTCxDQUExQjtBQUNBLGtCQUFPLElBQUksR0FBSixDQUFRLFFBQVIsRUFBUDtBQUNELFVBUGU7QUFRaEIsc0JBQWEsdUJBQWE7QUFBQSw4Q0FBVCxJQUFTO0FBQVQsaUJBQVM7QUFBQTs7QUFDeEIsZUFBTSxVQUFVLFNBQVYsT0FBVSxHQUFZO0FBQzFCLGtCQUFLLENBQUwsaUNBQVcsS0FBSyxLQUFMLENBQVcsQ0FBWCxDQUFYO0FBQ0QsWUFGRDtBQUdBLGlCQUFNLFdBQU4sQ0FBa0IsT0FBbEIsRUFBMkIsS0FBSyxDQUFMLENBQTNCO0FBQ0Esa0JBQU8sSUFBSSxHQUFKLENBQVEsUUFBUixFQUFQO0FBQ0QsVUFkZTtBQWVoQix1QkFBYyxzQkFBQyxDQUFELEVBQU87QUFDbkIsaUJBQU0sWUFBTixDQUFtQixDQUFuQjtBQUNELFVBakJlO0FBa0JoQix3QkFBZSx1QkFBQyxDQUFELEVBQU87QUFDcEIsaUJBQU0sYUFBTixDQUFvQixDQUFwQjtBQUNEO0FBcEJlLFFBQWxCOztBQXVCQTtBQUNBO0FBQ0EsV0FBSSxDQUFDLElBQUwsRUFBVztBQUNUO0FBQUEsY0FBTztBQUNMLHFCQUFRLFlBREg7QUFFTCxzQkFBUyxhQUZKO0FBR0wsdUJBQVUsY0FITDtBQUlMLHdCQUFXLGVBSk47QUFLTCx1QkFBVSxjQUxMO0FBTUwscUJBQVEsWUFOSDtBQU9MLDhCQUFpQixZQVBaO0FBUUwsaUNBQW9CLGVBUmY7QUFTTCxnQ0FBbUIsY0FUZDtBQVVMLCtCQUFrQixtQkFWYjtBQVdMLHlCQUFZLFVBQVUsVUFYakI7QUFZTCwwQkFBYSxVQUFVLFdBWmxCO0FBYUwsMkJBQWMsVUFBVSxZQWJuQjtBQWNMLDRCQUFlLFVBQVU7QUFkcEI7QUFBUDtBQWdCRDs7QUFFRCxXQUFNLEtBQUssSUFBSSxRQUFKLENBQ1QsUUFEUyxFQUVULFNBRlMsRUFHVCxVQUhTLEVBSVQsV0FKUyxFQUtULFVBTFMsRUFNVCxRQU5TLEVBT1QsaUJBUFMsRUFPVTtBQUNuQiwyQkFSUyxFQVFhO0FBQ3RCLDBCQVRTLEVBU1k7QUFDckIseUJBVlMsRUFXVCxZQVhTLEVBWVQsYUFaUyxFQWFULGNBYlMsRUFjVCxlQWRTLEVBZVQsWUFmUyxDQUFYOztBQWtCQSxVQUNFLFlBREYsRUFFRSxhQUZGLEVBR0UsY0FIRixFQUlFLGVBSkYsRUFLRSxjQUxGLEVBTUUsWUFORixFQU9FLFlBUEYsRUFRRSxlQVJGLEVBU0UsY0FURixFQVVFLG1CQVZGLEVBV0UsVUFBVSxVQVhaLEVBWUUsVUFBVSxXQVpaLEVBYUUsVUFBVSxZQWJaLEVBY0UsVUFBVSxhQWRaO0FBakVxRDs7QUFBQTtBQWdGdEQsSUFoRkQsTUFpRks7QUFDSCxTQUFNLE1BQUssSUFBSSxRQUFKLENBQ1QsUUFEUyxFQUVULFNBRlMsRUFHVCxVQUhTLEVBSVQsV0FKUyxFQUtULFVBTFMsRUFNVCxRQU5TLEVBT1QsaUJBUFMsRUFPVTtBQUNuQix5QkFSUyxFQVFhO0FBQ3RCLHdCQVRTLEVBU1k7QUFDckIsdUJBVlMsRUFXVCxZQVhTLENBQVg7O0FBY0EsU0FDRSxZQURGLEVBRUUsYUFGRixFQUdFLGNBSEYsRUFJRSxlQUpGLEVBS0UsY0FMRixFQU1FLFlBTkYsRUFPRSxZQVBGLEVBUUUsZUFSRixFQVNFLGNBVEYsRUFVRSxtQkFWRjtBQVdEOztBQUVELFVBQU8sTUFBUDtBQUNEOztBQUVEOzs7OztBQUtPLFVBQVMsT0FBVCxDQUFrQixHQUFsQixFQUF1QixJQUF2QixFQUE2QjtBQUNsQyxXQUFRLEtBQVIsZ0NBQTZDLElBQTdDLG1CQUN5QixJQUFJLEVBRDdCO0FBRUEsT0FBTSxLQUFLLElBQUksRUFBZjtBQUNBLE9BQUksTUFBTSxJQUFWLEVBQWdCO0FBQ2QsU0FBSSxHQUFKLENBQVEsS0FBUjtBQUNBLFNBQUksT0FBTyxHQUFHLFdBQVYsS0FBMEIsVUFBOUIsRUFBMEM7QUFDeEMsVUFBRyxXQUFILENBQWUsSUFBZjtBQUNELE1BRkQsTUFHSztBQUNILHlCQUFPLEVBQVAsRUFBVyxJQUFYO0FBQ0Q7QUFDRCxtQkFBYyxHQUFkO0FBQ0EsU0FBSSxHQUFKLENBQVEsUUFBUixDQUFpQixhQUFqQjtBQUNBLFNBQUksR0FBSixDQUFRLElBQVI7QUFDQTtBQUNEO0FBQ0QsVUFBTyxJQUFJLEtBQUosb0JBQTJCLElBQTNCLE9BQVA7QUFDRDs7QUFFRDs7OztBQUlPLFVBQVMsT0FBVCxDQUFrQixHQUFsQixFQUF1QjtBQUM1QixXQUFRLEtBQVIseUNBQW9ELElBQUksRUFBeEQ7O0FBRUEsT0FBSSxFQUFKLEdBQVMsRUFBVDtBQUNBLE9BQUksT0FBSixHQUFjLElBQWQ7QUFDQSxPQUFJLE1BQUosR0FBYSxJQUFiO0FBQ0EsT0FBSSxFQUFKLEdBQVMsSUFBVDtBQUNBLE9BQUksR0FBSixHQUFVLElBQVY7QUFDQSxPQUFJLGtCQUFKLEdBQXlCLElBQXpCO0FBQ0EsT0FBSSxTQUFKLEdBQWdCLElBQWhCO0FBQ0Q7O0FBRUQ7Ozs7O0FBS08sVUFBUyxjQUFULENBQXlCLEdBQXpCLEVBQThCO0FBQ25DLE9BQU0sTUFBTSxJQUFJLEdBQUosSUFBVyxFQUF2QjtBQUNBLE9BQU0sT0FBTyxJQUFJLElBQUosSUFBWSxFQUF6QjtBQUNBLFVBQU8sS0FBSyxNQUFMLEdBQWMsS0FBSyxNQUFMLEVBQWQsR0FBOEIsRUFBckM7QUFDRDs7QUFFRDs7Ozs7Ozs7OztBQVVPLFVBQVMsU0FBVCxDQUFvQixHQUFwQixFQUF5QixHQUF6QixFQUE4QixJQUE5QixFQUFvQyxDQUFwQyxFQUF1QyxVQUF2QyxFQUFtRDtBQUN4RCxXQUFRLEtBQVIsNkJBQXdDLElBQXhDLDhCQUFxRSxHQUFyRSxzQkFBeUYsSUFBSSxFQUE3RjtBQUNBLE9BQUksTUFBTSxPQUFOLENBQWMsR0FBZCxDQUFKLEVBQXdCO0FBQ3RCLFNBQUksSUFBSixDQUFTLFVBQUMsR0FBRCxFQUFTO0FBQ2hCLGNBQU8sSUFBSSxTQUFKLENBQWMsR0FBZCxFQUFtQixJQUFuQixFQUF5QixDQUF6QixNQUFnQyxLQUF2QztBQUNELE1BRkQ7QUFHQTtBQUNEO0FBQ0QsT0FBTSxLQUFLLElBQUksR0FBSixDQUFRLE1BQVIsQ0FBZSxHQUFmLENBQVg7QUFDQSxPQUFJLEVBQUosRUFBUTtBQUNOLFNBQUksR0FBSixDQUFRLEtBQVI7QUFDQSxTQUFNLFNBQVMsSUFBSSxHQUFKLENBQVEsU0FBUixDQUFrQixFQUFsQixFQUFzQixJQUF0QixFQUE0QixDQUE1QixFQUErQixVQUEvQixDQUFmO0FBQ0EsbUJBQWMsR0FBZDtBQUNBLFNBQUksR0FBSixDQUFRLFFBQVIsQ0FBaUIsWUFBakI7QUFDQSxTQUFJLEdBQUosQ0FBUSxJQUFSO0FBQ0EsWUFBTyxNQUFQO0FBQ0Q7QUFDRCxVQUFPLElBQUksS0FBSixpQ0FBd0MsR0FBeEMsT0FBUDtBQUNEOztBQUVEOzs7Ozs7O0FBT08sVUFBUyxRQUFULENBQW1CLEdBQW5CLEVBQXdCLFVBQXhCLEVBQW9DLElBQXBDLEVBQTBDLFdBQTFDLEVBQXVEO0FBQzVELFdBQVEsS0FBUix1Q0FBa0QsVUFBbEQsYUFBc0UsSUFBdEUsbUJBQ3lCLElBQUksRUFEN0I7QUFFQSxPQUFNLFdBQVcsSUFBSSxTQUFKLENBQWMsVUFBZCxDQUFqQjtBQUNBLE9BQUksT0FBTyxRQUFQLEtBQW9CLFVBQXhCLEVBQW9DO0FBQ2xDLFNBQUksR0FBSixDQUFRLEtBQVI7QUFDQSxjQUFTLElBQVQ7QUFDQSxTQUFJLE9BQU8sV0FBUCxLQUF1QixXQUF2QixJQUFzQyxnQkFBZ0IsS0FBMUQsRUFBaUU7QUFDL0QsV0FBSSxTQUFKLENBQWMsVUFBZCxJQUE0QixTQUE1QjtBQUNEO0FBQ0QsbUJBQWMsR0FBZDtBQUNBLFNBQUksR0FBSixDQUFRLFFBQVIsQ0FBaUIsWUFBakI7QUFDQSxTQUFJLEdBQUosQ0FBUSxJQUFSO0FBQ0E7QUFDRDtBQUNELFVBQU8sSUFBSSxLQUFKLDJCQUFrQyxVQUFsQyxPQUFQO0FBQ0Q7O0FBRUQ7Ozs7QUFJTyxVQUFTLGFBQVQsQ0FBd0IsR0FBeEIsRUFBNkI7QUFDbEMsT0FBSSxNQUFKLENBQVcsS0FBWDtBQUNBLE9BQU0sUUFBUSxFQUFkO0FBQ0EsT0FBSSxJQUFJLEdBQUosSUFBVyxJQUFJLEdBQUosQ0FBUSxRQUFuQixJQUErQixJQUFJLEdBQUosQ0FBUSxRQUFSLENBQWlCLE9BQWpCLENBQXlCLE1BQTVELEVBQW9FO0FBQ2xFLFdBQU0sSUFBTixpQ0FBYyxJQUFJLEdBQUosQ0FBUSxRQUFSLENBQWlCLE9BQS9CO0FBQ0EsU0FBSSxHQUFKLENBQVEsUUFBUixDQUFpQixPQUFqQixHQUEyQixFQUEzQjtBQUNEO0FBQ0QsT0FBSSxNQUFNLE1BQVYsRUFBa0I7QUFDaEIsWUFBTyxVQUFVLEdBQVYsRUFBZSxLQUFmLENBQVA7QUFDRDtBQUNGOztBQUVEOzs7OztBQUtPLFVBQVMsU0FBVCxDQUFvQixHQUFwQixFQUF5QixLQUF6QixFQUFnQztBQUNyQyxPQUFJLGlCQUFNLEtBQU4sTUFBaUIsT0FBckIsRUFBOEI7QUFDNUIsYUFBUSxDQUFDLEtBQUQsQ0FBUjtBQUNEOztBQUVELFNBQU0sT0FBTixDQUFjLFVBQUMsSUFBRCxFQUFVO0FBQ3RCLFVBQUssSUFBTCxHQUFZLEtBQUssSUFBTCxDQUFVLEdBQVYsQ0FBYztBQUFBLGNBQU8sVUFBVSxHQUFWLEVBQWUsR0FBZixDQUFQO0FBQUEsTUFBZCxDQUFaO0FBQ0QsSUFGRDs7QUFJQSxVQUFPLGlCQUFTLFNBQVQsQ0FBbUIsSUFBSSxFQUF2QixFQUEyQixLQUEzQixFQUFrQyxJQUFsQyxDQUFQO0FBQ0Q7O0FBRUQ7Ozs7Ozs7QUFPQSxVQUFTLFNBQVQsQ0FBb0IsQ0FBcEIsRUFBdUIsR0FBdkIsRUFBNEI7QUFDMUIsT0FBTSxPQUFPLGlCQUFNLENBQU4sQ0FBYjs7QUFFQSxXQUFRLElBQVI7QUFDRSxVQUFLLFdBQUw7QUFDQSxVQUFLLE1BQUw7QUFDRSxjQUFPLEVBQVA7QUFDRixVQUFLLFFBQUw7QUFDRSxjQUFPLEVBQUUsUUFBRixFQUFQO0FBQ0YsVUFBSyxNQUFMO0FBQ0UsY0FBTyxFQUFFLFdBQUYsRUFBUDtBQUNGLFVBQUssUUFBTDtBQUNBLFVBQUssUUFBTDtBQUNBLFVBQUssU0FBTDtBQUNBLFVBQUssT0FBTDtBQUNBLFVBQUssUUFBTDtBQUNFLFdBQUksYUFBYSxpQkFBUyxPQUExQixFQUFtQztBQUNqQyxnQkFBTyxFQUFFLEdBQVQ7QUFDRDtBQUNELGNBQU8sQ0FBUDtBQUNGLFVBQUssVUFBTDtBQUNFLFdBQUksU0FBSixDQUFjLEVBQUUsSUFBSSxHQUFwQixJQUEyQixDQUEzQjtBQUNBLGNBQU8sSUFBSSxHQUFKLENBQVEsUUFBUixFQUFQO0FBQ0Y7QUFDRSxjQUFPLEtBQUssU0FBTCxDQUFlLENBQWYsQ0FBUDtBQXJCSjtBQXVCRCxFOzs7Ozs7Ozs7Ozs7Ozs7Ozs7b0JDN1ZDLE07Ozs7OztvQkFDQSxHOzs7Ozs7b0JBQ0EsTTs7Ozs7O29CQUNBLE07Ozs7OztvQkFDQSxJOzs7Ozs7b0JBQ0EsTzs7Ozs7O29CQUNBLFE7Ozs7OztvQkFDQSxhOzs7U0FVYyxVLEdBQUEsVTtTQXVDQSxNLEdBQUEsTTtTQXNDQSxLLEdBQUEsSztTQWlCQSxnQixHQUFBLGdCO1NBS0EsYyxHQUFBLGM7OztBQTFHaEI7Ozs7Ozs7QUFPTyxVQUFTLFVBQVQsQ0FBcUIsR0FBckIsRUFBMEI7QUFDL0IsT0FBTSxJQUFJLENBQUMsTUFBTSxFQUFQLEVBQVcsVUFBWCxDQUFzQixDQUF0QixDQUFWO0FBQ0EsVUFBTyxNQUFNLElBQU4sSUFBYyxNQUFNLElBQTNCO0FBQ0Q7O0FBRUQ7QUFDTyxLQUFNLDhCQUFXLGVBQWUsRUFBaEM7O0FBRVAsS0FBSSxhQUFKO0FBQ0E7QUFDQSxLQUFJLE9BQU8sR0FBUCxLQUFlLFdBQWYsSUFBOEIsSUFBSSxRQUFKLEdBQWUsS0FBZixDQUFxQixhQUFyQixDQUFsQyxFQUF1RTtBQUNyRTtBQUNBLFdBa0JPLElBbEJQLFVBQU8sR0FBUDtBQUNELEVBSEQsTUFJSztBQUNIO0FBQ0EsV0FjTyxJQWRQLFVBQU8sZ0JBQVk7QUFDakIsVUFBSyxHQUFMLEdBQVcsT0FBTyxNQUFQLENBQWMsSUFBZCxDQUFYO0FBQ0QsSUFGRDtBQUdBLFFBQUssU0FBTCxDQUFlLEdBQWYsR0FBcUIsVUFBVSxHQUFWLEVBQWU7QUFDbEMsWUFBTyxLQUFLLEdBQUwsQ0FBUyxHQUFULE1BQWtCLFNBQXpCO0FBQ0QsSUFGRDtBQUdBLFFBQUssU0FBTCxDQUFlLEdBQWYsR0FBcUIsVUFBVSxHQUFWLEVBQWU7QUFDbEMsVUFBSyxHQUFMLENBQVMsR0FBVCxJQUFnQixDQUFoQjtBQUNELElBRkQ7QUFHQSxRQUFLLFNBQUwsQ0FBZSxLQUFmLEdBQXVCLFlBQVk7QUFDakMsVUFBSyxHQUFMLEdBQVcsT0FBTyxNQUFQLENBQWMsSUFBZCxDQUFYO0FBQ0QsSUFGRDtBQUdEOztTQUVRLEksR0FBQSxJOztBQUVUOzs7Ozs7O0FBT08sVUFBUyxNQUFULENBQWlCLEVBQWpCLEVBQXFCO0FBQzFCLE9BQU0sUUFBUSxPQUFPLE1BQVAsQ0FBYyxJQUFkLENBQWQ7QUFDQSxVQUFPLFNBQVMsUUFBVCxDQUFtQixHQUFuQixFQUF3QjtBQUM3QixTQUFNLE1BQU0sTUFBTSxHQUFOLENBQVo7QUFDQSxZQUFPLFFBQVEsTUFBTSxHQUFOLElBQWEsR0FBRyxHQUFILENBQXJCLENBQVA7QUFDRCxJQUhEO0FBSUQ7O0FBRUQ7Ozs7Ozs7QUFPQSxLQUFNLGFBQWEsUUFBbkI7QUFDTyxLQUFNLDhCQUFXLE9BQU8sZUFBTztBQUNwQyxVQUFPLElBQUksT0FBSixDQUFZLFVBQVosRUFBd0IsT0FBeEIsQ0FBUDtBQUNELEVBRnVCLENBQWpCOztBQUlQLFVBQVMsT0FBVCxDQUFrQixDQUFsQixFQUFxQixDQUFyQixFQUF3QjtBQUN0QixVQUFPLElBQUksRUFBRSxXQUFGLEVBQUosR0FBc0IsRUFBN0I7QUFDRDs7QUFFRDs7Ozs7OztBQU9BLEtBQU0sY0FBYyxtQkFBcEI7QUFDTyxLQUFNLGdDQUFZLE9BQU8sZUFBTztBQUNyQyxVQUFPLElBQ0osT0FESSxDQUNJLFdBREosRUFDaUIsT0FEakIsRUFFSixXQUZJLEVBQVA7QUFHRCxFQUp3QixDQUFsQjs7QUFNQSxVQUFTLEtBQVQsQ0FBZ0IsQ0FBaEIsRUFBbUI7QUFDeEIsT0FBTSxJQUFJLE9BQU8sU0FBUCxDQUFpQixRQUFqQixDQUEwQixJQUExQixDQUErQixDQUEvQixDQUFWO0FBQ0EsVUFBTyxFQUFFLFNBQUYsQ0FBWSxDQUFaLEVBQWUsRUFBRSxNQUFGLEdBQVcsQ0FBMUIsRUFBNkIsV0FBN0IsRUFBUDtBQUNEOztBQUVEOztBQUVBLEtBQU0scUJBQXFCLG9CQUEzQjtBQUNBLEtBQU0sa0JBQWtCLGlCQUF4QjtBQUNBLEtBQU0sb0JBQW9CLFlBQTFCO0FBQ0EsS0FBTSxnQkFBZ0IsT0FBdEI7O0FBRU8sS0FBTSw0Q0FBa0IsU0FBbEIsZUFBa0I7QUFBQSxVQUFRLENBQUMsQ0FBQyxLQUFLLEtBQUwsQ0FBVyxrQkFBWCxDQUFWO0FBQUEsRUFBeEI7QUFDQSxLQUFNLHNDQUFlLFNBQWYsWUFBZTtBQUFBLFVBQVEsQ0FBQyxDQUFDLEtBQUssS0FBTCxDQUFXLGVBQVgsQ0FBVjtBQUFBLEVBQXJCO0FBQ0EsS0FBTSwwQ0FBaUIsU0FBakIsY0FBaUI7QUFBQSxVQUFRLENBQUMsQ0FBQyxLQUFLLEtBQUwsQ0FBVyxpQkFBWCxDQUFWO0FBQUEsRUFBdkI7QUFDQSxLQUFNLG9DQUFjLFNBQWQsV0FBYztBQUFBLFVBQVEsQ0FBQyxnQkFBZ0IsSUFBaEIsQ0FBRCxJQUEwQixDQUFDLGFBQWEsSUFBYixDQUEzQixJQUFpRCxDQUFDLGVBQWUsSUFBZixDQUExRDtBQUFBLEVBQXBCOztBQUVBLFVBQVMsZ0JBQVQsQ0FBMkIsR0FBM0IsRUFBZ0M7QUFDckMsT0FBTSxTQUFTLElBQUksT0FBSixDQUFZLGtCQUFaLEVBQWdDLEVBQWhDLEVBQW9DLE9BQXBDLENBQTRDLGVBQTVDLEVBQTZELEVBQTdELENBQWY7QUFDQSxVQUFPLE1BQVA7QUFDRDs7QUFFTSxVQUFTLGNBQVQsQ0FBeUIsR0FBekIsRUFBOEI7QUFDbkMsVUFBTyxJQUFJLE9BQUosQ0FBWSxhQUFaLEVBQTJCLEVBQTNCLENBQVA7QUFDRCxFOzs7Ozs7Ozs7OztTQzVFZSxrQixHQUFBLGtCO1NBV0EsUSxHQUFBLFE7U0F3RUEsUyxHQUFBLFM7U0FxREEsUSxHQUFBLFE7O0FBcktoQjs7OztBQUNBOztBQVNBOzs7O0FBQ0E7O0FBS0E7O0tBQVksUzs7Ozs7O21OQTlCWjs7Ozs7Ozs7Ozs7Ozs7QUFnQ0E7Ozs7OztBQU1BLEtBQUksZ0JBQWdCLEVBQXBCOztBQUVBOzs7QUFHTyxVQUFTLGtCQUFULEdBQStCO0FBQ3BDLG1CQUFnQixFQUFoQjtBQUNEOztBQUVEOzs7Ozs7O0FBT08sVUFBUyxRQUFULENBQW1CLEdBQW5CLEVBQXdCLElBQXhCLEVBQXVDO0FBQzVDLFdBQVEsS0FBUix3Q0FBbUQsSUFBbkQ7O0FBRUE7QUFDQTtBQUNBO0FBQ0E7QUFDQSxPQUFJLGdCQUFKO0FBQUEsT0FBYSxtQkFBYjtBQUNBLE9BQUkscURBQWMsQ0FBbEIsRUFBcUI7QUFDbkI7QUFDRCxJQUZELE1BR0s7QUFDSDtBQUNEO0FBQ0QsT0FBSSxPQUFPLFVBQVAsS0FBc0IsVUFBMUIsRUFBc0M7QUFDcEMsZUFBVSxVQUFWO0FBQ0Esa0JBQWEsSUFBYjtBQUNEOztBQUVEO0FBQ0EsT0FBSSxPQUFKLEVBQWE7QUFDWCxTQUFNLElBQUksU0FBSixDQUFJLENBQUMsSUFBRCxFQUFVO0FBQ2xCLFdBQUksMkJBQWdCLElBQWhCLENBQUosRUFBMkI7QUFDekIsYUFBTSxZQUFZLDRCQUFpQixJQUFqQixDQUFsQjtBQUNBLGdCQUFPLHNDQUF1QixHQUF2QixFQUE0QixTQUE1QixDQUFQO0FBQ0Q7QUFDRCxXQUFJLHdCQUFhLElBQWIsQ0FBSixFQUF3QjtBQUN0QixhQUFNLGFBQVksNEJBQWlCLElBQWpCLENBQWxCO0FBQ0EsZ0JBQU8sSUFBSSxhQUFKLENBQWtCLFVBQWxCLENBQVA7QUFDRDtBQUNELFdBQUksMEJBQWUsSUFBZixLQUF3Qix1QkFBWSxJQUFaLENBQTVCLEVBQStDO0FBQzdDLGFBQU0sY0FBWSwwQkFBZSxJQUFmLENBQWxCO0FBQ0EsZ0JBQU8sY0FBYyxXQUFkLENBQVA7QUFDRDtBQUNGLE1BYkQ7QUFjQSxTQUFNLElBQUksRUFBRSxTQUFTLEVBQVgsRUFBVjtBQUNBLGFBQVEsQ0FBUixFQUFXLEVBQUUsT0FBYixFQUFzQixDQUF0QjtBQUNBLGtCQUFhLEVBQUUsT0FBZjtBQUNEOztBQUVEO0FBQ0EsT0FBSSwyQkFBZ0IsSUFBaEIsQ0FBSixFQUEyQjtBQUN6QixTQUFNLFlBQVksNEJBQWlCLElBQWpCLENBQWxCO0FBQ0EsNENBQXdCLEdBQXhCLEVBQTZCLFNBQTdCLEVBQXdDLFVBQXhDO0FBQ0QsSUFIRCxNQUlLLElBQUksd0JBQWEsSUFBYixDQUFKLEVBQXdCO0FBQzNCLFNBQU0sY0FBWSw0QkFBaUIsSUFBakIsQ0FBbEI7QUFDQSxvREFBZSxXQUFmLEVBQTJCLFVBQTNCO0FBQ0QsSUFISSxNQUlBLElBQUksMEJBQWUsSUFBZixDQUFKLEVBQTBCO0FBQzdCLFNBQU0sY0FBWSwwQkFBZSxJQUFmLENBQWxCO0FBQ0EsbUJBQWMsV0FBZCxJQUEyQixVQUEzQjtBQUNELElBSEksTUFJQSxJQUFJLHVCQUFZLElBQVosQ0FBSixFQUF1QjtBQUMxQixTQUFNLGNBQVksMEJBQWUsSUFBZixDQUFsQjtBQUNBLFNBQUksV0FBVyxRQUFYLElBQ0EsV0FBVyxLQURYLElBRUEsV0FBVyxPQUZmLEVBRXdCO0FBQ3RCO0FBQ0E7QUFDQTtBQUNBLDhDQUF3QixHQUF4QixFQUE2QixXQUE3QixFQUF3QyxVQUF4QztBQUNELE1BUEQsTUFRSztBQUNILHFCQUFjLFdBQWQsSUFBMkIsVUFBM0I7QUFDRDtBQUNGO0FBQ0Y7O0FBRUQ7OztBQUdPLFVBQVMsU0FBVCxDQUFvQixHQUFwQixFQUF5QixJQUF6QixFQUErQixNQUEvQixFQUF1QyxJQUF2QyxFQUE2QztBQUNsRCxXQUFRLEtBQVIsbUNBQThDLElBQTlDOztBQUVBO0FBQ0EsT0FBSSxrQkFBSjtBQUNBLE9BQUksMkJBQWdCLElBQWhCLENBQUosRUFBMkI7QUFDekIsaUJBQVksNEJBQWlCLElBQWpCLENBQVo7QUFDRCxJQUZELE1BR0ssSUFBSSx1QkFBWSxJQUFaLENBQUosRUFBdUI7QUFDMUIsaUJBQVksMEJBQWUsSUFBZixDQUFaO0FBQ0E7QUFDQTtBQUNBLFNBQUksQ0FBQyxzQ0FBdUIsR0FBdkIsRUFBNEIsU0FBNUIsQ0FBTCxFQUE2QztBQUMzQyxjQUFPLElBQUksS0FBSiw2QkFBbUMsSUFBbkMsQ0FBUDtBQUNEO0FBQ0YsSUFQSSxNQVFBO0FBQ0gsWUFBTyxJQUFJLEtBQUosNEJBQW1DLElBQW5DLENBQVA7QUFDRDs7QUFFRDtBQUNBLFlBQVMseUJBQWMsTUFBZCxJQUF3QixNQUF4QixHQUFpQyxFQUExQztBQUNBO0FBQ0EsT0FBSSxPQUFPLE9BQU8sa0JBQWQsS0FBcUMsUUFBckMsSUFDRixPQUFPLE9BQU8sa0JBQWQsS0FBcUMsUUFEbkMsSUFFRixDQUFDLGlCQUFPLFNBQVAsQ0FBaUIsT0FBTyxrQkFBeEIsRUFDQyxPQUFPLGtCQURSLENBRkgsRUFHZ0M7QUFDOUIsWUFBTyxJQUFJLEtBQUosQ0FBVSx3QkFBc0IsT0FBTyxrQkFBN0IsbUNBQ1EsT0FBTyxrQkFEZixDQUFWLENBQVA7QUFFRDtBQUNEO0FBQ0EsT0FBTSxrQkFBa0IsVUFBVSxLQUFWLENBQWdCLE9BQU8sU0FBdkIsQ0FBeEI7QUFDQTtBQUNBLE9BQUksZ0JBQWdCLFdBQXBCLEVBQWlDO0FBQy9CLFNBQUksU0FBSixDQUFjLENBQUM7QUFDYixlQUFRLGNBREs7QUFFYixlQUFRLE9BRks7QUFHYixhQUFNLENBQ0osZ0JBQWdCLFNBRFosRUFFSixnQkFBZ0IsSUFGWixFQUdKLGdCQUFnQixZQUhaO0FBSE8sTUFBRCxDQUFkO0FBU0EsWUFBTyxJQUFJLEtBQUosZ0JBQXVCLGdCQUFnQixJQUF2QyxXQUFpRCxnQkFBZ0IsWUFBakUsQ0FBUDtBQUNEOztBQUVEO0FBQ0EsT0FBSSxFQUFKLEdBQVMsaUJBQU8sU0FBUCxFQUFrQixJQUFsQixFQUF3QixFQUFFLE1BQU0sR0FBUixFQUF4QixFQUF1QyxJQUF2QyxFQUE2QyxJQUE3QyxDQUFUO0FBQ0Q7O0FBRUQ7OztBQUdPLFVBQVMsUUFBVCxDQUFtQixHQUFuQixFQUF3QixJQUF4QixFQUE4QixPQUE5QixFQUF1QztBQUM1QyxXQUFRLElBQVIsQ0FBYSw0RUFBYjtBQUNBLDBDQUF3QixHQUF4QixFQUE2QixJQUE3QixFQUFtQyxPQUFuQztBQUNELEU7Ozs7Ozs7Ozs7O0FDdExELFdBQVUsT0FBTyxPQUFQLEdBQWlCLE1BQTNCOztBQUVBO0FBQ0EsWUFBWSxJQUFJLEtBQUo7QUFDWixZQUFZLElBQUksUUFBTyxPQUFQLHlDQUFPLE9BQVAsT0FBbUIsUUFBbkI7QUFDWixZQUFZLFFBQVEsR0FEUjtBQUVaLFlBQVksUUFBUSxHQUFSLENBQVksVUFGWjtBQUdaLFlBQVksY0FBYyxJQUFkLENBQW1CLFFBQVEsR0FBUixDQUFZLFVBQS9CLENBSEo7QUFJVixjQUFZLFFBQVEsaUJBQVc7QUFDN0IsZ0JBQVksSUFBSSxPQUFPLE1BQU0sU0FBTixDQUFnQixLQUFoQixDQUFzQixJQUF0QixDQUEyQixTQUEzQixFQUFzQyxDQUF0QyxDQUFYO0FBQ1osZ0JBQVksS0FBSyxPQUFMLENBQWEsUUFBYjtBQUNaLGdCQUFZLFFBQVEsR0FBUixDQUFZLEtBQVosQ0FBa0IsT0FBbEIsRUFBMkIsSUFBM0I7QUFDWjtBQUFhLElBSkg7QUFLZCxjQVRZO0FBVVYsY0FBWSxRQUFRLGlCQUFXLENBQUUsQ0FBckI7O0FBRWQ7QUFDQTtBQUNBLFNBQVEsbUJBQVIsR0FBOEIsT0FBOUI7O0FBRUEsS0FBSSxhQUFhLEdBQWpCO0FBQ0EsS0FBSSxtQkFBbUIsT0FBTyxnQkFBUCxJQUEyQixnQkFBbEQ7O0FBRUE7QUFDQSxLQUFJLEtBQUssUUFBUSxFQUFSLEdBQWEsRUFBdEI7QUFDQSxLQUFJLE1BQU0sUUFBUSxHQUFSLEdBQWMsRUFBeEI7QUFDQSxLQUFJLElBQUksQ0FBUjs7QUFFQTtBQUNBOztBQUVBO0FBQ0E7O0FBRUEsS0FBSSxvQkFBb0IsR0FBeEI7QUFDQSxLQUFJLGlCQUFKLElBQXlCLGFBQXpCO0FBQ0EsS0FBSSx5QkFBeUIsR0FBN0I7QUFDQSxLQUFJLHNCQUFKLElBQThCLFFBQTlCOztBQUdBO0FBQ0E7QUFDQTs7QUFFQSxLQUFJLHVCQUF1QixHQUEzQjtBQUNBLEtBQUksb0JBQUosSUFBNEIsNEJBQTVCOztBQUdBO0FBQ0E7O0FBRUEsS0FBSSxjQUFjLEdBQWxCO0FBQ0EsS0FBSSxXQUFKLElBQW1CLE1BQU0sSUFBSSxpQkFBSixDQUFOLEdBQStCLE1BQS9CLEdBQ0EsR0FEQSxHQUNNLElBQUksaUJBQUosQ0FETixHQUMrQixNQUQvQixHQUVBLEdBRkEsR0FFTSxJQUFJLGlCQUFKLENBRk4sR0FFK0IsR0FGbEQ7O0FBSUEsS0FBSSxtQkFBbUIsR0FBdkI7QUFDQSxLQUFJLGdCQUFKLElBQXdCLE1BQU0sSUFBSSxzQkFBSixDQUFOLEdBQW9DLE1BQXBDLEdBQ0EsR0FEQSxHQUNNLElBQUksc0JBQUosQ0FETixHQUNvQyxNQURwQyxHQUVBLEdBRkEsR0FFTSxJQUFJLHNCQUFKLENBRk4sR0FFb0MsR0FGNUQ7O0FBSUE7QUFDQTs7QUFFQSxLQUFJLHVCQUF1QixHQUEzQjtBQUNBLEtBQUksb0JBQUosSUFBNEIsUUFBUSxJQUFJLGlCQUFKLENBQVIsR0FDQSxHQURBLEdBQ00sSUFBSSxvQkFBSixDQUROLEdBQ2tDLEdBRDlEOztBQUdBLEtBQUksNEJBQTRCLEdBQWhDO0FBQ0EsS0FBSSx5QkFBSixJQUFpQyxRQUFRLElBQUksc0JBQUosQ0FBUixHQUNBLEdBREEsR0FDTSxJQUFJLG9CQUFKLENBRE4sR0FDa0MsR0FEbkU7O0FBSUE7QUFDQTtBQUNBOztBQUVBLEtBQUksYUFBYSxHQUFqQjtBQUNBLEtBQUksVUFBSixJQUFrQixVQUFVLElBQUksb0JBQUosQ0FBVixHQUNBLFFBREEsR0FDVyxJQUFJLG9CQUFKLENBRFgsR0FDdUMsTUFEekQ7O0FBR0EsS0FBSSxrQkFBa0IsR0FBdEI7QUFDQSxLQUFJLGVBQUosSUFBdUIsV0FBVyxJQUFJLHlCQUFKLENBQVgsR0FDQSxRQURBLEdBQ1csSUFBSSx5QkFBSixDQURYLEdBQzRDLE1BRG5FOztBQUdBO0FBQ0E7O0FBRUEsS0FBSSxrQkFBa0IsR0FBdEI7QUFDQSxLQUFJLGVBQUosSUFBdUIsZUFBdkI7O0FBRUE7QUFDQTtBQUNBOztBQUVBLEtBQUksUUFBUSxHQUFaO0FBQ0EsS0FBSSxLQUFKLElBQWEsWUFBWSxJQUFJLGVBQUosQ0FBWixHQUNBLFFBREEsR0FDVyxJQUFJLGVBQUosQ0FEWCxHQUNrQyxNQUQvQzs7QUFJQTtBQUNBO0FBQ0E7O0FBRUE7QUFDQTtBQUNBO0FBQ0E7O0FBRUEsS0FBSSxPQUFPLEdBQVg7QUFDQSxLQUFJLFlBQVksT0FBTyxJQUFJLFdBQUosQ0FBUCxHQUNBLElBQUksVUFBSixDQURBLEdBQ2tCLEdBRGxCLEdBRUEsSUFBSSxLQUFKLENBRkEsR0FFYSxHQUY3Qjs7QUFJQSxLQUFJLElBQUosSUFBWSxNQUFNLFNBQU4sR0FBa0IsR0FBOUI7O0FBRUE7QUFDQTtBQUNBO0FBQ0EsS0FBSSxhQUFhLGFBQWEsSUFBSSxnQkFBSixDQUFiLEdBQ0EsSUFBSSxlQUFKLENBREEsR0FDdUIsR0FEdkIsR0FFQSxJQUFJLEtBQUosQ0FGQSxHQUVhLEdBRjlCOztBQUlBLEtBQUksUUFBUSxHQUFaO0FBQ0EsS0FBSSxLQUFKLElBQWEsTUFBTSxVQUFOLEdBQW1CLEdBQWhDOztBQUVBLEtBQUksT0FBTyxHQUFYO0FBQ0EsS0FBSSxJQUFKLElBQVksY0FBWjs7QUFFQTtBQUNBO0FBQ0E7QUFDQSxLQUFJLHdCQUF3QixHQUE1QjtBQUNBLEtBQUkscUJBQUosSUFBNkIsSUFBSSxzQkFBSixJQUE4QixVQUEzRDtBQUNBLEtBQUksbUJBQW1CLEdBQXZCO0FBQ0EsS0FBSSxnQkFBSixJQUF3QixJQUFJLGlCQUFKLElBQXlCLFVBQWpEOztBQUVBLEtBQUksY0FBYyxHQUFsQjtBQUNBLEtBQUksV0FBSixJQUFtQixjQUFjLElBQUksZ0JBQUosQ0FBZCxHQUFzQyxHQUF0QyxHQUNBLFNBREEsR0FDWSxJQUFJLGdCQUFKLENBRFosR0FDb0MsR0FEcEMsR0FFQSxTQUZBLEdBRVksSUFBSSxnQkFBSixDQUZaLEdBRW9DLEdBRnBDLEdBR0EsS0FIQSxHQUdRLElBQUksVUFBSixDQUhSLEdBRzBCLElBSDFCLEdBSUEsSUFBSSxLQUFKLENBSkEsR0FJYSxHQUpiLEdBS0EsTUFMbkI7O0FBT0EsS0FBSSxtQkFBbUIsR0FBdkI7QUFDQSxLQUFJLGdCQUFKLElBQXdCLGNBQWMsSUFBSSxxQkFBSixDQUFkLEdBQTJDLEdBQTNDLEdBQ0EsU0FEQSxHQUNZLElBQUkscUJBQUosQ0FEWixHQUN5QyxHQUR6QyxHQUVBLFNBRkEsR0FFWSxJQUFJLHFCQUFKLENBRlosR0FFeUMsR0FGekMsR0FHQSxLQUhBLEdBR1EsSUFBSSxlQUFKLENBSFIsR0FHK0IsSUFIL0IsR0FJQSxJQUFJLEtBQUosQ0FKQSxHQUlhLEdBSmIsR0FLQSxNQUx4Qjs7QUFPQSxLQUFJLFNBQVMsR0FBYjtBQUNBLEtBQUksTUFBSixJQUFjLE1BQU0sSUFBSSxJQUFKLENBQU4sR0FBa0IsTUFBbEIsR0FBMkIsSUFBSSxXQUFKLENBQTNCLEdBQThDLEdBQTVEO0FBQ0EsS0FBSSxjQUFjLEdBQWxCO0FBQ0EsS0FBSSxXQUFKLElBQW1CLE1BQU0sSUFBSSxJQUFKLENBQU4sR0FBa0IsTUFBbEIsR0FBMkIsSUFBSSxnQkFBSixDQUEzQixHQUFtRCxHQUF0RTs7QUFFQTtBQUNBO0FBQ0EsS0FBSSxZQUFZLEdBQWhCO0FBQ0EsS0FBSSxTQUFKLElBQWlCLFNBQWpCOztBQUVBLEtBQUksWUFBWSxHQUFoQjtBQUNBLEtBQUksU0FBSixJQUFpQixXQUFXLElBQUksU0FBSixDQUFYLEdBQTRCLE1BQTdDO0FBQ0EsSUFBRyxTQUFILElBQWdCLElBQUksTUFBSixDQUFXLElBQUksU0FBSixDQUFYLEVBQTJCLEdBQTNCLENBQWhCO0FBQ0EsS0FBSSxtQkFBbUIsS0FBdkI7O0FBRUEsS0FBSSxRQUFRLEdBQVo7QUFDQSxLQUFJLEtBQUosSUFBYSxNQUFNLElBQUksU0FBSixDQUFOLEdBQXVCLElBQUksV0FBSixDQUF2QixHQUEwQyxHQUF2RDtBQUNBLEtBQUksYUFBYSxHQUFqQjtBQUNBLEtBQUksVUFBSixJQUFrQixNQUFNLElBQUksU0FBSixDQUFOLEdBQXVCLElBQUksZ0JBQUosQ0FBdkIsR0FBK0MsR0FBakU7O0FBRUE7QUFDQTtBQUNBLEtBQUksWUFBWSxHQUFoQjtBQUNBLEtBQUksU0FBSixJQUFpQixTQUFqQjs7QUFFQSxLQUFJLFlBQVksR0FBaEI7QUFDQSxLQUFJLFNBQUosSUFBaUIsV0FBVyxJQUFJLFNBQUosQ0FBWCxHQUE0QixNQUE3QztBQUNBLElBQUcsU0FBSCxJQUFnQixJQUFJLE1BQUosQ0FBVyxJQUFJLFNBQUosQ0FBWCxFQUEyQixHQUEzQixDQUFoQjtBQUNBLEtBQUksbUJBQW1CLEtBQXZCOztBQUVBLEtBQUksUUFBUSxHQUFaO0FBQ0EsS0FBSSxLQUFKLElBQWEsTUFBTSxJQUFJLFNBQUosQ0FBTixHQUF1QixJQUFJLFdBQUosQ0FBdkIsR0FBMEMsR0FBdkQ7QUFDQSxLQUFJLGFBQWEsR0FBakI7QUFDQSxLQUFJLFVBQUosSUFBa0IsTUFBTSxJQUFJLFNBQUosQ0FBTixHQUF1QixJQUFJLGdCQUFKLENBQXZCLEdBQStDLEdBQWpFOztBQUVBO0FBQ0EsS0FBSSxrQkFBa0IsR0FBdEI7QUFDQSxLQUFJLGVBQUosSUFBdUIsTUFBTSxJQUFJLElBQUosQ0FBTixHQUFrQixPQUFsQixHQUE0QixVQUE1QixHQUF5QyxPQUFoRTtBQUNBLEtBQUksYUFBYSxHQUFqQjtBQUNBLEtBQUksVUFBSixJQUFrQixNQUFNLElBQUksSUFBSixDQUFOLEdBQWtCLE9BQWxCLEdBQTRCLFNBQTVCLEdBQXdDLE9BQTFEOztBQUdBO0FBQ0E7QUFDQSxLQUFJLGlCQUFpQixHQUFyQjtBQUNBLEtBQUksY0FBSixJQUFzQixXQUFXLElBQUksSUFBSixDQUFYLEdBQ0EsT0FEQSxHQUNVLFVBRFYsR0FDdUIsR0FEdkIsR0FDNkIsSUFBSSxXQUFKLENBRDdCLEdBQ2dELEdBRHRFOztBQUdBO0FBQ0EsSUFBRyxjQUFILElBQXFCLElBQUksTUFBSixDQUFXLElBQUksY0FBSixDQUFYLEVBQWdDLEdBQWhDLENBQXJCO0FBQ0EsS0FBSSx3QkFBd0IsUUFBNUI7O0FBR0E7QUFDQTtBQUNBO0FBQ0E7QUFDQSxLQUFJLGNBQWMsR0FBbEI7QUFDQSxLQUFJLFdBQUosSUFBbUIsV0FBVyxJQUFJLFdBQUosQ0FBWCxHQUE4QixHQUE5QixHQUNBLFdBREEsR0FFQSxHQUZBLEdBRU0sSUFBSSxXQUFKLENBRk4sR0FFeUIsR0FGekIsR0FHQSxPQUhuQjs7QUFLQSxLQUFJLG1CQUFtQixHQUF2QjtBQUNBLEtBQUksZ0JBQUosSUFBd0IsV0FBVyxJQUFJLGdCQUFKLENBQVgsR0FBbUMsR0FBbkMsR0FDQSxXQURBLEdBRUEsR0FGQSxHQUVNLElBQUksZ0JBQUosQ0FGTixHQUU4QixHQUY5QixHQUdBLE9BSHhCOztBQUtBO0FBQ0EsS0FBSSxPQUFPLEdBQVg7QUFDQSxLQUFJLElBQUosSUFBWSxpQkFBWjs7QUFFQTtBQUNBO0FBQ0EsTUFBSyxJQUFJLElBQUksQ0FBYixFQUFnQixJQUFJLENBQXBCLEVBQXVCLEdBQXZCLEVBQTRCO0FBQzFCLFNBQU0sQ0FBTixFQUFTLElBQUksQ0FBSixDQUFUO0FBQ0EsT0FBSSxDQUFDLEdBQUcsQ0FBSCxDQUFMLEVBQ0UsR0FBRyxDQUFILElBQVEsSUFBSSxNQUFKLENBQVcsSUFBSSxDQUFKLENBQVgsQ0FBUjtBQUNIOztBQUVELFNBQVEsS0FBUixHQUFnQixLQUFoQjtBQUNBLFVBQVMsS0FBVCxDQUFlLE9BQWYsRUFBd0IsS0FBeEIsRUFBK0I7QUFDN0IsT0FBSSxtQkFBbUIsTUFBdkIsRUFDRSxPQUFPLE9BQVA7O0FBRUYsT0FBSSxPQUFPLE9BQVAsS0FBbUIsUUFBdkIsRUFDRSxPQUFPLElBQVA7O0FBRUYsT0FBSSxRQUFRLE1BQVIsR0FBaUIsVUFBckIsRUFDRSxPQUFPLElBQVA7O0FBRUYsT0FBSSxJQUFJLFFBQVEsR0FBRyxLQUFILENBQVIsR0FBb0IsR0FBRyxJQUFILENBQTVCO0FBQ0EsT0FBSSxDQUFDLEVBQUUsSUFBRixDQUFPLE9BQVAsQ0FBTCxFQUNFLE9BQU8sSUFBUDs7QUFFRixPQUFJO0FBQ0YsWUFBTyxJQUFJLE1BQUosQ0FBVyxPQUFYLEVBQW9CLEtBQXBCLENBQVA7QUFDRCxJQUZELENBRUUsT0FBTyxFQUFQLEVBQVc7QUFDWCxZQUFPLElBQVA7QUFDRDtBQUNGOztBQUVELFNBQVEsS0FBUixHQUFnQixLQUFoQjtBQUNBLFVBQVMsS0FBVCxDQUFlLE9BQWYsRUFBd0IsS0FBeEIsRUFBK0I7QUFDN0IsT0FBSSxJQUFJLE1BQU0sT0FBTixFQUFlLEtBQWYsQ0FBUjtBQUNBLFVBQU8sSUFBSSxFQUFFLE9BQU4sR0FBZ0IsSUFBdkI7QUFDRDs7QUFHRCxTQUFRLEtBQVIsR0FBZ0IsS0FBaEI7QUFDQSxVQUFTLEtBQVQsQ0FBZSxPQUFmLEVBQXdCLEtBQXhCLEVBQStCO0FBQzdCLE9BQUksSUFBSSxNQUFNLFFBQVEsSUFBUixHQUFlLE9BQWYsQ0FBdUIsUUFBdkIsRUFBaUMsRUFBakMsQ0FBTixFQUE0QyxLQUE1QyxDQUFSO0FBQ0EsVUFBTyxJQUFJLEVBQUUsT0FBTixHQUFnQixJQUF2QjtBQUNEOztBQUVELFNBQVEsTUFBUixHQUFpQixNQUFqQjs7QUFFQSxVQUFTLE1BQVQsQ0FBZ0IsT0FBaEIsRUFBeUIsS0FBekIsRUFBZ0M7QUFDOUIsT0FBSSxtQkFBbUIsTUFBdkIsRUFBK0I7QUFDN0IsU0FBSSxRQUFRLEtBQVIsS0FBa0IsS0FBdEIsRUFDRSxPQUFPLE9BQVAsQ0FERixLQUdFLFVBQVUsUUFBUSxPQUFsQjtBQUNILElBTEQsTUFLTyxJQUFJLE9BQU8sT0FBUCxLQUFtQixRQUF2QixFQUFpQztBQUN0QyxXQUFNLElBQUksU0FBSixDQUFjLHNCQUFzQixPQUFwQyxDQUFOO0FBQ0Q7O0FBRUQsT0FBSSxRQUFRLE1BQVIsR0FBaUIsVUFBckIsRUFDRSxNQUFNLElBQUksU0FBSixDQUFjLDRCQUE0QixVQUE1QixHQUF5QyxhQUF2RCxDQUFOOztBQUVGLE9BQUksRUFBRSxnQkFBZ0IsTUFBbEIsQ0FBSixFQUNFLE9BQU8sSUFBSSxNQUFKLENBQVcsT0FBWCxFQUFvQixLQUFwQixDQUFQOztBQUVGLFNBQU0sUUFBTixFQUFnQixPQUFoQixFQUF5QixLQUF6QjtBQUNBLFFBQUssS0FBTCxHQUFhLEtBQWI7QUFDQSxPQUFJLElBQUksUUFBUSxJQUFSLEdBQWUsS0FBZixDQUFxQixRQUFRLEdBQUcsS0FBSCxDQUFSLEdBQW9CLEdBQUcsSUFBSCxDQUF6QyxDQUFSOztBQUVBLE9BQUksQ0FBQyxDQUFMLEVBQ0UsTUFBTSxJQUFJLFNBQUosQ0FBYyxzQkFBc0IsT0FBcEMsQ0FBTjs7QUFFRixRQUFLLEdBQUwsR0FBVyxPQUFYOztBQUVBO0FBQ0EsUUFBSyxLQUFMLEdBQWEsQ0FBQyxFQUFFLENBQUYsQ0FBZDtBQUNBLFFBQUssS0FBTCxHQUFhLENBQUMsRUFBRSxDQUFGLENBQWQ7QUFDQSxRQUFLLEtBQUwsR0FBYSxDQUFDLEVBQUUsQ0FBRixDQUFkOztBQUVBLE9BQUksS0FBSyxLQUFMLEdBQWEsZ0JBQWIsSUFBaUMsS0FBSyxLQUFMLEdBQWEsQ0FBbEQsRUFDRSxNQUFNLElBQUksU0FBSixDQUFjLHVCQUFkLENBQU47O0FBRUYsT0FBSSxLQUFLLEtBQUwsR0FBYSxnQkFBYixJQUFpQyxLQUFLLEtBQUwsR0FBYSxDQUFsRCxFQUNFLE1BQU0sSUFBSSxTQUFKLENBQWMsdUJBQWQsQ0FBTjs7QUFFRixPQUFJLEtBQUssS0FBTCxHQUFhLGdCQUFiLElBQWlDLEtBQUssS0FBTCxHQUFhLENBQWxELEVBQ0UsTUFBTSxJQUFJLFNBQUosQ0FBYyx1QkFBZCxDQUFOOztBQUVGO0FBQ0EsT0FBSSxDQUFDLEVBQUUsQ0FBRixDQUFMLEVBQ0UsS0FBSyxVQUFMLEdBQWtCLEVBQWxCLENBREYsS0FHRSxLQUFLLFVBQUwsR0FBa0IsRUFBRSxDQUFGLEVBQUssS0FBTCxDQUFXLEdBQVgsRUFBZ0IsR0FBaEIsQ0FBb0IsVUFBUyxFQUFULEVBQWE7QUFDakQsU0FBSSxXQUFXLElBQVgsQ0FBZ0IsRUFBaEIsQ0FBSixFQUF5QjtBQUN2QixXQUFJLE1BQU0sQ0FBQyxFQUFYO0FBQ0EsV0FBSSxPQUFPLENBQVAsSUFBWSxNQUFNLGdCQUF0QixFQUNFLE9BQU8sR0FBUDtBQUNIO0FBQ0QsWUFBTyxFQUFQO0FBQ0QsSUFQaUIsQ0FBbEI7O0FBU0YsUUFBSyxLQUFMLEdBQWEsRUFBRSxDQUFGLElBQU8sRUFBRSxDQUFGLEVBQUssS0FBTCxDQUFXLEdBQVgsQ0FBUCxHQUF5QixFQUF0QztBQUNBLFFBQUssTUFBTDtBQUNEOztBQUVELFFBQU8sU0FBUCxDQUFpQixNQUFqQixHQUEwQixZQUFXO0FBQ25DLFFBQUssT0FBTCxHQUFlLEtBQUssS0FBTCxHQUFhLEdBQWIsR0FBbUIsS0FBSyxLQUF4QixHQUFnQyxHQUFoQyxHQUFzQyxLQUFLLEtBQTFEO0FBQ0EsT0FBSSxLQUFLLFVBQUwsQ0FBZ0IsTUFBcEIsRUFDRSxLQUFLLE9BQUwsSUFBZ0IsTUFBTSxLQUFLLFVBQUwsQ0FBZ0IsSUFBaEIsQ0FBcUIsR0FBckIsQ0FBdEI7QUFDRixVQUFPLEtBQUssT0FBWjtBQUNELEVBTEQ7O0FBT0EsUUFBTyxTQUFQLENBQWlCLFFBQWpCLEdBQTRCLFlBQVc7QUFDckMsVUFBTyxLQUFLLE9BQVo7QUFDRCxFQUZEOztBQUlBLFFBQU8sU0FBUCxDQUFpQixPQUFqQixHQUEyQixVQUFTLEtBQVQsRUFBZ0I7QUFDekMsU0FBTSxnQkFBTixFQUF3QixLQUFLLE9BQTdCLEVBQXNDLEtBQUssS0FBM0MsRUFBa0QsS0FBbEQ7QUFDQSxPQUFJLEVBQUUsaUJBQWlCLE1BQW5CLENBQUosRUFDRSxRQUFRLElBQUksTUFBSixDQUFXLEtBQVgsRUFBa0IsS0FBSyxLQUF2QixDQUFSOztBQUVGLFVBQU8sS0FBSyxXQUFMLENBQWlCLEtBQWpCLEtBQTJCLEtBQUssVUFBTCxDQUFnQixLQUFoQixDQUFsQztBQUNELEVBTkQ7O0FBUUEsUUFBTyxTQUFQLENBQWlCLFdBQWpCLEdBQStCLFVBQVMsS0FBVCxFQUFnQjtBQUM3QyxPQUFJLEVBQUUsaUJBQWlCLE1BQW5CLENBQUosRUFDRSxRQUFRLElBQUksTUFBSixDQUFXLEtBQVgsRUFBa0IsS0FBSyxLQUF2QixDQUFSOztBQUVGLFVBQU8sbUJBQW1CLEtBQUssS0FBeEIsRUFBK0IsTUFBTSxLQUFyQyxLQUNBLG1CQUFtQixLQUFLLEtBQXhCLEVBQStCLE1BQU0sS0FBckMsQ0FEQSxJQUVBLG1CQUFtQixLQUFLLEtBQXhCLEVBQStCLE1BQU0sS0FBckMsQ0FGUDtBQUdELEVBUEQ7O0FBU0EsUUFBTyxTQUFQLENBQWlCLFVBQWpCLEdBQThCLFVBQVMsS0FBVCxFQUFnQjtBQUM1QyxPQUFJLEVBQUUsaUJBQWlCLE1BQW5CLENBQUosRUFDRSxRQUFRLElBQUksTUFBSixDQUFXLEtBQVgsRUFBa0IsS0FBSyxLQUF2QixDQUFSOztBQUVGO0FBQ0EsT0FBSSxLQUFLLFVBQUwsQ0FBZ0IsTUFBaEIsSUFBMEIsQ0FBQyxNQUFNLFVBQU4sQ0FBaUIsTUFBaEQsRUFDRSxPQUFPLENBQUMsQ0FBUixDQURGLEtBRUssSUFBSSxDQUFDLEtBQUssVUFBTCxDQUFnQixNQUFqQixJQUEyQixNQUFNLFVBQU4sQ0FBaUIsTUFBaEQsRUFDSCxPQUFPLENBQVAsQ0FERyxLQUVBLElBQUksQ0FBQyxLQUFLLFVBQUwsQ0FBZ0IsTUFBakIsSUFBMkIsQ0FBQyxNQUFNLFVBQU4sQ0FBaUIsTUFBakQsRUFDSCxPQUFPLENBQVA7O0FBRUYsT0FBSSxJQUFJLENBQVI7QUFDQSxNQUFHO0FBQ0QsU0FBSSxJQUFJLEtBQUssVUFBTCxDQUFnQixDQUFoQixDQUFSO0FBQ0EsU0FBSSxJQUFJLE1BQU0sVUFBTixDQUFpQixDQUFqQixDQUFSO0FBQ0EsV0FBTSxvQkFBTixFQUE0QixDQUE1QixFQUErQixDQUEvQixFQUFrQyxDQUFsQztBQUNBLFNBQUksTUFBTSxTQUFOLElBQW1CLE1BQU0sU0FBN0IsRUFDRSxPQUFPLENBQVAsQ0FERixLQUVLLElBQUksTUFBTSxTQUFWLEVBQ0gsT0FBTyxDQUFQLENBREcsS0FFQSxJQUFJLE1BQU0sU0FBVixFQUNILE9BQU8sQ0FBQyxDQUFSLENBREcsS0FFQSxJQUFJLE1BQU0sQ0FBVixFQUNILFNBREcsS0FHSCxPQUFPLG1CQUFtQixDQUFuQixFQUFzQixDQUF0QixDQUFQO0FBQ0gsSUFkRCxRQWNTLEVBQUUsQ0FkWDtBQWVELEVBNUJEOztBQThCQTtBQUNBO0FBQ0EsUUFBTyxTQUFQLENBQWlCLEdBQWpCLEdBQXVCLFVBQVMsT0FBVCxFQUFrQixVQUFsQixFQUE4QjtBQUNuRCxXQUFRLE9BQVI7QUFDRSxVQUFLLFVBQUw7QUFDRSxZQUFLLFVBQUwsQ0FBZ0IsTUFBaEIsR0FBeUIsQ0FBekI7QUFDQSxZQUFLLEtBQUwsR0FBYSxDQUFiO0FBQ0EsWUFBSyxLQUFMLEdBQWEsQ0FBYjtBQUNBLFlBQUssS0FBTDtBQUNBLFlBQUssR0FBTCxDQUFTLEtBQVQsRUFBZ0IsVUFBaEI7QUFDQTtBQUNGLFVBQUssVUFBTDtBQUNFLFlBQUssVUFBTCxDQUFnQixNQUFoQixHQUF5QixDQUF6QjtBQUNBLFlBQUssS0FBTCxHQUFhLENBQWI7QUFDQSxZQUFLLEtBQUw7QUFDQSxZQUFLLEdBQUwsQ0FBUyxLQUFULEVBQWdCLFVBQWhCO0FBQ0E7QUFDRixVQUFLLFVBQUw7QUFDRTtBQUNBO0FBQ0E7QUFDQSxZQUFLLFVBQUwsQ0FBZ0IsTUFBaEIsR0FBeUIsQ0FBekI7QUFDQSxZQUFLLEdBQUwsQ0FBUyxPQUFULEVBQWtCLFVBQWxCO0FBQ0EsWUFBSyxHQUFMLENBQVMsS0FBVCxFQUFnQixVQUFoQjtBQUNBO0FBQ0Y7QUFDQTtBQUNBLFVBQUssWUFBTDtBQUNFLFdBQUksS0FBSyxVQUFMLENBQWdCLE1BQWhCLEtBQTJCLENBQS9CLEVBQ0UsS0FBSyxHQUFMLENBQVMsT0FBVCxFQUFrQixVQUFsQjtBQUNGLFlBQUssR0FBTCxDQUFTLEtBQVQsRUFBZ0IsVUFBaEI7QUFDQTs7QUFFRixVQUFLLE9BQUw7QUFDRTtBQUNBO0FBQ0E7QUFDQTtBQUNBLFdBQUksS0FBSyxLQUFMLEtBQWUsQ0FBZixJQUFvQixLQUFLLEtBQUwsS0FBZSxDQUFuQyxJQUF3QyxLQUFLLFVBQUwsQ0FBZ0IsTUFBaEIsS0FBMkIsQ0FBdkUsRUFDRSxLQUFLLEtBQUw7QUFDRixZQUFLLEtBQUwsR0FBYSxDQUFiO0FBQ0EsWUFBSyxLQUFMLEdBQWEsQ0FBYjtBQUNBLFlBQUssVUFBTCxHQUFrQixFQUFsQjtBQUNBO0FBQ0YsVUFBSyxPQUFMO0FBQ0U7QUFDQTtBQUNBO0FBQ0E7QUFDQSxXQUFJLEtBQUssS0FBTCxLQUFlLENBQWYsSUFBb0IsS0FBSyxVQUFMLENBQWdCLE1BQWhCLEtBQTJCLENBQW5ELEVBQ0UsS0FBSyxLQUFMO0FBQ0YsWUFBSyxLQUFMLEdBQWEsQ0FBYjtBQUNBLFlBQUssVUFBTCxHQUFrQixFQUFsQjtBQUNBO0FBQ0YsVUFBSyxPQUFMO0FBQ0U7QUFDQTtBQUNBO0FBQ0E7QUFDQSxXQUFJLEtBQUssVUFBTCxDQUFnQixNQUFoQixLQUEyQixDQUEvQixFQUNFLEtBQUssS0FBTDtBQUNGLFlBQUssVUFBTCxHQUFrQixFQUFsQjtBQUNBO0FBQ0Y7QUFDQTtBQUNBLFVBQUssS0FBTDtBQUNFLFdBQUksS0FBSyxVQUFMLENBQWdCLE1BQWhCLEtBQTJCLENBQS9CLEVBQ0UsS0FBSyxVQUFMLEdBQWtCLENBQUMsQ0FBRCxDQUFsQixDQURGLEtBRUs7QUFDSCxhQUFJLElBQUksS0FBSyxVQUFMLENBQWdCLE1BQXhCO0FBQ0EsZ0JBQU8sRUFBRSxDQUFGLElBQU8sQ0FBZCxFQUFpQjtBQUNmLGVBQUksT0FBTyxLQUFLLFVBQUwsQ0FBZ0IsQ0FBaEIsQ0FBUCxLQUE4QixRQUFsQyxFQUE0QztBQUMxQyxrQkFBSyxVQUFMLENBQWdCLENBQWhCO0FBQ0EsaUJBQUksQ0FBQyxDQUFMO0FBQ0Q7QUFDRjtBQUNELGFBQUksTUFBTSxDQUFDLENBQVgsRUFBYztBQUNaLGdCQUFLLFVBQUwsQ0FBZ0IsSUFBaEIsQ0FBcUIsQ0FBckI7QUFDSDtBQUNELFdBQUksVUFBSixFQUFnQjtBQUNkO0FBQ0E7QUFDQSxhQUFJLEtBQUssVUFBTCxDQUFnQixDQUFoQixNQUF1QixVQUEzQixFQUF1QztBQUNyQyxlQUFJLE1BQU0sS0FBSyxVQUFMLENBQWdCLENBQWhCLENBQU4sQ0FBSixFQUNFLEtBQUssVUFBTCxHQUFrQixDQUFDLFVBQUQsRUFBYSxDQUFiLENBQWxCO0FBQ0gsVUFIRCxNQUlFLEtBQUssVUFBTCxHQUFrQixDQUFDLFVBQUQsRUFBYSxDQUFiLENBQWxCO0FBQ0g7QUFDRDs7QUFFRjtBQUNFLGFBQU0sSUFBSSxLQUFKLENBQVUsaUNBQWlDLE9BQTNDLENBQU47QUF4Rko7QUEwRkEsUUFBSyxNQUFMO0FBQ0EsUUFBSyxHQUFMLEdBQVcsS0FBSyxPQUFoQjtBQUNBLFVBQU8sSUFBUDtBQUNELEVBOUZEOztBQWdHQSxTQUFRLEdBQVIsR0FBYyxHQUFkO0FBQ0EsVUFBUyxHQUFULENBQWEsT0FBYixFQUFzQixPQUF0QixFQUErQixLQUEvQixFQUFzQyxVQUF0QyxFQUFrRDtBQUNoRCxPQUFJLE9BQU8sS0FBUCxLQUFrQixRQUF0QixFQUFnQztBQUM5QixrQkFBYSxLQUFiO0FBQ0EsYUFBUSxTQUFSO0FBQ0Q7O0FBRUQsT0FBSTtBQUNGLFlBQU8sSUFBSSxNQUFKLENBQVcsT0FBWCxFQUFvQixLQUFwQixFQUEyQixHQUEzQixDQUErQixPQUEvQixFQUF3QyxVQUF4QyxFQUFvRCxPQUEzRDtBQUNELElBRkQsQ0FFRSxPQUFPLEVBQVAsRUFBVztBQUNYLFlBQU8sSUFBUDtBQUNEO0FBQ0Y7O0FBRUQsU0FBUSxJQUFSLEdBQWUsSUFBZjtBQUNBLFVBQVMsSUFBVCxDQUFjLFFBQWQsRUFBd0IsUUFBeEIsRUFBa0M7QUFDaEMsT0FBSSxHQUFHLFFBQUgsRUFBYSxRQUFiLENBQUosRUFBNEI7QUFDMUIsWUFBTyxJQUFQO0FBQ0QsSUFGRCxNQUVPO0FBQ0wsU0FBSSxLQUFLLE1BQU0sUUFBTixDQUFUO0FBQ0EsU0FBSSxLQUFLLE1BQU0sUUFBTixDQUFUO0FBQ0EsU0FBSSxHQUFHLFVBQUgsQ0FBYyxNQUFkLElBQXdCLEdBQUcsVUFBSCxDQUFjLE1BQTFDLEVBQWtEO0FBQ2hELFlBQUssSUFBSSxHQUFULElBQWdCLEVBQWhCLEVBQW9CO0FBQ2xCLGFBQUksUUFBUSxPQUFSLElBQW1CLFFBQVEsT0FBM0IsSUFBc0MsUUFBUSxPQUFsRCxFQUEyRDtBQUN6RCxlQUFJLEdBQUcsR0FBSCxNQUFZLEdBQUcsR0FBSCxDQUFoQixFQUF5QjtBQUN2QixvQkFBTyxRQUFNLEdBQWI7QUFDRDtBQUNGO0FBQ0Y7QUFDRCxjQUFPLFlBQVA7QUFDRDtBQUNELFVBQUssSUFBSSxHQUFULElBQWdCLEVBQWhCLEVBQW9CO0FBQ2xCLFdBQUksUUFBUSxPQUFSLElBQW1CLFFBQVEsT0FBM0IsSUFBc0MsUUFBUSxPQUFsRCxFQUEyRDtBQUN6RCxhQUFJLEdBQUcsR0FBSCxNQUFZLEdBQUcsR0FBSCxDQUFoQixFQUF5QjtBQUN2QixrQkFBTyxHQUFQO0FBQ0Q7QUFDRjtBQUNGO0FBQ0Y7QUFDRjs7QUFFRCxTQUFRLGtCQUFSLEdBQTZCLGtCQUE3Qjs7QUFFQSxLQUFJLFVBQVUsVUFBZDtBQUNBLFVBQVMsa0JBQVQsQ0FBNEIsQ0FBNUIsRUFBK0IsQ0FBL0IsRUFBa0M7QUFDaEMsT0FBSSxPQUFPLFFBQVEsSUFBUixDQUFhLENBQWIsQ0FBWDtBQUNBLE9BQUksT0FBTyxRQUFRLElBQVIsQ0FBYSxDQUFiLENBQVg7O0FBRUEsT0FBSSxRQUFRLElBQVosRUFBa0I7QUFDaEIsU0FBSSxDQUFDLENBQUw7QUFDQSxTQUFJLENBQUMsQ0FBTDtBQUNEOztBQUVELFVBQVEsUUFBUSxDQUFDLElBQVYsR0FBa0IsQ0FBQyxDQUFuQixHQUNDLFFBQVEsQ0FBQyxJQUFWLEdBQWtCLENBQWxCLEdBQ0EsSUFBSSxDQUFKLEdBQVEsQ0FBQyxDQUFULEdBQ0EsSUFBSSxDQUFKLEdBQVEsQ0FBUixHQUNBLENBSlA7QUFLRDs7QUFFRCxTQUFRLG1CQUFSLEdBQThCLG1CQUE5QjtBQUNBLFVBQVMsbUJBQVQsQ0FBNkIsQ0FBN0IsRUFBZ0MsQ0FBaEMsRUFBbUM7QUFDakMsVUFBTyxtQkFBbUIsQ0FBbkIsRUFBc0IsQ0FBdEIsQ0FBUDtBQUNEOztBQUVELFNBQVEsS0FBUixHQUFnQixLQUFoQjtBQUNBLFVBQVMsS0FBVCxDQUFlLENBQWYsRUFBa0IsS0FBbEIsRUFBeUI7QUFDdkIsVUFBTyxJQUFJLE1BQUosQ0FBVyxDQUFYLEVBQWMsS0FBZCxFQUFxQixLQUE1QjtBQUNEOztBQUVELFNBQVEsS0FBUixHQUFnQixLQUFoQjtBQUNBLFVBQVMsS0FBVCxDQUFlLENBQWYsRUFBa0IsS0FBbEIsRUFBeUI7QUFDdkIsVUFBTyxJQUFJLE1BQUosQ0FBVyxDQUFYLEVBQWMsS0FBZCxFQUFxQixLQUE1QjtBQUNEOztBQUVELFNBQVEsS0FBUixHQUFnQixLQUFoQjtBQUNBLFVBQVMsS0FBVCxDQUFlLENBQWYsRUFBa0IsS0FBbEIsRUFBeUI7QUFDdkIsVUFBTyxJQUFJLE1BQUosQ0FBVyxDQUFYLEVBQWMsS0FBZCxFQUFxQixLQUE1QjtBQUNEOztBQUVELFNBQVEsT0FBUixHQUFrQixPQUFsQjtBQUNBLFVBQVMsT0FBVCxDQUFpQixDQUFqQixFQUFvQixDQUFwQixFQUF1QixLQUF2QixFQUE4QjtBQUM1QixVQUFPLElBQUksTUFBSixDQUFXLENBQVgsRUFBYyxLQUFkLEVBQXFCLE9BQXJCLENBQTZCLENBQTdCLENBQVA7QUFDRDs7QUFFRCxTQUFRLFlBQVIsR0FBdUIsWUFBdkI7QUFDQSxVQUFTLFlBQVQsQ0FBc0IsQ0FBdEIsRUFBeUIsQ0FBekIsRUFBNEI7QUFDMUIsVUFBTyxRQUFRLENBQVIsRUFBVyxDQUFYLEVBQWMsSUFBZCxDQUFQO0FBQ0Q7O0FBRUQsU0FBUSxRQUFSLEdBQW1CLFFBQW5CO0FBQ0EsVUFBUyxRQUFULENBQWtCLENBQWxCLEVBQXFCLENBQXJCLEVBQXdCLEtBQXhCLEVBQStCO0FBQzdCLFVBQU8sUUFBUSxDQUFSLEVBQVcsQ0FBWCxFQUFjLEtBQWQsQ0FBUDtBQUNEOztBQUVELFNBQVEsSUFBUixHQUFlLElBQWY7QUFDQSxVQUFTLElBQVQsQ0FBYyxJQUFkLEVBQW9CLEtBQXBCLEVBQTJCO0FBQ3pCLFVBQU8sS0FBSyxJQUFMLENBQVUsVUFBUyxDQUFULEVBQVksQ0FBWixFQUFlO0FBQzlCLFlBQU8sUUFBUSxPQUFSLENBQWdCLENBQWhCLEVBQW1CLENBQW5CLEVBQXNCLEtBQXRCLENBQVA7QUFDRCxJQUZNLENBQVA7QUFHRDs7QUFFRCxTQUFRLEtBQVIsR0FBZ0IsS0FBaEI7QUFDQSxVQUFTLEtBQVQsQ0FBZSxJQUFmLEVBQXFCLEtBQXJCLEVBQTRCO0FBQzFCLFVBQU8sS0FBSyxJQUFMLENBQVUsVUFBUyxDQUFULEVBQVksQ0FBWixFQUFlO0FBQzlCLFlBQU8sUUFBUSxRQUFSLENBQWlCLENBQWpCLEVBQW9CLENBQXBCLEVBQXVCLEtBQXZCLENBQVA7QUFDRCxJQUZNLENBQVA7QUFHRDs7QUFFRCxTQUFRLEVBQVIsR0FBYSxFQUFiO0FBQ0EsVUFBUyxFQUFULENBQVksQ0FBWixFQUFlLENBQWYsRUFBa0IsS0FBbEIsRUFBeUI7QUFDdkIsVUFBTyxRQUFRLENBQVIsRUFBVyxDQUFYLEVBQWMsS0FBZCxJQUF1QixDQUE5QjtBQUNEOztBQUVELFNBQVEsRUFBUixHQUFhLEVBQWI7QUFDQSxVQUFTLEVBQVQsQ0FBWSxDQUFaLEVBQWUsQ0FBZixFQUFrQixLQUFsQixFQUF5QjtBQUN2QixVQUFPLFFBQVEsQ0FBUixFQUFXLENBQVgsRUFBYyxLQUFkLElBQXVCLENBQTlCO0FBQ0Q7O0FBRUQsU0FBUSxFQUFSLEdBQWEsRUFBYjtBQUNBLFVBQVMsRUFBVCxDQUFZLENBQVosRUFBZSxDQUFmLEVBQWtCLEtBQWxCLEVBQXlCO0FBQ3ZCLFVBQU8sUUFBUSxDQUFSLEVBQVcsQ0FBWCxFQUFjLEtBQWQsTUFBeUIsQ0FBaEM7QUFDRDs7QUFFRCxTQUFRLEdBQVIsR0FBYyxHQUFkO0FBQ0EsVUFBUyxHQUFULENBQWEsQ0FBYixFQUFnQixDQUFoQixFQUFtQixLQUFuQixFQUEwQjtBQUN4QixVQUFPLFFBQVEsQ0FBUixFQUFXLENBQVgsRUFBYyxLQUFkLE1BQXlCLENBQWhDO0FBQ0Q7O0FBRUQsU0FBUSxHQUFSLEdBQWMsR0FBZDtBQUNBLFVBQVMsR0FBVCxDQUFhLENBQWIsRUFBZ0IsQ0FBaEIsRUFBbUIsS0FBbkIsRUFBMEI7QUFDeEIsVUFBTyxRQUFRLENBQVIsRUFBVyxDQUFYLEVBQWMsS0FBZCxLQUF3QixDQUEvQjtBQUNEOztBQUVELFNBQVEsR0FBUixHQUFjLEdBQWQ7QUFDQSxVQUFTLEdBQVQsQ0FBYSxDQUFiLEVBQWdCLENBQWhCLEVBQW1CLEtBQW5CLEVBQTBCO0FBQ3hCLFVBQU8sUUFBUSxDQUFSLEVBQVcsQ0FBWCxFQUFjLEtBQWQsS0FBd0IsQ0FBL0I7QUFDRDs7QUFFRCxTQUFRLEdBQVIsR0FBYyxHQUFkO0FBQ0EsVUFBUyxHQUFULENBQWEsQ0FBYixFQUFnQixFQUFoQixFQUFvQixDQUFwQixFQUF1QixLQUF2QixFQUE4QjtBQUM1QixPQUFJLEdBQUo7QUFDQSxXQUFRLEVBQVI7QUFDRSxVQUFLLEtBQUw7QUFDRSxXQUFJLFFBQU8sQ0FBUCx5Q0FBTyxDQUFQLE9BQWEsUUFBakIsRUFBMkIsSUFBSSxFQUFFLE9BQU47QUFDM0IsV0FBSSxRQUFPLENBQVAseUNBQU8sQ0FBUCxPQUFhLFFBQWpCLEVBQTJCLElBQUksRUFBRSxPQUFOO0FBQzNCLGFBQU0sTUFBTSxDQUFaO0FBQ0E7QUFDRixVQUFLLEtBQUw7QUFDRSxXQUFJLFFBQU8sQ0FBUCx5Q0FBTyxDQUFQLE9BQWEsUUFBakIsRUFBMkIsSUFBSSxFQUFFLE9BQU47QUFDM0IsV0FBSSxRQUFPLENBQVAseUNBQU8sQ0FBUCxPQUFhLFFBQWpCLEVBQTJCLElBQUksRUFBRSxPQUFOO0FBQzNCLGFBQU0sTUFBTSxDQUFaO0FBQ0E7QUFDRixVQUFLLEVBQUwsQ0FBUyxLQUFLLEdBQUwsQ0FBVSxLQUFLLElBQUw7QUFBVyxhQUFNLEdBQUcsQ0FBSCxFQUFNLENBQU4sRUFBUyxLQUFULENBQU4sQ0FBdUI7QUFDckQsVUFBSyxJQUFMO0FBQVcsYUFBTSxJQUFJLENBQUosRUFBTyxDQUFQLEVBQVUsS0FBVixDQUFOLENBQXdCO0FBQ25DLFVBQUssR0FBTDtBQUFVLGFBQU0sR0FBRyxDQUFILEVBQU0sQ0FBTixFQUFTLEtBQVQsQ0FBTixDQUF1QjtBQUNqQyxVQUFLLElBQUw7QUFBVyxhQUFNLElBQUksQ0FBSixFQUFPLENBQVAsRUFBVSxLQUFWLENBQU4sQ0FBd0I7QUFDbkMsVUFBSyxHQUFMO0FBQVUsYUFBTSxHQUFHLENBQUgsRUFBTSxDQUFOLEVBQVMsS0FBVCxDQUFOLENBQXVCO0FBQ2pDLFVBQUssSUFBTDtBQUFXLGFBQU0sSUFBSSxDQUFKLEVBQU8sQ0FBUCxFQUFVLEtBQVYsQ0FBTixDQUF3QjtBQUNuQztBQUFTLGFBQU0sSUFBSSxTQUFKLENBQWMsdUJBQXVCLEVBQXJDLENBQU47QUFqQlg7QUFtQkEsVUFBTyxHQUFQO0FBQ0Q7O0FBRUQsU0FBUSxVQUFSLEdBQXFCLFVBQXJCO0FBQ0EsVUFBUyxVQUFULENBQW9CLElBQXBCLEVBQTBCLEtBQTFCLEVBQWlDO0FBQy9CLE9BQUksZ0JBQWdCLFVBQXBCLEVBQWdDO0FBQzlCLFNBQUksS0FBSyxLQUFMLEtBQWUsS0FBbkIsRUFDRSxPQUFPLElBQVAsQ0FERixLQUdFLE9BQU8sS0FBSyxLQUFaO0FBQ0g7O0FBRUQsT0FBSSxFQUFFLGdCQUFnQixVQUFsQixDQUFKLEVBQ0UsT0FBTyxJQUFJLFVBQUosQ0FBZSxJQUFmLEVBQXFCLEtBQXJCLENBQVA7O0FBRUYsU0FBTSxZQUFOLEVBQW9CLElBQXBCLEVBQTBCLEtBQTFCO0FBQ0EsUUFBSyxLQUFMLEdBQWEsS0FBYjtBQUNBLFFBQUssS0FBTCxDQUFXLElBQVg7O0FBRUEsT0FBSSxLQUFLLE1BQUwsS0FBZ0IsR0FBcEIsRUFDRSxLQUFLLEtBQUwsR0FBYSxFQUFiLENBREYsS0FHRSxLQUFLLEtBQUwsR0FBYSxLQUFLLFFBQUwsR0FBZ0IsS0FBSyxNQUFMLENBQVksT0FBekM7O0FBRUYsU0FBTSxNQUFOLEVBQWMsSUFBZDtBQUNEOztBQUVELEtBQUksTUFBTSxFQUFWO0FBQ0EsWUFBVyxTQUFYLENBQXFCLEtBQXJCLEdBQTZCLFVBQVMsSUFBVCxFQUFlO0FBQzFDLE9BQUksSUFBSSxLQUFLLEtBQUwsR0FBYSxHQUFHLGVBQUgsQ0FBYixHQUFtQyxHQUFHLFVBQUgsQ0FBM0M7QUFDQSxPQUFJLElBQUksS0FBSyxLQUFMLENBQVcsQ0FBWCxDQUFSOztBQUVBLE9BQUksQ0FBQyxDQUFMLEVBQ0UsTUFBTSxJQUFJLFNBQUosQ0FBYyx5QkFBeUIsSUFBdkMsQ0FBTjs7QUFFRixRQUFLLFFBQUwsR0FBZ0IsRUFBRSxDQUFGLENBQWhCO0FBQ0EsT0FBSSxLQUFLLFFBQUwsS0FBa0IsR0FBdEIsRUFDRSxLQUFLLFFBQUwsR0FBZ0IsRUFBaEI7O0FBRUY7QUFDQSxPQUFJLENBQUMsRUFBRSxDQUFGLENBQUwsRUFDRSxLQUFLLE1BQUwsR0FBYyxHQUFkLENBREYsS0FHRSxLQUFLLE1BQUwsR0FBYyxJQUFJLE1BQUosQ0FBVyxFQUFFLENBQUYsQ0FBWCxFQUFpQixLQUFLLEtBQXRCLENBQWQ7QUFDSCxFQWhCRDs7QUFrQkEsWUFBVyxTQUFYLENBQXFCLFFBQXJCLEdBQWdDLFlBQVc7QUFDekMsVUFBTyxLQUFLLEtBQVo7QUFDRCxFQUZEOztBQUlBLFlBQVcsU0FBWCxDQUFxQixJQUFyQixHQUE0QixVQUFTLE9BQVQsRUFBa0I7QUFDNUMsU0FBTSxpQkFBTixFQUF5QixPQUF6QixFQUFrQyxLQUFLLEtBQXZDOztBQUVBLE9BQUksS0FBSyxNQUFMLEtBQWdCLEdBQXBCLEVBQ0UsT0FBTyxJQUFQOztBQUVGLE9BQUksT0FBTyxPQUFQLEtBQW1CLFFBQXZCLEVBQ0UsVUFBVSxJQUFJLE1BQUosQ0FBVyxPQUFYLEVBQW9CLEtBQUssS0FBekIsQ0FBVjs7QUFFRixVQUFPLElBQUksT0FBSixFQUFhLEtBQUssUUFBbEIsRUFBNEIsS0FBSyxNQUFqQyxFQUF5QyxLQUFLLEtBQTlDLENBQVA7QUFDRCxFQVZEOztBQWFBLFNBQVEsS0FBUixHQUFnQixLQUFoQjtBQUNBLFVBQVMsS0FBVCxDQUFlLEtBQWYsRUFBc0IsS0FBdEIsRUFBNkI7QUFDM0IsT0FBSyxpQkFBaUIsS0FBbEIsSUFBNEIsTUFBTSxLQUFOLEtBQWdCLEtBQWhELEVBQ0UsT0FBTyxLQUFQOztBQUVGLE9BQUksRUFBRSxnQkFBZ0IsS0FBbEIsQ0FBSixFQUNFLE9BQU8sSUFBSSxLQUFKLENBQVUsS0FBVixFQUFpQixLQUFqQixDQUFQOztBQUVGLFFBQUssS0FBTCxHQUFhLEtBQWI7O0FBRUE7QUFDQSxRQUFLLEdBQUwsR0FBVyxLQUFYO0FBQ0EsUUFBSyxHQUFMLEdBQVcsTUFBTSxLQUFOLENBQVksWUFBWixFQUEwQixHQUExQixDQUE4QixVQUFTLEtBQVQsRUFBZ0I7QUFDdkQsWUFBTyxLQUFLLFVBQUwsQ0FBZ0IsTUFBTSxJQUFOLEVBQWhCLENBQVA7QUFDRCxJQUZVLEVBRVIsSUFGUSxFQUVGLE1BRkUsQ0FFSyxVQUFTLENBQVQsRUFBWTtBQUMxQjtBQUNBLFlBQU8sRUFBRSxNQUFUO0FBQ0QsSUFMVSxDQUFYOztBQU9BLE9BQUksQ0FBQyxLQUFLLEdBQUwsQ0FBUyxNQUFkLEVBQXNCO0FBQ3BCLFdBQU0sSUFBSSxTQUFKLENBQWMsMkJBQTJCLEtBQXpDLENBQU47QUFDRDs7QUFFRCxRQUFLLE1BQUw7QUFDRDs7QUFFRCxPQUFNLFNBQU4sQ0FBZ0IsTUFBaEIsR0FBeUIsWUFBVztBQUNsQyxRQUFLLEtBQUwsR0FBYSxLQUFLLEdBQUwsQ0FBUyxHQUFULENBQWEsVUFBUyxLQUFULEVBQWdCO0FBQ3hDLFlBQU8sTUFBTSxJQUFOLENBQVcsR0FBWCxFQUFnQixJQUFoQixFQUFQO0FBQ0QsSUFGWSxFQUVWLElBRlUsQ0FFTCxJQUZLLEVBRUMsSUFGRCxFQUFiO0FBR0EsVUFBTyxLQUFLLEtBQVo7QUFDRCxFQUxEOztBQU9BLE9BQU0sU0FBTixDQUFnQixRQUFoQixHQUEyQixZQUFXO0FBQ3BDLFVBQU8sS0FBSyxLQUFaO0FBQ0QsRUFGRDs7QUFJQSxPQUFNLFNBQU4sQ0FBZ0IsVUFBaEIsR0FBNkIsVUFBUyxLQUFULEVBQWdCO0FBQzNDLE9BQUksUUFBUSxLQUFLLEtBQWpCO0FBQ0EsV0FBUSxNQUFNLElBQU4sRUFBUjtBQUNBLFNBQU0sT0FBTixFQUFlLEtBQWYsRUFBc0IsS0FBdEI7QUFDQTtBQUNBLE9BQUksS0FBSyxRQUFRLEdBQUcsZ0JBQUgsQ0FBUixHQUErQixHQUFHLFdBQUgsQ0FBeEM7QUFDQSxXQUFRLE1BQU0sT0FBTixDQUFjLEVBQWQsRUFBa0IsYUFBbEIsQ0FBUjtBQUNBLFNBQU0sZ0JBQU4sRUFBd0IsS0FBeEI7QUFDQTtBQUNBLFdBQVEsTUFBTSxPQUFOLENBQWMsR0FBRyxjQUFILENBQWQsRUFBa0MscUJBQWxDLENBQVI7QUFDQSxTQUFNLGlCQUFOLEVBQXlCLEtBQXpCLEVBQWdDLEdBQUcsY0FBSCxDQUFoQzs7QUFFQTtBQUNBLFdBQVEsTUFBTSxPQUFOLENBQWMsR0FBRyxTQUFILENBQWQsRUFBNkIsZ0JBQTdCLENBQVI7O0FBRUE7QUFDQSxXQUFRLE1BQU0sT0FBTixDQUFjLEdBQUcsU0FBSCxDQUFkLEVBQTZCLGdCQUE3QixDQUFSOztBQUVBO0FBQ0EsV0FBUSxNQUFNLEtBQU4sQ0FBWSxLQUFaLEVBQW1CLElBQW5CLENBQXdCLEdBQXhCLENBQVI7O0FBRUE7QUFDQTs7QUFFQSxPQUFJLFNBQVMsUUFBUSxHQUFHLGVBQUgsQ0FBUixHQUE4QixHQUFHLFVBQUgsQ0FBM0M7QUFDQSxPQUFJLE1BQU0sTUFBTSxLQUFOLENBQVksR0FBWixFQUFpQixHQUFqQixDQUFxQixVQUFTLElBQVQsRUFBZTtBQUM1QyxZQUFPLGdCQUFnQixJQUFoQixFQUFzQixLQUF0QixDQUFQO0FBQ0QsSUFGUyxFQUVQLElBRk8sQ0FFRixHQUZFLEVBRUcsS0FGSCxDQUVTLEtBRlQsQ0FBVjtBQUdBLE9BQUksS0FBSyxLQUFULEVBQWdCO0FBQ2Q7QUFDQSxXQUFNLElBQUksTUFBSixDQUFXLFVBQVMsSUFBVCxFQUFlO0FBQzlCLGNBQU8sQ0FBQyxDQUFDLEtBQUssS0FBTCxDQUFXLE1BQVgsQ0FBVDtBQUNELE1BRkssQ0FBTjtBQUdEO0FBQ0QsU0FBTSxJQUFJLEdBQUosQ0FBUSxVQUFTLElBQVQsRUFBZTtBQUMzQixZQUFPLElBQUksVUFBSixDQUFlLElBQWYsRUFBcUIsS0FBckIsQ0FBUDtBQUNELElBRkssQ0FBTjs7QUFJQSxVQUFPLEdBQVA7QUFDRCxFQXZDRDs7QUF5Q0E7QUFDQSxTQUFRLGFBQVIsR0FBd0IsYUFBeEI7QUFDQSxVQUFTLGFBQVQsQ0FBdUIsS0FBdkIsRUFBOEIsS0FBOUIsRUFBcUM7QUFDbkMsVUFBTyxJQUFJLEtBQUosQ0FBVSxLQUFWLEVBQWlCLEtBQWpCLEVBQXdCLEdBQXhCLENBQTRCLEdBQTVCLENBQWdDLFVBQVMsSUFBVCxFQUFlO0FBQ3BELFlBQU8sS0FBSyxHQUFMLENBQVMsVUFBUyxDQUFULEVBQVk7QUFDMUIsY0FBTyxFQUFFLEtBQVQ7QUFDRCxNQUZNLEVBRUosSUFGSSxDQUVDLEdBRkQsRUFFTSxJQUZOLEdBRWEsS0FGYixDQUVtQixHQUZuQixDQUFQO0FBR0QsSUFKTSxDQUFQO0FBS0Q7O0FBRUQ7QUFDQTtBQUNBO0FBQ0EsVUFBUyxlQUFULENBQXlCLElBQXpCLEVBQStCLEtBQS9CLEVBQXNDO0FBQ3BDLFNBQU0sTUFBTixFQUFjLElBQWQ7QUFDQSxVQUFPLGNBQWMsSUFBZCxFQUFvQixLQUFwQixDQUFQO0FBQ0EsU0FBTSxPQUFOLEVBQWUsSUFBZjtBQUNBLFVBQU8sY0FBYyxJQUFkLEVBQW9CLEtBQXBCLENBQVA7QUFDQSxTQUFNLFFBQU4sRUFBZ0IsSUFBaEI7QUFDQSxVQUFPLGVBQWUsSUFBZixFQUFxQixLQUFyQixDQUFQO0FBQ0EsU0FBTSxRQUFOLEVBQWdCLElBQWhCO0FBQ0EsVUFBTyxhQUFhLElBQWIsRUFBbUIsS0FBbkIsQ0FBUDtBQUNBLFNBQU0sT0FBTixFQUFlLElBQWY7QUFDQSxVQUFPLElBQVA7QUFDRDs7QUFFRCxVQUFTLEdBQVQsQ0FBYSxFQUFiLEVBQWlCO0FBQ2YsVUFBTyxDQUFDLEVBQUQsSUFBTyxHQUFHLFdBQUgsT0FBcUIsR0FBNUIsSUFBbUMsT0FBTyxHQUFqRDtBQUNEOztBQUVEO0FBQ0E7QUFDQTtBQUNBO0FBQ0E7QUFDQTtBQUNBLFVBQVMsYUFBVCxDQUF1QixJQUF2QixFQUE2QixLQUE3QixFQUFvQztBQUNsQyxVQUFPLEtBQUssSUFBTCxHQUFZLEtBQVosQ0FBa0IsS0FBbEIsRUFBeUIsR0FBekIsQ0FBNkIsVUFBUyxJQUFULEVBQWU7QUFDakQsWUFBTyxhQUFhLElBQWIsRUFBbUIsS0FBbkIsQ0FBUDtBQUNELElBRk0sRUFFSixJQUZJLENBRUMsR0FGRCxDQUFQO0FBR0Q7O0FBRUQsVUFBUyxZQUFULENBQXNCLElBQXRCLEVBQTRCLEtBQTVCLEVBQW1DO0FBQ2pDLE9BQUksSUFBSSxRQUFRLEdBQUcsVUFBSCxDQUFSLEdBQXlCLEdBQUcsS0FBSCxDQUFqQztBQUNBLFVBQU8sS0FBSyxPQUFMLENBQWEsQ0FBYixFQUFnQixVQUFTLENBQVQsRUFBWSxDQUFaLEVBQWUsQ0FBZixFQUFrQixDQUFsQixFQUFxQixFQUFyQixFQUF5QjtBQUM5QyxXQUFNLE9BQU4sRUFBZSxJQUFmLEVBQXFCLENBQXJCLEVBQXdCLENBQXhCLEVBQTJCLENBQTNCLEVBQThCLENBQTlCLEVBQWlDLEVBQWpDO0FBQ0EsU0FBSSxHQUFKOztBQUVBLFNBQUksSUFBSSxDQUFKLENBQUosRUFDRSxNQUFNLEVBQU4sQ0FERixLQUVLLElBQUksSUFBSSxDQUFKLENBQUosRUFDSCxNQUFNLE9BQU8sQ0FBUCxHQUFXLFFBQVgsSUFBdUIsQ0FBQyxDQUFELEdBQUssQ0FBNUIsSUFBaUMsTUFBdkMsQ0FERyxLQUVBLElBQUksSUFBSSxDQUFKLENBQUo7QUFDSDtBQUNBLGFBQU0sT0FBTyxDQUFQLEdBQVcsR0FBWCxHQUFpQixDQUFqQixHQUFxQixNQUFyQixHQUE4QixDQUE5QixHQUFrQyxHQUFsQyxJQUF5QyxDQUFDLENBQUQsR0FBSyxDQUE5QyxJQUFtRCxJQUF6RCxDQUZHLEtBR0EsSUFBSSxFQUFKLEVBQVE7QUFDWCxhQUFNLGlCQUFOLEVBQXlCLEVBQXpCO0FBQ0EsV0FBSSxHQUFHLE1BQUgsQ0FBVSxDQUFWLE1BQWlCLEdBQXJCLEVBQ0UsS0FBSyxNQUFNLEVBQVg7QUFDRixhQUFNLE9BQU8sQ0FBUCxHQUFXLEdBQVgsR0FBaUIsQ0FBakIsR0FBcUIsR0FBckIsR0FBMkIsQ0FBM0IsR0FBK0IsRUFBL0IsR0FDQSxJQURBLEdBQ08sQ0FEUCxHQUNXLEdBRFgsSUFDa0IsQ0FBQyxDQUFELEdBQUssQ0FEdkIsSUFDNEIsSUFEbEM7QUFFRCxNQU5JO0FBT0g7QUFDQSxhQUFNLE9BQU8sQ0FBUCxHQUFXLEdBQVgsR0FBaUIsQ0FBakIsR0FBcUIsR0FBckIsR0FBMkIsQ0FBM0IsR0FDQSxJQURBLEdBQ08sQ0FEUCxHQUNXLEdBRFgsSUFDa0IsQ0FBQyxDQUFELEdBQUssQ0FEdkIsSUFDNEIsSUFEbEM7O0FBR0YsV0FBTSxjQUFOLEVBQXNCLEdBQXRCO0FBQ0EsWUFBTyxHQUFQO0FBQ0QsSUF4Qk0sQ0FBUDtBQXlCRDs7QUFFRDtBQUNBO0FBQ0E7QUFDQTtBQUNBO0FBQ0E7QUFDQSxVQUFTLGFBQVQsQ0FBdUIsSUFBdkIsRUFBNkIsS0FBN0IsRUFBb0M7QUFDbEMsVUFBTyxLQUFLLElBQUwsR0FBWSxLQUFaLENBQWtCLEtBQWxCLEVBQXlCLEdBQXpCLENBQTZCLFVBQVMsSUFBVCxFQUFlO0FBQ2pELFlBQU8sYUFBYSxJQUFiLEVBQW1CLEtBQW5CLENBQVA7QUFDRCxJQUZNLEVBRUosSUFGSSxDQUVDLEdBRkQsQ0FBUDtBQUdEOztBQUVELFVBQVMsWUFBVCxDQUFzQixJQUF0QixFQUE0QixLQUE1QixFQUFtQztBQUNqQyxTQUFNLE9BQU4sRUFBZSxJQUFmLEVBQXFCLEtBQXJCO0FBQ0EsT0FBSSxJQUFJLFFBQVEsR0FBRyxVQUFILENBQVIsR0FBeUIsR0FBRyxLQUFILENBQWpDO0FBQ0EsVUFBTyxLQUFLLE9BQUwsQ0FBYSxDQUFiLEVBQWdCLFVBQVMsQ0FBVCxFQUFZLENBQVosRUFBZSxDQUFmLEVBQWtCLENBQWxCLEVBQXFCLEVBQXJCLEVBQXlCO0FBQzlDLFdBQU0sT0FBTixFQUFlLElBQWYsRUFBcUIsQ0FBckIsRUFBd0IsQ0FBeEIsRUFBMkIsQ0FBM0IsRUFBOEIsQ0FBOUIsRUFBaUMsRUFBakM7QUFDQSxTQUFJLEdBQUo7O0FBRUEsU0FBSSxJQUFJLENBQUosQ0FBSixFQUNFLE1BQU0sRUFBTixDQURGLEtBRUssSUFBSSxJQUFJLENBQUosQ0FBSixFQUNILE1BQU0sT0FBTyxDQUFQLEdBQVcsUUFBWCxJQUF1QixDQUFDLENBQUQsR0FBSyxDQUE1QixJQUFpQyxNQUF2QyxDQURHLEtBRUEsSUFBSSxJQUFJLENBQUosQ0FBSixFQUFZO0FBQ2YsV0FBSSxNQUFNLEdBQVYsRUFDRSxNQUFNLE9BQU8sQ0FBUCxHQUFXLEdBQVgsR0FBaUIsQ0FBakIsR0FBcUIsTUFBckIsR0FBOEIsQ0FBOUIsR0FBa0MsR0FBbEMsSUFBeUMsQ0FBQyxDQUFELEdBQUssQ0FBOUMsSUFBbUQsSUFBekQsQ0FERixLQUdFLE1BQU0sT0FBTyxDQUFQLEdBQVcsR0FBWCxHQUFpQixDQUFqQixHQUFxQixNQUFyQixJQUErQixDQUFDLENBQUQsR0FBSyxDQUFwQyxJQUF5QyxNQUEvQztBQUNILE1BTEksTUFLRSxJQUFJLEVBQUosRUFBUTtBQUNiLGFBQU0saUJBQU4sRUFBeUIsRUFBekI7QUFDQSxXQUFJLEdBQUcsTUFBSCxDQUFVLENBQVYsTUFBaUIsR0FBckIsRUFDRSxLQUFLLE1BQU0sRUFBWDtBQUNGLFdBQUksTUFBTSxHQUFWLEVBQWU7QUFDYixhQUFJLE1BQU0sR0FBVixFQUNFLE1BQU0sT0FBTyxDQUFQLEdBQVcsR0FBWCxHQUFpQixDQUFqQixHQUFxQixHQUFyQixHQUEyQixDQUEzQixHQUErQixFQUEvQixHQUNBLElBREEsR0FDTyxDQURQLEdBQ1csR0FEWCxHQUNpQixDQURqQixHQUNxQixHQURyQixJQUM0QixDQUFDLENBQUQsR0FBSyxDQURqQyxDQUFOLENBREYsS0FJRSxNQUFNLE9BQU8sQ0FBUCxHQUFXLEdBQVgsR0FBaUIsQ0FBakIsR0FBcUIsR0FBckIsR0FBMkIsQ0FBM0IsR0FBK0IsRUFBL0IsR0FDQSxJQURBLEdBQ08sQ0FEUCxHQUNXLEdBRFgsSUFDa0IsQ0FBQyxDQUFELEdBQUssQ0FEdkIsSUFDNEIsSUFEbEM7QUFFSCxRQVBELE1BUUUsTUFBTSxPQUFPLENBQVAsR0FBVyxHQUFYLEdBQWlCLENBQWpCLEdBQXFCLEdBQXJCLEdBQTJCLENBQTNCLEdBQStCLEVBQS9CLEdBQ0EsSUFEQSxJQUNRLENBQUMsQ0FBRCxHQUFLLENBRGIsSUFDa0IsTUFEeEI7QUFFSCxNQWRNLE1BY0E7QUFDTCxhQUFNLE9BQU47QUFDQSxXQUFJLE1BQU0sR0FBVixFQUFlO0FBQ2IsYUFBSSxNQUFNLEdBQVYsRUFDRSxNQUFNLE9BQU8sQ0FBUCxHQUFXLEdBQVgsR0FBaUIsQ0FBakIsR0FBcUIsR0FBckIsR0FBMkIsQ0FBM0IsR0FDQSxJQURBLEdBQ08sQ0FEUCxHQUNXLEdBRFgsR0FDaUIsQ0FEakIsR0FDcUIsR0FEckIsSUFDNEIsQ0FBQyxDQUFELEdBQUssQ0FEakMsQ0FBTixDQURGLEtBSUUsTUFBTSxPQUFPLENBQVAsR0FBVyxHQUFYLEdBQWlCLENBQWpCLEdBQXFCLEdBQXJCLEdBQTJCLENBQTNCLEdBQ0EsSUFEQSxHQUNPLENBRFAsR0FDVyxHQURYLElBQ2tCLENBQUMsQ0FBRCxHQUFLLENBRHZCLElBQzRCLElBRGxDO0FBRUgsUUFQRCxNQVFFLE1BQU0sT0FBTyxDQUFQLEdBQVcsR0FBWCxHQUFpQixDQUFqQixHQUFxQixHQUFyQixHQUEyQixDQUEzQixHQUNBLElBREEsSUFDUSxDQUFDLENBQUQsR0FBSyxDQURiLElBQ2tCLE1BRHhCO0FBRUg7O0FBRUQsV0FBTSxjQUFOLEVBQXNCLEdBQXRCO0FBQ0EsWUFBTyxHQUFQO0FBQ0QsSUEzQ00sQ0FBUDtBQTRDRDs7QUFFRCxVQUFTLGNBQVQsQ0FBd0IsSUFBeEIsRUFBOEIsS0FBOUIsRUFBcUM7QUFDbkMsU0FBTSxnQkFBTixFQUF3QixJQUF4QixFQUE4QixLQUE5QjtBQUNBLFVBQU8sS0FBSyxLQUFMLENBQVcsS0FBWCxFQUFrQixHQUFsQixDQUFzQixVQUFTLElBQVQsRUFBZTtBQUMxQyxZQUFPLGNBQWMsSUFBZCxFQUFvQixLQUFwQixDQUFQO0FBQ0QsSUFGTSxFQUVKLElBRkksQ0FFQyxHQUZELENBQVA7QUFHRDs7QUFFRCxVQUFTLGFBQVQsQ0FBdUIsSUFBdkIsRUFBNkIsS0FBN0IsRUFBb0M7QUFDbEMsVUFBTyxLQUFLLElBQUwsRUFBUDtBQUNBLE9BQUksSUFBSSxRQUFRLEdBQUcsV0FBSCxDQUFSLEdBQTBCLEdBQUcsTUFBSCxDQUFsQztBQUNBLFVBQU8sS0FBSyxPQUFMLENBQWEsQ0FBYixFQUFnQixVQUFTLEdBQVQsRUFBYyxJQUFkLEVBQW9CLENBQXBCLEVBQXVCLENBQXZCLEVBQTBCLENBQTFCLEVBQTZCLEVBQTdCLEVBQWlDO0FBQ3RELFdBQU0sUUFBTixFQUFnQixJQUFoQixFQUFzQixHQUF0QixFQUEyQixJQUEzQixFQUFpQyxDQUFqQyxFQUFvQyxDQUFwQyxFQUF1QyxDQUF2QyxFQUEwQyxFQUExQztBQUNBLFNBQUksS0FBSyxJQUFJLENBQUosQ0FBVDtBQUNBLFNBQUksS0FBSyxNQUFNLElBQUksQ0FBSixDQUFmO0FBQ0EsU0FBSSxLQUFLLE1BQU0sSUFBSSxDQUFKLENBQWY7QUFDQSxTQUFJLE9BQU8sRUFBWDs7QUFFQSxTQUFJLFNBQVMsR0FBVCxJQUFnQixJQUFwQixFQUNFLE9BQU8sRUFBUDs7QUFFRixTQUFJLEVBQUosRUFBUTtBQUNOLFdBQUksU0FBUyxHQUFULElBQWdCLFNBQVMsR0FBN0IsRUFBa0M7QUFDaEM7QUFDQSxlQUFNLFFBQU47QUFDRCxRQUhELE1BR087QUFDTDtBQUNBLGVBQU0sR0FBTjtBQUNEO0FBQ0YsTUFSRCxNQVFPLElBQUksUUFBUSxJQUFaLEVBQWtCO0FBQ3ZCO0FBQ0EsV0FBSSxFQUFKLEVBQ0UsSUFBSSxDQUFKO0FBQ0YsV0FBSSxFQUFKLEVBQ0UsSUFBSSxDQUFKOztBQUVGLFdBQUksU0FBUyxHQUFiLEVBQWtCO0FBQ2hCO0FBQ0E7QUFDQTtBQUNBLGdCQUFPLElBQVA7QUFDQSxhQUFJLEVBQUosRUFBUTtBQUNOLGVBQUksQ0FBQyxDQUFELEdBQUssQ0FBVDtBQUNBLGVBQUksQ0FBSjtBQUNBLGVBQUksQ0FBSjtBQUNELFVBSkQsTUFJTyxJQUFJLEVBQUosRUFBUTtBQUNiLGVBQUksQ0FBQyxDQUFELEdBQUssQ0FBVDtBQUNBLGVBQUksQ0FBSjtBQUNEO0FBQ0YsUUFiRCxNQWFPLElBQUksU0FBUyxJQUFiLEVBQW1CO0FBQ3hCO0FBQ0E7QUFDQSxnQkFBTyxHQUFQO0FBQ0EsYUFBSSxFQUFKLEVBQ0UsSUFBSSxDQUFDLENBQUQsR0FBSyxDQUFULENBREYsS0FHRSxJQUFJLENBQUMsQ0FBRCxHQUFLLENBQVQ7QUFDSDs7QUFFRCxhQUFNLE9BQU8sQ0FBUCxHQUFXLEdBQVgsR0FBaUIsQ0FBakIsR0FBcUIsR0FBckIsR0FBMkIsQ0FBakM7QUFDRCxNQS9CTSxNQStCQSxJQUFJLEVBQUosRUFBUTtBQUNiLGFBQU0sT0FBTyxDQUFQLEdBQVcsUUFBWCxJQUF1QixDQUFDLENBQUQsR0FBSyxDQUE1QixJQUFpQyxNQUF2QztBQUNELE1BRk0sTUFFQSxJQUFJLEVBQUosRUFBUTtBQUNiLGFBQU0sT0FBTyxDQUFQLEdBQVcsR0FBWCxHQUFpQixDQUFqQixHQUFxQixNQUFyQixHQUE4QixDQUE5QixHQUFrQyxHQUFsQyxJQUF5QyxDQUFDLENBQUQsR0FBSyxDQUE5QyxJQUFtRCxJQUF6RDtBQUNEOztBQUVELFdBQU0sZUFBTixFQUF1QixHQUF2Qjs7QUFFQSxZQUFPLEdBQVA7QUFDRCxJQTFETSxDQUFQO0FBMkREOztBQUVEO0FBQ0E7QUFDQSxVQUFTLFlBQVQsQ0FBc0IsSUFBdEIsRUFBNEIsS0FBNUIsRUFBbUM7QUFDakMsU0FBTSxjQUFOLEVBQXNCLElBQXRCLEVBQTRCLEtBQTVCO0FBQ0E7QUFDQSxVQUFPLEtBQUssSUFBTCxHQUFZLE9BQVosQ0FBb0IsR0FBRyxJQUFILENBQXBCLEVBQThCLEVBQTlCLENBQVA7QUFDRDs7QUFFRDtBQUNBO0FBQ0E7QUFDQTtBQUNBO0FBQ0EsVUFBUyxhQUFULENBQXVCLEVBQXZCLEVBQ3VCLElBRHZCLEVBQzZCLEVBRDdCLEVBQ2lDLEVBRGpDLEVBQ3FDLEVBRHJDLEVBQ3lDLEdBRHpDLEVBQzhDLEVBRDlDLEVBRXVCLEVBRnZCLEVBRTJCLEVBRjNCLEVBRStCLEVBRi9CLEVBRW1DLEVBRm5DLEVBRXVDLEdBRnZDLEVBRTRDLEVBRjVDLEVBRWdEOztBQUU5QyxPQUFJLElBQUksRUFBSixDQUFKLEVBQ0UsT0FBTyxFQUFQLENBREYsS0FFSyxJQUFJLElBQUksRUFBSixDQUFKLEVBQ0gsT0FBTyxPQUFPLEVBQVAsR0FBWSxNQUFuQixDQURHLEtBRUEsSUFBSSxJQUFJLEVBQUosQ0FBSixFQUNILE9BQU8sT0FBTyxFQUFQLEdBQVksR0FBWixHQUFrQixFQUFsQixHQUF1QixJQUE5QixDQURHLEtBR0gsT0FBTyxPQUFPLElBQWQ7O0FBRUYsT0FBSSxJQUFJLEVBQUosQ0FBSixFQUNFLEtBQUssRUFBTCxDQURGLEtBRUssSUFBSSxJQUFJLEVBQUosQ0FBSixFQUNILEtBQUssT0FBTyxDQUFDLEVBQUQsR0FBTSxDQUFiLElBQWtCLE1BQXZCLENBREcsS0FFQSxJQUFJLElBQUksRUFBSixDQUFKLEVBQ0gsS0FBSyxNQUFNLEVBQU4sR0FBVyxHQUFYLElBQWtCLENBQUMsRUFBRCxHQUFNLENBQXhCLElBQTZCLElBQWxDLENBREcsS0FFQSxJQUFJLEdBQUosRUFDSCxLQUFLLE9BQU8sRUFBUCxHQUFZLEdBQVosR0FBa0IsRUFBbEIsR0FBdUIsR0FBdkIsR0FBNkIsRUFBN0IsR0FBa0MsR0FBbEMsR0FBd0MsR0FBN0MsQ0FERyxLQUdILEtBQUssT0FBTyxFQUFaOztBQUVGLFVBQU8sQ0FBQyxPQUFPLEdBQVAsR0FBYSxFQUFkLEVBQWtCLElBQWxCLEVBQVA7QUFDRDs7QUFHRDtBQUNBLE9BQU0sU0FBTixDQUFnQixJQUFoQixHQUF1QixVQUFTLE9BQVQsRUFBa0I7QUFDdkMsT0FBSSxDQUFDLE9BQUwsRUFDRSxPQUFPLEtBQVA7O0FBRUYsT0FBSSxPQUFPLE9BQVAsS0FBbUIsUUFBdkIsRUFDRSxVQUFVLElBQUksTUFBSixDQUFXLE9BQVgsRUFBb0IsS0FBSyxLQUF6QixDQUFWOztBQUVGLFFBQUssSUFBSSxJQUFJLENBQWIsRUFBZ0IsSUFBSSxLQUFLLEdBQUwsQ0FBUyxNQUE3QixFQUFxQyxHQUFyQyxFQUEwQztBQUN4QyxTQUFJLFFBQVEsS0FBSyxHQUFMLENBQVMsQ0FBVCxDQUFSLEVBQXFCLE9BQXJCLENBQUosRUFDRSxPQUFPLElBQVA7QUFDSDtBQUNELFVBQU8sS0FBUDtBQUNELEVBWkQ7O0FBY0EsVUFBUyxPQUFULENBQWlCLEdBQWpCLEVBQXNCLE9BQXRCLEVBQStCO0FBQzdCLFFBQUssSUFBSSxJQUFJLENBQWIsRUFBZ0IsSUFBSSxJQUFJLE1BQXhCLEVBQWdDLEdBQWhDLEVBQXFDO0FBQ25DLFNBQUksQ0FBQyxJQUFJLENBQUosRUFBTyxJQUFQLENBQVksT0FBWixDQUFMLEVBQ0UsT0FBTyxLQUFQO0FBQ0g7O0FBRUQsT0FBSSxRQUFRLFVBQVIsQ0FBbUIsTUFBdkIsRUFBK0I7QUFDN0I7QUFDQTtBQUNBO0FBQ0E7QUFDQTtBQUNBLFVBQUssSUFBSSxJQUFJLENBQWIsRUFBZ0IsSUFBSSxJQUFJLE1BQXhCLEVBQWdDLEdBQWhDLEVBQXFDO0FBQ25DLGFBQU0sSUFBSSxDQUFKLEVBQU8sTUFBYjtBQUNBLFdBQUksSUFBSSxDQUFKLEVBQU8sTUFBUCxLQUFrQixHQUF0QixFQUNFOztBQUVGLFdBQUksSUFBSSxDQUFKLEVBQU8sTUFBUCxDQUFjLFVBQWQsQ0FBeUIsTUFBekIsR0FBa0MsQ0FBdEMsRUFBeUM7QUFDdkMsYUFBSSxVQUFVLElBQUksQ0FBSixFQUFPLE1BQXJCO0FBQ0EsYUFBSSxRQUFRLEtBQVIsS0FBa0IsUUFBUSxLQUExQixJQUNBLFFBQVEsS0FBUixLQUFrQixRQUFRLEtBRDFCLElBRUEsUUFBUSxLQUFSLEtBQWtCLFFBQVEsS0FGOUIsRUFHRSxPQUFPLElBQVA7QUFDSDtBQUNGOztBQUVEO0FBQ0EsWUFBTyxLQUFQO0FBQ0Q7O0FBRUQsVUFBTyxJQUFQO0FBQ0Q7O0FBRUQsU0FBUSxTQUFSLEdBQW9CLFNBQXBCO0FBQ0EsVUFBUyxTQUFULENBQW1CLE9BQW5CLEVBQTRCLEtBQTVCLEVBQW1DLEtBQW5DLEVBQTBDO0FBQ3hDLE9BQUk7QUFDRixhQUFRLElBQUksS0FBSixDQUFVLEtBQVYsRUFBaUIsS0FBakIsQ0FBUjtBQUNELElBRkQsQ0FFRSxPQUFPLEVBQVAsRUFBVztBQUNYLFlBQU8sS0FBUDtBQUNEO0FBQ0QsVUFBTyxNQUFNLElBQU4sQ0FBVyxPQUFYLENBQVA7QUFDRDs7QUFFRCxTQUFRLGFBQVIsR0FBd0IsYUFBeEI7QUFDQSxVQUFTLGFBQVQsQ0FBdUIsUUFBdkIsRUFBaUMsS0FBakMsRUFBd0MsS0FBeEMsRUFBK0M7QUFDN0MsVUFBTyxTQUFTLE1BQVQsQ0FBZ0IsVUFBUyxPQUFULEVBQWtCO0FBQ3ZDLFlBQU8sVUFBVSxPQUFWLEVBQW1CLEtBQW5CLEVBQTBCLEtBQTFCLENBQVA7QUFDRCxJQUZNLEVBRUosSUFGSSxDQUVDLFVBQVMsQ0FBVCxFQUFZLENBQVosRUFBZTtBQUNyQixZQUFPLFNBQVMsQ0FBVCxFQUFZLENBQVosRUFBZSxLQUFmLENBQVA7QUFDRCxJQUpNLEVBSUosQ0FKSSxLQUlFLElBSlQ7QUFLRDs7QUFFRCxTQUFRLGFBQVIsR0FBd0IsYUFBeEI7QUFDQSxVQUFTLGFBQVQsQ0FBdUIsUUFBdkIsRUFBaUMsS0FBakMsRUFBd0MsS0FBeEMsRUFBK0M7QUFDN0MsVUFBTyxTQUFTLE1BQVQsQ0FBZ0IsVUFBUyxPQUFULEVBQWtCO0FBQ3ZDLFlBQU8sVUFBVSxPQUFWLEVBQW1CLEtBQW5CLEVBQTBCLEtBQTFCLENBQVA7QUFDRCxJQUZNLEVBRUosSUFGSSxDQUVDLFVBQVMsQ0FBVCxFQUFZLENBQVosRUFBZTtBQUNyQixZQUFPLFFBQVEsQ0FBUixFQUFXLENBQVgsRUFBYyxLQUFkLENBQVA7QUFDRCxJQUpNLEVBSUosQ0FKSSxLQUlFLElBSlQ7QUFLRDs7QUFFRCxTQUFRLFVBQVIsR0FBcUIsVUFBckI7QUFDQSxVQUFTLFVBQVQsQ0FBb0IsS0FBcEIsRUFBMkIsS0FBM0IsRUFBa0M7QUFDaEMsT0FBSTtBQUNGO0FBQ0E7QUFDQSxZQUFPLElBQUksS0FBSixDQUFVLEtBQVYsRUFBaUIsS0FBakIsRUFBd0IsS0FBeEIsSUFBaUMsR0FBeEM7QUFDRCxJQUpELENBSUUsT0FBTyxFQUFQLEVBQVc7QUFDWCxZQUFPLElBQVA7QUFDRDtBQUNGOztBQUVEO0FBQ0EsU0FBUSxHQUFSLEdBQWMsR0FBZDtBQUNBLFVBQVMsR0FBVCxDQUFhLE9BQWIsRUFBc0IsS0FBdEIsRUFBNkIsS0FBN0IsRUFBb0M7QUFDbEMsVUFBTyxRQUFRLE9BQVIsRUFBaUIsS0FBakIsRUFBd0IsR0FBeEIsRUFBNkIsS0FBN0IsQ0FBUDtBQUNEOztBQUVEO0FBQ0EsU0FBUSxHQUFSLEdBQWMsR0FBZDtBQUNBLFVBQVMsR0FBVCxDQUFhLE9BQWIsRUFBc0IsS0FBdEIsRUFBNkIsS0FBN0IsRUFBb0M7QUFDbEMsVUFBTyxRQUFRLE9BQVIsRUFBaUIsS0FBakIsRUFBd0IsR0FBeEIsRUFBNkIsS0FBN0IsQ0FBUDtBQUNEOztBQUVELFNBQVEsT0FBUixHQUFrQixPQUFsQjtBQUNBLFVBQVMsT0FBVCxDQUFpQixPQUFqQixFQUEwQixLQUExQixFQUFpQyxJQUFqQyxFQUF1QyxLQUF2QyxFQUE4QztBQUM1QyxhQUFVLElBQUksTUFBSixDQUFXLE9BQVgsRUFBb0IsS0FBcEIsQ0FBVjtBQUNBLFdBQVEsSUFBSSxLQUFKLENBQVUsS0FBVixFQUFpQixLQUFqQixDQUFSOztBQUVBLE9BQUksSUFBSixFQUFVLEtBQVYsRUFBaUIsSUFBakIsRUFBdUIsSUFBdkIsRUFBNkIsS0FBN0I7QUFDQSxXQUFRLElBQVI7QUFDRSxVQUFLLEdBQUw7QUFDRSxjQUFPLEVBQVA7QUFDQSxlQUFRLEdBQVI7QUFDQSxjQUFPLEVBQVA7QUFDQSxjQUFPLEdBQVA7QUFDQSxlQUFRLElBQVI7QUFDQTtBQUNGLFVBQUssR0FBTDtBQUNFLGNBQU8sRUFBUDtBQUNBLGVBQVEsR0FBUjtBQUNBLGNBQU8sRUFBUDtBQUNBLGNBQU8sR0FBUDtBQUNBLGVBQVEsSUFBUjtBQUNBO0FBQ0Y7QUFDRSxhQUFNLElBQUksU0FBSixDQUFjLHVDQUFkLENBQU47QUFoQko7O0FBbUJBO0FBQ0EsT0FBSSxVQUFVLE9BQVYsRUFBbUIsS0FBbkIsRUFBMEIsS0FBMUIsQ0FBSixFQUFzQztBQUNwQyxZQUFPLEtBQVA7QUFDRDs7QUFFRDtBQUNBOztBQUVBLFFBQUssSUFBSSxJQUFJLENBQWIsRUFBZ0IsSUFBSSxNQUFNLEdBQU4sQ0FBVSxNQUE5QixFQUFzQyxFQUFFLENBQXhDLEVBQTJDO0FBQ3pDLFNBQUksY0FBYyxNQUFNLEdBQU4sQ0FBVSxDQUFWLENBQWxCOztBQUVBLFNBQUksT0FBTyxJQUFYO0FBQ0EsU0FBSSxNQUFNLElBQVY7O0FBRUEsaUJBQVksT0FBWixDQUFvQixVQUFTLFVBQVQsRUFBcUI7QUFDdkMsV0FBSSxXQUFXLE1BQVgsS0FBc0IsR0FBMUIsRUFBK0I7QUFDN0Isc0JBQWEsSUFBSSxVQUFKLENBQWUsU0FBZixDQUFiO0FBQ0Q7QUFDRCxjQUFPLFFBQVEsVUFBZjtBQUNBLGFBQU0sT0FBTyxVQUFiO0FBQ0EsV0FBSSxLQUFLLFdBQVcsTUFBaEIsRUFBd0IsS0FBSyxNQUE3QixFQUFxQyxLQUFyQyxDQUFKLEVBQWlEO0FBQy9DLGdCQUFPLFVBQVA7QUFDRCxRQUZELE1BRU8sSUFBSSxLQUFLLFdBQVcsTUFBaEIsRUFBd0IsSUFBSSxNQUE1QixFQUFvQyxLQUFwQyxDQUFKLEVBQWdEO0FBQ3JELGVBQU0sVUFBTjtBQUNEO0FBQ0YsTUFYRDs7QUFhQTtBQUNBO0FBQ0EsU0FBSSxLQUFLLFFBQUwsS0FBa0IsSUFBbEIsSUFBMEIsS0FBSyxRQUFMLEtBQWtCLEtBQWhELEVBQXVEO0FBQ3JELGNBQU8sS0FBUDtBQUNEOztBQUVEO0FBQ0E7QUFDQSxTQUFJLENBQUMsQ0FBQyxJQUFJLFFBQUwsSUFBaUIsSUFBSSxRQUFKLEtBQWlCLElBQW5DLEtBQ0EsTUFBTSxPQUFOLEVBQWUsSUFBSSxNQUFuQixDQURKLEVBQ2dDO0FBQzlCLGNBQU8sS0FBUDtBQUNELE1BSEQsTUFHTyxJQUFJLElBQUksUUFBSixLQUFpQixLQUFqQixJQUEwQixLQUFLLE9BQUwsRUFBYyxJQUFJLE1BQWxCLENBQTlCLEVBQXlEO0FBQzlELGNBQU8sS0FBUDtBQUNEO0FBQ0Y7QUFDRCxVQUFPLElBQVA7QUFDRDs7QUFFRCxTQUFRLFVBQVIsR0FBcUIsVUFBckI7QUFDQSxVQUFTLFVBQVQsQ0FBb0IsT0FBcEIsRUFBNkIsS0FBN0IsRUFBb0M7QUFDbEMsT0FBSSxTQUFTLE1BQU0sT0FBTixFQUFlLEtBQWYsQ0FBYjtBQUNBLFVBQVEsVUFBVSxPQUFPLFVBQVAsQ0FBa0IsTUFBN0IsR0FBdUMsT0FBTyxVQUE5QyxHQUEyRCxJQUFsRTtBQUNELEU7Ozs7Ozs7OztBQ2xyQ0Q7O0FBRUEsS0FBSSxVQUFVLE9BQU8sT0FBUCxHQUFpQixFQUEvQjs7QUFFQTtBQUNBO0FBQ0E7QUFDQTs7QUFFQSxLQUFJLGdCQUFKO0FBQ0EsS0FBSSxrQkFBSjs7QUFFQyxjQUFZO0FBQ1gsU0FBSTtBQUNGLDRCQUFtQixVQUFuQjtBQUNELE1BRkQsQ0FFRSxPQUFPLENBQVAsRUFBVTtBQUNWLDRCQUFtQiw0QkFBWTtBQUM3QixtQkFBTSxJQUFJLEtBQUosQ0FBVSwyQkFBVixDQUFOO0FBQ0QsVUFGRDtBQUdEO0FBQ0QsU0FBSTtBQUNGLDhCQUFxQixZQUFyQjtBQUNELE1BRkQsQ0FFRSxPQUFPLENBQVAsRUFBVTtBQUNWLDhCQUFxQiw4QkFBWTtBQUMvQixtQkFBTSxJQUFJLEtBQUosQ0FBVSw2QkFBVixDQUFOO0FBQ0QsVUFGRDtBQUdEO0FBQ0YsRUFmQSxHQUFEO0FBZ0JBLEtBQUksUUFBUSxFQUFaO0FBQ0EsS0FBSSxXQUFXLEtBQWY7QUFDQSxLQUFJLFlBQUo7QUFDQSxLQUFJLGFBQWEsQ0FBQyxDQUFsQjs7QUFFQSxVQUFTLGVBQVQsR0FBMkI7QUFDdkIsU0FBSSxDQUFDLFFBQUQsSUFBYSxDQUFDLFlBQWxCLEVBQWdDO0FBQzVCO0FBQ0g7QUFDRCxnQkFBVyxLQUFYO0FBQ0EsU0FBSSxhQUFhLE1BQWpCLEVBQXlCO0FBQ3JCLGlCQUFRLGFBQWEsTUFBYixDQUFvQixLQUFwQixDQUFSO0FBQ0gsTUFGRCxNQUVPO0FBQ0gsc0JBQWEsQ0FBQyxDQUFkO0FBQ0g7QUFDRCxTQUFJLE1BQU0sTUFBVixFQUFrQjtBQUNkO0FBQ0g7QUFDSjs7QUFFRCxVQUFTLFVBQVQsR0FBc0I7QUFDbEIsU0FBSSxRQUFKLEVBQWM7QUFDVjtBQUNIO0FBQ0QsU0FBSSxVQUFVLGlCQUFpQixlQUFqQixDQUFkO0FBQ0EsZ0JBQVcsSUFBWDs7QUFFQSxTQUFJLE1BQU0sTUFBTSxNQUFoQjtBQUNBLFlBQU0sR0FBTixFQUFXO0FBQ1Asd0JBQWUsS0FBZjtBQUNBLGlCQUFRLEVBQVI7QUFDQSxnQkFBTyxFQUFFLFVBQUYsR0FBZSxHQUF0QixFQUEyQjtBQUN2QixpQkFBSSxZQUFKLEVBQWtCO0FBQ2QsOEJBQWEsVUFBYixFQUF5QixHQUF6QjtBQUNIO0FBQ0o7QUFDRCxzQkFBYSxDQUFDLENBQWQ7QUFDQSxlQUFNLE1BQU0sTUFBWjtBQUNIO0FBQ0Qsb0JBQWUsSUFBZjtBQUNBLGdCQUFXLEtBQVg7QUFDQSx3QkFBbUIsT0FBbkI7QUFDSDs7QUFFRCxTQUFRLFFBQVIsR0FBbUIsVUFBVSxHQUFWLEVBQWU7QUFDOUIsU0FBSSxPQUFPLElBQUksS0FBSixDQUFVLFVBQVUsTUFBVixHQUFtQixDQUE3QixDQUFYO0FBQ0EsU0FBSSxVQUFVLE1BQVYsR0FBbUIsQ0FBdkIsRUFBMEI7QUFDdEIsY0FBSyxJQUFJLElBQUksQ0FBYixFQUFnQixJQUFJLFVBQVUsTUFBOUIsRUFBc0MsR0FBdEMsRUFBMkM7QUFDdkMsa0JBQUssSUFBSSxDQUFULElBQWMsVUFBVSxDQUFWLENBQWQ7QUFDSDtBQUNKO0FBQ0QsV0FBTSxJQUFOLENBQVcsSUFBSSxJQUFKLENBQVMsR0FBVCxFQUFjLElBQWQsQ0FBWDtBQUNBLFNBQUksTUFBTSxNQUFOLEtBQWlCLENBQWpCLElBQXNCLENBQUMsUUFBM0IsRUFBcUM7QUFDakMsMEJBQWlCLFVBQWpCLEVBQTZCLENBQTdCO0FBQ0g7QUFDSixFQVhEOztBQWFBO0FBQ0EsVUFBUyxJQUFULENBQWMsR0FBZCxFQUFtQixLQUFuQixFQUEwQjtBQUN0QixVQUFLLEdBQUwsR0FBVyxHQUFYO0FBQ0EsVUFBSyxLQUFMLEdBQWEsS0FBYjtBQUNIO0FBQ0QsTUFBSyxTQUFMLENBQWUsR0FBZixHQUFxQixZQUFZO0FBQzdCLFVBQUssR0FBTCxDQUFTLEtBQVQsQ0FBZSxJQUFmLEVBQXFCLEtBQUssS0FBMUI7QUFDSCxFQUZEO0FBR0EsU0FBUSxLQUFSLEdBQWdCLFNBQWhCO0FBQ0EsU0FBUSxPQUFSLEdBQWtCLElBQWxCO0FBQ0EsU0FBUSxHQUFSLEdBQWMsRUFBZDtBQUNBLFNBQVEsSUFBUixHQUFlLEVBQWY7QUFDQSxTQUFRLE9BQVIsR0FBa0IsRUFBbEIsQyxDQUFzQjtBQUN0QixTQUFRLFFBQVIsR0FBbUIsRUFBbkI7O0FBRUEsVUFBUyxJQUFULEdBQWdCLENBQUU7O0FBRWxCLFNBQVEsRUFBUixHQUFhLElBQWI7QUFDQSxTQUFRLFdBQVIsR0FBc0IsSUFBdEI7QUFDQSxTQUFRLElBQVIsR0FBZSxJQUFmO0FBQ0EsU0FBUSxHQUFSLEdBQWMsSUFBZDtBQUNBLFNBQVEsY0FBUixHQUF5QixJQUF6QjtBQUNBLFNBQVEsa0JBQVIsR0FBNkIsSUFBN0I7QUFDQSxTQUFRLElBQVIsR0FBZSxJQUFmOztBQUVBLFNBQVEsT0FBUixHQUFrQixVQUFVLElBQVYsRUFBZ0I7QUFDOUIsV0FBTSxJQUFJLEtBQUosQ0FBVSxrQ0FBVixDQUFOO0FBQ0gsRUFGRDs7QUFJQSxTQUFRLEdBQVIsR0FBYyxZQUFZO0FBQUUsWUFBTyxHQUFQO0FBQVksRUFBeEM7QUFDQSxTQUFRLEtBQVIsR0FBZ0IsVUFBVSxHQUFWLEVBQWU7QUFDM0IsV0FBTSxJQUFJLEtBQUosQ0FBVSxnQ0FBVixDQUFOO0FBQ0gsRUFGRDtBQUdBLFNBQVEsS0FBUixHQUFnQixZQUFXO0FBQUUsWUFBTyxDQUFQO0FBQVcsRUFBeEMsQzs7Ozs7Ozs7Ozs7bUJDM0Z3QixFOztBQXRCeEI7O0FBQ0E7O0FBR0E7O0FBR0E7O0FBS0E7Ozs7Ozs7Ozs7QUFqQkE7Ozs7O0FBMkJlLFVBQVMsRUFBVCxDQUNiLElBRGEsRUFFYixPQUZhLEVBR2IsUUFIYSxFQUliLFFBSmEsRUFLYixVQUxhLEVBTWIsY0FOYSxFQU9iO0FBQ0EsUUFBSyxPQUFMLEdBQWUsU0FBUyxXQUFULEdBQXVCLFNBQVMsV0FBaEMsR0FBOEMsUUFBN0Q7QUFDQSxRQUFLLElBQUwsR0FBWSxTQUFTLElBQXJCO0FBQ0EsWUFBUyxZQUFULElBQXlCLFNBQVMsWUFBVCxDQUFzQixJQUF0QixDQUEyQixJQUEzQixDQUF6Qjs7QUFFQSxPQUFJLENBQUMsT0FBTCxFQUFjO0FBQ1osZUFBVSxLQUFLLElBQUwsQ0FBVSxrQkFBVixDQUE2QixJQUE3QixLQUFzQyxFQUFoRDtBQUNEO0FBQ0QsT0FBTSxPQUFPLFFBQVEsSUFBUixJQUFnQixFQUE3Qjs7QUFFQSxRQUFLLFFBQUwsR0FBZ0IsT0FBaEI7QUFDQSxRQUFLLFFBQUwsR0FBZ0IsUUFBUSxPQUFSLElBQW1CLEVBQW5DO0FBQ0EsUUFBSyxTQUFMLEdBQWlCLFFBQVEsUUFBUixJQUFvQixFQUFyQztBQUNBLFFBQUssSUFBTCxHQUFZLFFBQVEsS0FBUixJQUFpQixFQUE3QjtBQUNBLFFBQUssSUFBTCxHQUFZLEVBQVo7QUFDQSxRQUFLLFNBQUwsR0FBaUIsRUFBakI7QUFDQSxRQUFLLFlBQUwsR0FBb0IsRUFBcEI7QUFDQSxRQUFLLEtBQUwsR0FBYSxJQUFiOztBQUVBO0FBQ0EsMkJBQVcsSUFBWCxFQUFpQixjQUFqQjs7QUFFQSxXQUFRLEtBQVIsNENBQXVELEtBQUssS0FBNUQ7QUFDQSxRQUFLLEtBQUwsQ0FBVyxXQUFYO0FBQ0EsUUFBSyxPQUFMLEdBQWUsSUFBZjs7QUFFQTtBQUNBO0FBQ0EsUUFBSyxLQUFMLEdBQWEsT0FBTyxJQUFQLEtBQWdCLFVBQWhCLEdBQTZCLE1BQTdCLEdBQXNDLElBQW5EO0FBQ0EsT0FBSSxVQUFKLEVBQWdCO0FBQ2QsdUJBQU8sS0FBSyxLQUFaLEVBQW1CLFVBQW5CO0FBQ0Q7QUFDRCx5QkFBVSxJQUFWOztBQUVBLFdBQVEsS0FBUiwrQ0FBMEQsS0FBSyxLQUEvRDtBQUNBLFFBQUssS0FBTCxDQUFXLGNBQVg7QUFDQSxRQUFLLFFBQUwsR0FBZ0IsSUFBaEI7O0FBRUE7QUFDQSxPQUFJLFFBQVEsT0FBUixJQUFtQixRQUFRLE9BQVIsQ0FBZ0IsS0FBdkMsRUFBOEM7QUFDNUMsYUFBUSxJQUFSLENBQWEsNENBQ1gsc0NBREY7QUFFQSxhQUFRLE9BQVIsQ0FBZ0IsS0FBaEIsQ0FBc0IsSUFBdEIsQ0FBMkIsSUFBM0I7QUFDRDs7QUFFRDtBQUNBLFFBQUssU0FBTCxHQUFpQixZQUFZLEtBQUssSUFBTCxDQUFVLEdBQVYsQ0FBYyxlQUEzQztBQUNBLHdCQUFNLElBQU47QUFDRDs7QUFFRCwwQkFBWSxHQUFHLFNBQWYsRTs7Ozs7Ozs7Ozs7U0N0RWdCLFMsR0FBQSxTO1NBT0EsUSxHQUFBLFE7U0FtQkEsWSxHQUFBLFk7U0EwQ0EsVyxHQUFBLFc7O0FBaEZoQjs7OztBQUNBOzs7O0FBQ0E7O0FBS0E7Ozs7QUFUQTs7QUFjTyxVQUFTLFNBQVQsQ0FBb0IsRUFBcEIsRUFBd0I7QUFDN0IsTUFBRyxTQUFILEdBQWUsRUFBZjtBQUNBLFlBQVMsRUFBVDtBQUNBLGdCQUFhLEVBQWI7QUFDQSxlQUFZLEVBQVo7QUFDRDs7QUFFTSxVQUFTLFFBQVQsQ0FBbUIsRUFBbkIsRUFBdUI7QUFDNUIsT0FBSSxPQUFPLEdBQUcsS0FBZDs7QUFFQSxPQUFJLENBQUMseUJBQWMsSUFBZCxDQUFMLEVBQTBCO0FBQ3hCLFlBQU8sRUFBUDtBQUNEO0FBQ0Q7QUFDQSxPQUFNLE9BQU8sT0FBTyxJQUFQLENBQVksSUFBWixDQUFiO0FBQ0EsT0FBSSxJQUFJLEtBQUssTUFBYjtBQUNBLFVBQU8sR0FBUCxFQUFZO0FBQ1YsMEJBQU0sRUFBTixFQUFVLEtBQUssQ0FBTCxDQUFWO0FBQ0Q7QUFDRDtBQUNBLDBCQUFRLElBQVIsRUFBYyxFQUFkO0FBQ0Q7O0FBRUQsVUFBUyxJQUFULEdBQWlCLENBQ2hCOztBQUVNLFVBQVMsWUFBVCxDQUF1QixFQUF2QixFQUEyQjtBQUNoQyxPQUFNLFdBQVcsR0FBRyxTQUFwQjtBQUNBLE9BQUksUUFBSixFQUFjO0FBQ1osVUFBSyxJQUFJLEdBQVQsSUFBZ0IsUUFBaEIsRUFBMEI7QUFDeEIsV0FBTSxVQUFVLFNBQVMsR0FBVCxDQUFoQjtBQUNBLFdBQU0sTUFBTTtBQUNWLHFCQUFZLElBREY7QUFFVix1QkFBYztBQUZKLFFBQVo7QUFJQSxXQUFJLE9BQU8sT0FBUCxLQUFtQixVQUF2QixFQUFtQztBQUNqQyxhQUFJLEdBQUosR0FBVSxtQkFBbUIsT0FBbkIsRUFBNEIsRUFBNUIsQ0FBVjtBQUNBLGFBQUksR0FBSixHQUFVLElBQVY7QUFDRCxRQUhELE1BR087QUFDTCxhQUFJLEdBQUosR0FBVSxRQUFRLEdBQVIsR0FDTixRQUFRLEtBQVIsS0FBa0IsS0FBbEIsR0FDRSxtQkFBbUIsUUFBUSxHQUEzQixFQUFnQyxFQUFoQyxDQURGLEdBRUUsZ0JBQUssUUFBUSxHQUFiLEVBQWtCLEVBQWxCLENBSEksR0FJTixJQUpKO0FBS0EsYUFBSSxHQUFKLEdBQVUsUUFBUSxHQUFSLEdBQ04sZ0JBQUssUUFBUSxHQUFiLEVBQWtCLEVBQWxCLENBRE0sR0FFTixJQUZKO0FBR0Q7QUFDRCxjQUFPLGNBQVAsQ0FBc0IsRUFBdEIsRUFBMEIsR0FBMUIsRUFBK0IsR0FBL0I7QUFDRDtBQUNGO0FBQ0Y7O0FBRUQsVUFBUyxrQkFBVCxDQUE2QixNQUE3QixFQUFxQyxLQUFyQyxFQUE0QztBQUMxQyxPQUFNLFVBQVUsc0JBQVksS0FBWixFQUFtQixNQUFuQixFQUEyQixJQUEzQixFQUFpQztBQUMvQyxXQUFNO0FBRHlDLElBQWpDLENBQWhCO0FBR0EsVUFBTyxTQUFTLGNBQVQsR0FBMkI7QUFDaEMsU0FBSSxRQUFRLEtBQVosRUFBbUI7QUFDakIsZUFBUSxRQUFSO0FBQ0Q7QUFDRCxTQUFJLGNBQUksTUFBUixFQUFnQjtBQUNkLGVBQVEsTUFBUjtBQUNEO0FBQ0QsWUFBTyxRQUFRLEtBQWY7QUFDRCxJQVJEO0FBU0Q7O0FBRU0sVUFBUyxXQUFULENBQXNCLEVBQXRCLEVBQTBCO0FBQy9CLE9BQU0sVUFBVSxHQUFHLFFBQW5CO0FBQ0EsT0FBSSxPQUFKLEVBQWE7QUFDWCxVQUFLLElBQUksR0FBVCxJQUFnQixPQUFoQixFQUF5QjtBQUN2QixVQUFHLEdBQUgsSUFBVSxRQUFRLEdBQVIsQ0FBVjtBQUNEO0FBQ0Y7QUFDRixFOzs7Ozs7Ozs7OzttQkNyRHVCLE87O0FBbEN4Qjs7OztBQUVBOzs7O0FBSkE7O0FBYUEsS0FBSSxNQUFNLENBQVY7QUFWQTs7QUFXQSxLQUFJLG1CQUFKOztBQUVBOzs7Ozs7Ozs7Ozs7Ozs7Ozs7OztBQW9CZSxVQUFTLE9BQVQsQ0FBa0IsRUFBbEIsRUFBc0IsT0FBdEIsRUFBK0IsRUFBL0IsRUFBbUMsT0FBbkMsRUFBNEM7QUFDekQ7QUFDQSxPQUFJLE9BQUosRUFBYTtBQUNYLHVCQUFPLElBQVAsRUFBYSxPQUFiO0FBQ0Q7QUFDRCxPQUFNLE9BQU8sT0FBTyxPQUFQLEtBQW1CLFVBQWhDO0FBQ0EsUUFBSyxFQUFMLEdBQVUsRUFBVjtBQUNBLE1BQUcsU0FBSCxDQUFhLElBQWIsQ0FBa0IsSUFBbEI7QUFDQSxRQUFLLFVBQUwsR0FBa0IsT0FBbEI7QUFDQSxRQUFLLEVBQUwsR0FBVSxFQUFWO0FBQ0EsUUFBSyxFQUFMLEdBQVUsRUFBRSxHQUFaLENBVnlELENBVXpDO0FBQ2hCLFFBQUssTUFBTCxHQUFjLElBQWQ7QUFDQSxRQUFLLEtBQUwsR0FBYSxLQUFLLElBQWxCLENBWnlELENBWWxDO0FBQ3ZCLFFBQUssSUFBTCxHQUFZLEVBQVo7QUFDQSxRQUFLLE9BQUwsR0FBZSxFQUFmO0FBQ0EsUUFBSyxNQUFMLEdBQWMsZ0JBQWQ7QUFDQSxRQUFLLFNBQUwsR0FBaUIsZ0JBQWpCO0FBQ0E7QUFDQSxPQUFJLElBQUosRUFBVTtBQUNSLFVBQUssTUFBTCxHQUFjLE9BQWQ7QUFDRCxJQUZELE1BRU87QUFDTCxVQUFLLE1BQUwsR0FBYyxxQkFBVSxPQUFWLENBQWQ7QUFDQSxTQUFJLENBQUMsS0FBSyxNQUFWLEVBQWtCO0FBQ2hCLFlBQUssTUFBTCxHQUFjLFlBQVksQ0FBRSxDQUE1QjtBQUNBLGVBQVEsR0FBUixDQUFZLFFBQVosS0FBeUIsWUFBekIsSUFBeUMsZ0JBQ3ZDLDJCQUEyQixPQUEzQixHQUNBLG1EQURBLEdBRUEsMkNBSHVDLEVBSXZDLEVBSnVDLENBQXpDO0FBTUQ7QUFDRjtBQUNELFFBQUssS0FBTCxHQUFhLEtBQUssSUFBTCxHQUNULFNBRFMsR0FFVCxLQUFLLEdBQUwsRUFGSjtBQUdBO0FBQ0E7QUFDQSxRQUFLLE1BQUwsR0FBYyxLQUFLLE9BQUwsR0FBZSxLQUE3QjtBQUNEOztBQUVEOzs7O0FBSUEsU0FBUSxTQUFSLENBQWtCLEdBQWxCLEdBQXdCLFlBQVk7QUFDbEMsUUFBSyxTQUFMO0FBQ0EsT0FBTSxRQUFRLEtBQUssTUFBTCxDQUFZLElBQVosQ0FBaUIsS0FBSyxFQUF0QixFQUEwQixLQUFLLEVBQS9CLENBQWQ7QUFDQTtBQUNBO0FBQ0EsT0FBSSxLQUFLLElBQVQsRUFBZTtBQUNiLGNBQVMsS0FBVDtBQUNEO0FBQ0QsUUFBSyxRQUFMO0FBQ0EsVUFBTyxLQUFQO0FBQ0QsRUFWRDs7QUFZQTs7OztBQUlBLFNBQVEsU0FBUixDQUFrQixTQUFsQixHQUE4QixZQUFZO0FBQ3hDLGdCQUFhLGNBQUksTUFBakI7QUFDQSxpQkFBSSxNQUFKLEdBQWEsSUFBYjtBQUNELEVBSEQ7O0FBS0E7Ozs7OztBQU1BLFNBQVEsU0FBUixDQUFrQixNQUFsQixHQUEyQixVQUFVLEdBQVYsRUFBZTtBQUN4QyxPQUFNLEtBQUssSUFBSSxFQUFmO0FBQ0EsT0FBSSxDQUFDLEtBQUssU0FBTCxDQUFlLEdBQWYsQ0FBbUIsRUFBbkIsQ0FBTCxFQUE2QjtBQUMzQixVQUFLLFNBQUwsQ0FBZSxHQUFmLENBQW1CLEVBQW5CO0FBQ0EsVUFBSyxPQUFMLENBQWEsSUFBYixDQUFrQixHQUFsQjtBQUNBLFNBQUksQ0FBQyxLQUFLLE1BQUwsQ0FBWSxHQUFaLENBQWdCLEVBQWhCLENBQUwsRUFBMEI7QUFDeEIsV0FBSSxNQUFKLENBQVcsSUFBWDtBQUNEO0FBQ0Y7QUFDRixFQVREOztBQVdBOzs7O0FBSUEsU0FBUSxTQUFSLENBQWtCLFFBQWxCLEdBQTZCLFlBQVk7QUFDdkMsaUJBQUksTUFBSixHQUFhLFVBQWI7QUFDQSxPQUFJLElBQUksS0FBSyxJQUFMLENBQVUsTUFBbEI7QUFDQSxVQUFPLEdBQVAsRUFBWTtBQUNWLFNBQU0sTUFBTSxLQUFLLElBQUwsQ0FBVSxDQUFWLENBQVo7QUFDQSxTQUFJLENBQUMsS0FBSyxTQUFMLENBQWUsR0FBZixDQUFtQixJQUFJLEVBQXZCLENBQUwsRUFBaUM7QUFDL0IsV0FBSSxTQUFKLENBQWMsSUFBZDtBQUNEO0FBQ0Y7QUFDRCxPQUFJLE1BQU0sS0FBSyxNQUFmO0FBQ0EsUUFBSyxNQUFMLEdBQWMsS0FBSyxTQUFuQjtBQUNBLFFBQUssU0FBTCxHQUFpQixHQUFqQjtBQUNBLFFBQUssU0FBTCxDQUFlLEtBQWY7QUFDQSxTQUFNLEtBQUssSUFBWDtBQUNBLFFBQUssSUFBTCxHQUFZLEtBQUssT0FBakI7QUFDQSxRQUFLLE9BQUwsR0FBZSxHQUFmO0FBQ0EsUUFBSyxPQUFMLENBQWEsTUFBYixHQUFzQixDQUF0QjtBQUNELEVBakJEOztBQW1CQTs7Ozs7OztBQU9BLFNBQVEsU0FBUixDQUFrQixNQUFsQixHQUEyQixVQUFVLE9BQVYsRUFBbUI7QUFDNUMsT0FBSSxLQUFLLElBQVQsRUFBZTtBQUNiLFVBQUssS0FBTCxHQUFhLElBQWI7QUFDRCxJQUZELE1BRU87QUFDTCxVQUFLLEdBQUw7QUFDRDtBQUNEO0FBQ0E7QUFDQTtBQUNBO0FBQ0E7QUFDQTtBQUNBO0FBQ0E7QUFDQTtBQUNBO0FBQ0E7QUFDQTtBQUNBO0FBQ0QsRUFuQkQ7O0FBcUJBOzs7OztBQUtBLFNBQVEsU0FBUixDQUFrQixHQUFsQixHQUF3QixZQUFZO0FBQ2xDLE9BQUksS0FBSyxNQUFULEVBQWlCO0FBQ2YsU0FBTSxRQUFRLEtBQUssR0FBTCxFQUFkO0FBQ0EsU0FDRSxVQUFVLEtBQUssS0FBZjtBQUNBO0FBQ0E7QUFDQTtBQUNBO0FBQ0MsTUFBQyxvQkFBUyxLQUFULEtBQW1CLEtBQUssSUFBekIsS0FBa0MsQ0FBQyxLQUFLLE9BTjNDLEVBT0U7QUFDQTtBQUNBLFdBQU0sV0FBVyxLQUFLLEtBQXRCO0FBQ0EsWUFBSyxLQUFMLEdBQWEsS0FBYjtBQUNBLFlBQUssRUFBTCxDQUFRLElBQVIsQ0FBYSxLQUFLLEVBQWxCLEVBQXNCLEtBQXRCLEVBQTZCLFFBQTdCO0FBQ0Q7QUFDRCxVQUFLLE1BQUwsR0FBYyxLQUFLLE9BQUwsR0FBZSxLQUE3QjtBQUNEO0FBQ0YsRUFsQkQ7O0FBb0JBOzs7OztBQUtBLFNBQVEsU0FBUixDQUFrQixRQUFsQixHQUE2QixZQUFZO0FBQ3ZDO0FBQ0E7QUFDQSxPQUFNLFVBQVUsY0FBSSxNQUFwQjtBQUNBLFFBQUssS0FBTCxHQUFhLEtBQUssR0FBTCxFQUFiO0FBQ0EsUUFBSyxLQUFMLEdBQWEsS0FBYjtBQUNBLGlCQUFJLE1BQUosR0FBYSxPQUFiO0FBQ0QsRUFQRDs7QUFTQTs7OztBQUlBLFNBQVEsU0FBUixDQUFrQixNQUFsQixHQUEyQixZQUFZO0FBQ3JDLE9BQUksSUFBSSxLQUFLLElBQUwsQ0FBVSxNQUFsQjtBQUNBLFVBQU8sR0FBUCxFQUFZO0FBQ1YsVUFBSyxJQUFMLENBQVUsQ0FBVixFQUFhLE1BQWI7QUFDRDtBQUNGLEVBTEQ7O0FBT0E7Ozs7QUFJQSxTQUFRLFNBQVIsQ0FBa0IsUUFBbEIsR0FBNkIsWUFBWTtBQUN2QyxPQUFJLEtBQUssTUFBVCxFQUFpQjtBQUNmO0FBQ0E7QUFDQTtBQUNBO0FBQ0EsU0FBSSxDQUFDLEtBQUssRUFBTCxDQUFRLGlCQUFULElBQThCLENBQUMsS0FBSyxFQUFMLENBQVEsYUFBM0MsRUFBMEQ7QUFDeEQseUJBQU8sS0FBSyxFQUFMLENBQVEsU0FBZixFQUEwQixJQUExQjtBQUNEO0FBQ0QsU0FBSSxJQUFJLEtBQUssSUFBTCxDQUFVLE1BQWxCO0FBQ0EsWUFBTyxHQUFQLEVBQVk7QUFDVixZQUFLLElBQUwsQ0FBVSxDQUFWLEVBQWEsU0FBYixDQUF1QixJQUF2QjtBQUNEO0FBQ0QsVUFBSyxNQUFMLEdBQWMsS0FBZDtBQUNBLFVBQUssRUFBTCxHQUFVLEtBQUssRUFBTCxHQUFVLEtBQUssS0FBTCxHQUFhLElBQWpDO0FBQ0Q7QUFDRixFQWhCRDs7QUFrQkE7Ozs7Ozs7OztBQVNBLEtBQU0sY0FBYyxnQkFBcEI7QUFDQSxVQUFTLFFBQVQsQ0FBbUIsR0FBbkIsRUFBd0IsSUFBeEIsRUFBOEI7QUFDNUIsT0FBSSxVQUFKO0FBQUEsT0FBTyxhQUFQO0FBQUEsT0FBYSxZQUFiO0FBQUEsT0FBa0IsWUFBbEI7QUFDQSxPQUFJLENBQUMsSUFBTCxFQUFXO0FBQ1QsWUFBTyxXQUFQO0FBQ0EsVUFBSyxLQUFMO0FBQ0Q7QUFDRCxTQUFNLE1BQU0sT0FBTixDQUFjLEdBQWQsQ0FBTjtBQUNBLFNBQU0sb0JBQVMsR0FBVCxDQUFOO0FBQ0EsT0FBSSxPQUFPLEdBQVgsRUFBZ0I7QUFDZCxTQUFJLElBQUksTUFBUixFQUFnQjtBQUNkLFdBQU0sUUFBUSxJQUFJLE1BQUosQ0FBVyxHQUFYLENBQWUsRUFBN0I7QUFDQSxXQUFJLEtBQUssR0FBTCxDQUFTLEtBQVQsQ0FBSixFQUFxQjtBQUNuQjtBQUNELFFBRkQsTUFFTztBQUNMLGNBQUssR0FBTCxDQUFTLEtBQVQ7QUFDRDtBQUNGO0FBQ0QsU0FBSSxHQUFKLEVBQVM7QUFDUCxXQUFJLElBQUksTUFBUjtBQUNBLGNBQU8sR0FBUDtBQUFZLGtCQUFTLElBQUksQ0FBSixDQUFULEVBQWlCLElBQWpCO0FBQVo7QUFDRCxNQUhELE1BR08sSUFBSSxHQUFKLEVBQVM7QUFDZCxjQUFPLE9BQU8sSUFBUCxDQUFZLEdBQVosQ0FBUDtBQUNBLFdBQUksS0FBSyxNQUFUO0FBQ0EsY0FBTyxHQUFQO0FBQVksa0JBQVMsSUFBSSxLQUFLLENBQUwsQ0FBSixDQUFULEVBQXVCLElBQXZCO0FBQVo7QUFDRDtBQUNGO0FBQ0YsRTs7Ozs7Ozs7Ozs7O21CQ3hRdUIsRzs7QUFYeEI7O0FBRUEsS0FBSSxNQUFNLENBQVY7O0FBRUE7Ozs7Ozs7QUFOQTs7QUFhZSxVQUFTLEdBQVQsR0FBZ0I7QUFDN0IsUUFBSyxFQUFMLEdBQVUsS0FBVjtBQUNBLFFBQUssSUFBTCxHQUFZLEVBQVo7QUFDRDs7QUFFRDtBQUNBO0FBQ0E7QUFDQSxLQUFJLE1BQUosR0FBYSxJQUFiOztBQUVBOzs7Ozs7QUFNQSxLQUFJLFNBQUosQ0FBYyxNQUFkLEdBQXVCLFVBQVUsR0FBVixFQUFlO0FBQ3BDLFFBQUssSUFBTCxDQUFVLElBQVYsQ0FBZSxHQUFmO0FBQ0QsRUFGRDs7QUFJQTs7Ozs7O0FBTUEsS0FBSSxTQUFKLENBQWMsU0FBZCxHQUEwQixVQUFVLEdBQVYsRUFBZTtBQUN2QyxxQkFBTyxLQUFLLElBQVosRUFBa0IsR0FBbEI7QUFDRCxFQUZEOztBQUlBOzs7O0FBSUEsS0FBSSxTQUFKLENBQWMsTUFBZCxHQUF1QixZQUFZO0FBQ2pDLE9BQUksTUFBSixDQUFXLE1BQVgsQ0FBa0IsSUFBbEI7QUFDRCxFQUZEOztBQUlBOzs7O0FBSUEsS0FBSSxTQUFKLENBQWMsTUFBZCxHQUF1QixZQUFZO0FBQ2pDO0FBQ0EsT0FBTSxPQUFPLEtBQUssSUFBTCxDQUFVLEtBQVYsRUFBYjtBQUNBLFFBQUssSUFBSSxJQUFJLENBQVIsRUFBVyxJQUFJLEtBQUssTUFBekIsRUFBaUMsSUFBSSxDQUFyQyxFQUF3QyxHQUF4QyxFQUE2QztBQUMzQyxVQUFLLENBQUwsRUFBUSxNQUFSO0FBQ0Q7QUFDRixFQU5ELEM7Ozs7Ozs7Ozs7O1NDN0JnQixRLEdBQUEsUTtTQXlIQSxPLEdBQUEsTztTQTRCQSxjLEdBQUEsYztTQTJEQSxHLEdBQUEsRztTQXFDQSxHLEdBQUEsRztTQTBCQSxLLEdBQUEsSztTQWVBLE8sR0FBQSxPOztBQXRUaEI7Ozs7QUFDQTs7QUFDQTs7OztBQVVBLEtBQU0sWUFBWSxPQUFPLG1CQUFQLHFCQUFsQjs7QUFFQTs7Ozs7Ozs7OztBQWhCQTs7QUEwQk8sVUFBUyxRQUFULENBQW1CLEtBQW5CLEVBQTBCO0FBQy9CLFFBQUssS0FBTCxHQUFhLEtBQWI7QUFDQSxRQUFLLEdBQUwsR0FBVyxtQkFBWDtBQUNBLGtCQUFJLEtBQUosRUFBVyxRQUFYLEVBQXFCLElBQXJCO0FBQ0EsT0FBSSxNQUFNLE9BQU4sQ0FBYyxLQUFkLENBQUosRUFBMEI7QUFDeEIsU0FBTSxVQUFVLGlCQUNaLFlBRFksR0FFWixXQUZKO0FBR0EsYUFBUSxLQUFSLHVCQUE2QixTQUE3QjtBQUNBLFVBQUssWUFBTCxDQUFrQixLQUFsQjtBQUNELElBTkQsTUFNTztBQUNMLFVBQUssSUFBTCxDQUFVLEtBQVY7QUFDRDtBQUNGOztBQUVEOztBQUVBOzs7Ozs7OztBQVFBLFVBQVMsU0FBVCxDQUFtQixJQUFuQixHQUEwQixVQUFVLEdBQVYsRUFBZTtBQUN2QyxRQUFLLElBQUksR0FBVCxJQUFnQixHQUFoQixFQUFxQjtBQUNuQixVQUFLLE9BQUwsQ0FBYSxHQUFiLEVBQWtCLElBQUksR0FBSixDQUFsQjtBQUNEO0FBQ0YsRUFKRDs7QUFNQTs7Ozs7O0FBTUEsVUFBUyxTQUFULENBQW1CLFlBQW5CLEdBQWtDLFVBQVUsS0FBVixFQUFpQjtBQUNqRCxRQUFLLElBQUksSUFBSSxDQUFSLEVBQVcsSUFBSSxNQUFNLE1BQTFCLEVBQWtDLElBQUksQ0FBdEMsRUFBeUMsR0FBekMsRUFBOEM7QUFDNUMsYUFBUSxNQUFNLENBQU4sQ0FBUjtBQUNEO0FBQ0YsRUFKRDs7QUFNQTs7Ozs7Ozs7QUFRQSxVQUFTLFNBQVQsQ0FBbUIsT0FBbkIsR0FBNkIsVUFBVSxHQUFWLEVBQWUsR0FBZixFQUFvQjtBQUMvQyxrQkFBZSxLQUFLLEtBQXBCLEVBQTJCLEdBQTNCLEVBQWdDLEdBQWhDO0FBQ0QsRUFGRDs7QUFJQTs7Ozs7Ozs7O0FBU0EsVUFBUyxTQUFULENBQW1CLEtBQW5CLEdBQTJCLFVBQVUsRUFBVixFQUFjO0FBQ3ZDLElBQUMsS0FBSyxHQUFMLEtBQWEsS0FBSyxHQUFMLEdBQVcsRUFBeEIsQ0FBRCxFQUE4QixJQUE5QixDQUFtQyxFQUFuQztBQUNELEVBRkQ7O0FBSUE7Ozs7Ozs7QUFPQSxVQUFTLFNBQVQsQ0FBbUIsUUFBbkIsR0FBOEIsVUFBVSxFQUFWLEVBQWM7QUFDMUMscUJBQU8sS0FBSyxHQUFaLEVBQWlCLEVBQWpCO0FBQ0QsRUFGRDs7QUFJQTs7QUFFQTs7Ozs7Ozs7QUFRQSxVQUFTLFlBQVQsQ0FBdUIsTUFBdkIsRUFBK0IsR0FBL0IsRUFBb0M7QUFDbEM7QUFDQSxVQUFPLFNBQVAsR0FBbUIsR0FBbkI7QUFDQTtBQUNEOztBQUVEOzs7Ozs7OztBQVFBLFVBQVMsV0FBVCxDQUFzQixNQUF0QixFQUE4QixHQUE5QixFQUFtQyxJQUFuQyxFQUF5QztBQUN2QyxRQUFLLElBQUksSUFBSSxDQUFSLEVBQVcsSUFBSSxLQUFLLE1BQXpCLEVBQWlDLElBQUksQ0FBckMsRUFBd0MsR0FBeEMsRUFBNkM7QUFDM0MsU0FBTSxNQUFNLEtBQUssQ0FBTCxDQUFaO0FBQ0Esb0JBQUksTUFBSixFQUFZLEdBQVosRUFBaUIsSUFBSSxHQUFKLENBQWpCO0FBQ0Q7QUFDRjs7QUFFRDs7Ozs7Ozs7Ozs7QUFXTyxVQUFTLE9BQVQsQ0FBa0IsS0FBbEIsRUFBeUIsRUFBekIsRUFBNkI7QUFDbEMsT0FBSSxDQUFDLG9CQUFTLEtBQVQsQ0FBTCxFQUFzQjtBQUNwQjtBQUNEO0FBQ0QsT0FBSSxXQUFKO0FBQ0EsT0FBSSxrQkFBTyxLQUFQLEVBQWMsUUFBZCxLQUEyQixNQUFNLE1BQU4sWUFBd0IsUUFBdkQsRUFBaUU7QUFDL0QsVUFBSyxNQUFNLE1BQVg7QUFDRCxJQUZELE1BRU8sSUFDTCxDQUFDLE1BQU0sT0FBTixDQUFjLEtBQWQsS0FBd0IseUJBQWMsS0FBZCxDQUF6QixLQUNBLE9BQU8sWUFBUCxDQUFvQixLQUFwQixDQURBLElBRUEsQ0FBQyxNQUFNLE1BSEYsRUFJTDtBQUNBLFVBQUssSUFBSSxRQUFKLENBQWEsS0FBYixDQUFMO0FBQ0Q7QUFDRCxPQUFJLE1BQU0sRUFBVixFQUFjO0FBQ1osUUFBRyxLQUFILENBQVMsRUFBVDtBQUNEO0FBQ0QsVUFBTyxFQUFQO0FBQ0Q7O0FBRUQ7Ozs7Ozs7O0FBUU8sVUFBUyxjQUFULENBQXlCLEdBQXpCLEVBQThCLEdBQTlCLEVBQW1DLEdBQW5DLEVBQXdDO0FBQzdDLE9BQU0sTUFBTSxtQkFBWjs7QUFFQSxPQUFNLFdBQVcsT0FBTyx3QkFBUCxDQUFnQyxHQUFoQyxFQUFxQyxHQUFyQyxDQUFqQjtBQUNBLE9BQUksWUFBWSxTQUFTLFlBQVQsS0FBMEIsS0FBMUMsRUFBaUQ7QUFDL0M7QUFDRDs7QUFFRDtBQUNBLE9BQU0sU0FBUyxZQUFZLFNBQVMsR0FBcEM7QUFDQSxPQUFNLFNBQVMsWUFBWSxTQUFTLEdBQXBDOztBQUVBLE9BQUksVUFBVSxRQUFRLEdBQVIsQ0FBZDtBQUNBLFVBQU8sY0FBUCxDQUFzQixHQUF0QixFQUEyQixHQUEzQixFQUFnQztBQUM5QixpQkFBWSxJQURrQjtBQUU5QixtQkFBYyxJQUZnQjtBQUc5QixVQUFLLFNBQVMsY0FBVCxHQUEyQjtBQUM5QixXQUFNLFFBQVEsU0FBUyxPQUFPLElBQVAsQ0FBWSxHQUFaLENBQVQsR0FBNEIsR0FBMUM7QUFDQSxXQUFJLGNBQUksTUFBUixFQUFnQjtBQUNkLGFBQUksTUFBSjtBQUNBLGFBQUksT0FBSixFQUFhO0FBQ1gsbUJBQVEsR0FBUixDQUFZLE1BQVo7QUFDRDtBQUNELGFBQUksTUFBTSxPQUFOLENBQWMsS0FBZCxDQUFKLEVBQTBCO0FBQ3hCLGdCQUFLLElBQUksQ0FBSixFQUFPLElBQUksQ0FBWCxFQUFjLElBQUksTUFBTSxNQUE3QixFQUFxQyxJQUFJLENBQXpDLEVBQTRDLEdBQTVDLEVBQWlEO0FBQy9DLGlCQUFJLE1BQU0sQ0FBTixDQUFKO0FBQ0Esa0JBQUssRUFBRSxNQUFQLElBQWlCLEVBQUUsTUFBRixDQUFTLEdBQVQsQ0FBYSxNQUFiLEVBQWpCO0FBQ0Q7QUFDRjtBQUNGO0FBQ0QsY0FBTyxLQUFQO0FBQ0QsTUFsQjZCO0FBbUI5QixVQUFLLFNBQVMsY0FBVCxDQUF5QixNQUF6QixFQUFpQztBQUNwQyxXQUFNLFFBQVEsU0FBUyxPQUFPLElBQVAsQ0FBWSxHQUFaLENBQVQsR0FBNEIsR0FBMUM7QUFDQSxXQUFJLFdBQVcsS0FBZixFQUFzQjtBQUNwQjtBQUNEO0FBQ0QsV0FBSSxNQUFKLEVBQVk7QUFDVixnQkFBTyxJQUFQLENBQVksR0FBWixFQUFpQixNQUFqQjtBQUNELFFBRkQsTUFFTztBQUNMLGVBQU0sTUFBTjtBQUNEO0FBQ0QsaUJBQVUsUUFBUSxNQUFSLENBQVY7QUFDQSxXQUFJLE1BQUo7QUFDRDtBQS9CNkIsSUFBaEM7QUFpQ0Q7O0FBRUQ7Ozs7Ozs7Ozs7O0FBV08sVUFBUyxHQUFULENBQWMsR0FBZCxFQUFtQixHQUFuQixFQUF3QixHQUF4QixFQUE2QjtBQUNsQyxPQUFJLE1BQU0sT0FBTixDQUFjLEdBQWQsQ0FBSixFQUF3QjtBQUN0QixZQUFPLElBQUksTUFBSixDQUFXLEdBQVgsRUFBZ0IsQ0FBaEIsRUFBbUIsR0FBbkIsQ0FBUDtBQUNEO0FBQ0QsT0FBSSxrQkFBTyxHQUFQLEVBQVksR0FBWixDQUFKLEVBQXNCO0FBQ3BCLFNBQUksR0FBSixJQUFXLEdBQVg7QUFDQTtBQUNEO0FBQ0QsT0FBSSxJQUFJLE1BQVIsRUFBZ0I7QUFDZCxTQUFJLElBQUksS0FBUixFQUFlLEdBQWYsRUFBb0IsR0FBcEI7QUFDQTtBQUNEO0FBQ0QsT0FBTSxLQUFLLElBQUksTUFBZjtBQUNBLE9BQUksQ0FBQyxFQUFMLEVBQVM7QUFDUCxTQUFJLEdBQUosSUFBVyxHQUFYO0FBQ0E7QUFDRDtBQUNELE1BQUcsT0FBSCxDQUFXLEdBQVgsRUFBZ0IsR0FBaEI7QUFDQSxNQUFHLEdBQUgsQ0FBTyxNQUFQO0FBQ0EsT0FBSSxHQUFHLEdBQVAsRUFBWTtBQUNWLFNBQUksSUFBSSxHQUFHLEdBQUgsQ0FBTyxNQUFmO0FBQ0EsWUFBTyxHQUFQLEVBQVk7QUFDVixXQUFNLEtBQUssR0FBRyxHQUFILENBQU8sQ0FBUCxDQUFYO0FBQ0EsYUFBTSxFQUFOLEVBQVUsR0FBVjtBQUNBLFVBQUcsWUFBSDtBQUNEO0FBQ0Y7QUFDRCxVQUFPLEdBQVA7QUFDRDs7QUFFRDs7Ozs7OztBQU9PLFVBQVMsR0FBVCxDQUFjLEdBQWQsRUFBbUIsR0FBbkIsRUFBd0I7QUFDN0IsT0FBSSxDQUFDLGtCQUFPLEdBQVAsRUFBWSxHQUFaLENBQUwsRUFBdUI7QUFDckI7QUFDRDtBQUNELFVBQU8sSUFBSSxHQUFKLENBQVA7QUFDQSxPQUFNLEtBQUssSUFBSSxNQUFmOztBQUVBLE9BQUksQ0FBQyxFQUFMLEVBQVM7QUFDUCxTQUFJLElBQUksTUFBUixFQUFnQjtBQUNkLGNBQU8sSUFBSSxLQUFKLENBQVUsR0FBVixDQUFQO0FBQ0EsV0FBSSxZQUFKO0FBQ0Q7QUFDRDtBQUNEO0FBQ0QsTUFBRyxHQUFILENBQU8sTUFBUDtBQUNBLE9BQUksR0FBRyxHQUFQLEVBQVk7QUFDVixTQUFJLElBQUksR0FBRyxHQUFILENBQU8sTUFBZjtBQUNBLFlBQU8sR0FBUCxFQUFZO0FBQ1YsV0FBTSxLQUFLLEdBQUcsR0FBSCxDQUFPLENBQVAsQ0FBWDtBQUNBLGVBQVEsRUFBUixFQUFZLEdBQVo7QUFDQSxVQUFHLFlBQUg7QUFDRDtBQUNGO0FBQ0Y7O0FBRUQsS0FBTSxZQUFZLENBQUMsUUFBRCxFQUFXLFFBQVgsRUFBcUIsUUFBckIsQ0FBbEI7QUFDTyxVQUFTLEtBQVQsQ0FBZ0IsRUFBaEIsRUFBb0IsR0FBcEIsRUFBeUI7QUFDOUIsT0FBSSxVQUFVLE9BQVYsQ0FBa0IsR0FBbEIsSUFBeUIsQ0FBQyxDQUExQixJQUErQixDQUFDLHNCQUFXLEdBQVgsQ0FBcEMsRUFBcUQ7QUFDbkQsWUFBTyxjQUFQLENBQXNCLEVBQXRCLEVBQTBCLEdBQTFCLEVBQStCO0FBQzdCLHFCQUFjLElBRGU7QUFFN0IsbUJBQVksSUFGaUI7QUFHN0IsWUFBSyxTQUFTLFdBQVQsR0FBd0I7QUFDM0IsZ0JBQU8sR0FBRyxLQUFILENBQVMsR0FBVCxDQUFQO0FBQ0QsUUFMNEI7QUFNN0IsWUFBSyxTQUFTLFdBQVQsQ0FBc0IsR0FBdEIsRUFBMkI7QUFDOUIsWUFBRyxLQUFILENBQVMsR0FBVCxJQUFnQixHQUFoQjtBQUNEO0FBUjRCLE1BQS9CO0FBVUQ7QUFDRjs7QUFFTSxVQUFTLE9BQVQsQ0FBa0IsRUFBbEIsRUFBc0IsR0FBdEIsRUFBMkI7QUFDaEMsT0FBSSxDQUFDLHNCQUFXLEdBQVgsQ0FBTCxFQUFzQjtBQUNwQixZQUFPLEdBQUcsR0FBSCxDQUFQO0FBQ0Q7QUFDRixFOzs7Ozs7Ozs7Ozs7O0FDMVREOztBQUVBLEtBQU0sYUFBYSxNQUFNLFNBQXpCLEMsQ0FKQTs7QUFLTyxLQUFNLHNDQUFlLE9BQU8sTUFBUCxDQUFjLFVBQWQ7O0FBRTVCOzs7O0FBRk8sRUFNTixDQUNDLE1BREQsRUFFQyxLQUZELEVBR0MsT0FIRCxFQUlDLFNBSkQsRUFLQyxRQUxELEVBTUMsTUFORCxFQU9DLFNBUEQsRUFTQSxPQVRBLENBU1EsVUFBVSxNQUFWLEVBQWtCO0FBQ3pCO0FBQ0EsT0FBTSxXQUFXLFdBQVcsTUFBWCxDQUFqQjtBQUNBLGtCQUFJLFlBQUosRUFBa0IsTUFBbEIsRUFBMEIsU0FBUyxPQUFULEdBQW9CO0FBQzVDO0FBQ0E7QUFDQSxTQUFJLElBQUksVUFBVSxNQUFsQjtBQUNBLFNBQU0sT0FBTyxJQUFJLEtBQUosQ0FBVSxDQUFWLENBQWI7QUFDQSxZQUFPLEdBQVAsRUFBWTtBQUNWLFlBQUssQ0FBTCxJQUFVLFVBQVUsQ0FBVixDQUFWO0FBQ0Q7QUFDRCxTQUFNLFNBQVMsU0FBUyxLQUFULENBQWUsSUFBZixFQUFxQixJQUFyQixDQUFmO0FBQ0EsU0FBTSxLQUFLLEtBQUssTUFBaEI7QUFDQSxTQUFJLGlCQUFKO0FBQ0EsYUFBUSxNQUFSO0FBQ0UsWUFBSyxNQUFMO0FBQ0Usb0JBQVcsSUFBWDtBQUNBO0FBQ0YsWUFBSyxTQUFMO0FBQ0Usb0JBQVcsSUFBWDtBQUNBO0FBQ0YsWUFBSyxRQUFMO0FBQ0Usb0JBQVcsS0FBSyxLQUFMLENBQVcsQ0FBWCxDQUFYO0FBQ0E7QUFUSjtBQVdBLFNBQUksUUFBSixFQUFjLEdBQUcsWUFBSCxDQUFnQixRQUFoQjtBQUNkO0FBQ0EsUUFBRyxHQUFILENBQU8sTUFBUDtBQUNBLFlBQU8sTUFBUDtBQUNELElBMUJEO0FBMkJELEVBdkNBOztBQXlDRDs7Ozs7Ozs7O0FBU0EsZ0JBQ0UsVUFERixFQUVFLE1BRkYsRUFHRSxTQUFTLElBQVQsQ0FBZSxLQUFmLEVBQXNCLEdBQXRCLEVBQTJCO0FBQ3pCLE9BQUksU0FBUyxLQUFLLE1BQWxCLEVBQTBCO0FBQ3hCLFVBQUssTUFBTCxHQUFjLFFBQVEsQ0FBdEI7QUFDRDtBQUNELFVBQU8sS0FBSyxNQUFMLENBQVksS0FBWixFQUFtQixDQUFuQixFQUFzQixHQUF0QixFQUEyQixDQUEzQixDQUFQO0FBQ0QsRUFSSDs7QUFXQTs7Ozs7OztBQU9BLGdCQUNFLFVBREYsRUFFRSxTQUZGLEVBR0UsU0FBUyxPQUFULENBQWtCLEtBQWxCLEVBQXlCO0FBQ3ZCO0FBQ0EsT0FBSSxDQUFDLEtBQUssTUFBVixFQUFrQjtBQUNsQixPQUFJLE9BQU8sS0FBUCxLQUFpQixRQUFyQixFQUErQjtBQUM3QixhQUFRLEtBQUssT0FBTCxDQUFhLEtBQWIsQ0FBUjtBQUNEO0FBQ0QsT0FBSSxRQUFRLENBQUMsQ0FBYixFQUFnQjtBQUNkLFVBQUssTUFBTCxDQUFZLEtBQVosRUFBbUIsQ0FBbkI7QUFDRDtBQUNGLEVBWkgsRTs7Ozs7Ozs7Ozs7O21QQy9FQTs7Ozs7U0FpRGdCLEssR0FBQSxLOztBQTVDaEI7O0FBSUE7O0FBSUE7O0FBUUE7O0FBU0E7Ozs7Ozs7Ozs7Ozs7Ozs7Ozs7QUFtQk8sVUFBUyxLQUFULENBQWdCLEVBQWhCLEVBQW9CO0FBQ3pCLE9BQU0sTUFBTSxHQUFHLFFBQUgsSUFBZSxFQUEzQjtBQUNBLE9BQU0sV0FBVyxJQUFJLFFBQUosSUFBZ0IsRUFBakM7O0FBRUEsT0FBSSxJQUFJLE9BQVIsRUFBaUI7QUFDZixTQUFJLFNBQVMsUUFBVCxJQUFxQixTQUFTLFFBQVQsQ0FBa0IsTUFBbEIsS0FBNkIsQ0FBdEQsRUFBeUQ7QUFDdkQsZUFBUSxFQUFSLEVBQVksU0FBUyxRQUFULENBQWtCLENBQWxCLENBQVosRUFBa0MsR0FBRyxTQUFyQztBQUNELE1BRkQsTUFHSztBQUNILGVBQVEsRUFBUixFQUFZLFNBQVMsUUFBckIsRUFBK0IsR0FBRyxTQUFsQztBQUNEO0FBQ0YsSUFQRCxNQVFLO0FBQ0gsYUFBUSxFQUFSLEVBQVksUUFBWixFQUFzQixHQUFHLFNBQXpCO0FBQ0Q7O0FBRUQsV0FBUSxLQUFSLDZDQUF3RCxHQUFHLEtBQTNEO0FBQ0EsTUFBRyxLQUFILENBQVMsWUFBVDtBQUNBLE1BQUcsTUFBSCxHQUFZLElBQVo7QUFDRDs7QUFFRDs7Ozs7Ozs7O0FBU0EsVUFBUyxPQUFULENBQWtCLEVBQWxCLEVBQXNCLE1BQXRCLEVBQThCLElBQTlCLEVBQW9DLElBQXBDLEVBQTBDO0FBQ3hDLE9BQU0sTUFBTSxHQUFHLElBQUgsSUFBVyxFQUF2Qjs7QUFFQSxPQUFJLElBQUksVUFBSixLQUFtQixDQUFDLENBQXhCLEVBQTJCO0FBQ3pCO0FBQ0Q7O0FBRUQsT0FBSSxpQkFBaUIsTUFBakIsQ0FBSixFQUE4QjtBQUM1QixxQkFBZ0IsRUFBaEIsRUFBb0IsTUFBcEIsRUFBNEIsSUFBNUIsRUFBa0MsSUFBbEM7QUFDQTtBQUNEO0FBQ0QsVUFBTyxRQUFRLEVBQWY7QUFDQSxPQUFJLGdCQUFnQixNQUFoQixDQUFKLEVBQTZCO0FBQzNCLGFBQVEsS0FBUixDQUFjLDJDQUFkLEVBQTJELE1BQTNEO0FBQ0EsUUFBRyxRQUFILEdBQWMsNEJBQVksRUFBWixFQUFnQixJQUFoQixDQUFkO0FBQ0E7QUFDRDs7QUFFRCxPQUFJLHNCQUFzQixNQUF0QixFQUE4QixJQUE5QixDQUFKLEVBQXlDO0FBQ3ZDLGFBQVEsS0FBUixDQUFjLDBDQUFkLEVBQTBELE1BQTFEO0FBQ0EsbUJBQWMsRUFBZCxFQUFrQixNQUFsQixFQUEwQixJQUExQjtBQUNBO0FBQ0Q7QUFDRCxPQUFJLHFCQUFxQixNQUFyQixFQUE2QixJQUE3QixDQUFKLEVBQXdDO0FBQ3RDLGFBQVEsS0FBUixDQUFjLHNDQUFkLEVBQXNELE1BQXREO0FBQ0Esa0JBQWEsRUFBYixFQUFpQixNQUFqQixFQUF5QixJQUF6QixFQUErQixJQUEvQjtBQUNBO0FBQ0Q7QUFDRCxPQUFNLGFBQWEsS0FBSyxJQUFMLElBQWEsT0FBTyxJQUF2QztBQUNBLE9BQUksb0JBQW9CLFVBQXBCLEVBQWdDLElBQWhDLENBQUosRUFBMkM7QUFDekMsaUJBQVksRUFBWixFQUFnQixNQUFoQixFQUF3QixJQUF4QixFQUE4QixVQUE5QixFQUEwQyxJQUExQztBQUNBO0FBQ0Q7QUFDRCxPQUFNLE9BQU8sVUFBYjtBQUNBLE9BQU0sWUFBWSxpQkFBaUIsRUFBakIsRUFBcUIsTUFBckIsRUFBNkIsSUFBN0IsQ0FBbEI7QUFDQSxPQUFJLFNBQUosRUFBZTtBQUNiLGFBQVEsS0FBUixDQUFjLDhDQUFkLEVBQThELE1BQTlEO0FBQ0EsNEJBQXVCLEVBQXZCLEVBQTJCLFNBQTNCLEVBQXNDLE1BQXRDLEVBQThDLElBQTlDLEVBQW9ELElBQXBELEVBQTBELElBQTFEO0FBQ0E7QUFDRDtBQUNELFdBQVEsS0FBUixDQUFjLDRDQUFkLEVBQTRELE1BQTVEO0FBQ0EsMEJBQXVCLEVBQXZCLEVBQTJCLE1BQTNCLEVBQW1DLElBQW5DLEVBQXlDLElBQXpDO0FBQ0Q7O0FBRUQ7Ozs7OztBQU1BLFVBQVMsZ0JBQVQsQ0FBMkIsTUFBM0IsRUFBbUM7QUFDakMsVUFBTyxNQUFNLE9BQU4sQ0FBYyxNQUFkLENBQVA7QUFDRDs7QUFFRDs7Ozs7O0FBTUEsVUFBUyxlQUFULENBQTBCLE1BQTFCLEVBQWtDO0FBQ2hDLFVBQU8sT0FBTyxJQUFQLEtBQWdCLFNBQWhCLElBQTZCLE9BQU8sSUFBUCxLQUFnQixNQUFwRDtBQUNEOztBQUVEOzs7Ozs7O0FBT0EsVUFBUyxxQkFBVCxDQUFnQyxNQUFoQyxFQUF3QyxJQUF4QyxFQUE4QztBQUM1QyxVQUFPLENBQUMsS0FBSyxjQUFMLENBQW9CLFFBQXBCLENBQUQsSUFBa0MsT0FBTyxNQUFoRDtBQUNEOztBQUVEOzs7Ozs7O0FBT0EsVUFBUyxvQkFBVCxDQUErQixNQUEvQixFQUF1QyxJQUF2QyxFQUE2QztBQUMzQyxVQUFPLENBQUMsS0FBSyxjQUFMLENBQW9CLE9BQXBCLENBQUQsSUFBaUMsT0FBTyxLQUEvQztBQUNEOztBQUVEOzs7Ozs7O0FBT0EsVUFBUyxtQkFBVCxDQUE4QixVQUE5QixFQUEwQyxJQUExQyxFQUFnRDtBQUM5QyxVQUFRLE9BQU8sVUFBUCxLQUFzQixVQUF2QixJQUFzQyxDQUFDLEtBQUssY0FBTCxDQUFvQixNQUFwQixDQUE5QztBQUNEOztBQUVEOzs7Ozs7QUFNQSxVQUFTLGdCQUFULENBQTJCLEVBQTNCLEVBQStCLE1BQS9CLEVBQXVDLElBQXZDLEVBQTZDO0FBQzNDLE9BQUksa0JBQUo7QUFDQSxPQUFJLEdBQUcsSUFBSCxJQUFXLEdBQUcsSUFBSCxDQUFRLGtCQUF2QixFQUEyQztBQUN6QyxpQkFBWSxHQUFHLElBQUgsQ0FBUSxrQkFBUixDQUEyQixJQUEzQixDQUFaO0FBQ0Q7QUFDRCxPQUFJLEdBQUcsUUFBSCxJQUFlLEdBQUcsUUFBSCxDQUFZLFVBQS9CLEVBQTJDO0FBQ3pDLGlCQUFZLEdBQUcsUUFBSCxDQUFZLFVBQVosQ0FBdUIsSUFBdkIsQ0FBWjtBQUNEO0FBQ0QsT0FBSSxPQUFPLFNBQVgsRUFBc0I7QUFDcEIsaUJBQVksYUFBYSxFQUF6QjtBQUNEO0FBQ0QsVUFBTyxTQUFQO0FBQ0Q7O0FBRUQ7Ozs7Ozs7QUFPQSxVQUFTLGVBQVQsQ0FBMEIsRUFBMUIsRUFBOEIsTUFBOUIsRUFBc0MsSUFBdEMsRUFBNEMsSUFBNUMsRUFBa0Q7QUFDaEQsT0FBTSxZQUFZLDRCQUFZLEVBQVosRUFBZ0IsSUFBaEIsQ0FBbEI7QUFDQSxVQUFPLE9BQVAsQ0FBZSxVQUFDLEtBQUQsRUFBVztBQUN4QixhQUFRLEVBQVIsRUFBWSxLQUFaLEVBQW1CLFNBQW5CLEVBQThCLElBQTlCO0FBQ0QsSUFGRDtBQUdEOztBQUVEOzs7Ozs7QUFNQSxVQUFTLGFBQVQsQ0FBd0IsRUFBeEIsRUFBNEIsTUFBNUIsRUFBb0MsSUFBcEMsRUFBMEM7QUFDeEMsT0FBTSxTQUFTLE9BQU8sTUFBdEI7QUFDQSxPQUFNLFdBQVcsT0FBTyxNQUFQLEtBQWtCLFVBQW5DO0FBQ0EsT0FBSSxTQUFTLE9BQU8sTUFBUCxJQUFpQixPQUFPLFVBQXhCLElBQXNDLE1BQW5EO0FBQ0EsT0FBSSxPQUFPLE1BQVAsS0FBa0IsVUFBdEIsRUFBa0M7QUFDaEMsY0FBUyxrQkFBWTtBQUFFLGNBQU8sRUFBUDtBQUFXLE1BQWxDO0FBQ0Q7QUFDRCxPQUFNLE1BQU0sT0FBTyxHQUFQLElBQWMsUUFBMUI7QUFDQSxPQUFNLFFBQVEsT0FBTyxLQUFQLElBQWdCLFFBQTlCO0FBQ0EsT0FBTSxVQUFVLE9BQU8sT0FBUCxJQUFrQixPQUFPLE9BQXpCLElBQ2IsT0FBTyxJQUFQLElBQWUsT0FBTyxJQUFQLENBQVksT0FEOUI7O0FBR0EsT0FBTSxZQUFZLDRCQUFZLEVBQVosRUFBZ0IsSUFBaEIsQ0FBbEI7QUFDQSxhQUFVLFFBQVYsR0FBcUIsRUFBckI7QUFDQSxhQUFVLElBQVYsR0FBaUIsRUFBakI7QUFDQSxhQUFVLEdBQVYsR0FBZ0IsRUFBaEI7O0FBRUEsY0FBVyxFQUFYLEVBQWUsTUFBZixFQUF1QixTQUF2QixFQUFrQyxFQUFFLGNBQUYsRUFBVSxRQUFWLEVBQWUsWUFBZixFQUFzQixnQkFBdEIsRUFBK0Isa0JBQS9CLEVBQWxDO0FBQ0Q7O0FBRUQ7Ozs7Ozs7QUFPQSxVQUFTLFlBQVQsQ0FBdUIsRUFBdkIsRUFBMkIsTUFBM0IsRUFBbUMsSUFBbkMsRUFBeUMsSUFBekMsRUFBK0M7QUFDN0MsT0FBTSxVQUFVLEVBQUUsT0FBTyxJQUFULEVBQWhCO0FBQ0EsT0FBTSxZQUFZLDRCQUFZLEVBQVosRUFBZ0IsSUFBaEIsQ0FBbEI7O0FBRUEsT0FBSSxLQUFLLE9BQUwsSUFBZ0IsS0FBSyxRQUF6QixFQUFtQztBQUNqQyxVQUFLLFFBQUwsQ0FBYyxJQUFkLENBQW1CLFNBQW5CO0FBQ0Q7O0FBRUQsT0FBSSxLQUFLLE1BQVQsRUFBaUI7QUFDZixhQUFRLE1BQVIsR0FBaUIsS0FBSyxNQUF0QjtBQUNEOztBQUVELGFBQVUsRUFBVixFQUFjLE1BQWQsRUFBc0IsU0FBdEIsRUFBaUMsT0FBakM7QUFDRDs7QUFFRDs7Ozs7OztBQU9BLFVBQVMsV0FBVCxDQUFzQixFQUF0QixFQUEwQixNQUExQixFQUFrQyxJQUFsQyxFQUF3QyxVQUF4QyxFQUFvRCxJQUFwRCxFQUEwRDtBQUN4RCxPQUFNLE9BQU8sV0FBVyxJQUFYLENBQWdCLEVBQWhCLENBQWI7QUFDQSxPQUFNLFVBQVUsa0JBQU8sRUFBRSxVQUFGLEVBQVAsRUFBaUIsSUFBakIsQ0FBaEI7QUFDQSxPQUFNLFlBQVksNEJBQVksRUFBWixFQUFnQixJQUFoQixDQUFsQjs7QUFFQSxPQUFJLEtBQUssT0FBTCxJQUFnQixLQUFLLFFBQXpCLEVBQW1DO0FBQ2pDLFVBQUssUUFBTCxDQUFjLElBQWQsQ0FBbUIsU0FBbkI7QUFDRDs7QUFFRCx5QkFBTSxFQUFOLEVBQVUsVUFBVixFQUFzQixVQUFDLEtBQUQsRUFBVztBQUMvQixTQUFNLFVBQVUsa0JBQU8sRUFBRSxNQUFNLEtBQVIsRUFBUCxFQUF3QixJQUF4QixDQUFoQjtBQUNBLGtDQUFhLEVBQWIsRUFBaUIsU0FBakIsRUFBNEIsSUFBNUI7QUFDQSxhQUFRLEVBQVIsRUFBWSxNQUFaLEVBQW9CLFNBQXBCLEVBQStCLE9BQS9CO0FBQ0QsSUFKRDs7QUFNQSxXQUFRLEVBQVIsRUFBWSxNQUFaLEVBQW9CLFNBQXBCLEVBQStCLE9BQS9CO0FBQ0Q7O0FBRUQ7Ozs7Ozs7QUFPQSxVQUFTLHNCQUFULENBQWlDLEVBQWpDLEVBQXFDLFNBQXJDLEVBQWdELE1BQWhELEVBQXdELElBQXhELEVBQThELElBQTlELEVBQW9FLElBQXBFLEVBQTBFO0FBQ3hFLE9BQU0sT0FBTyxHQUFHLFdBQWhCO0FBQ0EsT0FBTSxRQUFRLElBQUksSUFBSixDQUFTLElBQVQsRUFBZSxTQUFmLEVBQTBCLEVBQTFCLEVBQThCLElBQTlCLEVBQW9DLFNBQXBDLEVBQStDO0FBQzNELGtCQUFhLG9CQUFZO0FBQ3ZCLDZCQUFNLEVBQU4sRUFBVSxJQUFWLEVBQWdCLE9BQU8sRUFBdkIsRUFBMkIsSUFBM0I7QUFDQTtBQUNBLFlBQUssZ0JBQUwsR0FBd0I7QUFDdEIsaUJBQVEsRUFEYztBQUV0QixtQkFBVTtBQUZZLFFBQXhCO0FBSUQsTUFSMEQ7QUFTM0QscUJBQWdCLHVCQUFZO0FBQzFCLGlDQUFVLEVBQVYsRUFBYyxJQUFkLEVBQW9CLE1BQXBCLEVBQTRCLEtBQUssTUFBakM7QUFDRCxNQVgwRDtBQVkzRCxtQkFBYyxxQkFBWTtBQUN4QixXQUFJLEtBQUssUUFBVCxFQUFtQjtBQUNqQix5QkFBZ0IsRUFBaEIsRUFBb0IsTUFBcEIsRUFBNEIsS0FBSyxRQUFqQztBQUNEO0FBQ0Y7QUFoQjBELElBQS9DLENBQWQ7QUFrQkEsNkNBQTBCLEVBQTFCLEVBQThCLEtBQTlCLEVBQXFDLE1BQXJDO0FBQ0Q7O0FBRUQ7Ozs7Ozs7O0FBUUEsVUFBUyxzQkFBVCxDQUFpQyxFQUFqQyxFQUFxQyxRQUFyQyxFQUErQyxJQUEvQyxFQUFxRCxJQUFyRCxFQUEyRDtBQUN6RCwrQ0FBNEIsUUFBNUI7O0FBRUEsT0FBSSxnQkFBSjtBQUNBLE9BQUksS0FBSyxHQUFMLEtBQWEsa0JBQWpCLEVBQXFDO0FBQ25DO0FBQ0EsYUFBUSxLQUFSLGdEQUEyRCxJQUEzRDtBQUNBLGVBQVUsMkJBQVcsRUFBWCxFQUFlLElBQWYsQ0FBVjtBQUNELElBSkQsTUFLSztBQUNILGFBQVEsS0FBUixtREFBOEQsSUFBOUQ7QUFDQSxlQUFVLDhCQUFjLEVBQWQsRUFBa0IsSUFBbEIsQ0FBVjtBQUNEOztBQUVELE9BQUksQ0FBQyxHQUFHLE9BQVIsRUFBaUI7QUFDZixRQUFHLE9BQUgsR0FBYSxPQUFiO0FBQ0E7QUFDQSxTQUFNLFVBQVUsR0FBRyxnQkFBSCxJQUF1QixFQUF2QztBQUNBLFNBQU0sU0FBUyxRQUFRLFFBQXZCO0FBQ0EsU0FBTSxXQUFXLFFBQVEsTUFBekI7QUFDQSxTQUFJLFVBQVUsT0FBTyxNQUFqQixJQUEyQixRQUEzQixJQUF1QyxPQUEzQyxFQUFvRDtBQUNsRCxZQUFLLElBQU0sS0FBWCxJQUFtQixPQUFPLE1BQTFCLEVBQWtDO0FBQ2hDLGFBQU0sVUFBVSxTQUFTLE9BQU8sTUFBUCxDQUFjLEtBQWQsQ0FBVCxDQUFoQjtBQUNBLGFBQUksT0FBSixFQUFhO0FBQ1gsbUJBQVEsUUFBUixDQUFpQixLQUFqQixFQUF1QixnQkFBSyxPQUFMLEVBQWMsUUFBZCxDQUF2QjtBQUNEO0FBQ0Y7QUFDRjtBQUNGOztBQUVELCtCQUFZLEVBQVosRUFBZ0IsT0FBaEIsRUFBeUIsUUFBekI7O0FBRUEsT0FBSSxTQUFTLElBQVQsSUFBaUIsU0FBUyxJQUFULENBQWMsTUFBbkMsRUFBMkM7QUFBRTtBQUMzQyxjQUFTLE1BQVQsR0FBa0IsU0FBUyxJQUFULENBQWMsTUFBaEM7QUFDRDs7QUFFRCxPQUFJLFNBQVMsTUFBYixFQUFxQjtBQUFFO0FBQ3JCLGFBQVEsSUFBUixHQUFlLFFBQVEsSUFBUixJQUFnQixFQUEvQjtBQUNBLGFBQVEsSUFBUixDQUFhLE1BQWIsR0FBc0IsU0FBUyxNQUEvQjtBQUNEOztBQUVELE9BQU0sV0FBVyxTQUFTLE1BQVQsS0FBb0IsTUFBckM7QUFDQSxPQUFNLE1BQU0sR0FBRyxJQUFILElBQVcsRUFBdkI7QUFDQSxPQUFJLElBQUksVUFBSixLQUFtQixDQUFDLENBQXBCLElBQXlCLENBQUMsUUFBOUIsRUFBd0M7QUFDdEMsYUFBUSxLQUFSLENBQWMsa0RBQWQsRUFBa0UsT0FBbEU7QUFDQSxTQUFJLFVBQUosR0FBaUIsNkJBQWEsRUFBYixFQUFpQixPQUFqQixFQUEwQixJQUExQixDQUFqQjtBQUNEO0FBQ0QsT0FBSSxJQUFJLFVBQUosS0FBbUIsQ0FBQyxDQUF4QixFQUEyQjtBQUN6QixxQkFBZ0IsRUFBaEIsRUFBb0IsUUFBcEIsRUFBOEIsT0FBOUI7QUFDRDtBQUNELE9BQUksSUFBSSxVQUFKLEtBQW1CLENBQUMsQ0FBcEIsSUFBeUIsUUFBN0IsRUFBdUM7QUFDckMsYUFBUSxLQUFSLENBQWMsaURBQWQsRUFBaUUsT0FBakU7QUFDQSxTQUFJLFVBQUosR0FBaUIsNkJBQWEsRUFBYixFQUFpQixPQUFqQixFQUEwQixJQUExQixDQUFqQjtBQUNEO0FBQ0Y7O0FBRUQ7Ozs7OztBQU1BLFVBQVMsZUFBVCxDQUEwQixFQUExQixFQUE4QixRQUE5QixFQUF3QyxJQUF4QyxFQUE4QztBQUM1QyxPQUFNLE1BQU0sR0FBRyxJQUFILElBQVcsRUFBdkI7QUFDQSxPQUFNLFdBQVcsU0FBUyxRQUExQjtBQUNBLE9BQUksWUFBWSxTQUFTLE1BQXpCLEVBQWlDO0FBQy9CLGNBQVMsS0FBVCxDQUFlLFVBQUMsS0FBRCxFQUFXO0FBQ3hCLGVBQVEsRUFBUixFQUFZLEtBQVosRUFBbUIsSUFBbkI7QUFDQSxjQUFPLElBQUksVUFBSixLQUFtQixDQUFDLENBQTNCO0FBQ0QsTUFIRDtBQUlEO0FBQ0Y7O0FBRUQ7Ozs7Ozs7QUFPQSxVQUFTLFVBQVQsQ0FBcUIsRUFBckIsRUFBeUIsTUFBekIsRUFBaUMsU0FBakMsRUFBNEMsSUFBNUMsRUFBa0Q7QUFDaEQsT0FBTSxNQUFNLFVBQVUsR0FBdEI7QUFDQSxPQUFNLFdBQVcsVUFBVSxRQUEzQjtBQUZnRCxPQUd4QyxNQUh3QyxHQUdWLElBSFUsQ0FHeEMsTUFId0M7QUFBQSxPQUdoQyxPQUhnQyxHQUdWLElBSFUsQ0FHaEMsT0FIZ0M7QUFBQSxPQUd2QixRQUh1QixHQUdWLElBSFUsQ0FHdkIsUUFIdUI7O0FBSWhELE9BQU0sVUFBVSxLQUFLLEdBQXJCO0FBQ0EsT0FBTSxZQUFZLEtBQUssS0FBdkI7O0FBRUEsWUFBUyxXQUFULENBQXNCLElBQXRCLEVBQTRCLEtBQTVCLEVBQW1DLE9BQW5DLEVBQTRDO0FBQzFDLFNBQUksbUJBQUo7QUFDQSxTQUFJLFFBQUosRUFBYztBQUNaLG9CQUFhLElBQWI7QUFDQSxXQUFJLFFBQU8sSUFBUCx5Q0FBTyxJQUFQLE9BQWdCLFFBQXBCLEVBQThCO0FBQzVCLG9CQUFXLE9BQVgsSUFBc0IsS0FBdEI7QUFDQSxhQUFJLENBQUMsV0FBVyxjQUFYLENBQTBCLE9BQTFCLENBQUwsRUFBeUM7QUFDdkMsa0JBQU8sY0FBUCxDQUFzQixVQUF0QixFQUFrQyxPQUFsQyxFQUEyQztBQUN6QyxvQkFBTyxpQkFBTTtBQUNYLHVCQUFRLElBQVIsQ0FBYSxxREFDWCw2QkFERjtBQUVEO0FBSndDLFlBQTNDO0FBTUQ7QUFDRjtBQUNGLE1BYkQsTUFjSztBQUNILG9CQUFhLEVBQWI7QUFDQSxrQkFBVyxPQUFYLElBQXNCLEtBQXRCO0FBQ0Esa0JBQVcsU0FBWCxJQUF3QixJQUF4QjtBQUNEO0FBQ0QsU0FBTSxhQUFhLGFBQWEsT0FBYixFQUFzQixVQUF0QixDQUFuQjtBQUNBLFNBQUksSUFBSixDQUFTLFVBQVQ7QUFDQSxhQUFRLFVBQVIsRUFBb0IsTUFBcEIsRUFBNEIsU0FBNUIsRUFBdUMsRUFBRSxRQUFRLElBQVYsRUFBdkM7QUFDRDs7QUFFRCxPQUFNLE9BQU8sV0FBVyxFQUFYLEVBQWUsU0FBZixFQUEwQixNQUExQixFQUFrQyxRQUFsQyxFQUNYLFVBQUMsSUFBRCxFQUFVO0FBQ1IsYUFBUSxLQUFSLENBQWMsOENBQWQsRUFBOEQsSUFBOUQ7QUFDQSxTQUFJLENBQUMsU0FBTCxFQUFnQjtBQUNkO0FBQ0Q7O0FBRUQsU0FBTSxjQUFjLFNBQVMsS0FBVCxFQUFwQjtBQUNBLFNBQU0sU0FBUyxJQUFJLEtBQUosRUFBZjtBQUNBLFNBQU0sVUFBVSxVQUFVLElBQVYsQ0FBZSxLQUFmLEVBQWhCO0FBQ0E7QUFDQSxTQUFNLFdBQVcsRUFBakI7QUFDQSxTQUFNLFlBQVksRUFBbEI7QUFDQSxVQUFLLE9BQUwsQ0FBYSxVQUFDLElBQUQsRUFBTyxLQUFQLEVBQWlCO0FBQzVCLFdBQU0sTUFBTSxVQUFVLEtBQUssT0FBTCxDQUFWLEdBQTJCLFdBQVcsS0FBSyxPQUFMLENBQVgsR0FBMkIsS0FBbEU7QUFDQTtBQUNBLFdBQUksT0FBTyxJQUFQLElBQWUsUUFBUSxFQUEzQixFQUErQjtBQUM3QjtBQUNEO0FBQ0QsZ0JBQVMsR0FBVCxJQUFnQixJQUFoQjtBQUNELE1BUEQ7O0FBU0E7QUFDQSxTQUFNLGFBQWEsRUFBbkI7QUFDQSxhQUFRLE9BQVIsQ0FBZ0IsVUFBQyxJQUFELEVBQU8sS0FBUCxFQUFpQjtBQUMvQixXQUFNLE1BQU0sVUFBVSxLQUFLLE9BQUwsQ0FBVixHQUEyQixXQUFXLEtBQUssT0FBTCxDQUFYLEdBQTJCLEtBQWxFO0FBQ0EsV0FBSSxTQUFTLGNBQVQsQ0FBd0IsR0FBeEIsQ0FBSixFQUFrQztBQUNoQyxtQkFBVSxHQUFWLElBQWlCO0FBQ2YscUJBRGUsRUFDVCxZQURTLEVBQ0YsUUFERTtBQUVmLG1CQUFRLFlBQVksS0FBWixDQUZPO0FBR2YsZUFBSSxPQUFPLEtBQVA7QUFIVyxVQUFqQjtBQUtBLG9CQUFXLElBQVgsQ0FBZ0IsSUFBaEI7QUFDRCxRQVBELE1BUUs7QUFDSCxzQ0FBYSxFQUFiLEVBQWlCLFlBQVksS0FBWixDQUFqQjtBQUNEO0FBQ0YsTUFiRDs7QUFlQTtBQUNBLGNBQVMsTUFBVCxHQUFrQixDQUFsQjtBQUNBLFNBQUksTUFBSixHQUFhLENBQWI7QUFDQSxlQUFVLElBQVYsR0FBaUIsS0FBSyxLQUFMLEVBQWpCO0FBQ0EsZUFBVSxVQUFWLEdBQXVCLFVBQVUsS0FBakM7O0FBRUEsVUFBSyxPQUFMLENBQWEsVUFBQyxJQUFELEVBQU8sS0FBUCxFQUFpQjtBQUM1QixXQUFNLE1BQU0sVUFBVSxLQUFLLE9BQUwsQ0FBVixHQUEyQixXQUFXLEtBQUssT0FBTCxDQUFYLEdBQTJCLEtBQWxFO0FBQ0EsV0FBTSxTQUFTLFVBQVUsR0FBVixDQUFmO0FBQ0EsV0FBSSxNQUFKLEVBQVk7QUFDVixhQUFJLE9BQU8sSUFBUCxLQUFnQixXQUFXLENBQVgsQ0FBcEIsRUFBbUM7QUFDakMsc0JBQVcsS0FBWDtBQUNELFVBRkQsTUFHSztBQUNILHNCQUFXLE9BQVgsQ0FBbUIsT0FBTyxJQUExQjtBQUNBLHNDQUFXLEVBQVgsRUFBZSxPQUFPLE1BQXRCLEVBQThCLFVBQVUsVUFBeEMsRUFBb0QsSUFBcEQ7QUFDRDtBQUNELGtCQUFTLElBQVQsQ0FBYyxPQUFPLE1BQXJCO0FBQ0EsYUFBSSxJQUFKLENBQVMsT0FBTyxFQUFoQjtBQUNBLGFBQUksUUFBSixFQUFjO0FBQ1osa0JBQU8sRUFBUCxHQUFZLElBQVo7QUFDRCxVQUZELE1BR0s7QUFDSCxrQkFBTyxFQUFQLENBQVUsU0FBVixJQUF1QixJQUF2QjtBQUNEO0FBQ0QsZ0JBQU8sRUFBUCxDQUFVLE9BQVYsSUFBcUIsS0FBckI7QUFDQSxtQkFBVSxVQUFWLEdBQXVCLE9BQU8sTUFBOUI7QUFDRCxRQWxCRCxNQW1CSztBQUNILHFCQUFZLElBQVosRUFBa0IsS0FBbEIsRUFBeUIsRUFBekI7QUFDRDtBQUNGLE1BekJEOztBQTJCQSxZQUFPLFVBQVUsVUFBakI7QUFDRCxJQXpFVSxDQUFiOztBQTRFQSxhQUFVLElBQVYsR0FBaUIsS0FBSyxLQUFMLENBQVcsQ0FBWCxDQUFqQjtBQUNBLFFBQUssT0FBTCxDQUFhLFVBQUMsSUFBRCxFQUFPLEtBQVAsRUFBaUI7QUFDNUIsaUJBQVksSUFBWixFQUFrQixLQUFsQixFQUF5QixFQUF6QjtBQUNELElBRkQ7QUFHRDs7QUFFRDs7Ozs7OztBQU9BLFVBQVMsU0FBVCxDQUFvQixFQUFwQixFQUF3QixNQUF4QixFQUFnQyxTQUFoQyxFQUEyQyxJQUEzQyxFQUFpRDtBQUMvQyxPQUFNLFVBQVUsV0FBVyxFQUFYLEVBQWUsU0FBZixFQUEwQixPQUFPLEtBQWpDLEVBQXdDLE9BQXhDLEVBQ2QsVUFBQyxPQUFELEVBQWE7QUFDWCxhQUFRLEtBQVIsQ0FBYywwQ0FBZCxFQUEwRCxPQUExRDs7QUFFQSxTQUFJLENBQUMsU0FBRCxJQUFjLENBQUMsQ0FBQyxVQUFVLE9BQVosS0FBd0IsQ0FBQyxDQUFDLE9BQTVDLEVBQXFEO0FBQ25EO0FBQ0Q7QUFDRCxlQUFVLE9BQVYsR0FBb0IsQ0FBQyxDQUFDLE9BQXRCO0FBQ0EsU0FBSSxPQUFKLEVBQWE7QUFDWCxlQUFRLEVBQVIsRUFBWSxNQUFaLEVBQW9CLFNBQXBCLEVBQStCLElBQS9CO0FBQ0QsTUFGRCxNQUdLO0FBQ0gsb0NBQWEsRUFBYixFQUFpQixTQUFqQixFQUE0QixJQUE1QjtBQUNEO0FBQ0YsSUFkYSxDQUFoQjs7QUFpQkEsYUFBVSxPQUFWLEdBQW9CLENBQUMsQ0FBQyxPQUF0QjtBQUNBLE9BQUksT0FBSixFQUFhO0FBQ1gsYUFBUSxFQUFSLEVBQVksTUFBWixFQUFvQixTQUFwQixFQUErQixJQUEvQjtBQUNEO0FBQ0Y7O0FBRUQ7Ozs7Ozs7Ozs7QUFVQSxVQUFTLFVBQVQsQ0FBcUIsRUFBckIsRUFBeUIsU0FBekIsRUFBb0MsSUFBcEMsRUFBMEMsSUFBMUMsRUFBZ0QsT0FBaEQsRUFBeUQ7QUFDdkQsT0FBTSxTQUFTLE1BQU0sR0FBRyxJQUFULElBQWlCLEdBQUcsSUFBSCxDQUFRLE1BQXhDO0FBQ0EsT0FBTSxTQUFTLEVBQWY7QUFDQSxPQUFNLFFBQVEsQ0FBQyxVQUFVLE9BQVYsQ0FBa0IsS0FBbEIsSUFBMkIsQ0FBNUIsSUFBaUMsQ0FBL0M7O0FBRUEsVUFBTyxzQkFBTSxFQUFOLEVBQVUsSUFBVixFQUFnQixVQUFDLEtBQUQsRUFBVztBQUNoQyxZQUFPLFdBQVAsR0FBcUIsS0FBckI7QUFDQSxTQUFJLFVBQVUsQ0FBQyxPQUFPLFFBQXRCLEVBQWdDO0FBQzlCLGNBQU8sTUFBUCxDQUFjLElBQWQsRUFBb0IsS0FBcEIsRUFBMkIsVUFBVSxPQUFyQyxFQUE4QyxZQUFNO0FBQ2xELGFBQU0sY0FBYyxPQUFPLFdBQTNCO0FBQ0EsaUJBQVEsV0FBUjtBQUNBLGdCQUFPLFFBQVAsR0FBa0IsS0FBbEI7QUFDQSxnQkFBTyxXQUFQLEdBQXFCLFNBQXJCO0FBQ0QsUUFMRDtBQU1EO0FBQ0QsWUFBTyxRQUFQLEdBQWtCLElBQWxCO0FBQ0QsSUFYTSxDQUFQO0FBWUQ7O0FBRUQ7Ozs7OztBQU1BLFVBQVMsWUFBVCxDQUF1QixPQUF2QixFQUFnQyxVQUFoQyxFQUE0QztBQUMxQyxPQUFNLGFBQWEsT0FBTyxNQUFQLENBQWMsT0FBZCxDQUFuQjtBQUNBLGNBQVcsS0FBWCxHQUFtQixVQUFuQjtBQUNBLHdCQUFTLFVBQVQ7QUFDQSw0QkFBYSxVQUFiO0FBQ0EsY0FBVyxXQUFYLEdBQXlCLE9BQXpCO0FBQ0EsVUFBTyxVQUFQO0FBQ0QsRTs7Ozs7Ozs7Ozs7O21QQzdrQkQ7Ozs7O1NBc0JnQiwyQixHQUFBLDJCO1NBd0JBLFcsR0FBQSxXO1NBWUEsUyxHQUFBLFM7U0F1QkEseUIsR0FBQSx5QjtTQTRFQSxLLEdBQUEsSztTQW1LQSxLLEdBQUEsSzs7QUEzVGhCOztBQUVBOzs7O0FBQ0E7Ozs7OztLQUVRLGtCLG9CQUFBLGtCOzs7QUFFUixLQUFNLFVBQVU7QUFDZCxTQUFNLFNBRFE7QUFFZCxVQUFPLFVBRk87QUFHZCxVQUFPO0FBSE8sRUFBaEI7O0FBTUE7Ozs7QUFJTyxVQUFTLDJCQUFULENBQXNDLFFBQXRDLEVBQWdEO0FBQUEsT0FDN0MsSUFENkMsR0FDcEMsUUFEb0MsQ0FDN0MsSUFENkM7O0FBRXJELE9BQU0sVUFBVSxtQkFBbUIsSUFBbkIsQ0FBaEI7O0FBRUEsT0FBSSxRQUFPLE9BQVAseUNBQU8sT0FBUCxPQUFtQixRQUF2QixFQUFpQztBQUMvQixVQUFLLElBQU0sR0FBWCxJQUFrQixPQUFsQixFQUEyQjtBQUN6QixXQUFJLFNBQVMsR0FBVCxLQUFpQixJQUFyQixFQUEyQjtBQUN6QixrQkFBUyxHQUFULElBQWdCLFFBQVEsR0FBUixDQUFoQjtBQUNELFFBRkQsTUFHSyxJQUFJLGlCQUFNLFNBQVMsR0FBVCxDQUFOLE1BQXlCLFFBQXpCLElBQ1AsaUJBQU0sUUFBUSxHQUFSLENBQU4sTUFBd0IsUUFEckIsRUFDK0I7QUFDbEMsY0FBSyxJQUFNLE1BQVgsSUFBcUIsUUFBUSxHQUFSLENBQXJCLEVBQW1DO0FBQ2pDLGVBQUksU0FBUyxHQUFULEVBQWMsTUFBZCxLQUF5QixJQUE3QixFQUFtQztBQUNqQyxzQkFBUyxHQUFULEVBQWMsTUFBZCxJQUF3QixRQUFRLEdBQVIsRUFBYSxNQUFiLENBQXhCO0FBQ0Q7QUFDRjtBQUNGO0FBQ0Y7QUFDRjtBQUNGOztBQUVEOzs7QUFHTyxVQUFTLFdBQVQsQ0FBc0IsRUFBdEIsRUFBMEIsRUFBMUIsRUFBOEIsUUFBOUIsRUFBd0M7QUFDN0MsU0FBTSxFQUFOLEVBQVUsRUFBVixFQUFjLFNBQVMsRUFBdkIsRUFBMkIsRUFBM0I7QUFDQSxXQUFRLEVBQVIsRUFBWSxFQUFaLEVBQWdCLFNBQVMsSUFBekI7QUFDQSxZQUFTLEVBQVQsRUFBYSxFQUFiLEVBQWlCLFNBQVMsU0FBMUI7QUFDQSxZQUFTLEVBQVQsRUFBYSxFQUFiLEVBQWlCLFNBQVMsS0FBMUI7QUFDQSxjQUFXLEVBQVgsRUFBZSxFQUFmLEVBQW1CLFNBQVMsTUFBNUI7QUFDRDs7QUFFRDs7OztBQUlPLFVBQVMsU0FBVCxDQUFvQixFQUFwQixFQUF3QixLQUF4QixFQUErQixRQUEvQixFQUF5QyxVQUF6QyxFQUFxRDtBQUMxRCxXQUFRLFNBQVMsRUFBakI7QUFDQSxjQUFXLFlBQVksRUFBdkI7O0FBRUEsT0FBTSxVQUFVLE1BQU0sUUFBTixJQUFrQixFQUFsQzs7QUFFQTtBQUNBLE9BQUksUUFBUSxRQUFRLEtBQXBCOztBQUVBLE9BQUksTUFBTSxPQUFOLENBQWMsS0FBZCxDQUFKLEVBQTBCO0FBQ3hCLGFBQVEsTUFBTSxNQUFOLENBQWEsVUFBQyxNQUFELEVBQVMsS0FBVCxFQUFtQjtBQUN0QyxjQUFPLEtBQVAsSUFBZ0IsSUFBaEI7QUFDQSxjQUFPLE1BQVA7QUFDRCxNQUhPLEVBR0wsRUFISyxDQUFSO0FBSUQ7O0FBRUQsY0FBVyxVQUFYLEVBQXVCLEtBQXZCLEVBQThCLEVBQTlCLEVBQWtDLEtBQWxDO0FBQ0EsY0FBVyxTQUFTLElBQXBCLEVBQTBCLEtBQTFCLEVBQWlDLEVBQWpDLEVBQXFDLEtBQXJDO0FBQ0Q7O0FBRUQ7OztBQUdPLFVBQVMseUJBQVQsQ0FBb0MsRUFBcEMsRUFBd0MsS0FBeEMsRUFBK0MsUUFBL0MsRUFBeUQ7QUFDOUQsbUJBQWdCLFNBQVMsU0FBekIsRUFBb0MsRUFBcEMsRUFBd0MsS0FBeEM7QUFDQSxjQUFXLFNBQVMsS0FBcEIsRUFBMkIsRUFBM0IsRUFBK0IsS0FBL0I7QUFDRDs7QUFFRDs7O0FBR0EsVUFBUyxVQUFULENBQXFCLE1BQXJCLEVBQTZCLEtBQTdCLEVBQW9DLEVBQXBDLEVBQXdDLEtBQXhDLEVBQStDO0FBQzdDLE9BQUksQ0FBQyxNQUFMLEVBQWE7QUFDWDtBQUNEOztBQUg0Qyw4QkFJbEMsR0FKa0M7QUFLM0MsU0FBSSxDQUFDLEtBQUQsSUFBVSxNQUFNLEdBQU4sQ0FBZCxFQUEwQjtBQUN4QixXQUFNLFFBQVEsT0FBTyxHQUFQLENBQWQ7QUFDQSxXQUFJLE9BQU8sS0FBUCxLQUFpQixVQUFyQixFQUFpQztBQUMvQixhQUFNLGNBQWMsTUFBTSxFQUFOLEVBQVUsS0FBVixFQUFpQixVQUFVLENBQVYsRUFBYTtBQUNoRCxpQkFBTSxHQUFOLElBQWEsQ0FBYjtBQUNELFVBRm1CLENBQXBCO0FBR0EsZUFBTSxHQUFOLElBQWEsV0FBYjtBQUNELFFBTEQsTUFNSztBQUNILGVBQU0sR0FBTixJQUFhLEtBQWI7QUFDRDtBQUNGO0FBaEIwQzs7QUFJN0MsUUFBSyxJQUFNLEdBQVgsSUFBa0IsTUFBbEIsRUFBMEI7QUFBQSxXQUFmLEdBQWU7QUFhekI7QUFDRjs7QUFFRDs7O0FBR0EsVUFBUyxVQUFULENBQXFCLE1BQXJCLEVBQTZCLEVBQTdCLEVBQWlDLEtBQWpDLEVBQXdDO0FBQUEsZ0NBQzNCLEdBRDJCO0FBRXBDLFNBQU0sUUFBUSxPQUFPLEdBQVAsQ0FBZDtBQUNBLFNBQUksT0FBTyxLQUFQLEtBQWlCLFVBQXJCLEVBQWlDO0FBQy9CLFdBQU0sY0FBYyxNQUFNLEVBQU4sRUFBVSxLQUFWLEVBQWlCLFVBQVUsQ0FBVixFQUFhO0FBQ2hELGFBQUksTUFBTSxPQUFWLEVBQW1CO0FBQ2pCLGlCQUFNLE9BQU4sQ0FBYyxRQUFkLENBQXVCLEdBQXZCLEVBQTRCLENBQTVCO0FBQ0Q7QUFDRixRQUptQixDQUFwQjtBQUtBLGFBQU0sT0FBTixDQUFjLFFBQWQsQ0FBdUIsR0FBdkIsRUFBNEIsV0FBNUI7QUFDRCxNQVBELE1BUUs7QUFDSCxXQUFJLE1BQU0sT0FBVixFQUFtQjtBQUNqQixlQUFNLE9BQU4sQ0FBYyxRQUFkLENBQXVCLEdBQXZCLEVBQTRCLEtBQTVCO0FBQ0Q7QUFDRjtBQWZtQzs7QUFDdEMsUUFBSyxJQUFNLEdBQVgsSUFBa0IsTUFBbEIsRUFBMEI7QUFBQSxZQUFmLEdBQWU7QUFlekI7QUFDRjs7QUFFRDs7O0FBR0EsVUFBUyxlQUFULENBQTBCLE1BQTFCLEVBQWtDLEVBQWxDLEVBQXNDLEtBQXRDLEVBQTZDO0FBQzNDLE9BQU0sTUFBTSxHQUFHLFFBQUgsSUFBZSxHQUFHLFFBQUgsQ0FBWSxLQUEzQixJQUFvQyxFQUFoRDs7QUFFQTtBQUNBLE9BQUksQ0FBQyxNQUFNLE9BQVgsRUFBb0I7QUFDbEI7QUFDRDs7QUFFRCxPQUFJLE9BQU8sTUFBUCxLQUFrQixVQUF0QixFQUFrQztBQUNoQyxTQUFNLFNBQVEsTUFBTSxFQUFOLEVBQVUsTUFBVixFQUFrQixhQUFLO0FBQ25DLHFCQUFjLE1BQU0sT0FBcEIsRUFBNkIsR0FBN0IsRUFBa0MsQ0FBbEM7QUFDRCxNQUZhLENBQWQ7QUFHQSxtQkFBYyxNQUFNLE9BQXBCLEVBQTZCLEdBQTdCLEVBQWtDLE1BQWxDO0FBQ0QsSUFMRCxNQU1LLElBQUksVUFBVSxJQUFkLEVBQW9CO0FBQ3ZCLG1CQUFjLE1BQU0sT0FBcEIsRUFBNkIsR0FBN0IsRUFBa0MsTUFBbEM7QUFDRDtBQUNGOztBQUVEOzs7O0FBSU8sVUFBUyxLQUFULENBQWdCLEVBQWhCLEVBQW9CLEVBQXBCLEVBQXdCLEVBQXhCLEVBQTRCLE1BQTVCLEVBQW9DO0FBQ3pDLE9BQU0sTUFBTSxPQUFPLE1BQVAsQ0FBYyxJQUFkLENBQVo7O0FBRUEsVUFBTyxnQkFBUCxDQUF3QixHQUF4QixFQUE2QjtBQUMzQixTQUFJO0FBQ0YsY0FBTyxNQURMO0FBRUYsaUJBQVUsS0FGUjtBQUdGLHFCQUFjO0FBSFosTUFEdUI7QUFNM0IsU0FBSTtBQUNGLFlBQUs7QUFBQSxnQkFBTSxNQUFNLE9BQU8sT0FBbkI7QUFBQSxRQURIO0FBRUYscUJBQWM7QUFGWjtBQU51QixJQUE3Qjs7QUFZQSxPQUFJLE9BQU8sRUFBUCxLQUFjLFVBQWxCLEVBQThCO0FBQzVCLFNBQU0sVUFBVSxFQUFoQjtBQUNBLFVBQUssUUFBUSxJQUFSLENBQWEsRUFBYixDQUFMO0FBQ0EsU0FBSSxFQUFKLEVBQVE7QUFDTixVQUFHLElBQUgsQ0FBUSxFQUFSLElBQWMsR0FBZDtBQUNEO0FBQ0QsV0FBTSxFQUFOLEVBQVUsT0FBVixFQUFtQixVQUFDLEtBQUQsRUFBVztBQUM1QixXQUFJLEtBQUosRUFBVztBQUNULFlBQUcsSUFBSCxDQUFRLEtBQVIsSUFBaUIsR0FBakI7QUFDRDtBQUNGLE1BSkQ7QUFLRCxJQVhELE1BWUssSUFBSSxNQUFNLE9BQU8sRUFBUCxLQUFjLFFBQXhCLEVBQWtDO0FBQ3JDLFFBQUcsSUFBSCxDQUFRLEVBQVIsSUFBYyxHQUFkO0FBQ0Q7QUFDRjs7QUFFRDs7O0FBR0EsVUFBUyxPQUFULENBQWtCLEVBQWxCLEVBQXNCLEVBQXRCLEVBQTBCLElBQTFCLEVBQWdDO0FBQzlCLFdBQVEsRUFBUixFQUFZLEVBQVosRUFBZ0IsTUFBaEIsRUFBd0IsSUFBeEI7QUFDRDs7QUFFRCxVQUFTLGFBQVQsQ0FBd0IsRUFBeEIsRUFBNEIsR0FBNUIsRUFBaUMsU0FBakMsRUFBNEM7QUFDMUMsT0FBTSxhQUFhLEVBQW5CO0FBQ0EsT0FBTSxTQUFTLFVBQVUsTUFBekI7O0FBRUEsUUFBSyxJQUFJLElBQUksQ0FBYixFQUFnQixJQUFJLE1BQXBCLEVBQTRCLEdBQTVCLEVBQWlDO0FBQy9CLFNBQU0sUUFBUSxJQUFJLFVBQVUsQ0FBVixDQUFKLENBQWQ7QUFDQSxTQUFJLEtBQUosRUFBVztBQUNULFlBQUssSUFBTSxHQUFYLElBQWtCLEtBQWxCLEVBQXlCO0FBQ3ZCLG9CQUFXLEdBQVgsSUFBa0IsTUFBTSxHQUFOLENBQWxCO0FBQ0Q7QUFDRjtBQUNGO0FBQ0QsTUFBRyxhQUFILENBQWlCLFVBQWpCO0FBQ0Q7O0FBRUQ7OztBQUdBLFVBQVMsUUFBVCxDQUFtQixFQUFuQixFQUF1QixFQUF2QixFQUEyQixTQUEzQixFQUFzQztBQUNwQyxPQUFJLE9BQU8sU0FBUCxLQUFxQixVQUFyQixJQUFtQyxDQUFDLE1BQU0sT0FBTixDQUFjLFNBQWQsQ0FBeEMsRUFBa0U7QUFDaEU7QUFDRDtBQUNELE9BQUksTUFBTSxPQUFOLENBQWMsU0FBZCxLQUE0QixDQUFDLFVBQVUsTUFBM0MsRUFBbUQ7QUFDakQsUUFBRyxhQUFILENBQWlCLEVBQWpCO0FBQ0E7QUFDRDs7QUFFRCxPQUFNLFFBQVEsR0FBRyxRQUFILElBQWUsR0FBRyxRQUFILENBQVksS0FBM0IsSUFBb0MsRUFBbEQ7QUFDQSxPQUFJLE9BQU8sU0FBUCxLQUFxQixVQUF6QixFQUFxQztBQUNuQyxTQUFNLFVBQVEsTUFBTSxFQUFOLEVBQVUsU0FBVixFQUFxQixhQUFLO0FBQ3RDLHFCQUFjLEVBQWQsRUFBa0IsS0FBbEIsRUFBeUIsQ0FBekI7QUFDRCxNQUZhLENBQWQ7QUFHQSxtQkFBYyxFQUFkLEVBQWtCLEtBQWxCLEVBQXlCLE9BQXpCO0FBQ0QsSUFMRCxNQU1LO0FBQ0gsbUJBQWMsRUFBZCxFQUFrQixLQUFsQixFQUF5QixTQUF6QjtBQUNEO0FBQ0Y7O0FBRUQ7OztBQUdBLFVBQVMsUUFBVCxDQUFtQixFQUFuQixFQUF1QixFQUF2QixFQUEyQixLQUEzQixFQUFrQztBQUNoQyxXQUFRLEVBQVIsRUFBWSxFQUFaLEVBQWdCLE9BQWhCLEVBQXlCLEtBQXpCO0FBQ0Q7O0FBRUQ7OztBQUdBLFVBQVMsUUFBVCxDQUFtQixFQUFuQixFQUF1QixFQUF2QixFQUEyQixJQUEzQixFQUFpQyxPQUFqQyxFQUEwQztBQUN4QyxNQUFHLFFBQUgsQ0FBWSxJQUFaLEVBQWtCLGdCQUFLLE9BQUwsRUFBYyxFQUFkLENBQWxCO0FBQ0Q7O0FBRUQ7OztBQUdBLFVBQVMsVUFBVCxDQUFxQixFQUFyQixFQUF5QixFQUF6QixFQUE2QixNQUE3QixFQUFxQztBQUNuQyxPQUFJLENBQUMsTUFBTCxFQUFhO0FBQ1g7QUFDRDtBQUNELE9BQU0sT0FBTyxPQUFPLElBQVAsQ0FBWSxNQUFaLENBQWI7QUFDQSxPQUFJLElBQUksS0FBSyxNQUFiO0FBQ0EsVUFBTyxHQUFQLEVBQVk7QUFDVixTQUFNLE1BQU0sS0FBSyxDQUFMLENBQVo7QUFDQSxTQUFJLFVBQVUsT0FBTyxHQUFQLENBQWQ7QUFDQSxTQUFJLE9BQU8sT0FBUCxLQUFtQixRQUF2QixFQUFpQztBQUMvQixpQkFBVSxHQUFHLE9BQUgsQ0FBVjtBQUNBO0FBQ0EsV0FBSSxDQUFDLE9BQUwsRUFBYztBQUNaLGlCQUFRLEtBQVIsaUNBQTRDLE9BQTVDO0FBQ0Q7QUFDRjtBQUNELGNBQVMsRUFBVCxFQUFhLEVBQWIsRUFBaUIsR0FBakIsRUFBc0IsT0FBdEI7QUFDRDtBQUNGOztBQUVEOzs7OztBQUtBLFVBQVMsT0FBVCxDQUFrQixFQUFsQixFQUFzQixFQUF0QixFQUEwQixJQUExQixFQUFnQyxJQUFoQyxFQUFzQztBQUNwQyxPQUFJLENBQUMsSUFBTCxFQUFXO0FBQ1Q7QUFDRDtBQUNELE9BQU0sT0FBTyxPQUFPLElBQVAsQ0FBWSxJQUFaLENBQWI7QUFDQSxPQUFJLElBQUksS0FBSyxNQUFiO0FBQ0EsVUFBTyxHQUFQLEVBQVk7QUFDVixTQUFNLE1BQU0sS0FBSyxDQUFMLENBQVo7QUFDQSxTQUFNLFVBQVEsS0FBSyxHQUFMLENBQWQ7QUFDQSxTQUFJLE9BQU8sT0FBUCxLQUFpQixVQUFyQixFQUFpQztBQUMvQixlQUFRLEVBQVIsRUFBWSxFQUFaLEVBQWdCLElBQWhCLEVBQXNCLEdBQXRCLEVBQTJCLE9BQTNCO0FBQ0QsTUFGRCxNQUdLO0FBQ0gsVUFBRyxRQUFRLElBQVIsQ0FBSCxFQUFrQixHQUFsQixFQUF1QixPQUF2QjtBQUNEO0FBQ0Y7QUFDRjs7QUFFRDs7O0FBR0EsVUFBUyxPQUFULENBQWtCLEVBQWxCLEVBQXNCLEVBQXRCLEVBQTBCLElBQTFCLEVBQWdDLEdBQWhDLEVBQXFDLElBQXJDLEVBQTJDO0FBQ3pDLE9BQU0sYUFBYSxRQUFRLElBQVIsQ0FBbkI7QUFDQTtBQUNBLE9BQU0sUUFBUSxNQUFNLEVBQU4sRUFBVSxJQUFWLEVBQWdCLFVBQUMsS0FBRCxFQUFXO0FBQ3ZDLGNBQVMsT0FBVCxHQUFvQjtBQUNsQixVQUFHLFVBQUgsRUFBZSxHQUFmLEVBQW9CLEtBQXBCO0FBQ0Q7QUFDRCxTQUFNLFNBQVMsTUFBTSxHQUFHLElBQVQsSUFBaUIsR0FBRyxJQUFILENBQVEsTUFBeEM7QUFDQSxTQUFJLE1BQUosRUFBWTtBQUNWLGNBQU8sTUFBUCxDQUFjLFNBQWQsRUFBeUIsR0FBRyxLQUE1QixFQUFtQyxHQUFHLEdBQXRDLEVBQTJDLE9BQTNDO0FBQ0QsTUFGRCxNQUdLO0FBQ0g7QUFDRDtBQUNGLElBWGEsQ0FBZDs7QUFhQSxNQUFHLFVBQUgsRUFBZSxHQUFmLEVBQW9CLEtBQXBCO0FBQ0Q7O0FBRUQ7OztBQUdPLFVBQVMsS0FBVCxDQUFnQixFQUFoQixFQUFvQixJQUFwQixFQUEwQixRQUExQixFQUFvQztBQUN6QyxPQUFNLFVBQVUsc0JBQVksRUFBWixFQUFnQixJQUFoQixFQUFzQixVQUFVLEtBQVYsRUFBaUIsUUFBakIsRUFBMkI7QUFDL0Q7QUFDQSxTQUFJLFFBQU8sS0FBUCx5Q0FBTyxLQUFQLE9BQWlCLFFBQWpCLElBQTZCLFVBQVUsUUFBM0MsRUFBcUQ7QUFDbkQ7QUFDRDtBQUNELGNBQVMsS0FBVDtBQUNELElBTmUsQ0FBaEI7O0FBUUEsVUFBTyxRQUFRLEtBQWY7QUFDRCxFOzs7Ozs7Ozs7Ozs7OztTQzVUZSxVLEdBQUEsVTtTQVdBLGEsR0FBQSxhO1NBV0EsVyxHQUFBLFc7U0F5REEsWSxHQUFBLFk7U0F1Q0EsVSxHQUFBLFU7U0F3REEsWSxHQUFBLFk7QUE1TGhCOzs7Ozs7OztBQVFBOzs7Ozs7QUFNTyxVQUFTLFVBQVQsQ0FBcUIsRUFBckIsRUFBeUIsSUFBekIsRUFBK0I7QUFDcEMsT0FBTSxNQUFNLEdBQUcsSUFBSCxDQUFRLEdBQXBCO0FBQ0EsVUFBTyxJQUFJLFVBQUosQ0FBZSxJQUFmLENBQVA7QUFDRDs7QUFFRDs7Ozs7O0FBTU8sVUFBUyxhQUFULENBQXdCLEVBQXhCLEVBQTRCLElBQTVCLEVBQWtDO0FBQ3ZDLE9BQU0sTUFBTSxHQUFHLElBQUgsQ0FBUSxHQUFwQjtBQUNBLFVBQU8sSUFBSSxhQUFKLENBQWtCLElBQWxCLENBQVA7QUFDRDs7QUFFRDs7Ozs7O0FBTU8sVUFBUyxXQUFULENBQXNCLEVBQXRCLEVBQTBCLE9BQTFCLEVBQW1DO0FBQ3hDLE9BQU0sUUFBUSxpQkFBaUIsRUFBakIsQ0FBZDtBQUNBLE9BQU0sTUFBTSxlQUFlLEVBQWYsQ0FBWjtBQUNBLE9BQU0sVUFBVSxnQkFBaEI7QUFDQSxPQUFJLFFBQVEsT0FBWixFQUFxQjtBQUNuQixTQUFJLGFBQWEsUUFBUSxVQUF6QjtBQUNBLFNBQUksVUFBSixFQUFnQjtBQUNkLFdBQUksV0FBVyxPQUFmLEVBQXdCO0FBQ3RCLHNCQUFhLFdBQVcsR0FBeEI7QUFDRDtBQUNELGVBQVEsT0FBUixDQUFnQixXQUFoQixDQUE0QixHQUE1QixFQUFpQyxVQUFqQztBQUNBLGVBQVEsT0FBUixDQUFnQixXQUFoQixDQUE0QixLQUE1QixFQUFtQyxVQUFuQztBQUNBLGVBQVEsVUFBUixHQUFxQixHQUFyQjtBQUNELE1BUEQsTUFRSztBQUNILGVBQVEsT0FBUixDQUFnQixZQUFoQixDQUE2QixLQUE3QixFQUFvQyxRQUFRLEdBQTVDO0FBQ0EsZUFBUSxPQUFSLENBQWdCLFlBQWhCLENBQTZCLEdBQTdCLEVBQWtDLFFBQVEsR0FBMUM7QUFDRDtBQUNELGVBQVUsUUFBUSxPQUFsQjtBQUNELElBZkQsTUFnQks7QUFDSCxhQUFRLFdBQVIsQ0FBb0IsS0FBcEI7QUFDQSxhQUFRLFdBQVIsQ0FBb0IsR0FBcEI7QUFDRDtBQUNELFVBQU8sRUFBRSxZQUFGLEVBQVMsUUFBVCxFQUFjLGdCQUFkLEVBQXVCLGdCQUF2QixFQUFQO0FBQ0Q7O0FBRUQsS0FBSSxpQkFBaUIsQ0FBckI7O0FBRUE7Ozs7QUFJQSxVQUFTLGdCQUFULENBQTJCLEVBQTNCLEVBQStCO0FBQzdCLE9BQU0sTUFBTSxHQUFHLElBQUgsQ0FBUSxHQUFwQjtBQUNBLE9BQU0sU0FBUyxJQUFJLGFBQUosQ0FBa0IsT0FBbEIsQ0FBZjtBQUNBLFVBQU8sTUFBUDtBQUNEOztBQUVEOzs7O0FBSUEsVUFBUyxjQUFULENBQXlCLEVBQXpCLEVBQTZCO0FBQzNCLE9BQU0sTUFBTSxHQUFHLElBQUgsQ0FBUSxHQUFwQjtBQUNBLE9BQU0sU0FBUyxJQUFJLGFBQUosQ0FBa0IsS0FBbEIsQ0FBZjtBQUNBLFVBQU8sTUFBUDtBQUNEOztBQUVEOzs7Ozs7OztBQVFPLFVBQVMsWUFBVCxDQUF1QixFQUF2QixFQUEyQixNQUEzQixFQUFtQyxJQUFuQyxFQUF5QztBQUM5QyxPQUFJLEtBQUssT0FBVCxFQUFrQjtBQUNoQixTQUFNLFNBQVMsS0FBSyxHQUFwQjtBQUNBLFNBQU0sUUFBUSxLQUFLLFVBQW5CO0FBQ0E7QUFDQSxTQUFJLEtBQUssUUFBVCxFQUFtQjtBQUNqQixZQUFLLFFBQUwsQ0FBYyxJQUFkLENBQW1CLE1BQW5CO0FBQ0Q7QUFDRDtBQUNBLFNBQUksS0FBSixFQUFXO0FBQ1QsV0FBTSxTQUFTLFdBQVcsRUFBWCxFQUFlLE1BQWYsRUFBdUIsS0FBdkIsQ0FBZjtBQUNBLFlBQUssVUFBTCxHQUFrQixPQUFPLE9BQVAsR0FBaUIsT0FBTyxHQUF4QixHQUE4QixNQUFoRDtBQUNBLGNBQU8sTUFBUDtBQUNELE1BSkQsTUFLSyxJQUFJLE9BQU8sT0FBWCxFQUFvQjtBQUN2QixZQUFLLE9BQUwsQ0FBYSxZQUFiLENBQTBCLE9BQU8sS0FBakMsRUFBd0MsTUFBeEM7QUFDQSxZQUFLLE9BQUwsQ0FBYSxZQUFiLENBQTBCLE9BQU8sR0FBakMsRUFBc0MsTUFBdEM7QUFDRCxNQUhJLE1BSUE7QUFDSCxjQUFPLEtBQUssT0FBTCxDQUFhLFlBQWIsQ0FBMEIsTUFBMUIsRUFBa0MsTUFBbEMsQ0FBUDtBQUNEO0FBQ0YsSUFwQkQsTUFxQks7QUFDSCxTQUFJLE9BQU8sT0FBWCxFQUFvQjtBQUNsQixZQUFLLFdBQUwsQ0FBaUIsT0FBTyxLQUF4QjtBQUNBLFlBQUssV0FBTCxDQUFpQixPQUFPLEdBQXhCO0FBQ0QsTUFIRCxNQUlLO0FBQ0gsY0FBTyxLQUFLLFdBQUwsQ0FBaUIsTUFBakIsQ0FBUDtBQUNEO0FBQ0Y7QUFDRjs7QUFFRDs7Ozs7O0FBTU8sVUFBUyxVQUFULENBQXFCLEVBQXJCLEVBQXlCLE1BQXpCLEVBQWlDLEtBQWpDLEVBQXdDO0FBQzdDLE9BQUksT0FBTyxPQUFYLEVBQW9CO0FBQ2xCLFlBQU8sVUFBVSxNQUFWLEVBQWtCLEtBQWxCLENBQVA7QUFDRDtBQUNELFVBQU8sWUFBWSxNQUFaLEVBQW9CLEtBQXBCLENBQVA7QUFDRDs7QUFFRDs7Ozs7O0FBTUEsVUFBUyxXQUFULENBQXNCLE9BQXRCLEVBQStCLEtBQS9CLEVBQXNDO0FBQ3BDLE9BQU0sU0FBUyxNQUFNLFVBQXJCO0FBQ0EsT0FBSSxNQUFKLEVBQVk7QUFDVixZQUFPLE9BQU8sV0FBUCxDQUFtQixPQUFuQixFQUE0QixLQUE1QixDQUFQO0FBQ0Q7QUFDRjs7QUFFRDs7Ozs7O0FBTUEsVUFBUyxTQUFULENBQW9CLFNBQXBCLEVBQStCLEtBQS9CLEVBQXNDO0FBQ3BDLE9BQU0sU0FBUyxNQUFNLFVBQXJCOztBQUVBLE9BQUksTUFBSixFQUFZO0FBQUE7QUFDVixXQUFJLEtBQUssVUFBVSxLQUFuQjtBQUNBLFdBQUksZUFBSjtBQUNBLFdBQU0sUUFBUSxDQUFDLEVBQUQsQ0FBZDs7QUFFQSxjQUFPLE1BQU0sT0FBTyxVQUFVLEdBQTlCLEVBQW1DO0FBQ2pDLGNBQUssR0FBRyxXQUFSO0FBQ0EsZUFBTSxJQUFOLENBQVcsRUFBWDtBQUNEOztBQUVELFdBQUksT0FBTyxLQUFYO0FBQ0EsYUFBTSxLQUFOLENBQVksVUFBQyxFQUFELEVBQVE7QUFDbEIsa0JBQVMsT0FBTyxXQUFQLENBQW1CLEVBQW5CLEVBQXVCLElBQXZCLENBQVQ7QUFDQSxnQkFBTyxFQUFQO0FBQ0EsZ0JBQU8sV0FBVyxDQUFDLENBQW5CO0FBQ0QsUUFKRDs7QUFNQTtBQUFBLFlBQU87QUFBUDtBQWpCVTs7QUFBQTtBQWtCWDtBQUNGOztBQUVEOzs7Ozs7QUFNTyxVQUFTLFlBQVQsQ0FBdUIsRUFBdkIsRUFBMkIsTUFBM0IsRUFBMEQ7QUFBQSxPQUF2QixhQUF1Qix5REFBUCxLQUFPOztBQUMvRCxPQUFJLE9BQU8sT0FBWCxFQUFvQjtBQUNsQixpQkFBWSxNQUFaLEVBQW9CLGFBQXBCO0FBQ0QsSUFGRCxNQUdLO0FBQ0gsbUJBQWMsTUFBZDtBQUNEO0FBQ0Y7O0FBRUQ7Ozs7OztBQU1BLFVBQVMsYUFBVCxDQUF3QixNQUF4QixFQUFnQztBQUM5QixPQUFNLFNBQVMsT0FBTyxVQUF0Qjs7QUFFQSxPQUFJLE1BQUosRUFBWTtBQUNWLFlBQU8sV0FBUCxDQUFtQixNQUFuQjtBQUNEO0FBQ0Y7O0FBRUQ7Ozs7Ozs7QUFPQSxVQUFTLFdBQVQsQ0FBc0IsU0FBdEIsRUFBd0Q7QUFBQSxPQUF2QixhQUF1Qix5REFBUCxLQUFPOztBQUN0RCxPQUFNLFNBQVMsRUFBZjtBQUNBLE9BQUksS0FBSyxVQUFVLEtBQVYsQ0FBZ0IsV0FBekI7O0FBRUEsVUFBTyxNQUFNLE9BQU8sVUFBVSxHQUE5QixFQUFtQztBQUNqQyxZQUFPLElBQVAsQ0FBWSxFQUFaO0FBQ0EsVUFBSyxHQUFHLFdBQVI7QUFDRDs7QUFFRCxPQUFJLENBQUMsYUFBTCxFQUFvQjtBQUNsQixtQkFBYyxVQUFVLEtBQXhCO0FBQ0Q7QUFDRCxVQUFPLE9BQVAsQ0FBZSxVQUFDLEVBQUQsRUFBUTtBQUNyQixtQkFBYyxFQUFkO0FBQ0QsSUFGRDtBQUdBLE9BQUksQ0FBQyxhQUFMLEVBQW9CO0FBQ2xCLG1CQUFjLFVBQVUsR0FBeEI7QUFDRDtBQUNGLEU7Ozs7Ozs7Ozs7O1NDaE1lLEssR0FBQSxLO1NBZ0JBLFMsR0FBQSxTO1NBY0EsVSxHQUFBLFU7U0FnQkEsRyxHQUFBLEc7U0FvQkEsSSxHQUFBLEk7U0F5QkEsVSxHQUFBLFU7U0FrQkEsVyxHQUFBLFc7QUF6SmhCOzs7Ozs7QUFNQTs7Ozs7OztBQU9BLFVBQVMsR0FBVCxDQUFjLElBQWQsRUFBb0IsTUFBcEIsRUFBNEI7QUFDMUIsT0FBSSxrQkFBa0IsR0FBdEIsRUFBMkI7QUFDekIsWUFBTyxNQUFQO0FBQ0Q7O0FBRUQsUUFBSyxTQUFMLEdBQWlCLEtBQUssR0FBTCxFQUFqQjtBQUNBLFFBQUssTUFBTCxHQUFjLE1BQWQ7QUFDQSxRQUFLLElBQUwsR0FBWSxJQUFaOztBQUVBLE9BQUksYUFBYSxLQUFqQjs7QUFFQTs7O0FBR0EsUUFBSyxJQUFMLEdBQVksWUFBWTtBQUN0QixrQkFBYSxJQUFiO0FBQ0QsSUFGRDs7QUFJQTs7O0FBR0EsUUFBSyxVQUFMLEdBQWtCLFlBQVk7QUFDNUIsWUFBTyxVQUFQO0FBQ0QsSUFGRDtBQUdEOztBQUVEOzs7OztBQUtPLFVBQVMsS0FBVCxDQUFnQixJQUFoQixFQUFzQixNQUF0QixFQUE4QjtBQUFBOztBQUNuQyxPQUFNLFNBQVMsS0FBSyxTQUFwQjtBQUNBLE9BQU0sY0FBYyxPQUFPLElBQVAsQ0FBcEI7QUFDQSxPQUFJLFdBQUosRUFBaUI7QUFBQTtBQUNmLFdBQU0sTUFBTSxJQUFJLEdBQUosQ0FBUSxJQUFSLEVBQWMsTUFBZCxDQUFaO0FBQ0EsbUJBQVksT0FBWixDQUFvQixVQUFDLE9BQUQsRUFBYTtBQUMvQixpQkFBUSxJQUFSLFFBQW1CLEdBQW5CO0FBQ0QsUUFGRDtBQUZlO0FBS2hCO0FBQ0Y7O0FBRUQ7Ozs7O0FBS08sVUFBUyxTQUFULENBQW9CLElBQXBCLEVBQTBCLE1BQTFCLEVBQWtDO0FBQ3ZDLE9BQU0sTUFBTSxJQUFJLEdBQUosQ0FBUSxJQUFSLEVBQWMsTUFBZCxDQUFaO0FBQ0EsUUFBSyxLQUFMLENBQVcsSUFBWCxFQUFpQixHQUFqQjs7QUFFQSxPQUFJLENBQUMsSUFBSSxVQUFKLEVBQUQsSUFBcUIsS0FBSyxPQUExQixJQUFxQyxLQUFLLE9BQUwsQ0FBYSxTQUF0RCxFQUFpRTtBQUMvRCxVQUFLLE9BQUwsQ0FBYSxTQUFiLENBQXVCLElBQXZCLEVBQTZCLEdBQTdCO0FBQ0Q7QUFDRjs7QUFFRDs7Ozs7QUFLTyxVQUFTLFVBQVQsQ0FBcUIsSUFBckIsRUFBMkIsTUFBM0IsRUFBbUM7QUFDeEMsT0FBTSxNQUFNLElBQUksR0FBSixDQUFRLElBQVIsRUFBYyxNQUFkLENBQVo7QUFDQSxRQUFLLEtBQUwsQ0FBVyxJQUFYLEVBQWlCLEdBQWpCOztBQUVBLE9BQUksQ0FBQyxJQUFJLFVBQUosRUFBRCxJQUFxQixLQUFLLFlBQTlCLEVBQTRDO0FBQzFDLFVBQUssWUFBTCxDQUFrQixPQUFsQixDQUEwQixVQUFDLEtBQUQsRUFBVztBQUNuQyxhQUFNLFVBQU4sQ0FBaUIsSUFBakIsRUFBdUIsR0FBdkI7QUFDRCxNQUZEO0FBR0Q7QUFDRjs7QUFFRDs7Ozs7QUFLTyxVQUFTLEdBQVQsQ0FBYyxJQUFkLEVBQW9CLE9BQXBCLEVBQTZCO0FBQ2xDLE9BQUksQ0FBQyxJQUFELElBQVMsT0FBTyxPQUFQLEtBQW1CLFVBQWhDLEVBQTRDO0FBQzFDO0FBQ0Q7QUFDRCxPQUFNLFNBQVMsS0FBSyxTQUFwQjtBQUNBLE9BQU0sY0FBYyxPQUFPLElBQVAsS0FBZ0IsRUFBcEM7QUFDQSxlQUFZLElBQVosQ0FBaUIsT0FBakI7QUFDQSxVQUFPLElBQVAsSUFBZSxXQUFmOztBQUVBO0FBQ0EsT0FBSSxTQUFTLFlBQVQsSUFBeUIsS0FBSyxNQUFsQyxFQUEwQztBQUN4QyxVQUFLLEtBQUwsQ0FBVyxZQUFYO0FBQ0Q7QUFDRjs7QUFFRDs7Ozs7QUFLTyxVQUFTLElBQVQsQ0FBZSxJQUFmLEVBQXFCLE9BQXJCLEVBQThCO0FBQ25DLE9BQUksQ0FBQyxJQUFMLEVBQVc7QUFDVDtBQUNEO0FBQ0QsT0FBTSxTQUFTLEtBQUssU0FBcEI7QUFDQSxPQUFJLENBQUMsT0FBTCxFQUFjO0FBQ1osWUFBTyxPQUFPLElBQVAsQ0FBUDtBQUNBO0FBQ0Q7QUFDRCxPQUFNLGNBQWMsT0FBTyxJQUFQLENBQXBCO0FBQ0EsT0FBSSxDQUFDLFdBQUwsRUFBa0I7QUFDaEI7QUFDRDtBQUNELGVBQVksT0FBWixDQUFvQixPQUFwQjtBQUNEOztBQUVELEtBQU0sbUJBQW1CLENBQUMsTUFBRCxFQUFTLFNBQVQsRUFBb0IsT0FBcEIsQ0FBekI7O0FBRUE7Ozs7Ozs7QUFPTyxVQUFTLFVBQVQsQ0FBcUIsRUFBckIsRUFBeUIsY0FBekIsRUFBeUM7QUFDOUMsT0FBTSxVQUFVLEdBQUcsUUFBSCxJQUFlLEVBQS9CO0FBQ0EsT0FBTSxTQUFTLFFBQVEsTUFBUixJQUFrQixFQUFqQztBQUNBLFFBQUssSUFBTSxLQUFYLElBQW9CLE1BQXBCLEVBQTRCO0FBQzFCLFFBQUcsR0FBSCxDQUFPLEtBQVAsRUFBYyxPQUFPLEtBQVAsQ0FBZDtBQUNEO0FBQ0QsUUFBSyxJQUFNLEtBQVgsSUFBb0IsY0FBcEIsRUFBb0M7QUFDbEMsUUFBRyxHQUFILENBQU8sS0FBUCxFQUFjLGVBQWUsS0FBZixDQUFkO0FBQ0Q7QUFDRCxvQkFBaUIsT0FBakIsQ0FBeUIsVUFBQyxJQUFELEVBQVU7QUFDakMsUUFBRyxHQUFILFdBQWUsSUFBZixFQUF1QixRQUFRLElBQVIsQ0FBdkI7QUFDRCxJQUZEO0FBR0Q7O0FBRUQ7Ozs7QUFJTyxVQUFTLFdBQVQsQ0FBc0IsRUFBdEIsRUFBMEI7QUFDL0IsTUFBRyxLQUFILEdBQVcsS0FBWDtBQUNBLE1BQUcsU0FBSCxHQUFlLFNBQWY7QUFDQSxNQUFHLFVBQUgsR0FBZ0IsVUFBaEI7QUFDQSxNQUFHLEdBQUgsR0FBUyxHQUFUO0FBQ0EsTUFBRyxJQUFILEdBQVUsSUFBVjtBQUNELEU7Ozs7Ozs7Ozs7O1NDdkplLGdCLEdBQUEsZ0I7U0ErQkEsUSxHQUFBLFE7U0E2REEsSyxHQUFBLEs7O0FBcEdoQjs7OztBQUNBOzs7O0FBRUE7Ozs7O0FBS08sVUFBUyxnQkFBVCxDQUEyQixDQUEzQixFQUE4QjtBQUNuQyxPQUFNLFVBQVUsaUJBQU8sS0FBUCxDQUFhLENBQWIsQ0FBaEI7QUFDQSxPQUFJLE9BQUosRUFBYTtBQUNYLFlBQU8sQ0FBUDtBQUNEOztBQUVELE9BQUksT0FBUSxDQUFSLEtBQWUsUUFBZixHQUEwQixDQUExQixHQUE4QixFQUFsQztBQUNBLE9BQU0sUUFBUSxFQUFFLEtBQUYsQ0FBUSxHQUFSLENBQWQ7QUFDQSxPQUFJLElBQUksQ0FBUjtBQUNBLE9BQU0sU0FBUyxFQUFmOztBQUVBLFVBQU8sSUFBSSxDQUFYLEVBQWM7QUFDWixTQUFNLElBQUksT0FBUSxNQUFNLENBQU4sQ0FBUixLQUFzQixRQUF0QixJQUFrQyxNQUFNLENBQU4sQ0FBbEMsR0FBNkMsTUFBTSxDQUFOLENBQTdDLEdBQXdELEdBQWxFO0FBQ0EsWUFBTyxJQUFQLENBQVksQ0FBWjtBQUNBO0FBQ0Q7O0FBRUQsVUFBTyxPQUFPLElBQVAsQ0FBWSxHQUFaLENBQVA7QUFDRDs7QUFFRDs7Ozs7Ozs7Ozs7QUFXTyxVQUFTLFFBQVQsQ0FBbUIsR0FBbkIsRUFBd0IsR0FBeEIsRUFBNkIsUUFBN0IsRUFBdUM7QUFDNUMsT0FBTSxTQUFTO0FBQ2Isa0JBQWEsSUFEQTtBQUViLGdCQUFXLENBRkU7QUFHYixXQUFNO0FBSE8sSUFBZjtBQUtBLE9BQU0sU0FBUyxTQUFULE1BQVMsQ0FBVSxHQUFWLEVBQWUsR0FBZixFQUFvQixRQUFwQixFQUE4QjtBQUMzQyxZQUFPLGVBQWUsR0FBZixHQUFxQixrQkFBckIsR0FDSCxHQURHLEdBQ0csb0JBREgsR0FDMEIsUUFEakM7QUFFRCxJQUhEO0FBSUEsT0FBTSxPQUFPLElBQUksV0FBSixFQUFiOztBQUVBLFVBQU8sWUFBUCxHQUFzQixPQUFPLEdBQVAsRUFBWSxHQUFaLEVBQWlCLFFBQWpCLENBQXRCOztBQUVBLE9BQUksS0FBSyxPQUFMLENBQWEsV0FBYixLQUE2QixDQUFqQyxFQUFvQztBQUNsQyxZQUFPLElBQVAsR0FBYyxJQUFkO0FBQ0QsSUFGRCxNQUdLLElBQUksS0FBSyxPQUFMLENBQWEsWUFBYixLQUE4QixDQUFsQyxFQUFxQztBQUN4QyxZQUFPLElBQVAsR0FBYyxJQUFkO0FBQ0QsSUFGSSxNQUdBLElBQUksS0FBSyxPQUFMLENBQWEsYUFBYixLQUErQixDQUFuQyxFQUFzQztBQUN6QyxZQUFPLElBQVAsR0FBYyxJQUFkO0FBQ0QsSUFGSSxNQUdBLElBQUksS0FBSyxPQUFMLENBQWEsYUFBYixLQUErQixDQUFuQyxFQUFzQztBQUN6QyxZQUFPLElBQVAsR0FBYyxJQUFkO0FBQ0Q7O0FBRUQsVUFBTyxNQUFQO0FBQ0Q7O0FBRUQ7Ozs7Ozs7Ozs7Ozs7Ozs7Ozs7Ozs7Ozs7Ozs7Ozs7QUErQk8sVUFBUyxLQUFULENBQWdCLE1BQWhCLEVBQXdCLFVBQXhCLEVBQW9DO0FBQ3pDLGdCQUFhLGNBQWMsT0FBTyxhQUFsQztBQUNBLGdCQUFhLHlCQUFjLFVBQWQsSUFBNEIsVUFBNUIsR0FBeUMsRUFBdEQ7O0FBRUEsT0FBSSxTQUFTO0FBQ1gsa0JBQWEsS0FERixDQUNRO0FBRFIsSUFBYjs7QUFJQSxPQUFJLGlCQUFNLE1BQU4sTUFBa0IsVUFBdEIsRUFBa0M7QUFDaEMsU0FBSSxrQkFBa0IsT0FBTyxJQUFQLENBQVksSUFBWixFQUFrQixVQUFsQixFQUE4QjtBQUNsRCwrQkFEa0Q7QUFFbEQseUJBQWtCLEtBQUs7QUFGMkIsTUFBOUIsQ0FBdEI7O0FBS0EsdUJBQWtCLENBQUMsQ0FBQyxlQUFwQjs7QUFFQSxjQUFTLGtCQUFrQixLQUFLLFFBQUwsQ0FBYyxRQUFkLEVBQXdCLEVBQXhCLEVBQTRCLGVBQTVCLENBQWxCLEdBQWlFLE1BQTFFO0FBQ0QsSUFURCxNQVVLO0FBQ0gsY0FBUyx5QkFBYyxNQUFkLElBQXdCLE1BQXhCLEdBQWlDLEVBQTFDOztBQUVBLFNBQU0sV0FBVyxXQUFXLFFBQVgsSUFBdUIsUUFBeEM7QUFDQSxTQUFNLFlBQVksU0FBUyxXQUFULEVBQWxCO0FBQ0EsU0FBTSxPQUFPLE9BQU8sU0FBUCxLQUFxQixFQUFsQzs7QUFFQSxVQUFLLElBQU0sQ0FBWCxJQUFnQixVQUFoQixFQUE0QjtBQUMxQixXQUFNLE1BQU0sQ0FBWjtBQUNBLFdBQU0sV0FBVyxJQUFJLFdBQUosRUFBakI7QUFDQSxXQUFNLE1BQU0sV0FBVyxDQUFYLENBQVo7QUFDQSxXQUFNLFlBQVksU0FBUyxPQUFULENBQWlCLFNBQWpCLEtBQStCLENBQWpEO0FBQ0EsV0FBTSxnQkFBZ0IsU0FBUyxPQUFULENBQWlCLGFBQWpCLEtBQW1DLENBQXpEO0FBQ0EsV0FBTSxXQUFXLEtBQUssQ0FBTCxDQUFqQjs7QUFFQSxXQUFJLFlBQVksU0FBaEIsRUFBMkI7QUFDekIsYUFBTSxJQUFJLEtBQUssZ0JBQUwsQ0FBc0IsUUFBdEIsQ0FBVjtBQUNBLGFBQU0sSUFBSSxLQUFLLGdCQUFMLENBQXNCLFdBQVcsQ0FBWCxDQUF0QixDQUFWOztBQUVBLGFBQUksaUJBQU8sU0FBUCxDQUFpQixDQUFqQixFQUFvQixDQUFwQixDQUFKLEVBQTRCO0FBQzFCLG9CQUFTLEtBQUssUUFBTCxDQUFjLEdBQWQsRUFBbUIsR0FBbkIsRUFBd0IsUUFBeEIsQ0FBVDtBQUNBO0FBQ0Q7QUFDRixRQVJELE1BU0ssSUFBSSxhQUFKLEVBQW1CO0FBQ3RCLGFBQU0sWUFBWSxpQkFBTSxRQUFOLE1BQW9CLE9BQXBCLEdBQThCLFFBQTlCLEdBQXlDLENBQUMsUUFBRCxDQUEzRDtBQUNBLGFBQUksVUFBVSxPQUFWLENBQWtCLEdBQWxCLEtBQTBCLENBQTlCLEVBQWlDO0FBQy9CLG9CQUFTLEtBQUssUUFBTCxDQUFjLEdBQWQsRUFBbUIsR0FBbkIsRUFBd0IsUUFBeEIsQ0FBVDtBQUNBO0FBQ0Q7QUFDRjtBQUNGO0FBQ0Y7O0FBRUQsVUFBTyxNQUFQO0FBQ0QsRTs7Ozs7Ozs7Ozs7OztTQzVJZSxRLEdBQUEsUTtTQXNKQSxJLEdBQUEsSTtTQXFCQSxPLEdBQUEsTztTQTJWQSxPLEdBQUEsTzs7QUE5Z0JoQjs7OztBQUNBOzs7O0FBTkE7Ozs7O0FBUUEsS0FBTSxtQkFBbUIsS0FBekI7O0FBRU8sS0FBTSxvQ0FBYyxFQUFwQjtBQUNQLEtBQUksY0FBYyxDQUFsQjs7QUFFTyxVQUFTLFFBQVQsQ0FBbUIsRUFBbkIsRUFBdUIsR0FBdkIsRUFBNEIsT0FBNUIsRUFBcUM7QUFDMUMsUUFBSyxLQUFLLEdBQUcsUUFBSCxFQUFMLEdBQXFCLEVBQTFCO0FBQ0EsUUFBSyxFQUFMLEdBQVUsRUFBVjtBQUNBLFFBQUssR0FBTCxHQUFXLEdBQVg7O0FBRUEsZUFBWSxFQUFaLElBQWtCLElBQWxCO0FBQ0EsUUFBSyxPQUFMLEdBQWUsRUFBZjtBQUNBLFFBQUssUUFBTCxHQUFnQix1QkFBYSxFQUFiLEVBQWlCLFdBQVcsYUFBYSxFQUFiLENBQTVCLENBQWhCO0FBQ0EsUUFBSyxxQkFBTDtBQUNEOztBQUVELFVBQVMsWUFBVCxDQUF1QixFQUF2QixFQUEyQjtBQUN6QixVQUFPLFVBQUMsS0FBRCxFQUFXO0FBQ2hCLFNBQUksQ0FBQyxNQUFNLE9BQU4sQ0FBYyxLQUFkLENBQUwsRUFBMkI7QUFDekIsZUFBUSxDQUFDLEtBQUQsQ0FBUjtBQUNEO0FBQ0QsWUFBTyxXQUFXLEVBQVgsRUFBZSxLQUFmLEVBQXNCLElBQXRCLENBQVA7QUFDRCxJQUxEO0FBTUQ7O0FBRUQsVUFBUyxTQUFULENBQW1CLE9BQW5CLEdBQTZCLFlBQVk7QUFDdkMsVUFBTyxLQUFLLFFBQVo7QUFDQSxVQUFPLEtBQUssT0FBWjtBQUNBLFVBQU8sWUFBWSxLQUFLLEVBQWpCLENBQVA7QUFDRCxFQUpEOztBQU1BLFVBQVMsU0FBVCxDQUFtQixJQUFuQixHQUEwQixZQUFZO0FBQ3BDLFFBQUssUUFBTCxDQUFjLE9BQWQsR0FBd0IsS0FBeEI7QUFDRCxFQUZEOztBQUlBLFVBQVMsU0FBVCxDQUFtQixLQUFuQixHQUEyQixZQUFZO0FBQ3JDLFFBQUssUUFBTCxDQUFjLE9BQWQsR0FBd0IsSUFBeEI7QUFDRCxFQUZEOztBQUlBLFVBQVMsU0FBVCxDQUFtQixxQkFBbkIsR0FBMkMsWUFBWTtBQUFBOztBQUNyRCxPQUFJLENBQUMsS0FBSyxlQUFWLEVBQTJCO0FBQ3pCLFNBQU0sS0FBSyxJQUFJLE9BQUosQ0FBWSxVQUFaLENBQVg7QUFDQSxRQUFHLEtBQUgsR0FBVyxLQUFLLEVBQWhCO0FBQ0EsUUFBRyxhQUFILEdBQW1CLElBQW5CO0FBQ0EsUUFBRyxJQUFILEdBQVUsaUJBQVY7QUFDQSxRQUFHLEtBQUgsR0FBVyxDQUFYO0FBQ0EsUUFBRyxHQUFILEdBQVMsa0JBQVQ7QUFDQSxVQUFLLE9BQUwsQ0FBYSxnQkFBYixHQUFnQyxFQUFoQztBQUNBLFVBQUssZUFBTCxHQUF1QixFQUF2QjtBQUNBLFFBQUcsV0FBSCxHQUFpQixVQUFDLElBQUQsRUFBVTtBQUN6Qix5QkFBaUIsSUFBakI7QUFDRCxNQUZEO0FBR0EsUUFBRyxZQUFILEdBQWtCLFVBQUMsSUFBRCxFQUFPLE1BQVAsRUFBa0I7QUFDbEMseUJBQWlCLElBQWpCLEVBQXVCLE1BQXZCO0FBQ0QsTUFGRDtBQUdEOztBQUVELFVBQU8sS0FBSyxlQUFaO0FBQ0QsRUFuQkQ7O0FBcUJBLFVBQVMsVUFBVCxDQUFxQixHQUFyQixFQUEwQixJQUExQixFQUFnQyxNQUFoQyxFQUF3QztBQUFBLE9BQzlCLGVBRDhCLEdBQ1YsR0FEVSxDQUM5QixlQUQ4Qjs7O0FBR3RDLE9BQUksZ0JBQWdCLFlBQWhCLENBQTZCLE1BQTdCLEdBQXNDLENBQXRDLElBQTJDLEtBQUssVUFBcEQsRUFBZ0U7QUFDOUQ7QUFDRDtBQUNELE9BQU0sV0FBVyxnQkFBZ0IsUUFBakM7QUFDQSxPQUFNLGNBQWMsU0FBUyxPQUFULENBQWlCLE1BQWpCLENBQXBCO0FBQ0EsT0FBSSxjQUFjLENBQWxCLEVBQXFCO0FBQ25CLGNBQVMsSUFBVCxDQUFjLElBQWQ7QUFDRCxJQUZELE1BR0s7QUFDSCxjQUFTLE1BQVQsQ0FBZ0IsV0FBaEIsRUFBNkIsQ0FBN0IsRUFBZ0MsSUFBaEM7QUFDRDs7QUFFRCxPQUFJLEtBQUssUUFBTCxLQUFrQixDQUF0QixFQUF5QjtBQUN2QixTQUFJLEtBQUssSUFBTCxLQUFjLE1BQWxCLEVBQTBCO0FBQ3hCLFlBQUssS0FBTCxHQUFhLElBQUksRUFBakI7QUFDQSxZQUFLLGFBQUwsR0FBcUIsR0FBckI7QUFDQSxZQUFLLFVBQUwsR0FBa0IsZUFBbEI7QUFDRCxNQUpELE1BS0s7QUFDSCxZQUFLLFFBQUwsQ0FBYyxPQUFkLENBQXNCLGlCQUFTO0FBQzdCLGVBQU0sVUFBTixHQUFtQixJQUFuQjtBQUNELFFBRkQ7QUFHQSxlQUFRLEdBQVIsRUFBYSxJQUFiO0FBQ0EsWUFBSyxLQUFMLEdBQWEsSUFBSSxFQUFqQjtBQUNBLFlBQUssYUFBTCxHQUFxQixHQUFyQjtBQUNBLGtCQUFXLElBQVgsRUFBaUIsZUFBakI7QUFDQSxjQUFPLElBQUksT0FBSixDQUFZLEtBQUssTUFBakIsQ0FBUDtBQUNEO0FBQ0QscUJBQWdCLFlBQWhCLENBQTZCLElBQTdCLENBQWtDLElBQWxDO0FBQ0EsU0FBSSxRQUFKLENBQWEsVUFBYixDQUF3QixJQUF4QjtBQUNELElBbEJELE1BbUJLO0FBQ0gsVUFBSyxVQUFMLEdBQWtCLGVBQWxCO0FBQ0EsU0FBSSxPQUFKLENBQVksS0FBSyxHQUFqQixJQUF3QixJQUF4QjtBQUNEO0FBQ0Y7O0FBRUQsVUFBUyxPQUFULENBQWtCLEdBQWxCLEVBQXVCLEVBQXZCLEVBQTJCO0FBQ3pCLE1BQUcsSUFBSCxHQUFVLE1BQVY7QUFDQSxNQUFHLEtBQUgsR0FBVyxDQUFYO0FBQ0EsVUFBTyxJQUFJLE9BQUosQ0FBWSxHQUFHLE1BQWYsQ0FBUDtBQUNBLE1BQUcsR0FBSCxHQUFTLE9BQVQ7QUFDQSxPQUFJLE9BQUosQ0FBWSxLQUFaLEdBQW9CLEVBQXBCO0FBQ0EsT0FBSSxJQUFKLEdBQVcsRUFBWDtBQUNEOztBQUVELFVBQVMsU0FBVCxDQUFtQixVQUFuQixHQUFnQyxVQUFVLElBQVYsRUFBZ0IsS0FBaEIsRUFBdUI7QUFDckQsT0FBSSxDQUFDLEtBQUssSUFBVixFQUFnQjtBQUNkLFNBQU0sS0FBSyxJQUFJLE9BQUosQ0FBWSxJQUFaLEVBQWtCLEtBQWxCLENBQVg7QUFDQSxhQUFRLElBQVIsRUFBYyxFQUFkO0FBQ0Q7O0FBRUQsVUFBTyxLQUFLLElBQVo7QUFDRCxFQVBEOztBQVNBLFVBQVMsU0FBVCxDQUFtQixhQUFuQixHQUFtQyxVQUFVLE9BQVYsRUFBbUIsS0FBbkIsRUFBMEI7QUFDM0QsVUFBTyxJQUFJLE9BQUosQ0FBWSxPQUFaLEVBQXFCLEtBQXJCLENBQVA7QUFDRCxFQUZEOztBQUlBLFVBQVMsU0FBVCxDQUFtQixhQUFuQixHQUFtQyxVQUFVLElBQVYsRUFBZ0I7QUFDakQsVUFBTyxJQUFJLE9BQUosQ0FBWSxJQUFaLENBQVA7QUFDRCxFQUZEOztBQUlBLFVBQVMsU0FBVCxDQUFtQixTQUFuQixHQUErQixVQUFVLEVBQVYsRUFBYyxJQUFkLEVBQW9CLENBQXBCLEVBQXVCLFVBQXZCLEVBQW1DO0FBQ2hFLE9BQUksQ0FBQyxFQUFMLEVBQVM7QUFDUDtBQUNEO0FBQ0QsT0FBSSxLQUFLLEVBQVQ7QUFDQSxLQUFFLElBQUYsR0FBUyxJQUFUO0FBQ0EsS0FBRSxNQUFGLEdBQVcsRUFBWDtBQUNBLEtBQUUsU0FBRixHQUFjLEtBQUssR0FBTCxFQUFkO0FBQ0EsT0FBSSxVQUFKLEVBQWdCO0FBQ2QsbUJBQWMsRUFBZCxFQUFrQixVQUFsQjtBQUNEO0FBQ0QsVUFBTyxHQUFHLFNBQUgsQ0FBYSxJQUFiLEVBQW1CLENBQW5CLENBQVA7QUFDRCxFQVpEOztBQWNBLFVBQVMsU0FBVCxDQUFtQixNQUFuQixHQUE0QixVQUFVLEdBQVYsRUFBZTtBQUN6QyxVQUFPLEtBQUssT0FBTCxDQUFhLEdBQWIsQ0FBUDtBQUNELEVBRkQ7O0FBSUEsVUFBUyxhQUFULENBQXdCLEVBQXhCLEVBQTRCLE9BQTVCLEVBQXFDO0FBQ25DLE9BQU0sUUFBUSxRQUFRLEtBQVIsSUFBaUIsRUFBL0I7QUFDQSxRQUFLLElBQU0sSUFBWCxJQUFtQixLQUFuQixFQUEwQjtBQUN4QixRQUFHLE9BQUgsQ0FBVyxJQUFYLEVBQWlCLE1BQU0sSUFBTixDQUFqQixFQUE4QixJQUE5QjtBQUNEO0FBQ0QsT0FBTSxRQUFRLFFBQVEsS0FBUixJQUFpQixFQUEvQjtBQUNBLFFBQUssSUFBTSxLQUFYLElBQW1CLEtBQW5CLEVBQTBCO0FBQ3hCLFFBQUcsUUFBSCxDQUFZLEtBQVosRUFBa0IsTUFBTSxLQUFOLENBQWxCLEVBQStCLElBQS9CO0FBQ0Q7QUFDRjs7QUFFTSxVQUFTLElBQVQsR0FBaUI7QUFDdEIsUUFBSyxNQUFMLEdBQWMsQ0FBQyxhQUFELEVBQWdCLFFBQWhCLEVBQWQ7QUFDQSxRQUFLLEdBQUwsR0FBVyxLQUFLLE1BQWhCO0FBQ0EsUUFBSyxRQUFMLEdBQWdCLEVBQWhCO0FBQ0EsUUFBSyxZQUFMLEdBQW9CLEVBQXBCO0FBQ0EsUUFBSyxVQUFMLEdBQWtCLElBQWxCO0FBQ0EsUUFBSyxXQUFMLEdBQW1CLElBQW5CO0FBQ0EsUUFBSyxlQUFMLEdBQXVCLElBQXZCO0FBQ0Q7O0FBRUQsTUFBSyxTQUFMLENBQWUsT0FBZixHQUF5QixZQUFZO0FBQ25DLE9BQU0sTUFBTSxZQUFZLEtBQUssS0FBakIsQ0FBWjtBQUNBLE9BQUksR0FBSixFQUFTO0FBQ1AsWUFBTyxLQUFLLEtBQVo7QUFDQSxZQUFPLElBQUksT0FBSixDQUFZLEtBQUssTUFBakIsQ0FBUDtBQUNEO0FBQ0QsUUFBSyxRQUFMLENBQWMsT0FBZCxDQUFzQixpQkFBUztBQUM3QixXQUFNLE9BQU47QUFDRCxJQUZEO0FBR0QsRUFURDs7QUFXTyxVQUFTLE9BQVQsR0FBa0Q7QUFBQSxPQUFoQyxJQUFnQyx5REFBekIsZ0JBQXlCO0FBQUEsT0FBUCxLQUFPOztBQUN2RCxXQUFRLFNBQVMsRUFBakI7QUFDQSxRQUFLLFFBQUwsR0FBZ0IsQ0FBaEI7QUFDQSxRQUFLLE1BQUwsR0FBYyxDQUFDLGFBQUQsRUFBZ0IsUUFBaEIsRUFBZDtBQUNBLFFBQUssR0FBTCxHQUFXLEtBQUssTUFBaEI7QUFDQSxRQUFLLElBQUwsR0FBWSxJQUFaO0FBQ0EsUUFBSyxJQUFMLEdBQVksTUFBTSxJQUFOLElBQWMsRUFBMUI7QUFDQSxRQUFLLFVBQUwsR0FBa0IsTUFBTSxVQUFOLElBQW9CLEVBQXRDO0FBQ0EsUUFBSyxLQUFMLEdBQWEsTUFBTSxLQUFOLElBQWUsRUFBNUI7QUFDQSxRQUFLLEtBQUwsR0FBYSxFQUFiO0FBQ0EsUUFBSyxRQUFMLEdBQWdCLEVBQWhCO0FBQ0EsUUFBSyxZQUFMLEdBQW9CLEVBQXBCO0FBQ0Q7O0FBRUQsU0FBUSxTQUFSLEdBQW9CLElBQUksSUFBSixFQUFwQjs7QUFFQSxTQUFRLFNBQVIsQ0FBa0IsV0FBbEIsR0FBZ0MsVUFBVSxJQUFWLEVBQWdCO0FBQzlDLE9BQUksS0FBSyxVQUFMLElBQW1CLEtBQUssVUFBTCxLQUFvQixJQUEzQyxFQUFpRDtBQUMvQztBQUNEO0FBQ0QsT0FBSSxDQUFDLEtBQUssVUFBVixFQUFzQjtBQUNwQixnQkFBVyxJQUFYLEVBQWlCLElBQWpCO0FBQ0EsaUJBQVksSUFBWixFQUFrQixLQUFLLFFBQXZCLEVBQWlDLEtBQUssUUFBTCxDQUFjLE1BQS9DLEVBQXVELElBQXZEO0FBQ0EsU0FBSSxLQUFLLEtBQVQsRUFBZ0I7QUFDZCxvQkFBYSxLQUFLLEtBQWxCLEVBQXlCLElBQXpCO0FBQ0Q7QUFDRCxTQUFJLEtBQUssUUFBTCxLQUFrQixDQUF0QixFQUF5QjtBQUN2QixtQkFBWSxJQUFaLEVBQWtCLEtBQUssWUFBdkIsRUFBcUMsS0FBSyxZQUFMLENBQWtCLE1BQXZEO0FBQ0EsV0FBSSxLQUFLLEtBQVQsRUFBZ0I7QUFDZCxhQUFNLFdBQVcsWUFBWSxLQUFLLEtBQWpCLEVBQXdCLFFBQXpDO0FBQ0EsZ0JBQU8sU0FBUyxVQUFULENBQW9CLElBQXBCLEVBQTBCLEtBQUssR0FBL0IsRUFBb0MsQ0FBQyxDQUFyQyxDQUFQO0FBQ0Q7QUFDRjtBQUNGLElBYkQsTUFjSztBQUNILGVBQVUsSUFBVixFQUFnQixLQUFLLFFBQXJCLEVBQStCLEtBQUssUUFBTCxDQUFjLE1BQTdDLEVBQXFELElBQXJEO0FBQ0EsU0FBSSxLQUFLLFFBQUwsS0FBa0IsQ0FBdEIsRUFBeUI7QUFDdkIsV0FBTSxRQUFRLFVBQVUsSUFBVixFQUFnQixLQUFLLFlBQXJCLEVBQW1DLEtBQUssWUFBTCxDQUFrQixNQUFyRCxDQUFkO0FBQ0EsV0FBSSxLQUFLLEtBQUwsSUFBYyxTQUFTLENBQTNCLEVBQThCO0FBQzVCLGFBQU0sWUFBVyxZQUFZLEtBQUssS0FBakIsRUFBd0IsUUFBekM7QUFDQSxnQkFBTyxVQUFTLFdBQVQsQ0FBcUIsS0FBSyxHQUExQixFQUErQixLQUFLLEdBQXBDLEVBQXlDLEtBQXpDLENBQVA7QUFDRDtBQUNGO0FBQ0Y7QUFDRixFQTVCRDs7QUE4QkEsU0FBUSxTQUFSLENBQWtCLFlBQWxCLEdBQWlDLFVBQVUsSUFBVixFQUFnQixNQUFoQixFQUF3QjtBQUN2RCxPQUFJLEtBQUssVUFBTCxJQUFtQixLQUFLLFVBQUwsS0FBb0IsSUFBM0MsRUFBaUQ7QUFDL0M7QUFDRDtBQUNELE9BQUksU0FBUyxNQUFULElBQW1CLEtBQUssV0FBTCxLQUFxQixNQUE1QyxFQUFvRDtBQUNsRDtBQUNEO0FBQ0QsT0FBSSxDQUFDLEtBQUssVUFBVixFQUFzQjtBQUNwQixnQkFBVyxJQUFYLEVBQWlCLElBQWpCO0FBQ0EsaUJBQVksSUFBWixFQUFrQixLQUFLLFFBQXZCLEVBQWlDLEtBQUssUUFBTCxDQUFjLE9BQWQsQ0FBc0IsTUFBdEIsQ0FBakMsRUFBZ0UsSUFBaEU7QUFDQSxTQUFJLEtBQUssS0FBVCxFQUFnQjtBQUNkLG9CQUFhLEtBQUssS0FBbEIsRUFBeUIsSUFBekI7QUFDRDtBQUNELFNBQUksS0FBSyxRQUFMLEtBQWtCLENBQXRCLEVBQXlCO0FBQ3ZCLFdBQU0sYUFBYSxZQUFZLE1BQVosQ0FBbkI7QUFDQSxXQUFNLFFBQVEsWUFDWixJQURZLEVBRVosS0FBSyxZQUZPLEVBR1osYUFDSSxLQUFLLFlBQUwsQ0FBa0IsT0FBbEIsQ0FBMEIsVUFBMUIsQ0FESixHQUVJLEtBQUssWUFBTCxDQUFrQixNQUxWLENBQWQ7QUFPQSxXQUFJLEtBQUssS0FBVCxFQUFnQjtBQUNkLGFBQU0sV0FBVyxZQUFZLEtBQUssS0FBakIsRUFBd0IsUUFBekM7QUFDQSxnQkFBTyxTQUFTLFVBQVQsQ0FBb0IsSUFBcEIsRUFBMEIsS0FBSyxHQUEvQixFQUFvQyxLQUFwQyxDQUFQO0FBQ0Q7QUFDRjtBQUNGLElBcEJELE1BcUJLO0FBQ0gsZUFBVSxJQUFWLEVBQWdCLEtBQUssUUFBckIsRUFBK0IsS0FBSyxRQUFMLENBQWMsT0FBZCxDQUFzQixNQUF0QixDQUEvQixFQUE4RCxJQUE5RDtBQUNBLFNBQUksS0FBSyxRQUFMLEtBQWtCLENBQXRCLEVBQXlCO0FBQ3ZCLFdBQU0sY0FBYSxZQUFZLE1BQVosQ0FBbkI7QUFDQSxXQUFNLFNBQVEsVUFDWixJQURZLEVBRVosS0FBSyxZQUZPLEVBR1osY0FDSSxLQUFLLFlBQUwsQ0FBa0IsT0FBbEIsQ0FBMEIsV0FBMUIsQ0FESixHQUVJLEtBQUssWUFBTCxDQUFrQixNQUxWLENBQWQ7QUFPQSxXQUFJLEtBQUssS0FBTCxJQUFjLFVBQVMsQ0FBM0IsRUFBOEI7QUFDNUIsYUFBTSxhQUFXLFlBQVksS0FBSyxLQUFqQixFQUF3QixRQUF6QztBQUNBLGdCQUFPLFdBQVMsV0FBVCxDQUFxQixLQUFLLEdBQTFCLEVBQStCLEtBQUssR0FBcEMsRUFBeUMsTUFBekMsQ0FBUDtBQUNEO0FBQ0Y7QUFDRjtBQUNGLEVBN0NEOztBQStDQSxTQUFRLFNBQVIsQ0FBa0IsV0FBbEIsR0FBZ0MsVUFBVSxJQUFWLEVBQWdCLEtBQWhCLEVBQXVCO0FBQ3JELE9BQUksS0FBSyxVQUFMLElBQW1CLEtBQUssVUFBTCxLQUFvQixJQUEzQyxFQUFpRDtBQUMvQztBQUNEO0FBQ0QsT0FBSSxTQUFTLEtBQVQsSUFBa0IsS0FBSyxlQUFMLEtBQXlCLEtBQS9DLEVBQXNEO0FBQ3BEO0FBQ0Q7QUFDRCxPQUFJLENBQUMsS0FBSyxVQUFWLEVBQXNCO0FBQ3BCLGdCQUFXLElBQVgsRUFBaUIsSUFBakI7QUFDQSxpQkFBWSxJQUFaLEVBQWtCLEtBQUssUUFBdkIsRUFBaUMsS0FBSyxRQUFMLENBQWMsT0FBZCxDQUFzQixLQUF0QixJQUErQixDQUFoRSxFQUFtRSxJQUFuRTtBQUNBLFNBQUksS0FBSyxLQUFULEVBQWdCO0FBQ2Qsb0JBQWEsS0FBSyxLQUFsQixFQUF5QixJQUF6QjtBQUNEO0FBQ0QsU0FBSSxLQUFLLFFBQUwsS0FBa0IsQ0FBdEIsRUFBeUI7QUFDdkIsV0FBTSxRQUFRLFlBQ1osSUFEWSxFQUVaLEtBQUssWUFGTyxFQUdaLEtBQUssWUFBTCxDQUFrQixPQUFsQixDQUEwQixnQkFBZ0IsS0FBaEIsQ0FBMUIsSUFBb0QsQ0FIeEMsQ0FBZDtBQUtBLFdBQUksS0FBSyxLQUFULEVBQWdCO0FBQ2QsYUFBTSxXQUFXLFlBQVksS0FBSyxLQUFqQixFQUF3QixRQUF6QztBQUNBLGdCQUFPLFNBQVMsVUFBVCxDQUFvQixJQUFwQixFQUEwQixLQUFLLEdBQS9CLEVBQW9DLEtBQXBDLENBQVA7QUFDRDtBQUNGO0FBQ0YsSUFqQkQsTUFrQks7QUFDSCxlQUFVLElBQVYsRUFBZ0IsS0FBSyxRQUFyQixFQUErQixLQUFLLFFBQUwsQ0FBYyxPQUFkLENBQXNCLEtBQXRCLElBQStCLENBQTlELEVBQWlFLElBQWpFO0FBQ0EsU0FBSSxLQUFLLFFBQUwsS0FBa0IsQ0FBdEIsRUFBeUI7QUFDdkIsV0FBTSxVQUFRLFVBQ1osSUFEWSxFQUVaLEtBQUssWUFGTyxFQUdaLEtBQUssWUFBTCxDQUFrQixPQUFsQixDQUEwQixnQkFBZ0IsS0FBaEIsQ0FBMUIsSUFBb0QsQ0FIeEMsQ0FBZDtBQUtBLFdBQUksS0FBSyxLQUFMLElBQWMsV0FBUyxDQUEzQixFQUE4QjtBQUM1QixhQUFNLGFBQVcsWUFBWSxLQUFLLEtBQWpCLEVBQXdCLFFBQXpDO0FBQ0EsZ0JBQU8sV0FBUyxXQUFULENBQXFCLEtBQUssR0FBMUIsRUFBK0IsS0FBSyxHQUFwQyxFQUF5QyxPQUF6QyxDQUFQO0FBQ0Q7QUFDRjtBQUNGO0FBQ0YsRUF2Q0Q7O0FBeUNBLFNBQVEsU0FBUixDQUFrQixXQUFsQixHQUFnQyxVQUFVLElBQVYsRUFBZ0IsU0FBaEIsRUFBMkI7QUFDekQsT0FBSSxLQUFLLFVBQVQsRUFBcUI7QUFDbkIsaUJBQVksSUFBWixFQUFrQixLQUFLLFFBQXZCLEVBQWlDLElBQWpDO0FBQ0EsU0FBSSxLQUFLLFFBQUwsS0FBa0IsQ0FBdEIsRUFBeUI7QUFDdkIsbUJBQVksSUFBWixFQUFrQixLQUFLLFlBQXZCO0FBQ0EsV0FBSSxLQUFLLEtBQVQsRUFBZ0I7QUFDZCxhQUFNLFdBQVcsWUFBWSxLQUFLLEtBQWpCLEVBQXdCLFFBQXpDO0FBQ0Esa0JBQVMsYUFBVCxDQUF1QixLQUFLLEdBQTVCO0FBQ0Q7QUFDRjtBQUNGO0FBQ0QsT0FBSSxDQUFDLFNBQUwsRUFBZ0I7QUFDZCxVQUFLLE9BQUw7QUFDRDtBQUNGLEVBZEQ7O0FBZ0JBLFNBQVEsU0FBUixDQUFrQixLQUFsQixHQUEwQixZQUFZO0FBQUE7O0FBQ3BDLE9BQUksS0FBSyxLQUFULEVBQWdCO0FBQUE7QUFDZCxXQUFNLFdBQVcsWUFBWSxPQUFLLEtBQWpCLEVBQXdCLFFBQXpDO0FBQ0EsY0FBSyxZQUFMLENBQWtCLE9BQWxCLENBQTBCLGdCQUFRO0FBQ2hDLGtCQUFTLGFBQVQsQ0FBdUIsS0FBSyxHQUE1QjtBQUNELFFBRkQ7QUFGYztBQUtmO0FBQ0QsUUFBSyxRQUFMLENBQWMsT0FBZCxDQUFzQixnQkFBUTtBQUM1QixVQUFLLE9BQUw7QUFDRCxJQUZEO0FBR0EsUUFBSyxRQUFMLENBQWMsTUFBZCxHQUF1QixDQUF2QjtBQUNBLFFBQUssWUFBTCxDQUFrQixNQUFsQixHQUEyQixDQUEzQjtBQUNELEVBWkQ7O0FBY0EsVUFBUyxXQUFULENBQXNCLElBQXRCLEVBQTRCO0FBQzFCLFVBQU8sSUFBUCxFQUFhO0FBQ1gsU0FBSSxLQUFLLFFBQUwsS0FBa0IsQ0FBdEIsRUFBeUI7QUFDdkIsY0FBTyxJQUFQO0FBQ0Q7QUFDRCxZQUFPLEtBQUssV0FBWjtBQUNEO0FBQ0Y7O0FBRUQsVUFBUyxlQUFULENBQTBCLElBQTFCLEVBQWdDO0FBQzlCLFVBQU8sSUFBUCxFQUFhO0FBQ1gsU0FBSSxLQUFLLFFBQUwsS0FBa0IsQ0FBdEIsRUFBeUI7QUFDdkIsY0FBTyxJQUFQO0FBQ0Q7QUFDRCxZQUFPLEtBQUssZUFBWjtBQUNEO0FBQ0Y7O0FBRUQsVUFBUyxVQUFULENBQXFCLElBQXJCLEVBQTJCLE1BQTNCLEVBQW1DO0FBQ2pDLFFBQUssVUFBTCxHQUFrQixNQUFsQjtBQUNBLE9BQUksT0FBTyxLQUFYLEVBQWtCO0FBQ2hCLFVBQUssS0FBTCxHQUFhLE9BQU8sS0FBcEI7QUFDQSxVQUFLLGFBQUwsR0FBcUIsT0FBTyxhQUE1QjtBQUNBLFVBQUssYUFBTCxDQUFtQixPQUFuQixDQUEyQixLQUFLLE1BQWhDLElBQTBDLElBQTFDO0FBQ0EsVUFBSyxLQUFMLEdBQWEsT0FBTyxLQUFQLEdBQWUsQ0FBNUI7QUFDRDtBQUNELFFBQUssUUFBTCxDQUFjLE9BQWQsQ0FBc0IsaUJBQVM7QUFDN0IsZ0JBQVcsS0FBWCxFQUFrQixJQUFsQjtBQUNELElBRkQ7QUFHRDs7QUFFRCxVQUFTLFlBQVQsQ0FBdUIsS0FBdkIsRUFBOEIsSUFBOUIsRUFBb0M7QUFDbEMsT0FBTSxNQUFNLFlBQVksS0FBWixDQUFaO0FBQ0EsT0FBSSxPQUFKLENBQVksS0FBSyxNQUFqQixJQUEyQixJQUEzQjtBQUNEOztBQUVELFVBQVMsV0FBVCxDQUFzQixNQUF0QixFQUE4QixJQUE5QixFQUFvQyxRQUFwQyxFQUE4QyxhQUE5QyxFQUE2RDtBQUMzRCxPQUFJLFdBQVcsQ0FBZixFQUFrQjtBQUNoQixnQkFBVyxDQUFYO0FBQ0Q7QUFDRCxPQUFNLFNBQVMsS0FBSyxXQUFXLENBQWhCLENBQWY7QUFDQSxPQUFNLFFBQVEsS0FBSyxRQUFMLENBQWQ7QUFDQSxRQUFLLE1BQUwsQ0FBWSxRQUFaLEVBQXNCLENBQXRCLEVBQXlCLE1BQXpCO0FBQ0EsT0FBSSxhQUFKLEVBQW1CO0FBQ2pCLGdCQUFXLE9BQU8sV0FBUCxHQUFxQixNQUFoQztBQUNBLFlBQU8sZUFBUCxHQUF5QixNQUF6QjtBQUNBLFlBQU8sV0FBUCxHQUFxQixLQUFyQjtBQUNBLGVBQVUsTUFBTSxlQUFOLEdBQXdCLE1BQWxDO0FBQ0Q7QUFDRCxVQUFPLFFBQVA7QUFDRDs7QUFFRCxVQUFTLFNBQVQsQ0FBb0IsTUFBcEIsRUFBNEIsSUFBNUIsRUFBa0MsUUFBbEMsRUFBNEMsYUFBNUMsRUFBMkQ7QUFDekQsT0FBTSxRQUFRLEtBQUssT0FBTCxDQUFhLE1BQWIsQ0FBZDtBQUNBLE9BQUksUUFBUSxDQUFaLEVBQWU7QUFDYixZQUFPLENBQUMsQ0FBUjtBQUNEO0FBQ0QsT0FBSSxhQUFKLEVBQW1CO0FBQ2pCLFNBQU0sU0FBUyxLQUFLLFFBQVEsQ0FBYixDQUFmO0FBQ0EsU0FBTSxRQUFRLEtBQUssUUFBUSxDQUFiLENBQWQ7QUFDQSxnQkFBVyxPQUFPLFdBQVAsR0FBcUIsS0FBaEM7QUFDQSxlQUFVLE1BQU0sZUFBTixHQUF3QixNQUFsQztBQUNEO0FBQ0QsUUFBSyxNQUFMLENBQVksS0FBWixFQUFtQixDQUFuQjtBQUNBLE9BQUksZ0JBQWdCLFFBQXBCO0FBQ0EsT0FBSSxTQUFTLFFBQWIsRUFBdUI7QUFDckIscUJBQWdCLFdBQVcsQ0FBM0I7QUFDRDtBQUNELE9BQU0sWUFBWSxLQUFLLGdCQUFnQixDQUFyQixDQUFsQjtBQUNBLE9BQU0sV0FBVyxLQUFLLGFBQUwsQ0FBakI7QUFDQSxRQUFLLE1BQUwsQ0FBWSxhQUFaLEVBQTJCLENBQTNCLEVBQThCLE1BQTlCO0FBQ0EsT0FBSSxhQUFKLEVBQW1CO0FBQ2pCLG1CQUFjLFVBQVUsV0FBVixHQUF3QixNQUF0QztBQUNBLFlBQU8sZUFBUCxHQUF5QixTQUF6QjtBQUNBLFlBQU8sV0FBUCxHQUFxQixRQUFyQjtBQUNBLGtCQUFhLFNBQVMsZUFBVCxHQUEyQixNQUF4QztBQUNEO0FBQ0QsT0FBSSxVQUFVLGFBQWQsRUFBNkI7QUFDM0IsWUFBTyxDQUFDLENBQVI7QUFDRDtBQUNELFVBQU8sUUFBUDtBQUNEOztBQUVELFVBQVMsV0FBVCxDQUFzQixNQUF0QixFQUE4QixJQUE5QixFQUFvQyxhQUFwQyxFQUFtRDtBQUNqRCxPQUFNLFFBQVEsS0FBSyxPQUFMLENBQWEsTUFBYixDQUFkO0FBQ0EsT0FBSSxRQUFRLENBQVosRUFBZTtBQUNiO0FBQ0Q7QUFDRCxPQUFJLGFBQUosRUFBbUI7QUFDakIsU0FBTSxTQUFTLEtBQUssUUFBUSxDQUFiLENBQWY7QUFDQSxTQUFNLFFBQVEsS0FBSyxRQUFRLENBQWIsQ0FBZDtBQUNBLGdCQUFXLE9BQU8sV0FBUCxHQUFxQixLQUFoQztBQUNBLGVBQVUsTUFBTSxlQUFOLEdBQXdCLE1BQWxDO0FBQ0Q7QUFDRCxRQUFLLE1BQUwsQ0FBWSxLQUFaLEVBQW1CLENBQW5CO0FBQ0Q7O0FBRUQsU0FBUSxTQUFSLENBQWtCLE9BQWxCLEdBQTRCLFVBQVUsR0FBVixFQUFlLEtBQWYsRUFBc0IsTUFBdEIsRUFBOEI7QUFDeEQsT0FBSSxLQUFLLElBQUwsQ0FBVSxHQUFWLE1BQW1CLEtBQXZCLEVBQThCO0FBQzVCO0FBQ0Q7QUFDRCxRQUFLLElBQUwsQ0FBVSxHQUFWLElBQWlCLEtBQWpCO0FBQ0EsT0FBSSxDQUFDLE1BQUQsSUFBVyxLQUFLLEtBQXBCLEVBQTJCO0FBQ3pCLFNBQU0sV0FBVyxZQUFZLEtBQUssS0FBakIsRUFBd0IsUUFBekM7QUFDQSxjQUFTLE9BQVQsQ0FBaUIsS0FBSyxHQUF0QixFQUEyQixHQUEzQixFQUFnQyxLQUFoQztBQUNEO0FBQ0YsRUFURDs7QUFXQSxTQUFRLFNBQVIsQ0FBa0IsUUFBbEIsR0FBNkIsVUFBVSxHQUFWLEVBQWUsS0FBZixFQUFzQixNQUF0QixFQUE4QjtBQUN6RCxPQUFJLEtBQUssS0FBTCxDQUFXLEdBQVgsTUFBb0IsS0FBeEIsRUFBK0I7QUFDN0I7QUFDRDtBQUNELFFBQUssS0FBTCxDQUFXLEdBQVgsSUFBa0IsS0FBbEI7QUFDQSxPQUFJLENBQUMsTUFBRCxJQUFXLEtBQUssS0FBcEIsRUFBMkI7QUFDekIsU0FBTSxXQUFXLFlBQVksS0FBSyxLQUFqQixFQUF3QixRQUF6QztBQUNBLGNBQVMsUUFBVCxDQUFrQixLQUFLLEdBQXZCLEVBQTRCLEdBQTVCLEVBQWlDLEtBQWpDO0FBQ0Q7QUFDRixFQVREOztBQVdBLFNBQVEsU0FBUixDQUFrQixhQUFsQixHQUFrQyxVQUFVLFVBQVYsRUFBc0I7QUFDdEQsUUFBSyxVQUFMLEdBQWtCLFVBQWxCO0FBQ0EsT0FBSSxLQUFLLEtBQVQsRUFBZ0I7QUFDZCxTQUFNLFdBQVcsWUFBWSxLQUFLLEtBQWpCLEVBQXdCLFFBQXpDO0FBQ0EsY0FBUyxTQUFULENBQW1CLEtBQUssR0FBeEIsRUFBNkIsS0FBSyxPQUFMLEVBQTdCO0FBQ0Q7QUFDRixFQU5EOztBQVFBLFNBQVEsU0FBUixDQUFrQixRQUFsQixHQUE2QixVQUFVLElBQVYsRUFBZ0IsT0FBaEIsRUFBeUI7QUFDcEQsT0FBSSxDQUFDLEtBQUssS0FBTCxDQUFXLElBQVgsQ0FBTCxFQUF1QjtBQUNyQixVQUFLLEtBQUwsQ0FBVyxJQUFYLElBQW1CLE9BQW5CO0FBQ0EsU0FBSSxLQUFLLEtBQVQsRUFBZ0I7QUFDZCxXQUFNLFdBQVcsWUFBWSxLQUFLLEtBQWpCLEVBQXdCLFFBQXpDO0FBQ0EsZ0JBQVMsUUFBVCxDQUFrQixLQUFLLEdBQXZCLEVBQTRCLElBQTVCO0FBQ0Q7QUFDRjtBQUNGLEVBUkQ7O0FBVUEsU0FBUSxTQUFSLENBQWtCLFdBQWxCLEdBQWdDLFVBQVUsSUFBVixFQUFnQjtBQUM5QyxPQUFJLEtBQUssS0FBTCxDQUFXLElBQVgsQ0FBSixFQUFzQjtBQUNwQixZQUFPLEtBQUssS0FBTCxDQUFXLElBQVgsQ0FBUDtBQUNBLFNBQUksS0FBSyxLQUFULEVBQWdCO0FBQ2QsV0FBTSxXQUFXLFlBQVksS0FBSyxLQUFqQixFQUF3QixRQUF6QztBQUNBLGdCQUFTLFdBQVQsQ0FBcUIsS0FBSyxHQUExQixFQUErQixJQUEvQjtBQUNEO0FBQ0Y7QUFDRixFQVJEOztBQVVBLFNBQVEsU0FBUixDQUFrQixTQUFsQixHQUE4QixVQUFVLElBQVYsRUFBZ0IsQ0FBaEIsRUFBbUI7QUFDL0MsT0FBTSxVQUFVLEtBQUssS0FBTCxDQUFXLElBQVgsQ0FBaEI7QUFDQSxPQUFJLE9BQUosRUFBYTtBQUNYLFlBQU8sUUFBUSxJQUFSLENBQWEsSUFBYixFQUFtQixDQUFuQixDQUFQO0FBQ0Q7QUFDRixFQUxEOztBQU9BLFNBQVEsU0FBUixDQUFrQixPQUFsQixHQUE0QixZQUFZO0FBQ3RDLFVBQU8sb0JBQU8sRUFBUCxFQUFXLEtBQUssVUFBaEIsRUFBNEIsS0FBSyxLQUFqQyxDQUFQO0FBQ0QsRUFGRDs7QUFJQSxTQUFRLFNBQVIsQ0FBa0IsTUFBbEIsR0FBMkIsWUFBWTtBQUNyQyxPQUFNLFNBQVM7QUFDYixVQUFLLEtBQUssR0FBTCxDQUFTLFFBQVQsRUFEUTtBQUViLFdBQU0sS0FBSyxJQUZFO0FBR2IsV0FBTSxLQUFLLElBSEU7QUFJYixZQUFPLEtBQUssT0FBTDtBQUpNLElBQWY7QUFNQSxPQUFNLFFBQVEsT0FBTyxJQUFQLENBQVksS0FBSyxLQUFqQixDQUFkO0FBQ0EsT0FBSSxNQUFNLE1BQVYsRUFBa0I7QUFDaEIsWUFBTyxLQUFQLEdBQWUsS0FBZjtBQUNEO0FBQ0QsT0FBSSxLQUFLLFlBQUwsQ0FBa0IsTUFBdEIsRUFBOEI7QUFDNUIsWUFBTyxRQUFQLEdBQWtCLEtBQUssWUFBTCxDQUFrQixHQUFsQixDQUFzQixVQUFDLEtBQUQ7QUFBQSxjQUFXLE1BQU0sTUFBTixFQUFYO0FBQUEsTUFBdEIsQ0FBbEI7QUFDRDtBQUNELFVBQU8sTUFBUDtBQUNELEVBZkQ7O0FBaUJBLFNBQVEsU0FBUixDQUFrQixRQUFsQixHQUE2QixZQUFZO0FBQ3ZDLFVBQU8sTUFBTSxLQUFLLElBQVgsR0FDTCxRQURLLEdBQ00sS0FBSyxTQUFMLENBQWUsS0FBSyxJQUFwQixDQUROLEdBRUwsU0FGSyxHQUVPLEtBQUssU0FBTCxDQUFlLEtBQUssT0FBTCxFQUFmLENBRlAsR0FFd0MsR0FGeEMsR0FHTCxLQUFLLFlBQUwsQ0FBa0IsR0FBbEIsQ0FBc0IsVUFBQyxLQUFEO0FBQUEsWUFBVyxNQUFNLFFBQU4sRUFBWDtBQUFBLElBQXRCLEVBQW1ELElBQW5ELENBQXdELEVBQXhELENBSEssR0FJTCxJQUpLLEdBSUUsS0FBSyxJQUpQLEdBSWMsR0FKckI7QUFLRCxFQU5EOztBQVFPLFVBQVMsT0FBVCxDQUFrQixLQUFsQixFQUF5QjtBQUM5QixRQUFLLFFBQUwsR0FBZ0IsQ0FBaEI7QUFDQSxRQUFLLE1BQUwsR0FBYyxDQUFDLGFBQUQsRUFBZ0IsUUFBaEIsRUFBZDtBQUNBLFFBQUssR0FBTCxHQUFXLEtBQUssTUFBaEI7QUFDQSxRQUFLLElBQUwsR0FBWSxTQUFaO0FBQ0EsUUFBSyxLQUFMLEdBQWEsS0FBYjtBQUNBLFFBQUssUUFBTCxHQUFnQixFQUFoQjtBQUNBLFFBQUssWUFBTCxHQUFvQixFQUFwQjtBQUNEOztBQUVELFNBQVEsU0FBUixHQUFvQixJQUFJLElBQUosRUFBcEI7O0FBRUEsU0FBUSxTQUFSLENBQWtCLFFBQWxCLEdBQTZCLFlBQVk7QUFDdkMsVUFBTyxVQUFVLEtBQUssS0FBZixHQUF1QixNQUE5QjtBQUNELEVBRkQsQzs7Ozs7Ozs7Ozs7bUJDL2hCd0IsUTtTQW9HUixZLEdBQUEsWTtBQXBHRCxVQUFTLFFBQVQsQ0FBbUIsRUFBbkIsRUFBdUIsT0FBdkIsRUFBZ0M7QUFDN0MsUUFBSyxFQUFMLEdBQVUsRUFBVjtBQUNBLFFBQUssT0FBTCxHQUFlLEtBQWY7QUFDQSxRQUFLLE9BQUwsR0FBZSxFQUFmO0FBQ0EsT0FBSSxPQUFPLE9BQVAsS0FBbUIsVUFBdkIsRUFBbUM7QUFDakMsVUFBSyxPQUFMLEdBQWUsT0FBZjtBQUNEO0FBQ0Y7O0FBRUQsVUFBUyxTQUFULENBQW1CLFlBQW5CLEdBQWtDLFVBQVUsUUFBVixFQUFvQjtBQUNwRCxPQUFNLFVBQVUsS0FBSyxPQUFyQjtBQUNBLFVBQU8sUUFBUSxDQUFDLGFBQWEsY0FBYixFQUE2QixFQUE3QixDQUFELENBQVIsRUFBNEMsUUFBNUMsQ0FBUDtBQUNELEVBSEQ7O0FBS0EsVUFBUyxTQUFULENBQW1CLFlBQW5CLEdBQWtDLFVBQVUsUUFBVixFQUFvQjtBQUNwRCxPQUFNLFVBQVUsS0FBSyxPQUFyQjtBQUNBLFVBQU8sUUFBUSxDQUFDLGFBQWEsY0FBYixFQUE2QixFQUE3QixDQUFELENBQVIsRUFBNEMsUUFBNUMsQ0FBUDtBQUNELEVBSEQ7O0FBS0EsVUFBUyxTQUFULENBQW1CLGFBQW5CLEdBQW1DLFVBQVUsUUFBVixFQUFvQjtBQUNyRCxPQUFNLFVBQVUsS0FBSyxPQUFyQjtBQUNBLFVBQU8sUUFBUSxDQUFDLGFBQWEsZUFBYixFQUE4QixFQUE5QixDQUFELENBQVIsRUFBNkMsUUFBN0MsQ0FBUDtBQUNELEVBSEQ7O0FBS0EsVUFBUyxTQUFULENBQW1CLFVBQW5CLEdBQWdDLFVBQVUsT0FBVixFQUFtQjtBQUNqRCxPQUFNLE9BQU8sUUFBUSxNQUFSLEVBQWI7QUFDQSxPQUFNLFdBQVcsS0FBSyxRQUF0QjtBQUNBLFVBQU8sS0FBSyxRQUFaO0FBQ0EsT0FBTSxVQUFVLENBQUMsYUFBYSxZQUFiLEVBQTJCLENBQUMsSUFBRCxDQUEzQixDQUFELENBQWhCO0FBQ0EsT0FBSSxRQUFKLEVBQWM7QUFDWixhQUFRLElBQVIsQ0FBYSxLQUFiLENBQW1CLE9BQW5CLEVBQTRCLFNBQVMsR0FBVCxDQUFhLGlCQUFTO0FBQ2hELGNBQU8sYUFBYSxZQUFiLEVBQTJCLENBQUMsS0FBSyxHQUFOLEVBQVcsS0FBWCxFQUFrQixDQUFDLENBQW5CLENBQTNCLENBQVA7QUFDRCxNQUYyQixDQUE1QjtBQUdEO0FBQ0QsVUFBTyxLQUFLLFVBQUwsQ0FBZ0IsT0FBaEIsQ0FBUDtBQUNELEVBWEQ7O0FBYUEsVUFBUyxTQUFULENBQW1CLFVBQW5CLEdBQWdDLFVBQVUsT0FBVixFQUFtQixHQUFuQixFQUF3QixLQUF4QixFQUErQjtBQUM3RCxPQUFJLEVBQUUsU0FBUyxDQUFYLENBQUosRUFBbUI7QUFDakIsYUFBUSxDQUFDLENBQVQ7QUFDRDtBQUNELFVBQU8sS0FBSyxVQUFMLENBQWdCLGFBQWEsWUFBYixFQUEyQixDQUFDLEdBQUQsRUFBTSxRQUFRLE1BQVIsRUFBTixFQUF3QixLQUF4QixDQUEzQixDQUFoQixDQUFQO0FBQ0QsRUFMRDs7QUFPQSxVQUFTLFNBQVQsQ0FBbUIsYUFBbkIsR0FBbUMsVUFBVSxHQUFWLEVBQWU7QUFDaEQsT0FBSSxNQUFNLE9BQU4sQ0FBYyxHQUFkLENBQUosRUFBd0I7QUFDdEIsU0FBTSxVQUFVLElBQUksR0FBSixDQUFRLFVBQUMsQ0FBRDtBQUFBLGNBQU8sYUFBYSxlQUFiLEVBQThCLENBQUMsQ0FBRCxDQUE5QixDQUFQO0FBQUEsTUFBUixDQUFoQjtBQUNBLFlBQU8sS0FBSyxVQUFMLENBQWdCLE9BQWhCLENBQVA7QUFDRDtBQUNELFVBQU8sS0FBSyxVQUFMLENBQWdCLGFBQWEsZUFBYixFQUE4QixDQUFDLEdBQUQsQ0FBOUIsQ0FBaEIsQ0FBUDtBQUNELEVBTkQ7O0FBUUEsVUFBUyxTQUFULENBQW1CLFdBQW5CLEdBQWlDLFVBQVUsU0FBVixFQUFxQixTQUFyQixFQUFnQyxLQUFoQyxFQUF1QztBQUN0RSxVQUFPLEtBQUssVUFBTCxDQUFnQixhQUFhLGFBQWIsRUFBNEIsQ0FBQyxTQUFELEVBQVksU0FBWixFQUF1QixLQUF2QixDQUE1QixDQUFoQixDQUFQO0FBQ0QsRUFGRDs7QUFJQSxVQUFTLFNBQVQsQ0FBbUIsT0FBbkIsR0FBNkIsVUFBVSxHQUFWLEVBQWUsR0FBZixFQUFvQixLQUFwQixFQUEyQjtBQUN0RCxPQUFNLFNBQVMsRUFBZjtBQUNBLFVBQU8sR0FBUCxJQUFjLEtBQWQ7QUFDQSxVQUFPLEtBQUssVUFBTCxDQUFnQixhQUFhLGFBQWIsRUFBNEIsQ0FBQyxHQUFELEVBQU0sTUFBTixDQUE1QixDQUFoQixDQUFQO0FBQ0QsRUFKRDs7QUFNQSxVQUFTLFNBQVQsQ0FBbUIsUUFBbkIsR0FBOEIsVUFBVSxHQUFWLEVBQWUsR0FBZixFQUFvQixLQUFwQixFQUEyQjtBQUN2RCxPQUFNLFNBQVMsRUFBZjtBQUNBLFVBQU8sR0FBUCxJQUFjLEtBQWQ7QUFDQSxVQUFPLEtBQUssVUFBTCxDQUFnQixhQUFhLGFBQWIsRUFBNEIsQ0FBQyxHQUFELEVBQU0sTUFBTixDQUE1QixDQUFoQixDQUFQO0FBQ0QsRUFKRDs7QUFNQSxVQUFTLFNBQVQsQ0FBbUIsU0FBbkIsR0FBK0IsVUFBVSxHQUFWLEVBQWUsS0FBZixFQUFzQjtBQUNuRCxVQUFPLEtBQUssVUFBTCxDQUFnQixhQUFhLGFBQWIsRUFBNEIsQ0FBQyxHQUFELEVBQU0sS0FBTixDQUE1QixDQUFoQixDQUFQO0FBQ0QsRUFGRDs7QUFJQSxVQUFTLFNBQVQsQ0FBbUIsUUFBbkIsR0FBOEIsVUFBVSxHQUFWLEVBQWUsSUFBZixFQUFxQjtBQUNqRCxVQUFPLEtBQUssVUFBTCxDQUFnQixhQUFhLFVBQWIsRUFBeUIsQ0FBQyxHQUFELEVBQU0sSUFBTixDQUF6QixDQUFoQixDQUFQO0FBQ0QsRUFGRDs7QUFJQSxVQUFTLFNBQVQsQ0FBbUIsV0FBbkIsR0FBaUMsVUFBVSxHQUFWLEVBQWUsSUFBZixFQUFxQjtBQUNwRCxVQUFPLEtBQUssVUFBTCxDQUFnQixhQUFhLGFBQWIsRUFBNEIsQ0FBQyxHQUFELEVBQU0sSUFBTixDQUE1QixDQUFoQixDQUFQO0FBQ0QsRUFGRDs7QUFJQSxVQUFTLFNBQVQsQ0FBbUIsT0FBbkIsR0FBNkIsVUFBVSxPQUFWLEVBQW1CLEVBQW5CLEVBQXVCO0FBQ2xELFVBQU8sTUFBTSxJQUFiO0FBQ0QsRUFGRDs7QUFJQSxVQUFTLFNBQVQsQ0FBbUIsVUFBbkIsR0FBZ0MsVUFBVSxPQUFWLEVBQW1CO0FBQ2pELE9BQU0sVUFBVSxLQUFLLE9BQXJCO0FBQ0EsT0FBTSxVQUFVLEtBQUssT0FBckI7O0FBRUEsT0FBSSxDQUFDLE1BQU0sT0FBTixDQUFjLE9BQWQsQ0FBTCxFQUE2QjtBQUMzQixlQUFVLENBQUMsT0FBRCxDQUFWO0FBQ0Q7O0FBRUQsT0FBSSxLQUFLLE9BQVQsRUFBa0I7QUFDaEIsYUFBUSxJQUFSLENBQWEsS0FBYixDQUFtQixPQUFuQixFQUE0QixPQUE1QjtBQUNELElBRkQsTUFHSztBQUNILFlBQU8sUUFBUSxPQUFSLENBQVA7QUFDRDtBQUNGLEVBZEQ7O0FBZ0JPLFVBQVMsWUFBVCxDQUF1QixJQUF2QixFQUE2QixJQUE3QixFQUFtQztBQUN4QyxVQUFPLEVBQUUsUUFBUSxLQUFWLEVBQWlCLFFBQVEsSUFBekIsRUFBK0IsTUFBTSxJQUFyQyxFQUFQO0FBQ0QsRTs7Ozs7O0FDdEdEO0FBQ0E7QUFDQTtBQUNBO0FBQ0E7QUFDQTtBQUNBLEdBQUU7QUFDRjtBQUNBO0FBQ0E7QUFDQTtBQUNBO0FBQ0E7QUFDQTtBQUNBO0FBQ0E7QUFDQTtBQUNBLEdBQUU7QUFDRjtBQUNBO0FBQ0E7QUFDQTtBQUNBO0FBQ0E7QUFDQTtBQUNBO0FBQ0E7QUFDQTtBQUNBO0FBQ0EsR0FBRTtBQUNGO0FBQ0E7QUFDQSxHQUFFO0FBQ0Y7QUFDQTtBQUNBO0FBQ0E7QUFDQSxHOzs7Ozs7Ozs7OztTQ25CZ0IsQyxHQUFBLEM7U0FjQSxHLEdBQUEsRztTQWFBLEcsR0FBQSxHO1NBWUEsVyxHQUFBLFc7U0FjQSxTLEdBQUEsUztTQXFCQSxXLEdBQUEsVztTQTBCQSxVLEdBQUEsVTtTQWtCQSxTLEdBQUEsUztTQWFBLFEsR0FBQSxRO1NBYUEsUyxHQUFBLFM7U0FlQSxLLEdBQUEsSzs7QUE5S2hCOztBQUVBOzs7Ozs7QUFNQTs7Ozs7OztBQU9PLFVBQVMsQ0FBVCxDQUFZLEVBQVosRUFBZ0I7QUFDckIsV0FBUSxJQUFSLENBQWEsOERBQWI7QUFDQSxPQUFNLE9BQU8sS0FBSyxJQUFMLENBQVUsRUFBVixDQUFiO0FBQ0EsT0FBSSxJQUFKLEVBQVU7QUFDUixZQUFPLEtBQUssRUFBWjtBQUNEO0FBQ0Y7O0FBRUQ7Ozs7OztBQTFCQTs7O0FBZ0NPLFVBQVMsR0FBVCxDQUFjLEVBQWQsRUFBa0I7QUFDdkIsT0FBTSxPQUFPLEtBQUssSUFBTCxDQUFVLEVBQVYsQ0FBYjtBQUNBLE9BQUksSUFBSixFQUFVO0FBQ1IsWUFBTyxLQUFLLEVBQVo7QUFDRDtBQUNGOztBQUVEOzs7Ozs7QUFNTyxVQUFTLEdBQVQsQ0FBYyxFQUFkLEVBQWtCO0FBQ3ZCLE9BQU0sT0FBTyxLQUFLLElBQUwsQ0FBVSxFQUFWLENBQWI7QUFDQSxPQUFJLElBQUosRUFBVTtBQUNSLFlBQU8sS0FBSyxFQUFaO0FBQ0Q7QUFDRjs7QUFFRDs7Ozs7QUFLTyxVQUFTLFdBQVQsQ0FBc0IsRUFBdEIsRUFBMEI7QUFDL0IsT0FBTSxNQUFNLEtBQUssSUFBakI7QUFDQSxPQUFNLFNBQVMsSUFBSSxNQUFuQjtBQUNBLFVBQU8sT0FBTyxJQUFQLENBQVksWUFBTTtBQUN2QjtBQUNELElBRk0sQ0FBUDtBQUdEOztBQUVEOzs7Ozs7QUFNTyxVQUFTLFNBQVQsQ0FBb0IsRUFBcEIsRUFBd0IsTUFBeEIsRUFBZ0M7QUFDckMsV0FBUSxJQUFSLENBQWEsZ0RBQ0wsMkNBREssR0FFTCxpQ0FGUjtBQUdBLE9BQU0sS0FBSyxLQUFLLEdBQUwsQ0FBUyxFQUFULENBQVg7QUFDQSxPQUFJLEVBQUosRUFBUTtBQUNOLFNBQU0sTUFBTSxLQUFLLElBQUwsQ0FBVSxhQUFWLENBQXdCLEtBQXhCLENBQVo7QUFDQSxTQUFJLGVBQUosQ0FBb0IsR0FBRyxHQUF2QixFQUE0QixFQUFFLFFBQVEsTUFBVixFQUE1QjtBQUNEO0FBQ0Y7O0FBRUQ7Ozs7Ozs7Ozs7QUFVTyxVQUFTLFdBQVQsQ0FBc0IsRUFBdEIsRUFBMEIsT0FBMUIsRUFBbUMsUUFBbkMsRUFBNkM7QUFBQTs7QUFDbEQsT0FBTSxLQUFLLEtBQUssR0FBTCxDQUFTLEVBQVQsQ0FBWDtBQUNBLE9BQUksTUFBTSxPQUFOLElBQWlCLFFBQVEsTUFBN0IsRUFBcUM7QUFDbkMsU0FBTSxZQUFZLEtBQUssSUFBTCxDQUFVLGFBQVYsQ0FBd0IsV0FBeEIsQ0FBbEI7QUFDQSxlQUFVLFVBQVYsQ0FBcUIsR0FBRyxHQUF4QixFQUE2QixPQUE3QixFQUFzQyxZQUFhO0FBQ2pELGFBQUssU0FBTCxDQUFlLEVBQWYsRUFBbUIsUUFBUSxNQUEzQjtBQUNBLG1CQUFZLG9DQUFaO0FBQ0QsTUFIRDtBQUlEO0FBQ0Y7O0FBRUQ7Ozs7Ozs7Ozs7Ozs7OztBQWVPLFVBQVMsVUFBVCxDQUFxQixRQUFyQixFQUErQjtBQUNwQyxPQUFNLFNBQVMsa0JBQU87QUFDcEIsVUFBSyxPQUFPLGFBQVAsSUFBd0I7QUFEVCxJQUFQLEVBRVosS0FBSyxJQUFMLENBQVUsT0FGRSxDQUFmO0FBR0EsT0FBSSxpQkFBTSxRQUFOLE1BQW9CLFVBQXhCLEVBQW9DO0FBQ2xDLGFBQVEsSUFBUixDQUFhLDJFQUNYLCtDQURGO0FBRUEsY0FBUyxNQUFUO0FBQ0Q7QUFDRCxVQUFPLE1BQVA7QUFDRDs7QUFFRDs7Ozs7O0FBTU8sVUFBUyxTQUFULENBQW9CLE1BQXBCLEVBQTRCLFFBQTVCLEVBQXNDO0FBQzNDLFdBQVEsSUFBUixDQUFhLGdEQUNMLDhDQURLLEdBRUwsc0NBRlI7QUFHQSxPQUFNLFNBQVMsS0FBSyxJQUFMLENBQVUsYUFBVixDQUF3QixRQUF4QixDQUFmO0FBQ0EsVUFBTyxRQUFQLENBQWdCLE1BQWhCLEVBQXdCLFFBQXhCO0FBQ0Q7O0FBRUQ7Ozs7O0FBS08sVUFBUyxRQUFULENBQW1CLEdBQW5CLEVBQXdCO0FBQzdCLFdBQVEsSUFBUixDQUFhLCtDQUNMLDZDQURLLEdBRUwsd0JBRlI7QUFHQSxPQUFNLFFBQVEsS0FBSyxJQUFMLENBQVUsYUFBVixDQUF3QixPQUF4QixDQUFkO0FBQ0EsU0FBTSxPQUFOLENBQWMsR0FBZDtBQUNEOztBQUVEOzs7OztBQUtPLFVBQVMsU0FBVCxDQUFvQixLQUFwQixFQUEyQjtBQUNoQyxXQUFRLElBQVIsQ0FBYSxnREFDTCxnREFESyxHQUVMLDJCQUZSO0FBR0EsT0FBTSxXQUFXLEtBQUssSUFBTCxDQUFVLGFBQVYsQ0FBd0IsVUFBeEIsQ0FBakI7QUFDQSxZQUFTLFFBQVQsQ0FBa0IsS0FBbEI7QUFDRDs7QUFFRDs7Ozs7OztBQU9PLFVBQVMsS0FBVCxDQUFnQixVQUFoQixFQUE0QixVQUE1QixFQUFpRDtBQUN0RCxXQUFRLElBQVIsQ0FBYSw0Q0FDWCwyREFERjtBQUVBLE9BQU0sU0FBUyxLQUFLLElBQUwsQ0FBVSxhQUFWLENBQXdCLFVBQXhCLENBQWY7QUFDQSxPQUFJLFVBQVUsT0FBTyxVQUFQLENBQWQsRUFBa0M7QUFBQSx1Q0FKYyxJQUlkO0FBSmMsV0FJZDtBQUFBOztBQUNoQyxZQUFPLFVBQVAsZ0JBQXNCLElBQXRCO0FBQ0Q7QUFDRixFOzs7Ozs7Ozs7Ozs7U0N2S2UsVSxHQUFBLFU7U0FhQSxTLEdBQUEsUztTQXlCQSxhLEdBQUEsYTtTQWFBLE8sR0FBQSxPO1NBWUEsWSxHQUFBLFk7U0FZQSxNLEdBQUEsTTtTQVlBLE8sR0FBQSxPO0FBeEdoQjs7OztBQUlBOzs7Ozs7QUFNQTs7Ozs7OztBQU9PLFVBQVMsVUFBVCxDQUFvQixJQUFwQixFQUEwQixJQUExQixFQUFnQyxPQUFoQyxFQUF5QyxLQUF6QyxFQUFnRDtBQUNyRCxXQUFRLElBQVIsQ0FBYSxpREFDTCxpREFESyxHQUVMLDhDQUZSO0FBR0EsT0FBTSxZQUFZLEtBQUssSUFBTCxDQUFVLGFBQVYsQ0FBd0IsV0FBeEIsQ0FBbEI7QUFDQSxhQUFVLE1BQVYsQ0FBaUIsSUFBakIsRUFBdUIsSUFBdkIsRUFBNkIsT0FBN0IsRUFBc0MsS0FBdEM7QUFDRDs7QUFFRDs7Ozs7QUFLTyxVQUFTLFNBQVQsQ0FBbUIsTUFBbkIsRUFBMkIsUUFBM0IsRUFBcUM7QUFDMUMsV0FBUSxJQUFSLENBQWEsZ0RBQ0wsOENBREssR0FFTCxzQ0FGUjtBQUdBO0FBQ0EsT0FBSSxPQUFPLE1BQVAsS0FBa0IsV0FBdEIsRUFBbUM7QUFDakM7QUFDQSxTQUFNLFdBQVcsS0FBSyxJQUFMLENBQVUsYUFBVixDQUF3QixVQUF4QixDQUFqQjtBQUNBLGNBQVMsSUFBVCxDQUFjO0FBQ1osY0FBTyxjQURLO0FBRVosZUFBUSxNQUZJO0FBR1osYUFBTTtBQUhNLE1BQWQsRUFJRyxRQUpIO0FBS0QsSUFSRCxNQVFPO0FBQ0w7QUFDQSxTQUFNLFNBQVMsS0FBSyxJQUFMLENBQVUsYUFBVixDQUF3QixRQUF4QixDQUFmO0FBQ0EsWUFBTyxRQUFQLENBQWdCLE1BQWhCLEVBQXdCLFFBQXhCO0FBQ0Q7QUFDRjs7QUFFRDs7Ozs7QUFLTyxVQUFTLGFBQVQsQ0FBdUIsTUFBdkIsRUFBK0IsUUFBL0IsRUFBeUM7QUFDOUMsV0FBUSxJQUFSLENBQWEsb0RBQ0wsZ0RBREssR0FFTCxrQ0FGUjtBQUdBLE9BQU0sV0FBVyxLQUFLLElBQUwsQ0FBVSxhQUFWLENBQXdCLFVBQXhCLENBQWpCO0FBQ0EsWUFBUyxJQUFULENBQWMsTUFBZCxFQUFzQixRQUF0QjtBQUNEOztBQUVEOzs7OztBQUtPLFVBQVMsT0FBVCxDQUFpQixDQUFqQixFQUFvQixDQUFwQixFQUF1QjtBQUM1QixXQUFRLElBQVIsQ0FBYSw4Q0FDTCxnREFESyxHQUVMLHdCQUZSO0FBR0EsT0FBTSxXQUFXLEtBQUssSUFBTCxDQUFVLGFBQVYsQ0FBd0IsVUFBeEIsQ0FBakI7QUFDQSxZQUFTLE1BQVQsQ0FBZ0IsQ0FBaEIsRUFBbUIsQ0FBbkI7QUFDRDs7QUFFRDs7OztBQUlPLFVBQVMsWUFBVCxDQUFzQixRQUF0QixFQUFnQztBQUNyQyxXQUFRLElBQVIsQ0FBYSxtREFDTCw0Q0FESyxHQUVMLGlDQUZSO0FBR0EsT0FBTSxPQUFPLEtBQUssSUFBTCxDQUFVLGFBQVYsQ0FBd0IsTUFBeEIsQ0FBYjtBQUNBLFFBQUssV0FBTCxDQUFpQixRQUFqQjtBQUNEOztBQUVEOzs7O0FBSU8sVUFBUyxNQUFULENBQWdCLFFBQWhCLEVBQTBCO0FBQy9CLFdBQVEsSUFBUixDQUFhLDZDQUNMLDRDQURLLEdBRUwsMkJBRlI7QUFHQSxPQUFNLE9BQU8sS0FBSyxJQUFMLENBQVUsYUFBVixDQUF3QixNQUF4QixDQUFiO0FBQ0EsUUFBSyxLQUFMLENBQVcsUUFBWDtBQUNEOztBQUVEOzs7O0FBSU8sVUFBUyxPQUFULENBQWlCLFFBQWpCLEVBQTJCO0FBQ2hDLFdBQVEsSUFBUixDQUFhLDhDQUNMLDRDQURLLEdBRUwsNEJBRlI7QUFHQSxPQUFNLE9BQU8sS0FBSyxJQUFMLENBQVUsYUFBVixDQUF3QixNQUF4QixDQUFiO0FBQ0EsUUFBSyxNQUFMLENBQVksUUFBWjtBQUNELEUiLCJmaWxlIjoiaW5kZXguanMiLCJzb3VyY2VzQ29udGVudCI6WyIgXHQvLyBUaGUgbW9kdWxlIGNhY2hlXG4gXHR2YXIgaW5zdGFsbGVkTW9kdWxlcyA9IHt9O1xuXG4gXHQvLyBUaGUgcmVxdWlyZSBmdW5jdGlvblxuIFx0ZnVuY3Rpb24gX193ZWJwYWNrX3JlcXVpcmVfXyhtb2R1bGVJZCkge1xuXG4gXHRcdC8vIENoZWNrIGlmIG1vZHVsZSBpcyBpbiBjYWNoZVxuIFx0XHRpZihpbnN0YWxsZWRNb2R1bGVzW21vZHVsZUlkXSlcbiBcdFx0XHRyZXR1cm4gaW5zdGFsbGVkTW9kdWxlc1ttb2R1bGVJZF0uZXhwb3J0cztcblxuIFx0XHQvLyBDcmVhdGUgYSBuZXcgbW9kdWxlIChhbmQgcHV0IGl0IGludG8gdGhlIGNhY2hlKVxuIFx0XHR2YXIgbW9kdWxlID0gaW5zdGFsbGVkTW9kdWxlc1ttb2R1bGVJZF0gPSB7XG4gXHRcdFx0ZXhwb3J0czoge30sXG4gXHRcdFx0aWQ6IG1vZHVsZUlkLFxuIFx0XHRcdGxvYWRlZDogZmFsc2VcbiBcdFx0fTtcblxuIFx0XHQvLyBFeGVjdXRlIHRoZSBtb2R1bGUgZnVuY3Rpb25cbiBcdFx0bW9kdWxlc1ttb2R1bGVJZF0uY2FsbChtb2R1bGUuZXhwb3J0cywgbW9kdWxlLCBtb2R1bGUuZXhwb3J0cywgX193ZWJwYWNrX3JlcXVpcmVfXyk7XG5cbiBcdFx0Ly8gRmxhZyB0aGUgbW9kdWxlIGFzIGxvYWRlZFxuIFx0XHRtb2R1bGUubG9hZGVkID0gdHJ1ZTtcblxuIFx0XHQvLyBSZXR1cm4gdGhlIGV4cG9ydHMgb2YgdGhlIG1vZHVsZVxuIFx0XHRyZXR1cm4gbW9kdWxlLmV4cG9ydHM7XG4gXHR9XG5cblxuIFx0Ly8gZXhwb3NlIHRoZSBtb2R1bGVzIG9iamVjdCAoX193ZWJwYWNrX21vZHVsZXNfXylcbiBcdF9fd2VicGFja19yZXF1aXJlX18ubSA9IG1vZHVsZXM7XG5cbiBcdC8vIGV4cG9zZSB0aGUgbW9kdWxlIGNhY2hlXG4gXHRfX3dlYnBhY2tfcmVxdWlyZV9fLmMgPSBpbnN0YWxsZWRNb2R1bGVzO1xuXG4gXHQvLyBfX3dlYnBhY2tfcHVibGljX3BhdGhfX1xuIFx0X193ZWJwYWNrX3JlcXVpcmVfXy5wID0gXCJcIjtcblxuIFx0Ly8gTG9hZCBlbnRyeSBtb2R1bGUgYW5kIHJldHVybiBleHBvcnRzXG4gXHRyZXR1cm4gX193ZWJwYWNrX3JlcXVpcmVfXygwKTtcblxuXG5cbi8qKiBXRUJQQUNLIEZPT1RFUiAqKlxuICoqIHdlYnBhY2svYm9vdHN0cmFwIGVmZmU3MDMyZjZjYWIyN2JjNjYxXG4gKiovIiwiaW1wb3J0ICd3ZWV4LWpzLWZyYW1ld29yaydcblxuLyoqXG4gKiByZWdpc3RlciBtZXRob2RzXG4gKi9cbmNvbnN0IG1ldGhvZHMgPSByZXF1aXJlKCcuL2xpYi9hcGkvbWV0aG9kcycpXG5jb25zdCB7cmVnaXN0ZXJNZXRob2RzfSA9IGdsb2JhbFxucmVnaXN0ZXJNZXRob2RzKG1ldGhvZHMpXG5cblxuXG4vKiogV0VCUEFDSyBGT09URVIgKipcbiAqKiAuL2luZGV4LmpzXG4gKiovIiwiaW1wb3J0ICcuL3NyYy9uYXRpdmUnXG5cblxuXG4vKiogV0VCUEFDSyBGT09URVIgKipcbiAqKiAvVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvaW5kZXguanNcbiAqKi8iLCJpbXBvcnQgJy4uL3NoYXJlZCdcbmltcG9ydCBydW50aW1lIGZyb20gJy4uL3J1bnRpbWUnXG5pbXBvcnQgeyBzdWJ2ZXJzaW9uIH0gZnJvbSAnLi4vLi4vcGFja2FnZS5qc29uJ1xuaW1wb3J0ICogYXMgbWV0aG9kcyBmcm9tICcuLi9kZWZhdWx0L2FwaS9tZXRob2RzJ1xuXG5jb25zdCB7IG5hdGl2ZSwgdHJhbnNmb3JtZXIgfSA9IHN1YnZlcnNpb25cblxuLy8gcmVnaXN0ZXIgaW5zdGFuY2UgbWFuYWdlbWVudCBBUElzXG5mb3IgKGNvbnN0IG1ldGhvZE5hbWUgaW4gcnVudGltZSkge1xuICBnbG9iYWxbbWV0aG9kTmFtZV0gPSAoLi4uYXJncykgPT4ge1xuICAgIGNvbnN0IHJldCA9IHJ1bnRpbWVbbWV0aG9kTmFtZV0oLi4uYXJncylcbiAgICBpZiAocmV0IGluc3RhbmNlb2YgRXJyb3IpIHtcbiAgICAgIGNvbnNvbGUuZXJyb3IocmV0LnRvU3RyaW5nKCkpXG4gICAgfVxuICAgIHJldHVybiByZXRcbiAgfVxufVxuXG4vLyByZWdpc3RlciBmcmFtZXdvcmsgbWV0YSBpbmZvXG5nbG9iYWwuZnJhbWV3b3JrVmVyc2lvbiA9IG5hdGl2ZVxuZ2xvYmFsLnRyYW5zZm9ybWVyVmVyc2lvbiA9IHRyYW5zZm9ybWVyXG5cbi8vIHJlZ2lzdGVyIHNwZWNpYWwgbWV0aG9kcyBmb3IgV2VleCBmcmFtZXdvcmtcbmdsb2JhbC5yZWdpc3Rlck1ldGhvZHMobWV0aG9kcylcblxuXG5cbi8qKiBXRUJQQUNLIEZPT1RFUiAqKlxuICoqIC9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9zcmMvbmF0aXZlL2luZGV4LmpzXG4gKiovIiwiaW1wb3J0ICcuL3NldFRpbWVvdXQnXG5pbXBvcnQgJy4vcHJvbWlzZSdcbmltcG9ydCAnLi9jb25zb2xlJ1xuaW1wb3J0ICcuL29iamVjdEFzc2lnbidcblxuLyoqXG4gKiBNaXggcHJvcGVydGllcyBpbnRvIHRhcmdldCBvYmplY3QuXG4gKlxuICogQHBhcmFtIHtPYmplY3R9IHRvXG4gKiBAcGFyYW0ge09iamVjdH0gZnJvbVxuICovXG5cbmV4cG9ydCBmdW5jdGlvbiBleHRlbmQgKHRhcmdldCwgLi4uc3JjKSB7XG4gIGlmICh0eXBlb2YgT2JqZWN0LmFzc2lnbiA9PT0gJ2Z1bmN0aW9uJykge1xuICAgIE9iamVjdC5hc3NpZ24odGFyZ2V0LCAuLi5zcmMpXG4gIH1cbiAgZWxzZSB7XG4gICAgY29uc3QgZmlyc3QgPSBzcmMuc2hpZnQoKVxuICAgIGZvciAoY29uc3Qga2V5IGluIGZpcnN0KSB7XG4gICAgICB0YXJnZXRba2V5XSA9IGZpcnN0W2tleV1cbiAgICB9XG4gICAgaWYgKHNyYy5sZW5ndGgpIHtcbiAgICAgIGV4dGVuZCh0YXJnZXQsIC4uLnNyYylcbiAgICB9XG4gIH1cbiAgcmV0dXJuIHRhcmdldFxufVxuXG4vKipcbiAqIERlZmluZSBhIHByb3BlcnR5LlxuICpcbiAqIEBwYXJhbSB7T2JqZWN0fSBvYmpcbiAqIEBwYXJhbSB7U3RyaW5nfSBrZXlcbiAqIEBwYXJhbSB7Kn0gdmFsXG4gKiBAcGFyYW0ge0Jvb2xlYW59IFtlbnVtZXJhYmxlXVxuICovXG5cbmV4cG9ydCBmdW5jdGlvbiBkZWYgKG9iaiwga2V5LCB2YWwsIGVudW1lcmFibGUpIHtcbiAgT2JqZWN0LmRlZmluZVByb3BlcnR5KG9iaiwga2V5LCB7XG4gICAgdmFsdWU6IHZhbCxcbiAgICBlbnVtZXJhYmxlOiAhIWVudW1lcmFibGUsXG4gICAgd3JpdGFibGU6IHRydWUsXG4gICAgY29uZmlndXJhYmxlOiB0cnVlXG4gIH0pXG59XG5cbi8qKlxuICogUmVtb3ZlIGFuIGl0ZW0gZnJvbSBhbiBhcnJheVxuICpcbiAqIEBwYXJhbSB7QXJyYXl9IGFyclxuICogQHBhcmFtIHsqfSBpdGVtXG4gKi9cblxuZXhwb3J0IGZ1bmN0aW9uIHJlbW92ZSAoYXJyLCBpdGVtKSB7XG4gIGlmIChhcnIubGVuZ3RoKSB7XG4gICAgY29uc3QgaW5kZXggPSBhcnIuaW5kZXhPZihpdGVtKVxuICAgIGlmIChpbmRleCA+IC0xKSB7XG4gICAgICByZXR1cm4gYXJyLnNwbGljZShpbmRleCwgMSlcbiAgICB9XG4gIH1cbn1cblxuLyoqXG4gKiBDaGVjayB3aGV0aGVyIHRoZSBvYmplY3QgaGFzIHRoZSBwcm9wZXJ0eS5cbiAqXG4gKiBAcGFyYW0ge09iamVjdH0gb2JqXG4gKiBAcGFyYW0ge1N0cmluZ30ga2V5XG4gKiBAcmV0dXJuIHtCb29sZWFufVxuICovXG5cbmNvbnN0IGhhc093blByb3BlcnR5ID0gT2JqZWN0LnByb3RvdHlwZS5oYXNPd25Qcm9wZXJ0eVxuZXhwb3J0IGZ1bmN0aW9uIGhhc093biAob2JqLCBrZXkpIHtcbiAgcmV0dXJuIGhhc093blByb3BlcnR5LmNhbGwob2JqLCBrZXkpXG59XG5cbi8qKlxuICogU2ltcGxlIGJpbmQsIGZhc3RlciB0aGFuIG5hdGl2ZVxuICpcbiAqIEBwYXJhbSB7RnVuY3Rpb259IGZuXG4gKiBAcGFyYW0ge09iamVjdH0gY3R4XG4gKiBAcmV0dXJuIHtGdW5jdGlvbn1cbiAqL1xuXG5leHBvcnQgZnVuY3Rpb24gYmluZCAoZm4sIGN0eCkge1xuICByZXR1cm4gZnVuY3Rpb24gKGEpIHtcbiAgICBjb25zdCBsID0gYXJndW1lbnRzLmxlbmd0aFxuICAgIHJldHVybiBsXG4gICAgICA/IGwgPiAxXG4gICAgICAgID8gZm4uYXBwbHkoY3R4LCBhcmd1bWVudHMpXG4gICAgICAgIDogZm4uY2FsbChjdHgsIGEpXG4gICAgICA6IGZuLmNhbGwoY3R4KVxuICB9XG59XG5cbi8qKlxuICogQ29udmVydCBhbiBBcnJheS1saWtlIG9iamVjdCB0byBhIHJlYWwgQXJyYXkuXG4gKlxuICogQHBhcmFtIHtBcnJheS1saWtlfSBsaXN0XG4gKiBAcGFyYW0ge051bWJlcn0gW3N0YXJ0XSAtIHN0YXJ0IGluZGV4XG4gKiBAcmV0dXJuIHtBcnJheX1cbiAqL1xuXG5leHBvcnQgZnVuY3Rpb24gdG9BcnJheSAobGlzdCwgc3RhcnQpIHtcbiAgc3RhcnQgPSBzdGFydCB8fCAwXG4gIGxldCBpID0gbGlzdC5sZW5ndGggLSBzdGFydFxuICBjb25zdCByZXQgPSBuZXcgQXJyYXkoaSlcbiAgd2hpbGUgKGktLSkge1xuICAgIHJldFtpXSA9IGxpc3RbaSArIHN0YXJ0XVxuICB9XG4gIHJldHVybiByZXRcbn1cblxuLyoqXG4gKiBRdWljayBvYmplY3QgY2hlY2sgLSB0aGlzIGlzIHByaW1hcmlseSB1c2VkIHRvIHRlbGxcbiAqIE9iamVjdHMgZnJvbSBwcmltaXRpdmUgdmFsdWVzIHdoZW4gd2Uga25vdyB0aGUgdmFsdWVcbiAqIGlzIGEgSlNPTi1jb21wbGlhbnQgdHlwZS5cbiAqXG4gKiBAcGFyYW0geyp9IG9ialxuICogQHJldHVybiB7Qm9vbGVhbn1cbiAqL1xuXG5leHBvcnQgZnVuY3Rpb24gaXNPYmplY3QgKG9iaikge1xuICByZXR1cm4gb2JqICE9PSBudWxsICYmIHR5cGVvZiBvYmogPT09ICdvYmplY3QnXG59XG5cbi8qKlxuICogU3RyaWN0IG9iamVjdCB0eXBlIGNoZWNrLiBPbmx5IHJldHVybnMgdHJ1ZVxuICogZm9yIHBsYWluIEphdmFTY3JpcHQgb2JqZWN0cy5cbiAqXG4gKiBAcGFyYW0geyp9IG9ialxuICogQHJldHVybiB7Qm9vbGVhbn1cbiAqL1xuXG5jb25zdCB0b1N0cmluZyA9IE9iamVjdC5wcm90b3R5cGUudG9TdHJpbmdcbmNvbnN0IE9CSkVDVF9TVFJJTkcgPSAnW29iamVjdCBPYmplY3RdJ1xuZXhwb3J0IGZ1bmN0aW9uIGlzUGxhaW5PYmplY3QgKG9iaikge1xuICByZXR1cm4gdG9TdHJpbmcuY2FsbChvYmopID09PSBPQkpFQ1RfU1RSSU5HXG59XG5cblxuXG5cbi8qKiBXRUJQQUNLIEZPT1RFUiAqKlxuICoqIC9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9zcmMvc2hhcmVkL2luZGV4LmpzXG4gKiovIiwiLyoqXG4gKiBAZmlsZU92ZXJ2aWV3XG4gKiBQb2x5ZmlsbCBgc2V0VGltZW91dGAgb24gQW5kcm9pZCBWOCB1c2luZyBuYXRpdmUgbWV0aG9kXG4gKiBgc2V0VGltZW91dE5hdGl2ZShjYWxsYmFja0lkLCB0aW1lKWAgYW5kIEpTIG1ldGhvZFxuICogYHNldFRpbWVvdXRDYWxsYmFjayhjYWxsYmFja0lkKWAuXG4gKiBUaGlzIHBvbHlmaWxsIGlzIG9ubHkgdXNlZCBpbiB2aXJ0dWFsLURPTSBkaWZmICYgZmx1c2ggYWdvcml0aG0uIE5vdFxuICogYWNjZXNzZWQgYnkgSlMgQnVuZGxlIGNvZGUgKFRoZSB0aW1lciBBUElzIHBvbHlmaWxsIGZvciBKUyBCdW5kbGUgaXMgaW5cbiAqIGBodG1sNS9kZWZhdWx0L2FwcC9jdHJsLmpzYCkuXG4gKi9cblxuY29uc3Qge1xuICBzZXRUaW1lb3V0LFxuICBzZXRUaW1lb3V0TmF0aXZlXG59ID0gZ2xvYmFsXG5cbi8qIGlzdGFuYnVsIGlnbm9yZSBpZiAqL1xuaWYgKHR5cGVvZiBzZXRUaW1lb3V0ID09PSAndW5kZWZpbmVkJyAmJlxuICB0eXBlb2Ygc2V0VGltZW91dE5hdGl2ZSA9PT0gJ2Z1bmN0aW9uJykge1xuICBjb25zdCB0aW1lb3V0TWFwID0ge31cbiAgbGV0IHRpbWVvdXRJZCA9IDBcblxuICBnbG9iYWwuc2V0VGltZW91dCA9IChjYiwgdGltZSkgPT4ge1xuICAgIHRpbWVvdXRNYXBbKyt0aW1lb3V0SWRdID0gY2JcbiAgICBzZXRUaW1lb3V0TmF0aXZlKHRpbWVvdXRJZC50b1N0cmluZygpLCB0aW1lKVxuICB9XG5cbiAgZ2xvYmFsLnNldFRpbWVvdXRDYWxsYmFjayA9IChpZCkgPT4ge1xuICAgIGlmICh0eXBlb2YgdGltZW91dE1hcFtpZF0gPT09ICdmdW5jdGlvbicpIHtcbiAgICAgIHRpbWVvdXRNYXBbaWRdKClcbiAgICAgIGRlbGV0ZSB0aW1lb3V0TWFwW2lkXVxuICAgIH1cbiAgfVxufVxuXG5cblxuLyoqIFdFQlBBQ0sgRk9PVEVSICoqXG4gKiogL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL3NyYy9zaGFyZWQvc2V0VGltZW91dC5qc1xuICoqLyIsIi8vIGZpeCBQcm9taXNlIFByb2JsZW0gb24gSlNDb250ZXh0IG9mIGlPUzd+OFxuLy8gQHNlZSBodHRwczovL2J1Z3Mud2Via2l0Lm9yZy9zaG93X2J1Zy5jZ2k/aWQ9MTM1ODY2XG4vLyBnbG9iYWwuUHJvbWlzZSA9IG51bGxcbnJlcXVpcmUoJ2NvcmUtanMvbW9kdWxlcy9lczYub2JqZWN0LnRvLXN0cmluZycpXG5yZXF1aXJlKCdjb3JlLWpzL21vZHVsZXMvZXM2LnN0cmluZy5pdGVyYXRvcicpXG5yZXF1aXJlKCdjb3JlLWpzL21vZHVsZXMvd2ViLmRvbS5pdGVyYWJsZScpXG5yZXF1aXJlKCdjb3JlLWpzL21vZHVsZXMvZXM2LnByb21pc2UnKVxuXG5cblxuLyoqIFdFQlBBQ0sgRk9PVEVSICoqXG4gKiogL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL3NyYy9zaGFyZWQvcHJvbWlzZS5qc1xuICoqLyIsIid1c2Ugc3RyaWN0Jztcbi8vIDE5LjEuMy42IE9iamVjdC5wcm90b3R5cGUudG9TdHJpbmcoKVxudmFyIGNsYXNzb2YgPSByZXF1aXJlKCcuL19jbGFzc29mJylcbiAgLCB0ZXN0ICAgID0ge307XG50ZXN0W3JlcXVpcmUoJy4vX3drcycpKCd0b1N0cmluZ1RhZycpXSA9ICd6JztcbmlmKHRlc3QgKyAnJyAhPSAnW29iamVjdCB6XScpe1xuICByZXF1aXJlKCcuL19yZWRlZmluZScpKE9iamVjdC5wcm90b3R5cGUsICd0b1N0cmluZycsIGZ1bmN0aW9uIHRvU3RyaW5nKCl7XG4gICAgcmV0dXJuICdbb2JqZWN0ICcgKyBjbGFzc29mKHRoaXMpICsgJ10nO1xuICB9LCB0cnVlKTtcbn1cblxuXG4vKiogV0VCUEFDSyBGT09URVIgKipcbiAqKiAvVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvfi9jb3JlLWpzL21vZHVsZXMvZXM2Lm9iamVjdC50by1zdHJpbmcuanNcbiAqKi8iLCIvLyBnZXR0aW5nIHRhZyBmcm9tIDE5LjEuMy42IE9iamVjdC5wcm90b3R5cGUudG9TdHJpbmcoKVxudmFyIGNvZiA9IHJlcXVpcmUoJy4vX2NvZicpXG4gICwgVEFHID0gcmVxdWlyZSgnLi9fd2tzJykoJ3RvU3RyaW5nVGFnJylcbiAgLy8gRVMzIHdyb25nIGhlcmVcbiAgLCBBUkcgPSBjb2YoZnVuY3Rpb24oKXsgcmV0dXJuIGFyZ3VtZW50czsgfSgpKSA9PSAnQXJndW1lbnRzJztcblxuLy8gZmFsbGJhY2sgZm9yIElFMTEgU2NyaXB0IEFjY2VzcyBEZW5pZWQgZXJyb3JcbnZhciB0cnlHZXQgPSBmdW5jdGlvbihpdCwga2V5KXtcbiAgdHJ5IHtcbiAgICByZXR1cm4gaXRba2V5XTtcbiAgfSBjYXRjaChlKXsgLyogZW1wdHkgKi8gfVxufTtcblxubW9kdWxlLmV4cG9ydHMgPSBmdW5jdGlvbihpdCl7XG4gIHZhciBPLCBULCBCO1xuICByZXR1cm4gaXQgPT09IHVuZGVmaW5lZCA/ICdVbmRlZmluZWQnIDogaXQgPT09IG51bGwgPyAnTnVsbCdcbiAgICAvLyBAQHRvU3RyaW5nVGFnIGNhc2VcbiAgICA6IHR5cGVvZiAoVCA9IHRyeUdldChPID0gT2JqZWN0KGl0KSwgVEFHKSkgPT0gJ3N0cmluZycgPyBUXG4gICAgLy8gYnVpbHRpblRhZyBjYXNlXG4gICAgOiBBUkcgPyBjb2YoTylcbiAgICAvLyBFUzMgYXJndW1lbnRzIGZhbGxiYWNrXG4gICAgOiAoQiA9IGNvZihPKSkgPT0gJ09iamVjdCcgJiYgdHlwZW9mIE8uY2FsbGVlID09ICdmdW5jdGlvbicgPyAnQXJndW1lbnRzJyA6IEI7XG59O1xuXG5cbi8qKiBXRUJQQUNLIEZPT1RFUiAqKlxuICoqIC9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9fY2xhc3NvZi5qc1xuICoqLyIsInZhciB0b1N0cmluZyA9IHt9LnRvU3RyaW5nO1xuXG5tb2R1bGUuZXhwb3J0cyA9IGZ1bmN0aW9uKGl0KXtcbiAgcmV0dXJuIHRvU3RyaW5nLmNhbGwoaXQpLnNsaWNlKDgsIC0xKTtcbn07XG5cblxuLyoqIFdFQlBBQ0sgRk9PVEVSICoqXG4gKiogL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL34vY29yZS1qcy9tb2R1bGVzL19jb2YuanNcbiAqKi8iLCJ2YXIgc3RvcmUgICAgICA9IHJlcXVpcmUoJy4vX3NoYXJlZCcpKCd3a3MnKVxuICAsIHVpZCAgICAgICAgPSByZXF1aXJlKCcuL191aWQnKVxuICAsIFN5bWJvbCAgICAgPSByZXF1aXJlKCcuL19nbG9iYWwnKS5TeW1ib2xcbiAgLCBVU0VfU1lNQk9MID0gdHlwZW9mIFN5bWJvbCA9PSAnZnVuY3Rpb24nO1xuXG52YXIgJGV4cG9ydHMgPSBtb2R1bGUuZXhwb3J0cyA9IGZ1bmN0aW9uKG5hbWUpe1xuICByZXR1cm4gc3RvcmVbbmFtZV0gfHwgKHN0b3JlW25hbWVdID1cbiAgICBVU0VfU1lNQk9MICYmIFN5bWJvbFtuYW1lXSB8fCAoVVNFX1NZTUJPTCA/IFN5bWJvbCA6IHVpZCkoJ1N5bWJvbC4nICsgbmFtZSkpO1xufTtcblxuJGV4cG9ydHMuc3RvcmUgPSBzdG9yZTtcblxuXG4vKiogV0VCUEFDSyBGT09URVIgKipcbiAqKiAvVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvfi9jb3JlLWpzL21vZHVsZXMvX3drcy5qc1xuICoqLyIsInZhciBnbG9iYWwgPSByZXF1aXJlKCcuL19nbG9iYWwnKVxuICAsIFNIQVJFRCA9ICdfX2NvcmUtanNfc2hhcmVkX18nXG4gICwgc3RvcmUgID0gZ2xvYmFsW1NIQVJFRF0gfHwgKGdsb2JhbFtTSEFSRURdID0ge30pO1xubW9kdWxlLmV4cG9ydHMgPSBmdW5jdGlvbihrZXkpe1xuICByZXR1cm4gc3RvcmVba2V5XSB8fCAoc3RvcmVba2V5XSA9IHt9KTtcbn07XG5cblxuLyoqIFdFQlBBQ0sgRk9PVEVSICoqXG4gKiogL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL34vY29yZS1qcy9tb2R1bGVzL19zaGFyZWQuanNcbiAqKi8iLCIvLyBodHRwczovL2dpdGh1Yi5jb20vemxvaXJvY2svY29yZS1qcy9pc3N1ZXMvODYjaXNzdWVjb21tZW50LTExNTc1OTAyOFxudmFyIGdsb2JhbCA9IG1vZHVsZS5leHBvcnRzID0gdHlwZW9mIHdpbmRvdyAhPSAndW5kZWZpbmVkJyAmJiB3aW5kb3cuTWF0aCA9PSBNYXRoXG4gID8gd2luZG93IDogdHlwZW9mIHNlbGYgIT0gJ3VuZGVmaW5lZCcgJiYgc2VsZi5NYXRoID09IE1hdGggPyBzZWxmIDogRnVuY3Rpb24oJ3JldHVybiB0aGlzJykoKTtcbmlmKHR5cGVvZiBfX2cgPT0gJ251bWJlcicpX19nID0gZ2xvYmFsOyAvLyBlc2xpbnQtZGlzYWJsZS1saW5lIG5vLXVuZGVmXG5cblxuLyoqIFdFQlBBQ0sgRk9PVEVSICoqXG4gKiogL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL34vY29yZS1qcy9tb2R1bGVzL19nbG9iYWwuanNcbiAqKi8iLCJ2YXIgaWQgPSAwXG4gICwgcHggPSBNYXRoLnJhbmRvbSgpO1xubW9kdWxlLmV4cG9ydHMgPSBmdW5jdGlvbihrZXkpe1xuICByZXR1cm4gJ1N5bWJvbCgnLmNvbmNhdChrZXkgPT09IHVuZGVmaW5lZCA/ICcnIDoga2V5LCAnKV8nLCAoKytpZCArIHB4KS50b1N0cmluZygzNikpO1xufTtcblxuXG4vKiogV0VCUEFDSyBGT09URVIgKipcbiAqKiAvVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvfi9jb3JlLWpzL21vZHVsZXMvX3VpZC5qc1xuICoqLyIsInZhciBnbG9iYWwgICAgPSByZXF1aXJlKCcuL19nbG9iYWwnKVxuICAsIGhpZGUgICAgICA9IHJlcXVpcmUoJy4vX2hpZGUnKVxuICAsIGhhcyAgICAgICA9IHJlcXVpcmUoJy4vX2hhcycpXG4gICwgU1JDICAgICAgID0gcmVxdWlyZSgnLi9fdWlkJykoJ3NyYycpXG4gICwgVE9fU1RSSU5HID0gJ3RvU3RyaW5nJ1xuICAsICR0b1N0cmluZyA9IEZ1bmN0aW9uW1RPX1NUUklOR11cbiAgLCBUUEwgICAgICAgPSAoJycgKyAkdG9TdHJpbmcpLnNwbGl0KFRPX1NUUklORyk7XG5cbnJlcXVpcmUoJy4vX2NvcmUnKS5pbnNwZWN0U291cmNlID0gZnVuY3Rpb24oaXQpe1xuICByZXR1cm4gJHRvU3RyaW5nLmNhbGwoaXQpO1xufTtcblxuKG1vZHVsZS5leHBvcnRzID0gZnVuY3Rpb24oTywga2V5LCB2YWwsIHNhZmUpe1xuICB2YXIgaXNGdW5jdGlvbiA9IHR5cGVvZiB2YWwgPT0gJ2Z1bmN0aW9uJztcbiAgaWYoaXNGdW5jdGlvbiloYXModmFsLCAnbmFtZScpIHx8IGhpZGUodmFsLCAnbmFtZScsIGtleSk7XG4gIGlmKE9ba2V5XSA9PT0gdmFsKXJldHVybjtcbiAgaWYoaXNGdW5jdGlvbiloYXModmFsLCBTUkMpIHx8IGhpZGUodmFsLCBTUkMsIE9ba2V5XSA/ICcnICsgT1trZXldIDogVFBMLmpvaW4oU3RyaW5nKGtleSkpKTtcbiAgaWYoTyA9PT0gZ2xvYmFsKXtcbiAgICBPW2tleV0gPSB2YWw7XG4gIH0gZWxzZSB7XG4gICAgaWYoIXNhZmUpe1xuICAgICAgZGVsZXRlIE9ba2V5XTtcbiAgICAgIGhpZGUoTywga2V5LCB2YWwpO1xuICAgIH0gZWxzZSB7XG4gICAgICBpZihPW2tleV0pT1trZXldID0gdmFsO1xuICAgICAgZWxzZSBoaWRlKE8sIGtleSwgdmFsKTtcbiAgICB9XG4gIH1cbi8vIGFkZCBmYWtlIEZ1bmN0aW9uI3RvU3RyaW5nIGZvciBjb3JyZWN0IHdvcmsgd3JhcHBlZCBtZXRob2RzIC8gY29uc3RydWN0b3JzIHdpdGggbWV0aG9kcyBsaWtlIExvRGFzaCBpc05hdGl2ZVxufSkoRnVuY3Rpb24ucHJvdG90eXBlLCBUT19TVFJJTkcsIGZ1bmN0aW9uIHRvU3RyaW5nKCl7XG4gIHJldHVybiB0eXBlb2YgdGhpcyA9PSAnZnVuY3Rpb24nICYmIHRoaXNbU1JDXSB8fCAkdG9TdHJpbmcuY2FsbCh0aGlzKTtcbn0pO1xuXG5cbi8qKiBXRUJQQUNLIEZPT1RFUiAqKlxuICoqIC9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9fcmVkZWZpbmUuanNcbiAqKi8iLCJ2YXIgZFAgICAgICAgICA9IHJlcXVpcmUoJy4vX29iamVjdC1kcCcpXG4gICwgY3JlYXRlRGVzYyA9IHJlcXVpcmUoJy4vX3Byb3BlcnR5LWRlc2MnKTtcbm1vZHVsZS5leHBvcnRzID0gcmVxdWlyZSgnLi9fZGVzY3JpcHRvcnMnKSA/IGZ1bmN0aW9uKG9iamVjdCwga2V5LCB2YWx1ZSl7XG4gIHJldHVybiBkUC5mKG9iamVjdCwga2V5LCBjcmVhdGVEZXNjKDEsIHZhbHVlKSk7XG59IDogZnVuY3Rpb24ob2JqZWN0LCBrZXksIHZhbHVlKXtcbiAgb2JqZWN0W2tleV0gPSB2YWx1ZTtcbiAgcmV0dXJuIG9iamVjdDtcbn07XG5cblxuLyoqIFdFQlBBQ0sgRk9PVEVSICoqXG4gKiogL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL34vY29yZS1qcy9tb2R1bGVzL19oaWRlLmpzXG4gKiovIiwidmFyIGFuT2JqZWN0ICAgICAgID0gcmVxdWlyZSgnLi9fYW4tb2JqZWN0JylcbiAgLCBJRThfRE9NX0RFRklORSA9IHJlcXVpcmUoJy4vX2llOC1kb20tZGVmaW5lJylcbiAgLCB0b1ByaW1pdGl2ZSAgICA9IHJlcXVpcmUoJy4vX3RvLXByaW1pdGl2ZScpXG4gICwgZFAgICAgICAgICAgICAgPSBPYmplY3QuZGVmaW5lUHJvcGVydHk7XG5cbmV4cG9ydHMuZiA9IHJlcXVpcmUoJy4vX2Rlc2NyaXB0b3JzJykgPyBPYmplY3QuZGVmaW5lUHJvcGVydHkgOiBmdW5jdGlvbiBkZWZpbmVQcm9wZXJ0eShPLCBQLCBBdHRyaWJ1dGVzKXtcbiAgYW5PYmplY3QoTyk7XG4gIFAgPSB0b1ByaW1pdGl2ZShQLCB0cnVlKTtcbiAgYW5PYmplY3QoQXR0cmlidXRlcyk7XG4gIGlmKElFOF9ET01fREVGSU5FKXRyeSB7XG4gICAgcmV0dXJuIGRQKE8sIFAsIEF0dHJpYnV0ZXMpO1xuICB9IGNhdGNoKGUpeyAvKiBlbXB0eSAqLyB9XG4gIGlmKCdnZXQnIGluIEF0dHJpYnV0ZXMgfHwgJ3NldCcgaW4gQXR0cmlidXRlcyl0aHJvdyBUeXBlRXJyb3IoJ0FjY2Vzc29ycyBub3Qgc3VwcG9ydGVkIScpO1xuICBpZigndmFsdWUnIGluIEF0dHJpYnV0ZXMpT1tQXSA9IEF0dHJpYnV0ZXMudmFsdWU7XG4gIHJldHVybiBPO1xufTtcblxuXG4vKiogV0VCUEFDSyBGT09URVIgKipcbiAqKiAvVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvfi9jb3JlLWpzL21vZHVsZXMvX29iamVjdC1kcC5qc1xuICoqLyIsInZhciBpc09iamVjdCA9IHJlcXVpcmUoJy4vX2lzLW9iamVjdCcpO1xubW9kdWxlLmV4cG9ydHMgPSBmdW5jdGlvbihpdCl7XG4gIGlmKCFpc09iamVjdChpdCkpdGhyb3cgVHlwZUVycm9yKGl0ICsgJyBpcyBub3QgYW4gb2JqZWN0IScpO1xuICByZXR1cm4gaXQ7XG59O1xuXG5cbi8qKiBXRUJQQUNLIEZPT1RFUiAqKlxuICoqIC9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9fYW4tb2JqZWN0LmpzXG4gKiovIiwibW9kdWxlLmV4cG9ydHMgPSBmdW5jdGlvbihpdCl7XG4gIHJldHVybiB0eXBlb2YgaXQgPT09ICdvYmplY3QnID8gaXQgIT09IG51bGwgOiB0eXBlb2YgaXQgPT09ICdmdW5jdGlvbic7XG59O1xuXG5cbi8qKiBXRUJQQUNLIEZPT1RFUiAqKlxuICoqIC9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9faXMtb2JqZWN0LmpzXG4gKiovIiwibW9kdWxlLmV4cG9ydHMgPSAhcmVxdWlyZSgnLi9fZGVzY3JpcHRvcnMnKSAmJiAhcmVxdWlyZSgnLi9fZmFpbHMnKShmdW5jdGlvbigpe1xuICByZXR1cm4gT2JqZWN0LmRlZmluZVByb3BlcnR5KHJlcXVpcmUoJy4vX2RvbS1jcmVhdGUnKSgnZGl2JyksICdhJywge2dldDogZnVuY3Rpb24oKXsgcmV0dXJuIDc7IH19KS5hICE9IDc7XG59KTtcblxuXG4vKiogV0VCUEFDSyBGT09URVIgKipcbiAqKiAvVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvfi9jb3JlLWpzL21vZHVsZXMvX2llOC1kb20tZGVmaW5lLmpzXG4gKiovIiwiLy8gVGhhbmsncyBJRTggZm9yIGhpcyBmdW5ueSBkZWZpbmVQcm9wZXJ0eVxubW9kdWxlLmV4cG9ydHMgPSAhcmVxdWlyZSgnLi9fZmFpbHMnKShmdW5jdGlvbigpe1xuICByZXR1cm4gT2JqZWN0LmRlZmluZVByb3BlcnR5KHt9LCAnYScsIHtnZXQ6IGZ1bmN0aW9uKCl7IHJldHVybiA3OyB9fSkuYSAhPSA3O1xufSk7XG5cblxuLyoqIFdFQlBBQ0sgRk9PVEVSICoqXG4gKiogL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL34vY29yZS1qcy9tb2R1bGVzL19kZXNjcmlwdG9ycy5qc1xuICoqLyIsIm1vZHVsZS5leHBvcnRzID0gZnVuY3Rpb24oZXhlYyl7XG4gIHRyeSB7XG4gICAgcmV0dXJuICEhZXhlYygpO1xuICB9IGNhdGNoKGUpe1xuICAgIHJldHVybiB0cnVlO1xuICB9XG59O1xuXG5cbi8qKiBXRUJQQUNLIEZPT1RFUiAqKlxuICoqIC9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9fZmFpbHMuanNcbiAqKi8iLCJ2YXIgaXNPYmplY3QgPSByZXF1aXJlKCcuL19pcy1vYmplY3QnKVxuICAsIGRvY3VtZW50ID0gcmVxdWlyZSgnLi9fZ2xvYmFsJykuZG9jdW1lbnRcbiAgLy8gaW4gb2xkIElFIHR5cGVvZiBkb2N1bWVudC5jcmVhdGVFbGVtZW50IGlzICdvYmplY3QnXG4gICwgaXMgPSBpc09iamVjdChkb2N1bWVudCkgJiYgaXNPYmplY3QoZG9jdW1lbnQuY3JlYXRlRWxlbWVudCk7XG5tb2R1bGUuZXhwb3J0cyA9IGZ1bmN0aW9uKGl0KXtcbiAgcmV0dXJuIGlzID8gZG9jdW1lbnQuY3JlYXRlRWxlbWVudChpdCkgOiB7fTtcbn07XG5cblxuLyoqIFdFQlBBQ0sgRk9PVEVSICoqXG4gKiogL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL34vY29yZS1qcy9tb2R1bGVzL19kb20tY3JlYXRlLmpzXG4gKiovIiwiLy8gNy4xLjEgVG9QcmltaXRpdmUoaW5wdXQgWywgUHJlZmVycmVkVHlwZV0pXG52YXIgaXNPYmplY3QgPSByZXF1aXJlKCcuL19pcy1vYmplY3QnKTtcbi8vIGluc3RlYWQgb2YgdGhlIEVTNiBzcGVjIHZlcnNpb24sIHdlIGRpZG4ndCBpbXBsZW1lbnQgQEB0b1ByaW1pdGl2ZSBjYXNlXG4vLyBhbmQgdGhlIHNlY29uZCBhcmd1bWVudCAtIGZsYWcgLSBwcmVmZXJyZWQgdHlwZSBpcyBhIHN0cmluZ1xubW9kdWxlLmV4cG9ydHMgPSBmdW5jdGlvbihpdCwgUyl7XG4gIGlmKCFpc09iamVjdChpdCkpcmV0dXJuIGl0O1xuICB2YXIgZm4sIHZhbDtcbiAgaWYoUyAmJiB0eXBlb2YgKGZuID0gaXQudG9TdHJpbmcpID09ICdmdW5jdGlvbicgJiYgIWlzT2JqZWN0KHZhbCA9IGZuLmNhbGwoaXQpKSlyZXR1cm4gdmFsO1xuICBpZih0eXBlb2YgKGZuID0gaXQudmFsdWVPZikgPT0gJ2Z1bmN0aW9uJyAmJiAhaXNPYmplY3QodmFsID0gZm4uY2FsbChpdCkpKXJldHVybiB2YWw7XG4gIGlmKCFTICYmIHR5cGVvZiAoZm4gPSBpdC50b1N0cmluZykgPT0gJ2Z1bmN0aW9uJyAmJiAhaXNPYmplY3QodmFsID0gZm4uY2FsbChpdCkpKXJldHVybiB2YWw7XG4gIHRocm93IFR5cGVFcnJvcihcIkNhbid0IGNvbnZlcnQgb2JqZWN0IHRvIHByaW1pdGl2ZSB2YWx1ZVwiKTtcbn07XG5cblxuLyoqIFdFQlBBQ0sgRk9PVEVSICoqXG4gKiogL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL34vY29yZS1qcy9tb2R1bGVzL190by1wcmltaXRpdmUuanNcbiAqKi8iLCJtb2R1bGUuZXhwb3J0cyA9IGZ1bmN0aW9uKGJpdG1hcCwgdmFsdWUpe1xuICByZXR1cm4ge1xuICAgIGVudW1lcmFibGUgIDogIShiaXRtYXAgJiAxKSxcbiAgICBjb25maWd1cmFibGU6ICEoYml0bWFwICYgMiksXG4gICAgd3JpdGFibGUgICAgOiAhKGJpdG1hcCAmIDQpLFxuICAgIHZhbHVlICAgICAgIDogdmFsdWVcbiAgfTtcbn07XG5cblxuLyoqIFdFQlBBQ0sgRk9PVEVSICoqXG4gKiogL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL34vY29yZS1qcy9tb2R1bGVzL19wcm9wZXJ0eS1kZXNjLmpzXG4gKiovIiwidmFyIGhhc093blByb3BlcnR5ID0ge30uaGFzT3duUHJvcGVydHk7XG5tb2R1bGUuZXhwb3J0cyA9IGZ1bmN0aW9uKGl0LCBrZXkpe1xuICByZXR1cm4gaGFzT3duUHJvcGVydHkuY2FsbChpdCwga2V5KTtcbn07XG5cblxuLyoqIFdFQlBBQ0sgRk9PVEVSICoqXG4gKiogL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL34vY29yZS1qcy9tb2R1bGVzL19oYXMuanNcbiAqKi8iLCJ2YXIgY29yZSA9IG1vZHVsZS5leHBvcnRzID0ge3ZlcnNpb246ICcyLjQuMCd9O1xuaWYodHlwZW9mIF9fZSA9PSAnbnVtYmVyJylfX2UgPSBjb3JlOyAvLyBlc2xpbnQtZGlzYWJsZS1saW5lIG5vLXVuZGVmXG5cblxuLyoqIFdFQlBBQ0sgRk9PVEVSICoqXG4gKiogL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL34vY29yZS1qcy9tb2R1bGVzL19jb3JlLmpzXG4gKiovIiwiJ3VzZSBzdHJpY3QnO1xudmFyICRhdCAgPSByZXF1aXJlKCcuL19zdHJpbmctYXQnKSh0cnVlKTtcblxuLy8gMjEuMS4zLjI3IFN0cmluZy5wcm90b3R5cGVbQEBpdGVyYXRvcl0oKVxucmVxdWlyZSgnLi9faXRlci1kZWZpbmUnKShTdHJpbmcsICdTdHJpbmcnLCBmdW5jdGlvbihpdGVyYXRlZCl7XG4gIHRoaXMuX3QgPSBTdHJpbmcoaXRlcmF0ZWQpOyAvLyB0YXJnZXRcbiAgdGhpcy5faSA9IDA7ICAgICAgICAgICAgICAgIC8vIG5leHQgaW5kZXhcbi8vIDIxLjEuNS4yLjEgJVN0cmluZ0l0ZXJhdG9yUHJvdG90eXBlJS5uZXh0KClcbn0sIGZ1bmN0aW9uKCl7XG4gIHZhciBPICAgICA9IHRoaXMuX3RcbiAgICAsIGluZGV4ID0gdGhpcy5faVxuICAgICwgcG9pbnQ7XG4gIGlmKGluZGV4ID49IE8ubGVuZ3RoKXJldHVybiB7dmFsdWU6IHVuZGVmaW5lZCwgZG9uZTogdHJ1ZX07XG4gIHBvaW50ID0gJGF0KE8sIGluZGV4KTtcbiAgdGhpcy5faSArPSBwb2ludC5sZW5ndGg7XG4gIHJldHVybiB7dmFsdWU6IHBvaW50LCBkb25lOiBmYWxzZX07XG59KTtcblxuXG4vKiogV0VCUEFDSyBGT09URVIgKipcbiAqKiAvVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvfi9jb3JlLWpzL21vZHVsZXMvZXM2LnN0cmluZy5pdGVyYXRvci5qc1xuICoqLyIsInZhciB0b0ludGVnZXIgPSByZXF1aXJlKCcuL190by1pbnRlZ2VyJylcbiAgLCBkZWZpbmVkICAgPSByZXF1aXJlKCcuL19kZWZpbmVkJyk7XG4vLyB0cnVlICAtPiBTdHJpbmcjYXRcbi8vIGZhbHNlIC0+IFN0cmluZyNjb2RlUG9pbnRBdFxubW9kdWxlLmV4cG9ydHMgPSBmdW5jdGlvbihUT19TVFJJTkcpe1xuICByZXR1cm4gZnVuY3Rpb24odGhhdCwgcG9zKXtcbiAgICB2YXIgcyA9IFN0cmluZyhkZWZpbmVkKHRoYXQpKVxuICAgICAgLCBpID0gdG9JbnRlZ2VyKHBvcylcbiAgICAgICwgbCA9IHMubGVuZ3RoXG4gICAgICAsIGEsIGI7XG4gICAgaWYoaSA8IDAgfHwgaSA+PSBsKXJldHVybiBUT19TVFJJTkcgPyAnJyA6IHVuZGVmaW5lZDtcbiAgICBhID0gcy5jaGFyQ29kZUF0KGkpO1xuICAgIHJldHVybiBhIDwgMHhkODAwIHx8IGEgPiAweGRiZmYgfHwgaSArIDEgPT09IGwgfHwgKGIgPSBzLmNoYXJDb2RlQXQoaSArIDEpKSA8IDB4ZGMwMCB8fCBiID4gMHhkZmZmXG4gICAgICA/IFRPX1NUUklORyA/IHMuY2hhckF0KGkpIDogYVxuICAgICAgOiBUT19TVFJJTkcgPyBzLnNsaWNlKGksIGkgKyAyKSA6IChhIC0gMHhkODAwIDw8IDEwKSArIChiIC0gMHhkYzAwKSArIDB4MTAwMDA7XG4gIH07XG59O1xuXG5cbi8qKiBXRUJQQUNLIEZPT1RFUiAqKlxuICoqIC9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9fc3RyaW5nLWF0LmpzXG4gKiovIiwiLy8gNy4xLjQgVG9JbnRlZ2VyXG52YXIgY2VpbCAgPSBNYXRoLmNlaWxcbiAgLCBmbG9vciA9IE1hdGguZmxvb3I7XG5tb2R1bGUuZXhwb3J0cyA9IGZ1bmN0aW9uKGl0KXtcbiAgcmV0dXJuIGlzTmFOKGl0ID0gK2l0KSA/IDAgOiAoaXQgPiAwID8gZmxvb3IgOiBjZWlsKShpdCk7XG59O1xuXG5cbi8qKiBXRUJQQUNLIEZPT1RFUiAqKlxuICoqIC9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9fdG8taW50ZWdlci5qc1xuICoqLyIsIi8vIDcuMi4xIFJlcXVpcmVPYmplY3RDb2VyY2libGUoYXJndW1lbnQpXG5tb2R1bGUuZXhwb3J0cyA9IGZ1bmN0aW9uKGl0KXtcbiAgaWYoaXQgPT0gdW5kZWZpbmVkKXRocm93IFR5cGVFcnJvcihcIkNhbid0IGNhbGwgbWV0aG9kIG9uICBcIiArIGl0KTtcbiAgcmV0dXJuIGl0O1xufTtcblxuXG4vKiogV0VCUEFDSyBGT09URVIgKipcbiAqKiAvVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvfi9jb3JlLWpzL21vZHVsZXMvX2RlZmluZWQuanNcbiAqKi8iLCIndXNlIHN0cmljdCc7XG52YXIgTElCUkFSWSAgICAgICAgPSByZXF1aXJlKCcuL19saWJyYXJ5JylcbiAgLCAkZXhwb3J0ICAgICAgICA9IHJlcXVpcmUoJy4vX2V4cG9ydCcpXG4gICwgcmVkZWZpbmUgICAgICAgPSByZXF1aXJlKCcuL19yZWRlZmluZScpXG4gICwgaGlkZSAgICAgICAgICAgPSByZXF1aXJlKCcuL19oaWRlJylcbiAgLCBoYXMgICAgICAgICAgICA9IHJlcXVpcmUoJy4vX2hhcycpXG4gICwgSXRlcmF0b3JzICAgICAgPSByZXF1aXJlKCcuL19pdGVyYXRvcnMnKVxuICAsICRpdGVyQ3JlYXRlICAgID0gcmVxdWlyZSgnLi9faXRlci1jcmVhdGUnKVxuICAsIHNldFRvU3RyaW5nVGFnID0gcmVxdWlyZSgnLi9fc2V0LXRvLXN0cmluZy10YWcnKVxuICAsIGdldFByb3RvdHlwZU9mID0gcmVxdWlyZSgnLi9fb2JqZWN0LWdwbycpXG4gICwgSVRFUkFUT1IgICAgICAgPSByZXF1aXJlKCcuL193a3MnKSgnaXRlcmF0b3InKVxuICAsIEJVR0dZICAgICAgICAgID0gIShbXS5rZXlzICYmICduZXh0JyBpbiBbXS5rZXlzKCkpIC8vIFNhZmFyaSBoYXMgYnVnZ3kgaXRlcmF0b3JzIHcvbyBgbmV4dGBcbiAgLCBGRl9JVEVSQVRPUiAgICA9ICdAQGl0ZXJhdG9yJ1xuICAsIEtFWVMgICAgICAgICAgID0gJ2tleXMnXG4gICwgVkFMVUVTICAgICAgICAgPSAndmFsdWVzJztcblxudmFyIHJldHVyblRoaXMgPSBmdW5jdGlvbigpeyByZXR1cm4gdGhpczsgfTtcblxubW9kdWxlLmV4cG9ydHMgPSBmdW5jdGlvbihCYXNlLCBOQU1FLCBDb25zdHJ1Y3RvciwgbmV4dCwgREVGQVVMVCwgSVNfU0VULCBGT1JDRUQpe1xuICAkaXRlckNyZWF0ZShDb25zdHJ1Y3RvciwgTkFNRSwgbmV4dCk7XG4gIHZhciBnZXRNZXRob2QgPSBmdW5jdGlvbihraW5kKXtcbiAgICBpZighQlVHR1kgJiYga2luZCBpbiBwcm90bylyZXR1cm4gcHJvdG9ba2luZF07XG4gICAgc3dpdGNoKGtpbmQpe1xuICAgICAgY2FzZSBLRVlTOiByZXR1cm4gZnVuY3Rpb24ga2V5cygpeyByZXR1cm4gbmV3IENvbnN0cnVjdG9yKHRoaXMsIGtpbmQpOyB9O1xuICAgICAgY2FzZSBWQUxVRVM6IHJldHVybiBmdW5jdGlvbiB2YWx1ZXMoKXsgcmV0dXJuIG5ldyBDb25zdHJ1Y3Rvcih0aGlzLCBraW5kKTsgfTtcbiAgICB9IHJldHVybiBmdW5jdGlvbiBlbnRyaWVzKCl7IHJldHVybiBuZXcgQ29uc3RydWN0b3IodGhpcywga2luZCk7IH07XG4gIH07XG4gIHZhciBUQUcgICAgICAgID0gTkFNRSArICcgSXRlcmF0b3InXG4gICAgLCBERUZfVkFMVUVTID0gREVGQVVMVCA9PSBWQUxVRVNcbiAgICAsIFZBTFVFU19CVUcgPSBmYWxzZVxuICAgICwgcHJvdG8gICAgICA9IEJhc2UucHJvdG90eXBlXG4gICAgLCAkbmF0aXZlICAgID0gcHJvdG9bSVRFUkFUT1JdIHx8IHByb3RvW0ZGX0lURVJBVE9SXSB8fCBERUZBVUxUICYmIHByb3RvW0RFRkFVTFRdXG4gICAgLCAkZGVmYXVsdCAgID0gJG5hdGl2ZSB8fCBnZXRNZXRob2QoREVGQVVMVClcbiAgICAsICRlbnRyaWVzICAgPSBERUZBVUxUID8gIURFRl9WQUxVRVMgPyAkZGVmYXVsdCA6IGdldE1ldGhvZCgnZW50cmllcycpIDogdW5kZWZpbmVkXG4gICAgLCAkYW55TmF0aXZlID0gTkFNRSA9PSAnQXJyYXknID8gcHJvdG8uZW50cmllcyB8fCAkbmF0aXZlIDogJG5hdGl2ZVxuICAgICwgbWV0aG9kcywga2V5LCBJdGVyYXRvclByb3RvdHlwZTtcbiAgLy8gRml4IG5hdGl2ZVxuICBpZigkYW55TmF0aXZlKXtcbiAgICBJdGVyYXRvclByb3RvdHlwZSA9IGdldFByb3RvdHlwZU9mKCRhbnlOYXRpdmUuY2FsbChuZXcgQmFzZSkpO1xuICAgIGlmKEl0ZXJhdG9yUHJvdG90eXBlICE9PSBPYmplY3QucHJvdG90eXBlKXtcbiAgICAgIC8vIFNldCBAQHRvU3RyaW5nVGFnIHRvIG5hdGl2ZSBpdGVyYXRvcnNcbiAgICAgIHNldFRvU3RyaW5nVGFnKEl0ZXJhdG9yUHJvdG90eXBlLCBUQUcsIHRydWUpO1xuICAgICAgLy8gZml4IGZvciBzb21lIG9sZCBlbmdpbmVzXG4gICAgICBpZighTElCUkFSWSAmJiAhaGFzKEl0ZXJhdG9yUHJvdG90eXBlLCBJVEVSQVRPUikpaGlkZShJdGVyYXRvclByb3RvdHlwZSwgSVRFUkFUT1IsIHJldHVyblRoaXMpO1xuICAgIH1cbiAgfVxuICAvLyBmaXggQXJyYXkje3ZhbHVlcywgQEBpdGVyYXRvcn0ubmFtZSBpbiBWOCAvIEZGXG4gIGlmKERFRl9WQUxVRVMgJiYgJG5hdGl2ZSAmJiAkbmF0aXZlLm5hbWUgIT09IFZBTFVFUyl7XG4gICAgVkFMVUVTX0JVRyA9IHRydWU7XG4gICAgJGRlZmF1bHQgPSBmdW5jdGlvbiB2YWx1ZXMoKXsgcmV0dXJuICRuYXRpdmUuY2FsbCh0aGlzKTsgfTtcbiAgfVxuICAvLyBEZWZpbmUgaXRlcmF0b3JcbiAgaWYoKCFMSUJSQVJZIHx8IEZPUkNFRCkgJiYgKEJVR0dZIHx8IFZBTFVFU19CVUcgfHwgIXByb3RvW0lURVJBVE9SXSkpe1xuICAgIGhpZGUocHJvdG8sIElURVJBVE9SLCAkZGVmYXVsdCk7XG4gIH1cbiAgLy8gUGx1ZyBmb3IgbGlicmFyeVxuICBJdGVyYXRvcnNbTkFNRV0gPSAkZGVmYXVsdDtcbiAgSXRlcmF0b3JzW1RBR10gID0gcmV0dXJuVGhpcztcbiAgaWYoREVGQVVMVCl7XG4gICAgbWV0aG9kcyA9IHtcbiAgICAgIHZhbHVlczogIERFRl9WQUxVRVMgPyAkZGVmYXVsdCA6IGdldE1ldGhvZChWQUxVRVMpLFxuICAgICAga2V5czogICAgSVNfU0VUICAgICA/ICRkZWZhdWx0IDogZ2V0TWV0aG9kKEtFWVMpLFxuICAgICAgZW50cmllczogJGVudHJpZXNcbiAgICB9O1xuICAgIGlmKEZPUkNFRClmb3Ioa2V5IGluIG1ldGhvZHMpe1xuICAgICAgaWYoIShrZXkgaW4gcHJvdG8pKXJlZGVmaW5lKHByb3RvLCBrZXksIG1ldGhvZHNba2V5XSk7XG4gICAgfSBlbHNlICRleHBvcnQoJGV4cG9ydC5QICsgJGV4cG9ydC5GICogKEJVR0dZIHx8IFZBTFVFU19CVUcpLCBOQU1FLCBtZXRob2RzKTtcbiAgfVxuICByZXR1cm4gbWV0aG9kcztcbn07XG5cblxuLyoqIFdFQlBBQ0sgRk9PVEVSICoqXG4gKiogL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL34vY29yZS1qcy9tb2R1bGVzL19pdGVyLWRlZmluZS5qc1xuICoqLyIsIm1vZHVsZS5leHBvcnRzID0gZmFsc2U7XG5cblxuLyoqIFdFQlBBQ0sgRk9PVEVSICoqXG4gKiogL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL34vY29yZS1qcy9tb2R1bGVzL19saWJyYXJ5LmpzXG4gKiovIiwidmFyIGdsb2JhbCAgICA9IHJlcXVpcmUoJy4vX2dsb2JhbCcpXG4gICwgY29yZSAgICAgID0gcmVxdWlyZSgnLi9fY29yZScpXG4gICwgaGlkZSAgICAgID0gcmVxdWlyZSgnLi9faGlkZScpXG4gICwgcmVkZWZpbmUgID0gcmVxdWlyZSgnLi9fcmVkZWZpbmUnKVxuICAsIGN0eCAgICAgICA9IHJlcXVpcmUoJy4vX2N0eCcpXG4gICwgUFJPVE9UWVBFID0gJ3Byb3RvdHlwZSc7XG5cbnZhciAkZXhwb3J0ID0gZnVuY3Rpb24odHlwZSwgbmFtZSwgc291cmNlKXtcbiAgdmFyIElTX0ZPUkNFRCA9IHR5cGUgJiAkZXhwb3J0LkZcbiAgICAsIElTX0dMT0JBTCA9IHR5cGUgJiAkZXhwb3J0LkdcbiAgICAsIElTX1NUQVRJQyA9IHR5cGUgJiAkZXhwb3J0LlNcbiAgICAsIElTX1BST1RPICA9IHR5cGUgJiAkZXhwb3J0LlBcbiAgICAsIElTX0JJTkQgICA9IHR5cGUgJiAkZXhwb3J0LkJcbiAgICAsIHRhcmdldCAgICA9IElTX0dMT0JBTCA/IGdsb2JhbCA6IElTX1NUQVRJQyA/IGdsb2JhbFtuYW1lXSB8fCAoZ2xvYmFsW25hbWVdID0ge30pIDogKGdsb2JhbFtuYW1lXSB8fCB7fSlbUFJPVE9UWVBFXVxuICAgICwgZXhwb3J0cyAgID0gSVNfR0xPQkFMID8gY29yZSA6IGNvcmVbbmFtZV0gfHwgKGNvcmVbbmFtZV0gPSB7fSlcbiAgICAsIGV4cFByb3RvICA9IGV4cG9ydHNbUFJPVE9UWVBFXSB8fCAoZXhwb3J0c1tQUk9UT1RZUEVdID0ge30pXG4gICAgLCBrZXksIG93biwgb3V0LCBleHA7XG4gIGlmKElTX0dMT0JBTClzb3VyY2UgPSBuYW1lO1xuICBmb3Ioa2V5IGluIHNvdXJjZSl7XG4gICAgLy8gY29udGFpbnMgaW4gbmF0aXZlXG4gICAgb3duID0gIUlTX0ZPUkNFRCAmJiB0YXJnZXQgJiYgdGFyZ2V0W2tleV0gIT09IHVuZGVmaW5lZDtcbiAgICAvLyBleHBvcnQgbmF0aXZlIG9yIHBhc3NlZFxuICAgIG91dCA9IChvd24gPyB0YXJnZXQgOiBzb3VyY2UpW2tleV07XG4gICAgLy8gYmluZCB0aW1lcnMgdG8gZ2xvYmFsIGZvciBjYWxsIGZyb20gZXhwb3J0IGNvbnRleHRcbiAgICBleHAgPSBJU19CSU5EICYmIG93biA/IGN0eChvdXQsIGdsb2JhbCkgOiBJU19QUk9UTyAmJiB0eXBlb2Ygb3V0ID09ICdmdW5jdGlvbicgPyBjdHgoRnVuY3Rpb24uY2FsbCwgb3V0KSA6IG91dDtcbiAgICAvLyBleHRlbmQgZ2xvYmFsXG4gICAgaWYodGFyZ2V0KXJlZGVmaW5lKHRhcmdldCwga2V5LCBvdXQsIHR5cGUgJiAkZXhwb3J0LlUpO1xuICAgIC8vIGV4cG9ydFxuICAgIGlmKGV4cG9ydHNba2V5XSAhPSBvdXQpaGlkZShleHBvcnRzLCBrZXksIGV4cCk7XG4gICAgaWYoSVNfUFJPVE8gJiYgZXhwUHJvdG9ba2V5XSAhPSBvdXQpZXhwUHJvdG9ba2V5XSA9IG91dDtcbiAgfVxufTtcbmdsb2JhbC5jb3JlID0gY29yZTtcbi8vIHR5cGUgYml0bWFwXG4kZXhwb3J0LkYgPSAxOyAgIC8vIGZvcmNlZFxuJGV4cG9ydC5HID0gMjsgICAvLyBnbG9iYWxcbiRleHBvcnQuUyA9IDQ7ICAgLy8gc3RhdGljXG4kZXhwb3J0LlAgPSA4OyAgIC8vIHByb3RvXG4kZXhwb3J0LkIgPSAxNjsgIC8vIGJpbmRcbiRleHBvcnQuVyA9IDMyOyAgLy8gd3JhcFxuJGV4cG9ydC5VID0gNjQ7ICAvLyBzYWZlXG4kZXhwb3J0LlIgPSAxMjg7IC8vIHJlYWwgcHJvdG8gbWV0aG9kIGZvciBgbGlicmFyeWAgXG5tb2R1bGUuZXhwb3J0cyA9ICRleHBvcnQ7XG5cblxuLyoqIFdFQlBBQ0sgRk9PVEVSICoqXG4gKiogL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL34vY29yZS1qcy9tb2R1bGVzL19leHBvcnQuanNcbiAqKi8iLCIvLyBvcHRpb25hbCAvIHNpbXBsZSBjb250ZXh0IGJpbmRpbmdcbnZhciBhRnVuY3Rpb24gPSByZXF1aXJlKCcuL19hLWZ1bmN0aW9uJyk7XG5tb2R1bGUuZXhwb3J0cyA9IGZ1bmN0aW9uKGZuLCB0aGF0LCBsZW5ndGgpe1xuICBhRnVuY3Rpb24oZm4pO1xuICBpZih0aGF0ID09PSB1bmRlZmluZWQpcmV0dXJuIGZuO1xuICBzd2l0Y2gobGVuZ3RoKXtcbiAgICBjYXNlIDE6IHJldHVybiBmdW5jdGlvbihhKXtcbiAgICAgIHJldHVybiBmbi5jYWxsKHRoYXQsIGEpO1xuICAgIH07XG4gICAgY2FzZSAyOiByZXR1cm4gZnVuY3Rpb24oYSwgYil7XG4gICAgICByZXR1cm4gZm4uY2FsbCh0aGF0LCBhLCBiKTtcbiAgICB9O1xuICAgIGNhc2UgMzogcmV0dXJuIGZ1bmN0aW9uKGEsIGIsIGMpe1xuICAgICAgcmV0dXJuIGZuLmNhbGwodGhhdCwgYSwgYiwgYyk7XG4gICAgfTtcbiAgfVxuICByZXR1cm4gZnVuY3Rpb24oLyogLi4uYXJncyAqLyl7XG4gICAgcmV0dXJuIGZuLmFwcGx5KHRoYXQsIGFyZ3VtZW50cyk7XG4gIH07XG59O1xuXG5cbi8qKiBXRUJQQUNLIEZPT1RFUiAqKlxuICoqIC9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9fY3R4LmpzXG4gKiovIiwibW9kdWxlLmV4cG9ydHMgPSBmdW5jdGlvbihpdCl7XG4gIGlmKHR5cGVvZiBpdCAhPSAnZnVuY3Rpb24nKXRocm93IFR5cGVFcnJvcihpdCArICcgaXMgbm90IGEgZnVuY3Rpb24hJyk7XG4gIHJldHVybiBpdDtcbn07XG5cblxuLyoqIFdFQlBBQ0sgRk9PVEVSICoqXG4gKiogL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL34vY29yZS1qcy9tb2R1bGVzL19hLWZ1bmN0aW9uLmpzXG4gKiovIiwibW9kdWxlLmV4cG9ydHMgPSB7fTtcblxuXG4vKiogV0VCUEFDSyBGT09URVIgKipcbiAqKiAvVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvfi9jb3JlLWpzL21vZHVsZXMvX2l0ZXJhdG9ycy5qc1xuICoqLyIsIid1c2Ugc3RyaWN0JztcbnZhciBjcmVhdGUgICAgICAgICA9IHJlcXVpcmUoJy4vX29iamVjdC1jcmVhdGUnKVxuICAsIGRlc2NyaXB0b3IgICAgID0gcmVxdWlyZSgnLi9fcHJvcGVydHktZGVzYycpXG4gICwgc2V0VG9TdHJpbmdUYWcgPSByZXF1aXJlKCcuL19zZXQtdG8tc3RyaW5nLXRhZycpXG4gICwgSXRlcmF0b3JQcm90b3R5cGUgPSB7fTtcblxuLy8gMjUuMS4yLjEuMSAlSXRlcmF0b3JQcm90b3R5cGUlW0BAaXRlcmF0b3JdKClcbnJlcXVpcmUoJy4vX2hpZGUnKShJdGVyYXRvclByb3RvdHlwZSwgcmVxdWlyZSgnLi9fd2tzJykoJ2l0ZXJhdG9yJyksIGZ1bmN0aW9uKCl7IHJldHVybiB0aGlzOyB9KTtcblxubW9kdWxlLmV4cG9ydHMgPSBmdW5jdGlvbihDb25zdHJ1Y3RvciwgTkFNRSwgbmV4dCl7XG4gIENvbnN0cnVjdG9yLnByb3RvdHlwZSA9IGNyZWF0ZShJdGVyYXRvclByb3RvdHlwZSwge25leHQ6IGRlc2NyaXB0b3IoMSwgbmV4dCl9KTtcbiAgc2V0VG9TdHJpbmdUYWcoQ29uc3RydWN0b3IsIE5BTUUgKyAnIEl0ZXJhdG9yJyk7XG59O1xuXG5cbi8qKiBXRUJQQUNLIEZPT1RFUiAqKlxuICoqIC9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9faXRlci1jcmVhdGUuanNcbiAqKi8iLCIvLyAxOS4xLjIuMiAvIDE1LjIuMy41IE9iamVjdC5jcmVhdGUoTyBbLCBQcm9wZXJ0aWVzXSlcbnZhciBhbk9iamVjdCAgICA9IHJlcXVpcmUoJy4vX2FuLW9iamVjdCcpXG4gICwgZFBzICAgICAgICAgPSByZXF1aXJlKCcuL19vYmplY3QtZHBzJylcbiAgLCBlbnVtQnVnS2V5cyA9IHJlcXVpcmUoJy4vX2VudW0tYnVnLWtleXMnKVxuICAsIElFX1BST1RPICAgID0gcmVxdWlyZSgnLi9fc2hhcmVkLWtleScpKCdJRV9QUk9UTycpXG4gICwgRW1wdHkgICAgICAgPSBmdW5jdGlvbigpeyAvKiBlbXB0eSAqLyB9XG4gICwgUFJPVE9UWVBFICAgPSAncHJvdG90eXBlJztcblxuLy8gQ3JlYXRlIG9iamVjdCB3aXRoIGZha2UgYG51bGxgIHByb3RvdHlwZTogdXNlIGlmcmFtZSBPYmplY3Qgd2l0aCBjbGVhcmVkIHByb3RvdHlwZVxudmFyIGNyZWF0ZURpY3QgPSBmdW5jdGlvbigpe1xuICAvLyBUaHJhc2gsIHdhc3RlIGFuZCBzb2RvbXk6IElFIEdDIGJ1Z1xuICB2YXIgaWZyYW1lID0gcmVxdWlyZSgnLi9fZG9tLWNyZWF0ZScpKCdpZnJhbWUnKVxuICAgICwgaSAgICAgID0gZW51bUJ1Z0tleXMubGVuZ3RoXG4gICAgLCBsdCAgICAgPSAnPCdcbiAgICAsIGd0ICAgICA9ICc+J1xuICAgICwgaWZyYW1lRG9jdW1lbnQ7XG4gIGlmcmFtZS5zdHlsZS5kaXNwbGF5ID0gJ25vbmUnO1xuICByZXF1aXJlKCcuL19odG1sJykuYXBwZW5kQ2hpbGQoaWZyYW1lKTtcbiAgaWZyYW1lLnNyYyA9ICdqYXZhc2NyaXB0Oic7IC8vIGVzbGludC1kaXNhYmxlLWxpbmUgbm8tc2NyaXB0LXVybFxuICAvLyBjcmVhdGVEaWN0ID0gaWZyYW1lLmNvbnRlbnRXaW5kb3cuT2JqZWN0O1xuICAvLyBodG1sLnJlbW92ZUNoaWxkKGlmcmFtZSk7XG4gIGlmcmFtZURvY3VtZW50ID0gaWZyYW1lLmNvbnRlbnRXaW5kb3cuZG9jdW1lbnQ7XG4gIGlmcmFtZURvY3VtZW50Lm9wZW4oKTtcbiAgaWZyYW1lRG9jdW1lbnQud3JpdGUobHQgKyAnc2NyaXB0JyArIGd0ICsgJ2RvY3VtZW50LkY9T2JqZWN0JyArIGx0ICsgJy9zY3JpcHQnICsgZ3QpO1xuICBpZnJhbWVEb2N1bWVudC5jbG9zZSgpO1xuICBjcmVhdGVEaWN0ID0gaWZyYW1lRG9jdW1lbnQuRjtcbiAgd2hpbGUoaS0tKWRlbGV0ZSBjcmVhdGVEaWN0W1BST1RPVFlQRV1bZW51bUJ1Z0tleXNbaV1dO1xuICByZXR1cm4gY3JlYXRlRGljdCgpO1xufTtcblxubW9kdWxlLmV4cG9ydHMgPSBPYmplY3QuY3JlYXRlIHx8IGZ1bmN0aW9uIGNyZWF0ZShPLCBQcm9wZXJ0aWVzKXtcbiAgdmFyIHJlc3VsdDtcbiAgaWYoTyAhPT0gbnVsbCl7XG4gICAgRW1wdHlbUFJPVE9UWVBFXSA9IGFuT2JqZWN0KE8pO1xuICAgIHJlc3VsdCA9IG5ldyBFbXB0eTtcbiAgICBFbXB0eVtQUk9UT1RZUEVdID0gbnVsbDtcbiAgICAvLyBhZGQgXCJfX3Byb3RvX19cIiBmb3IgT2JqZWN0LmdldFByb3RvdHlwZU9mIHBvbHlmaWxsXG4gICAgcmVzdWx0W0lFX1BST1RPXSA9IE87XG4gIH0gZWxzZSByZXN1bHQgPSBjcmVhdGVEaWN0KCk7XG4gIHJldHVybiBQcm9wZXJ0aWVzID09PSB1bmRlZmluZWQgPyByZXN1bHQgOiBkUHMocmVzdWx0LCBQcm9wZXJ0aWVzKTtcbn07XG5cblxuXG4vKiogV0VCUEFDSyBGT09URVIgKipcbiAqKiAvVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvfi9jb3JlLWpzL21vZHVsZXMvX29iamVjdC1jcmVhdGUuanNcbiAqKi8iLCJ2YXIgZFAgICAgICAgPSByZXF1aXJlKCcuL19vYmplY3QtZHAnKVxuICAsIGFuT2JqZWN0ID0gcmVxdWlyZSgnLi9fYW4tb2JqZWN0JylcbiAgLCBnZXRLZXlzICA9IHJlcXVpcmUoJy4vX29iamVjdC1rZXlzJyk7XG5cbm1vZHVsZS5leHBvcnRzID0gcmVxdWlyZSgnLi9fZGVzY3JpcHRvcnMnKSA/IE9iamVjdC5kZWZpbmVQcm9wZXJ0aWVzIDogZnVuY3Rpb24gZGVmaW5lUHJvcGVydGllcyhPLCBQcm9wZXJ0aWVzKXtcbiAgYW5PYmplY3QoTyk7XG4gIHZhciBrZXlzICAgPSBnZXRLZXlzKFByb3BlcnRpZXMpXG4gICAgLCBsZW5ndGggPSBrZXlzLmxlbmd0aFxuICAgICwgaSA9IDBcbiAgICAsIFA7XG4gIHdoaWxlKGxlbmd0aCA+IGkpZFAuZihPLCBQID0ga2V5c1tpKytdLCBQcm9wZXJ0aWVzW1BdKTtcbiAgcmV0dXJuIE87XG59O1xuXG5cbi8qKiBXRUJQQUNLIEZPT1RFUiAqKlxuICoqIC9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9fb2JqZWN0LWRwcy5qc1xuICoqLyIsIi8vIDE5LjEuMi4xNCAvIDE1LjIuMy4xNCBPYmplY3Qua2V5cyhPKVxudmFyICRrZXlzICAgICAgID0gcmVxdWlyZSgnLi9fb2JqZWN0LWtleXMtaW50ZXJuYWwnKVxuICAsIGVudW1CdWdLZXlzID0gcmVxdWlyZSgnLi9fZW51bS1idWcta2V5cycpO1xuXG5tb2R1bGUuZXhwb3J0cyA9IE9iamVjdC5rZXlzIHx8IGZ1bmN0aW9uIGtleXMoTyl7XG4gIHJldHVybiAka2V5cyhPLCBlbnVtQnVnS2V5cyk7XG59O1xuXG5cbi8qKiBXRUJQQUNLIEZPT1RFUiAqKlxuICoqIC9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9fb2JqZWN0LWtleXMuanNcbiAqKi8iLCJ2YXIgaGFzICAgICAgICAgID0gcmVxdWlyZSgnLi9faGFzJylcbiAgLCB0b0lPYmplY3QgICAgPSByZXF1aXJlKCcuL190by1pb2JqZWN0JylcbiAgLCBhcnJheUluZGV4T2YgPSByZXF1aXJlKCcuL19hcnJheS1pbmNsdWRlcycpKGZhbHNlKVxuICAsIElFX1BST1RPICAgICA9IHJlcXVpcmUoJy4vX3NoYXJlZC1rZXknKSgnSUVfUFJPVE8nKTtcblxubW9kdWxlLmV4cG9ydHMgPSBmdW5jdGlvbihvYmplY3QsIG5hbWVzKXtcbiAgdmFyIE8gICAgICA9IHRvSU9iamVjdChvYmplY3QpXG4gICAgLCBpICAgICAgPSAwXG4gICAgLCByZXN1bHQgPSBbXVxuICAgICwga2V5O1xuICBmb3Ioa2V5IGluIE8paWYoa2V5ICE9IElFX1BST1RPKWhhcyhPLCBrZXkpICYmIHJlc3VsdC5wdXNoKGtleSk7XG4gIC8vIERvbid0IGVudW0gYnVnICYgaGlkZGVuIGtleXNcbiAgd2hpbGUobmFtZXMubGVuZ3RoID4gaSlpZihoYXMoTywga2V5ID0gbmFtZXNbaSsrXSkpe1xuICAgIH5hcnJheUluZGV4T2YocmVzdWx0LCBrZXkpIHx8IHJlc3VsdC5wdXNoKGtleSk7XG4gIH1cbiAgcmV0dXJuIHJlc3VsdDtcbn07XG5cblxuLyoqIFdFQlBBQ0sgRk9PVEVSICoqXG4gKiogL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL34vY29yZS1qcy9tb2R1bGVzL19vYmplY3Qta2V5cy1pbnRlcm5hbC5qc1xuICoqLyIsIi8vIHRvIGluZGV4ZWQgb2JqZWN0LCB0b09iamVjdCB3aXRoIGZhbGxiYWNrIGZvciBub24tYXJyYXktbGlrZSBFUzMgc3RyaW5nc1xudmFyIElPYmplY3QgPSByZXF1aXJlKCcuL19pb2JqZWN0JylcbiAgLCBkZWZpbmVkID0gcmVxdWlyZSgnLi9fZGVmaW5lZCcpO1xubW9kdWxlLmV4cG9ydHMgPSBmdW5jdGlvbihpdCl7XG4gIHJldHVybiBJT2JqZWN0KGRlZmluZWQoaXQpKTtcbn07XG5cblxuLyoqIFdFQlBBQ0sgRk9PVEVSICoqXG4gKiogL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL34vY29yZS1qcy9tb2R1bGVzL190by1pb2JqZWN0LmpzXG4gKiovIiwiLy8gZmFsbGJhY2sgZm9yIG5vbi1hcnJheS1saWtlIEVTMyBhbmQgbm9uLWVudW1lcmFibGUgb2xkIFY4IHN0cmluZ3NcbnZhciBjb2YgPSByZXF1aXJlKCcuL19jb2YnKTtcbm1vZHVsZS5leHBvcnRzID0gT2JqZWN0KCd6JykucHJvcGVydHlJc0VudW1lcmFibGUoMCkgPyBPYmplY3QgOiBmdW5jdGlvbihpdCl7XG4gIHJldHVybiBjb2YoaXQpID09ICdTdHJpbmcnID8gaXQuc3BsaXQoJycpIDogT2JqZWN0KGl0KTtcbn07XG5cblxuLyoqIFdFQlBBQ0sgRk9PVEVSICoqXG4gKiogL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL34vY29yZS1qcy9tb2R1bGVzL19pb2JqZWN0LmpzXG4gKiovIiwiLy8gZmFsc2UgLT4gQXJyYXkjaW5kZXhPZlxuLy8gdHJ1ZSAgLT4gQXJyYXkjaW5jbHVkZXNcbnZhciB0b0lPYmplY3QgPSByZXF1aXJlKCcuL190by1pb2JqZWN0JylcbiAgLCB0b0xlbmd0aCAgPSByZXF1aXJlKCcuL190by1sZW5ndGgnKVxuICAsIHRvSW5kZXggICA9IHJlcXVpcmUoJy4vX3RvLWluZGV4Jyk7XG5tb2R1bGUuZXhwb3J0cyA9IGZ1bmN0aW9uKElTX0lOQ0xVREVTKXtcbiAgcmV0dXJuIGZ1bmN0aW9uKCR0aGlzLCBlbCwgZnJvbUluZGV4KXtcbiAgICB2YXIgTyAgICAgID0gdG9JT2JqZWN0KCR0aGlzKVxuICAgICAgLCBsZW5ndGggPSB0b0xlbmd0aChPLmxlbmd0aClcbiAgICAgICwgaW5kZXggID0gdG9JbmRleChmcm9tSW5kZXgsIGxlbmd0aClcbiAgICAgICwgdmFsdWU7XG4gICAgLy8gQXJyYXkjaW5jbHVkZXMgdXNlcyBTYW1lVmFsdWVaZXJvIGVxdWFsaXR5IGFsZ29yaXRobVxuICAgIGlmKElTX0lOQ0xVREVTICYmIGVsICE9IGVsKXdoaWxlKGxlbmd0aCA+IGluZGV4KXtcbiAgICAgIHZhbHVlID0gT1tpbmRleCsrXTtcbiAgICAgIGlmKHZhbHVlICE9IHZhbHVlKXJldHVybiB0cnVlO1xuICAgIC8vIEFycmF5I3RvSW5kZXggaWdub3JlcyBob2xlcywgQXJyYXkjaW5jbHVkZXMgLSBub3RcbiAgICB9IGVsc2UgZm9yKDtsZW5ndGggPiBpbmRleDsgaW5kZXgrKylpZihJU19JTkNMVURFUyB8fCBpbmRleCBpbiBPKXtcbiAgICAgIGlmKE9baW5kZXhdID09PSBlbClyZXR1cm4gSVNfSU5DTFVERVMgfHwgaW5kZXggfHwgMDtcbiAgICB9IHJldHVybiAhSVNfSU5DTFVERVMgJiYgLTE7XG4gIH07XG59O1xuXG5cbi8qKiBXRUJQQUNLIEZPT1RFUiAqKlxuICoqIC9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9fYXJyYXktaW5jbHVkZXMuanNcbiAqKi8iLCIvLyA3LjEuMTUgVG9MZW5ndGhcbnZhciB0b0ludGVnZXIgPSByZXF1aXJlKCcuL190by1pbnRlZ2VyJylcbiAgLCBtaW4gICAgICAgPSBNYXRoLm1pbjtcbm1vZHVsZS5leHBvcnRzID0gZnVuY3Rpb24oaXQpe1xuICByZXR1cm4gaXQgPiAwID8gbWluKHRvSW50ZWdlcihpdCksIDB4MWZmZmZmZmZmZmZmZmYpIDogMDsgLy8gcG93KDIsIDUzKSAtIDEgPT0gOTAwNzE5OTI1NDc0MDk5MVxufTtcblxuXG4vKiogV0VCUEFDSyBGT09URVIgKipcbiAqKiAvVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvfi9jb3JlLWpzL21vZHVsZXMvX3RvLWxlbmd0aC5qc1xuICoqLyIsInZhciB0b0ludGVnZXIgPSByZXF1aXJlKCcuL190by1pbnRlZ2VyJylcbiAgLCBtYXggICAgICAgPSBNYXRoLm1heFxuICAsIG1pbiAgICAgICA9IE1hdGgubWluO1xubW9kdWxlLmV4cG9ydHMgPSBmdW5jdGlvbihpbmRleCwgbGVuZ3RoKXtcbiAgaW5kZXggPSB0b0ludGVnZXIoaW5kZXgpO1xuICByZXR1cm4gaW5kZXggPCAwID8gbWF4KGluZGV4ICsgbGVuZ3RoLCAwKSA6IG1pbihpbmRleCwgbGVuZ3RoKTtcbn07XG5cblxuLyoqIFdFQlBBQ0sgRk9PVEVSICoqXG4gKiogL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL34vY29yZS1qcy9tb2R1bGVzL190by1pbmRleC5qc1xuICoqLyIsInZhciBzaGFyZWQgPSByZXF1aXJlKCcuL19zaGFyZWQnKSgna2V5cycpXG4gICwgdWlkICAgID0gcmVxdWlyZSgnLi9fdWlkJyk7XG5tb2R1bGUuZXhwb3J0cyA9IGZ1bmN0aW9uKGtleSl7XG4gIHJldHVybiBzaGFyZWRba2V5XSB8fCAoc2hhcmVkW2tleV0gPSB1aWQoa2V5KSk7XG59O1xuXG5cbi8qKiBXRUJQQUNLIEZPT1RFUiAqKlxuICoqIC9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9fc2hhcmVkLWtleS5qc1xuICoqLyIsIi8vIElFIDgtIGRvbid0IGVudW0gYnVnIGtleXNcbm1vZHVsZS5leHBvcnRzID0gKFxuICAnY29uc3RydWN0b3IsaGFzT3duUHJvcGVydHksaXNQcm90b3R5cGVPZixwcm9wZXJ0eUlzRW51bWVyYWJsZSx0b0xvY2FsZVN0cmluZyx0b1N0cmluZyx2YWx1ZU9mJ1xuKS5zcGxpdCgnLCcpO1xuXG5cbi8qKiBXRUJQQUNLIEZPT1RFUiAqKlxuICoqIC9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9fZW51bS1idWcta2V5cy5qc1xuICoqLyIsIm1vZHVsZS5leHBvcnRzID0gcmVxdWlyZSgnLi9fZ2xvYmFsJykuZG9jdW1lbnQgJiYgZG9jdW1lbnQuZG9jdW1lbnRFbGVtZW50O1xuXG5cbi8qKiBXRUJQQUNLIEZPT1RFUiAqKlxuICoqIC9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9faHRtbC5qc1xuICoqLyIsInZhciBkZWYgPSByZXF1aXJlKCcuL19vYmplY3QtZHAnKS5mXG4gICwgaGFzID0gcmVxdWlyZSgnLi9faGFzJylcbiAgLCBUQUcgPSByZXF1aXJlKCcuL193a3MnKSgndG9TdHJpbmdUYWcnKTtcblxubW9kdWxlLmV4cG9ydHMgPSBmdW5jdGlvbihpdCwgdGFnLCBzdGF0KXtcbiAgaWYoaXQgJiYgIWhhcyhpdCA9IHN0YXQgPyBpdCA6IGl0LnByb3RvdHlwZSwgVEFHKSlkZWYoaXQsIFRBRywge2NvbmZpZ3VyYWJsZTogdHJ1ZSwgdmFsdWU6IHRhZ30pO1xufTtcblxuXG4vKiogV0VCUEFDSyBGT09URVIgKipcbiAqKiAvVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvfi9jb3JlLWpzL21vZHVsZXMvX3NldC10by1zdHJpbmctdGFnLmpzXG4gKiovIiwiLy8gMTkuMS4yLjkgLyAxNS4yLjMuMiBPYmplY3QuZ2V0UHJvdG90eXBlT2YoTylcbnZhciBoYXMgICAgICAgICA9IHJlcXVpcmUoJy4vX2hhcycpXG4gICwgdG9PYmplY3QgICAgPSByZXF1aXJlKCcuL190by1vYmplY3QnKVxuICAsIElFX1BST1RPICAgID0gcmVxdWlyZSgnLi9fc2hhcmVkLWtleScpKCdJRV9QUk9UTycpXG4gICwgT2JqZWN0UHJvdG8gPSBPYmplY3QucHJvdG90eXBlO1xuXG5tb2R1bGUuZXhwb3J0cyA9IE9iamVjdC5nZXRQcm90b3R5cGVPZiB8fCBmdW5jdGlvbihPKXtcbiAgTyA9IHRvT2JqZWN0KE8pO1xuICBpZihoYXMoTywgSUVfUFJPVE8pKXJldHVybiBPW0lFX1BST1RPXTtcbiAgaWYodHlwZW9mIE8uY29uc3RydWN0b3IgPT0gJ2Z1bmN0aW9uJyAmJiBPIGluc3RhbmNlb2YgTy5jb25zdHJ1Y3Rvcil7XG4gICAgcmV0dXJuIE8uY29uc3RydWN0b3IucHJvdG90eXBlO1xuICB9IHJldHVybiBPIGluc3RhbmNlb2YgT2JqZWN0ID8gT2JqZWN0UHJvdG8gOiBudWxsO1xufTtcblxuXG4vKiogV0VCUEFDSyBGT09URVIgKipcbiAqKiAvVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvfi9jb3JlLWpzL21vZHVsZXMvX29iamVjdC1ncG8uanNcbiAqKi8iLCIvLyA3LjEuMTMgVG9PYmplY3QoYXJndW1lbnQpXG52YXIgZGVmaW5lZCA9IHJlcXVpcmUoJy4vX2RlZmluZWQnKTtcbm1vZHVsZS5leHBvcnRzID0gZnVuY3Rpb24oaXQpe1xuICByZXR1cm4gT2JqZWN0KGRlZmluZWQoaXQpKTtcbn07XG5cblxuLyoqIFdFQlBBQ0sgRk9PVEVSICoqXG4gKiogL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL34vY29yZS1qcy9tb2R1bGVzL190by1vYmplY3QuanNcbiAqKi8iLCJ2YXIgJGl0ZXJhdG9ycyAgICA9IHJlcXVpcmUoJy4vZXM2LmFycmF5Lml0ZXJhdG9yJylcbiAgLCByZWRlZmluZSAgICAgID0gcmVxdWlyZSgnLi9fcmVkZWZpbmUnKVxuICAsIGdsb2JhbCAgICAgICAgPSByZXF1aXJlKCcuL19nbG9iYWwnKVxuICAsIGhpZGUgICAgICAgICAgPSByZXF1aXJlKCcuL19oaWRlJylcbiAgLCBJdGVyYXRvcnMgICAgID0gcmVxdWlyZSgnLi9faXRlcmF0b3JzJylcbiAgLCB3a3MgICAgICAgICAgID0gcmVxdWlyZSgnLi9fd2tzJylcbiAgLCBJVEVSQVRPUiAgICAgID0gd2tzKCdpdGVyYXRvcicpXG4gICwgVE9fU1RSSU5HX1RBRyA9IHdrcygndG9TdHJpbmdUYWcnKVxuICAsIEFycmF5VmFsdWVzICAgPSBJdGVyYXRvcnMuQXJyYXk7XG5cbmZvcih2YXIgY29sbGVjdGlvbnMgPSBbJ05vZGVMaXN0JywgJ0RPTVRva2VuTGlzdCcsICdNZWRpYUxpc3QnLCAnU3R5bGVTaGVldExpc3QnLCAnQ1NTUnVsZUxpc3QnXSwgaSA9IDA7IGkgPCA1OyBpKyspe1xuICB2YXIgTkFNRSAgICAgICA9IGNvbGxlY3Rpb25zW2ldXG4gICAgLCBDb2xsZWN0aW9uID0gZ2xvYmFsW05BTUVdXG4gICAgLCBwcm90byAgICAgID0gQ29sbGVjdGlvbiAmJiBDb2xsZWN0aW9uLnByb3RvdHlwZVxuICAgICwga2V5O1xuICBpZihwcm90byl7XG4gICAgaWYoIXByb3RvW0lURVJBVE9SXSloaWRlKHByb3RvLCBJVEVSQVRPUiwgQXJyYXlWYWx1ZXMpO1xuICAgIGlmKCFwcm90b1tUT19TVFJJTkdfVEFHXSloaWRlKHByb3RvLCBUT19TVFJJTkdfVEFHLCBOQU1FKTtcbiAgICBJdGVyYXRvcnNbTkFNRV0gPSBBcnJheVZhbHVlcztcbiAgICBmb3Ioa2V5IGluICRpdGVyYXRvcnMpaWYoIXByb3RvW2tleV0pcmVkZWZpbmUocHJvdG8sIGtleSwgJGl0ZXJhdG9yc1trZXldLCB0cnVlKTtcbiAgfVxufVxuXG5cbi8qKiBXRUJQQUNLIEZPT1RFUiAqKlxuICoqIC9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy93ZWIuZG9tLml0ZXJhYmxlLmpzXG4gKiovIiwiJ3VzZSBzdHJpY3QnO1xudmFyIGFkZFRvVW5zY29wYWJsZXMgPSByZXF1aXJlKCcuL19hZGQtdG8tdW5zY29wYWJsZXMnKVxuICAsIHN0ZXAgICAgICAgICAgICAgPSByZXF1aXJlKCcuL19pdGVyLXN0ZXAnKVxuICAsIEl0ZXJhdG9ycyAgICAgICAgPSByZXF1aXJlKCcuL19pdGVyYXRvcnMnKVxuICAsIHRvSU9iamVjdCAgICAgICAgPSByZXF1aXJlKCcuL190by1pb2JqZWN0Jyk7XG5cbi8vIDIyLjEuMy40IEFycmF5LnByb3RvdHlwZS5lbnRyaWVzKClcbi8vIDIyLjEuMy4xMyBBcnJheS5wcm90b3R5cGUua2V5cygpXG4vLyAyMi4xLjMuMjkgQXJyYXkucHJvdG90eXBlLnZhbHVlcygpXG4vLyAyMi4xLjMuMzAgQXJyYXkucHJvdG90eXBlW0BAaXRlcmF0b3JdKClcbm1vZHVsZS5leHBvcnRzID0gcmVxdWlyZSgnLi9faXRlci1kZWZpbmUnKShBcnJheSwgJ0FycmF5JywgZnVuY3Rpb24oaXRlcmF0ZWQsIGtpbmQpe1xuICB0aGlzLl90ID0gdG9JT2JqZWN0KGl0ZXJhdGVkKTsgLy8gdGFyZ2V0XG4gIHRoaXMuX2kgPSAwOyAgICAgICAgICAgICAgICAgICAvLyBuZXh0IGluZGV4XG4gIHRoaXMuX2sgPSBraW5kOyAgICAgICAgICAgICAgICAvLyBraW5kXG4vLyAyMi4xLjUuMi4xICVBcnJheUl0ZXJhdG9yUHJvdG90eXBlJS5uZXh0KClcbn0sIGZ1bmN0aW9uKCl7XG4gIHZhciBPICAgICA9IHRoaXMuX3RcbiAgICAsIGtpbmQgID0gdGhpcy5fa1xuICAgICwgaW5kZXggPSB0aGlzLl9pKys7XG4gIGlmKCFPIHx8IGluZGV4ID49IE8ubGVuZ3RoKXtcbiAgICB0aGlzLl90ID0gdW5kZWZpbmVkO1xuICAgIHJldHVybiBzdGVwKDEpO1xuICB9XG4gIGlmKGtpbmQgPT0gJ2tleXMnICApcmV0dXJuIHN0ZXAoMCwgaW5kZXgpO1xuICBpZihraW5kID09ICd2YWx1ZXMnKXJldHVybiBzdGVwKDAsIE9baW5kZXhdKTtcbiAgcmV0dXJuIHN0ZXAoMCwgW2luZGV4LCBPW2luZGV4XV0pO1xufSwgJ3ZhbHVlcycpO1xuXG4vLyBhcmd1bWVudHNMaXN0W0BAaXRlcmF0b3JdIGlzICVBcnJheVByb3RvX3ZhbHVlcyUgKDkuNC40LjYsIDkuNC40LjcpXG5JdGVyYXRvcnMuQXJndW1lbnRzID0gSXRlcmF0b3JzLkFycmF5O1xuXG5hZGRUb1Vuc2NvcGFibGVzKCdrZXlzJyk7XG5hZGRUb1Vuc2NvcGFibGVzKCd2YWx1ZXMnKTtcbmFkZFRvVW5zY29wYWJsZXMoJ2VudHJpZXMnKTtcblxuXG4vKiogV0VCUEFDSyBGT09URVIgKipcbiAqKiAvVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvfi9jb3JlLWpzL21vZHVsZXMvZXM2LmFycmF5Lml0ZXJhdG9yLmpzXG4gKiovIiwiLy8gMjIuMS4zLjMxIEFycmF5LnByb3RvdHlwZVtAQHVuc2NvcGFibGVzXVxudmFyIFVOU0NPUEFCTEVTID0gcmVxdWlyZSgnLi9fd2tzJykoJ3Vuc2NvcGFibGVzJylcbiAgLCBBcnJheVByb3RvICA9IEFycmF5LnByb3RvdHlwZTtcbmlmKEFycmF5UHJvdG9bVU5TQ09QQUJMRVNdID09IHVuZGVmaW5lZClyZXF1aXJlKCcuL19oaWRlJykoQXJyYXlQcm90bywgVU5TQ09QQUJMRVMsIHt9KTtcbm1vZHVsZS5leHBvcnRzID0gZnVuY3Rpb24oa2V5KXtcbiAgQXJyYXlQcm90b1tVTlNDT1BBQkxFU11ba2V5XSA9IHRydWU7XG59O1xuXG5cbi8qKiBXRUJQQUNLIEZPT1RFUiAqKlxuICoqIC9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9fYWRkLXRvLXVuc2NvcGFibGVzLmpzXG4gKiovIiwibW9kdWxlLmV4cG9ydHMgPSBmdW5jdGlvbihkb25lLCB2YWx1ZSl7XG4gIHJldHVybiB7dmFsdWU6IHZhbHVlLCBkb25lOiAhIWRvbmV9O1xufTtcblxuXG4vKiogV0VCUEFDSyBGT09URVIgKipcbiAqKiAvVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvfi9jb3JlLWpzL21vZHVsZXMvX2l0ZXItc3RlcC5qc1xuICoqLyIsIid1c2Ugc3RyaWN0JztcbnZhciBMSUJSQVJZICAgICAgICAgICAgPSByZXF1aXJlKCcuL19saWJyYXJ5JylcbiAgLCBnbG9iYWwgICAgICAgICAgICAgPSByZXF1aXJlKCcuL19nbG9iYWwnKVxuICAsIGN0eCAgICAgICAgICAgICAgICA9IHJlcXVpcmUoJy4vX2N0eCcpXG4gICwgY2xhc3NvZiAgICAgICAgICAgID0gcmVxdWlyZSgnLi9fY2xhc3NvZicpXG4gICwgJGV4cG9ydCAgICAgICAgICAgID0gcmVxdWlyZSgnLi9fZXhwb3J0JylcbiAgLCBpc09iamVjdCAgICAgICAgICAgPSByZXF1aXJlKCcuL19pcy1vYmplY3QnKVxuICAsIGFGdW5jdGlvbiAgICAgICAgICA9IHJlcXVpcmUoJy4vX2EtZnVuY3Rpb24nKVxuICAsIGFuSW5zdGFuY2UgICAgICAgICA9IHJlcXVpcmUoJy4vX2FuLWluc3RhbmNlJylcbiAgLCBmb3JPZiAgICAgICAgICAgICAgPSByZXF1aXJlKCcuL19mb3Itb2YnKVxuICAsIHNwZWNpZXNDb25zdHJ1Y3RvciA9IHJlcXVpcmUoJy4vX3NwZWNpZXMtY29uc3RydWN0b3InKVxuICAsIHRhc2sgICAgICAgICAgICAgICA9IHJlcXVpcmUoJy4vX3Rhc2snKS5zZXRcbiAgLCBtaWNyb3Rhc2sgICAgICAgICAgPSByZXF1aXJlKCcuL19taWNyb3Rhc2snKSgpXG4gICwgUFJPTUlTRSAgICAgICAgICAgID0gJ1Byb21pc2UnXG4gICwgVHlwZUVycm9yICAgICAgICAgID0gZ2xvYmFsLlR5cGVFcnJvclxuICAsIHByb2Nlc3MgICAgICAgICAgICA9IGdsb2JhbC5wcm9jZXNzXG4gICwgJFByb21pc2UgICAgICAgICAgID0gZ2xvYmFsW1BST01JU0VdXG4gICwgcHJvY2VzcyAgICAgICAgICAgID0gZ2xvYmFsLnByb2Nlc3NcbiAgLCBpc05vZGUgICAgICAgICAgICAgPSBjbGFzc29mKHByb2Nlc3MpID09ICdwcm9jZXNzJ1xuICAsIGVtcHR5ICAgICAgICAgICAgICA9IGZ1bmN0aW9uKCl7IC8qIGVtcHR5ICovIH1cbiAgLCBJbnRlcm5hbCwgR2VuZXJpY1Byb21pc2VDYXBhYmlsaXR5LCBXcmFwcGVyO1xuXG52YXIgVVNFX05BVElWRSA9ICEhZnVuY3Rpb24oKXtcbiAgdHJ5IHtcbiAgICAvLyBjb3JyZWN0IHN1YmNsYXNzaW5nIHdpdGggQEBzcGVjaWVzIHN1cHBvcnRcbiAgICB2YXIgcHJvbWlzZSAgICAgPSAkUHJvbWlzZS5yZXNvbHZlKDEpXG4gICAgICAsIEZha2VQcm9taXNlID0gKHByb21pc2UuY29uc3RydWN0b3IgPSB7fSlbcmVxdWlyZSgnLi9fd2tzJykoJ3NwZWNpZXMnKV0gPSBmdW5jdGlvbihleGVjKXsgZXhlYyhlbXB0eSwgZW1wdHkpOyB9O1xuICAgIC8vIHVuaGFuZGxlZCByZWplY3Rpb25zIHRyYWNraW5nIHN1cHBvcnQsIE5vZGVKUyBQcm9taXNlIHdpdGhvdXQgaXQgZmFpbHMgQEBzcGVjaWVzIHRlc3RcbiAgICByZXR1cm4gKGlzTm9kZSB8fCB0eXBlb2YgUHJvbWlzZVJlamVjdGlvbkV2ZW50ID09ICdmdW5jdGlvbicpICYmIHByb21pc2UudGhlbihlbXB0eSkgaW5zdGFuY2VvZiBGYWtlUHJvbWlzZTtcbiAgfSBjYXRjaChlKXsgLyogZW1wdHkgKi8gfVxufSgpO1xuXG4vLyBoZWxwZXJzXG52YXIgc2FtZUNvbnN0cnVjdG9yID0gZnVuY3Rpb24oYSwgYil7XG4gIC8vIHdpdGggbGlicmFyeSB3cmFwcGVyIHNwZWNpYWwgY2FzZVxuICByZXR1cm4gYSA9PT0gYiB8fCBhID09PSAkUHJvbWlzZSAmJiBiID09PSBXcmFwcGVyO1xufTtcbnZhciBpc1RoZW5hYmxlID0gZnVuY3Rpb24oaXQpe1xuICB2YXIgdGhlbjtcbiAgcmV0dXJuIGlzT2JqZWN0KGl0KSAmJiB0eXBlb2YgKHRoZW4gPSBpdC50aGVuKSA9PSAnZnVuY3Rpb24nID8gdGhlbiA6IGZhbHNlO1xufTtcbnZhciBuZXdQcm9taXNlQ2FwYWJpbGl0eSA9IGZ1bmN0aW9uKEMpe1xuICByZXR1cm4gc2FtZUNvbnN0cnVjdG9yKCRQcm9taXNlLCBDKVxuICAgID8gbmV3IFByb21pc2VDYXBhYmlsaXR5KEMpXG4gICAgOiBuZXcgR2VuZXJpY1Byb21pc2VDYXBhYmlsaXR5KEMpO1xufTtcbnZhciBQcm9taXNlQ2FwYWJpbGl0eSA9IEdlbmVyaWNQcm9taXNlQ2FwYWJpbGl0eSA9IGZ1bmN0aW9uKEMpe1xuICB2YXIgcmVzb2x2ZSwgcmVqZWN0O1xuICB0aGlzLnByb21pc2UgPSBuZXcgQyhmdW5jdGlvbigkJHJlc29sdmUsICQkcmVqZWN0KXtcbiAgICBpZihyZXNvbHZlICE9PSB1bmRlZmluZWQgfHwgcmVqZWN0ICE9PSB1bmRlZmluZWQpdGhyb3cgVHlwZUVycm9yKCdCYWQgUHJvbWlzZSBjb25zdHJ1Y3RvcicpO1xuICAgIHJlc29sdmUgPSAkJHJlc29sdmU7XG4gICAgcmVqZWN0ICA9ICQkcmVqZWN0O1xuICB9KTtcbiAgdGhpcy5yZXNvbHZlID0gYUZ1bmN0aW9uKHJlc29sdmUpO1xuICB0aGlzLnJlamVjdCAgPSBhRnVuY3Rpb24ocmVqZWN0KTtcbn07XG52YXIgcGVyZm9ybSA9IGZ1bmN0aW9uKGV4ZWMpe1xuICB0cnkge1xuICAgIGV4ZWMoKTtcbiAgfSBjYXRjaChlKXtcbiAgICByZXR1cm4ge2Vycm9yOiBlfTtcbiAgfVxufTtcbnZhciBub3RpZnkgPSBmdW5jdGlvbihwcm9taXNlLCBpc1JlamVjdCl7XG4gIGlmKHByb21pc2UuX24pcmV0dXJuO1xuICBwcm9taXNlLl9uID0gdHJ1ZTtcbiAgdmFyIGNoYWluID0gcHJvbWlzZS5fYztcbiAgbWljcm90YXNrKGZ1bmN0aW9uKCl7XG4gICAgdmFyIHZhbHVlID0gcHJvbWlzZS5fdlxuICAgICAgLCBvayAgICA9IHByb21pc2UuX3MgPT0gMVxuICAgICAgLCBpICAgICA9IDA7XG4gICAgdmFyIHJ1biA9IGZ1bmN0aW9uKHJlYWN0aW9uKXtcbiAgICAgIHZhciBoYW5kbGVyID0gb2sgPyByZWFjdGlvbi5vayA6IHJlYWN0aW9uLmZhaWxcbiAgICAgICAgLCByZXNvbHZlID0gcmVhY3Rpb24ucmVzb2x2ZVxuICAgICAgICAsIHJlamVjdCAgPSByZWFjdGlvbi5yZWplY3RcbiAgICAgICAgLCBkb21haW4gID0gcmVhY3Rpb24uZG9tYWluXG4gICAgICAgICwgcmVzdWx0LCB0aGVuO1xuICAgICAgdHJ5IHtcbiAgICAgICAgaWYoaGFuZGxlcil7XG4gICAgICAgICAgaWYoIW9rKXtcbiAgICAgICAgICAgIGlmKHByb21pc2UuX2ggPT0gMilvbkhhbmRsZVVuaGFuZGxlZChwcm9taXNlKTtcbiAgICAgICAgICAgIHByb21pc2UuX2ggPSAxO1xuICAgICAgICAgIH1cbiAgICAgICAgICBpZihoYW5kbGVyID09PSB0cnVlKXJlc3VsdCA9IHZhbHVlO1xuICAgICAgICAgIGVsc2Uge1xuICAgICAgICAgICAgaWYoZG9tYWluKWRvbWFpbi5lbnRlcigpO1xuICAgICAgICAgICAgcmVzdWx0ID0gaGFuZGxlcih2YWx1ZSk7XG4gICAgICAgICAgICBpZihkb21haW4pZG9tYWluLmV4aXQoKTtcbiAgICAgICAgICB9XG4gICAgICAgICAgaWYocmVzdWx0ID09PSByZWFjdGlvbi5wcm9taXNlKXtcbiAgICAgICAgICAgIHJlamVjdChUeXBlRXJyb3IoJ1Byb21pc2UtY2hhaW4gY3ljbGUnKSk7XG4gICAgICAgICAgfSBlbHNlIGlmKHRoZW4gPSBpc1RoZW5hYmxlKHJlc3VsdCkpe1xuICAgICAgICAgICAgdGhlbi5jYWxsKHJlc3VsdCwgcmVzb2x2ZSwgcmVqZWN0KTtcbiAgICAgICAgICB9IGVsc2UgcmVzb2x2ZShyZXN1bHQpO1xuICAgICAgICB9IGVsc2UgcmVqZWN0KHZhbHVlKTtcbiAgICAgIH0gY2F0Y2goZSl7XG4gICAgICAgIHJlamVjdChlKTtcbiAgICAgIH1cbiAgICB9O1xuICAgIHdoaWxlKGNoYWluLmxlbmd0aCA+IGkpcnVuKGNoYWluW2krK10pOyAvLyB2YXJpYWJsZSBsZW5ndGggLSBjYW4ndCB1c2UgZm9yRWFjaFxuICAgIHByb21pc2UuX2MgPSBbXTtcbiAgICBwcm9taXNlLl9uID0gZmFsc2U7XG4gICAgaWYoaXNSZWplY3QgJiYgIXByb21pc2UuX2gpb25VbmhhbmRsZWQocHJvbWlzZSk7XG4gIH0pO1xufTtcbnZhciBvblVuaGFuZGxlZCA9IGZ1bmN0aW9uKHByb21pc2Upe1xuICB0YXNrLmNhbGwoZ2xvYmFsLCBmdW5jdGlvbigpe1xuICAgIHZhciB2YWx1ZSA9IHByb21pc2UuX3ZcbiAgICAgICwgYWJydXB0LCBoYW5kbGVyLCBjb25zb2xlO1xuICAgIGlmKGlzVW5oYW5kbGVkKHByb21pc2UpKXtcbiAgICAgIGFicnVwdCA9IHBlcmZvcm0oZnVuY3Rpb24oKXtcbiAgICAgICAgaWYoaXNOb2RlKXtcbiAgICAgICAgICBwcm9jZXNzLmVtaXQoJ3VuaGFuZGxlZFJlamVjdGlvbicsIHZhbHVlLCBwcm9taXNlKTtcbiAgICAgICAgfSBlbHNlIGlmKGhhbmRsZXIgPSBnbG9iYWwub251bmhhbmRsZWRyZWplY3Rpb24pe1xuICAgICAgICAgIGhhbmRsZXIoe3Byb21pc2U6IHByb21pc2UsIHJlYXNvbjogdmFsdWV9KTtcbiAgICAgICAgfSBlbHNlIGlmKChjb25zb2xlID0gZ2xvYmFsLmNvbnNvbGUpICYmIGNvbnNvbGUuZXJyb3Ipe1xuICAgICAgICAgIGNvbnNvbGUuZXJyb3IoJ1VuaGFuZGxlZCBwcm9taXNlIHJlamVjdGlvbicsIHZhbHVlKTtcbiAgICAgICAgfVxuICAgICAgfSk7XG4gICAgICAvLyBCcm93c2VycyBzaG91bGQgbm90IHRyaWdnZXIgYHJlamVjdGlvbkhhbmRsZWRgIGV2ZW50IGlmIGl0IHdhcyBoYW5kbGVkIGhlcmUsIE5vZGVKUyAtIHNob3VsZFxuICAgICAgcHJvbWlzZS5faCA9IGlzTm9kZSB8fCBpc1VuaGFuZGxlZChwcm9taXNlKSA/IDIgOiAxO1xuICAgIH0gcHJvbWlzZS5fYSA9IHVuZGVmaW5lZDtcbiAgICBpZihhYnJ1cHQpdGhyb3cgYWJydXB0LmVycm9yO1xuICB9KTtcbn07XG52YXIgaXNVbmhhbmRsZWQgPSBmdW5jdGlvbihwcm9taXNlKXtcbiAgaWYocHJvbWlzZS5faCA9PSAxKXJldHVybiBmYWxzZTtcbiAgdmFyIGNoYWluID0gcHJvbWlzZS5fYSB8fCBwcm9taXNlLl9jXG4gICAgLCBpICAgICA9IDBcbiAgICAsIHJlYWN0aW9uO1xuICB3aGlsZShjaGFpbi5sZW5ndGggPiBpKXtcbiAgICByZWFjdGlvbiA9IGNoYWluW2krK107XG4gICAgaWYocmVhY3Rpb24uZmFpbCB8fCAhaXNVbmhhbmRsZWQocmVhY3Rpb24ucHJvbWlzZSkpcmV0dXJuIGZhbHNlO1xuICB9IHJldHVybiB0cnVlO1xufTtcbnZhciBvbkhhbmRsZVVuaGFuZGxlZCA9IGZ1bmN0aW9uKHByb21pc2Upe1xuICB0YXNrLmNhbGwoZ2xvYmFsLCBmdW5jdGlvbigpe1xuICAgIHZhciBoYW5kbGVyO1xuICAgIGlmKGlzTm9kZSl7XG4gICAgICBwcm9jZXNzLmVtaXQoJ3JlamVjdGlvbkhhbmRsZWQnLCBwcm9taXNlKTtcbiAgICB9IGVsc2UgaWYoaGFuZGxlciA9IGdsb2JhbC5vbnJlamVjdGlvbmhhbmRsZWQpe1xuICAgICAgaGFuZGxlcih7cHJvbWlzZTogcHJvbWlzZSwgcmVhc29uOiBwcm9taXNlLl92fSk7XG4gICAgfVxuICB9KTtcbn07XG52YXIgJHJlamVjdCA9IGZ1bmN0aW9uKHZhbHVlKXtcbiAgdmFyIHByb21pc2UgPSB0aGlzO1xuICBpZihwcm9taXNlLl9kKXJldHVybjtcbiAgcHJvbWlzZS5fZCA9IHRydWU7XG4gIHByb21pc2UgPSBwcm9taXNlLl93IHx8IHByb21pc2U7IC8vIHVud3JhcFxuICBwcm9taXNlLl92ID0gdmFsdWU7XG4gIHByb21pc2UuX3MgPSAyO1xuICBpZighcHJvbWlzZS5fYSlwcm9taXNlLl9hID0gcHJvbWlzZS5fYy5zbGljZSgpO1xuICBub3RpZnkocHJvbWlzZSwgdHJ1ZSk7XG59O1xudmFyICRyZXNvbHZlID0gZnVuY3Rpb24odmFsdWUpe1xuICB2YXIgcHJvbWlzZSA9IHRoaXNcbiAgICAsIHRoZW47XG4gIGlmKHByb21pc2UuX2QpcmV0dXJuO1xuICBwcm9taXNlLl9kID0gdHJ1ZTtcbiAgcHJvbWlzZSA9IHByb21pc2UuX3cgfHwgcHJvbWlzZTsgLy8gdW53cmFwXG4gIHRyeSB7XG4gICAgaWYocHJvbWlzZSA9PT0gdmFsdWUpdGhyb3cgVHlwZUVycm9yKFwiUHJvbWlzZSBjYW4ndCBiZSByZXNvbHZlZCBpdHNlbGZcIik7XG4gICAgaWYodGhlbiA9IGlzVGhlbmFibGUodmFsdWUpKXtcbiAgICAgIG1pY3JvdGFzayhmdW5jdGlvbigpe1xuICAgICAgICB2YXIgd3JhcHBlciA9IHtfdzogcHJvbWlzZSwgX2Q6IGZhbHNlfTsgLy8gd3JhcFxuICAgICAgICB0cnkge1xuICAgICAgICAgIHRoZW4uY2FsbCh2YWx1ZSwgY3R4KCRyZXNvbHZlLCB3cmFwcGVyLCAxKSwgY3R4KCRyZWplY3QsIHdyYXBwZXIsIDEpKTtcbiAgICAgICAgfSBjYXRjaChlKXtcbiAgICAgICAgICAkcmVqZWN0LmNhbGwod3JhcHBlciwgZSk7XG4gICAgICAgIH1cbiAgICAgIH0pO1xuICAgIH0gZWxzZSB7XG4gICAgICBwcm9taXNlLl92ID0gdmFsdWU7XG4gICAgICBwcm9taXNlLl9zID0gMTtcbiAgICAgIG5vdGlmeShwcm9taXNlLCBmYWxzZSk7XG4gICAgfVxuICB9IGNhdGNoKGUpe1xuICAgICRyZWplY3QuY2FsbCh7X3c6IHByb21pc2UsIF9kOiBmYWxzZX0sIGUpOyAvLyB3cmFwXG4gIH1cbn07XG5cbi8vIGNvbnN0cnVjdG9yIHBvbHlmaWxsXG5pZighVVNFX05BVElWRSl7XG4gIC8vIDI1LjQuMy4xIFByb21pc2UoZXhlY3V0b3IpXG4gICRQcm9taXNlID0gZnVuY3Rpb24gUHJvbWlzZShleGVjdXRvcil7XG4gICAgYW5JbnN0YW5jZSh0aGlzLCAkUHJvbWlzZSwgUFJPTUlTRSwgJ19oJyk7XG4gICAgYUZ1bmN0aW9uKGV4ZWN1dG9yKTtcbiAgICBJbnRlcm5hbC5jYWxsKHRoaXMpO1xuICAgIHRyeSB7XG4gICAgICBleGVjdXRvcihjdHgoJHJlc29sdmUsIHRoaXMsIDEpLCBjdHgoJHJlamVjdCwgdGhpcywgMSkpO1xuICAgIH0gY2F0Y2goZXJyKXtcbiAgICAgICRyZWplY3QuY2FsbCh0aGlzLCBlcnIpO1xuICAgIH1cbiAgfTtcbiAgSW50ZXJuYWwgPSBmdW5jdGlvbiBQcm9taXNlKGV4ZWN1dG9yKXtcbiAgICB0aGlzLl9jID0gW107ICAgICAgICAgICAgIC8vIDwtIGF3YWl0aW5nIHJlYWN0aW9uc1xuICAgIHRoaXMuX2EgPSB1bmRlZmluZWQ7ICAgICAgLy8gPC0gY2hlY2tlZCBpbiBpc1VuaGFuZGxlZCByZWFjdGlvbnNcbiAgICB0aGlzLl9zID0gMDsgICAgICAgICAgICAgIC8vIDwtIHN0YXRlXG4gICAgdGhpcy5fZCA9IGZhbHNlOyAgICAgICAgICAvLyA8LSBkb25lXG4gICAgdGhpcy5fdiA9IHVuZGVmaW5lZDsgICAgICAvLyA8LSB2YWx1ZVxuICAgIHRoaXMuX2ggPSAwOyAgICAgICAgICAgICAgLy8gPC0gcmVqZWN0aW9uIHN0YXRlLCAwIC0gZGVmYXVsdCwgMSAtIGhhbmRsZWQsIDIgLSB1bmhhbmRsZWRcbiAgICB0aGlzLl9uID0gZmFsc2U7ICAgICAgICAgIC8vIDwtIG5vdGlmeVxuICB9O1xuICBJbnRlcm5hbC5wcm90b3R5cGUgPSByZXF1aXJlKCcuL19yZWRlZmluZS1hbGwnKSgkUHJvbWlzZS5wcm90b3R5cGUsIHtcbiAgICAvLyAyNS40LjUuMyBQcm9taXNlLnByb3RvdHlwZS50aGVuKG9uRnVsZmlsbGVkLCBvblJlamVjdGVkKVxuICAgIHRoZW46IGZ1bmN0aW9uIHRoZW4ob25GdWxmaWxsZWQsIG9uUmVqZWN0ZWQpe1xuICAgICAgdmFyIHJlYWN0aW9uICAgID0gbmV3UHJvbWlzZUNhcGFiaWxpdHkoc3BlY2llc0NvbnN0cnVjdG9yKHRoaXMsICRQcm9taXNlKSk7XG4gICAgICByZWFjdGlvbi5vayAgICAgPSB0eXBlb2Ygb25GdWxmaWxsZWQgPT0gJ2Z1bmN0aW9uJyA/IG9uRnVsZmlsbGVkIDogdHJ1ZTtcbiAgICAgIHJlYWN0aW9uLmZhaWwgICA9IHR5cGVvZiBvblJlamVjdGVkID09ICdmdW5jdGlvbicgJiYgb25SZWplY3RlZDtcbiAgICAgIHJlYWN0aW9uLmRvbWFpbiA9IGlzTm9kZSA/IHByb2Nlc3MuZG9tYWluIDogdW5kZWZpbmVkO1xuICAgICAgdGhpcy5fYy5wdXNoKHJlYWN0aW9uKTtcbiAgICAgIGlmKHRoaXMuX2EpdGhpcy5fYS5wdXNoKHJlYWN0aW9uKTtcbiAgICAgIGlmKHRoaXMuX3Mpbm90aWZ5KHRoaXMsIGZhbHNlKTtcbiAgICAgIHJldHVybiByZWFjdGlvbi5wcm9taXNlO1xuICAgIH0sXG4gICAgLy8gMjUuNC41LjEgUHJvbWlzZS5wcm90b3R5cGUuY2F0Y2gob25SZWplY3RlZClcbiAgICAnY2F0Y2gnOiBmdW5jdGlvbihvblJlamVjdGVkKXtcbiAgICAgIHJldHVybiB0aGlzLnRoZW4odW5kZWZpbmVkLCBvblJlamVjdGVkKTtcbiAgICB9XG4gIH0pO1xuICBQcm9taXNlQ2FwYWJpbGl0eSA9IGZ1bmN0aW9uKCl7XG4gICAgdmFyIHByb21pc2UgID0gbmV3IEludGVybmFsO1xuICAgIHRoaXMucHJvbWlzZSA9IHByb21pc2U7XG4gICAgdGhpcy5yZXNvbHZlID0gY3R4KCRyZXNvbHZlLCBwcm9taXNlLCAxKTtcbiAgICB0aGlzLnJlamVjdCAgPSBjdHgoJHJlamVjdCwgcHJvbWlzZSwgMSk7XG4gIH07XG59XG5cbiRleHBvcnQoJGV4cG9ydC5HICsgJGV4cG9ydC5XICsgJGV4cG9ydC5GICogIVVTRV9OQVRJVkUsIHtQcm9taXNlOiAkUHJvbWlzZX0pO1xucmVxdWlyZSgnLi9fc2V0LXRvLXN0cmluZy10YWcnKSgkUHJvbWlzZSwgUFJPTUlTRSk7XG5yZXF1aXJlKCcuL19zZXQtc3BlY2llcycpKFBST01JU0UpO1xuV3JhcHBlciA9IHJlcXVpcmUoJy4vX2NvcmUnKVtQUk9NSVNFXTtcblxuLy8gc3RhdGljc1xuJGV4cG9ydCgkZXhwb3J0LlMgKyAkZXhwb3J0LkYgKiAhVVNFX05BVElWRSwgUFJPTUlTRSwge1xuICAvLyAyNS40LjQuNSBQcm9taXNlLnJlamVjdChyKVxuICByZWplY3Q6IGZ1bmN0aW9uIHJlamVjdChyKXtcbiAgICB2YXIgY2FwYWJpbGl0eSA9IG5ld1Byb21pc2VDYXBhYmlsaXR5KHRoaXMpXG4gICAgICAsICQkcmVqZWN0ICAgPSBjYXBhYmlsaXR5LnJlamVjdDtcbiAgICAkJHJlamVjdChyKTtcbiAgICByZXR1cm4gY2FwYWJpbGl0eS5wcm9taXNlO1xuICB9XG59KTtcbiRleHBvcnQoJGV4cG9ydC5TICsgJGV4cG9ydC5GICogKExJQlJBUlkgfHwgIVVTRV9OQVRJVkUpLCBQUk9NSVNFLCB7XG4gIC8vIDI1LjQuNC42IFByb21pc2UucmVzb2x2ZSh4KVxuICByZXNvbHZlOiBmdW5jdGlvbiByZXNvbHZlKHgpe1xuICAgIC8vIGluc3RhbmNlb2YgaW5zdGVhZCBvZiBpbnRlcm5hbCBzbG90IGNoZWNrIGJlY2F1c2Ugd2Ugc2hvdWxkIGZpeCBpdCB3aXRob3V0IHJlcGxhY2VtZW50IG5hdGl2ZSBQcm9taXNlIGNvcmVcbiAgICBpZih4IGluc3RhbmNlb2YgJFByb21pc2UgJiYgc2FtZUNvbnN0cnVjdG9yKHguY29uc3RydWN0b3IsIHRoaXMpKXJldHVybiB4O1xuICAgIHZhciBjYXBhYmlsaXR5ID0gbmV3UHJvbWlzZUNhcGFiaWxpdHkodGhpcylcbiAgICAgICwgJCRyZXNvbHZlICA9IGNhcGFiaWxpdHkucmVzb2x2ZTtcbiAgICAkJHJlc29sdmUoeCk7XG4gICAgcmV0dXJuIGNhcGFiaWxpdHkucHJvbWlzZTtcbiAgfVxufSk7XG4kZXhwb3J0KCRleHBvcnQuUyArICRleHBvcnQuRiAqICEoVVNFX05BVElWRSAmJiByZXF1aXJlKCcuL19pdGVyLWRldGVjdCcpKGZ1bmN0aW9uKGl0ZXIpe1xuICAkUHJvbWlzZS5hbGwoaXRlcilbJ2NhdGNoJ10oZW1wdHkpO1xufSkpLCBQUk9NSVNFLCB7XG4gIC8vIDI1LjQuNC4xIFByb21pc2UuYWxsKGl0ZXJhYmxlKVxuICBhbGw6IGZ1bmN0aW9uIGFsbChpdGVyYWJsZSl7XG4gICAgdmFyIEMgICAgICAgICAgPSB0aGlzXG4gICAgICAsIGNhcGFiaWxpdHkgPSBuZXdQcm9taXNlQ2FwYWJpbGl0eShDKVxuICAgICAgLCByZXNvbHZlICAgID0gY2FwYWJpbGl0eS5yZXNvbHZlXG4gICAgICAsIHJlamVjdCAgICAgPSBjYXBhYmlsaXR5LnJlamVjdDtcbiAgICB2YXIgYWJydXB0ID0gcGVyZm9ybShmdW5jdGlvbigpe1xuICAgICAgdmFyIHZhbHVlcyAgICA9IFtdXG4gICAgICAgICwgaW5kZXggICAgID0gMFxuICAgICAgICAsIHJlbWFpbmluZyA9IDE7XG4gICAgICBmb3JPZihpdGVyYWJsZSwgZmFsc2UsIGZ1bmN0aW9uKHByb21pc2Upe1xuICAgICAgICB2YXIgJGluZGV4ICAgICAgICA9IGluZGV4KytcbiAgICAgICAgICAsIGFscmVhZHlDYWxsZWQgPSBmYWxzZTtcbiAgICAgICAgdmFsdWVzLnB1c2godW5kZWZpbmVkKTtcbiAgICAgICAgcmVtYWluaW5nKys7XG4gICAgICAgIEMucmVzb2x2ZShwcm9taXNlKS50aGVuKGZ1bmN0aW9uKHZhbHVlKXtcbiAgICAgICAgICBpZihhbHJlYWR5Q2FsbGVkKXJldHVybjtcbiAgICAgICAgICBhbHJlYWR5Q2FsbGVkICA9IHRydWU7XG4gICAgICAgICAgdmFsdWVzWyRpbmRleF0gPSB2YWx1ZTtcbiAgICAgICAgICAtLXJlbWFpbmluZyB8fCByZXNvbHZlKHZhbHVlcyk7XG4gICAgICAgIH0sIHJlamVjdCk7XG4gICAgICB9KTtcbiAgICAgIC0tcmVtYWluaW5nIHx8IHJlc29sdmUodmFsdWVzKTtcbiAgICB9KTtcbiAgICBpZihhYnJ1cHQpcmVqZWN0KGFicnVwdC5lcnJvcik7XG4gICAgcmV0dXJuIGNhcGFiaWxpdHkucHJvbWlzZTtcbiAgfSxcbiAgLy8gMjUuNC40LjQgUHJvbWlzZS5yYWNlKGl0ZXJhYmxlKVxuICByYWNlOiBmdW5jdGlvbiByYWNlKGl0ZXJhYmxlKXtcbiAgICB2YXIgQyAgICAgICAgICA9IHRoaXNcbiAgICAgICwgY2FwYWJpbGl0eSA9IG5ld1Byb21pc2VDYXBhYmlsaXR5KEMpXG4gICAgICAsIHJlamVjdCAgICAgPSBjYXBhYmlsaXR5LnJlamVjdDtcbiAgICB2YXIgYWJydXB0ID0gcGVyZm9ybShmdW5jdGlvbigpe1xuICAgICAgZm9yT2YoaXRlcmFibGUsIGZhbHNlLCBmdW5jdGlvbihwcm9taXNlKXtcbiAgICAgICAgQy5yZXNvbHZlKHByb21pc2UpLnRoZW4oY2FwYWJpbGl0eS5yZXNvbHZlLCByZWplY3QpO1xuICAgICAgfSk7XG4gICAgfSk7XG4gICAgaWYoYWJydXB0KXJlamVjdChhYnJ1cHQuZXJyb3IpO1xuICAgIHJldHVybiBjYXBhYmlsaXR5LnByb21pc2U7XG4gIH1cbn0pO1xuXG5cbi8qKiBXRUJQQUNLIEZPT1RFUiAqKlxuICoqIC9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9lczYucHJvbWlzZS5qc1xuICoqLyIsIm1vZHVsZS5leHBvcnRzID0gZnVuY3Rpb24oaXQsIENvbnN0cnVjdG9yLCBuYW1lLCBmb3JiaWRkZW5GaWVsZCl7XG4gIGlmKCEoaXQgaW5zdGFuY2VvZiBDb25zdHJ1Y3RvcikgfHwgKGZvcmJpZGRlbkZpZWxkICE9PSB1bmRlZmluZWQgJiYgZm9yYmlkZGVuRmllbGQgaW4gaXQpKXtcbiAgICB0aHJvdyBUeXBlRXJyb3IobmFtZSArICc6IGluY29ycmVjdCBpbnZvY2F0aW9uIScpO1xuICB9IHJldHVybiBpdDtcbn07XG5cblxuLyoqIFdFQlBBQ0sgRk9PVEVSICoqXG4gKiogL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL34vY29yZS1qcy9tb2R1bGVzL19hbi1pbnN0YW5jZS5qc1xuICoqLyIsInZhciBjdHggICAgICAgICA9IHJlcXVpcmUoJy4vX2N0eCcpXG4gICwgY2FsbCAgICAgICAgPSByZXF1aXJlKCcuL19pdGVyLWNhbGwnKVxuICAsIGlzQXJyYXlJdGVyID0gcmVxdWlyZSgnLi9faXMtYXJyYXktaXRlcicpXG4gICwgYW5PYmplY3QgICAgPSByZXF1aXJlKCcuL19hbi1vYmplY3QnKVxuICAsIHRvTGVuZ3RoICAgID0gcmVxdWlyZSgnLi9fdG8tbGVuZ3RoJylcbiAgLCBnZXRJdGVyRm4gICA9IHJlcXVpcmUoJy4vY29yZS5nZXQtaXRlcmF0b3ItbWV0aG9kJylcbiAgLCBCUkVBSyAgICAgICA9IHt9XG4gICwgUkVUVVJOICAgICAgPSB7fTtcbnZhciBleHBvcnRzID0gbW9kdWxlLmV4cG9ydHMgPSBmdW5jdGlvbihpdGVyYWJsZSwgZW50cmllcywgZm4sIHRoYXQsIElURVJBVE9SKXtcbiAgdmFyIGl0ZXJGbiA9IElURVJBVE9SID8gZnVuY3Rpb24oKXsgcmV0dXJuIGl0ZXJhYmxlOyB9IDogZ2V0SXRlckZuKGl0ZXJhYmxlKVxuICAgICwgZiAgICAgID0gY3R4KGZuLCB0aGF0LCBlbnRyaWVzID8gMiA6IDEpXG4gICAgLCBpbmRleCAgPSAwXG4gICAgLCBsZW5ndGgsIHN0ZXAsIGl0ZXJhdG9yLCByZXN1bHQ7XG4gIGlmKHR5cGVvZiBpdGVyRm4gIT0gJ2Z1bmN0aW9uJyl0aHJvdyBUeXBlRXJyb3IoaXRlcmFibGUgKyAnIGlzIG5vdCBpdGVyYWJsZSEnKTtcbiAgLy8gZmFzdCBjYXNlIGZvciBhcnJheXMgd2l0aCBkZWZhdWx0IGl0ZXJhdG9yXG4gIGlmKGlzQXJyYXlJdGVyKGl0ZXJGbikpZm9yKGxlbmd0aCA9IHRvTGVuZ3RoKGl0ZXJhYmxlLmxlbmd0aCk7IGxlbmd0aCA+IGluZGV4OyBpbmRleCsrKXtcbiAgICByZXN1bHQgPSBlbnRyaWVzID8gZihhbk9iamVjdChzdGVwID0gaXRlcmFibGVbaW5kZXhdKVswXSwgc3RlcFsxXSkgOiBmKGl0ZXJhYmxlW2luZGV4XSk7XG4gICAgaWYocmVzdWx0ID09PSBCUkVBSyB8fCByZXN1bHQgPT09IFJFVFVSTilyZXR1cm4gcmVzdWx0O1xuICB9IGVsc2UgZm9yKGl0ZXJhdG9yID0gaXRlckZuLmNhbGwoaXRlcmFibGUpOyAhKHN0ZXAgPSBpdGVyYXRvci5uZXh0KCkpLmRvbmU7ICl7XG4gICAgcmVzdWx0ID0gY2FsbChpdGVyYXRvciwgZiwgc3RlcC52YWx1ZSwgZW50cmllcyk7XG4gICAgaWYocmVzdWx0ID09PSBCUkVBSyB8fCByZXN1bHQgPT09IFJFVFVSTilyZXR1cm4gcmVzdWx0O1xuICB9XG59O1xuZXhwb3J0cy5CUkVBSyAgPSBCUkVBSztcbmV4cG9ydHMuUkVUVVJOID0gUkVUVVJOO1xuXG5cbi8qKiBXRUJQQUNLIEZPT1RFUiAqKlxuICoqIC9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9fZm9yLW9mLmpzXG4gKiovIiwiLy8gY2FsbCBzb21ldGhpbmcgb24gaXRlcmF0b3Igc3RlcCB3aXRoIHNhZmUgY2xvc2luZyBvbiBlcnJvclxudmFyIGFuT2JqZWN0ID0gcmVxdWlyZSgnLi9fYW4tb2JqZWN0Jyk7XG5tb2R1bGUuZXhwb3J0cyA9IGZ1bmN0aW9uKGl0ZXJhdG9yLCBmbiwgdmFsdWUsIGVudHJpZXMpe1xuICB0cnkge1xuICAgIHJldHVybiBlbnRyaWVzID8gZm4oYW5PYmplY3QodmFsdWUpWzBdLCB2YWx1ZVsxXSkgOiBmbih2YWx1ZSk7XG4gIC8vIDcuNC42IEl0ZXJhdG9yQ2xvc2UoaXRlcmF0b3IsIGNvbXBsZXRpb24pXG4gIH0gY2F0Y2goZSl7XG4gICAgdmFyIHJldCA9IGl0ZXJhdG9yWydyZXR1cm4nXTtcbiAgICBpZihyZXQgIT09IHVuZGVmaW5lZClhbk9iamVjdChyZXQuY2FsbChpdGVyYXRvcikpO1xuICAgIHRocm93IGU7XG4gIH1cbn07XG5cblxuLyoqIFdFQlBBQ0sgRk9PVEVSICoqXG4gKiogL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL34vY29yZS1qcy9tb2R1bGVzL19pdGVyLWNhbGwuanNcbiAqKi8iLCIvLyBjaGVjayBvbiBkZWZhdWx0IEFycmF5IGl0ZXJhdG9yXG52YXIgSXRlcmF0b3JzICA9IHJlcXVpcmUoJy4vX2l0ZXJhdG9ycycpXG4gICwgSVRFUkFUT1IgICA9IHJlcXVpcmUoJy4vX3drcycpKCdpdGVyYXRvcicpXG4gICwgQXJyYXlQcm90byA9IEFycmF5LnByb3RvdHlwZTtcblxubW9kdWxlLmV4cG9ydHMgPSBmdW5jdGlvbihpdCl7XG4gIHJldHVybiBpdCAhPT0gdW5kZWZpbmVkICYmIChJdGVyYXRvcnMuQXJyYXkgPT09IGl0IHx8IEFycmF5UHJvdG9bSVRFUkFUT1JdID09PSBpdCk7XG59O1xuXG5cbi8qKiBXRUJQQUNLIEZPT1RFUiAqKlxuICoqIC9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9faXMtYXJyYXktaXRlci5qc1xuICoqLyIsInZhciBjbGFzc29mICAgPSByZXF1aXJlKCcuL19jbGFzc29mJylcbiAgLCBJVEVSQVRPUiAgPSByZXF1aXJlKCcuL193a3MnKSgnaXRlcmF0b3InKVxuICAsIEl0ZXJhdG9ycyA9IHJlcXVpcmUoJy4vX2l0ZXJhdG9ycycpO1xubW9kdWxlLmV4cG9ydHMgPSByZXF1aXJlKCcuL19jb3JlJykuZ2V0SXRlcmF0b3JNZXRob2QgPSBmdW5jdGlvbihpdCl7XG4gIGlmKGl0ICE9IHVuZGVmaW5lZClyZXR1cm4gaXRbSVRFUkFUT1JdXG4gICAgfHwgaXRbJ0BAaXRlcmF0b3InXVxuICAgIHx8IEl0ZXJhdG9yc1tjbGFzc29mKGl0KV07XG59O1xuXG5cbi8qKiBXRUJQQUNLIEZPT1RFUiAqKlxuICoqIC9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9jb3JlLmdldC1pdGVyYXRvci1tZXRob2QuanNcbiAqKi8iLCIvLyA3LjMuMjAgU3BlY2llc0NvbnN0cnVjdG9yKE8sIGRlZmF1bHRDb25zdHJ1Y3RvcilcbnZhciBhbk9iamVjdCAgPSByZXF1aXJlKCcuL19hbi1vYmplY3QnKVxuICAsIGFGdW5jdGlvbiA9IHJlcXVpcmUoJy4vX2EtZnVuY3Rpb24nKVxuICAsIFNQRUNJRVMgICA9IHJlcXVpcmUoJy4vX3drcycpKCdzcGVjaWVzJyk7XG5tb2R1bGUuZXhwb3J0cyA9IGZ1bmN0aW9uKE8sIEQpe1xuICB2YXIgQyA9IGFuT2JqZWN0KE8pLmNvbnN0cnVjdG9yLCBTO1xuICByZXR1cm4gQyA9PT0gdW5kZWZpbmVkIHx8IChTID0gYW5PYmplY3QoQylbU1BFQ0lFU10pID09IHVuZGVmaW5lZCA/IEQgOiBhRnVuY3Rpb24oUyk7XG59O1xuXG5cbi8qKiBXRUJQQUNLIEZPT1RFUiAqKlxuICoqIC9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9fc3BlY2llcy1jb25zdHJ1Y3Rvci5qc1xuICoqLyIsInZhciBjdHggICAgICAgICAgICAgICAgPSByZXF1aXJlKCcuL19jdHgnKVxuICAsIGludm9rZSAgICAgICAgICAgICA9IHJlcXVpcmUoJy4vX2ludm9rZScpXG4gICwgaHRtbCAgICAgICAgICAgICAgID0gcmVxdWlyZSgnLi9faHRtbCcpXG4gICwgY2VsICAgICAgICAgICAgICAgID0gcmVxdWlyZSgnLi9fZG9tLWNyZWF0ZScpXG4gICwgZ2xvYmFsICAgICAgICAgICAgID0gcmVxdWlyZSgnLi9fZ2xvYmFsJylcbiAgLCBwcm9jZXNzICAgICAgICAgICAgPSBnbG9iYWwucHJvY2Vzc1xuICAsIHNldFRhc2sgICAgICAgICAgICA9IGdsb2JhbC5zZXRJbW1lZGlhdGVcbiAgLCBjbGVhclRhc2sgICAgICAgICAgPSBnbG9iYWwuY2xlYXJJbW1lZGlhdGVcbiAgLCBNZXNzYWdlQ2hhbm5lbCAgICAgPSBnbG9iYWwuTWVzc2FnZUNoYW5uZWxcbiAgLCBjb3VudGVyICAgICAgICAgICAgPSAwXG4gICwgcXVldWUgICAgICAgICAgICAgID0ge31cbiAgLCBPTlJFQURZU1RBVEVDSEFOR0UgPSAnb25yZWFkeXN0YXRlY2hhbmdlJ1xuICAsIGRlZmVyLCBjaGFubmVsLCBwb3J0O1xudmFyIHJ1biA9IGZ1bmN0aW9uKCl7XG4gIHZhciBpZCA9ICt0aGlzO1xuICBpZihxdWV1ZS5oYXNPd25Qcm9wZXJ0eShpZCkpe1xuICAgIHZhciBmbiA9IHF1ZXVlW2lkXTtcbiAgICBkZWxldGUgcXVldWVbaWRdO1xuICAgIGZuKCk7XG4gIH1cbn07XG52YXIgbGlzdGVuZXIgPSBmdW5jdGlvbihldmVudCl7XG4gIHJ1bi5jYWxsKGV2ZW50LmRhdGEpO1xufTtcbi8vIE5vZGUuanMgMC45KyAmIElFMTArIGhhcyBzZXRJbW1lZGlhdGUsIG90aGVyd2lzZTpcbmlmKCFzZXRUYXNrIHx8ICFjbGVhclRhc2spe1xuICBzZXRUYXNrID0gZnVuY3Rpb24gc2V0SW1tZWRpYXRlKGZuKXtcbiAgICB2YXIgYXJncyA9IFtdLCBpID0gMTtcbiAgICB3aGlsZShhcmd1bWVudHMubGVuZ3RoID4gaSlhcmdzLnB1c2goYXJndW1lbnRzW2krK10pO1xuICAgIHF1ZXVlWysrY291bnRlcl0gPSBmdW5jdGlvbigpe1xuICAgICAgaW52b2tlKHR5cGVvZiBmbiA9PSAnZnVuY3Rpb24nID8gZm4gOiBGdW5jdGlvbihmbiksIGFyZ3MpO1xuICAgIH07XG4gICAgZGVmZXIoY291bnRlcik7XG4gICAgcmV0dXJuIGNvdW50ZXI7XG4gIH07XG4gIGNsZWFyVGFzayA9IGZ1bmN0aW9uIGNsZWFySW1tZWRpYXRlKGlkKXtcbiAgICBkZWxldGUgcXVldWVbaWRdO1xuICB9O1xuICAvLyBOb2RlLmpzIDAuOC1cbiAgaWYocmVxdWlyZSgnLi9fY29mJykocHJvY2VzcykgPT0gJ3Byb2Nlc3MnKXtcbiAgICBkZWZlciA9IGZ1bmN0aW9uKGlkKXtcbiAgICAgIHByb2Nlc3MubmV4dFRpY2soY3R4KHJ1biwgaWQsIDEpKTtcbiAgICB9O1xuICAvLyBCcm93c2VycyB3aXRoIE1lc3NhZ2VDaGFubmVsLCBpbmNsdWRlcyBXZWJXb3JrZXJzXG4gIH0gZWxzZSBpZihNZXNzYWdlQ2hhbm5lbCl7XG4gICAgY2hhbm5lbCA9IG5ldyBNZXNzYWdlQ2hhbm5lbDtcbiAgICBwb3J0ICAgID0gY2hhbm5lbC5wb3J0MjtcbiAgICBjaGFubmVsLnBvcnQxLm9ubWVzc2FnZSA9IGxpc3RlbmVyO1xuICAgIGRlZmVyID0gY3R4KHBvcnQucG9zdE1lc3NhZ2UsIHBvcnQsIDEpO1xuICAvLyBCcm93c2VycyB3aXRoIHBvc3RNZXNzYWdlLCBza2lwIFdlYldvcmtlcnNcbiAgLy8gSUU4IGhhcyBwb3N0TWVzc2FnZSwgYnV0IGl0J3Mgc3luYyAmIHR5cGVvZiBpdHMgcG9zdE1lc3NhZ2UgaXMgJ29iamVjdCdcbiAgfSBlbHNlIGlmKGdsb2JhbC5hZGRFdmVudExpc3RlbmVyICYmIHR5cGVvZiBwb3N0TWVzc2FnZSA9PSAnZnVuY3Rpb24nICYmICFnbG9iYWwuaW1wb3J0U2NyaXB0cyl7XG4gICAgZGVmZXIgPSBmdW5jdGlvbihpZCl7XG4gICAgICBnbG9iYWwucG9zdE1lc3NhZ2UoaWQgKyAnJywgJyonKTtcbiAgICB9O1xuICAgIGdsb2JhbC5hZGRFdmVudExpc3RlbmVyKCdtZXNzYWdlJywgbGlzdGVuZXIsIGZhbHNlKTtcbiAgLy8gSUU4LVxuICB9IGVsc2UgaWYoT05SRUFEWVNUQVRFQ0hBTkdFIGluIGNlbCgnc2NyaXB0Jykpe1xuICAgIGRlZmVyID0gZnVuY3Rpb24oaWQpe1xuICAgICAgaHRtbC5hcHBlbmRDaGlsZChjZWwoJ3NjcmlwdCcpKVtPTlJFQURZU1RBVEVDSEFOR0VdID0gZnVuY3Rpb24oKXtcbiAgICAgICAgaHRtbC5yZW1vdmVDaGlsZCh0aGlzKTtcbiAgICAgICAgcnVuLmNhbGwoaWQpO1xuICAgICAgfTtcbiAgICB9O1xuICAvLyBSZXN0IG9sZCBicm93c2Vyc1xuICB9IGVsc2Uge1xuICAgIGRlZmVyID0gZnVuY3Rpb24oaWQpe1xuICAgICAgc2V0VGltZW91dChjdHgocnVuLCBpZCwgMSksIDApO1xuICAgIH07XG4gIH1cbn1cbm1vZHVsZS5leHBvcnRzID0ge1xuICBzZXQ6ICAgc2V0VGFzayxcbiAgY2xlYXI6IGNsZWFyVGFza1xufTtcblxuXG4vKiogV0VCUEFDSyBGT09URVIgKipcbiAqKiAvVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvfi9jb3JlLWpzL21vZHVsZXMvX3Rhc2suanNcbiAqKi8iLCIvLyBmYXN0IGFwcGx5LCBodHRwOi8vanNwZXJmLmxua2l0LmNvbS9mYXN0LWFwcGx5LzVcbm1vZHVsZS5leHBvcnRzID0gZnVuY3Rpb24oZm4sIGFyZ3MsIHRoYXQpe1xuICB2YXIgdW4gPSB0aGF0ID09PSB1bmRlZmluZWQ7XG4gIHN3aXRjaChhcmdzLmxlbmd0aCl7XG4gICAgY2FzZSAwOiByZXR1cm4gdW4gPyBmbigpXG4gICAgICAgICAgICAgICAgICAgICAgOiBmbi5jYWxsKHRoYXQpO1xuICAgIGNhc2UgMTogcmV0dXJuIHVuID8gZm4oYXJnc1swXSlcbiAgICAgICAgICAgICAgICAgICAgICA6IGZuLmNhbGwodGhhdCwgYXJnc1swXSk7XG4gICAgY2FzZSAyOiByZXR1cm4gdW4gPyBmbihhcmdzWzBdLCBhcmdzWzFdKVxuICAgICAgICAgICAgICAgICAgICAgIDogZm4uY2FsbCh0aGF0LCBhcmdzWzBdLCBhcmdzWzFdKTtcbiAgICBjYXNlIDM6IHJldHVybiB1biA/IGZuKGFyZ3NbMF0sIGFyZ3NbMV0sIGFyZ3NbMl0pXG4gICAgICAgICAgICAgICAgICAgICAgOiBmbi5jYWxsKHRoYXQsIGFyZ3NbMF0sIGFyZ3NbMV0sIGFyZ3NbMl0pO1xuICAgIGNhc2UgNDogcmV0dXJuIHVuID8gZm4oYXJnc1swXSwgYXJnc1sxXSwgYXJnc1syXSwgYXJnc1szXSlcbiAgICAgICAgICAgICAgICAgICAgICA6IGZuLmNhbGwodGhhdCwgYXJnc1swXSwgYXJnc1sxXSwgYXJnc1syXSwgYXJnc1szXSk7XG4gIH0gcmV0dXJuICAgICAgICAgICAgICBmbi5hcHBseSh0aGF0LCBhcmdzKTtcbn07XG5cblxuLyoqIFdFQlBBQ0sgRk9PVEVSICoqXG4gKiogL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL34vY29yZS1qcy9tb2R1bGVzL19pbnZva2UuanNcbiAqKi8iLCJ2YXIgZ2xvYmFsICAgID0gcmVxdWlyZSgnLi9fZ2xvYmFsJylcbiAgLCBtYWNyb3Rhc2sgPSByZXF1aXJlKCcuL190YXNrJykuc2V0XG4gICwgT2JzZXJ2ZXIgID0gZ2xvYmFsLk11dGF0aW9uT2JzZXJ2ZXIgfHwgZ2xvYmFsLldlYktpdE11dGF0aW9uT2JzZXJ2ZXJcbiAgLCBwcm9jZXNzICAgPSBnbG9iYWwucHJvY2Vzc1xuICAsIFByb21pc2UgICA9IGdsb2JhbC5Qcm9taXNlXG4gICwgaXNOb2RlICAgID0gcmVxdWlyZSgnLi9fY29mJykocHJvY2VzcykgPT0gJ3Byb2Nlc3MnO1xuXG5tb2R1bGUuZXhwb3J0cyA9IGZ1bmN0aW9uKCl7XG4gIHZhciBoZWFkLCBsYXN0LCBub3RpZnk7XG5cbiAgdmFyIGZsdXNoID0gZnVuY3Rpb24oKXtcbiAgICB2YXIgcGFyZW50LCBmbjtcbiAgICBpZihpc05vZGUgJiYgKHBhcmVudCA9IHByb2Nlc3MuZG9tYWluKSlwYXJlbnQuZXhpdCgpO1xuICAgIHdoaWxlKGhlYWQpe1xuICAgICAgZm4gICA9IGhlYWQuZm47XG4gICAgICBoZWFkID0gaGVhZC5uZXh0O1xuICAgICAgdHJ5IHtcbiAgICAgICAgZm4oKTtcbiAgICAgIH0gY2F0Y2goZSl7XG4gICAgICAgIGlmKGhlYWQpbm90aWZ5KCk7XG4gICAgICAgIGVsc2UgbGFzdCA9IHVuZGVmaW5lZDtcbiAgICAgICAgdGhyb3cgZTtcbiAgICAgIH1cbiAgICB9IGxhc3QgPSB1bmRlZmluZWQ7XG4gICAgaWYocGFyZW50KXBhcmVudC5lbnRlcigpO1xuICB9O1xuXG4gIC8vIE5vZGUuanNcbiAgaWYoaXNOb2RlKXtcbiAgICBub3RpZnkgPSBmdW5jdGlvbigpe1xuICAgICAgcHJvY2Vzcy5uZXh0VGljayhmbHVzaCk7XG4gICAgfTtcbiAgLy8gYnJvd3NlcnMgd2l0aCBNdXRhdGlvbk9ic2VydmVyXG4gIH0gZWxzZSBpZihPYnNlcnZlcil7XG4gICAgdmFyIHRvZ2dsZSA9IHRydWVcbiAgICAgICwgbm9kZSAgID0gZG9jdW1lbnQuY3JlYXRlVGV4dE5vZGUoJycpO1xuICAgIG5ldyBPYnNlcnZlcihmbHVzaCkub2JzZXJ2ZShub2RlLCB7Y2hhcmFjdGVyRGF0YTogdHJ1ZX0pOyAvLyBlc2xpbnQtZGlzYWJsZS1saW5lIG5vLW5ld1xuICAgIG5vdGlmeSA9IGZ1bmN0aW9uKCl7XG4gICAgICBub2RlLmRhdGEgPSB0b2dnbGUgPSAhdG9nZ2xlO1xuICAgIH07XG4gIC8vIGVudmlyb25tZW50cyB3aXRoIG1heWJlIG5vbi1jb21wbGV0ZWx5IGNvcnJlY3QsIGJ1dCBleGlzdGVudCBQcm9taXNlXG4gIH0gZWxzZSBpZihQcm9taXNlICYmIFByb21pc2UucmVzb2x2ZSl7XG4gICAgdmFyIHByb21pc2UgPSBQcm9taXNlLnJlc29sdmUoKTtcbiAgICBub3RpZnkgPSBmdW5jdGlvbigpe1xuICAgICAgcHJvbWlzZS50aGVuKGZsdXNoKTtcbiAgICB9O1xuICAvLyBmb3Igb3RoZXIgZW52aXJvbm1lbnRzIC0gbWFjcm90YXNrIGJhc2VkIG9uOlxuICAvLyAtIHNldEltbWVkaWF0ZVxuICAvLyAtIE1lc3NhZ2VDaGFubmVsXG4gIC8vIC0gd2luZG93LnBvc3RNZXNzYWdcbiAgLy8gLSBvbnJlYWR5c3RhdGVjaGFuZ2VcbiAgLy8gLSBzZXRUaW1lb3V0XG4gIH0gZWxzZSB7XG4gICAgbm90aWZ5ID0gZnVuY3Rpb24oKXtcbiAgICAgIC8vIHN0cmFuZ2UgSUUgKyB3ZWJwYWNrIGRldiBzZXJ2ZXIgYnVnIC0gdXNlIC5jYWxsKGdsb2JhbClcbiAgICAgIG1hY3JvdGFzay5jYWxsKGdsb2JhbCwgZmx1c2gpO1xuICAgIH07XG4gIH1cblxuICByZXR1cm4gZnVuY3Rpb24oZm4pe1xuICAgIHZhciB0YXNrID0ge2ZuOiBmbiwgbmV4dDogdW5kZWZpbmVkfTtcbiAgICBpZihsYXN0KWxhc3QubmV4dCA9IHRhc2s7XG4gICAgaWYoIWhlYWQpe1xuICAgICAgaGVhZCA9IHRhc2s7XG4gICAgICBub3RpZnkoKTtcbiAgICB9IGxhc3QgPSB0YXNrO1xuICB9O1xufTtcblxuXG4vKiogV0VCUEFDSyBGT09URVIgKipcbiAqKiAvVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvfi9jb3JlLWpzL21vZHVsZXMvX21pY3JvdGFzay5qc1xuICoqLyIsInZhciByZWRlZmluZSA9IHJlcXVpcmUoJy4vX3JlZGVmaW5lJyk7XG5tb2R1bGUuZXhwb3J0cyA9IGZ1bmN0aW9uKHRhcmdldCwgc3JjLCBzYWZlKXtcbiAgZm9yKHZhciBrZXkgaW4gc3JjKXJlZGVmaW5lKHRhcmdldCwga2V5LCBzcmNba2V5XSwgc2FmZSk7XG4gIHJldHVybiB0YXJnZXQ7XG59O1xuXG5cbi8qKiBXRUJQQUNLIEZPT1RFUiAqKlxuICoqIC9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9fcmVkZWZpbmUtYWxsLmpzXG4gKiovIiwiJ3VzZSBzdHJpY3QnO1xudmFyIGdsb2JhbCAgICAgID0gcmVxdWlyZSgnLi9fZ2xvYmFsJylcbiAgLCBkUCAgICAgICAgICA9IHJlcXVpcmUoJy4vX29iamVjdC1kcCcpXG4gICwgREVTQ1JJUFRPUlMgPSByZXF1aXJlKCcuL19kZXNjcmlwdG9ycycpXG4gICwgU1BFQ0lFUyAgICAgPSByZXF1aXJlKCcuL193a3MnKSgnc3BlY2llcycpO1xuXG5tb2R1bGUuZXhwb3J0cyA9IGZ1bmN0aW9uKEtFWSl7XG4gIHZhciBDID0gZ2xvYmFsW0tFWV07XG4gIGlmKERFU0NSSVBUT1JTICYmIEMgJiYgIUNbU1BFQ0lFU10pZFAuZihDLCBTUEVDSUVTLCB7XG4gICAgY29uZmlndXJhYmxlOiB0cnVlLFxuICAgIGdldDogZnVuY3Rpb24oKXsgcmV0dXJuIHRoaXM7IH1cbiAgfSk7XG59O1xuXG5cbi8qKiBXRUJQQUNLIEZPT1RFUiAqKlxuICoqIC9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9fc2V0LXNwZWNpZXMuanNcbiAqKi8iLCJ2YXIgSVRFUkFUT1IgICAgID0gcmVxdWlyZSgnLi9fd2tzJykoJ2l0ZXJhdG9yJylcbiAgLCBTQUZFX0NMT1NJTkcgPSBmYWxzZTtcblxudHJ5IHtcbiAgdmFyIHJpdGVyID0gWzddW0lURVJBVE9SXSgpO1xuICByaXRlclsncmV0dXJuJ10gPSBmdW5jdGlvbigpeyBTQUZFX0NMT1NJTkcgPSB0cnVlOyB9O1xuICBBcnJheS5mcm9tKHJpdGVyLCBmdW5jdGlvbigpeyB0aHJvdyAyOyB9KTtcbn0gY2F0Y2goZSl7IC8qIGVtcHR5ICovIH1cblxubW9kdWxlLmV4cG9ydHMgPSBmdW5jdGlvbihleGVjLCBza2lwQ2xvc2luZyl7XG4gIGlmKCFza2lwQ2xvc2luZyAmJiAhU0FGRV9DTE9TSU5HKXJldHVybiBmYWxzZTtcbiAgdmFyIHNhZmUgPSBmYWxzZTtcbiAgdHJ5IHtcbiAgICB2YXIgYXJyICA9IFs3XVxuICAgICAgLCBpdGVyID0gYXJyW0lURVJBVE9SXSgpO1xuICAgIGl0ZXIubmV4dCA9IGZ1bmN0aW9uKCl7IHJldHVybiB7ZG9uZTogc2FmZSA9IHRydWV9OyB9O1xuICAgIGFycltJVEVSQVRPUl0gPSBmdW5jdGlvbigpeyByZXR1cm4gaXRlcjsgfTtcbiAgICBleGVjKGFycik7XG4gIH0gY2F0Y2goZSl7IC8qIGVtcHR5ICovIH1cbiAgcmV0dXJuIHNhZmU7XG59O1xuXG5cbi8qKiBXRUJQQUNLIEZPT1RFUiAqKlxuICoqIC9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9faXRlci1kZXRlY3QuanNcbiAqKi8iLCIvKipcbiAqIEBmaWxlT3ZlcnZpZXdcbiAqIFRoaXMgZmlsZSB3aWxsIGhhY2sgYGNvbnNvbGVgIG1ldGhvZHMgYnkgYFdYRW52aXJvbm1lbnQubG9nTGV2ZWxgLlxuICogU28gd2UgY2FuIGNvbnRyb2wgaG93IG1hbnkgYW5kIHdoaWNoIG1lc3NhZ2VzIHdpbGwgYmUgc2VudCBieSBjaGFuZ2UgdGhlIGxvZyBsZXZlbC5cbiAqIEFkZGl0aW9uYWxseSBpbiBuYXRpdmUgcGxhdGZvcm0gdGhlIG1lc3NhZ2UgY29udGVudCBtdXN0IGJlIHByaW1pdGl2ZSB2YWx1ZXMgYW5kXG4gKiB1c2luZyBgbmF0aXZlTG9nKC4uLmFyZ3MsIGxvZ0xldmVsTWFyaylgIHNvIHdlIGNyZWF0ZSBhIG5ldyBgY29uc29sZWAgb2JqZWN0IGluXG4gKiBnbG9iYWwgYWRkIGEgZm9ybWF0IHByb2Nlc3MgZm9yIGl0cyBtZXRob2RzLlxuICovXG5cbmNvbnN0IHsgY29uc29sZSwgbmF0aXZlTG9nIH0gPSBnbG9iYWxcbmNvbnN0IExFVkVMUyA9IFsnZXJyb3InLCAnd2FybicsICdpbmZvJywgJ2xvZycsICdkZWJ1ZyddXG5jb25zdCBsZXZlbE1hcCA9IHt9XG5cbmdlbmVyYXRlTGV2ZWxNYXAoKVxuXG4vKiBpc3RhbmJ1bCBpZ25vcmUgaWYgKi9cbmlmIChcbiAgdHlwZW9mIGNvbnNvbGUgPT09ICd1bmRlZmluZWQnIHx8IC8vIEFuZHJvaWRcbiAgKGdsb2JhbC5XWEVudmlyb25tZW50ICYmIGdsb2JhbC5XWEVudmlyb25tZW50LnBsYXRmb3JtID09PSAnaU9TJykgLy8gaU9TXG4pIHtcbiAgZ2xvYmFsLmNvbnNvbGUgPSB7XG4gICAgZGVidWc6ICguLi5hcmdzKSA9PiB7XG4gICAgICBpZiAoY2hlY2tMZXZlbCgnZGVidWcnKSkgeyBuYXRpdmVMb2coLi4uZm9ybWF0KGFyZ3MpLCAnX19ERUJVRycpIH1cbiAgICB9LFxuICAgIGxvZzogKC4uLmFyZ3MpID0+IHtcbiAgICAgIGlmIChjaGVja0xldmVsKCdsb2cnKSkgeyBuYXRpdmVMb2coLi4uZm9ybWF0KGFyZ3MpLCAnX19MT0cnKSB9XG4gICAgfSxcbiAgICBpbmZvOiAoLi4uYXJncykgPT4ge1xuICAgICAgaWYgKGNoZWNrTGV2ZWwoJ2luZm8nKSkgeyBuYXRpdmVMb2coLi4uZm9ybWF0KGFyZ3MpLCAnX19JTkZPJykgfVxuICAgIH0sXG4gICAgd2FybjogKC4uLmFyZ3MpID0+IHtcbiAgICAgIGlmIChjaGVja0xldmVsKCd3YXJuJykpIHsgbmF0aXZlTG9nKC4uLmZvcm1hdChhcmdzKSwgJ19fV0FSTicpIH1cbiAgICB9LFxuICAgIGVycm9yOiAoLi4uYXJncykgPT4ge1xuICAgICAgaWYgKGNoZWNrTGV2ZWwoJ2Vycm9yJykpIHsgbmF0aXZlTG9nKC4uLmZvcm1hdChhcmdzKSwgJ19fRVJST1InKSB9XG4gICAgfVxuICB9XG59XG5lbHNlIHsgLy8gSFRNTDVcbiAgY29uc3QgeyBkZWJ1ZywgbG9nLCBpbmZvLCB3YXJuLCBlcnJvciB9ID0gY29uc29sZVxuICBjb25zb2xlLl9fb3JpX18gPSB7IGRlYnVnLCBsb2csIGluZm8sIHdhcm4sIGVycm9yIH1cbiAgY29uc29sZS5kZWJ1ZyA9ICguLi5hcmdzKSA9PiB7XG4gICAgaWYgKGNoZWNrTGV2ZWwoJ2RlYnVnJykpIHsgY29uc29sZS5fX29yaV9fLmRlYnVnLmFwcGx5KGNvbnNvbGUsIGFyZ3MpIH1cbiAgfVxuICBjb25zb2xlLmxvZyA9ICguLi5hcmdzKSA9PiB7XG4gICAgaWYgKGNoZWNrTGV2ZWwoJ2xvZycpKSB7IGNvbnNvbGUuX19vcmlfXy5sb2cuYXBwbHkoY29uc29sZSwgYXJncykgfVxuICB9XG4gIGNvbnNvbGUuaW5mbyA9ICguLi5hcmdzKSA9PiB7XG4gICAgaWYgKGNoZWNrTGV2ZWwoJ2luZm8nKSkgeyBjb25zb2xlLl9fb3JpX18uaW5mby5hcHBseShjb25zb2xlLCBhcmdzKSB9XG4gIH1cbiAgY29uc29sZS53YXJuID0gKC4uLmFyZ3MpID0+IHtcbiAgICBpZiAoY2hlY2tMZXZlbCgnd2FybicpKSB7IGNvbnNvbGUuX19vcmlfXy53YXJuLmFwcGx5KGNvbnNvbGUsIGFyZ3MpIH1cbiAgfVxuICBjb25zb2xlLmVycm9yID0gKC4uLmFyZ3MpID0+IHtcbiAgICBpZiAoY2hlY2tMZXZlbCgnZXJyb3InKSkgeyBjb25zb2xlLl9fb3JpX18uZXJyb3IuYXBwbHkoY29uc29sZSwgYXJncykgfVxuICB9XG59XG5cbi8qKlxuICogR2VuZXJhdGUgbWFwIGZvciB3aGljaCB0eXBlcyBvZiBtZXNzYWdlIHdpbGwgYmUgc2VudCBpbiBhIGNlcnRhaW4gbWVzc2FnZSBsZXZlbFxuICogYXMgdGhlIG9yZGVyIG9mIExFVkVMUy5cbiAqL1xuZnVuY3Rpb24gZ2VuZXJhdGVMZXZlbE1hcCAoKSB7XG4gIExFVkVMUy5mb3JFYWNoKGxldmVsID0+IHtcbiAgICBjb25zdCBsZXZlbEluZGV4ID0gTEVWRUxTLmluZGV4T2YobGV2ZWwpXG4gICAgbGV2ZWxNYXBbbGV2ZWxdID0ge31cbiAgICBMRVZFTFMuZm9yRWFjaCh0eXBlID0+IHtcbiAgICAgIGNvbnN0IHR5cGVJbmRleCA9IExFVkVMUy5pbmRleE9mKHR5cGUpXG4gICAgICBpZiAodHlwZUluZGV4IDw9IGxldmVsSW5kZXgpIHtcbiAgICAgICAgbGV2ZWxNYXBbbGV2ZWxdW3R5cGVdID0gdHJ1ZVxuICAgICAgfVxuICAgIH0pXG4gIH0pXG59XG5cbi8qKlxuICogQ2hlY2sgaWYgYSBjZXJ0YWluIHR5cGUgb2YgbWVzc2FnZSB3aWxsIGJlIHNlbnQgaW4gY3VycmVudCBsb2cgbGV2ZWwgb2YgZW52LlxuICogQHBhcmFtICB7c3RyaW5nfSB0eXBlXG4gKiBAcmV0dXJuIHtib29sZWFufVxuICovXG5mdW5jdGlvbiBjaGVja0xldmVsICh0eXBlKSB7XG4gIGNvbnN0IGxvZ0xldmVsID0gKGdsb2JhbC5XWEVudmlyb25tZW50ICYmIGdsb2JhbC5XWEVudmlyb25tZW50LmxvZ0xldmVsKSB8fCAnbG9nJ1xuICByZXR1cm4gbGV2ZWxNYXBbbG9nTGV2ZWxdICYmIGxldmVsTWFwW2xvZ0xldmVsXVt0eXBlXVxufVxuXG4vKipcbiAqIENvbnZlcnQgYWxsIGxvZyBhcmd1bWVudHMgaW50byBwcmltaXRpdmUgdmFsdWVzLlxuICogQHBhcmFtICB7YXJyYXl9IGFyZ3NcbiAqIEByZXR1cm4ge2FycmF5fVxuICovXG5mdW5jdGlvbiBmb3JtYXQgKGFyZ3MpIHtcbiAgcmV0dXJuIGFyZ3MubWFwKCh2KSA9PiB7XG4gICAgY29uc3QgdHlwZSA9IE9iamVjdC5wcm90b3R5cGUudG9TdHJpbmcuY2FsbCh2KVxuICAgIGlmICh0eXBlLnRvTG93ZXJDYXNlKCkgPT09ICdbb2JqZWN0IG9iamVjdF0nKSB7XG4gICAgICB2ID0gSlNPTi5zdHJpbmdpZnkodilcbiAgICB9XG4gICAgZWxzZSB7XG4gICAgICB2ID0gU3RyaW5nKHYpXG4gICAgfVxuICAgIHJldHVybiB2XG4gIH0pXG59XG5cblxuXG4vKiogV0VCUEFDSyBGT09URVIgKipcbiAqKiAvVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvc3JjL3NoYXJlZC9jb25zb2xlLmpzXG4gKiovIiwiaW1wb3J0ICdjb3JlLWpzL2ZuL29iamVjdC9hc3NpZ24nXG5cblxuXG4vKiogV0VCUEFDSyBGT09URVIgKipcbiAqKiAvVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvc3JjL3NoYXJlZC9vYmplY3RBc3NpZ24uanNcbiAqKi8iLCJyZXF1aXJlKCcuLi8uLi9tb2R1bGVzL2VzNi5vYmplY3QuYXNzaWduJyk7XG5tb2R1bGUuZXhwb3J0cyA9IHJlcXVpcmUoJy4uLy4uL21vZHVsZXMvX2NvcmUnKS5PYmplY3QuYXNzaWduO1xuXG5cbi8qKiBXRUJQQUNLIEZPT1RFUiAqKlxuICoqIC9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvZm4vb2JqZWN0L2Fzc2lnbi5qc1xuICoqLyIsIi8vIDE5LjEuMy4xIE9iamVjdC5hc3NpZ24odGFyZ2V0LCBzb3VyY2UpXG52YXIgJGV4cG9ydCA9IHJlcXVpcmUoJy4vX2V4cG9ydCcpO1xuXG4kZXhwb3J0KCRleHBvcnQuUyArICRleHBvcnQuRiwgJ09iamVjdCcsIHthc3NpZ246IHJlcXVpcmUoJy4vX29iamVjdC1hc3NpZ24nKX0pO1xuXG5cbi8qKiBXRUJQQUNLIEZPT1RFUiAqKlxuICoqIC9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9+L2NvcmUtanMvbW9kdWxlcy9lczYub2JqZWN0LmFzc2lnbi5qc1xuICoqLyIsIid1c2Ugc3RyaWN0Jztcbi8vIDE5LjEuMi4xIE9iamVjdC5hc3NpZ24odGFyZ2V0LCBzb3VyY2UsIC4uLilcbnZhciBnZXRLZXlzICA9IHJlcXVpcmUoJy4vX29iamVjdC1rZXlzJylcbiAgLCBnT1BTICAgICA9IHJlcXVpcmUoJy4vX29iamVjdC1nb3BzJylcbiAgLCBwSUUgICAgICA9IHJlcXVpcmUoJy4vX29iamVjdC1waWUnKVxuICAsIHRvT2JqZWN0ID0gcmVxdWlyZSgnLi9fdG8tb2JqZWN0JylcbiAgLCBJT2JqZWN0ICA9IHJlcXVpcmUoJy4vX2lvYmplY3QnKVxuICAsICRhc3NpZ24gID0gT2JqZWN0LmFzc2lnbjtcblxuLy8gc2hvdWxkIHdvcmsgd2l0aCBzeW1ib2xzIGFuZCBzaG91bGQgaGF2ZSBkZXRlcm1pbmlzdGljIHByb3BlcnR5IG9yZGVyIChWOCBidWcpXG5tb2R1bGUuZXhwb3J0cyA9ICEkYXNzaWduIHx8IHJlcXVpcmUoJy4vX2ZhaWxzJykoZnVuY3Rpb24oKXtcbiAgdmFyIEEgPSB7fVxuICAgICwgQiA9IHt9XG4gICAgLCBTID0gU3ltYm9sKClcbiAgICAsIEsgPSAnYWJjZGVmZ2hpamtsbW5vcHFyc3QnO1xuICBBW1NdID0gNztcbiAgSy5zcGxpdCgnJykuZm9yRWFjaChmdW5jdGlvbihrKXsgQltrXSA9IGs7IH0pO1xuICByZXR1cm4gJGFzc2lnbih7fSwgQSlbU10gIT0gNyB8fCBPYmplY3Qua2V5cygkYXNzaWduKHt9LCBCKSkuam9pbignJykgIT0gSztcbn0pID8gZnVuY3Rpb24gYXNzaWduKHRhcmdldCwgc291cmNlKXsgLy8gZXNsaW50LWRpc2FibGUtbGluZSBuby11bnVzZWQtdmFyc1xuICB2YXIgVCAgICAgPSB0b09iamVjdCh0YXJnZXQpXG4gICAgLCBhTGVuICA9IGFyZ3VtZW50cy5sZW5ndGhcbiAgICAsIGluZGV4ID0gMVxuICAgICwgZ2V0U3ltYm9scyA9IGdPUFMuZlxuICAgICwgaXNFbnVtICAgICA9IHBJRS5mO1xuICB3aGlsZShhTGVuID4gaW5kZXgpe1xuICAgIHZhciBTICAgICAgPSBJT2JqZWN0KGFyZ3VtZW50c1tpbmRleCsrXSlcbiAgICAgICwga2V5cyAgID0gZ2V0U3ltYm9scyA/IGdldEtleXMoUykuY29uY2F0KGdldFN5bWJvbHMoUykpIDogZ2V0S2V5cyhTKVxuICAgICAgLCBsZW5ndGggPSBrZXlzLmxlbmd0aFxuICAgICAgLCBqICAgICAgPSAwXG4gICAgICAsIGtleTtcbiAgICB3aGlsZShsZW5ndGggPiBqKWlmKGlzRW51bS5jYWxsKFMsIGtleSA9IGtleXNbaisrXSkpVFtrZXldID0gU1trZXldO1xuICB9IHJldHVybiBUO1xufSA6ICRhc3NpZ247XG5cblxuLyoqIFdFQlBBQ0sgRk9PVEVSICoqXG4gKiogL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL34vY29yZS1qcy9tb2R1bGVzL19vYmplY3QtYXNzaWduLmpzXG4gKiovIiwiZXhwb3J0cy5mID0gT2JqZWN0LmdldE93blByb3BlcnR5U3ltYm9scztcblxuXG4vKiogV0VCUEFDSyBGT09URVIgKipcbiAqKiAvVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvfi9jb3JlLWpzL21vZHVsZXMvX29iamVjdC1nb3BzLmpzXG4gKiovIiwiZXhwb3J0cy5mID0ge30ucHJvcGVydHlJc0VudW1lcmFibGU7XG5cblxuLyoqIFdFQlBBQ0sgRk9PVEVSICoqXG4gKiogL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL34vY29yZS1qcy9tb2R1bGVzL19vYmplY3QtcGllLmpzXG4gKiovIiwiLyoqXG4gKiBAZmlsZU92ZXJ2aWV3XG4gKiBSZWdpc3RlciBmcmFtZXdvcmsocykgaW4gSlMgcnVudGltZS4gV2VleCBzdXBwbHkgdHdvIGxheWVycyBmb3IgM3JkLXBhcnR5XG4gKiBmcmFtZXdvcmsocyk6IG9uZSBpcyB0aGUgaW5zdGFuY2UgbWFuYWdlbWVudCBsYXllciwgYW5vdGhlciBpcyB0aGVcbiAqIHZpcnR1YWwtRE9NIGxheWVyLlxuICovXG5cbi8vIFRoaXMgY29uZmlnIGlzIGdlbmVyYXRlZCBieSBgbnBtIHJ1biBidWlsZDpjb25maWdgLlxuLy8gSXQgd2lsbCBjb2xsZWN0IGFsbCBgcnVudGltZS9mcmFtZXdvcmstKi5qc2AgZmlsZXMgYW5kIGltcG9ydCBlYWNoIG9mIHRoZW0uXG4vLyBUaGUgZmlsZW5hbWUgaXMgYWxzbyB0aGUgZnJhbWV3b3JrIG5hbWUuXG5pbXBvcnQgZnJhbWV3b3JrcyBmcm9tICcuL2NvbmZpZydcblxuaW1wb3J0IHsgRG9jdW1lbnQsIEVsZW1lbnQsIENvbW1lbnQgfSBmcm9tICcuLi92ZG9tJ1xuXG5jb25zdCBjb25maWcgPSB7XG4gIERvY3VtZW50LCBFbGVtZW50LCBDb21tZW50LFxuICBzZW5kVGFza3MgKC4uLmFyZ3MpIHtcbiAgICByZXR1cm4gZ2xvYmFsLmNhbGxOYXRpdmUoLi4uYXJncylcbiAgfVxufVxuXG4vLyBJbml0IGVhY2ggZnJhbWV3b3JrIGJ5IGBpbml0YCBtZXRob2QgYW5kIGBjb25maWdgIHdoaWNoIGNvbnRhaW5zIHRocmVlXG4vLyB2aXJ0dWFsLURPTSBDbGFzczogYERvY3VtZW50YCwgYEVsZW1lbnRgICYgYENvbW1lbnRgLCBhbmQgYSBKUyBicmlkZ2UgbWV0aG9kOlxuLy8gYHNlbmRUYXNrcyguLi5hcmdzKWAuXG5mb3IgKGNvbnN0IG5hbWUgaW4gZnJhbWV3b3Jrcykge1xuICBjb25zdCBmcmFtZXdvcmsgPSBmcmFtZXdvcmtzW25hbWVdXG4gIGZyYW1ld29yay5pbml0KGNvbmZpZylcbn1cblxuY29uc3QgdmVyc2lvblJlZ0V4cCA9IC9eXFwvXFwvICooXFx7W15cXH1dKlxcfSkgKlxccj9cXG4vXG5cbi8qKlxuICogRGV0ZWN0IGEgSlMgQnVuZGxlIGNvZGUgYW5kIG1ha2Ugc3VyZSB3aGljaCBmcmFtZXdvcmsgaXQncyBiYXNlZCB0by4gRWFjaCBKU1xuICogQnVuZGxlIHNob3VsZCBtYWtlIHN1cmUgdGhhdCBpdCBzdGFydHMgd2l0aCBhIGxpbmUgb2YgSlNPTiBjb21tZW50IGFuZCBpc1xuICogbW9yZSB0aGF0IG9uZSBsaW5lLlxuICogQHBhcmFtICB7c3RyaW5nfSBjb2RlXG4gKiBAcmV0dXJuIHtvYmplY3R9XG4gKi9cbmZ1bmN0aW9uIGNoZWNrVmVyc2lvbiAoY29kZSkge1xuICBsZXQgaW5mb1xuICBjb25zdCByZXN1bHQgPSB2ZXJzaW9uUmVnRXhwLmV4ZWMoY29kZSlcbiAgaWYgKHJlc3VsdCkge1xuICAgIHRyeSB7XG4gICAgICBpbmZvID0gSlNPTi5wYXJzZShyZXN1bHRbMV0pXG4gICAgfVxuICAgIGNhdGNoIChlKSB7fVxuICB9XG4gIHJldHVybiBpbmZvXG59XG5cbmNvbnN0IGluc3RhbmNlTWFwID0ge31cblxuZ2xvYmFsLldYSW5zdGFuY2VNYXAgPSBpbnN0YW5jZU1hcFxuXG4vKipcbiAqIFByZXBhcmUgZW52IGZvciBhbiBuZXcgaW5zdGFuY2UgaW5zdGVhZCBjcmVhdGUgaXQgZGlyZWN0bHkuIFRoaXMgbW9kZSBjb3VsZFxuICogYXZvaWQgcnVubmluZyBgbmV3IEZ1bmN0aW9uYCBkdXJpbmcgY3JlYXRlIGFuIGluc3RhbmNlLlxuICogQHBhcmFtICB7c3RyaW5nfSBpZFxuICogQHBhcmFtICB7c3RyaW5nfSB0eXBlXG4gKiBAcGFyYW0gIHtvYmplY3R9IGNvbmZpZ1xuICogQHBhcmFtICB7b2JqZWN0fSBkYXRhXG4gKi9cbmV4cG9ydCBmdW5jdGlvbiBwcmVwYXJlSW5zdGFuY2UgKGlkLCB0eXBlLCBjb25maWcsIGRhdGEpIHtcbiAgdHlwZSA9IHR5cGUgfHwgJ1dlZXgnXG4gIGNvbnN0IGZyYW1ld29yayA9IGZyYW1ld29ya3NbdHlwZV1cbiAgaWYgKGZyYW1ld29yayAmJiBmcmFtZXdvcmsucHJlcGFyZUluc3RhbmNlKSB7XG4gICAgaW5zdGFuY2VNYXBbaWRdID0gZnJhbWV3b3JrLnByZXBhcmVJbnN0YW5jZShpZCwgY29uZmlnLCBkYXRhKVxuICAgIGluc3RhbmNlTWFwW2lkXS5mcmFtZXdvcmsgPSB0eXBlXG4gIH1cbiAgZWxzZSB7XG4gICAgaW5zdGFuY2VNYXBbaWRdID0ge31cbiAgfVxufVxuXG4vKipcbiAqIENoZWNrIHdoaWNoIGZyYW1ld29yayBhIGNlcnRhaW4gSlMgQnVuZGxlIGNvZGUgYmFzZWQgdG8uIEFuZCBjcmVhdGUgaW5zdGFuY2VcbiAqIGJ5IHRoaXMgZnJhbWV3b3JrLlxuICogQHBhcmFtIHtzdHJpbmd9IGlkXG4gKiBAcGFyYW0ge3N0cmluZ30gY29kZVxuICogQHBhcmFtIHtvYmplY3R9IGNvbmZpZ1xuICogQHBhcmFtIHtvYmplY3R9IGRhdGFcbiAqL1xuZnVuY3Rpb24gY3JlYXRlSW5zdGFuY2UgKGlkLCBjb2RlLCBjb25maWcsIGRhdGEpIHtcbiAgbGV0IGluZm8gPSBpbnN0YW5jZU1hcFtpZF1cbiAgaWYgKCFpbmZvKSB7XG4gICAgaW5mbyA9IGNoZWNrVmVyc2lvbihjb2RlKSB8fCB7fVxuICAgIGlmICghZnJhbWV3b3Jrc1tpbmZvLmZyYW1ld29ya10pIHtcbiAgICAgIGluZm8uZnJhbWV3b3JrID0gJ1dlZXgnXG4gICAgfVxuICAgIGluc3RhbmNlTWFwW2lkXSA9IGluZm9cbiAgICBjb25maWcgPSBjb25maWcgfHwge31cbiAgICBjb25maWcuYnVuZGxlVmVyc2lvbiA9IGluZm8udmVyc2lvblxuICAgIGNvbnNvbGUuZGVidWcoYFtKUyBGcmFtZXdvcmtdIGNyZWF0ZSBhbiAke2luZm8uZnJhbWV3b3JrfUAke2NvbmZpZy5idW5kbGVWZXJzaW9ufSBpbnN0YW5jZSBmcm9tICR7Y29uZmlnLmJ1bmRsZVZlcnNpb259YClcbiAgICByZXR1cm4gZnJhbWV3b3Jrc1tpbmZvLmZyYW1ld29ya10uY3JlYXRlSW5zdGFuY2UoaWQsIGNvZGUsIGNvbmZpZywgZGF0YSlcbiAgfVxuICByZXR1cm4gbmV3IEVycm9yKGBpbnZhbGlkIGluc3RhbmNlIGlkIFwiJHtpZH1cImApXG59XG5cbmNvbnN0IG1ldGhvZHMgPSB7XG4gIHByZXBhcmVJbnN0YW5jZSxcbiAgY3JlYXRlSW5zdGFuY2Vcbn1cblxuLyoqXG4gKiBSZWdpc3RlciBtZXRob2RzIHdoaWNoIGluaXQgZWFjaCBmcmFtZXdvcmtzLlxuICogQHBhcmFtIHtzdHJpbmd9IG1ldGhvZE5hbWVcbiAqL1xuZnVuY3Rpb24gZ2VuSW5pdCAobWV0aG9kTmFtZSkge1xuICBtZXRob2RzW21ldGhvZE5hbWVdID0gZnVuY3Rpb24gKC4uLmFyZ3MpIHtcbiAgICBmb3IgKGNvbnN0IG5hbWUgaW4gZnJhbWV3b3Jrcykge1xuICAgICAgY29uc3QgZnJhbWV3b3JrID0gZnJhbWV3b3Jrc1tuYW1lXVxuICAgICAgaWYgKGZyYW1ld29yayAmJiBmcmFtZXdvcmtbbWV0aG9kTmFtZV0pIHtcbiAgICAgICAgZnJhbWV3b3JrW21ldGhvZE5hbWVdKC4uLmFyZ3MpXG4gICAgICB9XG4gICAgfVxuICB9XG59XG5cbi8vIEB0b2RvOiBUaGUgbWV0aG9kIGByZWdpc3Rlck1ldGhvZHNgIHdpbGwgYmUgcmUtZGVzaWduZWQgb3IgcmVtb3ZlZCBsYXRlci5cblsncmVnaXN0ZXJDb21wb25lbnRzJywgJ3JlZ2lzdGVyTW9kdWxlcycsICdyZWdpc3Rlck1ldGhvZHMnXS5mb3JFYWNoKGdlbkluaXQpXG5cbi8qKlxuICogUmVnaXN0ZXIgbWV0aG9kcyB3aGljaCB3aWxsIGJlIGNhbGxlZCBmb3IgZWFjaCBpbnN0YW5jZS5cbiAqIEBwYXJhbSB7c3RyaW5nfSBtZXRob2ROYW1lXG4gKi9cbmZ1bmN0aW9uIGdlbkluc3RhbmNlIChtZXRob2ROYW1lKSB7XG4gIG1ldGhvZHNbbWV0aG9kTmFtZV0gPSBmdW5jdGlvbiAoLi4uYXJncykge1xuICAgIGNvbnN0IGlkID0gYXJnc1swXVxuICAgIGNvbnN0IGluZm8gPSBpbnN0YW5jZU1hcFtpZF1cbiAgICBpZiAobWV0aG9kTmFtZSA9PT0gJ2Rlc3Ryb3lJbnN0YW5jZScpIHtcbiAgICAgIGRlbGV0ZSBpbnN0YW5jZU1hcFtpZF1cbiAgICB9XG4gICAgaWYgKGluZm8gJiYgZnJhbWV3b3Jrc1tpbmZvLmZyYW1ld29ya10pIHtcbiAgICAgIHJldHVybiBmcmFtZXdvcmtzW2luZm8uZnJhbWV3b3JrXVttZXRob2ROYW1lXSguLi5hcmdzKVxuICAgIH1cbiAgICByZXR1cm4gbmV3IEVycm9yKGBpbnZhbGlkIGluc3RhbmNlIGlkIFwiJHtpZH1cImApXG4gIH1cbn1cblxuWydkZXN0cm95SW5zdGFuY2UnLCAncmVmcmVzaEluc3RhbmNlJywgJ3JlY2VpdmVUYXNrcycsICdnZXRSb290J10uZm9yRWFjaChnZW5JbnN0YW5jZSlcblxuLyoqXG4gKiBBZGFwdCBzb21lIGxlZ2FjeSBtZXRob2Qocykgd2hpY2ggd2lsbCBiZSBjYWxsZWQgZm9yIGVhY2ggaW5zdGFuY2UuIFRoZXNlXG4gKiBtZXRob2RzIHNob3VsZCBiZSBkZXByZWNhdGVkIGFuZCByZW1vdmVkIGxhdGVyLlxuICogQHBhcmFtIHtzdHJpbmd9IG1ldGhvZE5hbWVcbiAqIEBwYXJhbSB7c3RyaW5nfSBuYXRpdmVNZXRob2ROYW1lXG4gKi9cbmZ1bmN0aW9uIGFkYXB0SW5zdGFuY2UgKG1ldGhvZE5hbWUsIG5hdGl2ZU1ldGhvZE5hbWUpIHtcbiAgbWV0aG9kc1tuYXRpdmVNZXRob2ROYW1lXSA9IGZ1bmN0aW9uICguLi5hcmdzKSB7XG4gICAgY29uc3QgaWQgPSBhcmdzWzBdXG4gICAgY29uc3QgaW5mbyA9IGluc3RhbmNlTWFwW2lkXVxuICAgIGlmIChpbmZvICYmIGZyYW1ld29ya3NbaW5mby5mcmFtZXdvcmtdKSB7XG4gICAgICByZXR1cm4gZnJhbWV3b3Jrc1tpbmZvLmZyYW1ld29ya11bbWV0aG9kTmFtZV0oLi4uYXJncylcbiAgICB9XG4gICAgcmV0dXJuIG5ldyBFcnJvcihgaW52YWxpZCBpbnN0YW5jZSBpZCBcIiR7aWR9XCJgKVxuICB9XG59XG5cbmFkYXB0SW5zdGFuY2UoJ3JlY2VpdmVUYXNrcycsICdjYWxsSlMnKVxuXG5leHBvcnQgZGVmYXVsdCBtZXRob2RzXG5cblxuXG4vKiogV0VCUEFDSyBGT09URVIgKipcbiAqKiAvVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvc3JjL3J1bnRpbWUvaW5kZXguanNcbiAqKi8iLCIvLyBidWlsdCBieSBucG0gcnVuIGJ1aWxkOmNvbmZpZ1xuXG5pbXBvcnQgKiBhcyBXZWV4IGZyb20gJy4uL2RlZmF1bHQnXG5cbmV4cG9ydCBkZWZhdWx0IHtcbiAgV2VleFxufVxuXG5cblxuLyoqIFdFQlBBQ0sgRk9PVEVSICoqXG4gKiogL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL3NyYy9ydW50aW1lL2NvbmZpZy5qc1xuICoqLyIsIi8qKlxuICogQGZpbGVPdmVydmlldyBXZWV4IGZyYW1ld29yayBlbnRyeS5cbiAqL1xuXG5pbXBvcnQgY29uZmlnIGZyb20gJy4vY29uZmlnJ1xuaW1wb3J0IEFwcCBmcm9tICcuL2FwcCdcbmltcG9ydCB7XG4gIGluaXQgYXMgaW5pdEFwcCxcbiAgcmVmcmVzaCxcbiAgZGVzdHJveSxcbiAgZ2V0Um9vdEVsZW1lbnQsXG4gIGZpcmVFdmVudCxcbiAgY2FsbGJhY2tcbn0gZnJvbSAnLi9hcHAvY3RybCdcbmltcG9ydCB7XG4gIGluaXRNb2R1bGVzLFxuICBpbml0TWV0aG9kc1xufSBmcm9tICcuL2FwcC9yZWdpc3RlcidcbmltcG9ydCBWbSBmcm9tICcuL3ZtJ1xuXG5jb25zdCB7XG4gIG5hdGl2ZUNvbXBvbmVudE1hcFxufSA9IGNvbmZpZ1xuY29uc3QgaW5zdGFuY2VNYXAgPSB7fVxuXG4vKipcbiAqIEluaXQgY29uZmlnIGluZm9ybWF0aW9ucyBmb3IgV2VleCBmcmFtZXdvcmtcbiAqIEBwYXJhbSAge29iamVjdH0gY2ZnXG4gKi9cbmV4cG9ydCBmdW5jdGlvbiBpbml0IChjZmcpIHtcbiAgY29uZmlnLkRvY3VtZW50ID0gY2ZnLkRvY3VtZW50XG4gIGNvbmZpZy5FbGVtZW50ID0gY2ZnLkVsZW1lbnRcbiAgY29uZmlnLkNvbW1lbnQgPSBjZmcuQ29tbWVudFxuICBjb25maWcuc2VuZFRhc2tzID0gY2ZnLnNlbmRUYXNrc1xufVxuXG4vKipcbiAqIFByZXBhcmUgYSBXZWV4IGluc3RhbmNlIGFuZCByZXR1cm4gdGhlIGVudmlyb25tZW50LlxuICpcbiAqIEBwYXJhbSAge3N0cmluZ30gaWRcbiAqIEBwYXJhbSAge29iamVjdH0gb3B0aW9uc1xuICogQHBhcmFtICB7b2JqZWN0fSBkYXRhXG4gKiBAcmV0dXJuIHtvYmplY3R9XG4gKi9cbmV4cG9ydCBmdW5jdGlvbiBwcmVwYXJlSW5zdGFuY2UgKGlkLCBvcHRpb25zLCBkYXRhKSB7XG4gIGxldCBpbnN0YW5jZSA9IGluc3RhbmNlTWFwW2lkXVxuICBvcHRpb25zID0gb3B0aW9ucyB8fCB7fVxuICBsZXQgcmVzdWx0XG4gIGlmICghaW5zdGFuY2UpIHtcbiAgICBpbnN0YW5jZSA9IG5ldyBBcHAoaWQsIG9wdGlvbnMpXG4gICAgaW5zdGFuY2VNYXBbaWRdID0gaW5zdGFuY2VcbiAgICByZXN1bHQgPSBpbml0QXBwKGluc3RhbmNlLCAnJywgZGF0YSlcbiAgfVxuICByZXR1cm4gcmVzdWx0IHx8IHt9XG59XG5cbi8qKlxuICogQ3JlYXRlIGEgV2VleCBpbnN0YW5jZS5cbiAqXG4gKiBAcGFyYW0gIHtzdHJpbmd9IGlkXG4gKiBAcGFyYW0gIHtzdHJpbmd9IGNvZGVcbiAqIEBwYXJhbSAge29iamVjdH0gW29wdGlvbnNdIG9wdGlvbiBgSEFTX0xPR2AgZW5hYmxlIHByaW50IGxvZ1xuICogQHBhcmFtICB7b2JqZWN0fSBbZGF0YV1cbiAqL1xuZXhwb3J0IGZ1bmN0aW9uIGNyZWF0ZUluc3RhbmNlIChpZCwgY29kZSwgb3B0aW9ucywgZGF0YSkge1xuICBsZXQgaW5zdGFuY2UgPSBpbnN0YW5jZU1hcFtpZF1cbiAgb3B0aW9ucyA9IG9wdGlvbnMgfHwge31cbiAgbGV0IHJlc3VsdFxuICBpZiAoIWluc3RhbmNlKSB7XG4gICAgaW5zdGFuY2UgPSBuZXcgQXBwKGlkLCBvcHRpb25zKVxuICAgIGluc3RhbmNlTWFwW2lkXSA9IGluc3RhbmNlXG4gICAgcmVzdWx0ID0gaW5pdEFwcChpbnN0YW5jZSwgY29kZSwgZGF0YSlcbiAgfVxuICBlbHNlIHtcbiAgICByZXN1bHQgPSBuZXcgRXJyb3IoYGludmFsaWQgaW5zdGFuY2UgaWQgXCIke2lkfVwiYClcbiAgfVxuICByZXR1cm4gcmVzdWx0XG59XG5cbi8qKlxuICogUmVmcmVzaCBhIFdlZXggaW5zdGFuY2Ugd2l0aCBkYXRhLlxuICpcbiAqIEBwYXJhbSAge3N0cmluZ30gaWRcbiAqIEBwYXJhbSAge29iamVjdH0gZGF0YVxuICovXG5leHBvcnQgZnVuY3Rpb24gcmVmcmVzaEluc3RhbmNlIChpZCwgZGF0YSkge1xuICBjb25zdCBpbnN0YW5jZSA9IGluc3RhbmNlTWFwW2lkXVxuICBsZXQgcmVzdWx0XG4gIGlmIChpbnN0YW5jZSkge1xuICAgIHJlc3VsdCA9IHJlZnJlc2goaW5zdGFuY2UsIGRhdGEpXG4gIH1cbiAgZWxzZSB7XG4gICAgcmVzdWx0ID0gbmV3IEVycm9yKGBpbnZhbGlkIGluc3RhbmNlIGlkIFwiJHtpZH1cImApXG4gIH1cbiAgcmV0dXJuIHJlc3VsdFxufVxuXG4vKipcbiAqIERlc3Ryb3kgYSBXZWV4IGluc3RhbmNlLlxuICogQHBhcmFtICB7c3RyaW5nfSBpZFxuICovXG5leHBvcnQgZnVuY3Rpb24gZGVzdHJveUluc3RhbmNlIChpZCkge1xuICBjb25zdCBpbnN0YW5jZSA9IGluc3RhbmNlTWFwW2lkXVxuICBpZiAoIWluc3RhbmNlKSB7XG4gICAgcmV0dXJuIG5ldyBFcnJvcihgaW52YWxpZCBpbnN0YW5jZSBpZCBcIiR7aWR9XCJgKVxuICB9XG4gIGRlc3Ryb3koaW5zdGFuY2UpXG4gIGRlbGV0ZSBpbnN0YW5jZU1hcFtpZF1cbiAgcmV0dXJuIGluc3RhbmNlTWFwXG59XG5cbi8qKlxuICogUmVnaXN0ZXIgdGhlIG5hbWUgb2YgZWFjaCBuYXRpdmUgY29tcG9uZW50LlxuICogQHBhcmFtICB7YXJyYXl9IGNvbXBvbmVudHMgYXJyYXkgb2YgbmFtZVxuICovXG5leHBvcnQgZnVuY3Rpb24gcmVnaXN0ZXJDb21wb25lbnRzIChjb21wb25lbnRzKSB7XG4gIGlmIChBcnJheS5pc0FycmF5KGNvbXBvbmVudHMpKSB7XG4gICAgY29tcG9uZW50cy5mb3JFYWNoKGZ1bmN0aW9uIHJlZ2lzdGVyIChuYW1lKSB7XG4gICAgICAvKiBpc3RhbmJ1bCBpZ25vcmUgaWYgKi9cbiAgICAgIGlmICghbmFtZSkge1xuICAgICAgICByZXR1cm5cbiAgICAgIH1cbiAgICAgIGlmICh0eXBlb2YgbmFtZSA9PT0gJ3N0cmluZycpIHtcbiAgICAgICAgbmF0aXZlQ29tcG9uZW50TWFwW25hbWVdID0gdHJ1ZVxuICAgICAgfVxuICAgICAgZWxzZSBpZiAodHlwZW9mIG5hbWUgPT09ICdvYmplY3QnICYmIHR5cGVvZiBuYW1lLnR5cGUgPT09ICdzdHJpbmcnKSB7XG4gICAgICAgIG5hdGl2ZUNvbXBvbmVudE1hcFtuYW1lLnR5cGVdID0gbmFtZVxuICAgICAgfVxuICAgIH0pXG4gIH1cbn1cblxuLyoqXG4gKiBSZWdpc3RlciB0aGUgbmFtZSBhbmQgbWV0aG9kcyBvZiBlYWNoIG1vZHVsZS5cbiAqIEBwYXJhbSAge29iamVjdH0gbW9kdWxlcyBhIG9iamVjdCBvZiBtb2R1bGVzXG4gKi9cbmV4cG9ydCBmdW5jdGlvbiByZWdpc3Rlck1vZHVsZXMgKG1vZHVsZXMpIHtcbiAgaWYgKHR5cGVvZiBtb2R1bGVzID09PSAnb2JqZWN0Jykge1xuICAgIGluaXRNb2R1bGVzKG1vZHVsZXMpXG4gIH1cbn1cblxuLyoqXG4gKiBSZWdpc3RlciB0aGUgbmFtZSBhbmQgbWV0aG9kcyBvZiBlYWNoIGFwaS5cbiAqIEBwYXJhbSAge29iamVjdH0gYXBpcyBhIG9iamVjdCBvZiBhcGlzXG4gKi9cbmV4cG9ydCBmdW5jdGlvbiByZWdpc3Rlck1ldGhvZHMgKG1ldGhvZHMpIHtcbiAgaWYgKHR5cGVvZiBtZXRob2RzID09PSAnb2JqZWN0Jykge1xuICAgIGluaXRNZXRob2RzKFZtLCBtZXRob2RzKVxuICB9XG59XG5cbi8vIEB0b2RvOiBIYWNrIGZvciB0aGlzIGZyYW1ld29yayBvbmx5LiBXaWxsIGJlIHJlLWRlc2lnbmVkIG9yIHJlbW92ZWQgbGF0ZXIuXG5nbG9iYWwucmVnaXN0ZXJNZXRob2RzID0gcmVnaXN0ZXJNZXRob2RzXG5cbi8qKlxuICogR2V0IGEgd2hvbGUgZWxlbWVudCB0cmVlIG9mIGFuIGluc3RhbmNlIGZvciBkZWJ1Z2dpbmcuXG4gKiBAcGFyYW0gIHtzdHJpbmd9IGlkXG4gKiBAcmV0dXJuIHtvYmplY3R9IGEgdmlydHVhbCBkb20gdHJlZVxuICovXG5leHBvcnQgZnVuY3Rpb24gZ2V0Um9vdCAoaWQpIHtcbiAgY29uc3QgaW5zdGFuY2UgPSBpbnN0YW5jZU1hcFtpZF1cbiAgbGV0IHJlc3VsdFxuICBpZiAoaW5zdGFuY2UpIHtcbiAgICByZXN1bHQgPSBnZXRSb290RWxlbWVudChpbnN0YW5jZSlcbiAgfVxuICBlbHNlIHtcbiAgICByZXN1bHQgPSBuZXcgRXJyb3IoYGludmFsaWQgaW5zdGFuY2UgaWQgXCIke2lkfVwiYClcbiAgfVxuICByZXR1cm4gcmVzdWx0XG59XG5cbmNvbnN0IGpzSGFuZGxlcnMgPSB7XG4gIGZpcmVFdmVudDogKGlkLCAuLi5hcmdzKSA9PiB7XG4gICAgcmV0dXJuIGZpcmVFdmVudChpbnN0YW5jZU1hcFtpZF0sIC4uLmFyZ3MpXG4gIH0sXG4gIGNhbGxiYWNrOiAoaWQsIC4uLmFyZ3MpID0+IHtcbiAgICByZXR1cm4gY2FsbGJhY2soaW5zdGFuY2VNYXBbaWRdLCAuLi5hcmdzKVxuICB9XG59XG5cbi8qKlxuICogQWNjZXB0IGNhbGxzIGZyb20gbmF0aXZlIChldmVudCBvciBjYWxsYmFjaykuXG4gKlxuICogQHBhcmFtICB7c3RyaW5nfSBpZFxuICogQHBhcmFtICB7YXJyYXl9IHRhc2tzIGxpc3Qgd2l0aCBgbWV0aG9kYCBhbmQgYGFyZ3NgXG4gKi9cbmV4cG9ydCBmdW5jdGlvbiByZWNlaXZlVGFza3MgKGlkLCB0YXNrcykge1xuICBjb25zdCBpbnN0YW5jZSA9IGluc3RhbmNlTWFwW2lkXVxuICBpZiAoaW5zdGFuY2UgJiYgQXJyYXkuaXNBcnJheSh0YXNrcykpIHtcbiAgICBjb25zdCByZXN1bHRzID0gW11cbiAgICB0YXNrcy5mb3JFYWNoKCh0YXNrKSA9PiB7XG4gICAgICBjb25zdCBoYW5kbGVyID0ganNIYW5kbGVyc1t0YXNrLm1ldGhvZF1cbiAgICAgIGNvbnN0IGFyZ3MgPSBbLi4udGFzay5hcmdzXVxuICAgICAgaWYgKHR5cGVvZiBoYW5kbGVyID09PSAnZnVuY3Rpb24nKSB7XG4gICAgICAgIGFyZ3MudW5zaGlmdChpZClcbiAgICAgICAgcmVzdWx0cy5wdXNoKGhhbmRsZXIoLi4uYXJncykpXG4gICAgICB9XG4gICAgfSlcbiAgICByZXR1cm4gcmVzdWx0c1xuICB9XG4gIHJldHVybiBuZXcgRXJyb3IoYGludmFsaWQgaW5zdGFuY2UgaWQgXCIke2lkfVwiIG9yIHRhc2tzYClcbn1cblxuXG5cbi8qKiBXRUJQQUNLIEZPT1RFUiAqKlxuICoqIC9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9zcmMvZGVmYXVsdC9pbmRleC5qc1xuICoqLyIsIi8vIEB0b2RvOiBJdCBzaG91bGQgYmUgcmVnaXN0ZXJlZCBieSBuYXRpdmUgZnJvbSBgcmVnaXN0ZXJDb21wb25lbnRzKClgLlxuZXhwb3J0IGRlZmF1bHQge1xuICBuYXRpdmVDb21wb25lbnRNYXA6IHtcbiAgICB0ZXh0OiB0cnVlLFxuICAgIGltYWdlOiB0cnVlLFxuICAgIGNvbnRhaW5lcjogdHJ1ZSxcbiAgICBzbGlkZXI6IHtcbiAgICAgIHR5cGU6ICdzbGlkZXInLFxuICAgICAgYXBwZW5kOiAndHJlZSdcbiAgICB9LFxuICAgIGNlbGw6IHtcbiAgICAgIHR5cGU6ICdjZWxsJyxcbiAgICAgIGFwcGVuZDogJ3RyZWUnXG4gICAgfVxuICB9XG59XG5cblxuXG4vKiogV0VCUEFDSyBGT09URVIgKipcbiAqKiAvVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvc3JjL2RlZmF1bHQvY29uZmlnLmpzXG4gKiovIiwiLyoqXG4gKiBAZmlsZU92ZXJ2aWV3XG4gKiBXZWV4IGluc3RhbmNlIGNvbnN0cnVjdG9yICYgZGVmaW5pdGlvblxuICovXG5cbmltcG9ydCBEaWZmZXIgZnJvbSAnLi9kaWZmZXInXG5cbmltcG9ydCByZW5kZXJlciBmcm9tICcuLi9jb25maWcnXG5pbXBvcnQgeyByZXF1aXJlTW9kdWxlIH0gZnJvbSAnLi9yZWdpc3RlcidcbmltcG9ydCB7IHVwZGF0ZUFjdGlvbnMsIGNhbGxUYXNrcyB9IGZyb20gJy4vY3RybCdcblxuLyoqXG4gKiBBcHAgY29uc3RydWN0b3IgZm9yIFdlZXggZnJhbWV3b3JrLlxuICogQHBhcmFtIHtzdHJpbmd9IGlkXG4gKiBAcGFyYW0ge29iamVjdH0gb3B0aW9uc1xuICovXG5leHBvcnQgZGVmYXVsdCBmdW5jdGlvbiBBcHAgKGlkLCBvcHRpb25zKSB7XG4gIHRoaXMuaWQgPSBpZFxuICB0aGlzLm9wdGlvbnMgPSBvcHRpb25zIHx8IHt9XG4gIHRoaXMudm0gPSBudWxsXG4gIHRoaXMuY3VzdG9tQ29tcG9uZW50TWFwID0ge31cbiAgdGhpcy5jYWxsYmFja3MgPSB7fVxuICB0aGlzLmRvYyA9IG5ldyByZW5kZXJlci5Eb2N1bWVudChpZCwgdGhpcy5vcHRpb25zLmJ1bmRsZVVybClcbiAgdGhpcy5kaWZmZXIgPSBuZXcgRGlmZmVyKGlkKVxuICB0aGlzLnVpZCA9IDBcbn1cblxuLyoqXG4gKiBAZGVwcmVjYXRlZFxuICovXG5BcHAucHJvdG90eXBlLnJlcXVpcmVNb2R1bGUgPSBmdW5jdGlvbiAobmFtZSkge1xuICByZXR1cm4gcmVxdWlyZU1vZHVsZSh0aGlzLCBuYW1lKVxufVxuXG4vKipcbiAqIEBkZXByZWNhdGVkXG4gKi9cbkFwcC5wcm90b3R5cGUudXBkYXRlQWN0aW9ucyA9IGZ1bmN0aW9uICgpIHtcbiAgdXBkYXRlQWN0aW9ucyh0aGlzKVxufVxuXG4vKipcbiAqIEBkZXByZWNhdGVkXG4gKi9cbkFwcC5wcm90b3R5cGUuY2FsbFRhc2tzID0gZnVuY3Rpb24gKHRhc2tzKSB7XG4gIGNhbGxUYXNrcyh0aGlzLCB0YXNrcylcbn1cblxuXG5cbi8qKiBXRUJQQUNLIEZPT1RFUiAqKlxuICoqIC9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9zcmMvZGVmYXVsdC9hcHAvaW5kZXguanNcbiAqKi8iLCJleHBvcnQgZGVmYXVsdCBjbGFzcyBEaWZmZXIge1xuICBjb25zdHJ1Y3RvciAoaWQpIHtcbiAgICB0aGlzLmlkID0gaWRcbiAgICB0aGlzLm1hcCA9IFtdXG4gICAgdGhpcy5ob29rcyA9IFtdXG4gIH1cbiAgaXNFbXB0eSAoKSB7XG4gICAgcmV0dXJuIHRoaXMubWFwLmxlbmd0aCA9PT0gMFxuICB9XG4gIGFwcGVuZCAodHlwZSwgZGVwdGgsIHJlZiwgaGFuZGxlcikge1xuICAgIGlmICghdGhpcy5oYXNUaW1lcikge1xuICAgICAgdGhpcy5oYXNUaW1lciA9IHRydWVcbiAgICAgIHNldFRpbWVvdXQoKCkgPT4ge1xuICAgICAgICB0aGlzLmhhc1RpbWVyID0gZmFsc2VcbiAgICAgICAgdGhpcy5mbHVzaCh0cnVlKVxuICAgICAgfSwgMClcbiAgICB9XG4gICAgY29uc3QgbWFwID0gdGhpcy5tYXBcbiAgICBpZiAoIW1hcFtkZXB0aF0pIHtcbiAgICAgIG1hcFtkZXB0aF0gPSB7fVxuICAgIH1cbiAgICBjb25zdCBncm91cCA9IG1hcFtkZXB0aF1cbiAgICBpZiAoIWdyb3VwW3R5cGVdKSB7XG4gICAgICBncm91cFt0eXBlXSA9IHt9XG4gICAgfVxuICAgIGlmICh0eXBlID09PSAnZWxlbWVudCcpIHtcbiAgICAgIGlmICghZ3JvdXBbdHlwZV1bcmVmXSkge1xuICAgICAgICBncm91cFt0eXBlXVtyZWZdID0gW11cbiAgICAgIH1cbiAgICAgIGdyb3VwW3R5cGVdW3JlZl0ucHVzaChoYW5kbGVyKVxuICAgIH1cbiAgICBlbHNlIHtcbiAgICAgIGdyb3VwW3R5cGVdW3JlZl0gPSBoYW5kbGVyXG4gICAgfVxuICB9XG4gIGZsdXNoIChpc1RpbWVvdXQpIHtcbiAgICBjb25zdCBtYXAgPSB0aGlzLm1hcC5zbGljZSgpXG4gICAgdGhpcy5tYXAubGVuZ3RoID0gMFxuICAgIG1hcC5mb3JFYWNoKChncm91cCkgPT4ge1xuICAgICAgY2FsbFR5cGVNYXAoZ3JvdXAsICdyZXBlYXQnKVxuICAgICAgY2FsbFR5cGVNYXAoZ3JvdXAsICdzaG93bicpXG4gICAgICBjYWxsVHlwZUxpc3QoZ3JvdXAsICdlbGVtZW50JylcbiAgICB9KVxuXG4gICAgY29uc3QgaG9va3MgPSB0aGlzLmhvb2tzLnNsaWNlKClcbiAgICB0aGlzLmhvb2tzLmxlbmd0aCA9IDBcbiAgICBob29rcy5mb3JFYWNoKChmbikgPT4ge1xuICAgICAgZm4oKVxuICAgIH0pXG5cbiAgICBpZiAoIXRoaXMuaXNFbXB0eSgpKSB7XG4gICAgICB0aGlzLmZsdXNoKClcbiAgICB9XG4gIH1cbiAgdGhlbiAoZm4pIHtcbiAgICB0aGlzLmhvb2tzLnB1c2goZm4pXG4gIH1cbn1cblxuZnVuY3Rpb24gY2FsbFR5cGVNYXAgKGdyb3VwLCB0eXBlKSB7XG4gIGNvbnN0IG1hcCA9IGdyb3VwW3R5cGVdXG4gIGZvciAoY29uc3QgcmVmIGluIG1hcCkge1xuICAgIG1hcFtyZWZdKClcbiAgfVxufVxuXG5mdW5jdGlvbiBjYWxsVHlwZUxpc3QgKGdyb3VwLCB0eXBlKSB7XG4gIGNvbnN0IG1hcCA9IGdyb3VwW3R5cGVdXG4gIGZvciAoY29uc3QgcmVmIGluIG1hcCkge1xuICAgIGNvbnN0IGxpc3QgPSBtYXBbcmVmXVxuICAgIGxpc3QuZm9yRWFjaCgoaGFuZGxlcikgPT4geyBoYW5kbGVyKCkgfSlcbiAgfVxufVxuXG5cblxuLyoqIFdFQlBBQ0sgRk9PVEVSICoqXG4gKiogL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL3NyYy9kZWZhdWx0L2FwcC9kaWZmZXIuanNcbiAqKi8iLCJsZXQgbmF0aXZlTW9kdWxlcyA9IHt9XG5cbi8vIGZvciB0ZXN0aW5nXG5cbi8qKlxuICogZm9yIHRlc3RpbmdcbiAqL1xuZXhwb3J0IGZ1bmN0aW9uIGdldE1vZHVsZSAobW9kdWxlTmFtZSkge1xuICByZXR1cm4gbmF0aXZlTW9kdWxlc1ttb2R1bGVOYW1lXVxufVxuXG4vKipcbiAqIGZvciB0ZXN0aW5nXG4gKi9cbmV4cG9ydCBmdW5jdGlvbiBjbGVhck1vZHVsZXMgKCkge1xuICBuYXRpdmVNb2R1bGVzID0ge31cbn1cblxuLy8gZm9yIGZyYW1ld29ya1xuXG4vKipcbiAqIGluaXQgbW9kdWxlcyBmb3IgYW4gYXBwIGluc3RhbmNlXG4gKiB0aGUgc2Vjb25kIHBhcmFtIGRldGVybWluZXMgd2hldGhlciB0byByZXBsYWNlIGFuIGV4aXN0ZWQgbWV0aG9kXG4gKi9cbmV4cG9ydCBmdW5jdGlvbiBpbml0TW9kdWxlcyAobW9kdWxlcywgaWZSZXBsYWNlKSB7XG4gIGZvciAoY29uc3QgbW9kdWxlTmFtZSBpbiBtb2R1bGVzKSB7XG4gICAgLy8gaW5pdCBgbW9kdWxlc1ttb2R1bGVOYW1lXVtdYFxuICAgIGxldCBtZXRob2RzID0gbmF0aXZlTW9kdWxlc1ttb2R1bGVOYW1lXVxuICAgIGlmICghbWV0aG9kcykge1xuICAgICAgbWV0aG9kcyA9IHt9XG4gICAgICBuYXRpdmVNb2R1bGVzW21vZHVsZU5hbWVdID0gbWV0aG9kc1xuICAgIH1cblxuICAgIC8vIHB1c2ggZWFjaCBub24tZXhpc3RlZCBuZXcgbWV0aG9kXG4gICAgbW9kdWxlc1ttb2R1bGVOYW1lXS5mb3JFYWNoKGZ1bmN0aW9uIChtZXRob2QpIHtcbiAgICAgIGlmICh0eXBlb2YgbWV0aG9kID09PSAnc3RyaW5nJykge1xuICAgICAgICBtZXRob2QgPSB7XG4gICAgICAgICAgbmFtZTogbWV0aG9kXG4gICAgICAgIH1cbiAgICAgIH1cblxuICAgICAgaWYgKCFtZXRob2RzW21ldGhvZC5uYW1lXSB8fCBpZlJlcGxhY2UpIHtcbiAgICAgICAgbWV0aG9kc1ttZXRob2QubmFtZV0gPSBtZXRob2RcbiAgICAgIH1cbiAgICB9KVxuICB9XG59XG5cbi8qKlxuICogaW5pdCBhcHAgbWV0aG9kc1xuICovXG5leHBvcnQgZnVuY3Rpb24gaW5pdE1ldGhvZHMgKFZtLCBhcGlzKSB7XG4gIGNvbnN0IHAgPSBWbS5wcm90b3R5cGVcblxuICBmb3IgKGNvbnN0IGFwaU5hbWUgaW4gYXBpcykge1xuICAgIGlmICghcC5oYXNPd25Qcm9wZXJ0eShhcGlOYW1lKSkge1xuICAgICAgcFthcGlOYW1lXSA9IGFwaXNbYXBpTmFtZV1cbiAgICB9XG4gIH1cbn1cblxuLy8gZm9yIGFwcFxuXG4vKipcbiAqIGdldCBhIG1vZHVsZSBvZiBtZXRob2RzIGZvciBhbiBhcHAgaW5zdGFuY2VcbiAqL1xuZXhwb3J0IGZ1bmN0aW9uIHJlcXVpcmVNb2R1bGUgKGFwcCwgbmFtZSkge1xuICBjb25zdCBtZXRob2RzID0gbmF0aXZlTW9kdWxlc1tuYW1lXVxuICBjb25zdCB0YXJnZXQgPSB7fVxuICBmb3IgKGNvbnN0IG1ldGhvZE5hbWUgaW4gbWV0aG9kcykge1xuICAgIHRhcmdldFttZXRob2ROYW1lXSA9ICguLi5hcmdzKSA9PiBhcHAuY2FsbFRhc2tzKHtcbiAgICAgIG1vZHVsZTogbmFtZSxcbiAgICAgIG1ldGhvZDogbWV0aG9kTmFtZSxcbiAgICAgIGFyZ3M6IGFyZ3NcbiAgICB9KVxuICB9XG4gIHJldHVybiB0YXJnZXRcbn1cblxuLyoqXG4gKiBnZXQgYSBjdXN0b20gY29tcG9uZW50IG9wdGlvbnNcbiAqL1xuZXhwb3J0IGZ1bmN0aW9uIHJlcXVpcmVDdXN0b21Db21wb25lbnQgKGFwcCwgbmFtZSkge1xuICBjb25zdCB7IGN1c3RvbUNvbXBvbmVudE1hcCB9ID0gYXBwXG4gIHJldHVybiBjdXN0b21Db21wb25lbnRNYXBbbmFtZV1cbn1cblxuLyoqXG4gKiByZWdpc3RlciBhIGN1c3RvbSBjb21wb25lbnQgb3B0aW9uc1xuICovXG5leHBvcnQgZnVuY3Rpb24gcmVnaXN0ZXJDdXN0b21Db21wb25lbnQgKGFwcCwgbmFtZSwgZGVmKSB7XG4gIGNvbnN0IHsgY3VzdG9tQ29tcG9uZW50TWFwIH0gPSBhcHBcblxuICBpZiAoY3VzdG9tQ29tcG9uZW50TWFwW25hbWVdKSB7XG4gICAgY29uc29sZS5lcnJvcihgW0pTIEZyYW1ld29ya10gZGVmaW5lIGEgY29tcG9uZW50KCR7bmFtZX0pIHRoYXQgYWxyZWFkeSBleGlzdHNgKVxuICAgIHJldHVyblxuICB9XG5cbiAgY3VzdG9tQ29tcG9uZW50TWFwW25hbWVdID0gZGVmXG59XG5cblxuXG4vKiogV0VCUEFDSyBGT09URVIgKipcbiAqKiAvVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvc3JjL2RlZmF1bHQvYXBwL3JlZ2lzdGVyLmpzXG4gKiovIiwiLyoqXG4gKiBAZmlsZU92ZXJ2aWV3XG4gKiBpbnN0YW5jZSBjb250cm9scyBmcm9tIG5hdGl2ZVxuICpcbiAqIC0gaW5pdCBidW5kbGVcbiAqIC0gZmlyZSBldmVudFxuICogLSBjYWxsYmFja1xuICogLSByZWZyZXNoXG4gKiAtIGRlc3Ryb3lcbiAqXG4gKiBjb3JyZXNwb25kZWQgd2l0aCB0aGUgQVBJIG9mIGluc3RhbmNlIG1hbmFnZXIgKGZyYW1ld29yay5qcylcbiAqL1xuXG5pbXBvcnQgeyBleHRlbmQsIHR5cG9mLCByZW1vdmVXZWV4UHJlZml4IH0gZnJvbSAnLi4vdXRpbCdcbmltcG9ydCByZW5kZXJlciBmcm9tICcuLi9jb25maWcnXG5pbXBvcnQge1xuICBkZWZpbmVGbixcbiAgYm9vdHN0cmFwLFxuICByZWdpc3RlclxufSBmcm9tICcuL2J1bmRsZSdcblxuLyoqXG4gKiBJbml0IGFuIGFwcCBieSBydW4gY29kZSB3aXRoIGRhdGEuXG4gKiBJbiBuYXRpdmUgcmVuZGVyZXIgaWYgdGhlIGNvZGUgaXMgZW1wdHkgdGhlbiB3ZSBqdXN0IHByZXBhcmUgYW4gaW5zdGFuY2VcbiAqIGVudiBvbmx5IGZvciBydW4gdGhlIHJlYWwgY29kZSBsYXRlci5cbiAqIEBwYXJhbSAge29iamVjdH0gYXBwXG4gKiBAcGFyYW0gIHtzdHJpbmd9IGNvZGVcbiAqIEBwYXJhbSAge29iamVjdH0gZGF0YVxuICovXG5leHBvcnQgZnVuY3Rpb24gaW5pdCAoYXBwLCBjb2RlLCBkYXRhKSB7XG4gIGNvbnNvbGUuZGVidWcoJ1tKUyBGcmFtZXdvcmtdIEludGlhbGl6ZSBhbiBpbnN0YW5jZSB3aXRoOlxcbicsIGRhdGEpXG4gIGxldCByZXN1bHRcblxuICAvLyBwcmVwYXJlIGFwcCBlbnYgbWV0aG9kc1xuICBjb25zdCBidW5kbGVEZWZpbmUgPSAoLi4uYXJncykgPT4gZGVmaW5lRm4oYXBwLCAuLi5hcmdzKVxuICBjb25zdCBidW5kbGVCb290c3RyYXAgPSAobmFtZSwgY29uZmlnLCBfZGF0YSkgPT4ge1xuICAgIHJlc3VsdCA9IGJvb3RzdHJhcChhcHAsIG5hbWUsIGNvbmZpZywgX2RhdGEgfHwgZGF0YSlcbiAgICB1cGRhdGVBY3Rpb25zKGFwcClcbiAgICBhcHAuZG9jLmxpc3RlbmVyLmNyZWF0ZUZpbmlzaCgpXG4gICAgY29uc29sZS5kZWJ1ZyhgW0pTIEZyYW1ld29ya10gQWZ0ZXIgaW50aWFsaXplZCBhbiBpbnN0YW5jZSgke2FwcC5pZH0pYClcbiAgfVxuICBjb25zdCBidW5kbGVSZWdpc3RlciA9ICguLi5hcmdzKSA9PiByZWdpc3RlcihhcHAsIC4uLmFyZ3MpXG4gIGNvbnN0IGJ1bmRsZVJlbmRlciA9IChuYW1lLCBfZGF0YSkgPT4ge1xuICAgIHJlc3VsdCA9IGJvb3RzdHJhcChhcHAsIG5hbWUsIHt9LCBfZGF0YSlcbiAgfVxuICBjb25zdCBidW5kbGVSZXF1aXJlID0gbmFtZSA9PiBfZGF0YSA9PiB7XG4gICAgcmVzdWx0ID0gYm9vdHN0cmFwKGFwcCwgbmFtZSwge30sIF9kYXRhKVxuICB9XG4gIGNvbnN0IGJ1bmRsZURvY3VtZW50ID0gYXBwLmRvY1xuICBjb25zdCBidW5kbGVSZXF1aXJlTW9kdWxlID0gbmFtZSA9PiBhcHAucmVxdWlyZU1vZHVsZShyZW1vdmVXZWV4UHJlZml4KG5hbWUpKVxuXG4gIC8vIHByZXBhcmUgY29kZVxuICBsZXQgZnVuY3Rpb25Cb2R5XG4gIC8qIGlzdGFuYnVsIGlnbm9yZSBpZiAqL1xuICBpZiAodHlwZW9mIGNvZGUgPT09ICdmdW5jdGlvbicpIHtcbiAgICAvLyBgZnVuY3Rpb24gKCkgey4uLn1gIC0+IGB7Li4ufWBcbiAgICAvLyBub3QgdmVyeSBzdHJpY3RcbiAgICBmdW5jdGlvbkJvZHkgPSBjb2RlLnRvU3RyaW5nKCkuc3Vic3RyKDEyKVxuICB9XG4gIGVsc2UgaWYgKGNvZGUpIHtcbiAgICBmdW5jdGlvbkJvZHkgPSBjb2RlLnRvU3RyaW5nKClcbiAgfVxuXG4gIC8vIHJ1biBjb2RlIGFuZCBnZXQgcmVzdWx0XG4gIGNvbnN0IHsgV1hFbnZpcm9ubWVudCB9ID0gZ2xvYmFsXG4gIGlmIChXWEVudmlyb25tZW50ICYmIFdYRW52aXJvbm1lbnQucGxhdGZvcm0gIT09ICdXZWInKSB7XG4gICAgLy8gdGltZXIgQVBJcyBwb2x5ZmlsbCBpbiBuYXRpdmVcbiAgICBjb25zdCB0aW1lciA9IGFwcC5yZXF1aXJlTW9kdWxlKCd0aW1lcicpXG4gICAgY29uc3QgdGltZXJBUElzID0ge1xuICAgICAgc2V0VGltZW91dDogKC4uLmFyZ3MpID0+IHtcbiAgICAgICAgY29uc3QgaGFuZGxlciA9IGZ1bmN0aW9uICgpIHtcbiAgICAgICAgICBhcmdzWzBdKC4uLmFyZ3Muc2xpY2UoMikpXG4gICAgICAgIH1cbiAgICAgICAgdGltZXIuc2V0VGltZW91dChoYW5kbGVyLCBhcmdzWzFdKVxuICAgICAgICByZXR1cm4gYXBwLnVpZC50b1N0cmluZygpXG4gICAgICB9LFxuICAgICAgc2V0SW50ZXJ2YWw6ICguLi5hcmdzKSA9PiB7XG4gICAgICAgIGNvbnN0IGhhbmRsZXIgPSBmdW5jdGlvbiAoKSB7XG4gICAgICAgICAgYXJnc1swXSguLi5hcmdzLnNsaWNlKDIpKVxuICAgICAgICB9XG4gICAgICAgIHRpbWVyLnNldEludGVydmFsKGhhbmRsZXIsIGFyZ3NbMV0pXG4gICAgICAgIHJldHVybiBhcHAudWlkLnRvU3RyaW5nKClcbiAgICAgIH0sXG4gICAgICBjbGVhclRpbWVvdXQ6IChuKSA9PiB7XG4gICAgICAgIHRpbWVyLmNsZWFyVGltZW91dChuKVxuICAgICAgfSxcbiAgICAgIGNsZWFySW50ZXJ2YWw6IChuKSA9PiB7XG4gICAgICAgIHRpbWVyLmNsZWFySW50ZXJ2YWwobilcbiAgICAgIH1cbiAgICB9XG5cbiAgICAvLyBTcGVjaWFsOiBpZiB0aGUgY29kZSBpcyBlbXB0eSB0aGF0IG1lYW5zIHdlIG9ubHkgbmVlZCB0byBwcmVwYXJlIGFuXG4gICAgLy8gZW52aXJvbm1lbnQuIElmIHNvIHdlIGp1c3QgcmV0dXJuIGEgZW52aW9ybWVudCBvYmplY3QuXG4gICAgaWYgKCFjb2RlKSB7XG4gICAgICByZXR1cm4ge1xuICAgICAgICBkZWZpbmU6IGJ1bmRsZURlZmluZSxcbiAgICAgICAgcmVxdWlyZTogYnVuZGxlUmVxdWlyZSxcbiAgICAgICAgZG9jdW1lbnQ6IGJ1bmRsZURvY3VtZW50LFxuICAgICAgICBib290c3RyYXA6IGJ1bmRsZUJvb3RzdHJhcCxcbiAgICAgICAgcmVnaXN0ZXI6IGJ1bmRsZVJlZ2lzdGVyLFxuICAgICAgICByZW5kZXI6IGJ1bmRsZVJlbmRlcixcbiAgICAgICAgX193ZWV4X2RlZmluZV9fOiBidW5kbGVEZWZpbmUsXG4gICAgICAgIF9fd2VleF9ib290c3RyYXBfXzogYnVuZGxlQm9vdHN0cmFwLFxuICAgICAgICBfX3dlZXhfZG9jdW1lbnRfXzogYnVuZGxlRG9jdW1lbnQsXG4gICAgICAgIF9fd2VleF9yZXF1aXJlX186IGJ1bmRsZVJlcXVpcmVNb2R1bGUsXG4gICAgICAgIHNldFRpbWVvdXQ6IHRpbWVyQVBJcy5zZXRUaW1lb3V0LFxuICAgICAgICBzZXRJbnRlcnZhbDogdGltZXJBUElzLnNldEludGVydmFsLFxuICAgICAgICBjbGVhclRpbWVvdXQ6IHRpbWVyQVBJcy5jbGVhclRpbWVvdXQsXG4gICAgICAgIGNsZWFySW50ZXJ2YWw6IHRpbWVyQVBJcy5jbGVhckludGVydmFsXG4gICAgICB9XG4gICAgfVxuXG4gICAgY29uc3QgZm4gPSBuZXcgRnVuY3Rpb24oXG4gICAgICAnZGVmaW5lJyxcbiAgICAgICdyZXF1aXJlJyxcbiAgICAgICdkb2N1bWVudCcsXG4gICAgICAnYm9vdHN0cmFwJyxcbiAgICAgICdyZWdpc3RlcicsXG4gICAgICAncmVuZGVyJyxcbiAgICAgICdfX3dlZXhfZGVmaW5lX18nLCAvLyBhbGlhcyBmb3IgZGVmaW5lXG4gICAgICAnX193ZWV4X2Jvb3RzdHJhcF9fJywgLy8gYWxpYXMgZm9yIGJvb3RzdHJhcFxuICAgICAgJ19fd2VleF9kb2N1bWVudF9fJywgLy8gYWxpYXMgZm9yIGJvb3RzdHJhcFxuICAgICAgJ19fd2VleF9yZXF1aXJlX18nLFxuICAgICAgJ3NldFRpbWVvdXQnLFxuICAgICAgJ3NldEludGVydmFsJyxcbiAgICAgICdjbGVhclRpbWVvdXQnLFxuICAgICAgJ2NsZWFySW50ZXJ2YWwnLFxuICAgICAgZnVuY3Rpb25Cb2R5XG4gICAgKVxuXG4gICAgZm4oXG4gICAgICBidW5kbGVEZWZpbmUsXG4gICAgICBidW5kbGVSZXF1aXJlLFxuICAgICAgYnVuZGxlRG9jdW1lbnQsXG4gICAgICBidW5kbGVCb290c3RyYXAsXG4gICAgICBidW5kbGVSZWdpc3RlcixcbiAgICAgIGJ1bmRsZVJlbmRlcixcbiAgICAgIGJ1bmRsZURlZmluZSxcbiAgICAgIGJ1bmRsZUJvb3RzdHJhcCxcbiAgICAgIGJ1bmRsZURvY3VtZW50LFxuICAgICAgYnVuZGxlUmVxdWlyZU1vZHVsZSxcbiAgICAgIHRpbWVyQVBJcy5zZXRUaW1lb3V0LFxuICAgICAgdGltZXJBUElzLnNldEludGVydmFsLFxuICAgICAgdGltZXJBUElzLmNsZWFyVGltZW91dCxcbiAgICAgIHRpbWVyQVBJcy5jbGVhckludGVydmFsKVxuICB9XG4gIGVsc2Uge1xuICAgIGNvbnN0IGZuID0gbmV3IEZ1bmN0aW9uKFxuICAgICAgJ2RlZmluZScsXG4gICAgICAncmVxdWlyZScsXG4gICAgICAnZG9jdW1lbnQnLFxuICAgICAgJ2Jvb3RzdHJhcCcsXG4gICAgICAncmVnaXN0ZXInLFxuICAgICAgJ3JlbmRlcicsXG4gICAgICAnX193ZWV4X2RlZmluZV9fJywgLy8gYWxpYXMgZm9yIGRlZmluZVxuICAgICAgJ19fd2VleF9ib290c3RyYXBfXycsIC8vIGFsaWFzIGZvciBib290c3RyYXBcbiAgICAgICdfX3dlZXhfZG9jdW1lbnRfXycsIC8vIGFsaWFzIGZvciBib290c3RyYXBcbiAgICAgICdfX3dlZXhfcmVxdWlyZV9fJyxcbiAgICAgIGZ1bmN0aW9uQm9keVxuICAgIClcblxuICAgIGZuKFxuICAgICAgYnVuZGxlRGVmaW5lLFxuICAgICAgYnVuZGxlUmVxdWlyZSxcbiAgICAgIGJ1bmRsZURvY3VtZW50LFxuICAgICAgYnVuZGxlQm9vdHN0cmFwLFxuICAgICAgYnVuZGxlUmVnaXN0ZXIsXG4gICAgICBidW5kbGVSZW5kZXIsXG4gICAgICBidW5kbGVEZWZpbmUsXG4gICAgICBidW5kbGVCb290c3RyYXAsXG4gICAgICBidW5kbGVEb2N1bWVudCxcbiAgICAgIGJ1bmRsZVJlcXVpcmVNb2R1bGUpXG4gIH1cblxuICByZXR1cm4gcmVzdWx0XG59XG5cbi8qKlxuICogUmVmcmVzaCBhbiBhcHAgd2l0aCBkYXRhIHRvIGl0cyByb290IGNvbXBvbmVudCBvcHRpb25zLlxuICogQHBhcmFtICB7b2JqZWN0fSBhcHBcbiAqIEBwYXJhbSAge2FueX0gICAgZGF0YVxuICovXG5leHBvcnQgZnVuY3Rpb24gcmVmcmVzaCAoYXBwLCBkYXRhKSB7XG4gIGNvbnNvbGUuZGVidWcoYFtKUyBGcmFtZXdvcmtdIFJlZnJlc2ggd2l0aGAsIGRhdGEsXG4gICAgICAgICAgICBgaW4gaW5zdGFuY2VbJHthcHAuaWR9XWApXG4gIGNvbnN0IHZtID0gYXBwLnZtXG4gIGlmICh2bSAmJiBkYXRhKSB7XG4gICAgYXBwLmRvYy5jbG9zZSgpXG4gICAgaWYgKHR5cGVvZiB2bS5yZWZyZXNoRGF0YSA9PT0gJ2Z1bmN0aW9uJykge1xuICAgICAgdm0ucmVmcmVzaERhdGEoZGF0YSlcbiAgICB9XG4gICAgZWxzZSB7XG4gICAgICBleHRlbmQodm0sIGRhdGEpXG4gICAgfVxuICAgIHVwZGF0ZUFjdGlvbnMoYXBwKVxuICAgIGFwcC5kb2MubGlzdGVuZXIucmVmcmVzaEZpbmlzaCgpXG4gICAgYXBwLmRvYy5vcGVuKClcbiAgICByZXR1cm5cbiAgfVxuICByZXR1cm4gbmV3IEVycm9yKGBpbnZhbGlkIGRhdGEgXCIke2RhdGF9XCJgKVxufVxuXG4vKipcbiAqIERlc3Ryb3kgYW4gYXBwLlxuICogQHBhcmFtICB7b2JqZWN0fSBhcHBcbiAqL1xuZXhwb3J0IGZ1bmN0aW9uIGRlc3Ryb3kgKGFwcCkge1xuICBjb25zb2xlLmRlYnVnKGBbSlMgRnJhbWV3b3JrXSBEZXN0b3J5IGFuIGluc3RhbmNlKCR7YXBwLmlkfSlgKVxuXG4gIGFwcC5pZCA9ICcnXG4gIGFwcC5vcHRpb25zID0gbnVsbFxuICBhcHAuYmxvY2tzID0gbnVsbFxuICBhcHAudm0gPSBudWxsXG4gIGFwcC5kb2MgPSBudWxsXG4gIGFwcC5jdXN0b21Db21wb25lbnRNYXAgPSBudWxsXG4gIGFwcC5jYWxsYmFja3MgPSBudWxsXG59XG5cbi8qKlxuICogR2V0IGEgSlNPTiBvYmplY3QgdG8gZGVzY3JpYmUgdGhlIGRvY3VtZW50IGJvZHkuXG4gKiBAcGFyYW0gIHtvYmplY3R9IGFwcFxuICogQHJldHVybiB7b2JqZWN0fVxuICovXG5leHBvcnQgZnVuY3Rpb24gZ2V0Um9vdEVsZW1lbnQgKGFwcCkge1xuICBjb25zdCBkb2MgPSBhcHAuZG9jIHx8IHt9XG4gIGNvbnN0IGJvZHkgPSBkb2MuYm9keSB8fCB7fVxuICByZXR1cm4gYm9keS50b0pTT04gPyBib2R5LnRvSlNPTigpIDoge31cbn1cblxuLyoqXG4gKiBGaXJlIGFuIGV2ZW50IGZyb20gcmVuZGVyZXIuIFRoZSBldmVudCBoYXMgdHlwZSwgYW4gZXZlbnQgb2JqZWN0IGFuZCBhblxuICogZWxlbWVudCByZWYuIElmIHRoZSBldmVudCBjb21lcyB3aXRoIHNvbWUgdmlydHVhbC1ET00gY2hhbmdlcywgaXQgc2hvdWxkXG4gKiBoYXZlIG9uZSBtb3JlIHBhcmFtZXRlciB0byBkZXNjcmliZSB0aGUgY2hhbmdlcy5cbiAqIEBwYXJhbSAge29iamVjdH0gYXBwXG4gKiBAcGFyYW0gIHtzdHJpbmd9IHJlZlxuICogQHBhcmFtICB7dHlwZX0gICB0eXBlXG4gKiBAcGFyYW0gIHtvYmplY3R9IGVcbiAqIEBwYXJhbSAge29iamVjdH0gZG9tQ2hhbmdlc1xuICovXG5leHBvcnQgZnVuY3Rpb24gZmlyZUV2ZW50IChhcHAsIHJlZiwgdHlwZSwgZSwgZG9tQ2hhbmdlcykge1xuICBjb25zb2xlLmRlYnVnKGBbSlMgRnJhbWV3b3JrXSBGaXJlIGEgXCIke3R5cGV9XCIgZXZlbnQgb24gYW4gZWxlbWVudCgke3JlZn0pIGluIGluc3RhbmNlKCR7YXBwLmlkfSlgKVxuICBpZiAoQXJyYXkuaXNBcnJheShyZWYpKSB7XG4gICAgcmVmLnNvbWUoKHJlZikgPT4ge1xuICAgICAgcmV0dXJuIGFwcC5maXJlRXZlbnQocmVmLCB0eXBlLCBlKSAhPT0gZmFsc2VcbiAgICB9KVxuICAgIHJldHVyblxuICB9XG4gIGNvbnN0IGVsID0gYXBwLmRvYy5nZXRSZWYocmVmKVxuICBpZiAoZWwpIHtcbiAgICBhcHAuZG9jLmNsb3NlKClcbiAgICBjb25zdCByZXN1bHQgPSBhcHAuZG9jLmZpcmVFdmVudChlbCwgdHlwZSwgZSwgZG9tQ2hhbmdlcylcbiAgICB1cGRhdGVBY3Rpb25zKGFwcClcbiAgICBhcHAuZG9jLmxpc3RlbmVyLnVwZGF0ZUZpbmlzaCgpXG4gICAgYXBwLmRvYy5vcGVuKClcbiAgICByZXR1cm4gcmVzdWx0XG4gIH1cbiAgcmV0dXJuIG5ldyBFcnJvcihgaW52YWxpZCBlbGVtZW50IHJlZmVyZW5jZSBcIiR7cmVmfVwiYClcbn1cblxuLyoqXG4gKiBNYWtlIGEgY2FsbGJhY2sgZm9yIGEgY2VydGFpbiBhcHAuXG4gKiBAcGFyYW0gIHtvYmplY3R9ICAgYXBwXG4gKiBAcGFyYW0gIHtudW1iZXJ9ICAgY2FsbGJhY2tJZFxuICogQHBhcmFtICB7YW55fSAgICAgIGRhdGFcbiAqIEBwYXJhbSAge2Jvb2xlYW59ICBpZktlZXBBbGl2ZVxuICovXG5leHBvcnQgZnVuY3Rpb24gY2FsbGJhY2sgKGFwcCwgY2FsbGJhY2tJZCwgZGF0YSwgaWZLZWVwQWxpdmUpIHtcbiAgY29uc29sZS5kZWJ1ZyhgW0pTIEZyYW1ld29ya10gSW52b2tlIGEgY2FsbGJhY2soJHtjYWxsYmFja0lkfSkgd2l0aGAsIGRhdGEsXG4gICAgICAgICAgICBgaW4gaW5zdGFuY2UoJHthcHAuaWR9KWApXG4gIGNvbnN0IGNhbGxiYWNrID0gYXBwLmNhbGxiYWNrc1tjYWxsYmFja0lkXVxuICBpZiAodHlwZW9mIGNhbGxiYWNrID09PSAnZnVuY3Rpb24nKSB7XG4gICAgYXBwLmRvYy5jbG9zZSgpXG4gICAgY2FsbGJhY2soZGF0YSlcbiAgICBpZiAodHlwZW9mIGlmS2VlcEFsaXZlID09PSAndW5kZWZpbmVkJyB8fCBpZktlZXBBbGl2ZSA9PT0gZmFsc2UpIHtcbiAgICAgIGFwcC5jYWxsYmFja3NbY2FsbGJhY2tJZF0gPSB1bmRlZmluZWRcbiAgICB9XG4gICAgdXBkYXRlQWN0aW9ucyhhcHApXG4gICAgYXBwLmRvYy5saXN0ZW5lci51cGRhdGVGaW5pc2goKVxuICAgIGFwcC5kb2Mub3BlbigpXG4gICAgcmV0dXJuXG4gIH1cbiAgcmV0dXJuIG5ldyBFcnJvcihgaW52YWxpZCBjYWxsYmFjayBpZCBcIiR7Y2FsbGJhY2tJZH1cImApXG59XG5cbi8qKlxuICogQ29sbGVjdCBhbGwgdmlydHVhbC1ET00gbXV0YXRpb25zIHRvZ2V0aGVyIGFuZCBzZW5kIHRoZW0gdG8gcmVuZGVyZXIuXG4gKiBAcGFyYW0gIHtvYmplY3R9IGFwcFxuICovXG5leHBvcnQgZnVuY3Rpb24gdXBkYXRlQWN0aW9ucyAoYXBwKSB7XG4gIGFwcC5kaWZmZXIuZmx1c2goKVxuICBjb25zdCB0YXNrcyA9IFtdXG4gIGlmIChhcHAuZG9jICYmIGFwcC5kb2MubGlzdGVuZXIgJiYgYXBwLmRvYy5saXN0ZW5lci51cGRhdGVzLmxlbmd0aCkge1xuICAgIHRhc2tzLnB1c2goLi4uYXBwLmRvYy5saXN0ZW5lci51cGRhdGVzKVxuICAgIGFwcC5kb2MubGlzdGVuZXIudXBkYXRlcyA9IFtdXG4gIH1cbiAgaWYgKHRhc2tzLmxlbmd0aCkge1xuICAgIHJldHVybiBjYWxsVGFza3MoYXBwLCB0YXNrcylcbiAgfVxufVxuXG4vKipcbiAqIENhbGwgYWxsIHRhc2tzIGZyb20gYW4gYXBwIHRvIHJlbmRlcmVyIChuYXRpdmUpLlxuICogQHBhcmFtICB7b2JqZWN0fSBhcHBcbiAqIEBwYXJhbSAge2FycmF5fSAgdGFza3NcbiAqL1xuZXhwb3J0IGZ1bmN0aW9uIGNhbGxUYXNrcyAoYXBwLCB0YXNrcykge1xuICBpZiAodHlwb2YodGFza3MpICE9PSAnYXJyYXknKSB7XG4gICAgdGFza3MgPSBbdGFza3NdXG4gIH1cblxuICB0YXNrcy5mb3JFYWNoKCh0YXNrKSA9PiB7XG4gICAgdGFzay5hcmdzID0gdGFzay5hcmdzLm1hcChhcmcgPT4gbm9ybWFsaXplKGFyZywgYXBwKSlcbiAgfSlcblxuICByZXR1cm4gcmVuZGVyZXIuc2VuZFRhc2tzKGFwcC5pZCwgdGFza3MsICctMScpXG59XG5cbi8qKlxuICogTm9ybWFsaXplIGEgdmFsdWUuIFNwZWNpYWxseSwgaWYgdGhlIHZhbHVlIGlzIGEgZnVuY3Rpb24sIHRoZW4gZ2VuZXJhdGUgYSBmdW5jdGlvbiBpZFxuICogYW5kIHNhdmUgaXQgdG8gYGFwcC5jYWxsYmFja3NgLCBhdCBsYXN0IHJldHVybiB0aGUgZnVuY3Rpb24gaWQuXG4gKiBAcGFyYW0gIHthbnl9ICAgICAgICB2XG4gKiBAcGFyYW0gIHtvYmplY3R9ICAgICBhcHBcbiAqIEByZXR1cm4ge3ByaW1pdGl2ZX1cbiAqL1xuZnVuY3Rpb24gbm9ybWFsaXplICh2LCBhcHApIHtcbiAgY29uc3QgdHlwZSA9IHR5cG9mKHYpXG5cbiAgc3dpdGNoICh0eXBlKSB7XG4gICAgY2FzZSAndW5kZWZpbmVkJzpcbiAgICBjYXNlICdudWxsJzpcbiAgICAgIHJldHVybiAnJ1xuICAgIGNhc2UgJ3JlZ2V4cCc6XG4gICAgICByZXR1cm4gdi50b1N0cmluZygpXG4gICAgY2FzZSAnZGF0ZSc6XG4gICAgICByZXR1cm4gdi50b0lTT1N0cmluZygpXG4gICAgY2FzZSAnbnVtYmVyJzpcbiAgICBjYXNlICdzdHJpbmcnOlxuICAgIGNhc2UgJ2Jvb2xlYW4nOlxuICAgIGNhc2UgJ2FycmF5JzpcbiAgICBjYXNlICdvYmplY3QnOlxuICAgICAgaWYgKHYgaW5zdGFuY2VvZiByZW5kZXJlci5FbGVtZW50KSB7XG4gICAgICAgIHJldHVybiB2LnJlZlxuICAgICAgfVxuICAgICAgcmV0dXJuIHZcbiAgICBjYXNlICdmdW5jdGlvbic6XG4gICAgICBhcHAuY2FsbGJhY2tzWysrYXBwLnVpZF0gPSB2XG4gICAgICByZXR1cm4gYXBwLnVpZC50b1N0cmluZygpXG4gICAgZGVmYXVsdDpcbiAgICAgIHJldHVybiBKU09OLnN0cmluZ2lmeSh2KVxuICB9XG59XG5cblxuXG4vKiogV0VCUEFDSyBGT09URVIgKipcbiAqKiAvVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvc3JjL2RlZmF1bHQvYXBwL2N0cmwuanNcbiAqKi8iLCJleHBvcnQge1xuICBleHRlbmQsXG4gIGRlZixcbiAgcmVtb3ZlLFxuICBoYXNPd24sXG4gIGJpbmQsXG4gIHRvQXJyYXksXG4gIGlzT2JqZWN0LFxuICBpc1BsYWluT2JqZWN0XG59IGZyb20gJy4uLy4uL3NoYXJlZCdcblxuLyoqXG4gKiBDaGVjayBpZiBhIHN0cmluZyBzdGFydHMgd2l0aCAkIG9yIF9cbiAqXG4gKiBAcGFyYW0ge1N0cmluZ30gc3RyXG4gKiBAcmV0dXJuIHtCb29sZWFufVxuICovXG5cbmV4cG9ydCBmdW5jdGlvbiBpc1Jlc2VydmVkIChzdHIpIHtcbiAgY29uc3QgYyA9IChzdHIgKyAnJykuY2hhckNvZGVBdCgwKVxuICByZXR1cm4gYyA9PT0gMHgyNCB8fCBjID09PSAweDVGXG59XG5cbi8vIGNhbiB3ZSB1c2UgX19wcm90b19fP1xuZXhwb3J0IGNvbnN0IGhhc1Byb3RvID0gJ19fcHJvdG9fXycgaW4ge31cblxubGV0IF9TZXRcbi8qIGlzdGFuYnVsIGlnbm9yZSBpZiAqL1xuaWYgKHR5cGVvZiBTZXQgIT09ICd1bmRlZmluZWQnICYmIFNldC50b1N0cmluZygpLm1hdGNoKC9uYXRpdmUgY29kZS8pKSB7XG4gIC8vIHVzZSBuYXRpdmUgU2V0IHdoZW4gYXZhaWxhYmxlLlxuICBfU2V0ID0gU2V0XG59XG5lbHNlIHtcbiAgLy8gYSBub24tc3RhbmRhcmQgU2V0IHBvbHlmaWxsIHRoYXQgb25seSB3b3JrcyB3aXRoIHByaW1pdGl2ZSBrZXlzLlxuICBfU2V0ID0gZnVuY3Rpb24gKCkge1xuICAgIHRoaXMuc2V0ID0gT2JqZWN0LmNyZWF0ZShudWxsKVxuICB9XG4gIF9TZXQucHJvdG90eXBlLmhhcyA9IGZ1bmN0aW9uIChrZXkpIHtcbiAgICByZXR1cm4gdGhpcy5zZXRba2V5XSAhPT0gdW5kZWZpbmVkXG4gIH1cbiAgX1NldC5wcm90b3R5cGUuYWRkID0gZnVuY3Rpb24gKGtleSkge1xuICAgIHRoaXMuc2V0W2tleV0gPSAxXG4gIH1cbiAgX1NldC5wcm90b3R5cGUuY2xlYXIgPSBmdW5jdGlvbiAoKSB7XG4gICAgdGhpcy5zZXQgPSBPYmplY3QuY3JlYXRlKG51bGwpXG4gIH1cbn1cblxuZXhwb3J0IHsgX1NldCB9XG5cbi8qKlxuICogQ3JlYXRlIGEgY2FjaGVkIHZlcnNpb24gb2YgYSBwdXJlIGZ1bmN0aW9uLlxuICpcbiAqIEBwYXJhbSB7RnVuY3Rpb259IGZuXG4gKiBAcmV0dXJuIHtGdW5jdGlvbn1cbiAqL1xuXG5leHBvcnQgZnVuY3Rpb24gY2FjaGVkIChmbikge1xuICBjb25zdCBjYWNoZSA9IE9iamVjdC5jcmVhdGUobnVsbClcbiAgcmV0dXJuIGZ1bmN0aW9uIGNhY2hlZEZuIChzdHIpIHtcbiAgICBjb25zdCBoaXQgPSBjYWNoZVtzdHJdXG4gICAgcmV0dXJuIGhpdCB8fCAoY2FjaGVbc3RyXSA9IGZuKHN0cikpXG4gIH1cbn1cblxuLyoqXG4gKiBDYW1lbGl6ZSBhIGh5cGhlbi1kZWxtaXRlZCBzdHJpbmcuXG4gKlxuICogQHBhcmFtIHtTdHJpbmd9IHN0clxuICogQHJldHVybiB7U3RyaW5nfVxuICovXG5cbmNvbnN0IGNhbWVsaXplUkUgPSAvLShcXHcpL2dcbmV4cG9ydCBjb25zdCBjYW1lbGl6ZSA9IGNhY2hlZChzdHIgPT4ge1xuICByZXR1cm4gc3RyLnJlcGxhY2UoY2FtZWxpemVSRSwgdG9VcHBlcilcbn0pXG5cbmZ1bmN0aW9uIHRvVXBwZXIgKF8sIGMpIHtcbiAgcmV0dXJuIGMgPyBjLnRvVXBwZXJDYXNlKCkgOiAnJ1xufVxuXG4vKipcbiAqIEh5cGhlbmF0ZSBhIGNhbWVsQ2FzZSBzdHJpbmcuXG4gKlxuICogQHBhcmFtIHtTdHJpbmd9IHN0clxuICogQHJldHVybiB7U3RyaW5nfVxuICovXG5cbmNvbnN0IGh5cGhlbmF0ZVJFID0gLyhbYS16XFxkXSkoW0EtWl0pL2dcbmV4cG9ydCBjb25zdCBoeXBoZW5hdGUgPSBjYWNoZWQoc3RyID0+IHtcbiAgcmV0dXJuIHN0clxuICAgIC5yZXBsYWNlKGh5cGhlbmF0ZVJFLCAnJDEtJDInKVxuICAgIC50b0xvd2VyQ2FzZSgpXG59KVxuXG5leHBvcnQgZnVuY3Rpb24gdHlwb2YgKHYpIHtcbiAgY29uc3QgcyA9IE9iamVjdC5wcm90b3R5cGUudG9TdHJpbmcuY2FsbCh2KVxuICByZXR1cm4gcy5zdWJzdHJpbmcoOCwgcy5sZW5ndGggLSAxKS50b0xvd2VyQ2FzZSgpXG59XG5cbi8vIHdlZXggbmFtZSBydWxlc1xuXG5jb25zdCBXRUVYX0NPTVBPTkVOVF9SRUcgPSAvXkB3ZWV4LWNvbXBvbmVudFxcLy9cbmNvbnN0IFdFRVhfTU9EVUxFX1JFRyA9IC9eQHdlZXgtbW9kdWxlXFwvL1xuY29uc3QgTk9STUFMX01PRFVMRV9SRUcgPSAvXlxcLnsxLDJ9XFwvL1xuY29uc3QgSlNfU1VSRklYX1JFRyA9IC9cXC5qcyQvXG5cbmV4cG9ydCBjb25zdCBpc1dlZXhDb21wb25lbnQgPSBuYW1lID0+ICEhbmFtZS5tYXRjaChXRUVYX0NPTVBPTkVOVF9SRUcpXG5leHBvcnQgY29uc3QgaXNXZWV4TW9kdWxlID0gbmFtZSA9PiAhIW5hbWUubWF0Y2goV0VFWF9NT0RVTEVfUkVHKVxuZXhwb3J0IGNvbnN0IGlzTm9ybWFsTW9kdWxlID0gbmFtZSA9PiAhIW5hbWUubWF0Y2goTk9STUFMX01PRFVMRV9SRUcpXG5leHBvcnQgY29uc3QgaXNOcG1Nb2R1bGUgPSBuYW1lID0+ICFpc1dlZXhDb21wb25lbnQobmFtZSkgJiYgIWlzV2VleE1vZHVsZShuYW1lKSAmJiAhaXNOb3JtYWxNb2R1bGUobmFtZSlcblxuZXhwb3J0IGZ1bmN0aW9uIHJlbW92ZVdlZXhQcmVmaXggKHN0cikge1xuICBjb25zdCByZXN1bHQgPSBzdHIucmVwbGFjZShXRUVYX0NPTVBPTkVOVF9SRUcsICcnKS5yZXBsYWNlKFdFRVhfTU9EVUxFX1JFRywgJycpXG4gIHJldHVybiByZXN1bHRcbn1cblxuZXhwb3J0IGZ1bmN0aW9uIHJlbW92ZUpTU3VyZml4IChzdHIpIHtcbiAgcmV0dXJuIHN0ci5yZXBsYWNlKEpTX1NVUkZJWF9SRUcsICcnKVxufVxuXG5cblxuLyoqIFdFQlBBQ0sgRk9PVEVSICoqXG4gKiogL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL3NyYy9kZWZhdWx0L3V0aWwvaW5kZXguanNcbiAqKi8iLCIvKipcbiAqIEBmaWxlT3ZlcnZpZXdcbiAqIGFwaSB0aGF0IGludm9rZWQgYnkganMgYnVuZGxlIGNvZGVcbiAqXG4gKiAtIGRlZmluZShuYW1lLCBmYWN0b3J5KTogZGVmaW5lIGEgbmV3IGNvbXBvc2VkIGNvbXBvbmVudCB0eXBlXG4gKiAtIGJvb3RzdHJhcCh0eXBlLCBjb25maWcsIGRhdGEpOiByZXF1aXJlIGEgY2VydGFpbiB0eXBlICZcbiAqICAgICAgICAgcmVuZGVyIHdpdGggKG9wdGlvbmFsKSBkYXRhXG4gKlxuICogZGVwcmVjYXRlZDpcbiAqIC0gcmVnaXN0ZXIodHlwZSwgb3B0aW9ucyk6IHJlZ2lzdGVyIGEgbmV3IGNvbXBvc2VkIGNvbXBvbmVudCB0eXBlXG4gKiAtIHJlbmRlcih0eXBlLCBkYXRhKTogcmVuZGVyIGJ5IGEgY2VydGFpbiB0eXBlIHdpdGggKG9wdGlvbmFsKSBkYXRhXG4gKiAtIHJlcXVpcmUodHlwZSkoZGF0YSk6IHJlcXVpcmUgYSB0eXBlIHRoZW4gcmVuZGVyIHdpdGggZGF0YVxuICovXG5cbmltcG9ydCBzZW12ZXIgZnJvbSAnc2VtdmVyJ1xuaW1wb3J0IHtcbiAgaXNQbGFpbk9iamVjdCxcbiAgaXNXZWV4Q29tcG9uZW50LFxuICBpc1dlZXhNb2R1bGUsXG4gIGlzTm9ybWFsTW9kdWxlLFxuICBpc05wbU1vZHVsZSxcbiAgcmVtb3ZlV2VleFByZWZpeCxcbiAgcmVtb3ZlSlNTdXJmaXhcbn0gZnJvbSAnLi4vdXRpbCdcbmltcG9ydCBWbSBmcm9tICcuLi92bSdcbmltcG9ydCB7XG4gIHJlZ2lzdGVyQ3VzdG9tQ29tcG9uZW50LFxuICByZXF1aXJlQ3VzdG9tQ29tcG9uZW50LFxuICBpbml0TW9kdWxlc1xufSBmcm9tICcuL3JlZ2lzdGVyJ1xuaW1wb3J0ICogYXMgZG93bmdyYWRlIGZyb20gJy4vZG93bmdyYWRlJ1xuXG4vKipcbiAqIEBkZXByZWNhdGVkXG4gKlxuICogY29tbW9uIG1vZHVsZXMgYXJlIHNoYXJlZCB0byBhbGwgaW5zdGFuY2VzXG4gKiBpdCdzIHZlcnkgZGFuZ2Vyb3VzXG4gKi9cbmxldCBjb21tb25Nb2R1bGVzID0ge31cblxuLyoqXG4gKiBAZGVwcmVjYXRlZFxuICovXG5leHBvcnQgZnVuY3Rpb24gY2xlYXJDb21tb25Nb2R1bGVzICgpIHtcbiAgY29tbW9uTW9kdWxlcyA9IHt9XG59XG5cbi8qKlxuICogZGVmaW5lKG5hbWUsIGZhY3RvcnkpIGZvciBwcmltYXJ5IHVzYWdlXG4gKiBvclxuICogZGVmaW5lKG5hbWUsIGRlcHMsIGZhY3RvcnkpIGZvciBjb21wYXRpYmlsaXR5XG4gKiBOb3RpY2U6IERPIE5PVCB1c2UgZnVuY3Rpb24gZGVmaW5lKCkge30sXG4gKiBpdCB3aWxsIGNhdXNlIGVycm9yIGFmdGVyIGJ1aWxkZWQgYnkgd2VicGFja1xuICovXG5leHBvcnQgZnVuY3Rpb24gZGVmaW5lRm4gKGFwcCwgbmFtZSwgLi4uYXJncykge1xuICBjb25zb2xlLmRlYnVnKGBbSlMgRnJhbWV3b3JrXSBkZWZpbmUgYSBjb21wb25lbnQgJHtuYW1lfWApXG5cbiAgLy8gYWRhcHQgYXJnczpcbiAgLy8gMS4gbmFtZSwgZGVwc1tdLCBmYWN0b3J5KClcbiAgLy8gMi4gbmFtZSwgZmFjdG9yeSgpXG4gIC8vIDMuIG5hbWUsIGRlZmluaXRpb257fVxuICBsZXQgZmFjdG9yeSwgZGVmaW5pdGlvblxuICBpZiAoYXJncy5sZW5ndGggPiAxKSB7XG4gICAgZGVmaW5pdGlvbiA9IGFyZ3NbMV1cbiAgfVxuICBlbHNlIHtcbiAgICBkZWZpbml0aW9uID0gYXJnc1swXVxuICB9XG4gIGlmICh0eXBlb2YgZGVmaW5pdGlvbiA9PT0gJ2Z1bmN0aW9uJykge1xuICAgIGZhY3RvcnkgPSBkZWZpbml0aW9uXG4gICAgZGVmaW5pdGlvbiA9IG51bGxcbiAgfVxuXG4gIC8vIHJlc29sdmUgZGVmaW5pdGlvbiBmcm9tIGZhY3RvcnlcbiAgaWYgKGZhY3RvcnkpIHtcbiAgICBjb25zdCByID0gKG5hbWUpID0+IHtcbiAgICAgIGlmIChpc1dlZXhDb21wb25lbnQobmFtZSkpIHtcbiAgICAgICAgY29uc3QgY2xlYW5OYW1lID0gcmVtb3ZlV2VleFByZWZpeChuYW1lKVxuICAgICAgICByZXR1cm4gcmVxdWlyZUN1c3RvbUNvbXBvbmVudChhcHAsIGNsZWFuTmFtZSlcbiAgICAgIH1cbiAgICAgIGlmIChpc1dlZXhNb2R1bGUobmFtZSkpIHtcbiAgICAgICAgY29uc3QgY2xlYW5OYW1lID0gcmVtb3ZlV2VleFByZWZpeChuYW1lKVxuICAgICAgICByZXR1cm4gYXBwLnJlcXVpcmVNb2R1bGUoY2xlYW5OYW1lKVxuICAgICAgfVxuICAgICAgaWYgKGlzTm9ybWFsTW9kdWxlKG5hbWUpIHx8IGlzTnBtTW9kdWxlKG5hbWUpKSB7XG4gICAgICAgIGNvbnN0IGNsZWFuTmFtZSA9IHJlbW92ZUpTU3VyZml4KG5hbWUpXG4gICAgICAgIHJldHVybiBjb21tb25Nb2R1bGVzW2NsZWFuTmFtZV1cbiAgICAgIH1cbiAgICB9XG4gICAgY29uc3QgbSA9IHsgZXhwb3J0czoge319XG4gICAgZmFjdG9yeShyLCBtLmV4cG9ydHMsIG0pXG4gICAgZGVmaW5pdGlvbiA9IG0uZXhwb3J0c1xuICB9XG5cbiAgLy8gYXBwbHkgZGVmaW5pdGlvblxuICBpZiAoaXNXZWV4Q29tcG9uZW50KG5hbWUpKSB7XG4gICAgY29uc3QgY2xlYW5OYW1lID0gcmVtb3ZlV2VleFByZWZpeChuYW1lKVxuICAgIHJlZ2lzdGVyQ3VzdG9tQ29tcG9uZW50KGFwcCwgY2xlYW5OYW1lLCBkZWZpbml0aW9uKVxuICB9XG4gIGVsc2UgaWYgKGlzV2VleE1vZHVsZShuYW1lKSkge1xuICAgIGNvbnN0IGNsZWFuTmFtZSA9IHJlbW92ZVdlZXhQcmVmaXgobmFtZSlcbiAgICBpbml0TW9kdWxlcyh7IFtjbGVhbk5hbWVdOiBkZWZpbml0aW9uIH0pXG4gIH1cbiAgZWxzZSBpZiAoaXNOb3JtYWxNb2R1bGUobmFtZSkpIHtcbiAgICBjb25zdCBjbGVhbk5hbWUgPSByZW1vdmVKU1N1cmZpeChuYW1lKVxuICAgIGNvbW1vbk1vZHVsZXNbY2xlYW5OYW1lXSA9IGRlZmluaXRpb25cbiAgfVxuICBlbHNlIGlmIChpc05wbU1vZHVsZShuYW1lKSkge1xuICAgIGNvbnN0IGNsZWFuTmFtZSA9IHJlbW92ZUpTU3VyZml4KG5hbWUpXG4gICAgaWYgKGRlZmluaXRpb24udGVtcGxhdGUgfHxcbiAgICAgICAgZGVmaW5pdGlvbi5zdHlsZSB8fFxuICAgICAgICBkZWZpbml0aW9uLm1ldGhvZHMpIHtcbiAgICAgIC8vIGRvd25ncmFkZSB0byBvbGQgZGVmaW5lIG1ldGhvZCAoZGVmaW5lKCdjb21wb25lbnROYW1lJywgZmFjdG9yeSkpXG4gICAgICAvLyB0aGUgZXhwb3J0cyBjb250YWluIG9uZSBrZXkgb2YgdGVtcGxhdGUsIHN0eWxlIG9yIG1ldGhvZHNcbiAgICAgIC8vIGJ1dCBpdCBoYXMgcmlzayEhIVxuICAgICAgcmVnaXN0ZXJDdXN0b21Db21wb25lbnQoYXBwLCBjbGVhbk5hbWUsIGRlZmluaXRpb24pXG4gICAgfVxuICAgIGVsc2Uge1xuICAgICAgY29tbW9uTW9kdWxlc1tjbGVhbk5hbWVdID0gZGVmaW5pdGlvblxuICAgIH1cbiAgfVxufVxuXG4vKipcbiAqIGJvb3RzdHJhcCBhcHAgZnJvbSBhIGNlcnRhaW4gY3VzdG9tIGNvbXBvbmVudCB3aXRoIGNvbmZpZyAmIGRhdGFcbiAqL1xuZXhwb3J0IGZ1bmN0aW9uIGJvb3RzdHJhcCAoYXBwLCBuYW1lLCBjb25maWcsIGRhdGEpIHtcbiAgY29uc29sZS5kZWJ1ZyhgW0pTIEZyYW1ld29ya10gYm9vdHN0cmFwIGZvciAke25hbWV9YClcblxuICAvLyAxLiB2YWxpZGF0ZSBjdXN0b20gY29tcG9uZW50IG5hbWUgZmlyc3RcbiAgbGV0IGNsZWFuTmFtZVxuICBpZiAoaXNXZWV4Q29tcG9uZW50KG5hbWUpKSB7XG4gICAgY2xlYW5OYW1lID0gcmVtb3ZlV2VleFByZWZpeChuYW1lKVxuICB9XG4gIGVsc2UgaWYgKGlzTnBtTW9kdWxlKG5hbWUpKSB7XG4gICAgY2xlYW5OYW1lID0gcmVtb3ZlSlNTdXJmaXgobmFtZSlcbiAgICAvLyBjaGVjayBpZiBkZWZpbmUgYnkgb2xkICdkZWZpbmUnIG1ldGhvZFxuICAgIC8qIGlzdGFuYnVsIGlnbm9yZSBpZiAqL1xuICAgIGlmICghcmVxdWlyZUN1c3RvbUNvbXBvbmVudChhcHAsIGNsZWFuTmFtZSkpIHtcbiAgICAgIHJldHVybiBuZXcgRXJyb3IoYEl0J3Mgbm90IGEgY29tcG9uZW50OiAke25hbWV9YClcbiAgICB9XG4gIH1cbiAgZWxzZSB7XG4gICAgcmV0dXJuIG5ldyBFcnJvcihgV3JvbmcgY29tcG9uZW50IG5hbWU6ICR7bmFtZX1gKVxuICB9XG5cbiAgLy8gMi4gdmFsaWRhdGUgY29uZmlndXJhdGlvblxuICBjb25maWcgPSBpc1BsYWluT2JqZWN0KGNvbmZpZykgPyBjb25maWcgOiB7fVxuICAvLyAyLjEgdHJhbnNmb3JtZXIgdmVyc2lvbiBjaGVja1xuICBpZiAodHlwZW9mIGNvbmZpZy50cmFuc2Zvcm1lclZlcnNpb24gPT09ICdzdHJpbmcnICYmXG4gICAgdHlwZW9mIGdsb2JhbC50cmFuc2Zvcm1lclZlcnNpb24gPT09ICdzdHJpbmcnICYmXG4gICAgIXNlbXZlci5zYXRpc2ZpZXMoY29uZmlnLnRyYW5zZm9ybWVyVmVyc2lvbixcbiAgICAgIGdsb2JhbC50cmFuc2Zvcm1lclZlcnNpb24pKSB7XG4gICAgcmV0dXJuIG5ldyBFcnJvcihgSlMgQnVuZGxlIHZlcnNpb246ICR7Y29uZmlnLnRyYW5zZm9ybWVyVmVyc2lvbn0gYCArXG4gICAgICBgbm90IGNvbXBhdGlibGUgd2l0aCAke2dsb2JhbC50cmFuc2Zvcm1lclZlcnNpb259YClcbiAgfVxuICAvLyAyLjIgZG93bmdyYWRlIHZlcnNpb24gY2hlY2tcbiAgY29uc3QgZG93bmdyYWRlUmVzdWx0ID0gZG93bmdyYWRlLmNoZWNrKGNvbmZpZy5kb3duZ3JhZGUpXG4gIC8qIGlzdGFuYnVsIGlnbm9yZSBpZiAqL1xuICBpZiAoZG93bmdyYWRlUmVzdWx0LmlzRG93bmdyYWRlKSB7XG4gICAgYXBwLmNhbGxUYXNrcyhbe1xuICAgICAgbW9kdWxlOiAnaW5zdGFuY2VXcmFwJyxcbiAgICAgIG1ldGhvZDogJ2Vycm9yJyxcbiAgICAgIGFyZ3M6IFtcbiAgICAgICAgZG93bmdyYWRlUmVzdWx0LmVycm9yVHlwZSxcbiAgICAgICAgZG93bmdyYWRlUmVzdWx0LmNvZGUsXG4gICAgICAgIGRvd25ncmFkZVJlc3VsdC5lcnJvck1lc3NhZ2VcbiAgICAgIF1cbiAgICB9XSlcbiAgICByZXR1cm4gbmV3IEVycm9yKGBEb3duZ3JhZGVbJHtkb3duZ3JhZGVSZXN1bHQuY29kZX1dOiAke2Rvd25ncmFkZVJlc3VsdC5lcnJvck1lc3NhZ2V9YClcbiAgfVxuXG4gIC8vIDMuIGNyZWF0ZSBhIG5ldyBWbSB3aXRoIGN1c3RvbSBjb21wb25lbnQgbmFtZSBhbmQgZGF0YVxuICBhcHAudm0gPSBuZXcgVm0oY2xlYW5OYW1lLCBudWxsLCB7IF9hcHA6IGFwcCB9LCBudWxsLCBkYXRhKVxufVxuXG4vKipcbiAqIEBkZXByZWNhdGVkXG4gKi9cbmV4cG9ydCBmdW5jdGlvbiByZWdpc3RlciAoYXBwLCB0eXBlLCBvcHRpb25zKSB7XG4gIGNvbnNvbGUud2FybignW0pTIEZyYW1ld29ya10gUmVnaXN0ZXIgaXMgZGVwcmVjYXRlZCwgcGxlYXNlIGluc3RhbGwgbGFzdGVzdCB0cmFuc2Zvcm1lci4nKVxuICByZWdpc3RlckN1c3RvbUNvbXBvbmVudChhcHAsIHR5cGUsIG9wdGlvbnMpXG59XG5cblxuXG4vKiogV0VCUEFDSyBGT09URVIgKipcbiAqKiAvVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvc3JjL2RlZmF1bHQvYXBwL2J1bmRsZS5qc1xuICoqLyIsImV4cG9ydHMgPSBtb2R1bGUuZXhwb3J0cyA9IFNlbVZlcjtcblxuLy8gVGhlIGRlYnVnIGZ1bmN0aW9uIGlzIGV4Y2x1ZGVkIGVudGlyZWx5IGZyb20gdGhlIG1pbmlmaWVkIHZlcnNpb24uXG4vKiBub21pbiAqLyB2YXIgZGVidWc7XG4vKiBub21pbiAqLyBpZiAodHlwZW9mIHByb2Nlc3MgPT09ICdvYmplY3QnICYmXG4gICAgLyogbm9taW4gKi8gcHJvY2Vzcy5lbnYgJiZcbiAgICAvKiBub21pbiAqLyBwcm9jZXNzLmVudi5OT0RFX0RFQlVHICYmXG4gICAgLyogbm9taW4gKi8gL1xcYnNlbXZlclxcYi9pLnRlc3QocHJvY2Vzcy5lbnYuTk9ERV9ERUJVRykpXG4gIC8qIG5vbWluICovIGRlYnVnID0gZnVuY3Rpb24oKSB7XG4gICAgLyogbm9taW4gKi8gdmFyIGFyZ3MgPSBBcnJheS5wcm90b3R5cGUuc2xpY2UuY2FsbChhcmd1bWVudHMsIDApO1xuICAgIC8qIG5vbWluICovIGFyZ3MudW5zaGlmdCgnU0VNVkVSJyk7XG4gICAgLyogbm9taW4gKi8gY29uc29sZS5sb2cuYXBwbHkoY29uc29sZSwgYXJncyk7XG4gICAgLyogbm9taW4gKi8gfTtcbi8qIG5vbWluICovIGVsc2VcbiAgLyogbm9taW4gKi8gZGVidWcgPSBmdW5jdGlvbigpIHt9O1xuXG4vLyBOb3RlOiB0aGlzIGlzIHRoZSBzZW12ZXIub3JnIHZlcnNpb24gb2YgdGhlIHNwZWMgdGhhdCBpdCBpbXBsZW1lbnRzXG4vLyBOb3QgbmVjZXNzYXJpbHkgdGhlIHBhY2thZ2UgdmVyc2lvbiBvZiB0aGlzIGNvZGUuXG5leHBvcnRzLlNFTVZFUl9TUEVDX1ZFUlNJT04gPSAnMi4wLjAnO1xuXG52YXIgTUFYX0xFTkdUSCA9IDI1NjtcbnZhciBNQVhfU0FGRV9JTlRFR0VSID0gTnVtYmVyLk1BWF9TQUZFX0lOVEVHRVIgfHwgOTAwNzE5OTI1NDc0MDk5MTtcblxuLy8gVGhlIGFjdHVhbCByZWdleHBzIGdvIG9uIGV4cG9ydHMucmVcbnZhciByZSA9IGV4cG9ydHMucmUgPSBbXTtcbnZhciBzcmMgPSBleHBvcnRzLnNyYyA9IFtdO1xudmFyIFIgPSAwO1xuXG4vLyBUaGUgZm9sbG93aW5nIFJlZ3VsYXIgRXhwcmVzc2lvbnMgY2FuIGJlIHVzZWQgZm9yIHRva2VuaXppbmcsXG4vLyB2YWxpZGF0aW5nLCBhbmQgcGFyc2luZyBTZW1WZXIgdmVyc2lvbiBzdHJpbmdzLlxuXG4vLyAjIyBOdW1lcmljIElkZW50aWZpZXJcbi8vIEEgc2luZ2xlIGAwYCwgb3IgYSBub24temVybyBkaWdpdCBmb2xsb3dlZCBieSB6ZXJvIG9yIG1vcmUgZGlnaXRzLlxuXG52YXIgTlVNRVJJQ0lERU5USUZJRVIgPSBSKys7XG5zcmNbTlVNRVJJQ0lERU5USUZJRVJdID0gJzB8WzEtOV1cXFxcZConO1xudmFyIE5VTUVSSUNJREVOVElGSUVSTE9PU0UgPSBSKys7XG5zcmNbTlVNRVJJQ0lERU5USUZJRVJMT09TRV0gPSAnWzAtOV0rJztcblxuXG4vLyAjIyBOb24tbnVtZXJpYyBJZGVudGlmaWVyXG4vLyBaZXJvIG9yIG1vcmUgZGlnaXRzLCBmb2xsb3dlZCBieSBhIGxldHRlciBvciBoeXBoZW4sIGFuZCB0aGVuIHplcm8gb3Jcbi8vIG1vcmUgbGV0dGVycywgZGlnaXRzLCBvciBoeXBoZW5zLlxuXG52YXIgTk9OTlVNRVJJQ0lERU5USUZJRVIgPSBSKys7XG5zcmNbTk9OTlVNRVJJQ0lERU5USUZJRVJdID0gJ1xcXFxkKlthLXpBLVotXVthLXpBLVowLTktXSonO1xuXG5cbi8vICMjIE1haW4gVmVyc2lvblxuLy8gVGhyZWUgZG90LXNlcGFyYXRlZCBudW1lcmljIGlkZW50aWZpZXJzLlxuXG52YXIgTUFJTlZFUlNJT04gPSBSKys7XG5zcmNbTUFJTlZFUlNJT05dID0gJygnICsgc3JjW05VTUVSSUNJREVOVElGSUVSXSArICcpXFxcXC4nICtcbiAgICAgICAgICAgICAgICAgICAnKCcgKyBzcmNbTlVNRVJJQ0lERU5USUZJRVJdICsgJylcXFxcLicgK1xuICAgICAgICAgICAgICAgICAgICcoJyArIHNyY1tOVU1FUklDSURFTlRJRklFUl0gKyAnKSc7XG5cbnZhciBNQUlOVkVSU0lPTkxPT1NFID0gUisrO1xuc3JjW01BSU5WRVJTSU9OTE9PU0VdID0gJygnICsgc3JjW05VTUVSSUNJREVOVElGSUVSTE9PU0VdICsgJylcXFxcLicgK1xuICAgICAgICAgICAgICAgICAgICAgICAgJygnICsgc3JjW05VTUVSSUNJREVOVElGSUVSTE9PU0VdICsgJylcXFxcLicgK1xuICAgICAgICAgICAgICAgICAgICAgICAgJygnICsgc3JjW05VTUVSSUNJREVOVElGSUVSTE9PU0VdICsgJyknO1xuXG4vLyAjIyBQcmUtcmVsZWFzZSBWZXJzaW9uIElkZW50aWZpZXJcbi8vIEEgbnVtZXJpYyBpZGVudGlmaWVyLCBvciBhIG5vbi1udW1lcmljIGlkZW50aWZpZXIuXG5cbnZhciBQUkVSRUxFQVNFSURFTlRJRklFUiA9IFIrKztcbnNyY1tQUkVSRUxFQVNFSURFTlRJRklFUl0gPSAnKD86JyArIHNyY1tOVU1FUklDSURFTlRJRklFUl0gK1xuICAgICAgICAgICAgICAgICAgICAgICAgICAgICd8JyArIHNyY1tOT05OVU1FUklDSURFTlRJRklFUl0gKyAnKSc7XG5cbnZhciBQUkVSRUxFQVNFSURFTlRJRklFUkxPT1NFID0gUisrO1xuc3JjW1BSRVJFTEVBU0VJREVOVElGSUVSTE9PU0VdID0gJyg/OicgKyBzcmNbTlVNRVJJQ0lERU5USUZJRVJMT09TRV0gK1xuICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgJ3wnICsgc3JjW05PTk5VTUVSSUNJREVOVElGSUVSXSArICcpJztcblxuXG4vLyAjIyBQcmUtcmVsZWFzZSBWZXJzaW9uXG4vLyBIeXBoZW4sIGZvbGxvd2VkIGJ5IG9uZSBvciBtb3JlIGRvdC1zZXBhcmF0ZWQgcHJlLXJlbGVhc2UgdmVyc2lvblxuLy8gaWRlbnRpZmllcnMuXG5cbnZhciBQUkVSRUxFQVNFID0gUisrO1xuc3JjW1BSRVJFTEVBU0VdID0gJyg/Oi0oJyArIHNyY1tQUkVSRUxFQVNFSURFTlRJRklFUl0gK1xuICAgICAgICAgICAgICAgICAgJyg/OlxcXFwuJyArIHNyY1tQUkVSRUxFQVNFSURFTlRJRklFUl0gKyAnKSopKSc7XG5cbnZhciBQUkVSRUxFQVNFTE9PU0UgPSBSKys7XG5zcmNbUFJFUkVMRUFTRUxPT1NFXSA9ICcoPzotPygnICsgc3JjW1BSRVJFTEVBU0VJREVOVElGSUVSTE9PU0VdICtcbiAgICAgICAgICAgICAgICAgICAgICAgJyg/OlxcXFwuJyArIHNyY1tQUkVSRUxFQVNFSURFTlRJRklFUkxPT1NFXSArICcpKikpJztcblxuLy8gIyMgQnVpbGQgTWV0YWRhdGEgSWRlbnRpZmllclxuLy8gQW55IGNvbWJpbmF0aW9uIG9mIGRpZ2l0cywgbGV0dGVycywgb3IgaHlwaGVucy5cblxudmFyIEJVSUxESURFTlRJRklFUiA9IFIrKztcbnNyY1tCVUlMRElERU5USUZJRVJdID0gJ1swLTlBLVphLXotXSsnO1xuXG4vLyAjIyBCdWlsZCBNZXRhZGF0YVxuLy8gUGx1cyBzaWduLCBmb2xsb3dlZCBieSBvbmUgb3IgbW9yZSBwZXJpb2Qtc2VwYXJhdGVkIGJ1aWxkIG1ldGFkYXRhXG4vLyBpZGVudGlmaWVycy5cblxudmFyIEJVSUxEID0gUisrO1xuc3JjW0JVSUxEXSA9ICcoPzpcXFxcKygnICsgc3JjW0JVSUxESURFTlRJRklFUl0gK1xuICAgICAgICAgICAgICcoPzpcXFxcLicgKyBzcmNbQlVJTERJREVOVElGSUVSXSArICcpKikpJztcblxuXG4vLyAjIyBGdWxsIFZlcnNpb24gU3RyaW5nXG4vLyBBIG1haW4gdmVyc2lvbiwgZm9sbG93ZWQgb3B0aW9uYWxseSBieSBhIHByZS1yZWxlYXNlIHZlcnNpb24gYW5kXG4vLyBidWlsZCBtZXRhZGF0YS5cblxuLy8gTm90ZSB0aGF0IHRoZSBvbmx5IG1ham9yLCBtaW5vciwgcGF0Y2gsIGFuZCBwcmUtcmVsZWFzZSBzZWN0aW9ucyBvZlxuLy8gdGhlIHZlcnNpb24gc3RyaW5nIGFyZSBjYXB0dXJpbmcgZ3JvdXBzLiAgVGhlIGJ1aWxkIG1ldGFkYXRhIGlzIG5vdCBhXG4vLyBjYXB0dXJpbmcgZ3JvdXAsIGJlY2F1c2UgaXQgc2hvdWxkIG5vdCBldmVyIGJlIHVzZWQgaW4gdmVyc2lvblxuLy8gY29tcGFyaXNvbi5cblxudmFyIEZVTEwgPSBSKys7XG52YXIgRlVMTFBMQUlOID0gJ3Y/JyArIHNyY1tNQUlOVkVSU0lPTl0gK1xuICAgICAgICAgICAgICAgIHNyY1tQUkVSRUxFQVNFXSArICc/JyArXG4gICAgICAgICAgICAgICAgc3JjW0JVSUxEXSArICc/Jztcblxuc3JjW0ZVTExdID0gJ14nICsgRlVMTFBMQUlOICsgJyQnO1xuXG4vLyBsaWtlIGZ1bGwsIGJ1dCBhbGxvd3MgdjEuMi4zIGFuZCA9MS4yLjMsIHdoaWNoIHBlb3BsZSBkbyBzb21ldGltZXMuXG4vLyBhbHNvLCAxLjAuMGFscGhhMSAocHJlcmVsZWFzZSB3aXRob3V0IHRoZSBoeXBoZW4pIHdoaWNoIGlzIHByZXR0eVxuLy8gY29tbW9uIGluIHRoZSBucG0gcmVnaXN0cnkuXG52YXIgTE9PU0VQTEFJTiA9ICdbdj1cXFxcc10qJyArIHNyY1tNQUlOVkVSU0lPTkxPT1NFXSArXG4gICAgICAgICAgICAgICAgIHNyY1tQUkVSRUxFQVNFTE9PU0VdICsgJz8nICtcbiAgICAgICAgICAgICAgICAgc3JjW0JVSUxEXSArICc/JztcblxudmFyIExPT1NFID0gUisrO1xuc3JjW0xPT1NFXSA9ICdeJyArIExPT1NFUExBSU4gKyAnJCc7XG5cbnZhciBHVExUID0gUisrO1xuc3JjW0dUTFRdID0gJygoPzo8fD4pPz0/KSc7XG5cbi8vIFNvbWV0aGluZyBsaWtlIFwiMi4qXCIgb3IgXCIxLjIueFwiLlxuLy8gTm90ZSB0aGF0IFwieC54XCIgaXMgYSB2YWxpZCB4UmFuZ2UgaWRlbnRpZmVyLCBtZWFuaW5nIFwiYW55IHZlcnNpb25cIlxuLy8gT25seSB0aGUgZmlyc3QgaXRlbSBpcyBzdHJpY3RseSByZXF1aXJlZC5cbnZhciBYUkFOR0VJREVOVElGSUVSTE9PU0UgPSBSKys7XG5zcmNbWFJBTkdFSURFTlRJRklFUkxPT1NFXSA9IHNyY1tOVU1FUklDSURFTlRJRklFUkxPT1NFXSArICd8eHxYfFxcXFwqJztcbnZhciBYUkFOR0VJREVOVElGSUVSID0gUisrO1xuc3JjW1hSQU5HRUlERU5USUZJRVJdID0gc3JjW05VTUVSSUNJREVOVElGSUVSXSArICd8eHxYfFxcXFwqJztcblxudmFyIFhSQU5HRVBMQUlOID0gUisrO1xuc3JjW1hSQU5HRVBMQUlOXSA9ICdbdj1cXFxcc10qKCcgKyBzcmNbWFJBTkdFSURFTlRJRklFUl0gKyAnKScgK1xuICAgICAgICAgICAgICAgICAgICcoPzpcXFxcLignICsgc3JjW1hSQU5HRUlERU5USUZJRVJdICsgJyknICtcbiAgICAgICAgICAgICAgICAgICAnKD86XFxcXC4oJyArIHNyY1tYUkFOR0VJREVOVElGSUVSXSArICcpJyArXG4gICAgICAgICAgICAgICAgICAgJyg/OicgKyBzcmNbUFJFUkVMRUFTRV0gKyAnKT8nICtcbiAgICAgICAgICAgICAgICAgICBzcmNbQlVJTERdICsgJz8nICtcbiAgICAgICAgICAgICAgICAgICAnKT8pPyc7XG5cbnZhciBYUkFOR0VQTEFJTkxPT1NFID0gUisrO1xuc3JjW1hSQU5HRVBMQUlOTE9PU0VdID0gJ1t2PVxcXFxzXSooJyArIHNyY1tYUkFOR0VJREVOVElGSUVSTE9PU0VdICsgJyknICtcbiAgICAgICAgICAgICAgICAgICAgICAgICcoPzpcXFxcLignICsgc3JjW1hSQU5HRUlERU5USUZJRVJMT09TRV0gKyAnKScgK1xuICAgICAgICAgICAgICAgICAgICAgICAgJyg/OlxcXFwuKCcgKyBzcmNbWFJBTkdFSURFTlRJRklFUkxPT1NFXSArICcpJyArXG4gICAgICAgICAgICAgICAgICAgICAgICAnKD86JyArIHNyY1tQUkVSRUxFQVNFTE9PU0VdICsgJyk/JyArXG4gICAgICAgICAgICAgICAgICAgICAgICBzcmNbQlVJTERdICsgJz8nICtcbiAgICAgICAgICAgICAgICAgICAgICAgICcpPyk/JztcblxudmFyIFhSQU5HRSA9IFIrKztcbnNyY1tYUkFOR0VdID0gJ14nICsgc3JjW0dUTFRdICsgJ1xcXFxzKicgKyBzcmNbWFJBTkdFUExBSU5dICsgJyQnO1xudmFyIFhSQU5HRUxPT1NFID0gUisrO1xuc3JjW1hSQU5HRUxPT1NFXSA9ICdeJyArIHNyY1tHVExUXSArICdcXFxccyonICsgc3JjW1hSQU5HRVBMQUlOTE9PU0VdICsgJyQnO1xuXG4vLyBUaWxkZSByYW5nZXMuXG4vLyBNZWFuaW5nIGlzIFwicmVhc29uYWJseSBhdCBvciBncmVhdGVyIHRoYW5cIlxudmFyIExPTkVUSUxERSA9IFIrKztcbnNyY1tMT05FVElMREVdID0gJyg/On4+PyknO1xuXG52YXIgVElMREVUUklNID0gUisrO1xuc3JjW1RJTERFVFJJTV0gPSAnKFxcXFxzKiknICsgc3JjW0xPTkVUSUxERV0gKyAnXFxcXHMrJztcbnJlW1RJTERFVFJJTV0gPSBuZXcgUmVnRXhwKHNyY1tUSUxERVRSSU1dLCAnZycpO1xudmFyIHRpbGRlVHJpbVJlcGxhY2UgPSAnJDF+JztcblxudmFyIFRJTERFID0gUisrO1xuc3JjW1RJTERFXSA9ICdeJyArIHNyY1tMT05FVElMREVdICsgc3JjW1hSQU5HRVBMQUlOXSArICckJztcbnZhciBUSUxERUxPT1NFID0gUisrO1xuc3JjW1RJTERFTE9PU0VdID0gJ14nICsgc3JjW0xPTkVUSUxERV0gKyBzcmNbWFJBTkdFUExBSU5MT09TRV0gKyAnJCc7XG5cbi8vIENhcmV0IHJhbmdlcy5cbi8vIE1lYW5pbmcgaXMgXCJhdCBsZWFzdCBhbmQgYmFja3dhcmRzIGNvbXBhdGlibGUgd2l0aFwiXG52YXIgTE9ORUNBUkVUID0gUisrO1xuc3JjW0xPTkVDQVJFVF0gPSAnKD86XFxcXF4pJztcblxudmFyIENBUkVUVFJJTSA9IFIrKztcbnNyY1tDQVJFVFRSSU1dID0gJyhcXFxccyopJyArIHNyY1tMT05FQ0FSRVRdICsgJ1xcXFxzKyc7XG5yZVtDQVJFVFRSSU1dID0gbmV3IFJlZ0V4cChzcmNbQ0FSRVRUUklNXSwgJ2cnKTtcbnZhciBjYXJldFRyaW1SZXBsYWNlID0gJyQxXic7XG5cbnZhciBDQVJFVCA9IFIrKztcbnNyY1tDQVJFVF0gPSAnXicgKyBzcmNbTE9ORUNBUkVUXSArIHNyY1tYUkFOR0VQTEFJTl0gKyAnJCc7XG52YXIgQ0FSRVRMT09TRSA9IFIrKztcbnNyY1tDQVJFVExPT1NFXSA9ICdeJyArIHNyY1tMT05FQ0FSRVRdICsgc3JjW1hSQU5HRVBMQUlOTE9PU0VdICsgJyQnO1xuXG4vLyBBIHNpbXBsZSBndC9sdC9lcSB0aGluZywgb3IganVzdCBcIlwiIHRvIGluZGljYXRlIFwiYW55IHZlcnNpb25cIlxudmFyIENPTVBBUkFUT1JMT09TRSA9IFIrKztcbnNyY1tDT01QQVJBVE9STE9PU0VdID0gJ14nICsgc3JjW0dUTFRdICsgJ1xcXFxzKignICsgTE9PU0VQTEFJTiArICcpJHxeJCc7XG52YXIgQ09NUEFSQVRPUiA9IFIrKztcbnNyY1tDT01QQVJBVE9SXSA9ICdeJyArIHNyY1tHVExUXSArICdcXFxccyooJyArIEZVTExQTEFJTiArICcpJHxeJCc7XG5cblxuLy8gQW4gZXhwcmVzc2lvbiB0byBzdHJpcCBhbnkgd2hpdGVzcGFjZSBiZXR3ZWVuIHRoZSBndGx0IGFuZCB0aGUgdGhpbmdcbi8vIGl0IG1vZGlmaWVzLCBzbyB0aGF0IGA+IDEuMi4zYCA9PT4gYD4xLjIuM2BcbnZhciBDT01QQVJBVE9SVFJJTSA9IFIrKztcbnNyY1tDT01QQVJBVE9SVFJJTV0gPSAnKFxcXFxzKiknICsgc3JjW0dUTFRdICtcbiAgICAgICAgICAgICAgICAgICAgICAnXFxcXHMqKCcgKyBMT09TRVBMQUlOICsgJ3wnICsgc3JjW1hSQU5HRVBMQUlOXSArICcpJztcblxuLy8gdGhpcyBvbmUgaGFzIHRvIHVzZSB0aGUgL2cgZmxhZ1xucmVbQ09NUEFSQVRPUlRSSU1dID0gbmV3IFJlZ0V4cChzcmNbQ09NUEFSQVRPUlRSSU1dLCAnZycpO1xudmFyIGNvbXBhcmF0b3JUcmltUmVwbGFjZSA9ICckMSQyJDMnO1xuXG5cbi8vIFNvbWV0aGluZyBsaWtlIGAxLjIuMyAtIDEuMi40YFxuLy8gTm90ZSB0aGF0IHRoZXNlIGFsbCB1c2UgdGhlIGxvb3NlIGZvcm0sIGJlY2F1c2UgdGhleSdsbCBiZVxuLy8gY2hlY2tlZCBhZ2FpbnN0IGVpdGhlciB0aGUgc3RyaWN0IG9yIGxvb3NlIGNvbXBhcmF0b3IgZm9ybVxuLy8gbGF0ZXIuXG52YXIgSFlQSEVOUkFOR0UgPSBSKys7XG5zcmNbSFlQSEVOUkFOR0VdID0gJ15cXFxccyooJyArIHNyY1tYUkFOR0VQTEFJTl0gKyAnKScgK1xuICAgICAgICAgICAgICAgICAgICdcXFxccystXFxcXHMrJyArXG4gICAgICAgICAgICAgICAgICAgJygnICsgc3JjW1hSQU5HRVBMQUlOXSArICcpJyArXG4gICAgICAgICAgICAgICAgICAgJ1xcXFxzKiQnO1xuXG52YXIgSFlQSEVOUkFOR0VMT09TRSA9IFIrKztcbnNyY1tIWVBIRU5SQU5HRUxPT1NFXSA9ICdeXFxcXHMqKCcgKyBzcmNbWFJBTkdFUExBSU5MT09TRV0gKyAnKScgK1xuICAgICAgICAgICAgICAgICAgICAgICAgJ1xcXFxzKy1cXFxccysnICtcbiAgICAgICAgICAgICAgICAgICAgICAgICcoJyArIHNyY1tYUkFOR0VQTEFJTkxPT1NFXSArICcpJyArXG4gICAgICAgICAgICAgICAgICAgICAgICAnXFxcXHMqJCc7XG5cbi8vIFN0YXIgcmFuZ2VzIGJhc2ljYWxseSBqdXN0IGFsbG93IGFueXRoaW5nIGF0IGFsbC5cbnZhciBTVEFSID0gUisrO1xuc3JjW1NUQVJdID0gJyg8fD4pPz0/XFxcXHMqXFxcXConO1xuXG4vLyBDb21waWxlIHRvIGFjdHVhbCByZWdleHAgb2JqZWN0cy5cbi8vIEFsbCBhcmUgZmxhZy1mcmVlLCB1bmxlc3MgdGhleSB3ZXJlIGNyZWF0ZWQgYWJvdmUgd2l0aCBhIGZsYWcuXG5mb3IgKHZhciBpID0gMDsgaSA8IFI7IGkrKykge1xuICBkZWJ1ZyhpLCBzcmNbaV0pO1xuICBpZiAoIXJlW2ldKVxuICAgIHJlW2ldID0gbmV3IFJlZ0V4cChzcmNbaV0pO1xufVxuXG5leHBvcnRzLnBhcnNlID0gcGFyc2U7XG5mdW5jdGlvbiBwYXJzZSh2ZXJzaW9uLCBsb29zZSkge1xuICBpZiAodmVyc2lvbiBpbnN0YW5jZW9mIFNlbVZlcilcbiAgICByZXR1cm4gdmVyc2lvbjtcblxuICBpZiAodHlwZW9mIHZlcnNpb24gIT09ICdzdHJpbmcnKVxuICAgIHJldHVybiBudWxsO1xuXG4gIGlmICh2ZXJzaW9uLmxlbmd0aCA+IE1BWF9MRU5HVEgpXG4gICAgcmV0dXJuIG51bGw7XG5cbiAgdmFyIHIgPSBsb29zZSA/IHJlW0xPT1NFXSA6IHJlW0ZVTExdO1xuICBpZiAoIXIudGVzdCh2ZXJzaW9uKSlcbiAgICByZXR1cm4gbnVsbDtcblxuICB0cnkge1xuICAgIHJldHVybiBuZXcgU2VtVmVyKHZlcnNpb24sIGxvb3NlKTtcbiAgfSBjYXRjaCAoZXIpIHtcbiAgICByZXR1cm4gbnVsbDtcbiAgfVxufVxuXG5leHBvcnRzLnZhbGlkID0gdmFsaWQ7XG5mdW5jdGlvbiB2YWxpZCh2ZXJzaW9uLCBsb29zZSkge1xuICB2YXIgdiA9IHBhcnNlKHZlcnNpb24sIGxvb3NlKTtcbiAgcmV0dXJuIHYgPyB2LnZlcnNpb24gOiBudWxsO1xufVxuXG5cbmV4cG9ydHMuY2xlYW4gPSBjbGVhbjtcbmZ1bmN0aW9uIGNsZWFuKHZlcnNpb24sIGxvb3NlKSB7XG4gIHZhciBzID0gcGFyc2UodmVyc2lvbi50cmltKCkucmVwbGFjZSgvXls9dl0rLywgJycpLCBsb29zZSk7XG4gIHJldHVybiBzID8gcy52ZXJzaW9uIDogbnVsbDtcbn1cblxuZXhwb3J0cy5TZW1WZXIgPSBTZW1WZXI7XG5cbmZ1bmN0aW9uIFNlbVZlcih2ZXJzaW9uLCBsb29zZSkge1xuICBpZiAodmVyc2lvbiBpbnN0YW5jZW9mIFNlbVZlcikge1xuICAgIGlmICh2ZXJzaW9uLmxvb3NlID09PSBsb29zZSlcbiAgICAgIHJldHVybiB2ZXJzaW9uO1xuICAgIGVsc2VcbiAgICAgIHZlcnNpb24gPSB2ZXJzaW9uLnZlcnNpb247XG4gIH0gZWxzZSBpZiAodHlwZW9mIHZlcnNpb24gIT09ICdzdHJpbmcnKSB7XG4gICAgdGhyb3cgbmV3IFR5cGVFcnJvcignSW52YWxpZCBWZXJzaW9uOiAnICsgdmVyc2lvbik7XG4gIH1cblxuICBpZiAodmVyc2lvbi5sZW5ndGggPiBNQVhfTEVOR1RIKVxuICAgIHRocm93IG5ldyBUeXBlRXJyb3IoJ3ZlcnNpb24gaXMgbG9uZ2VyIHRoYW4gJyArIE1BWF9MRU5HVEggKyAnIGNoYXJhY3RlcnMnKVxuXG4gIGlmICghKHRoaXMgaW5zdGFuY2VvZiBTZW1WZXIpKVxuICAgIHJldHVybiBuZXcgU2VtVmVyKHZlcnNpb24sIGxvb3NlKTtcblxuICBkZWJ1ZygnU2VtVmVyJywgdmVyc2lvbiwgbG9vc2UpO1xuICB0aGlzLmxvb3NlID0gbG9vc2U7XG4gIHZhciBtID0gdmVyc2lvbi50cmltKCkubWF0Y2gobG9vc2UgPyByZVtMT09TRV0gOiByZVtGVUxMXSk7XG5cbiAgaWYgKCFtKVxuICAgIHRocm93IG5ldyBUeXBlRXJyb3IoJ0ludmFsaWQgVmVyc2lvbjogJyArIHZlcnNpb24pO1xuXG4gIHRoaXMucmF3ID0gdmVyc2lvbjtcblxuICAvLyB0aGVzZSBhcmUgYWN0dWFsbHkgbnVtYmVyc1xuICB0aGlzLm1ham9yID0gK21bMV07XG4gIHRoaXMubWlub3IgPSArbVsyXTtcbiAgdGhpcy5wYXRjaCA9ICttWzNdO1xuXG4gIGlmICh0aGlzLm1ham9yID4gTUFYX1NBRkVfSU5URUdFUiB8fCB0aGlzLm1ham9yIDwgMClcbiAgICB0aHJvdyBuZXcgVHlwZUVycm9yKCdJbnZhbGlkIG1ham9yIHZlcnNpb24nKVxuXG4gIGlmICh0aGlzLm1pbm9yID4gTUFYX1NBRkVfSU5URUdFUiB8fCB0aGlzLm1pbm9yIDwgMClcbiAgICB0aHJvdyBuZXcgVHlwZUVycm9yKCdJbnZhbGlkIG1pbm9yIHZlcnNpb24nKVxuXG4gIGlmICh0aGlzLnBhdGNoID4gTUFYX1NBRkVfSU5URUdFUiB8fCB0aGlzLnBhdGNoIDwgMClcbiAgICB0aHJvdyBuZXcgVHlwZUVycm9yKCdJbnZhbGlkIHBhdGNoIHZlcnNpb24nKVxuXG4gIC8vIG51bWJlcmlmeSBhbnkgcHJlcmVsZWFzZSBudW1lcmljIGlkc1xuICBpZiAoIW1bNF0pXG4gICAgdGhpcy5wcmVyZWxlYXNlID0gW107XG4gIGVsc2VcbiAgICB0aGlzLnByZXJlbGVhc2UgPSBtWzRdLnNwbGl0KCcuJykubWFwKGZ1bmN0aW9uKGlkKSB7XG4gICAgICBpZiAoL15bMC05XSskLy50ZXN0KGlkKSkge1xuICAgICAgICB2YXIgbnVtID0gK2lkO1xuICAgICAgICBpZiAobnVtID49IDAgJiYgbnVtIDwgTUFYX1NBRkVfSU5URUdFUilcbiAgICAgICAgICByZXR1cm4gbnVtO1xuICAgICAgfVxuICAgICAgcmV0dXJuIGlkO1xuICAgIH0pO1xuXG4gIHRoaXMuYnVpbGQgPSBtWzVdID8gbVs1XS5zcGxpdCgnLicpIDogW107XG4gIHRoaXMuZm9ybWF0KCk7XG59XG5cblNlbVZlci5wcm90b3R5cGUuZm9ybWF0ID0gZnVuY3Rpb24oKSB7XG4gIHRoaXMudmVyc2lvbiA9IHRoaXMubWFqb3IgKyAnLicgKyB0aGlzLm1pbm9yICsgJy4nICsgdGhpcy5wYXRjaDtcbiAgaWYgKHRoaXMucHJlcmVsZWFzZS5sZW5ndGgpXG4gICAgdGhpcy52ZXJzaW9uICs9ICctJyArIHRoaXMucHJlcmVsZWFzZS5qb2luKCcuJyk7XG4gIHJldHVybiB0aGlzLnZlcnNpb247XG59O1xuXG5TZW1WZXIucHJvdG90eXBlLnRvU3RyaW5nID0gZnVuY3Rpb24oKSB7XG4gIHJldHVybiB0aGlzLnZlcnNpb247XG59O1xuXG5TZW1WZXIucHJvdG90eXBlLmNvbXBhcmUgPSBmdW5jdGlvbihvdGhlcikge1xuICBkZWJ1ZygnU2VtVmVyLmNvbXBhcmUnLCB0aGlzLnZlcnNpb24sIHRoaXMubG9vc2UsIG90aGVyKTtcbiAgaWYgKCEob3RoZXIgaW5zdGFuY2VvZiBTZW1WZXIpKVxuICAgIG90aGVyID0gbmV3IFNlbVZlcihvdGhlciwgdGhpcy5sb29zZSk7XG5cbiAgcmV0dXJuIHRoaXMuY29tcGFyZU1haW4ob3RoZXIpIHx8IHRoaXMuY29tcGFyZVByZShvdGhlcik7XG59O1xuXG5TZW1WZXIucHJvdG90eXBlLmNvbXBhcmVNYWluID0gZnVuY3Rpb24ob3RoZXIpIHtcbiAgaWYgKCEob3RoZXIgaW5zdGFuY2VvZiBTZW1WZXIpKVxuICAgIG90aGVyID0gbmV3IFNlbVZlcihvdGhlciwgdGhpcy5sb29zZSk7XG5cbiAgcmV0dXJuIGNvbXBhcmVJZGVudGlmaWVycyh0aGlzLm1ham9yLCBvdGhlci5tYWpvcikgfHxcbiAgICAgICAgIGNvbXBhcmVJZGVudGlmaWVycyh0aGlzLm1pbm9yLCBvdGhlci5taW5vcikgfHxcbiAgICAgICAgIGNvbXBhcmVJZGVudGlmaWVycyh0aGlzLnBhdGNoLCBvdGhlci5wYXRjaCk7XG59O1xuXG5TZW1WZXIucHJvdG90eXBlLmNvbXBhcmVQcmUgPSBmdW5jdGlvbihvdGhlcikge1xuICBpZiAoIShvdGhlciBpbnN0YW5jZW9mIFNlbVZlcikpXG4gICAgb3RoZXIgPSBuZXcgU2VtVmVyKG90aGVyLCB0aGlzLmxvb3NlKTtcblxuICAvLyBOT1QgaGF2aW5nIGEgcHJlcmVsZWFzZSBpcyA+IGhhdmluZyBvbmVcbiAgaWYgKHRoaXMucHJlcmVsZWFzZS5sZW5ndGggJiYgIW90aGVyLnByZXJlbGVhc2UubGVuZ3RoKVxuICAgIHJldHVybiAtMTtcbiAgZWxzZSBpZiAoIXRoaXMucHJlcmVsZWFzZS5sZW5ndGggJiYgb3RoZXIucHJlcmVsZWFzZS5sZW5ndGgpXG4gICAgcmV0dXJuIDE7XG4gIGVsc2UgaWYgKCF0aGlzLnByZXJlbGVhc2UubGVuZ3RoICYmICFvdGhlci5wcmVyZWxlYXNlLmxlbmd0aClcbiAgICByZXR1cm4gMDtcblxuICB2YXIgaSA9IDA7XG4gIGRvIHtcbiAgICB2YXIgYSA9IHRoaXMucHJlcmVsZWFzZVtpXTtcbiAgICB2YXIgYiA9IG90aGVyLnByZXJlbGVhc2VbaV07XG4gICAgZGVidWcoJ3ByZXJlbGVhc2UgY29tcGFyZScsIGksIGEsIGIpO1xuICAgIGlmIChhID09PSB1bmRlZmluZWQgJiYgYiA9PT0gdW5kZWZpbmVkKVxuICAgICAgcmV0dXJuIDA7XG4gICAgZWxzZSBpZiAoYiA9PT0gdW5kZWZpbmVkKVxuICAgICAgcmV0dXJuIDE7XG4gICAgZWxzZSBpZiAoYSA9PT0gdW5kZWZpbmVkKVxuICAgICAgcmV0dXJuIC0xO1xuICAgIGVsc2UgaWYgKGEgPT09IGIpXG4gICAgICBjb250aW51ZTtcbiAgICBlbHNlXG4gICAgICByZXR1cm4gY29tcGFyZUlkZW50aWZpZXJzKGEsIGIpO1xuICB9IHdoaWxlICgrK2kpO1xufTtcblxuLy8gcHJlbWlub3Igd2lsbCBidW1wIHRoZSB2ZXJzaW9uIHVwIHRvIHRoZSBuZXh0IG1pbm9yIHJlbGVhc2UsIGFuZCBpbW1lZGlhdGVseVxuLy8gZG93biB0byBwcmUtcmVsZWFzZS4gcHJlbWFqb3IgYW5kIHByZXBhdGNoIHdvcmsgdGhlIHNhbWUgd2F5LlxuU2VtVmVyLnByb3RvdHlwZS5pbmMgPSBmdW5jdGlvbihyZWxlYXNlLCBpZGVudGlmaWVyKSB7XG4gIHN3aXRjaCAocmVsZWFzZSkge1xuICAgIGNhc2UgJ3ByZW1ham9yJzpcbiAgICAgIHRoaXMucHJlcmVsZWFzZS5sZW5ndGggPSAwO1xuICAgICAgdGhpcy5wYXRjaCA9IDA7XG4gICAgICB0aGlzLm1pbm9yID0gMDtcbiAgICAgIHRoaXMubWFqb3IrKztcbiAgICAgIHRoaXMuaW5jKCdwcmUnLCBpZGVudGlmaWVyKTtcbiAgICAgIGJyZWFrO1xuICAgIGNhc2UgJ3ByZW1pbm9yJzpcbiAgICAgIHRoaXMucHJlcmVsZWFzZS5sZW5ndGggPSAwO1xuICAgICAgdGhpcy5wYXRjaCA9IDA7XG4gICAgICB0aGlzLm1pbm9yKys7XG4gICAgICB0aGlzLmluYygncHJlJywgaWRlbnRpZmllcik7XG4gICAgICBicmVhaztcbiAgICBjYXNlICdwcmVwYXRjaCc6XG4gICAgICAvLyBJZiB0aGlzIGlzIGFscmVhZHkgYSBwcmVyZWxlYXNlLCBpdCB3aWxsIGJ1bXAgdG8gdGhlIG5leHQgdmVyc2lvblxuICAgICAgLy8gZHJvcCBhbnkgcHJlcmVsZWFzZXMgdGhhdCBtaWdodCBhbHJlYWR5IGV4aXN0LCBzaW5jZSB0aGV5IGFyZSBub3RcbiAgICAgIC8vIHJlbGV2YW50IGF0IHRoaXMgcG9pbnQuXG4gICAgICB0aGlzLnByZXJlbGVhc2UubGVuZ3RoID0gMDtcbiAgICAgIHRoaXMuaW5jKCdwYXRjaCcsIGlkZW50aWZpZXIpO1xuICAgICAgdGhpcy5pbmMoJ3ByZScsIGlkZW50aWZpZXIpO1xuICAgICAgYnJlYWs7XG4gICAgLy8gSWYgdGhlIGlucHV0IGlzIGEgbm9uLXByZXJlbGVhc2UgdmVyc2lvbiwgdGhpcyBhY3RzIHRoZSBzYW1lIGFzXG4gICAgLy8gcHJlcGF0Y2guXG4gICAgY2FzZSAncHJlcmVsZWFzZSc6XG4gICAgICBpZiAodGhpcy5wcmVyZWxlYXNlLmxlbmd0aCA9PT0gMClcbiAgICAgICAgdGhpcy5pbmMoJ3BhdGNoJywgaWRlbnRpZmllcik7XG4gICAgICB0aGlzLmluYygncHJlJywgaWRlbnRpZmllcik7XG4gICAgICBicmVhaztcblxuICAgIGNhc2UgJ21ham9yJzpcbiAgICAgIC8vIElmIHRoaXMgaXMgYSBwcmUtbWFqb3IgdmVyc2lvbiwgYnVtcCB1cCB0byB0aGUgc2FtZSBtYWpvciB2ZXJzaW9uLlxuICAgICAgLy8gT3RoZXJ3aXNlIGluY3JlbWVudCBtYWpvci5cbiAgICAgIC8vIDEuMC4wLTUgYnVtcHMgdG8gMS4wLjBcbiAgICAgIC8vIDEuMS4wIGJ1bXBzIHRvIDIuMC4wXG4gICAgICBpZiAodGhpcy5taW5vciAhPT0gMCB8fCB0aGlzLnBhdGNoICE9PSAwIHx8IHRoaXMucHJlcmVsZWFzZS5sZW5ndGggPT09IDApXG4gICAgICAgIHRoaXMubWFqb3IrKztcbiAgICAgIHRoaXMubWlub3IgPSAwO1xuICAgICAgdGhpcy5wYXRjaCA9IDA7XG4gICAgICB0aGlzLnByZXJlbGVhc2UgPSBbXTtcbiAgICAgIGJyZWFrO1xuICAgIGNhc2UgJ21pbm9yJzpcbiAgICAgIC8vIElmIHRoaXMgaXMgYSBwcmUtbWlub3IgdmVyc2lvbiwgYnVtcCB1cCB0byB0aGUgc2FtZSBtaW5vciB2ZXJzaW9uLlxuICAgICAgLy8gT3RoZXJ3aXNlIGluY3JlbWVudCBtaW5vci5cbiAgICAgIC8vIDEuMi4wLTUgYnVtcHMgdG8gMS4yLjBcbiAgICAgIC8vIDEuMi4xIGJ1bXBzIHRvIDEuMy4wXG4gICAgICBpZiAodGhpcy5wYXRjaCAhPT0gMCB8fCB0aGlzLnByZXJlbGVhc2UubGVuZ3RoID09PSAwKVxuICAgICAgICB0aGlzLm1pbm9yKys7XG4gICAgICB0aGlzLnBhdGNoID0gMDtcbiAgICAgIHRoaXMucHJlcmVsZWFzZSA9IFtdO1xuICAgICAgYnJlYWs7XG4gICAgY2FzZSAncGF0Y2gnOlxuICAgICAgLy8gSWYgdGhpcyBpcyBub3QgYSBwcmUtcmVsZWFzZSB2ZXJzaW9uLCBpdCB3aWxsIGluY3JlbWVudCB0aGUgcGF0Y2guXG4gICAgICAvLyBJZiBpdCBpcyBhIHByZS1yZWxlYXNlIGl0IHdpbGwgYnVtcCB1cCB0byB0aGUgc2FtZSBwYXRjaCB2ZXJzaW9uLlxuICAgICAgLy8gMS4yLjAtNSBwYXRjaGVzIHRvIDEuMi4wXG4gICAgICAvLyAxLjIuMCBwYXRjaGVzIHRvIDEuMi4xXG4gICAgICBpZiAodGhpcy5wcmVyZWxlYXNlLmxlbmd0aCA9PT0gMClcbiAgICAgICAgdGhpcy5wYXRjaCsrO1xuICAgICAgdGhpcy5wcmVyZWxlYXNlID0gW107XG4gICAgICBicmVhaztcbiAgICAvLyBUaGlzIHByb2JhYmx5IHNob3VsZG4ndCBiZSB1c2VkIHB1YmxpY2x5LlxuICAgIC8vIDEuMC4wIFwicHJlXCIgd291bGQgYmVjb21lIDEuMC4wLTAgd2hpY2ggaXMgdGhlIHdyb25nIGRpcmVjdGlvbi5cbiAgICBjYXNlICdwcmUnOlxuICAgICAgaWYgKHRoaXMucHJlcmVsZWFzZS5sZW5ndGggPT09IDApXG4gICAgICAgIHRoaXMucHJlcmVsZWFzZSA9IFswXTtcbiAgICAgIGVsc2Uge1xuICAgICAgICB2YXIgaSA9IHRoaXMucHJlcmVsZWFzZS5sZW5ndGg7XG4gICAgICAgIHdoaWxlICgtLWkgPj0gMCkge1xuICAgICAgICAgIGlmICh0eXBlb2YgdGhpcy5wcmVyZWxlYXNlW2ldID09PSAnbnVtYmVyJykge1xuICAgICAgICAgICAgdGhpcy5wcmVyZWxlYXNlW2ldKys7XG4gICAgICAgICAgICBpID0gLTI7XG4gICAgICAgICAgfVxuICAgICAgICB9XG4gICAgICAgIGlmIChpID09PSAtMSkgLy8gZGlkbid0IGluY3JlbWVudCBhbnl0aGluZ1xuICAgICAgICAgIHRoaXMucHJlcmVsZWFzZS5wdXNoKDApO1xuICAgICAgfVxuICAgICAgaWYgKGlkZW50aWZpZXIpIHtcbiAgICAgICAgLy8gMS4yLjAtYmV0YS4xIGJ1bXBzIHRvIDEuMi4wLWJldGEuMixcbiAgICAgICAgLy8gMS4yLjAtYmV0YS5mb29ibHogb3IgMS4yLjAtYmV0YSBidW1wcyB0byAxLjIuMC1iZXRhLjBcbiAgICAgICAgaWYgKHRoaXMucHJlcmVsZWFzZVswXSA9PT0gaWRlbnRpZmllcikge1xuICAgICAgICAgIGlmIChpc05hTih0aGlzLnByZXJlbGVhc2VbMV0pKVxuICAgICAgICAgICAgdGhpcy5wcmVyZWxlYXNlID0gW2lkZW50aWZpZXIsIDBdO1xuICAgICAgICB9IGVsc2VcbiAgICAgICAgICB0aGlzLnByZXJlbGVhc2UgPSBbaWRlbnRpZmllciwgMF07XG4gICAgICB9XG4gICAgICBicmVhaztcblxuICAgIGRlZmF1bHQ6XG4gICAgICB0aHJvdyBuZXcgRXJyb3IoJ2ludmFsaWQgaW5jcmVtZW50IGFyZ3VtZW50OiAnICsgcmVsZWFzZSk7XG4gIH1cbiAgdGhpcy5mb3JtYXQoKTtcbiAgdGhpcy5yYXcgPSB0aGlzLnZlcnNpb247XG4gIHJldHVybiB0aGlzO1xufTtcblxuZXhwb3J0cy5pbmMgPSBpbmM7XG5mdW5jdGlvbiBpbmModmVyc2lvbiwgcmVsZWFzZSwgbG9vc2UsIGlkZW50aWZpZXIpIHtcbiAgaWYgKHR5cGVvZihsb29zZSkgPT09ICdzdHJpbmcnKSB7XG4gICAgaWRlbnRpZmllciA9IGxvb3NlO1xuICAgIGxvb3NlID0gdW5kZWZpbmVkO1xuICB9XG5cbiAgdHJ5IHtcbiAgICByZXR1cm4gbmV3IFNlbVZlcih2ZXJzaW9uLCBsb29zZSkuaW5jKHJlbGVhc2UsIGlkZW50aWZpZXIpLnZlcnNpb247XG4gIH0gY2F0Y2ggKGVyKSB7XG4gICAgcmV0dXJuIG51bGw7XG4gIH1cbn1cblxuZXhwb3J0cy5kaWZmID0gZGlmZjtcbmZ1bmN0aW9uIGRpZmYodmVyc2lvbjEsIHZlcnNpb24yKSB7XG4gIGlmIChlcSh2ZXJzaW9uMSwgdmVyc2lvbjIpKSB7XG4gICAgcmV0dXJuIG51bGw7XG4gIH0gZWxzZSB7XG4gICAgdmFyIHYxID0gcGFyc2UodmVyc2lvbjEpO1xuICAgIHZhciB2MiA9IHBhcnNlKHZlcnNpb24yKTtcbiAgICBpZiAodjEucHJlcmVsZWFzZS5sZW5ndGggfHwgdjIucHJlcmVsZWFzZS5sZW5ndGgpIHtcbiAgICAgIGZvciAodmFyIGtleSBpbiB2MSkge1xuICAgICAgICBpZiAoa2V5ID09PSAnbWFqb3InIHx8IGtleSA9PT0gJ21pbm9yJyB8fCBrZXkgPT09ICdwYXRjaCcpIHtcbiAgICAgICAgICBpZiAodjFba2V5XSAhPT0gdjJba2V5XSkge1xuICAgICAgICAgICAgcmV0dXJuICdwcmUnK2tleTtcbiAgICAgICAgICB9XG4gICAgICAgIH1cbiAgICAgIH1cbiAgICAgIHJldHVybiAncHJlcmVsZWFzZSc7XG4gICAgfVxuICAgIGZvciAodmFyIGtleSBpbiB2MSkge1xuICAgICAgaWYgKGtleSA9PT0gJ21ham9yJyB8fCBrZXkgPT09ICdtaW5vcicgfHwga2V5ID09PSAncGF0Y2gnKSB7XG4gICAgICAgIGlmICh2MVtrZXldICE9PSB2MltrZXldKSB7XG4gICAgICAgICAgcmV0dXJuIGtleTtcbiAgICAgICAgfVxuICAgICAgfVxuICAgIH1cbiAgfVxufVxuXG5leHBvcnRzLmNvbXBhcmVJZGVudGlmaWVycyA9IGNvbXBhcmVJZGVudGlmaWVycztcblxudmFyIG51bWVyaWMgPSAvXlswLTldKyQvO1xuZnVuY3Rpb24gY29tcGFyZUlkZW50aWZpZXJzKGEsIGIpIHtcbiAgdmFyIGFudW0gPSBudW1lcmljLnRlc3QoYSk7XG4gIHZhciBibnVtID0gbnVtZXJpYy50ZXN0KGIpO1xuXG4gIGlmIChhbnVtICYmIGJudW0pIHtcbiAgICBhID0gK2E7XG4gICAgYiA9ICtiO1xuICB9XG5cbiAgcmV0dXJuIChhbnVtICYmICFibnVtKSA/IC0xIDpcbiAgICAgICAgIChibnVtICYmICFhbnVtKSA/IDEgOlxuICAgICAgICAgYSA8IGIgPyAtMSA6XG4gICAgICAgICBhID4gYiA/IDEgOlxuICAgICAgICAgMDtcbn1cblxuZXhwb3J0cy5yY29tcGFyZUlkZW50aWZpZXJzID0gcmNvbXBhcmVJZGVudGlmaWVycztcbmZ1bmN0aW9uIHJjb21wYXJlSWRlbnRpZmllcnMoYSwgYikge1xuICByZXR1cm4gY29tcGFyZUlkZW50aWZpZXJzKGIsIGEpO1xufVxuXG5leHBvcnRzLm1ham9yID0gbWFqb3I7XG5mdW5jdGlvbiBtYWpvcihhLCBsb29zZSkge1xuICByZXR1cm4gbmV3IFNlbVZlcihhLCBsb29zZSkubWFqb3I7XG59XG5cbmV4cG9ydHMubWlub3IgPSBtaW5vcjtcbmZ1bmN0aW9uIG1pbm9yKGEsIGxvb3NlKSB7XG4gIHJldHVybiBuZXcgU2VtVmVyKGEsIGxvb3NlKS5taW5vcjtcbn1cblxuZXhwb3J0cy5wYXRjaCA9IHBhdGNoO1xuZnVuY3Rpb24gcGF0Y2goYSwgbG9vc2UpIHtcbiAgcmV0dXJuIG5ldyBTZW1WZXIoYSwgbG9vc2UpLnBhdGNoO1xufVxuXG5leHBvcnRzLmNvbXBhcmUgPSBjb21wYXJlO1xuZnVuY3Rpb24gY29tcGFyZShhLCBiLCBsb29zZSkge1xuICByZXR1cm4gbmV3IFNlbVZlcihhLCBsb29zZSkuY29tcGFyZShiKTtcbn1cblxuZXhwb3J0cy5jb21wYXJlTG9vc2UgPSBjb21wYXJlTG9vc2U7XG5mdW5jdGlvbiBjb21wYXJlTG9vc2UoYSwgYikge1xuICByZXR1cm4gY29tcGFyZShhLCBiLCB0cnVlKTtcbn1cblxuZXhwb3J0cy5yY29tcGFyZSA9IHJjb21wYXJlO1xuZnVuY3Rpb24gcmNvbXBhcmUoYSwgYiwgbG9vc2UpIHtcbiAgcmV0dXJuIGNvbXBhcmUoYiwgYSwgbG9vc2UpO1xufVxuXG5leHBvcnRzLnNvcnQgPSBzb3J0O1xuZnVuY3Rpb24gc29ydChsaXN0LCBsb29zZSkge1xuICByZXR1cm4gbGlzdC5zb3J0KGZ1bmN0aW9uKGEsIGIpIHtcbiAgICByZXR1cm4gZXhwb3J0cy5jb21wYXJlKGEsIGIsIGxvb3NlKTtcbiAgfSk7XG59XG5cbmV4cG9ydHMucnNvcnQgPSByc29ydDtcbmZ1bmN0aW9uIHJzb3J0KGxpc3QsIGxvb3NlKSB7XG4gIHJldHVybiBsaXN0LnNvcnQoZnVuY3Rpb24oYSwgYikge1xuICAgIHJldHVybiBleHBvcnRzLnJjb21wYXJlKGEsIGIsIGxvb3NlKTtcbiAgfSk7XG59XG5cbmV4cG9ydHMuZ3QgPSBndDtcbmZ1bmN0aW9uIGd0KGEsIGIsIGxvb3NlKSB7XG4gIHJldHVybiBjb21wYXJlKGEsIGIsIGxvb3NlKSA+IDA7XG59XG5cbmV4cG9ydHMubHQgPSBsdDtcbmZ1bmN0aW9uIGx0KGEsIGIsIGxvb3NlKSB7XG4gIHJldHVybiBjb21wYXJlKGEsIGIsIGxvb3NlKSA8IDA7XG59XG5cbmV4cG9ydHMuZXEgPSBlcTtcbmZ1bmN0aW9uIGVxKGEsIGIsIGxvb3NlKSB7XG4gIHJldHVybiBjb21wYXJlKGEsIGIsIGxvb3NlKSA9PT0gMDtcbn1cblxuZXhwb3J0cy5uZXEgPSBuZXE7XG5mdW5jdGlvbiBuZXEoYSwgYiwgbG9vc2UpIHtcbiAgcmV0dXJuIGNvbXBhcmUoYSwgYiwgbG9vc2UpICE9PSAwO1xufVxuXG5leHBvcnRzLmd0ZSA9IGd0ZTtcbmZ1bmN0aW9uIGd0ZShhLCBiLCBsb29zZSkge1xuICByZXR1cm4gY29tcGFyZShhLCBiLCBsb29zZSkgPj0gMDtcbn1cblxuZXhwb3J0cy5sdGUgPSBsdGU7XG5mdW5jdGlvbiBsdGUoYSwgYiwgbG9vc2UpIHtcbiAgcmV0dXJuIGNvbXBhcmUoYSwgYiwgbG9vc2UpIDw9IDA7XG59XG5cbmV4cG9ydHMuY21wID0gY21wO1xuZnVuY3Rpb24gY21wKGEsIG9wLCBiLCBsb29zZSkge1xuICB2YXIgcmV0O1xuICBzd2l0Y2ggKG9wKSB7XG4gICAgY2FzZSAnPT09JzpcbiAgICAgIGlmICh0eXBlb2YgYSA9PT0gJ29iamVjdCcpIGEgPSBhLnZlcnNpb247XG4gICAgICBpZiAodHlwZW9mIGIgPT09ICdvYmplY3QnKSBiID0gYi52ZXJzaW9uO1xuICAgICAgcmV0ID0gYSA9PT0gYjtcbiAgICAgIGJyZWFrO1xuICAgIGNhc2UgJyE9PSc6XG4gICAgICBpZiAodHlwZW9mIGEgPT09ICdvYmplY3QnKSBhID0gYS52ZXJzaW9uO1xuICAgICAgaWYgKHR5cGVvZiBiID09PSAnb2JqZWN0JykgYiA9IGIudmVyc2lvbjtcbiAgICAgIHJldCA9IGEgIT09IGI7XG4gICAgICBicmVhaztcbiAgICBjYXNlICcnOiBjYXNlICc9JzogY2FzZSAnPT0nOiByZXQgPSBlcShhLCBiLCBsb29zZSk7IGJyZWFrO1xuICAgIGNhc2UgJyE9JzogcmV0ID0gbmVxKGEsIGIsIGxvb3NlKTsgYnJlYWs7XG4gICAgY2FzZSAnPic6IHJldCA9IGd0KGEsIGIsIGxvb3NlKTsgYnJlYWs7XG4gICAgY2FzZSAnPj0nOiByZXQgPSBndGUoYSwgYiwgbG9vc2UpOyBicmVhaztcbiAgICBjYXNlICc8JzogcmV0ID0gbHQoYSwgYiwgbG9vc2UpOyBicmVhaztcbiAgICBjYXNlICc8PSc6IHJldCA9IGx0ZShhLCBiLCBsb29zZSk7IGJyZWFrO1xuICAgIGRlZmF1bHQ6IHRocm93IG5ldyBUeXBlRXJyb3IoJ0ludmFsaWQgb3BlcmF0b3I6ICcgKyBvcCk7XG4gIH1cbiAgcmV0dXJuIHJldDtcbn1cblxuZXhwb3J0cy5Db21wYXJhdG9yID0gQ29tcGFyYXRvcjtcbmZ1bmN0aW9uIENvbXBhcmF0b3IoY29tcCwgbG9vc2UpIHtcbiAgaWYgKGNvbXAgaW5zdGFuY2VvZiBDb21wYXJhdG9yKSB7XG4gICAgaWYgKGNvbXAubG9vc2UgPT09IGxvb3NlKVxuICAgICAgcmV0dXJuIGNvbXA7XG4gICAgZWxzZVxuICAgICAgY29tcCA9IGNvbXAudmFsdWU7XG4gIH1cblxuICBpZiAoISh0aGlzIGluc3RhbmNlb2YgQ29tcGFyYXRvcikpXG4gICAgcmV0dXJuIG5ldyBDb21wYXJhdG9yKGNvbXAsIGxvb3NlKTtcblxuICBkZWJ1ZygnY29tcGFyYXRvcicsIGNvbXAsIGxvb3NlKTtcbiAgdGhpcy5sb29zZSA9IGxvb3NlO1xuICB0aGlzLnBhcnNlKGNvbXApO1xuXG4gIGlmICh0aGlzLnNlbXZlciA9PT0gQU5ZKVxuICAgIHRoaXMudmFsdWUgPSAnJztcbiAgZWxzZVxuICAgIHRoaXMudmFsdWUgPSB0aGlzLm9wZXJhdG9yICsgdGhpcy5zZW12ZXIudmVyc2lvbjtcblxuICBkZWJ1ZygnY29tcCcsIHRoaXMpO1xufVxuXG52YXIgQU5ZID0ge307XG5Db21wYXJhdG9yLnByb3RvdHlwZS5wYXJzZSA9IGZ1bmN0aW9uKGNvbXApIHtcbiAgdmFyIHIgPSB0aGlzLmxvb3NlID8gcmVbQ09NUEFSQVRPUkxPT1NFXSA6IHJlW0NPTVBBUkFUT1JdO1xuICB2YXIgbSA9IGNvbXAubWF0Y2gocik7XG5cbiAgaWYgKCFtKVxuICAgIHRocm93IG5ldyBUeXBlRXJyb3IoJ0ludmFsaWQgY29tcGFyYXRvcjogJyArIGNvbXApO1xuXG4gIHRoaXMub3BlcmF0b3IgPSBtWzFdO1xuICBpZiAodGhpcy5vcGVyYXRvciA9PT0gJz0nKVxuICAgIHRoaXMub3BlcmF0b3IgPSAnJztcblxuICAvLyBpZiBpdCBsaXRlcmFsbHkgaXMganVzdCAnPicgb3IgJycgdGhlbiBhbGxvdyBhbnl0aGluZy5cbiAgaWYgKCFtWzJdKVxuICAgIHRoaXMuc2VtdmVyID0gQU5ZO1xuICBlbHNlXG4gICAgdGhpcy5zZW12ZXIgPSBuZXcgU2VtVmVyKG1bMl0sIHRoaXMubG9vc2UpO1xufTtcblxuQ29tcGFyYXRvci5wcm90b3R5cGUudG9TdHJpbmcgPSBmdW5jdGlvbigpIHtcbiAgcmV0dXJuIHRoaXMudmFsdWU7XG59O1xuXG5Db21wYXJhdG9yLnByb3RvdHlwZS50ZXN0ID0gZnVuY3Rpb24odmVyc2lvbikge1xuICBkZWJ1ZygnQ29tcGFyYXRvci50ZXN0JywgdmVyc2lvbiwgdGhpcy5sb29zZSk7XG5cbiAgaWYgKHRoaXMuc2VtdmVyID09PSBBTlkpXG4gICAgcmV0dXJuIHRydWU7XG5cbiAgaWYgKHR5cGVvZiB2ZXJzaW9uID09PSAnc3RyaW5nJylcbiAgICB2ZXJzaW9uID0gbmV3IFNlbVZlcih2ZXJzaW9uLCB0aGlzLmxvb3NlKTtcblxuICByZXR1cm4gY21wKHZlcnNpb24sIHRoaXMub3BlcmF0b3IsIHRoaXMuc2VtdmVyLCB0aGlzLmxvb3NlKTtcbn07XG5cblxuZXhwb3J0cy5SYW5nZSA9IFJhbmdlO1xuZnVuY3Rpb24gUmFuZ2UocmFuZ2UsIGxvb3NlKSB7XG4gIGlmICgocmFuZ2UgaW5zdGFuY2VvZiBSYW5nZSkgJiYgcmFuZ2UubG9vc2UgPT09IGxvb3NlKVxuICAgIHJldHVybiByYW5nZTtcblxuICBpZiAoISh0aGlzIGluc3RhbmNlb2YgUmFuZ2UpKVxuICAgIHJldHVybiBuZXcgUmFuZ2UocmFuZ2UsIGxvb3NlKTtcblxuICB0aGlzLmxvb3NlID0gbG9vc2U7XG5cbiAgLy8gRmlyc3QsIHNwbGl0IGJhc2VkIG9uIGJvb2xlYW4gb3IgfHxcbiAgdGhpcy5yYXcgPSByYW5nZTtcbiAgdGhpcy5zZXQgPSByYW5nZS5zcGxpdCgvXFxzKlxcfFxcfFxccyovKS5tYXAoZnVuY3Rpb24ocmFuZ2UpIHtcbiAgICByZXR1cm4gdGhpcy5wYXJzZVJhbmdlKHJhbmdlLnRyaW0oKSk7XG4gIH0sIHRoaXMpLmZpbHRlcihmdW5jdGlvbihjKSB7XG4gICAgLy8gdGhyb3cgb3V0IGFueSB0aGF0IGFyZSBub3QgcmVsZXZhbnQgZm9yIHdoYXRldmVyIHJlYXNvblxuICAgIHJldHVybiBjLmxlbmd0aDtcbiAgfSk7XG5cbiAgaWYgKCF0aGlzLnNldC5sZW5ndGgpIHtcbiAgICB0aHJvdyBuZXcgVHlwZUVycm9yKCdJbnZhbGlkIFNlbVZlciBSYW5nZTogJyArIHJhbmdlKTtcbiAgfVxuXG4gIHRoaXMuZm9ybWF0KCk7XG59XG5cblJhbmdlLnByb3RvdHlwZS5mb3JtYXQgPSBmdW5jdGlvbigpIHtcbiAgdGhpcy5yYW5nZSA9IHRoaXMuc2V0Lm1hcChmdW5jdGlvbihjb21wcykge1xuICAgIHJldHVybiBjb21wcy5qb2luKCcgJykudHJpbSgpO1xuICB9KS5qb2luKCd8fCcpLnRyaW0oKTtcbiAgcmV0dXJuIHRoaXMucmFuZ2U7XG59O1xuXG5SYW5nZS5wcm90b3R5cGUudG9TdHJpbmcgPSBmdW5jdGlvbigpIHtcbiAgcmV0dXJuIHRoaXMucmFuZ2U7XG59O1xuXG5SYW5nZS5wcm90b3R5cGUucGFyc2VSYW5nZSA9IGZ1bmN0aW9uKHJhbmdlKSB7XG4gIHZhciBsb29zZSA9IHRoaXMubG9vc2U7XG4gIHJhbmdlID0gcmFuZ2UudHJpbSgpO1xuICBkZWJ1ZygncmFuZ2UnLCByYW5nZSwgbG9vc2UpO1xuICAvLyBgMS4yLjMgLSAxLjIuNGAgPT4gYD49MS4yLjMgPD0xLjIuNGBcbiAgdmFyIGhyID0gbG9vc2UgPyByZVtIWVBIRU5SQU5HRUxPT1NFXSA6IHJlW0hZUEhFTlJBTkdFXTtcbiAgcmFuZ2UgPSByYW5nZS5yZXBsYWNlKGhyLCBoeXBoZW5SZXBsYWNlKTtcbiAgZGVidWcoJ2h5cGhlbiByZXBsYWNlJywgcmFuZ2UpO1xuICAvLyBgPiAxLjIuMyA8IDEuMi41YCA9PiBgPjEuMi4zIDwxLjIuNWBcbiAgcmFuZ2UgPSByYW5nZS5yZXBsYWNlKHJlW0NPTVBBUkFUT1JUUklNXSwgY29tcGFyYXRvclRyaW1SZXBsYWNlKTtcbiAgZGVidWcoJ2NvbXBhcmF0b3IgdHJpbScsIHJhbmdlLCByZVtDT01QQVJBVE9SVFJJTV0pO1xuXG4gIC8vIGB+IDEuMi4zYCA9PiBgfjEuMi4zYFxuICByYW5nZSA9IHJhbmdlLnJlcGxhY2UocmVbVElMREVUUklNXSwgdGlsZGVUcmltUmVwbGFjZSk7XG5cbiAgLy8gYF4gMS4yLjNgID0+IGBeMS4yLjNgXG4gIHJhbmdlID0gcmFuZ2UucmVwbGFjZShyZVtDQVJFVFRSSU1dLCBjYXJldFRyaW1SZXBsYWNlKTtcblxuICAvLyBub3JtYWxpemUgc3BhY2VzXG4gIHJhbmdlID0gcmFuZ2Uuc3BsaXQoL1xccysvKS5qb2luKCcgJyk7XG5cbiAgLy8gQXQgdGhpcyBwb2ludCwgdGhlIHJhbmdlIGlzIGNvbXBsZXRlbHkgdHJpbW1lZCBhbmRcbiAgLy8gcmVhZHkgdG8gYmUgc3BsaXQgaW50byBjb21wYXJhdG9ycy5cblxuICB2YXIgY29tcFJlID0gbG9vc2UgPyByZVtDT01QQVJBVE9STE9PU0VdIDogcmVbQ09NUEFSQVRPUl07XG4gIHZhciBzZXQgPSByYW5nZS5zcGxpdCgnICcpLm1hcChmdW5jdGlvbihjb21wKSB7XG4gICAgcmV0dXJuIHBhcnNlQ29tcGFyYXRvcihjb21wLCBsb29zZSk7XG4gIH0pLmpvaW4oJyAnKS5zcGxpdCgvXFxzKy8pO1xuICBpZiAodGhpcy5sb29zZSkge1xuICAgIC8vIGluIGxvb3NlIG1vZGUsIHRocm93IG91dCBhbnkgdGhhdCBhcmUgbm90IHZhbGlkIGNvbXBhcmF0b3JzXG4gICAgc2V0ID0gc2V0LmZpbHRlcihmdW5jdGlvbihjb21wKSB7XG4gICAgICByZXR1cm4gISFjb21wLm1hdGNoKGNvbXBSZSk7XG4gICAgfSk7XG4gIH1cbiAgc2V0ID0gc2V0Lm1hcChmdW5jdGlvbihjb21wKSB7XG4gICAgcmV0dXJuIG5ldyBDb21wYXJhdG9yKGNvbXAsIGxvb3NlKTtcbiAgfSk7XG5cbiAgcmV0dXJuIHNldDtcbn07XG5cbi8vIE1vc3RseSBqdXN0IGZvciB0ZXN0aW5nIGFuZCBsZWdhY3kgQVBJIHJlYXNvbnNcbmV4cG9ydHMudG9Db21wYXJhdG9ycyA9IHRvQ29tcGFyYXRvcnM7XG5mdW5jdGlvbiB0b0NvbXBhcmF0b3JzKHJhbmdlLCBsb29zZSkge1xuICByZXR1cm4gbmV3IFJhbmdlKHJhbmdlLCBsb29zZSkuc2V0Lm1hcChmdW5jdGlvbihjb21wKSB7XG4gICAgcmV0dXJuIGNvbXAubWFwKGZ1bmN0aW9uKGMpIHtcbiAgICAgIHJldHVybiBjLnZhbHVlO1xuICAgIH0pLmpvaW4oJyAnKS50cmltKCkuc3BsaXQoJyAnKTtcbiAgfSk7XG59XG5cbi8vIGNvbXByaXNlZCBvZiB4cmFuZ2VzLCB0aWxkZXMsIHN0YXJzLCBhbmQgZ3RsdCdzIGF0IHRoaXMgcG9pbnQuXG4vLyBhbHJlYWR5IHJlcGxhY2VkIHRoZSBoeXBoZW4gcmFuZ2VzXG4vLyB0dXJuIGludG8gYSBzZXQgb2YgSlVTVCBjb21wYXJhdG9ycy5cbmZ1bmN0aW9uIHBhcnNlQ29tcGFyYXRvcihjb21wLCBsb29zZSkge1xuICBkZWJ1ZygnY29tcCcsIGNvbXApO1xuICBjb21wID0gcmVwbGFjZUNhcmV0cyhjb21wLCBsb29zZSk7XG4gIGRlYnVnKCdjYXJldCcsIGNvbXApO1xuICBjb21wID0gcmVwbGFjZVRpbGRlcyhjb21wLCBsb29zZSk7XG4gIGRlYnVnKCd0aWxkZXMnLCBjb21wKTtcbiAgY29tcCA9IHJlcGxhY2VYUmFuZ2VzKGNvbXAsIGxvb3NlKTtcbiAgZGVidWcoJ3hyYW5nZScsIGNvbXApO1xuICBjb21wID0gcmVwbGFjZVN0YXJzKGNvbXAsIGxvb3NlKTtcbiAgZGVidWcoJ3N0YXJzJywgY29tcCk7XG4gIHJldHVybiBjb21wO1xufVxuXG5mdW5jdGlvbiBpc1goaWQpIHtcbiAgcmV0dXJuICFpZCB8fCBpZC50b0xvd2VyQ2FzZSgpID09PSAneCcgfHwgaWQgPT09ICcqJztcbn1cblxuLy8gfiwgfj4gLS0+ICogKGFueSwga2luZGEgc2lsbHkpXG4vLyB+MiwgfjIueCwgfjIueC54LCB+PjIsIH4+Mi54IH4+Mi54LnggLS0+ID49Mi4wLjAgPDMuMC4wXG4vLyB+Mi4wLCB+Mi4wLngsIH4+Mi4wLCB+PjIuMC54IC0tPiA+PTIuMC4wIDwyLjEuMFxuLy8gfjEuMiwgfjEuMi54LCB+PjEuMiwgfj4xLjIueCAtLT4gPj0xLjIuMCA8MS4zLjBcbi8vIH4xLjIuMywgfj4xLjIuMyAtLT4gPj0xLjIuMyA8MS4zLjBcbi8vIH4xLjIuMCwgfj4xLjIuMCAtLT4gPj0xLjIuMCA8MS4zLjBcbmZ1bmN0aW9uIHJlcGxhY2VUaWxkZXMoY29tcCwgbG9vc2UpIHtcbiAgcmV0dXJuIGNvbXAudHJpbSgpLnNwbGl0KC9cXHMrLykubWFwKGZ1bmN0aW9uKGNvbXApIHtcbiAgICByZXR1cm4gcmVwbGFjZVRpbGRlKGNvbXAsIGxvb3NlKTtcbiAgfSkuam9pbignICcpO1xufVxuXG5mdW5jdGlvbiByZXBsYWNlVGlsZGUoY29tcCwgbG9vc2UpIHtcbiAgdmFyIHIgPSBsb29zZSA/IHJlW1RJTERFTE9PU0VdIDogcmVbVElMREVdO1xuICByZXR1cm4gY29tcC5yZXBsYWNlKHIsIGZ1bmN0aW9uKF8sIE0sIG0sIHAsIHByKSB7XG4gICAgZGVidWcoJ3RpbGRlJywgY29tcCwgXywgTSwgbSwgcCwgcHIpO1xuICAgIHZhciByZXQ7XG5cbiAgICBpZiAoaXNYKE0pKVxuICAgICAgcmV0ID0gJyc7XG4gICAgZWxzZSBpZiAoaXNYKG0pKVxuICAgICAgcmV0ID0gJz49JyArIE0gKyAnLjAuMCA8JyArICgrTSArIDEpICsgJy4wLjAnO1xuICAgIGVsc2UgaWYgKGlzWChwKSlcbiAgICAgIC8vIH4xLjIgPT0gPj0xLjIuMCA8MS4zLjBcbiAgICAgIHJldCA9ICc+PScgKyBNICsgJy4nICsgbSArICcuMCA8JyArIE0gKyAnLicgKyAoK20gKyAxKSArICcuMCc7XG4gICAgZWxzZSBpZiAocHIpIHtcbiAgICAgIGRlYnVnKCdyZXBsYWNlVGlsZGUgcHInLCBwcik7XG4gICAgICBpZiAocHIuY2hhckF0KDApICE9PSAnLScpXG4gICAgICAgIHByID0gJy0nICsgcHI7XG4gICAgICByZXQgPSAnPj0nICsgTSArICcuJyArIG0gKyAnLicgKyBwICsgcHIgK1xuICAgICAgICAgICAgJyA8JyArIE0gKyAnLicgKyAoK20gKyAxKSArICcuMCc7XG4gICAgfSBlbHNlXG4gICAgICAvLyB+MS4yLjMgPT0gPj0xLjIuMyA8MS4zLjBcbiAgICAgIHJldCA9ICc+PScgKyBNICsgJy4nICsgbSArICcuJyArIHAgK1xuICAgICAgICAgICAgJyA8JyArIE0gKyAnLicgKyAoK20gKyAxKSArICcuMCc7XG5cbiAgICBkZWJ1ZygndGlsZGUgcmV0dXJuJywgcmV0KTtcbiAgICByZXR1cm4gcmV0O1xuICB9KTtcbn1cblxuLy8gXiAtLT4gKiAoYW55LCBraW5kYSBzaWxseSlcbi8vIF4yLCBeMi54LCBeMi54LnggLS0+ID49Mi4wLjAgPDMuMC4wXG4vLyBeMi4wLCBeMi4wLnggLS0+ID49Mi4wLjAgPDMuMC4wXG4vLyBeMS4yLCBeMS4yLnggLS0+ID49MS4yLjAgPDIuMC4wXG4vLyBeMS4yLjMgLS0+ID49MS4yLjMgPDIuMC4wXG4vLyBeMS4yLjAgLS0+ID49MS4yLjAgPDIuMC4wXG5mdW5jdGlvbiByZXBsYWNlQ2FyZXRzKGNvbXAsIGxvb3NlKSB7XG4gIHJldHVybiBjb21wLnRyaW0oKS5zcGxpdCgvXFxzKy8pLm1hcChmdW5jdGlvbihjb21wKSB7XG4gICAgcmV0dXJuIHJlcGxhY2VDYXJldChjb21wLCBsb29zZSk7XG4gIH0pLmpvaW4oJyAnKTtcbn1cblxuZnVuY3Rpb24gcmVwbGFjZUNhcmV0KGNvbXAsIGxvb3NlKSB7XG4gIGRlYnVnKCdjYXJldCcsIGNvbXAsIGxvb3NlKTtcbiAgdmFyIHIgPSBsb29zZSA/IHJlW0NBUkVUTE9PU0VdIDogcmVbQ0FSRVRdO1xuICByZXR1cm4gY29tcC5yZXBsYWNlKHIsIGZ1bmN0aW9uKF8sIE0sIG0sIHAsIHByKSB7XG4gICAgZGVidWcoJ2NhcmV0JywgY29tcCwgXywgTSwgbSwgcCwgcHIpO1xuICAgIHZhciByZXQ7XG5cbiAgICBpZiAoaXNYKE0pKVxuICAgICAgcmV0ID0gJyc7XG4gICAgZWxzZSBpZiAoaXNYKG0pKVxuICAgICAgcmV0ID0gJz49JyArIE0gKyAnLjAuMCA8JyArICgrTSArIDEpICsgJy4wLjAnO1xuICAgIGVsc2UgaWYgKGlzWChwKSkge1xuICAgICAgaWYgKE0gPT09ICcwJylcbiAgICAgICAgcmV0ID0gJz49JyArIE0gKyAnLicgKyBtICsgJy4wIDwnICsgTSArICcuJyArICgrbSArIDEpICsgJy4wJztcbiAgICAgIGVsc2VcbiAgICAgICAgcmV0ID0gJz49JyArIE0gKyAnLicgKyBtICsgJy4wIDwnICsgKCtNICsgMSkgKyAnLjAuMCc7XG4gICAgfSBlbHNlIGlmIChwcikge1xuICAgICAgZGVidWcoJ3JlcGxhY2VDYXJldCBwcicsIHByKTtcbiAgICAgIGlmIChwci5jaGFyQXQoMCkgIT09ICctJylcbiAgICAgICAgcHIgPSAnLScgKyBwcjtcbiAgICAgIGlmIChNID09PSAnMCcpIHtcbiAgICAgICAgaWYgKG0gPT09ICcwJylcbiAgICAgICAgICByZXQgPSAnPj0nICsgTSArICcuJyArIG0gKyAnLicgKyBwICsgcHIgK1xuICAgICAgICAgICAgICAgICcgPCcgKyBNICsgJy4nICsgbSArICcuJyArICgrcCArIDEpO1xuICAgICAgICBlbHNlXG4gICAgICAgICAgcmV0ID0gJz49JyArIE0gKyAnLicgKyBtICsgJy4nICsgcCArIHByICtcbiAgICAgICAgICAgICAgICAnIDwnICsgTSArICcuJyArICgrbSArIDEpICsgJy4wJztcbiAgICAgIH0gZWxzZVxuICAgICAgICByZXQgPSAnPj0nICsgTSArICcuJyArIG0gKyAnLicgKyBwICsgcHIgK1xuICAgICAgICAgICAgICAnIDwnICsgKCtNICsgMSkgKyAnLjAuMCc7XG4gICAgfSBlbHNlIHtcbiAgICAgIGRlYnVnKCdubyBwcicpO1xuICAgICAgaWYgKE0gPT09ICcwJykge1xuICAgICAgICBpZiAobSA9PT0gJzAnKVxuICAgICAgICAgIHJldCA9ICc+PScgKyBNICsgJy4nICsgbSArICcuJyArIHAgK1xuICAgICAgICAgICAgICAgICcgPCcgKyBNICsgJy4nICsgbSArICcuJyArICgrcCArIDEpO1xuICAgICAgICBlbHNlXG4gICAgICAgICAgcmV0ID0gJz49JyArIE0gKyAnLicgKyBtICsgJy4nICsgcCArXG4gICAgICAgICAgICAgICAgJyA8JyArIE0gKyAnLicgKyAoK20gKyAxKSArICcuMCc7XG4gICAgICB9IGVsc2VcbiAgICAgICAgcmV0ID0gJz49JyArIE0gKyAnLicgKyBtICsgJy4nICsgcCArXG4gICAgICAgICAgICAgICcgPCcgKyAoK00gKyAxKSArICcuMC4wJztcbiAgICB9XG5cbiAgICBkZWJ1ZygnY2FyZXQgcmV0dXJuJywgcmV0KTtcbiAgICByZXR1cm4gcmV0O1xuICB9KTtcbn1cblxuZnVuY3Rpb24gcmVwbGFjZVhSYW5nZXMoY29tcCwgbG9vc2UpIHtcbiAgZGVidWcoJ3JlcGxhY2VYUmFuZ2VzJywgY29tcCwgbG9vc2UpO1xuICByZXR1cm4gY29tcC5zcGxpdCgvXFxzKy8pLm1hcChmdW5jdGlvbihjb21wKSB7XG4gICAgcmV0dXJuIHJlcGxhY2VYUmFuZ2UoY29tcCwgbG9vc2UpO1xuICB9KS5qb2luKCcgJyk7XG59XG5cbmZ1bmN0aW9uIHJlcGxhY2VYUmFuZ2UoY29tcCwgbG9vc2UpIHtcbiAgY29tcCA9IGNvbXAudHJpbSgpO1xuICB2YXIgciA9IGxvb3NlID8gcmVbWFJBTkdFTE9PU0VdIDogcmVbWFJBTkdFXTtcbiAgcmV0dXJuIGNvbXAucmVwbGFjZShyLCBmdW5jdGlvbihyZXQsIGd0bHQsIE0sIG0sIHAsIHByKSB7XG4gICAgZGVidWcoJ3hSYW5nZScsIGNvbXAsIHJldCwgZ3RsdCwgTSwgbSwgcCwgcHIpO1xuICAgIHZhciB4TSA9IGlzWChNKTtcbiAgICB2YXIgeG0gPSB4TSB8fCBpc1gobSk7XG4gICAgdmFyIHhwID0geG0gfHwgaXNYKHApO1xuICAgIHZhciBhbnlYID0geHA7XG5cbiAgICBpZiAoZ3RsdCA9PT0gJz0nICYmIGFueVgpXG4gICAgICBndGx0ID0gJyc7XG5cbiAgICBpZiAoeE0pIHtcbiAgICAgIGlmIChndGx0ID09PSAnPicgfHwgZ3RsdCA9PT0gJzwnKSB7XG4gICAgICAgIC8vIG5vdGhpbmcgaXMgYWxsb3dlZFxuICAgICAgICByZXQgPSAnPDAuMC4wJztcbiAgICAgIH0gZWxzZSB7XG4gICAgICAgIC8vIG5vdGhpbmcgaXMgZm9yYmlkZGVuXG4gICAgICAgIHJldCA9ICcqJztcbiAgICAgIH1cbiAgICB9IGVsc2UgaWYgKGd0bHQgJiYgYW55WCkge1xuICAgICAgLy8gcmVwbGFjZSBYIHdpdGggMFxuICAgICAgaWYgKHhtKVxuICAgICAgICBtID0gMDtcbiAgICAgIGlmICh4cClcbiAgICAgICAgcCA9IDA7XG5cbiAgICAgIGlmIChndGx0ID09PSAnPicpIHtcbiAgICAgICAgLy8gPjEgPT4gPj0yLjAuMFxuICAgICAgICAvLyA+MS4yID0+ID49MS4zLjBcbiAgICAgICAgLy8gPjEuMi4zID0+ID49IDEuMi40XG4gICAgICAgIGd0bHQgPSAnPj0nO1xuICAgICAgICBpZiAoeG0pIHtcbiAgICAgICAgICBNID0gK00gKyAxO1xuICAgICAgICAgIG0gPSAwO1xuICAgICAgICAgIHAgPSAwO1xuICAgICAgICB9IGVsc2UgaWYgKHhwKSB7XG4gICAgICAgICAgbSA9ICttICsgMTtcbiAgICAgICAgICBwID0gMDtcbiAgICAgICAgfVxuICAgICAgfSBlbHNlIGlmIChndGx0ID09PSAnPD0nKSB7XG4gICAgICAgIC8vIDw9MC43LnggaXMgYWN0dWFsbHkgPDAuOC4wLCBzaW5jZSBhbnkgMC43Lnggc2hvdWxkXG4gICAgICAgIC8vIHBhc3MuICBTaW1pbGFybHksIDw9Ny54IGlzIGFjdHVhbGx5IDw4LjAuMCwgZXRjLlxuICAgICAgICBndGx0ID0gJzwnO1xuICAgICAgICBpZiAoeG0pXG4gICAgICAgICAgTSA9ICtNICsgMTtcbiAgICAgICAgZWxzZVxuICAgICAgICAgIG0gPSArbSArIDE7XG4gICAgICB9XG5cbiAgICAgIHJldCA9IGd0bHQgKyBNICsgJy4nICsgbSArICcuJyArIHA7XG4gICAgfSBlbHNlIGlmICh4bSkge1xuICAgICAgcmV0ID0gJz49JyArIE0gKyAnLjAuMCA8JyArICgrTSArIDEpICsgJy4wLjAnO1xuICAgIH0gZWxzZSBpZiAoeHApIHtcbiAgICAgIHJldCA9ICc+PScgKyBNICsgJy4nICsgbSArICcuMCA8JyArIE0gKyAnLicgKyAoK20gKyAxKSArICcuMCc7XG4gICAgfVxuXG4gICAgZGVidWcoJ3hSYW5nZSByZXR1cm4nLCByZXQpO1xuXG4gICAgcmV0dXJuIHJldDtcbiAgfSk7XG59XG5cbi8vIEJlY2F1c2UgKiBpcyBBTkQtZWQgd2l0aCBldmVyeXRoaW5nIGVsc2UgaW4gdGhlIGNvbXBhcmF0b3IsXG4vLyBhbmQgJycgbWVhbnMgXCJhbnkgdmVyc2lvblwiLCBqdXN0IHJlbW92ZSB0aGUgKnMgZW50aXJlbHkuXG5mdW5jdGlvbiByZXBsYWNlU3RhcnMoY29tcCwgbG9vc2UpIHtcbiAgZGVidWcoJ3JlcGxhY2VTdGFycycsIGNvbXAsIGxvb3NlKTtcbiAgLy8gTG9vc2VuZXNzIGlzIGlnbm9yZWQgaGVyZS4gIHN0YXIgaXMgYWx3YXlzIGFzIGxvb3NlIGFzIGl0IGdldHMhXG4gIHJldHVybiBjb21wLnRyaW0oKS5yZXBsYWNlKHJlW1NUQVJdLCAnJyk7XG59XG5cbi8vIFRoaXMgZnVuY3Rpb24gaXMgcGFzc2VkIHRvIHN0cmluZy5yZXBsYWNlKHJlW0hZUEhFTlJBTkdFXSlcbi8vIE0sIG0sIHBhdGNoLCBwcmVyZWxlYXNlLCBidWlsZFxuLy8gMS4yIC0gMy40LjUgPT4gPj0xLjIuMCA8PTMuNC41XG4vLyAxLjIuMyAtIDMuNCA9PiA+PTEuMi4wIDwzLjUuMCBBbnkgMy40Lnggd2lsbCBkb1xuLy8gMS4yIC0gMy40ID0+ID49MS4yLjAgPDMuNS4wXG5mdW5jdGlvbiBoeXBoZW5SZXBsYWNlKCQwLFxuICAgICAgICAgICAgICAgICAgICAgICBmcm9tLCBmTSwgZm0sIGZwLCBmcHIsIGZiLFxuICAgICAgICAgICAgICAgICAgICAgICB0bywgdE0sIHRtLCB0cCwgdHByLCB0Yikge1xuXG4gIGlmIChpc1goZk0pKVxuICAgIGZyb20gPSAnJztcbiAgZWxzZSBpZiAoaXNYKGZtKSlcbiAgICBmcm9tID0gJz49JyArIGZNICsgJy4wLjAnO1xuICBlbHNlIGlmIChpc1goZnApKVxuICAgIGZyb20gPSAnPj0nICsgZk0gKyAnLicgKyBmbSArICcuMCc7XG4gIGVsc2VcbiAgICBmcm9tID0gJz49JyArIGZyb207XG5cbiAgaWYgKGlzWCh0TSkpXG4gICAgdG8gPSAnJztcbiAgZWxzZSBpZiAoaXNYKHRtKSlcbiAgICB0byA9ICc8JyArICgrdE0gKyAxKSArICcuMC4wJztcbiAgZWxzZSBpZiAoaXNYKHRwKSlcbiAgICB0byA9ICc8JyArIHRNICsgJy4nICsgKCt0bSArIDEpICsgJy4wJztcbiAgZWxzZSBpZiAodHByKVxuICAgIHRvID0gJzw9JyArIHRNICsgJy4nICsgdG0gKyAnLicgKyB0cCArICctJyArIHRwcjtcbiAgZWxzZVxuICAgIHRvID0gJzw9JyArIHRvO1xuXG4gIHJldHVybiAoZnJvbSArICcgJyArIHRvKS50cmltKCk7XG59XG5cblxuLy8gaWYgQU5ZIG9mIHRoZSBzZXRzIG1hdGNoIEFMTCBvZiBpdHMgY29tcGFyYXRvcnMsIHRoZW4gcGFzc1xuUmFuZ2UucHJvdG90eXBlLnRlc3QgPSBmdW5jdGlvbih2ZXJzaW9uKSB7XG4gIGlmICghdmVyc2lvbilcbiAgICByZXR1cm4gZmFsc2U7XG5cbiAgaWYgKHR5cGVvZiB2ZXJzaW9uID09PSAnc3RyaW5nJylcbiAgICB2ZXJzaW9uID0gbmV3IFNlbVZlcih2ZXJzaW9uLCB0aGlzLmxvb3NlKTtcblxuICBmb3IgKHZhciBpID0gMDsgaSA8IHRoaXMuc2V0Lmxlbmd0aDsgaSsrKSB7XG4gICAgaWYgKHRlc3RTZXQodGhpcy5zZXRbaV0sIHZlcnNpb24pKVxuICAgICAgcmV0dXJuIHRydWU7XG4gIH1cbiAgcmV0dXJuIGZhbHNlO1xufTtcblxuZnVuY3Rpb24gdGVzdFNldChzZXQsIHZlcnNpb24pIHtcbiAgZm9yICh2YXIgaSA9IDA7IGkgPCBzZXQubGVuZ3RoOyBpKyspIHtcbiAgICBpZiAoIXNldFtpXS50ZXN0KHZlcnNpb24pKVxuICAgICAgcmV0dXJuIGZhbHNlO1xuICB9XG5cbiAgaWYgKHZlcnNpb24ucHJlcmVsZWFzZS5sZW5ndGgpIHtcbiAgICAvLyBGaW5kIHRoZSBzZXQgb2YgdmVyc2lvbnMgdGhhdCBhcmUgYWxsb3dlZCB0byBoYXZlIHByZXJlbGVhc2VzXG4gICAgLy8gRm9yIGV4YW1wbGUsIF4xLjIuMy1wci4xIGRlc3VnYXJzIHRvID49MS4yLjMtcHIuMSA8Mi4wLjBcbiAgICAvLyBUaGF0IHNob3VsZCBhbGxvdyBgMS4yLjMtcHIuMmAgdG8gcGFzcy5cbiAgICAvLyBIb3dldmVyLCBgMS4yLjQtYWxwaGEubm90cmVhZHlgIHNob3VsZCBOT1QgYmUgYWxsb3dlZCxcbiAgICAvLyBldmVuIHRob3VnaCBpdCdzIHdpdGhpbiB0aGUgcmFuZ2Ugc2V0IGJ5IHRoZSBjb21wYXJhdG9ycy5cbiAgICBmb3IgKHZhciBpID0gMDsgaSA8IHNldC5sZW5ndGg7IGkrKykge1xuICAgICAgZGVidWcoc2V0W2ldLnNlbXZlcik7XG4gICAgICBpZiAoc2V0W2ldLnNlbXZlciA9PT0gQU5ZKVxuICAgICAgICBjb250aW51ZTtcblxuICAgICAgaWYgKHNldFtpXS5zZW12ZXIucHJlcmVsZWFzZS5sZW5ndGggPiAwKSB7XG4gICAgICAgIHZhciBhbGxvd2VkID0gc2V0W2ldLnNlbXZlcjtcbiAgICAgICAgaWYgKGFsbG93ZWQubWFqb3IgPT09IHZlcnNpb24ubWFqb3IgJiZcbiAgICAgICAgICAgIGFsbG93ZWQubWlub3IgPT09IHZlcnNpb24ubWlub3IgJiZcbiAgICAgICAgICAgIGFsbG93ZWQucGF0Y2ggPT09IHZlcnNpb24ucGF0Y2gpXG4gICAgICAgICAgcmV0dXJuIHRydWU7XG4gICAgICB9XG4gICAgfVxuXG4gICAgLy8gVmVyc2lvbiBoYXMgYSAtcHJlLCBidXQgaXQncyBub3Qgb25lIG9mIHRoZSBvbmVzIHdlIGxpa2UuXG4gICAgcmV0dXJuIGZhbHNlO1xuICB9XG5cbiAgcmV0dXJuIHRydWU7XG59XG5cbmV4cG9ydHMuc2F0aXNmaWVzID0gc2F0aXNmaWVzO1xuZnVuY3Rpb24gc2F0aXNmaWVzKHZlcnNpb24sIHJhbmdlLCBsb29zZSkge1xuICB0cnkge1xuICAgIHJhbmdlID0gbmV3IFJhbmdlKHJhbmdlLCBsb29zZSk7XG4gIH0gY2F0Y2ggKGVyKSB7XG4gICAgcmV0dXJuIGZhbHNlO1xuICB9XG4gIHJldHVybiByYW5nZS50ZXN0KHZlcnNpb24pO1xufVxuXG5leHBvcnRzLm1heFNhdGlzZnlpbmcgPSBtYXhTYXRpc2Z5aW5nO1xuZnVuY3Rpb24gbWF4U2F0aXNmeWluZyh2ZXJzaW9ucywgcmFuZ2UsIGxvb3NlKSB7XG4gIHJldHVybiB2ZXJzaW9ucy5maWx0ZXIoZnVuY3Rpb24odmVyc2lvbikge1xuICAgIHJldHVybiBzYXRpc2ZpZXModmVyc2lvbiwgcmFuZ2UsIGxvb3NlKTtcbiAgfSkuc29ydChmdW5jdGlvbihhLCBiKSB7XG4gICAgcmV0dXJuIHJjb21wYXJlKGEsIGIsIGxvb3NlKTtcbiAgfSlbMF0gfHwgbnVsbDtcbn1cblxuZXhwb3J0cy5taW5TYXRpc2Z5aW5nID0gbWluU2F0aXNmeWluZztcbmZ1bmN0aW9uIG1pblNhdGlzZnlpbmcodmVyc2lvbnMsIHJhbmdlLCBsb29zZSkge1xuICByZXR1cm4gdmVyc2lvbnMuZmlsdGVyKGZ1bmN0aW9uKHZlcnNpb24pIHtcbiAgICByZXR1cm4gc2F0aXNmaWVzKHZlcnNpb24sIHJhbmdlLCBsb29zZSk7XG4gIH0pLnNvcnQoZnVuY3Rpb24oYSwgYikge1xuICAgIHJldHVybiBjb21wYXJlKGEsIGIsIGxvb3NlKTtcbiAgfSlbMF0gfHwgbnVsbDtcbn1cblxuZXhwb3J0cy52YWxpZFJhbmdlID0gdmFsaWRSYW5nZTtcbmZ1bmN0aW9uIHZhbGlkUmFuZ2UocmFuZ2UsIGxvb3NlKSB7XG4gIHRyeSB7XG4gICAgLy8gUmV0dXJuICcqJyBpbnN0ZWFkIG9mICcnIHNvIHRoYXQgdHJ1dGhpbmVzcyB3b3Jrcy5cbiAgICAvLyBUaGlzIHdpbGwgdGhyb3cgaWYgaXQncyBpbnZhbGlkIGFueXdheVxuICAgIHJldHVybiBuZXcgUmFuZ2UocmFuZ2UsIGxvb3NlKS5yYW5nZSB8fCAnKic7XG4gIH0gY2F0Y2ggKGVyKSB7XG4gICAgcmV0dXJuIG51bGw7XG4gIH1cbn1cblxuLy8gRGV0ZXJtaW5lIGlmIHZlcnNpb24gaXMgbGVzcyB0aGFuIGFsbCB0aGUgdmVyc2lvbnMgcG9zc2libGUgaW4gdGhlIHJhbmdlXG5leHBvcnRzLmx0ciA9IGx0cjtcbmZ1bmN0aW9uIGx0cih2ZXJzaW9uLCByYW5nZSwgbG9vc2UpIHtcbiAgcmV0dXJuIG91dHNpZGUodmVyc2lvbiwgcmFuZ2UsICc8JywgbG9vc2UpO1xufVxuXG4vLyBEZXRlcm1pbmUgaWYgdmVyc2lvbiBpcyBncmVhdGVyIHRoYW4gYWxsIHRoZSB2ZXJzaW9ucyBwb3NzaWJsZSBpbiB0aGUgcmFuZ2UuXG5leHBvcnRzLmd0ciA9IGd0cjtcbmZ1bmN0aW9uIGd0cih2ZXJzaW9uLCByYW5nZSwgbG9vc2UpIHtcbiAgcmV0dXJuIG91dHNpZGUodmVyc2lvbiwgcmFuZ2UsICc+JywgbG9vc2UpO1xufVxuXG5leHBvcnRzLm91dHNpZGUgPSBvdXRzaWRlO1xuZnVuY3Rpb24gb3V0c2lkZSh2ZXJzaW9uLCByYW5nZSwgaGlsbywgbG9vc2UpIHtcbiAgdmVyc2lvbiA9IG5ldyBTZW1WZXIodmVyc2lvbiwgbG9vc2UpO1xuICByYW5nZSA9IG5ldyBSYW5nZShyYW5nZSwgbG9vc2UpO1xuXG4gIHZhciBndGZuLCBsdGVmbiwgbHRmbiwgY29tcCwgZWNvbXA7XG4gIHN3aXRjaCAoaGlsbykge1xuICAgIGNhc2UgJz4nOlxuICAgICAgZ3RmbiA9IGd0O1xuICAgICAgbHRlZm4gPSBsdGU7XG4gICAgICBsdGZuID0gbHQ7XG4gICAgICBjb21wID0gJz4nO1xuICAgICAgZWNvbXAgPSAnPj0nO1xuICAgICAgYnJlYWs7XG4gICAgY2FzZSAnPCc6XG4gICAgICBndGZuID0gbHQ7XG4gICAgICBsdGVmbiA9IGd0ZTtcbiAgICAgIGx0Zm4gPSBndDtcbiAgICAgIGNvbXAgPSAnPCc7XG4gICAgICBlY29tcCA9ICc8PSc7XG4gICAgICBicmVhaztcbiAgICBkZWZhdWx0OlxuICAgICAgdGhyb3cgbmV3IFR5cGVFcnJvcignTXVzdCBwcm92aWRlIGEgaGlsbyB2YWwgb2YgXCI8XCIgb3IgXCI+XCInKTtcbiAgfVxuXG4gIC8vIElmIGl0IHNhdGlzaWZlcyB0aGUgcmFuZ2UgaXQgaXMgbm90IG91dHNpZGVcbiAgaWYgKHNhdGlzZmllcyh2ZXJzaW9uLCByYW5nZSwgbG9vc2UpKSB7XG4gICAgcmV0dXJuIGZhbHNlO1xuICB9XG5cbiAgLy8gRnJvbSBub3cgb24sIHZhcmlhYmxlIHRlcm1zIGFyZSBhcyBpZiB3ZSdyZSBpbiBcImd0clwiIG1vZGUuXG4gIC8vIGJ1dCBub3RlIHRoYXQgZXZlcnl0aGluZyBpcyBmbGlwcGVkIGZvciB0aGUgXCJsdHJcIiBmdW5jdGlvbi5cblxuICBmb3IgKHZhciBpID0gMDsgaSA8IHJhbmdlLnNldC5sZW5ndGg7ICsraSkge1xuICAgIHZhciBjb21wYXJhdG9ycyA9IHJhbmdlLnNldFtpXTtcblxuICAgIHZhciBoaWdoID0gbnVsbDtcbiAgICB2YXIgbG93ID0gbnVsbDtcblxuICAgIGNvbXBhcmF0b3JzLmZvckVhY2goZnVuY3Rpb24oY29tcGFyYXRvcikge1xuICAgICAgaWYgKGNvbXBhcmF0b3Iuc2VtdmVyID09PSBBTlkpIHtcbiAgICAgICAgY29tcGFyYXRvciA9IG5ldyBDb21wYXJhdG9yKCc+PTAuMC4wJylcbiAgICAgIH1cbiAgICAgIGhpZ2ggPSBoaWdoIHx8IGNvbXBhcmF0b3I7XG4gICAgICBsb3cgPSBsb3cgfHwgY29tcGFyYXRvcjtcbiAgICAgIGlmIChndGZuKGNvbXBhcmF0b3Iuc2VtdmVyLCBoaWdoLnNlbXZlciwgbG9vc2UpKSB7XG4gICAgICAgIGhpZ2ggPSBjb21wYXJhdG9yO1xuICAgICAgfSBlbHNlIGlmIChsdGZuKGNvbXBhcmF0b3Iuc2VtdmVyLCBsb3cuc2VtdmVyLCBsb29zZSkpIHtcbiAgICAgICAgbG93ID0gY29tcGFyYXRvcjtcbiAgICAgIH1cbiAgICB9KTtcblxuICAgIC8vIElmIHRoZSBlZGdlIHZlcnNpb24gY29tcGFyYXRvciBoYXMgYSBvcGVyYXRvciB0aGVuIG91ciB2ZXJzaW9uXG4gICAgLy8gaXNuJ3Qgb3V0c2lkZSBpdFxuICAgIGlmIChoaWdoLm9wZXJhdG9yID09PSBjb21wIHx8IGhpZ2gub3BlcmF0b3IgPT09IGVjb21wKSB7XG4gICAgICByZXR1cm4gZmFsc2U7XG4gICAgfVxuXG4gICAgLy8gSWYgdGhlIGxvd2VzdCB2ZXJzaW9uIGNvbXBhcmF0b3IgaGFzIGFuIG9wZXJhdG9yIGFuZCBvdXIgdmVyc2lvblxuICAgIC8vIGlzIGxlc3MgdGhhbiBpdCB0aGVuIGl0IGlzbid0IGhpZ2hlciB0aGFuIHRoZSByYW5nZVxuICAgIGlmICgoIWxvdy5vcGVyYXRvciB8fCBsb3cub3BlcmF0b3IgPT09IGNvbXApICYmXG4gICAgICAgIGx0ZWZuKHZlcnNpb24sIGxvdy5zZW12ZXIpKSB7XG4gICAgICByZXR1cm4gZmFsc2U7XG4gICAgfSBlbHNlIGlmIChsb3cub3BlcmF0b3IgPT09IGVjb21wICYmIGx0Zm4odmVyc2lvbiwgbG93LnNlbXZlcikpIHtcbiAgICAgIHJldHVybiBmYWxzZTtcbiAgICB9XG4gIH1cbiAgcmV0dXJuIHRydWU7XG59XG5cbmV4cG9ydHMucHJlcmVsZWFzZSA9IHByZXJlbGVhc2U7XG5mdW5jdGlvbiBwcmVyZWxlYXNlKHZlcnNpb24sIGxvb3NlKSB7XG4gIHZhciBwYXJzZWQgPSBwYXJzZSh2ZXJzaW9uLCBsb29zZSk7XG4gIHJldHVybiAocGFyc2VkICYmIHBhcnNlZC5wcmVyZWxlYXNlLmxlbmd0aCkgPyBwYXJzZWQucHJlcmVsZWFzZSA6IG51bGw7XG59XG5cblxuXG4vKiogV0VCUEFDSyBGT09URVIgKipcbiAqKiAvVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvfi9zZW12ZXIvc2VtdmVyLmpzXG4gKiovIiwiLy8gc2hpbSBmb3IgdXNpbmcgcHJvY2VzcyBpbiBicm93c2VyXG5cbnZhciBwcm9jZXNzID0gbW9kdWxlLmV4cG9ydHMgPSB7fTtcblxuLy8gY2FjaGVkIGZyb20gd2hhdGV2ZXIgZ2xvYmFsIGlzIHByZXNlbnQgc28gdGhhdCB0ZXN0IHJ1bm5lcnMgdGhhdCBzdHViIGl0XG4vLyBkb24ndCBicmVhayB0aGluZ3MuICBCdXQgd2UgbmVlZCB0byB3cmFwIGl0IGluIGEgdHJ5IGNhdGNoIGluIGNhc2UgaXQgaXNcbi8vIHdyYXBwZWQgaW4gc3RyaWN0IG1vZGUgY29kZSB3aGljaCBkb2Vzbid0IGRlZmluZSBhbnkgZ2xvYmFscy4gIEl0J3MgaW5zaWRlIGFcbi8vIGZ1bmN0aW9uIGJlY2F1c2UgdHJ5L2NhdGNoZXMgZGVvcHRpbWl6ZSBpbiBjZXJ0YWluIGVuZ2luZXMuXG5cbnZhciBjYWNoZWRTZXRUaW1lb3V0O1xudmFyIGNhY2hlZENsZWFyVGltZW91dDtcblxuKGZ1bmN0aW9uICgpIHtcbiAgdHJ5IHtcbiAgICBjYWNoZWRTZXRUaW1lb3V0ID0gc2V0VGltZW91dDtcbiAgfSBjYXRjaCAoZSkge1xuICAgIGNhY2hlZFNldFRpbWVvdXQgPSBmdW5jdGlvbiAoKSB7XG4gICAgICB0aHJvdyBuZXcgRXJyb3IoJ3NldFRpbWVvdXQgaXMgbm90IGRlZmluZWQnKTtcbiAgICB9XG4gIH1cbiAgdHJ5IHtcbiAgICBjYWNoZWRDbGVhclRpbWVvdXQgPSBjbGVhclRpbWVvdXQ7XG4gIH0gY2F0Y2ggKGUpIHtcbiAgICBjYWNoZWRDbGVhclRpbWVvdXQgPSBmdW5jdGlvbiAoKSB7XG4gICAgICB0aHJvdyBuZXcgRXJyb3IoJ2NsZWFyVGltZW91dCBpcyBub3QgZGVmaW5lZCcpO1xuICAgIH1cbiAgfVxufSAoKSlcbnZhciBxdWV1ZSA9IFtdO1xudmFyIGRyYWluaW5nID0gZmFsc2U7XG52YXIgY3VycmVudFF1ZXVlO1xudmFyIHF1ZXVlSW5kZXggPSAtMTtcblxuZnVuY3Rpb24gY2xlYW5VcE5leHRUaWNrKCkge1xuICAgIGlmICghZHJhaW5pbmcgfHwgIWN1cnJlbnRRdWV1ZSkge1xuICAgICAgICByZXR1cm47XG4gICAgfVxuICAgIGRyYWluaW5nID0gZmFsc2U7XG4gICAgaWYgKGN1cnJlbnRRdWV1ZS5sZW5ndGgpIHtcbiAgICAgICAgcXVldWUgPSBjdXJyZW50UXVldWUuY29uY2F0KHF1ZXVlKTtcbiAgICB9IGVsc2Uge1xuICAgICAgICBxdWV1ZUluZGV4ID0gLTE7XG4gICAgfVxuICAgIGlmIChxdWV1ZS5sZW5ndGgpIHtcbiAgICAgICAgZHJhaW5RdWV1ZSgpO1xuICAgIH1cbn1cblxuZnVuY3Rpb24gZHJhaW5RdWV1ZSgpIHtcbiAgICBpZiAoZHJhaW5pbmcpIHtcbiAgICAgICAgcmV0dXJuO1xuICAgIH1cbiAgICB2YXIgdGltZW91dCA9IGNhY2hlZFNldFRpbWVvdXQoY2xlYW5VcE5leHRUaWNrKTtcbiAgICBkcmFpbmluZyA9IHRydWU7XG5cbiAgICB2YXIgbGVuID0gcXVldWUubGVuZ3RoO1xuICAgIHdoaWxlKGxlbikge1xuICAgICAgICBjdXJyZW50UXVldWUgPSBxdWV1ZTtcbiAgICAgICAgcXVldWUgPSBbXTtcbiAgICAgICAgd2hpbGUgKCsrcXVldWVJbmRleCA8IGxlbikge1xuICAgICAgICAgICAgaWYgKGN1cnJlbnRRdWV1ZSkge1xuICAgICAgICAgICAgICAgIGN1cnJlbnRRdWV1ZVtxdWV1ZUluZGV4XS5ydW4oKTtcbiAgICAgICAgICAgIH1cbiAgICAgICAgfVxuICAgICAgICBxdWV1ZUluZGV4ID0gLTE7XG4gICAgICAgIGxlbiA9IHF1ZXVlLmxlbmd0aDtcbiAgICB9XG4gICAgY3VycmVudFF1ZXVlID0gbnVsbDtcbiAgICBkcmFpbmluZyA9IGZhbHNlO1xuICAgIGNhY2hlZENsZWFyVGltZW91dCh0aW1lb3V0KTtcbn1cblxucHJvY2Vzcy5uZXh0VGljayA9IGZ1bmN0aW9uIChmdW4pIHtcbiAgICB2YXIgYXJncyA9IG5ldyBBcnJheShhcmd1bWVudHMubGVuZ3RoIC0gMSk7XG4gICAgaWYgKGFyZ3VtZW50cy5sZW5ndGggPiAxKSB7XG4gICAgICAgIGZvciAodmFyIGkgPSAxOyBpIDwgYXJndW1lbnRzLmxlbmd0aDsgaSsrKSB7XG4gICAgICAgICAgICBhcmdzW2kgLSAxXSA9IGFyZ3VtZW50c1tpXTtcbiAgICAgICAgfVxuICAgIH1cbiAgICBxdWV1ZS5wdXNoKG5ldyBJdGVtKGZ1biwgYXJncykpO1xuICAgIGlmIChxdWV1ZS5sZW5ndGggPT09IDEgJiYgIWRyYWluaW5nKSB7XG4gICAgICAgIGNhY2hlZFNldFRpbWVvdXQoZHJhaW5RdWV1ZSwgMCk7XG4gICAgfVxufTtcblxuLy8gdjggbGlrZXMgcHJlZGljdGlibGUgb2JqZWN0c1xuZnVuY3Rpb24gSXRlbShmdW4sIGFycmF5KSB7XG4gICAgdGhpcy5mdW4gPSBmdW47XG4gICAgdGhpcy5hcnJheSA9IGFycmF5O1xufVxuSXRlbS5wcm90b3R5cGUucnVuID0gZnVuY3Rpb24gKCkge1xuICAgIHRoaXMuZnVuLmFwcGx5KG51bGwsIHRoaXMuYXJyYXkpO1xufTtcbnByb2Nlc3MudGl0bGUgPSAnYnJvd3Nlcic7XG5wcm9jZXNzLmJyb3dzZXIgPSB0cnVlO1xucHJvY2Vzcy5lbnYgPSB7fTtcbnByb2Nlc3MuYXJndiA9IFtdO1xucHJvY2Vzcy52ZXJzaW9uID0gJyc7IC8vIGVtcHR5IHN0cmluZyB0byBhdm9pZCByZWdleHAgaXNzdWVzXG5wcm9jZXNzLnZlcnNpb25zID0ge307XG5cbmZ1bmN0aW9uIG5vb3AoKSB7fVxuXG5wcm9jZXNzLm9uID0gbm9vcDtcbnByb2Nlc3MuYWRkTGlzdGVuZXIgPSBub29wO1xucHJvY2Vzcy5vbmNlID0gbm9vcDtcbnByb2Nlc3Mub2ZmID0gbm9vcDtcbnByb2Nlc3MucmVtb3ZlTGlzdGVuZXIgPSBub29wO1xucHJvY2Vzcy5yZW1vdmVBbGxMaXN0ZW5lcnMgPSBub29wO1xucHJvY2Vzcy5lbWl0ID0gbm9vcDtcblxucHJvY2Vzcy5iaW5kaW5nID0gZnVuY3Rpb24gKG5hbWUpIHtcbiAgICB0aHJvdyBuZXcgRXJyb3IoJ3Byb2Nlc3MuYmluZGluZyBpcyBub3Qgc3VwcG9ydGVkJyk7XG59O1xuXG5wcm9jZXNzLmN3ZCA9IGZ1bmN0aW9uICgpIHsgcmV0dXJuICcvJyB9O1xucHJvY2Vzcy5jaGRpciA9IGZ1bmN0aW9uIChkaXIpIHtcbiAgICB0aHJvdyBuZXcgRXJyb3IoJ3Byb2Nlc3MuY2hkaXIgaXMgbm90IHN1cHBvcnRlZCcpO1xufTtcbnByb2Nlc3MudW1hc2sgPSBmdW5jdGlvbigpIHsgcmV0dXJuIDA7IH07XG5cblxuXG4vKiogV0VCUEFDSyBGT09URVIgKipcbiAqKiAod2VicGFjaykvfi9ub2RlLWxpYnMtYnJvd3Nlci9+L3Byb2Nlc3MvYnJvd3Nlci5qc1xuICoqLyIsIi8qKlxuICogQGZpbGVPdmVydmlld1xuICogVmlld01vZGVsIENvbnN0cnVjdG9yICYgZGVmaW5pdGlvblxuICovXG5cbmltcG9ydCB7IGV4dGVuZCB9IGZyb20gJy4uL3V0aWwnXG5pbXBvcnQge1xuICBpbml0U3RhdGVcbn0gZnJvbSAnLi4vY29yZS9zdGF0ZSdcbmltcG9ydCB7XG4gIGJ1aWxkXG59IGZyb20gJy4vY29tcGlsZXInXG5pbXBvcnQge1xuICBpbml0RXZlbnRzLFxuICBtaXhpbkV2ZW50c1xufSBmcm9tICcuL2V2ZW50cydcblxuLyoqXG4gKiBWaWV3TW9kZWwgY29uc3RydWN0b3JcbiAqXG4gKiBAcGFyYW0ge3N0cmluZ30gdHlwZVxuICogQHBhcmFtIHtvYmplY3R9IG9wdGlvbnMgICAgY29tcG9uZW50IG9wdGlvbnNcbiAqIEBwYXJhbSB7b2JqZWN0fSBwYXJlbnRWbSAgIHdoaWNoIGNvbnRhaW5zIF9hcHBcbiAqIEBwYXJhbSB7b2JqZWN0fSBwYXJlbnRFbCAgIHJvb3QgZWxlbWVudCBvciBmcmFnIGJsb2NrXG4gKiBAcGFyYW0ge29iamVjdH0gbWVyZ2VkRGF0YSBleHRlcm5hbCBkYXRhXG4gKiBAcGFyYW0ge29iamVjdH0gZXh0ZXJuYWxFdmVudHMgZXh0ZXJuYWwgZXZlbnRzXG4gKi9cbmV4cG9ydCBkZWZhdWx0IGZ1bmN0aW9uIFZtIChcbiAgdHlwZSxcbiAgb3B0aW9ucyxcbiAgcGFyZW50Vm0sXG4gIHBhcmVudEVsLFxuICBtZXJnZWREYXRhLFxuICBleHRlcm5hbEV2ZW50c1xuKSB7XG4gIHRoaXMuX3BhcmVudCA9IHBhcmVudFZtLl9yZWFsUGFyZW50ID8gcGFyZW50Vm0uX3JlYWxQYXJlbnQgOiBwYXJlbnRWbVxuICB0aGlzLl9hcHAgPSBwYXJlbnRWbS5fYXBwXG4gIHBhcmVudFZtLl9jaGlsZHJlblZtcyAmJiBwYXJlbnRWbS5fY2hpbGRyZW5WbXMucHVzaCh0aGlzKVxuXG4gIGlmICghb3B0aW9ucykge1xuICAgIG9wdGlvbnMgPSB0aGlzLl9hcHAuY3VzdG9tQ29tcG9uZW50TWFwW3R5cGVdIHx8IHt9XG4gIH1cbiAgY29uc3QgZGF0YSA9IG9wdGlvbnMuZGF0YSB8fCB7fVxuXG4gIHRoaXMuX29wdGlvbnMgPSBvcHRpb25zXG4gIHRoaXMuX21ldGhvZHMgPSBvcHRpb25zLm1ldGhvZHMgfHwge31cbiAgdGhpcy5fY29tcHV0ZWQgPSBvcHRpb25zLmNvbXB1dGVkIHx8IHt9XG4gIHRoaXMuX2NzcyA9IG9wdGlvbnMuc3R5bGUgfHwge31cbiAgdGhpcy5faWRzID0ge31cbiAgdGhpcy5fdm1FdmVudHMgPSB7fVxuICB0aGlzLl9jaGlsZHJlblZtcyA9IFtdXG4gIHRoaXMuX3R5cGUgPSB0eXBlXG5cbiAgLy8gYmluZCBldmVudHMgYW5kIGxpZmVjeWNsZXNcbiAgaW5pdEV2ZW50cyh0aGlzLCBleHRlcm5hbEV2ZW50cylcblxuICBjb25zb2xlLmRlYnVnKGBbSlMgRnJhbWV3b3JrXSBcImluaXRcIiBsaWZlY3ljbGUgaW4gVm0oJHt0aGlzLl90eXBlfSlgKVxuICB0aGlzLiRlbWl0KCdob29rOmluaXQnKVxuICB0aGlzLl9pbml0ZWQgPSB0cnVlXG5cbiAgLy8gcHJveHkgZGF0YSBhbmQgbWV0aG9kc1xuICAvLyBvYnNlcnZlIGRhdGEgYW5kIGFkZCB0aGlzIHRvIHZtc1xuICB0aGlzLl9kYXRhID0gdHlwZW9mIGRhdGEgPT09ICdmdW5jdGlvbicgPyBkYXRhKCkgOiBkYXRhXG4gIGlmIChtZXJnZWREYXRhKSB7XG4gICAgZXh0ZW5kKHRoaXMuX2RhdGEsIG1lcmdlZERhdGEpXG4gIH1cbiAgaW5pdFN0YXRlKHRoaXMpXG5cbiAgY29uc29sZS5kZWJ1ZyhgW0pTIEZyYW1ld29ya10gXCJjcmVhdGVkXCIgbGlmZWN5Y2xlIGluIFZtKCR7dGhpcy5fdHlwZX0pYClcbiAgdGhpcy4kZW1pdCgnaG9vazpjcmVhdGVkJylcbiAgdGhpcy5fY3JlYXRlZCA9IHRydWVcblxuICAvLyBiYWNrd2FyZCBvbGQgcmVhZHkgZW50cnlcbiAgaWYgKG9wdGlvbnMubWV0aG9kcyAmJiBvcHRpb25zLm1ldGhvZHMucmVhZHkpIHtcbiAgICBjb25zb2xlLndhcm4oJ1wiZXhwb3J0cy5tZXRob2RzLnJlYWR5XCIgaXMgZGVwcmVjYXRlZCwgJyArXG4gICAgICAncGxlYXNlIHVzZSBcImV4cG9ydHMuY3JlYXRlZFwiIGluc3RlYWQnKVxuICAgIG9wdGlvbnMubWV0aG9kcy5yZWFkeS5jYWxsKHRoaXMpXG4gIH1cblxuICAvLyBpZiBubyBwYXJlbnRFbGVtZW50IHRoZW4gc3BlY2lmeSB0aGUgZG9jdW1lbnRFbGVtZW50XG4gIHRoaXMuX3BhcmVudEVsID0gcGFyZW50RWwgfHwgdGhpcy5fYXBwLmRvYy5kb2N1bWVudEVsZW1lbnRcbiAgYnVpbGQodGhpcylcbn1cblxubWl4aW5FdmVudHMoVm0ucHJvdG90eXBlKVxuXG5cblxuLyoqIFdFQlBBQ0sgRk9PVEVSICoqXG4gKiogL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL3NyYy9kZWZhdWx0L3ZtL2luZGV4LmpzXG4gKiovIiwiLyogZXNsaW50LWRpc2FibGUgKi9cblxuaW1wb3J0IFdhdGNoZXIgZnJvbSAnLi93YXRjaGVyJ1xuaW1wb3J0IERlcCBmcm9tICcuL2RlcCdcbmltcG9ydCB7XG4gIG9ic2VydmUsXG4gIHByb3h5LFxuICB1bnByb3h5XG59IGZyb20gJy4vb2JzZXJ2ZXInXG5pbXBvcnQge1xuICBpc1BsYWluT2JqZWN0LFxuICBiaW5kXG59IGZyb20gJy4uL3V0aWwnXG5cbmV4cG9ydCBmdW5jdGlvbiBpbml0U3RhdGUgKHZtKSB7XG4gIHZtLl93YXRjaGVycyA9IFtdXG4gIGluaXREYXRhKHZtKVxuICBpbml0Q29tcHV0ZWQodm0pXG4gIGluaXRNZXRob2RzKHZtKVxufVxuXG5leHBvcnQgZnVuY3Rpb24gaW5pdERhdGEgKHZtKSB7XG4gIGxldCBkYXRhID0gdm0uX2RhdGFcblxuICBpZiAoIWlzUGxhaW5PYmplY3QoZGF0YSkpIHtcbiAgICBkYXRhID0ge31cbiAgfVxuICAvLyBwcm94eSBkYXRhIG9uIGluc3RhbmNlXG4gIGNvbnN0IGtleXMgPSBPYmplY3Qua2V5cyhkYXRhKVxuICBsZXQgaSA9IGtleXMubGVuZ3RoXG4gIHdoaWxlIChpLS0pIHtcbiAgICBwcm94eSh2bSwga2V5c1tpXSlcbiAgfVxuICAvLyBvYnNlcnZlIGRhdGFcbiAgb2JzZXJ2ZShkYXRhLCB2bSlcbn1cblxuZnVuY3Rpb24gbm9vcCAoKSB7XG59XG5cbmV4cG9ydCBmdW5jdGlvbiBpbml0Q29tcHV0ZWQgKHZtKSB7XG4gIGNvbnN0IGNvbXB1dGVkID0gdm0uX2NvbXB1dGVkXG4gIGlmIChjb21wdXRlZCkge1xuICAgIGZvciAobGV0IGtleSBpbiBjb21wdXRlZCkge1xuICAgICAgY29uc3QgdXNlckRlZiA9IGNvbXB1dGVkW2tleV1cbiAgICAgIGNvbnN0IGRlZiA9IHtcbiAgICAgICAgZW51bWVyYWJsZTogdHJ1ZSxcbiAgICAgICAgY29uZmlndXJhYmxlOiB0cnVlXG4gICAgICB9XG4gICAgICBpZiAodHlwZW9mIHVzZXJEZWYgPT09ICdmdW5jdGlvbicpIHtcbiAgICAgICAgZGVmLmdldCA9IG1ha2VDb21wdXRlZEdldHRlcih1c2VyRGVmLCB2bSlcbiAgICAgICAgZGVmLnNldCA9IG5vb3BcbiAgICAgIH0gZWxzZSB7XG4gICAgICAgIGRlZi5nZXQgPSB1c2VyRGVmLmdldFxuICAgICAgICAgID8gdXNlckRlZi5jYWNoZSAhPT0gZmFsc2VcbiAgICAgICAgICAgID8gbWFrZUNvbXB1dGVkR2V0dGVyKHVzZXJEZWYuZ2V0LCB2bSlcbiAgICAgICAgICAgIDogYmluZCh1c2VyRGVmLmdldCwgdm0pXG4gICAgICAgICAgOiBub29wXG4gICAgICAgIGRlZi5zZXQgPSB1c2VyRGVmLnNldFxuICAgICAgICAgID8gYmluZCh1c2VyRGVmLnNldCwgdm0pXG4gICAgICAgICAgOiBub29wXG4gICAgICB9XG4gICAgICBPYmplY3QuZGVmaW5lUHJvcGVydHkodm0sIGtleSwgZGVmKVxuICAgIH1cbiAgfVxufVxuXG5mdW5jdGlvbiBtYWtlQ29tcHV0ZWRHZXR0ZXIgKGdldHRlciwgb3duZXIpIHtcbiAgY29uc3Qgd2F0Y2hlciA9IG5ldyBXYXRjaGVyKG93bmVyLCBnZXR0ZXIsIG51bGwsIHtcbiAgICBsYXp5OiB0cnVlXG4gIH0pXG4gIHJldHVybiBmdW5jdGlvbiBjb21wdXRlZEdldHRlciAoKSB7XG4gICAgaWYgKHdhdGNoZXIuZGlydHkpIHtcbiAgICAgIHdhdGNoZXIuZXZhbHVhdGUoKVxuICAgIH1cbiAgICBpZiAoRGVwLnRhcmdldCkge1xuICAgICAgd2F0Y2hlci5kZXBlbmQoKVxuICAgIH1cbiAgICByZXR1cm4gd2F0Y2hlci52YWx1ZVxuICB9XG59XG5cbmV4cG9ydCBmdW5jdGlvbiBpbml0TWV0aG9kcyAodm0pIHtcbiAgY29uc3QgbWV0aG9kcyA9IHZtLl9tZXRob2RzXG4gIGlmIChtZXRob2RzKSB7XG4gICAgZm9yIChsZXQga2V5IGluIG1ldGhvZHMpIHtcbiAgICAgIHZtW2tleV0gPSBtZXRob2RzW2tleV1cbiAgICB9XG4gIH1cbn1cblxuXG5cbi8qKiBXRUJQQUNLIEZPT1RFUiAqKlxuICoqIC9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9zcmMvZGVmYXVsdC9jb3JlL3N0YXRlLmpzXG4gKiovIiwiLyogZXNsaW50LWRpc2FibGUgKi9cblxuaW1wb3J0IERlcCBmcm9tICcuL2RlcCdcbi8vIGltcG9ydCB7IHB1c2hXYXRjaGVyIH0gZnJvbSAnLi9iYXRjaGVyJ1xuaW1wb3J0IHtcbiAgd2FybixcbiAgcmVtb3ZlLFxuICBleHRlbmQsXG4gIGlzT2JqZWN0LFxuICBwYXJzZVBhdGgsXG4gIF9TZXQgYXMgU2V0XG59IGZyb20gJy4uL3V0aWwnXG5cbmxldCB1aWQgPSAwXG5sZXQgcHJldlRhcmdldFxuXG4vKipcbiAqIEEgd2F0Y2hlciBwYXJzZXMgYW4gZXhwcmVzc2lvbiwgY29sbGVjdHMgZGVwZW5kZW5jaWVzLFxuICogYW5kIGZpcmVzIGNhbGxiYWNrIHdoZW4gdGhlIGV4cHJlc3Npb24gdmFsdWUgY2hhbmdlcy5cbiAqIFRoaXMgaXMgdXNlZCBmb3IgYm90aCB0aGUgJHdhdGNoKCkgYXBpIGFuZCBkaXJlY3RpdmVzLlxuICpcbiAqIEBwYXJhbSB7VnVlfSB2bVxuICogQHBhcmFtIHtTdHJpbmd8RnVuY3Rpb259IGV4cE9yRm5cbiAqIEBwYXJhbSB7RnVuY3Rpb259IGNiXG4gKiBAcGFyYW0ge09iamVjdH0gb3B0aW9uc1xuICogICAgICAgICAgICAgICAgIC0ge0FycmF5fSBmaWx0ZXJzXG4gKiAgICAgICAgICAgICAgICAgLSB7Qm9vbGVhbn0gdHdvV2F5XG4gKiAgICAgICAgICAgICAgICAgLSB7Qm9vbGVhbn0gZGVlcFxuICogICAgICAgICAgICAgICAgIC0ge0Jvb2xlYW59IHVzZXJcbiAqICAgICAgICAgICAgICAgICAtIHtCb29sZWFufSBzeW5jXG4gKiAgICAgICAgICAgICAgICAgLSB7Qm9vbGVhbn0gbGF6eVxuICogICAgICAgICAgICAgICAgIC0ge0Z1bmN0aW9ufSBbcHJlUHJvY2Vzc11cbiAqICAgICAgICAgICAgICAgICAtIHtGdW5jdGlvbn0gW3Bvc3RQcm9jZXNzXVxuICogQGNvbnN0cnVjdG9yXG4gKi9cblxuZXhwb3J0IGRlZmF1bHQgZnVuY3Rpb24gV2F0Y2hlciAodm0sIGV4cE9yRm4sIGNiLCBvcHRpb25zKSB7XG4gIC8vIG1peCBpbiBvcHRpb25zXG4gIGlmIChvcHRpb25zKSB7XG4gICAgZXh0ZW5kKHRoaXMsIG9wdGlvbnMpXG4gIH1cbiAgY29uc3QgaXNGbiA9IHR5cGVvZiBleHBPckZuID09PSAnZnVuY3Rpb24nXG4gIHRoaXMudm0gPSB2bVxuICB2bS5fd2F0Y2hlcnMucHVzaCh0aGlzKVxuICB0aGlzLmV4cHJlc3Npb24gPSBleHBPckZuXG4gIHRoaXMuY2IgPSBjYlxuICB0aGlzLmlkID0gKyt1aWQgLy8gdWlkIGZvciBiYXRjaGluZ1xuICB0aGlzLmFjdGl2ZSA9IHRydWVcbiAgdGhpcy5kaXJ0eSA9IHRoaXMubGF6eSAvLyBmb3IgbGF6eSB3YXRjaGVyc1xuICB0aGlzLmRlcHMgPSBbXVxuICB0aGlzLm5ld0RlcHMgPSBbXVxuICB0aGlzLmRlcElkcyA9IG5ldyBTZXQoKVxuICB0aGlzLm5ld0RlcElkcyA9IG5ldyBTZXQoKVxuICAvLyBwYXJzZSBleHByZXNzaW9uIGZvciBnZXR0ZXJcbiAgaWYgKGlzRm4pIHtcbiAgICB0aGlzLmdldHRlciA9IGV4cE9yRm5cbiAgfSBlbHNlIHtcbiAgICB0aGlzLmdldHRlciA9IHBhcnNlUGF0aChleHBPckZuKVxuICAgIGlmICghdGhpcy5nZXR0ZXIpIHtcbiAgICAgIHRoaXMuZ2V0dGVyID0gZnVuY3Rpb24gKCkge31cbiAgICAgIHByb2Nlc3MuZW52Lk5PREVfRU5WICE9PSAncHJvZHVjdGlvbicgJiYgd2FybihcbiAgICAgICAgJ0ZhaWxlZCB3YXRjaGluZyBwYXRoOiAnICsgZXhwT3JGbiArXG4gICAgICAgICdXYXRjaGVyIG9ubHkgYWNjZXB0cyBzaW1wbGUgZG90LWRlbGltaXRlZCBwYXRocy4gJyArXG4gICAgICAgICdGb3IgZnVsbCBjb250cm9sLCB1c2UgYSBmdW5jdGlvbiBpbnN0ZWFkLicsXG4gICAgICAgIHZtXG4gICAgICApXG4gICAgfVxuICB9XG4gIHRoaXMudmFsdWUgPSB0aGlzLmxhenlcbiAgICA/IHVuZGVmaW5lZFxuICAgIDogdGhpcy5nZXQoKVxuICAvLyBzdGF0ZSBmb3IgYXZvaWRpbmcgZmFsc2UgdHJpZ2dlcnMgZm9yIGRlZXAgYW5kIEFycmF5XG4gIC8vIHdhdGNoZXJzIGR1cmluZyB2bS5fZGlnZXN0KClcbiAgdGhpcy5xdWV1ZWQgPSB0aGlzLnNoYWxsb3cgPSBmYWxzZVxufVxuXG4vKipcbiAqIEV2YWx1YXRlIHRoZSBnZXR0ZXIsIGFuZCByZS1jb2xsZWN0IGRlcGVuZGVuY2llcy5cbiAqL1xuXG5XYXRjaGVyLnByb3RvdHlwZS5nZXQgPSBmdW5jdGlvbiAoKSB7XG4gIHRoaXMuYmVmb3JlR2V0KClcbiAgY29uc3QgdmFsdWUgPSB0aGlzLmdldHRlci5jYWxsKHRoaXMudm0sIHRoaXMudm0pXG4gIC8vIFwidG91Y2hcIiBldmVyeSBwcm9wZXJ0eSBzbyB0aGV5IGFyZSBhbGwgdHJhY2tlZCBhc1xuICAvLyBkZXBlbmRlbmNpZXMgZm9yIGRlZXAgd2F0Y2hpbmdcbiAgaWYgKHRoaXMuZGVlcCkge1xuICAgIHRyYXZlcnNlKHZhbHVlKVxuICB9XG4gIHRoaXMuYWZ0ZXJHZXQoKVxuICByZXR1cm4gdmFsdWVcbn1cblxuLyoqXG4gKiBQcmVwYXJlIGZvciBkZXBlbmRlbmN5IGNvbGxlY3Rpb24uXG4gKi9cblxuV2F0Y2hlci5wcm90b3R5cGUuYmVmb3JlR2V0ID0gZnVuY3Rpb24gKCkge1xuICBwcmV2VGFyZ2V0ID0gRGVwLnRhcmdldFxuICBEZXAudGFyZ2V0ID0gdGhpc1xufVxuXG4vKipcbiAqIEFkZCBhIGRlcGVuZGVuY3kgdG8gdGhpcyBkaXJlY3RpdmUuXG4gKlxuICogQHBhcmFtIHtEZXB9IGRlcFxuICovXG5cbldhdGNoZXIucHJvdG90eXBlLmFkZERlcCA9IGZ1bmN0aW9uIChkZXApIHtcbiAgY29uc3QgaWQgPSBkZXAuaWRcbiAgaWYgKCF0aGlzLm5ld0RlcElkcy5oYXMoaWQpKSB7XG4gICAgdGhpcy5uZXdEZXBJZHMuYWRkKGlkKVxuICAgIHRoaXMubmV3RGVwcy5wdXNoKGRlcClcbiAgICBpZiAoIXRoaXMuZGVwSWRzLmhhcyhpZCkpIHtcbiAgICAgIGRlcC5hZGRTdWIodGhpcylcbiAgICB9XG4gIH1cbn1cblxuLyoqXG4gKiBDbGVhbiB1cCBmb3IgZGVwZW5kZW5jeSBjb2xsZWN0aW9uLlxuICovXG5cbldhdGNoZXIucHJvdG90eXBlLmFmdGVyR2V0ID0gZnVuY3Rpb24gKCkge1xuICBEZXAudGFyZ2V0ID0gcHJldlRhcmdldFxuICBsZXQgaSA9IHRoaXMuZGVwcy5sZW5ndGhcbiAgd2hpbGUgKGktLSkge1xuICAgIGNvbnN0IGRlcCA9IHRoaXMuZGVwc1tpXVxuICAgIGlmICghdGhpcy5uZXdEZXBJZHMuaGFzKGRlcC5pZCkpIHtcbiAgICAgIGRlcC5yZW1vdmVTdWIodGhpcylcbiAgICB9XG4gIH1cbiAgbGV0IHRtcCA9IHRoaXMuZGVwSWRzXG4gIHRoaXMuZGVwSWRzID0gdGhpcy5uZXdEZXBJZHNcbiAgdGhpcy5uZXdEZXBJZHMgPSB0bXBcbiAgdGhpcy5uZXdEZXBJZHMuY2xlYXIoKVxuICB0bXAgPSB0aGlzLmRlcHNcbiAgdGhpcy5kZXBzID0gdGhpcy5uZXdEZXBzXG4gIHRoaXMubmV3RGVwcyA9IHRtcFxuICB0aGlzLm5ld0RlcHMubGVuZ3RoID0gMFxufVxuXG4vKipcbiAqIFN1YnNjcmliZXIgaW50ZXJmYWNlLlxuICogV2lsbCBiZSBjYWxsZWQgd2hlbiBhIGRlcGVuZGVuY3kgY2hhbmdlcy5cbiAqXG4gKiBAcGFyYW0ge0Jvb2xlYW59IHNoYWxsb3dcbiAqL1xuXG5XYXRjaGVyLnByb3RvdHlwZS51cGRhdGUgPSBmdW5jdGlvbiAoc2hhbGxvdykge1xuICBpZiAodGhpcy5sYXp5KSB7XG4gICAgdGhpcy5kaXJ0eSA9IHRydWVcbiAgfSBlbHNlIHtcbiAgICB0aGlzLnJ1bigpXG4gIH1cbiAgLy8gfSBlbHNlIGlmICh0aGlzLnN5bmMpIHtcbiAgLy8gICB0aGlzLnJ1bigpXG4gIC8vIH0gZWxzZSB7XG4gIC8vICAgLy8gaWYgcXVldWVkLCBvbmx5IG92ZXJ3cml0ZSBzaGFsbG93IHdpdGggbm9uLXNoYWxsb3csXG4gIC8vICAgLy8gYnV0IG5vdCB0aGUgb3RoZXIgd2F5IGFyb3VuZC5cbiAgLy8gICB0aGlzLnNoYWxsb3cgPSB0aGlzLnF1ZXVlZFxuICAvLyAgICAgPyBzaGFsbG93XG4gIC8vICAgICAgID8gdGhpcy5zaGFsbG93XG4gIC8vICAgICAgIDogZmFsc2VcbiAgLy8gICAgIDogISFzaGFsbG93XG4gIC8vICAgdGhpcy5xdWV1ZWQgPSB0cnVlXG4gIC8vICAgcHVzaFdhdGNoZXIodGhpcylcbiAgLy8gfVxufVxuXG4vKipcbiAqIEJhdGNoZXIgam9iIGludGVyZmFjZS5cbiAqIFdpbGwgYmUgY2FsbGVkIGJ5IHRoZSBiYXRjaGVyLlxuICovXG5cbldhdGNoZXIucHJvdG90eXBlLnJ1biA9IGZ1bmN0aW9uICgpIHtcbiAgaWYgKHRoaXMuYWN0aXZlKSB7XG4gICAgY29uc3QgdmFsdWUgPSB0aGlzLmdldCgpXG4gICAgaWYgKFxuICAgICAgdmFsdWUgIT09IHRoaXMudmFsdWUgfHxcbiAgICAgIC8vIERlZXAgd2F0Y2hlcnMgYW5kIHdhdGNoZXJzIG9uIE9iamVjdC9BcnJheXMgc2hvdWxkIGZpcmUgZXZlblxuICAgICAgLy8gd2hlbiB0aGUgdmFsdWUgaXMgdGhlIHNhbWUsIGJlY2F1c2UgdGhlIHZhbHVlIG1heVxuICAgICAgLy8gaGF2ZSBtdXRhdGVkOyBidXQgb25seSBkbyBzbyBpZiB0aGlzIGlzIGFcbiAgICAgIC8vIG5vbi1zaGFsbG93IHVwZGF0ZSAoY2F1c2VkIGJ5IGEgdm0gZGlnZXN0KS5cbiAgICAgICgoaXNPYmplY3QodmFsdWUpIHx8IHRoaXMuZGVlcCkgJiYgIXRoaXMuc2hhbGxvdylcbiAgICApIHtcbiAgICAgIC8vIHNldCBuZXcgdmFsdWVcbiAgICAgIGNvbnN0IG9sZFZhbHVlID0gdGhpcy52YWx1ZVxuICAgICAgdGhpcy52YWx1ZSA9IHZhbHVlXG4gICAgICB0aGlzLmNiLmNhbGwodGhpcy52bSwgdmFsdWUsIG9sZFZhbHVlKVxuICAgIH1cbiAgICB0aGlzLnF1ZXVlZCA9IHRoaXMuc2hhbGxvdyA9IGZhbHNlXG4gIH1cbn1cblxuLyoqXG4gKiBFdmFsdWF0ZSB0aGUgdmFsdWUgb2YgdGhlIHdhdGNoZXIuXG4gKiBUaGlzIG9ubHkgZ2V0cyBjYWxsZWQgZm9yIGxhenkgd2F0Y2hlcnMuXG4gKi9cblxuV2F0Y2hlci5wcm90b3R5cGUuZXZhbHVhdGUgPSBmdW5jdGlvbiAoKSB7XG4gIC8vIGF2b2lkIG92ZXJ3cml0aW5nIGFub3RoZXIgd2F0Y2hlciB0aGF0IGlzIGJlaW5nXG4gIC8vIGNvbGxlY3RlZC5cbiAgY29uc3QgY3VycmVudCA9IERlcC50YXJnZXRcbiAgdGhpcy52YWx1ZSA9IHRoaXMuZ2V0KClcbiAgdGhpcy5kaXJ0eSA9IGZhbHNlXG4gIERlcC50YXJnZXQgPSBjdXJyZW50XG59XG5cbi8qKlxuICogRGVwZW5kIG9uIGFsbCBkZXBzIGNvbGxlY3RlZCBieSB0aGlzIHdhdGNoZXIuXG4gKi9cblxuV2F0Y2hlci5wcm90b3R5cGUuZGVwZW5kID0gZnVuY3Rpb24gKCkge1xuICBsZXQgaSA9IHRoaXMuZGVwcy5sZW5ndGhcbiAgd2hpbGUgKGktLSkge1xuICAgIHRoaXMuZGVwc1tpXS5kZXBlbmQoKVxuICB9XG59XG5cbi8qKlxuICogUmVtb3ZlIHNlbGYgZnJvbSBhbGwgZGVwZW5kZW5jaWVzJyBzdWJjcmliZXIgbGlzdC5cbiAqL1xuXG5XYXRjaGVyLnByb3RvdHlwZS50ZWFyZG93biA9IGZ1bmN0aW9uICgpIHtcbiAgaWYgKHRoaXMuYWN0aXZlKSB7XG4gICAgLy8gcmVtb3ZlIHNlbGYgZnJvbSB2bSdzIHdhdGNoZXIgbGlzdFxuICAgIC8vIHRoaXMgaXMgYSBzb21ld2hhdCBleHBlbnNpdmUgb3BlcmF0aW9uIHNvIHdlIHNraXAgaXRcbiAgICAvLyBpZiB0aGUgdm0gaXMgYmVpbmcgZGVzdHJveWVkIG9yIGlzIHBlcmZvcm1pbmcgYSB2LWZvclxuICAgIC8vIHJlLXJlbmRlciAodGhlIHdhdGNoZXIgbGlzdCBpcyB0aGVuIGZpbHRlcmVkIGJ5IHYtZm9yKS5cbiAgICBpZiAoIXRoaXMudm0uX2lzQmVpbmdEZXN0cm95ZWQgJiYgIXRoaXMudm0uX3ZGb3JSZW1vdmluZykge1xuICAgICAgcmVtb3ZlKHRoaXMudm0uX3dhdGNoZXJzLCB0aGlzKVxuICAgIH1cbiAgICBsZXQgaSA9IHRoaXMuZGVwcy5sZW5ndGhcbiAgICB3aGlsZSAoaS0tKSB7XG4gICAgICB0aGlzLmRlcHNbaV0ucmVtb3ZlU3ViKHRoaXMpXG4gICAgfVxuICAgIHRoaXMuYWN0aXZlID0gZmFsc2VcbiAgICB0aGlzLnZtID0gdGhpcy5jYiA9IHRoaXMudmFsdWUgPSBudWxsXG4gIH1cbn1cblxuLyoqXG4gKiBSZWNydXNpdmVseSB0cmF2ZXJzZSBhbiBvYmplY3QgdG8gZXZva2UgYWxsIGNvbnZlcnRlZFxuICogZ2V0dGVycywgc28gdGhhdCBldmVyeSBuZXN0ZWQgcHJvcGVydHkgaW5zaWRlIHRoZSBvYmplY3RcbiAqIGlzIGNvbGxlY3RlZCBhcyBhIFwiZGVlcFwiIGRlcGVuZGVuY3kuXG4gKlxuICogQHBhcmFtIHsqfSB2YWxcbiAqIEBwYXJhbSB7U2V0fSBzZWVuXG4gKi9cblxuY29uc3Qgc2Vlbk9iamVjdHMgPSBuZXcgU2V0KClcbmZ1bmN0aW9uIHRyYXZlcnNlICh2YWwsIHNlZW4pIHtcbiAgbGV0IGksIGtleXMsIGlzQSwgaXNPXG4gIGlmICghc2Vlbikge1xuICAgIHNlZW4gPSBzZWVuT2JqZWN0c1xuICAgIHNlZW4uY2xlYXIoKVxuICB9XG4gIGlzQSA9IEFycmF5LmlzQXJyYXkodmFsKVxuICBpc08gPSBpc09iamVjdCh2YWwpXG4gIGlmIChpc0EgfHwgaXNPKSB7XG4gICAgaWYgKHZhbC5fX29iX18pIHtcbiAgICAgIGNvbnN0IGRlcElkID0gdmFsLl9fb2JfXy5kZXAuaWRcbiAgICAgIGlmIChzZWVuLmhhcyhkZXBJZCkpIHtcbiAgICAgICAgcmV0dXJuXG4gICAgICB9IGVsc2Uge1xuICAgICAgICBzZWVuLmFkZChkZXBJZClcbiAgICAgIH1cbiAgICB9XG4gICAgaWYgKGlzQSkge1xuICAgICAgaSA9IHZhbC5sZW5ndGhcbiAgICAgIHdoaWxlIChpLS0pIHRyYXZlcnNlKHZhbFtpXSwgc2VlbilcbiAgICB9IGVsc2UgaWYgKGlzTykge1xuICAgICAga2V5cyA9IE9iamVjdC5rZXlzKHZhbClcbiAgICAgIGkgPSBrZXlzLmxlbmd0aFxuICAgICAgd2hpbGUgKGktLSkgdHJhdmVyc2UodmFsW2tleXNbaV1dLCBzZWVuKVxuICAgIH1cbiAgfVxufVxuXG5cblxuLyoqIFdFQlBBQ0sgRk9PVEVSICoqXG4gKiogL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL3NyYy9kZWZhdWx0L2NvcmUvd2F0Y2hlci5qc1xuICoqLyIsIi8qIGVzbGludC1kaXNhYmxlICovXG5cbmltcG9ydCB7IHJlbW92ZSB9IGZyb20gJy4uL3V0aWwnXG5cbmxldCB1aWQgPSAwXG5cbi8qKlxuICogQSBkZXAgaXMgYW4gb2JzZXJ2YWJsZSB0aGF0IGNhbiBoYXZlIG11bHRpcGxlXG4gKiBkaXJlY3RpdmVzIHN1YnNjcmliaW5nIHRvIGl0LlxuICpcbiAqIEBjb25zdHJ1Y3RvclxuICovXG5cbmV4cG9ydCBkZWZhdWx0IGZ1bmN0aW9uIERlcCAoKSB7XG4gIHRoaXMuaWQgPSB1aWQrK1xuICB0aGlzLnN1YnMgPSBbXVxufVxuXG4vLyB0aGUgY3VycmVudCB0YXJnZXQgd2F0Y2hlciBiZWluZyBldmFsdWF0ZWQuXG4vLyB0aGlzIGlzIGdsb2JhbGx5IHVuaXF1ZSBiZWNhdXNlIHRoZXJlIGNvdWxkIGJlIG9ubHkgb25lXG4vLyB3YXRjaGVyIGJlaW5nIGV2YWx1YXRlZCBhdCBhbnkgdGltZS5cbkRlcC50YXJnZXQgPSBudWxsXG5cbi8qKlxuICogQWRkIGEgZGlyZWN0aXZlIHN1YnNjcmliZXIuXG4gKlxuICogQHBhcmFtIHtEaXJlY3RpdmV9IHN1YlxuICovXG5cbkRlcC5wcm90b3R5cGUuYWRkU3ViID0gZnVuY3Rpb24gKHN1Yikge1xuICB0aGlzLnN1YnMucHVzaChzdWIpXG59XG5cbi8qKlxuICogUmVtb3ZlIGEgZGlyZWN0aXZlIHN1YnNjcmliZXIuXG4gKlxuICogQHBhcmFtIHtEaXJlY3RpdmV9IHN1YlxuICovXG5cbkRlcC5wcm90b3R5cGUucmVtb3ZlU3ViID0gZnVuY3Rpb24gKHN1Yikge1xuICByZW1vdmUodGhpcy5zdWJzLCBzdWIpXG59XG5cbi8qKlxuICogQWRkIHNlbGYgYXMgYSBkZXBlbmRlbmN5IHRvIHRoZSB0YXJnZXQgd2F0Y2hlci5cbiAqL1xuXG5EZXAucHJvdG90eXBlLmRlcGVuZCA9IGZ1bmN0aW9uICgpIHtcbiAgRGVwLnRhcmdldC5hZGREZXAodGhpcylcbn1cblxuLyoqXG4gKiBOb3RpZnkgYWxsIHN1YnNjcmliZXJzIG9mIGEgbmV3IHZhbHVlLlxuICovXG5cbkRlcC5wcm90b3R5cGUubm90aWZ5ID0gZnVuY3Rpb24gKCkge1xuICAvLyBzdGFibGl6ZSB0aGUgc3Vic2NyaWJlciBsaXN0IGZpcnN0XG4gIGNvbnN0IHN1YnMgPSB0aGlzLnN1YnMuc2xpY2UoKVxuICBmb3IgKGxldCBpID0gMCwgbCA9IHN1YnMubGVuZ3RoOyBpIDwgbDsgaSsrKSB7XG4gICAgc3Vic1tpXS51cGRhdGUoKVxuICB9XG59XG5cblxuXG4vKiogV0VCUEFDSyBGT09URVIgKipcbiAqKiAvVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvc3JjL2RlZmF1bHQvY29yZS9kZXAuanNcbiAqKi8iLCIvKiBlc2xpbnQtZGlzYWJsZSAqL1xuXG5pbXBvcnQgRGVwIGZyb20gJy4vZGVwJ1xuaW1wb3J0IHsgYXJyYXlNZXRob2RzIH0gZnJvbSAnLi9hcnJheSdcbmltcG9ydCB7XG4gIGRlZixcbiAgcmVtb3ZlLFxuICBpc09iamVjdCxcbiAgaXNQbGFpbk9iamVjdCxcbiAgaGFzUHJvdG8sXG4gIGhhc093bixcbiAgaXNSZXNlcnZlZFxufSBmcm9tICcuLi91dGlsJ1xuXG5jb25zdCBhcnJheUtleXMgPSBPYmplY3QuZ2V0T3duUHJvcGVydHlOYW1lcyhhcnJheU1ldGhvZHMpXG5cbi8qKlxuICogT2JzZXJ2ZXIgY2xhc3MgdGhhdCBhcmUgYXR0YWNoZWQgdG8gZWFjaCBvYnNlcnZlZFxuICogb2JqZWN0LiBPbmNlIGF0dGFjaGVkLCB0aGUgb2JzZXJ2ZXIgY29udmVydHMgdGFyZ2V0XG4gKiBvYmplY3QncyBwcm9wZXJ0eSBrZXlzIGludG8gZ2V0dGVyL3NldHRlcnMgdGhhdFxuICogY29sbGVjdCBkZXBlbmRlbmNpZXMgYW5kIGRpc3BhdGNoZXMgdXBkYXRlcy5cbiAqXG4gKiBAcGFyYW0ge0FycmF5fE9iamVjdH0gdmFsdWVcbiAqIEBjb25zdHJ1Y3RvclxuICovXG5cbmV4cG9ydCBmdW5jdGlvbiBPYnNlcnZlciAodmFsdWUpIHtcbiAgdGhpcy52YWx1ZSA9IHZhbHVlXG4gIHRoaXMuZGVwID0gbmV3IERlcCgpXG4gIGRlZih2YWx1ZSwgJ19fb2JfXycsIHRoaXMpXG4gIGlmIChBcnJheS5pc0FycmF5KHZhbHVlKSkge1xuICAgIGNvbnN0IGF1Z21lbnQgPSBoYXNQcm90b1xuICAgICAgPyBwcm90b0F1Z21lbnRcbiAgICAgIDogY29weUF1Z21lbnRcbiAgICBhdWdtZW50KHZhbHVlLCBhcnJheU1ldGhvZHMsIGFycmF5S2V5cylcbiAgICB0aGlzLm9ic2VydmVBcnJheSh2YWx1ZSlcbiAgfSBlbHNlIHtcbiAgICB0aGlzLndhbGsodmFsdWUpXG4gIH1cbn1cblxuLy8gSW5zdGFuY2UgbWV0aG9kc1xuXG4vKipcbiAqIFdhbGsgdGhyb3VnaCBlYWNoIHByb3BlcnR5IGFuZCBjb252ZXJ0IHRoZW0gaW50b1xuICogZ2V0dGVyL3NldHRlcnMuIFRoaXMgbWV0aG9kIHNob3VsZCBvbmx5IGJlIGNhbGxlZCB3aGVuXG4gKiB2YWx1ZSB0eXBlIGlzIE9iamVjdC5cbiAqXG4gKiBAcGFyYW0ge09iamVjdH0gb2JqXG4gKi9cblxuT2JzZXJ2ZXIucHJvdG90eXBlLndhbGsgPSBmdW5jdGlvbiAob2JqKSB7XG4gIGZvciAobGV0IGtleSBpbiBvYmopIHtcbiAgICB0aGlzLmNvbnZlcnQoa2V5LCBvYmpba2V5XSlcbiAgfVxufVxuXG4vKipcbiAqIE9ic2VydmUgYSBsaXN0IG9mIEFycmF5IGl0ZW1zLlxuICpcbiAqIEBwYXJhbSB7QXJyYXl9IGl0ZW1zXG4gKi9cblxuT2JzZXJ2ZXIucHJvdG90eXBlLm9ic2VydmVBcnJheSA9IGZ1bmN0aW9uIChpdGVtcykge1xuICBmb3IgKGxldCBpID0gMCwgbCA9IGl0ZW1zLmxlbmd0aDsgaSA8IGw7IGkrKykge1xuICAgIG9ic2VydmUoaXRlbXNbaV0pXG4gIH1cbn1cblxuLyoqXG4gKiBDb252ZXJ0IGEgcHJvcGVydHkgaW50byBnZXR0ZXIvc2V0dGVyIHNvIHdlIGNhbiBlbWl0XG4gKiB0aGUgZXZlbnRzIHdoZW4gdGhlIHByb3BlcnR5IGlzIGFjY2Vzc2VkL2NoYW5nZWQuXG4gKlxuICogQHBhcmFtIHtTdHJpbmd9IGtleVxuICogQHBhcmFtIHsqfSB2YWxcbiAqL1xuXG5PYnNlcnZlci5wcm90b3R5cGUuY29udmVydCA9IGZ1bmN0aW9uIChrZXksIHZhbCkge1xuICBkZWZpbmVSZWFjdGl2ZSh0aGlzLnZhbHVlLCBrZXksIHZhbClcbn1cblxuLyoqXG4gKiBBZGQgYW4gb3duZXIgdm0sIHNvIHRoYXQgd2hlbiAkc2V0LyRkZWxldGUgbXV0YXRpb25zXG4gKiBoYXBwZW4gd2UgY2FuIG5vdGlmeSBvd25lciB2bXMgdG8gcHJveHkgdGhlIGtleXMgYW5kXG4gKiBkaWdlc3QgdGhlIHdhdGNoZXJzLiBUaGlzIGlzIG9ubHkgY2FsbGVkIHdoZW4gdGhlIG9iamVjdFxuICogaXMgb2JzZXJ2ZWQgYXMgYW4gaW5zdGFuY2UncyByb290ICRkYXRhLlxuICpcbiAqIEBwYXJhbSB7VnVlfSB2bVxuICovXG5cbk9ic2VydmVyLnByb3RvdHlwZS5hZGRWbSA9IGZ1bmN0aW9uICh2bSkge1xuICAodGhpcy52bXMgfHwgKHRoaXMudm1zID0gW10pKS5wdXNoKHZtKVxufVxuXG4vKipcbiAqIFJlbW92ZSBhbiBvd25lciB2bS4gVGhpcyBpcyBjYWxsZWQgd2hlbiB0aGUgb2JqZWN0IGlzXG4gKiBzd2FwcGVkIG91dCBhcyBhbiBpbnN0YW5jZSdzICRkYXRhIG9iamVjdC5cbiAqXG4gKiBAcGFyYW0ge1Z1ZX0gdm1cbiAqL1xuXG5PYnNlcnZlci5wcm90b3R5cGUucmVtb3ZlVm0gPSBmdW5jdGlvbiAodm0pIHtcbiAgcmVtb3ZlKHRoaXMudm1zLCB2bSlcbn1cblxuLy8gaGVscGVyc1xuXG4vKipcbiAqIEF1Z21lbnQgYW4gdGFyZ2V0IE9iamVjdCBvciBBcnJheSBieSBpbnRlcmNlcHRpbmdcbiAqIHRoZSBwcm90b3R5cGUgY2hhaW4gdXNpbmcgX19wcm90b19fXG4gKlxuICogQHBhcmFtIHtPYmplY3R8QXJyYXl9IHRhcmdldFxuICogQHBhcmFtIHtPYmplY3R9IHNyY1xuICovXG5cbmZ1bmN0aW9uIHByb3RvQXVnbWVudCAodGFyZ2V0LCBzcmMpIHtcbiAgLyogZXNsaW50LWRpc2FibGUgbm8tcHJvdG8gKi9cbiAgdGFyZ2V0Ll9fcHJvdG9fXyA9IHNyY1xuICAvKiBlc2xpbnQtZW5hYmxlIG5vLXByb3RvICovXG59XG5cbi8qKlxuICogQXVnbWVudCBhbiB0YXJnZXQgT2JqZWN0IG9yIEFycmF5IGJ5IGRlZmluaW5nXG4gKiBoaWRkZW4gcHJvcGVydGllcy5cbiAqXG4gKiBAcGFyYW0ge09iamVjdHxBcnJheX0gdGFyZ2V0XG4gKiBAcGFyYW0ge09iamVjdH0gcHJvdG9cbiAqL1xuXG5mdW5jdGlvbiBjb3B5QXVnbWVudCAodGFyZ2V0LCBzcmMsIGtleXMpIHtcbiAgZm9yIChsZXQgaSA9IDAsIGwgPSBrZXlzLmxlbmd0aDsgaSA8IGw7IGkrKykge1xuICAgIGNvbnN0IGtleSA9IGtleXNbaV1cbiAgICBkZWYodGFyZ2V0LCBrZXksIHNyY1trZXldKVxuICB9XG59XG5cbi8qKlxuICogQXR0ZW1wdCB0byBjcmVhdGUgYW4gb2JzZXJ2ZXIgaW5zdGFuY2UgZm9yIGEgdmFsdWUsXG4gKiByZXR1cm5zIHRoZSBuZXcgb2JzZXJ2ZXIgaWYgc3VjY2Vzc2Z1bGx5IG9ic2VydmVkLFxuICogb3IgdGhlIGV4aXN0aW5nIG9ic2VydmVyIGlmIHRoZSB2YWx1ZSBhbHJlYWR5IGhhcyBvbmUuXG4gKlxuICogQHBhcmFtIHsqfSB2YWx1ZVxuICogQHBhcmFtIHtWdWV9IFt2bV1cbiAqIEByZXR1cm4ge09ic2VydmVyfHVuZGVmaW5lZH1cbiAqIEBzdGF0aWNcbiAqL1xuXG5leHBvcnQgZnVuY3Rpb24gb2JzZXJ2ZSAodmFsdWUsIHZtKSB7XG4gIGlmICghaXNPYmplY3QodmFsdWUpKSB7XG4gICAgcmV0dXJuXG4gIH1cbiAgbGV0IG9iXG4gIGlmIChoYXNPd24odmFsdWUsICdfX29iX18nKSAmJiB2YWx1ZS5fX29iX18gaW5zdGFuY2VvZiBPYnNlcnZlcikge1xuICAgIG9iID0gdmFsdWUuX19vYl9fXG4gIH0gZWxzZSBpZiAoXG4gICAgKEFycmF5LmlzQXJyYXkodmFsdWUpIHx8IGlzUGxhaW5PYmplY3QodmFsdWUpKSAmJlxuICAgIE9iamVjdC5pc0V4dGVuc2libGUodmFsdWUpICYmXG4gICAgIXZhbHVlLl9pc1Z1ZVxuICApIHtcbiAgICBvYiA9IG5ldyBPYnNlcnZlcih2YWx1ZSlcbiAgfVxuICBpZiAob2IgJiYgdm0pIHtcbiAgICBvYi5hZGRWbSh2bSlcbiAgfVxuICByZXR1cm4gb2Jcbn1cblxuLyoqXG4gKiBEZWZpbmUgYSByZWFjdGl2ZSBwcm9wZXJ0eSBvbiBhbiBPYmplY3QuXG4gKlxuICogQHBhcmFtIHtPYmplY3R9IG9ialxuICogQHBhcmFtIHtTdHJpbmd9IGtleVxuICogQHBhcmFtIHsqfSB2YWxcbiAqL1xuXG5leHBvcnQgZnVuY3Rpb24gZGVmaW5lUmVhY3RpdmUgKG9iaiwga2V5LCB2YWwpIHtcbiAgY29uc3QgZGVwID0gbmV3IERlcCgpXG5cbiAgY29uc3QgcHJvcGVydHkgPSBPYmplY3QuZ2V0T3duUHJvcGVydHlEZXNjcmlwdG9yKG9iaiwga2V5KVxuICBpZiAocHJvcGVydHkgJiYgcHJvcGVydHkuY29uZmlndXJhYmxlID09PSBmYWxzZSkge1xuICAgIHJldHVyblxuICB9XG5cbiAgLy8gY2F0ZXIgZm9yIHByZS1kZWZpbmVkIGdldHRlci9zZXR0ZXJzXG4gIGNvbnN0IGdldHRlciA9IHByb3BlcnR5ICYmIHByb3BlcnR5LmdldFxuICBjb25zdCBzZXR0ZXIgPSBwcm9wZXJ0eSAmJiBwcm9wZXJ0eS5zZXRcblxuICBsZXQgY2hpbGRPYiA9IG9ic2VydmUodmFsKVxuICBPYmplY3QuZGVmaW5lUHJvcGVydHkob2JqLCBrZXksIHtcbiAgICBlbnVtZXJhYmxlOiB0cnVlLFxuICAgIGNvbmZpZ3VyYWJsZTogdHJ1ZSxcbiAgICBnZXQ6IGZ1bmN0aW9uIHJlYWN0aXZlR2V0dGVyICgpIHtcbiAgICAgIGNvbnN0IHZhbHVlID0gZ2V0dGVyID8gZ2V0dGVyLmNhbGwob2JqKSA6IHZhbFxuICAgICAgaWYgKERlcC50YXJnZXQpIHtcbiAgICAgICAgZGVwLmRlcGVuZCgpXG4gICAgICAgIGlmIChjaGlsZE9iKSB7XG4gICAgICAgICAgY2hpbGRPYi5kZXAuZGVwZW5kKClcbiAgICAgICAgfVxuICAgICAgICBpZiAoQXJyYXkuaXNBcnJheSh2YWx1ZSkpIHtcbiAgICAgICAgICBmb3IgKGxldCBlLCBpID0gMCwgbCA9IHZhbHVlLmxlbmd0aDsgaSA8IGw7IGkrKykge1xuICAgICAgICAgICAgZSA9IHZhbHVlW2ldXG4gICAgICAgICAgICBlICYmIGUuX19vYl9fICYmIGUuX19vYl9fLmRlcC5kZXBlbmQoKVxuICAgICAgICAgIH1cbiAgICAgICAgfVxuICAgICAgfVxuICAgICAgcmV0dXJuIHZhbHVlXG4gICAgfSxcbiAgICBzZXQ6IGZ1bmN0aW9uIHJlYWN0aXZlU2V0dGVyIChuZXdWYWwpIHtcbiAgICAgIGNvbnN0IHZhbHVlID0gZ2V0dGVyID8gZ2V0dGVyLmNhbGwob2JqKSA6IHZhbFxuICAgICAgaWYgKG5ld1ZhbCA9PT0gdmFsdWUpIHtcbiAgICAgICAgcmV0dXJuXG4gICAgICB9XG4gICAgICBpZiAoc2V0dGVyKSB7XG4gICAgICAgIHNldHRlci5jYWxsKG9iaiwgbmV3VmFsKVxuICAgICAgfSBlbHNlIHtcbiAgICAgICAgdmFsID0gbmV3VmFsXG4gICAgICB9XG4gICAgICBjaGlsZE9iID0gb2JzZXJ2ZShuZXdWYWwpXG4gICAgICBkZXAubm90aWZ5KClcbiAgICB9XG4gIH0pXG59XG5cbi8qKlxuICogU2V0IGEgcHJvcGVydHkgb24gYW4gb2JqZWN0LiBBZGRzIHRoZSBuZXcgcHJvcGVydHkgYW5kXG4gKiB0cmlnZ2VycyBjaGFuZ2Ugbm90aWZpY2F0aW9uIGlmIHRoZSBwcm9wZXJ0eSBkb2Vzbid0XG4gKiBhbHJlYWR5IGV4aXN0LlxuICpcbiAqIEBwYXJhbSB7T2JqZWN0fSBvYmpcbiAqIEBwYXJhbSB7U3RyaW5nfSBrZXlcbiAqIEBwYXJhbSB7Kn0gdmFsXG4gKiBAcHVibGljXG4gKi9cblxuZXhwb3J0IGZ1bmN0aW9uIHNldCAob2JqLCBrZXksIHZhbCkge1xuICBpZiAoQXJyYXkuaXNBcnJheShvYmopKSB7XG4gICAgcmV0dXJuIG9iai5zcGxpY2Uoa2V5LCAxLCB2YWwpXG4gIH1cbiAgaWYgKGhhc093bihvYmosIGtleSkpIHtcbiAgICBvYmpba2V5XSA9IHZhbFxuICAgIHJldHVyblxuICB9XG4gIGlmIChvYmouX2lzVnVlKSB7XG4gICAgc2V0KG9iai5fZGF0YSwga2V5LCB2YWwpXG4gICAgcmV0dXJuXG4gIH1cbiAgY29uc3Qgb2IgPSBvYmouX19vYl9fXG4gIGlmICghb2IpIHtcbiAgICBvYmpba2V5XSA9IHZhbFxuICAgIHJldHVyblxuICB9XG4gIG9iLmNvbnZlcnQoa2V5LCB2YWwpXG4gIG9iLmRlcC5ub3RpZnkoKVxuICBpZiAob2Iudm1zKSB7XG4gICAgbGV0IGkgPSBvYi52bXMubGVuZ3RoXG4gICAgd2hpbGUgKGktLSkge1xuICAgICAgY29uc3Qgdm0gPSBvYi52bXNbaV1cbiAgICAgIHByb3h5KHZtLCBrZXkpXG4gICAgICB2bS4kZm9yY2VVcGRhdGUoKVxuICAgIH1cbiAgfVxuICByZXR1cm4gdmFsXG59XG5cbi8qKlxuICogRGVsZXRlIGEgcHJvcGVydHkgYW5kIHRyaWdnZXIgY2hhbmdlIGlmIG5lY2Vzc2FyeS5cbiAqXG4gKiBAcGFyYW0ge09iamVjdH0gb2JqXG4gKiBAcGFyYW0ge1N0cmluZ30ga2V5XG4gKi9cblxuZXhwb3J0IGZ1bmN0aW9uIGRlbCAob2JqLCBrZXkpIHtcbiAgaWYgKCFoYXNPd24ob2JqLCBrZXkpKSB7XG4gICAgcmV0dXJuXG4gIH1cbiAgZGVsZXRlIG9ialtrZXldXG4gIGNvbnN0IG9iID0gb2JqLl9fb2JfX1xuXG4gIGlmICghb2IpIHtcbiAgICBpZiAob2JqLl9pc1Z1ZSkge1xuICAgICAgZGVsZXRlIG9iai5fZGF0YVtrZXldXG4gICAgICBvYmouJGZvcmNlVXBkYXRlKClcbiAgICB9XG4gICAgcmV0dXJuXG4gIH1cbiAgb2IuZGVwLm5vdGlmeSgpXG4gIGlmIChvYi52bXMpIHtcbiAgICBsZXQgaSA9IG9iLnZtcy5sZW5ndGhcbiAgICB3aGlsZSAoaS0tKSB7XG4gICAgICBjb25zdCB2bSA9IG9iLnZtc1tpXVxuICAgICAgdW5wcm94eSh2bSwga2V5KVxuICAgICAgdm0uJGZvcmNlVXBkYXRlKClcbiAgICB9XG4gIH1cbn1cblxuY29uc3QgS0VZX1dPUkRTID0gWyckaW5kZXgnLCAnJHZhbHVlJywgJyRldmVudCddXG5leHBvcnQgZnVuY3Rpb24gcHJveHkgKHZtLCBrZXkpIHtcbiAgaWYgKEtFWV9XT1JEUy5pbmRleE9mKGtleSkgPiAtMSB8fCAhaXNSZXNlcnZlZChrZXkpKSB7XG4gICAgT2JqZWN0LmRlZmluZVByb3BlcnR5KHZtLCBrZXksIHtcbiAgICAgIGNvbmZpZ3VyYWJsZTogdHJ1ZSxcbiAgICAgIGVudW1lcmFibGU6IHRydWUsXG4gICAgICBnZXQ6IGZ1bmN0aW9uIHByb3h5R2V0dGVyICgpIHtcbiAgICAgICAgcmV0dXJuIHZtLl9kYXRhW2tleV1cbiAgICAgIH0sXG4gICAgICBzZXQ6IGZ1bmN0aW9uIHByb3h5U2V0dGVyICh2YWwpIHtcbiAgICAgICAgdm0uX2RhdGFba2V5XSA9IHZhbFxuICAgICAgfVxuICAgIH0pXG4gIH1cbn1cblxuZXhwb3J0IGZ1bmN0aW9uIHVucHJveHkgKHZtLCBrZXkpIHtcbiAgaWYgKCFpc1Jlc2VydmVkKGtleSkpIHtcbiAgICBkZWxldGUgdm1ba2V5XVxuICB9XG59XG5cblxuXG4vKiogV0VCUEFDSyBGT09URVIgKipcbiAqKiAvVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvc3JjL2RlZmF1bHQvY29yZS9vYnNlcnZlci5qc1xuICoqLyIsIi8qIGVzbGludC1kaXNhYmxlICovXG5cbmltcG9ydCB7IGRlZiB9IGZyb20gJy4uL3V0aWwnXG5cbmNvbnN0IGFycmF5UHJvdG8gPSBBcnJheS5wcm90b3R5cGVcbmV4cG9ydCBjb25zdCBhcnJheU1ldGhvZHMgPSBPYmplY3QuY3JlYXRlKGFycmF5UHJvdG8pXG5cbi8qKlxuICogSW50ZXJjZXB0IG11dGF0aW5nIG1ldGhvZHMgYW5kIGVtaXQgZXZlbnRzXG4gKi9cblxuO1tcbiAgJ3B1c2gnLFxuICAncG9wJyxcbiAgJ3NoaWZ0JyxcbiAgJ3Vuc2hpZnQnLFxuICAnc3BsaWNlJyxcbiAgJ3NvcnQnLFxuICAncmV2ZXJzZSdcbl1cbi5mb3JFYWNoKGZ1bmN0aW9uIChtZXRob2QpIHtcbiAgLy8gY2FjaGUgb3JpZ2luYWwgbWV0aG9kXG4gIGNvbnN0IG9yaWdpbmFsID0gYXJyYXlQcm90b1ttZXRob2RdXG4gIGRlZihhcnJheU1ldGhvZHMsIG1ldGhvZCwgZnVuY3Rpb24gbXV0YXRvciAoKSB7XG4gICAgLy8gYXZvaWQgbGVha2luZyBhcmd1bWVudHM6XG4gICAgLy8gaHR0cDovL2pzcGVyZi5jb20vY2xvc3VyZS13aXRoLWFyZ3VtZW50c1xuICAgIGxldCBpID0gYXJndW1lbnRzLmxlbmd0aFxuICAgIGNvbnN0IGFyZ3MgPSBuZXcgQXJyYXkoaSlcbiAgICB3aGlsZSAoaS0tKSB7XG4gICAgICBhcmdzW2ldID0gYXJndW1lbnRzW2ldXG4gICAgfVxuICAgIGNvbnN0IHJlc3VsdCA9IG9yaWdpbmFsLmFwcGx5KHRoaXMsIGFyZ3MpXG4gICAgY29uc3Qgb2IgPSB0aGlzLl9fb2JfX1xuICAgIGxldCBpbnNlcnRlZFxuICAgIHN3aXRjaCAobWV0aG9kKSB7XG4gICAgICBjYXNlICdwdXNoJzpcbiAgICAgICAgaW5zZXJ0ZWQgPSBhcmdzXG4gICAgICAgIGJyZWFrXG4gICAgICBjYXNlICd1bnNoaWZ0JzpcbiAgICAgICAgaW5zZXJ0ZWQgPSBhcmdzXG4gICAgICAgIGJyZWFrXG4gICAgICBjYXNlICdzcGxpY2UnOlxuICAgICAgICBpbnNlcnRlZCA9IGFyZ3Muc2xpY2UoMilcbiAgICAgICAgYnJlYWtcbiAgICB9XG4gICAgaWYgKGluc2VydGVkKSBvYi5vYnNlcnZlQXJyYXkoaW5zZXJ0ZWQpXG4gICAgLy8gbm90aWZ5IGNoYW5nZVxuICAgIG9iLmRlcC5ub3RpZnkoKVxuICAgIHJldHVybiByZXN1bHRcbiAgfSlcbn0pXG5cbi8qKlxuICogU3dhcCB0aGUgZWxlbWVudCBhdCB0aGUgZ2l2ZW4gaW5kZXggd2l0aCBhIG5ldyB2YWx1ZVxuICogYW5kIGVtaXRzIGNvcnJlc3BvbmRpbmcgZXZlbnQuXG4gKlxuICogQHBhcmFtIHtOdW1iZXJ9IGluZGV4XG4gKiBAcGFyYW0geyp9IHZhbFxuICogQHJldHVybiB7Kn0gLSByZXBsYWNlZCBlbGVtZW50XG4gKi9cblxuZGVmKFxuICBhcnJheVByb3RvLFxuICAnJHNldCcsXG4gIGZ1bmN0aW9uICRzZXQgKGluZGV4LCB2YWwpIHtcbiAgICBpZiAoaW5kZXggPj0gdGhpcy5sZW5ndGgpIHtcbiAgICAgIHRoaXMubGVuZ3RoID0gaW5kZXggKyAxXG4gICAgfVxuICAgIHJldHVybiB0aGlzLnNwbGljZShpbmRleCwgMSwgdmFsKVswXVxuICB9XG4pXG5cbi8qKlxuICogQ29udmVuaWVuY2UgbWV0aG9kIHRvIHJlbW92ZSB0aGUgZWxlbWVudCBhdCBnaXZlbiBpbmRleC5cbiAqXG4gKiBAcGFyYW0ge051bWJlcn0gaW5kZXhcbiAqIEBwYXJhbSB7Kn0gdmFsXG4gKi9cblxuZGVmKFxuICBhcnJheVByb3RvLFxuICAnJHJlbW92ZScsXG4gIGZ1bmN0aW9uICRyZW1vdmUgKGluZGV4KSB7XG4gICAgLyogaXN0YW5idWwgaWdub3JlIGlmICovXG4gICAgaWYgKCF0aGlzLmxlbmd0aCkgcmV0dXJuXG4gICAgaWYgKHR5cGVvZiBpbmRleCAhPT0gJ251bWJlcicpIHtcbiAgICAgIGluZGV4ID0gdGhpcy5pbmRleE9mKGluZGV4KVxuICAgIH1cbiAgICBpZiAoaW5kZXggPiAtMSkge1xuICAgICAgdGhpcy5zcGxpY2UoaW5kZXgsIDEpXG4gICAgfVxuICB9XG4pXG5cblxuXG4vKiogV0VCUEFDSyBGT09URVIgKipcbiAqKiAvVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvc3JjL2RlZmF1bHQvY29yZS9hcnJheS5qc1xuICoqLyIsIi8qKlxuICogQGZpbGVPdmVydmlld1xuICogVmlld01vZGVsIHRlbXBsYXRlIHBhcnNlciAmIGRhdGEtYmluZGluZyBwcm9jZXNzXG4gKi9cblxuaW1wb3J0IHtcbiAgZXh0ZW5kLFxuICBiaW5kXG59IGZyb20gJy4uL3V0aWwnXG5pbXBvcnQge1xuICBpbml0RGF0YSxcbiAgaW5pdENvbXB1dGVkXG59IGZyb20gJy4uL2NvcmUvc3RhdGUnXG5pbXBvcnQge1xuICBiaW5kRWxlbWVudCxcbiAgc2V0SWQsXG4gIGJpbmRTdWJWbSxcbiAgYmluZFN1YlZtQWZ0ZXJJbml0aWFsaXplZCxcbiAgYXBwbHlOYWl0dmVDb21wb25lbnRPcHRpb25zLFxuICB3YXRjaFxufSBmcm9tICcuL2RpcmVjdGl2ZSdcbmltcG9ydCB7XG4gIGNyZWF0ZUJsb2NrLFxuICBjcmVhdGVCb2R5LFxuICBjcmVhdGVFbGVtZW50LFxuICBhdHRhY2hUYXJnZXQsXG4gIG1vdmVUYXJnZXQsXG4gIHJlbW92ZVRhcmdldFxufSBmcm9tICcuL2RvbS1oZWxwZXInXG5cbi8qKlxuICogYnVpbGQoKVxuICogICBjb21waWxlKHRlbXBsYXRlLCBwYXJlbnROb2RlKVxuICogICAgIGlmICh0eXBlIGlzIGNvbnRlbnQpIGNyZWF0ZSBjb250ZW50Tm9kZVxuICogICAgIGVsc2UgaWYgKGRpcnMgaGF2ZSB2LWZvcikgZm9yZWFjaCAtPiBjcmVhdGUgY29udGV4dFxuICogICAgICAgLT4gY29tcGlsZSh0ZW1wbGF0ZVdpdGhvdXRGb3IsIHBhcmVudE5vZGUpOiBkaWZmKGxpc3QpIG9uY2hhbmdlXG4gKiAgICAgZWxzZSBpZiAoZGlycyBoYXZlIHYtaWYpIGFzc2VydFxuICogICAgICAgLT4gY29tcGlsZSh0ZW1wbGF0ZVdpdGhvdXRJZiwgcGFyZW50Tm9kZSk6IHRvZ2dsZShzaG93bikgb25jaGFuZ2VcbiAqICAgICBlbHNlIGlmICh0eXBlIGlzIGR5bmFtaWMpXG4gKiAgICAgICAtPiBjb21waWxlKHRlbXBsYXRlV2l0aG91dER5bmFtaWNUeXBlLCBwYXJlbnROb2RlKTogd2F0Y2godHlwZSkgb25jaGFuZ2VcbiAqICAgICBlbHNlIGlmICh0eXBlIGlzIGN1c3RvbSlcbiAqICAgICAgIGFkZENoaWxkVm0odm0sIHBhcmVudFZtKVxuICogICAgICAgYnVpbGQoZXh0ZXJuYWxEaXJzKVxuICogICAgICAgZm9yZWFjaCBjaGlsZE5vZGVzIC0+IGNvbXBpbGUoY2hpbGROb2RlLCB0ZW1wbGF0ZSlcbiAqICAgICBlbHNlIGlmICh0eXBlIGlzIG5hdGl2ZSlcbiAqICAgICAgIHNldChkaXJzKTogdXBkYXRlKGlkL2F0dHIvc3R5bGUvY2xhc3MpIG9uY2hhbmdlXG4gKiAgICAgICBhcHBlbmQodGVtcGxhdGUsIHBhcmVudE5vZGUpXG4gKiAgICAgICBmb3JlYWNoIGNoaWxkTm9kZXMgLT4gY29tcGlsZShjaGlsZE5vZGUsIHRlbXBsYXRlKVxuICovXG5leHBvcnQgZnVuY3Rpb24gYnVpbGQgKHZtKSB7XG4gIGNvbnN0IG9wdCA9IHZtLl9vcHRpb25zIHx8IHt9XG4gIGNvbnN0IHRlbXBsYXRlID0gb3B0LnRlbXBsYXRlIHx8IHt9XG5cbiAgaWYgKG9wdC5yZXBsYWNlKSB7XG4gICAgaWYgKHRlbXBsYXRlLmNoaWxkcmVuICYmIHRlbXBsYXRlLmNoaWxkcmVuLmxlbmd0aCA9PT0gMSkge1xuICAgICAgY29tcGlsZSh2bSwgdGVtcGxhdGUuY2hpbGRyZW5bMF0sIHZtLl9wYXJlbnRFbClcbiAgICB9XG4gICAgZWxzZSB7XG4gICAgICBjb21waWxlKHZtLCB0ZW1wbGF0ZS5jaGlsZHJlbiwgdm0uX3BhcmVudEVsKVxuICAgIH1cbiAgfVxuICBlbHNlIHtcbiAgICBjb21waWxlKHZtLCB0ZW1wbGF0ZSwgdm0uX3BhcmVudEVsKVxuICB9XG5cbiAgY29uc29sZS5kZWJ1ZyhgW0pTIEZyYW1ld29ya10gXCJyZWFkeVwiIGxpZmVjeWNsZSBpbiBWbSgke3ZtLl90eXBlfSlgKVxuICB2bS4kZW1pdCgnaG9vazpyZWFkeScpXG4gIHZtLl9yZWFkeSA9IHRydWVcbn1cblxuLyoqXG4gKiBHZW5lcmF0ZSBlbGVtZW50cyBieSBjaGlsZCBvciBjaGlsZHJlbiBhbmQgYXBwZW5kIHRvIHBhcmVudCBlbGVtZW50cy5cbiAqIFJvb3QgZWxlbWVudCBpbmZvIHdvdWxkIGJlIG1lcmdlZCBpZiBoYXMuIFRoZSBmaXJzdCBhcmd1bWVudCBtYXkgYmUgYW4gYXJyYXlcbiAqIGlmIHRoZSByb290IGVsZW1lbnQgd2l0aCBvcHRpb25zLnJlcGxhY2UgaGFzIG5vdCBvbmx5IG9uZSBjaGlsZC5cbiAqXG4gKiBAcGFyYW0ge29iamVjdHxhcnJheX0gdGFyZ2V0XG4gKiBAcGFyYW0ge29iamVjdH0gICAgICAgZGVzdFxuICogQHBhcmFtIHtvYmplY3R9ICAgICAgIG1ldGFcbiAqL1xuZnVuY3Rpb24gY29tcGlsZSAodm0sIHRhcmdldCwgZGVzdCwgbWV0YSkge1xuICBjb25zdCBhcHAgPSB2bS5fYXBwIHx8IHt9XG5cbiAgaWYgKGFwcC5sYXN0U2lnbmFsID09PSAtMSkge1xuICAgIHJldHVyblxuICB9XG5cbiAgaWYgKHRhcmdldElzRnJhZ21lbnQodGFyZ2V0KSkge1xuICAgIGNvbXBpbGVGcmFnbWVudCh2bSwgdGFyZ2V0LCBkZXN0LCBtZXRhKVxuICAgIHJldHVyblxuICB9XG4gIG1ldGEgPSBtZXRhIHx8IHt9XG4gIGlmICh0YXJnZXRJc0NvbnRlbnQodGFyZ2V0KSkge1xuICAgIGNvbnNvbGUuZGVidWcoJ1tKUyBGcmFtZXdvcmtdIGNvbXBpbGUgXCJjb250ZW50XCIgYmxvY2sgYnknLCB0YXJnZXQpXG4gICAgdm0uX2NvbnRlbnQgPSBjcmVhdGVCbG9jayh2bSwgZGVzdClcbiAgICByZXR1cm5cbiAgfVxuXG4gIGlmICh0YXJnZXROZWVkQ2hlY2tSZXBlYXQodGFyZ2V0LCBtZXRhKSkge1xuICAgIGNvbnNvbGUuZGVidWcoJ1tKUyBGcmFtZXdvcmtdIGNvbXBpbGUgXCJyZXBlYXRcIiBsb2dpYyBieScsIHRhcmdldClcbiAgICBjb21waWxlUmVwZWF0KHZtLCB0YXJnZXQsIGRlc3QpXG4gICAgcmV0dXJuXG4gIH1cbiAgaWYgKHRhcmdldE5lZWRDaGVja1Nob3duKHRhcmdldCwgbWV0YSkpIHtcbiAgICBjb25zb2xlLmRlYnVnKCdbSlMgRnJhbWV3b3JrXSBjb21waWxlIFwiaWZcIiBsb2dpYyBieScsIHRhcmdldClcbiAgICBjb21waWxlU2hvd24odm0sIHRhcmdldCwgZGVzdCwgbWV0YSlcbiAgICByZXR1cm5cbiAgfVxuICBjb25zdCB0eXBlR2V0dGVyID0gbWV0YS50eXBlIHx8IHRhcmdldC50eXBlXG4gIGlmICh0YXJnZXROZWVkQ2hlY2tUeXBlKHR5cGVHZXR0ZXIsIG1ldGEpKSB7XG4gICAgY29tcGlsZVR5cGUodm0sIHRhcmdldCwgZGVzdCwgdHlwZUdldHRlciwgbWV0YSlcbiAgICByZXR1cm5cbiAgfVxuICBjb25zdCB0eXBlID0gdHlwZUdldHRlclxuICBjb25zdCBjb21wb25lbnQgPSB0YXJnZXRJc0NvbXBvc2VkKHZtLCB0YXJnZXQsIHR5cGUpXG4gIGlmIChjb21wb25lbnQpIHtcbiAgICBjb25zb2xlLmRlYnVnKCdbSlMgRnJhbWV3b3JrXSBjb21waWxlIGNvbXBvc2VkIGNvbXBvbmVudCBieScsIHRhcmdldClcbiAgICBjb21waWxlQ3VzdG9tQ29tcG9uZW50KHZtLCBjb21wb25lbnQsIHRhcmdldCwgZGVzdCwgdHlwZSwgbWV0YSlcbiAgICByZXR1cm5cbiAgfVxuICBjb25zb2xlLmRlYnVnKCdbSlMgRnJhbWV3b3JrXSBjb21waWxlIG5hdGl2ZSBjb21wb25lbnQgYnknLCB0YXJnZXQpXG4gIGNvbXBpbGVOYXRpdmVDb21wb25lbnQodm0sIHRhcmdldCwgZGVzdCwgdHlwZSlcbn1cblxuLyoqXG4gKiBDaGVjayBpZiB0YXJnZXQgaXMgYSBmcmFnbWVudCAoYW4gYXJyYXkpLlxuICpcbiAqIEBwYXJhbSAge29iamVjdH0gIHRhcmdldFxuICogQHJldHVybiB7Ym9vbGVhbn1cbiAqL1xuZnVuY3Rpb24gdGFyZ2V0SXNGcmFnbWVudCAodGFyZ2V0KSB7XG4gIHJldHVybiBBcnJheS5pc0FycmF5KHRhcmdldClcbn1cblxuLyoqXG4gKiBDaGVjayBpZiB0YXJnZXQgdHlwZSBpcyBjb250ZW50L3Nsb3QuXG4gKlxuICogQHBhcmFtICB7b2JqZWN0fSAgdGFyZ2V0XG4gKiBAcmV0dXJuIHtib29sZWFufVxuICovXG5mdW5jdGlvbiB0YXJnZXRJc0NvbnRlbnQgKHRhcmdldCkge1xuICByZXR1cm4gdGFyZ2V0LnR5cGUgPT09ICdjb250ZW50JyB8fCB0YXJnZXQudHlwZSA9PT0gJ3Nsb3QnXG59XG5cbi8qKlxuICogQ2hlY2sgaWYgdGFyZ2V0IG5lZWQgdG8gY29tcGlsZSBieSBhIGxpc3QuXG4gKlxuICogQHBhcmFtICB7b2JqZWN0fSAgdGFyZ2V0XG4gKiBAcGFyYW0gIHtvYmplY3R9ICBtZXRhXG4gKiBAcmV0dXJuIHtib29sZWFufVxuICovXG5mdW5jdGlvbiB0YXJnZXROZWVkQ2hlY2tSZXBlYXQgKHRhcmdldCwgbWV0YSkge1xuICByZXR1cm4gIW1ldGEuaGFzT3duUHJvcGVydHkoJ3JlcGVhdCcpICYmIHRhcmdldC5yZXBlYXRcbn1cblxuLyoqXG4gKiBDaGVjayBpZiB0YXJnZXQgbmVlZCB0byBjb21waWxlIGJ5IGEgYm9vbGVhbiB2YWx1ZS5cbiAqXG4gKiBAcGFyYW0gIHtvYmplY3R9ICB0YXJnZXRcbiAqIEBwYXJhbSAge29iamVjdH0gIG1ldGFcbiAqIEByZXR1cm4ge2Jvb2xlYW59XG4gKi9cbmZ1bmN0aW9uIHRhcmdldE5lZWRDaGVja1Nob3duICh0YXJnZXQsIG1ldGEpIHtcbiAgcmV0dXJuICFtZXRhLmhhc093blByb3BlcnR5KCdzaG93bicpICYmIHRhcmdldC5zaG93blxufVxuXG4vKipcbiAqIENoZWNrIGlmIHRhcmdldCBuZWVkIHRvIGNvbXBpbGUgYnkgYSBkeW5hbWljIHR5cGUuXG4gKlxuICogQHBhcmFtICB7c3RyaW5nfGZ1bmN0aW9ufSB0eXBlR2V0dGVyXG4gKiBAcGFyYW0gIHtvYmplY3R9ICAgICAgICAgIG1ldGFcbiAqIEByZXR1cm4ge2Jvb2xlYW59XG4gKi9cbmZ1bmN0aW9uIHRhcmdldE5lZWRDaGVja1R5cGUgKHR5cGVHZXR0ZXIsIG1ldGEpIHtcbiAgcmV0dXJuICh0eXBlb2YgdHlwZUdldHRlciA9PT0gJ2Z1bmN0aW9uJykgJiYgIW1ldGEuaGFzT3duUHJvcGVydHkoJ3R5cGUnKVxufVxuXG4vKipcbiAqIENoZWNrIGlmIHRoaXMga2luZCBvZiBjb21wb25lbnQgaXMgY29tcG9zZWQuXG4gKlxuICogQHBhcmFtICB7c3RyaW5nfSAgdHlwZVxuICogQHJldHVybiB7Ym9vbGVhbn1cbiAqL1xuZnVuY3Rpb24gdGFyZ2V0SXNDb21wb3NlZCAodm0sIHRhcmdldCwgdHlwZSkge1xuICBsZXQgY29tcG9uZW50XG4gIGlmICh2bS5fYXBwICYmIHZtLl9hcHAuY3VzdG9tQ29tcG9uZW50TWFwKSB7XG4gICAgY29tcG9uZW50ID0gdm0uX2FwcC5jdXN0b21Db21wb25lbnRNYXBbdHlwZV1cbiAgfVxuICBpZiAodm0uX29wdGlvbnMgJiYgdm0uX29wdGlvbnMuY29tcG9uZW50cykge1xuICAgIGNvbXBvbmVudCA9IHZtLl9vcHRpb25zLmNvbXBvbmVudHNbdHlwZV1cbiAgfVxuICBpZiAodGFyZ2V0LmNvbXBvbmVudCkge1xuICAgIGNvbXBvbmVudCA9IGNvbXBvbmVudCB8fCB7fVxuICB9XG4gIHJldHVybiBjb21wb25lbnRcbn1cblxuLyoqXG4gKiBDb21waWxlIGEgbGlzdCBvZiB0YXJnZXRzLlxuICpcbiAqIEBwYXJhbSB7b2JqZWN0fSB0YXJnZXRcbiAqIEBwYXJhbSB7b2JqZWN0fSBkZXN0XG4gKiBAcGFyYW0ge29iamVjdH0gbWV0YVxuICovXG5mdW5jdGlvbiBjb21waWxlRnJhZ21lbnQgKHZtLCB0YXJnZXQsIGRlc3QsIG1ldGEpIHtcbiAgY29uc3QgZnJhZ0Jsb2NrID0gY3JlYXRlQmxvY2sodm0sIGRlc3QpXG4gIHRhcmdldC5mb3JFYWNoKChjaGlsZCkgPT4ge1xuICAgIGNvbXBpbGUodm0sIGNoaWxkLCBmcmFnQmxvY2ssIG1ldGEpXG4gIH0pXG59XG5cbi8qKlxuICogQ29tcGlsZSBhIHRhcmdldCB3aXRoIHJlcGVhdCBkaXJlY3RpdmUuXG4gKlxuICogQHBhcmFtIHtvYmplY3R9IHRhcmdldFxuICogQHBhcmFtIHtvYmplY3R9IGRlc3RcbiAqL1xuZnVuY3Rpb24gY29tcGlsZVJlcGVhdCAodm0sIHRhcmdldCwgZGVzdCkge1xuICBjb25zdCByZXBlYXQgPSB0YXJnZXQucmVwZWF0XG4gIGNvbnN0IG9sZFN0eWxlID0gdHlwZW9mIHJlcGVhdCA9PT0gJ2Z1bmN0aW9uJ1xuICBsZXQgZ2V0dGVyID0gcmVwZWF0LmdldHRlciB8fCByZXBlYXQuZXhwcmVzc2lvbiB8fCByZXBlYXRcbiAgaWYgKHR5cGVvZiBnZXR0ZXIgIT09ICdmdW5jdGlvbicpIHtcbiAgICBnZXR0ZXIgPSBmdW5jdGlvbiAoKSB7IHJldHVybiBbXSB9XG4gIH1cbiAgY29uc3Qga2V5ID0gcmVwZWF0LmtleSB8fCAnJGluZGV4J1xuICBjb25zdCB2YWx1ZSA9IHJlcGVhdC52YWx1ZSB8fCAnJHZhbHVlJ1xuICBjb25zdCB0cmFja0J5ID0gcmVwZWF0LnRyYWNrQnkgfHwgdGFyZ2V0LnRyYWNrQnkgfHxcbiAgICAodGFyZ2V0LmF0dHIgJiYgdGFyZ2V0LmF0dHIudHJhY2tCeSlcblxuICBjb25zdCBmcmFnQmxvY2sgPSBjcmVhdGVCbG9jayh2bSwgZGVzdClcbiAgZnJhZ0Jsb2NrLmNoaWxkcmVuID0gW11cbiAgZnJhZ0Jsb2NrLmRhdGEgPSBbXVxuICBmcmFnQmxvY2sudm1zID0gW11cblxuICBiaW5kUmVwZWF0KHZtLCB0YXJnZXQsIGZyYWdCbG9jaywgeyBnZXR0ZXIsIGtleSwgdmFsdWUsIHRyYWNrQnksIG9sZFN0eWxlIH0pXG59XG5cbi8qKlxuICogQ29tcGlsZSBhIHRhcmdldCB3aXRoIGlmIGRpcmVjdGl2ZS5cbiAqXG4gKiBAcGFyYW0ge29iamVjdH0gdGFyZ2V0XG4gKiBAcGFyYW0ge29iamVjdH0gZGVzdFxuICogQHBhcmFtIHtvYmplY3R9IG1ldGFcbiAqL1xuZnVuY3Rpb24gY29tcGlsZVNob3duICh2bSwgdGFyZ2V0LCBkZXN0LCBtZXRhKSB7XG4gIGNvbnN0IG5ld01ldGEgPSB7IHNob3duOiB0cnVlIH1cbiAgY29uc3QgZnJhZ0Jsb2NrID0gY3JlYXRlQmxvY2sodm0sIGRlc3QpXG5cbiAgaWYgKGRlc3QuZWxlbWVudCAmJiBkZXN0LmNoaWxkcmVuKSB7XG4gICAgZGVzdC5jaGlsZHJlbi5wdXNoKGZyYWdCbG9jaylcbiAgfVxuXG4gIGlmIChtZXRhLnJlcGVhdCkge1xuICAgIG5ld01ldGEucmVwZWF0ID0gbWV0YS5yZXBlYXRcbiAgfVxuXG4gIGJpbmRTaG93bih2bSwgdGFyZ2V0LCBmcmFnQmxvY2ssIG5ld01ldGEpXG59XG5cbi8qKlxuICogQ29tcGlsZSBhIHRhcmdldCB3aXRoIGR5bmFtaWMgY29tcG9uZW50IHR5cGUuXG4gKlxuICogQHBhcmFtIHtvYmplY3R9ICAgdGFyZ2V0XG4gKiBAcGFyYW0ge29iamVjdH0gICBkZXN0XG4gKiBAcGFyYW0ge2Z1bmN0aW9ufSB0eXBlR2V0dGVyXG4gKi9cbmZ1bmN0aW9uIGNvbXBpbGVUeXBlICh2bSwgdGFyZ2V0LCBkZXN0LCB0eXBlR2V0dGVyLCBtZXRhKSB7XG4gIGNvbnN0IHR5cGUgPSB0eXBlR2V0dGVyLmNhbGwodm0pXG4gIGNvbnN0IG5ld01ldGEgPSBleHRlbmQoeyB0eXBlIH0sIG1ldGEpXG4gIGNvbnN0IGZyYWdCbG9jayA9IGNyZWF0ZUJsb2NrKHZtLCBkZXN0KVxuXG4gIGlmIChkZXN0LmVsZW1lbnQgJiYgZGVzdC5jaGlsZHJlbikge1xuICAgIGRlc3QuY2hpbGRyZW4ucHVzaChmcmFnQmxvY2spXG4gIH1cblxuICB3YXRjaCh2bSwgdHlwZUdldHRlciwgKHZhbHVlKSA9PiB7XG4gICAgY29uc3QgbmV3TWV0YSA9IGV4dGVuZCh7IHR5cGU6IHZhbHVlIH0sIG1ldGEpXG4gICAgcmVtb3ZlVGFyZ2V0KHZtLCBmcmFnQmxvY2ssIHRydWUpXG4gICAgY29tcGlsZSh2bSwgdGFyZ2V0LCBmcmFnQmxvY2ssIG5ld01ldGEpXG4gIH0pXG5cbiAgY29tcGlsZSh2bSwgdGFyZ2V0LCBmcmFnQmxvY2ssIG5ld01ldGEpXG59XG5cbi8qKlxuICogQ29tcGlsZSBhIGNvbXBvc2VkIGNvbXBvbmVudC5cbiAqXG4gKiBAcGFyYW0ge29iamVjdH0gdGFyZ2V0XG4gKiBAcGFyYW0ge29iamVjdH0gZGVzdFxuICogQHBhcmFtIHtzdHJpbmd9IHR5cGVcbiAqL1xuZnVuY3Rpb24gY29tcGlsZUN1c3RvbUNvbXBvbmVudCAodm0sIGNvbXBvbmVudCwgdGFyZ2V0LCBkZXN0LCB0eXBlLCBtZXRhKSB7XG4gIGNvbnN0IEN0b3IgPSB2bS5jb25zdHJ1Y3RvclxuICBjb25zdCBzdWJWbSA9IG5ldyBDdG9yKHR5cGUsIGNvbXBvbmVudCwgdm0sIGRlc3QsIHVuZGVmaW5lZCwge1xuICAgICdob29rOmluaXQnOiBmdW5jdGlvbiAoKSB7XG4gICAgICBzZXRJZCh2bSwgbnVsbCwgdGFyZ2V0LmlkLCB0aGlzKVxuICAgICAgLy8gYmluZCB0ZW1wbGF0ZSBlYXJsaWVyIGJlY2F1c2Ugb2YgbGlmZWN5Y2xlIGlzc3Vlc1xuICAgICAgdGhpcy5fZXh0ZXJuYWxCaW5kaW5nID0ge1xuICAgICAgICBwYXJlbnQ6IHZtLFxuICAgICAgICB0ZW1wbGF0ZTogdGFyZ2V0XG4gICAgICB9XG4gICAgfSxcbiAgICAnaG9vazpjcmVhdGVkJzogZnVuY3Rpb24gKCkge1xuICAgICAgYmluZFN1YlZtKHZtLCB0aGlzLCB0YXJnZXQsIG1ldGEucmVwZWF0KVxuICAgIH0sXG4gICAgJ2hvb2s6cmVhZHknOiBmdW5jdGlvbiAoKSB7XG4gICAgICBpZiAodGhpcy5fY29udGVudCkge1xuICAgICAgICBjb21waWxlQ2hpbGRyZW4odm0sIHRhcmdldCwgdGhpcy5fY29udGVudClcbiAgICAgIH1cbiAgICB9XG4gIH0pXG4gIGJpbmRTdWJWbUFmdGVySW5pdGlhbGl6ZWQodm0sIHN1YlZtLCB0YXJnZXQpXG59XG5cbi8qKlxuICogR2VuZXJhdGUgZWxlbWVudCBmcm9tIHRlbXBsYXRlIGFuZCBhdHRhY2ggdG8gdGhlIGRlc3QgaWYgbmVlZGVkLlxuICogVGhlIHRpbWUgdG8gYXR0YWNoIGRlcGVuZHMgb24gd2hldGhlciB0aGUgbW9kZSBzdGF0dXMgaXMgbm9kZSBvciB0cmVlLlxuICpcbiAqIEBwYXJhbSB7b2JqZWN0fSB0ZW1wbGF0ZVxuICogQHBhcmFtIHtvYmplY3R9IGRlc3RcbiAqIEBwYXJhbSB7c3RyaW5nfSB0eXBlXG4gKi9cbmZ1bmN0aW9uIGNvbXBpbGVOYXRpdmVDb21wb25lbnQgKHZtLCB0ZW1wbGF0ZSwgZGVzdCwgdHlwZSkge1xuICBhcHBseU5haXR2ZUNvbXBvbmVudE9wdGlvbnModGVtcGxhdGUpXG5cbiAgbGV0IGVsZW1lbnRcbiAgaWYgKGRlc3QucmVmID09PSAnX2RvY3VtZW50RWxlbWVudCcpIHtcbiAgICAvLyBpZiBpdHMgcGFyZW50IGlzIGRvY3VtZW50RWxlbWVudCB0aGVuIGl0J3MgYSBib2R5XG4gICAgY29uc29sZS5kZWJ1ZyhgW0pTIEZyYW1ld29ya10gY29tcGlsZSB0byBjcmVhdGUgYm9keSBmb3IgJHt0eXBlfWApXG4gICAgZWxlbWVudCA9IGNyZWF0ZUJvZHkodm0sIHR5cGUpXG4gIH1cbiAgZWxzZSB7XG4gICAgY29uc29sZS5kZWJ1ZyhgW0pTIEZyYW1ld29ya10gY29tcGlsZSB0byBjcmVhdGUgZWxlbWVudCBmb3IgJHt0eXBlfWApXG4gICAgZWxlbWVudCA9IGNyZWF0ZUVsZW1lbnQodm0sIHR5cGUpXG4gIH1cblxuICBpZiAoIXZtLl9yb290RWwpIHtcbiAgICB2bS5fcm9vdEVsID0gZWxlbWVudFxuICAgIC8vIGJpbmQgZXZlbnQgZWFybGllciBiZWNhdXNlIG9mIGxpZmVjeWNsZSBpc3N1ZXNcbiAgICBjb25zdCBiaW5kaW5nID0gdm0uX2V4dGVybmFsQmluZGluZyB8fCB7fVxuICAgIGNvbnN0IHRhcmdldCA9IGJpbmRpbmcudGVtcGxhdGVcbiAgICBjb25zdCBwYXJlbnRWbSA9IGJpbmRpbmcucGFyZW50XG4gICAgaWYgKHRhcmdldCAmJiB0YXJnZXQuZXZlbnRzICYmIHBhcmVudFZtICYmIGVsZW1lbnQpIHtcbiAgICAgIGZvciAoY29uc3QgdHlwZSBpbiB0YXJnZXQuZXZlbnRzKSB7XG4gICAgICAgIGNvbnN0IGhhbmRsZXIgPSBwYXJlbnRWbVt0YXJnZXQuZXZlbnRzW3R5cGVdXVxuICAgICAgICBpZiAoaGFuZGxlcikge1xuICAgICAgICAgIGVsZW1lbnQuYWRkRXZlbnQodHlwZSwgYmluZChoYW5kbGVyLCBwYXJlbnRWbSkpXG4gICAgICAgIH1cbiAgICAgIH1cbiAgICB9XG4gIH1cblxuICBiaW5kRWxlbWVudCh2bSwgZWxlbWVudCwgdGVtcGxhdGUpXG5cbiAgaWYgKHRlbXBsYXRlLmF0dHIgJiYgdGVtcGxhdGUuYXR0ci5hcHBlbmQpIHsgLy8gYmFja3dhcmQsIGFwcGVuZCBwcm9wIGluIGF0dHJcbiAgICB0ZW1wbGF0ZS5hcHBlbmQgPSB0ZW1wbGF0ZS5hdHRyLmFwcGVuZFxuICB9XG5cbiAgaWYgKHRlbXBsYXRlLmFwcGVuZCkgeyAvLyBnaXZlIHRoZSBhcHBlbmQgYXR0cmlidXRlIGZvciBpb3MgYWRhcHRhdGlvblxuICAgIGVsZW1lbnQuYXR0ciA9IGVsZW1lbnQuYXR0ciB8fCB7fVxuICAgIGVsZW1lbnQuYXR0ci5hcHBlbmQgPSB0ZW1wbGF0ZS5hcHBlbmRcbiAgfVxuXG4gIGNvbnN0IHRyZWVNb2RlID0gdGVtcGxhdGUuYXBwZW5kID09PSAndHJlZSdcbiAgY29uc3QgYXBwID0gdm0uX2FwcCB8fCB7fVxuICBpZiAoYXBwLmxhc3RTaWduYWwgIT09IC0xICYmICF0cmVlTW9kZSkge1xuICAgIGNvbnNvbGUuZGVidWcoJ1tKUyBGcmFtZXdvcmtdIGNvbXBpbGUgdG8gYXBwZW5kIHNpbmdsZSBub2RlIGZvcicsIGVsZW1lbnQpXG4gICAgYXBwLmxhc3RTaWduYWwgPSBhdHRhY2hUYXJnZXQodm0sIGVsZW1lbnQsIGRlc3QpXG4gIH1cbiAgaWYgKGFwcC5sYXN0U2lnbmFsICE9PSAtMSkge1xuICAgIGNvbXBpbGVDaGlsZHJlbih2bSwgdGVtcGxhdGUsIGVsZW1lbnQpXG4gIH1cbiAgaWYgKGFwcC5sYXN0U2lnbmFsICE9PSAtMSAmJiB0cmVlTW9kZSkge1xuICAgIGNvbnNvbGUuZGVidWcoJ1tKUyBGcmFtZXdvcmtdIGNvbXBpbGUgdG8gYXBwZW5kIHdob2xlIHRyZWUgZm9yJywgZWxlbWVudClcbiAgICBhcHAubGFzdFNpZ25hbCA9IGF0dGFjaFRhcmdldCh2bSwgZWxlbWVudCwgZGVzdClcbiAgfVxufVxuXG4vKipcbiAqIFNldCBhbGwgY2hpbGRyZW4gdG8gYSBjZXJ0YWluIHBhcmVudCBlbGVtZW50LlxuICpcbiAqIEBwYXJhbSB7b2JqZWN0fSB0ZW1wbGF0ZVxuICogQHBhcmFtIHtvYmplY3R9IGRlc3RcbiAqL1xuZnVuY3Rpb24gY29tcGlsZUNoaWxkcmVuICh2bSwgdGVtcGxhdGUsIGRlc3QpIHtcbiAgY29uc3QgYXBwID0gdm0uX2FwcCB8fCB7fVxuICBjb25zdCBjaGlsZHJlbiA9IHRlbXBsYXRlLmNoaWxkcmVuXG4gIGlmIChjaGlsZHJlbiAmJiBjaGlsZHJlbi5sZW5ndGgpIHtcbiAgICBjaGlsZHJlbi5ldmVyeSgoY2hpbGQpID0+IHtcbiAgICAgIGNvbXBpbGUodm0sIGNoaWxkLCBkZXN0KVxuICAgICAgcmV0dXJuIGFwcC5sYXN0U2lnbmFsICE9PSAtMVxuICAgIH0pXG4gIH1cbn1cblxuLyoqXG4gKiBXYXRjaCB0aGUgbGlzdCB1cGRhdGUgYW5kIHJlZnJlc2ggdGhlIGNoYW5nZXMuXG4gKlxuICogQHBhcmFtIHtvYmplY3R9IHRhcmdldFxuICogQHBhcmFtIHtvYmplY3R9IGZyYWdCbG9jayB7dm1zLCBkYXRhLCBjaGlsZHJlbn1cbiAqIEBwYXJhbSB7b2JqZWN0fSBpbmZvICAgICAge2dldHRlciwga2V5LCB2YWx1ZSwgdHJhY2tCeSwgb2xkU3R5bGV9XG4gKi9cbmZ1bmN0aW9uIGJpbmRSZXBlYXQgKHZtLCB0YXJnZXQsIGZyYWdCbG9jaywgaW5mbykge1xuICBjb25zdCB2bXMgPSBmcmFnQmxvY2sudm1zXG4gIGNvbnN0IGNoaWxkcmVuID0gZnJhZ0Jsb2NrLmNoaWxkcmVuXG4gIGNvbnN0IHsgZ2V0dGVyLCB0cmFja0J5LCBvbGRTdHlsZSB9ID0gaW5mb1xuICBjb25zdCBrZXlOYW1lID0gaW5mby5rZXlcbiAgY29uc3QgdmFsdWVOYW1lID0gaW5mby52YWx1ZVxuXG4gIGZ1bmN0aW9uIGNvbXBpbGVJdGVtIChpdGVtLCBpbmRleCwgY29udGV4dCkge1xuICAgIGxldCBtZXJnZWREYXRhXG4gICAgaWYgKG9sZFN0eWxlKSB7XG4gICAgICBtZXJnZWREYXRhID0gaXRlbVxuICAgICAgaWYgKHR5cGVvZiBpdGVtID09PSAnb2JqZWN0Jykge1xuICAgICAgICBtZXJnZWREYXRhW2tleU5hbWVdID0gaW5kZXhcbiAgICAgICAgaWYgKCFtZXJnZWREYXRhLmhhc093blByb3BlcnR5KCdJTkRFWCcpKSB7XG4gICAgICAgICAgT2JqZWN0LmRlZmluZVByb3BlcnR5KG1lcmdlZERhdGEsICdJTkRFWCcsIHtcbiAgICAgICAgICAgIHZhbHVlOiAoKSA9PiB7XG4gICAgICAgICAgICAgIGNvbnNvbGUud2FybignW0pTIEZyYW1ld29ya10gXCJJTkRFWFwiIGluIHJlcGVhdCBpcyBkZXByZWNhdGVkLCAnICtcbiAgICAgICAgICAgICAgICAncGxlYXNlIHVzZSBcIiRpbmRleFwiIGluc3RlYWQnKVxuICAgICAgICAgICAgfVxuICAgICAgICAgIH0pXG4gICAgICAgIH1cbiAgICAgIH1cbiAgICB9XG4gICAgZWxzZSB7XG4gICAgICBtZXJnZWREYXRhID0ge31cbiAgICAgIG1lcmdlZERhdGFba2V5TmFtZV0gPSBpbmRleFxuICAgICAgbWVyZ2VkRGF0YVt2YWx1ZU5hbWVdID0gaXRlbVxuICAgIH1cbiAgICBjb25zdCBuZXdDb250ZXh0ID0gbWVyZ2VDb250ZXh0KGNvbnRleHQsIG1lcmdlZERhdGEpXG4gICAgdm1zLnB1c2gobmV3Q29udGV4dClcbiAgICBjb21waWxlKG5ld0NvbnRleHQsIHRhcmdldCwgZnJhZ0Jsb2NrLCB7IHJlcGVhdDogaXRlbSB9KVxuICB9XG5cbiAgY29uc3QgbGlzdCA9IHdhdGNoQmxvY2sodm0sIGZyYWdCbG9jaywgZ2V0dGVyLCAncmVwZWF0JyxcbiAgICAoZGF0YSkgPT4ge1xuICAgICAgY29uc29sZS5kZWJ1ZygnW0pTIEZyYW1ld29ya10gdGhlIFwicmVwZWF0XCIgaXRlbSBoYXMgY2hhbmdlZCcsIGRhdGEpXG4gICAgICBpZiAoIWZyYWdCbG9jaykge1xuICAgICAgICByZXR1cm5cbiAgICAgIH1cblxuICAgICAgY29uc3Qgb2xkQ2hpbGRyZW4gPSBjaGlsZHJlbi5zbGljZSgpXG4gICAgICBjb25zdCBvbGRWbXMgPSB2bXMuc2xpY2UoKVxuICAgICAgY29uc3Qgb2xkRGF0YSA9IGZyYWdCbG9jay5kYXRhLnNsaWNlKClcbiAgICAgIC8vIDEuIGNvbGxlY3QgYWxsIG5ldyByZWZzIHRyYWNrIGJ5XG4gICAgICBjb25zdCB0cmFja01hcCA9IHt9XG4gICAgICBjb25zdCByZXVzZWRNYXAgPSB7fVxuICAgICAgZGF0YS5mb3JFYWNoKChpdGVtLCBpbmRleCkgPT4ge1xuICAgICAgICBjb25zdCBrZXkgPSB0cmFja0J5ID8gaXRlbVt0cmFja0J5XSA6IChvbGRTdHlsZSA/IGl0ZW1ba2V5TmFtZV0gOiBpbmRleClcbiAgICAgICAgLyogaXN0YW5idWwgaWdub3JlIGlmICovXG4gICAgICAgIGlmIChrZXkgPT0gbnVsbCB8fCBrZXkgPT09ICcnKSB7XG4gICAgICAgICAgcmV0dXJuXG4gICAgICAgIH1cbiAgICAgICAgdHJhY2tNYXBba2V5XSA9IGl0ZW1cbiAgICAgIH0pXG5cbiAgICAgIC8vIDIuIHJlbW92ZSB1bnVzZWQgZWxlbWVudCBmb3JlYWNoIG9sZCBpdGVtXG4gICAgICBjb25zdCByZXVzZWRMaXN0ID0gW11cbiAgICAgIG9sZERhdGEuZm9yRWFjaCgoaXRlbSwgaW5kZXgpID0+IHtcbiAgICAgICAgY29uc3Qga2V5ID0gdHJhY2tCeSA/IGl0ZW1bdHJhY2tCeV0gOiAob2xkU3R5bGUgPyBpdGVtW2tleU5hbWVdIDogaW5kZXgpXG4gICAgICAgIGlmICh0cmFja01hcC5oYXNPd25Qcm9wZXJ0eShrZXkpKSB7XG4gICAgICAgICAgcmV1c2VkTWFwW2tleV0gPSB7XG4gICAgICAgICAgICBpdGVtLCBpbmRleCwga2V5LFxuICAgICAgICAgICAgdGFyZ2V0OiBvbGRDaGlsZHJlbltpbmRleF0sXG4gICAgICAgICAgICB2bTogb2xkVm1zW2luZGV4XVxuICAgICAgICAgIH1cbiAgICAgICAgICByZXVzZWRMaXN0LnB1c2goaXRlbSlcbiAgICAgICAgfVxuICAgICAgICBlbHNlIHtcbiAgICAgICAgICByZW1vdmVUYXJnZXQodm0sIG9sZENoaWxkcmVuW2luZGV4XSlcbiAgICAgICAgfVxuICAgICAgfSlcblxuICAgICAgLy8gMy4gY3JlYXRlIG5ldyBlbGVtZW50IGZvcmVhY2ggbmV3IGl0ZW1cbiAgICAgIGNoaWxkcmVuLmxlbmd0aCA9IDBcbiAgICAgIHZtcy5sZW5ndGggPSAwXG4gICAgICBmcmFnQmxvY2suZGF0YSA9IGRhdGEuc2xpY2UoKVxuICAgICAgZnJhZ0Jsb2NrLnVwZGF0ZU1hcmsgPSBmcmFnQmxvY2suc3RhcnRcblxuICAgICAgZGF0YS5mb3JFYWNoKChpdGVtLCBpbmRleCkgPT4ge1xuICAgICAgICBjb25zdCBrZXkgPSB0cmFja0J5ID8gaXRlbVt0cmFja0J5XSA6IChvbGRTdHlsZSA/IGl0ZW1ba2V5TmFtZV0gOiBpbmRleClcbiAgICAgICAgY29uc3QgcmV1c2VkID0gcmV1c2VkTWFwW2tleV1cbiAgICAgICAgaWYgKHJldXNlZCkge1xuICAgICAgICAgIGlmIChyZXVzZWQuaXRlbSA9PT0gcmV1c2VkTGlzdFswXSkge1xuICAgICAgICAgICAgcmV1c2VkTGlzdC5zaGlmdCgpXG4gICAgICAgICAgfVxuICAgICAgICAgIGVsc2Uge1xuICAgICAgICAgICAgcmV1c2VkTGlzdC4kcmVtb3ZlKHJldXNlZC5pdGVtKVxuICAgICAgICAgICAgbW92ZVRhcmdldCh2bSwgcmV1c2VkLnRhcmdldCwgZnJhZ0Jsb2NrLnVwZGF0ZU1hcmssIHRydWUpXG4gICAgICAgICAgfVxuICAgICAgICAgIGNoaWxkcmVuLnB1c2gocmV1c2VkLnRhcmdldClcbiAgICAgICAgICB2bXMucHVzaChyZXVzZWQudm0pXG4gICAgICAgICAgaWYgKG9sZFN0eWxlKSB7XG4gICAgICAgICAgICByZXVzZWQudm0gPSBpdGVtXG4gICAgICAgICAgfVxuICAgICAgICAgIGVsc2Uge1xuICAgICAgICAgICAgcmV1c2VkLnZtW3ZhbHVlTmFtZV0gPSBpdGVtXG4gICAgICAgICAgfVxuICAgICAgICAgIHJldXNlZC52bVtrZXlOYW1lXSA9IGluZGV4XG4gICAgICAgICAgZnJhZ0Jsb2NrLnVwZGF0ZU1hcmsgPSByZXVzZWQudGFyZ2V0XG4gICAgICAgIH1cbiAgICAgICAgZWxzZSB7XG4gICAgICAgICAgY29tcGlsZUl0ZW0oaXRlbSwgaW5kZXgsIHZtKVxuICAgICAgICB9XG4gICAgICB9KVxuXG4gICAgICBkZWxldGUgZnJhZ0Jsb2NrLnVwZGF0ZU1hcmtcbiAgICB9XG4gIClcblxuICBmcmFnQmxvY2suZGF0YSA9IGxpc3Quc2xpY2UoMClcbiAgbGlzdC5mb3JFYWNoKChpdGVtLCBpbmRleCkgPT4ge1xuICAgIGNvbXBpbGVJdGVtKGl0ZW0sIGluZGV4LCB2bSlcbiAgfSlcbn1cblxuLyoqXG4gKiBXYXRjaCB0aGUgZGlzcGxheSB1cGRhdGUgYW5kIGFkZC9yZW1vdmUgdGhlIGVsZW1lbnQuXG4gKlxuICogQHBhcmFtICB7b2JqZWN0fSB0YXJnZXRcbiAqIEBwYXJhbSAge29iamVjdH0gZnJhZ0Jsb2NrXG4gKiBAcGFyYW0gIHtvYmplY3R9IGNvbnRleHRcbiAqL1xuZnVuY3Rpb24gYmluZFNob3duICh2bSwgdGFyZ2V0LCBmcmFnQmxvY2ssIG1ldGEpIHtcbiAgY29uc3QgZGlzcGxheSA9IHdhdGNoQmxvY2sodm0sIGZyYWdCbG9jaywgdGFyZ2V0LnNob3duLCAnc2hvd24nLFxuICAgIChkaXNwbGF5KSA9PiB7XG4gICAgICBjb25zb2xlLmRlYnVnKCdbSlMgRnJhbWV3b3JrXSB0aGUgXCJpZlwiIGl0ZW0gd2FzIGNoYW5nZWQnLCBkaXNwbGF5KVxuXG4gICAgICBpZiAoIWZyYWdCbG9jayB8fCAhIWZyYWdCbG9jay5kaXNwbGF5ID09PSAhIWRpc3BsYXkpIHtcbiAgICAgICAgcmV0dXJuXG4gICAgICB9XG4gICAgICBmcmFnQmxvY2suZGlzcGxheSA9ICEhZGlzcGxheVxuICAgICAgaWYgKGRpc3BsYXkpIHtcbiAgICAgICAgY29tcGlsZSh2bSwgdGFyZ2V0LCBmcmFnQmxvY2ssIG1ldGEpXG4gICAgICB9XG4gICAgICBlbHNlIHtcbiAgICAgICAgcmVtb3ZlVGFyZ2V0KHZtLCBmcmFnQmxvY2ssIHRydWUpXG4gICAgICB9XG4gICAgfVxuICApXG5cbiAgZnJhZ0Jsb2NrLmRpc3BsYXkgPSAhIWRpc3BsYXlcbiAgaWYgKGRpc3BsYXkpIHtcbiAgICBjb21waWxlKHZtLCB0YXJnZXQsIGZyYWdCbG9jaywgbWV0YSlcbiAgfVxufVxuXG4vKipcbiAqIFdhdGNoIGNhbGMgdmFsdWUgY2hhbmdlcyBhbmQgYXBwZW5kIGNlcnRhaW4gdHlwZSBhY3Rpb24gdG8gZGlmZmVyLlxuICogSXQgaXMgdXNlZCBmb3IgaWYgb3IgcmVwZWF0IGRhdGEtYmluZGluZyBnZW5lcmF0b3IuXG4gKlxuICogQHBhcmFtICB7b2JqZWN0fSAgIGZyYWdCbG9ja1xuICogQHBhcmFtICB7ZnVuY3Rpb259IGNhbGNcbiAqIEBwYXJhbSAge3N0cmluZ30gICB0eXBlXG4gKiBAcGFyYW0gIHtmdW5jdGlvbn0gaGFuZGxlclxuICogQHJldHVybiB7YW55fSAgICAgIGluaXQgdmFsdWUgb2YgY2FsY1xuICovXG5mdW5jdGlvbiB3YXRjaEJsb2NrICh2bSwgZnJhZ0Jsb2NrLCBjYWxjLCB0eXBlLCBoYW5kbGVyKSB7XG4gIGNvbnN0IGRpZmZlciA9IHZtICYmIHZtLl9hcHAgJiYgdm0uX2FwcC5kaWZmZXJcbiAgY29uc3QgY29uZmlnID0ge31cbiAgY29uc3QgZGVwdGggPSAoZnJhZ0Jsb2NrLmVsZW1lbnQuZGVwdGggfHwgMCkgKyAxXG5cbiAgcmV0dXJuIHdhdGNoKHZtLCBjYWxjLCAodmFsdWUpID0+IHtcbiAgICBjb25maWcubGF0ZXN0VmFsdWUgPSB2YWx1ZVxuICAgIGlmIChkaWZmZXIgJiYgIWNvbmZpZy5yZWNvcmRlZCkge1xuICAgICAgZGlmZmVyLmFwcGVuZCh0eXBlLCBkZXB0aCwgZnJhZ0Jsb2NrLmJsb2NrSWQsICgpID0+IHtcbiAgICAgICAgY29uc3QgbGF0ZXN0VmFsdWUgPSBjb25maWcubGF0ZXN0VmFsdWVcbiAgICAgICAgaGFuZGxlcihsYXRlc3RWYWx1ZSlcbiAgICAgICAgY29uZmlnLnJlY29yZGVkID0gZmFsc2VcbiAgICAgICAgY29uZmlnLmxhdGVzdFZhbHVlID0gdW5kZWZpbmVkXG4gICAgICB9KVxuICAgIH1cbiAgICBjb25maWcucmVjb3JkZWQgPSB0cnVlXG4gIH0pXG59XG5cbi8qKlxuICogQ2xvbmUgYSBjb250ZXh0IGFuZCBtZXJnZSBjZXJ0YWluIGRhdGEuXG4gKlxuICogQHBhcmFtICB7b2JqZWN0fSBtZXJnZWREYXRhXG4gKiBAcmV0dXJuIHtvYmplY3R9XG4gKi9cbmZ1bmN0aW9uIG1lcmdlQ29udGV4dCAoY29udGV4dCwgbWVyZ2VkRGF0YSkge1xuICBjb25zdCBuZXdDb250ZXh0ID0gT2JqZWN0LmNyZWF0ZShjb250ZXh0KVxuICBuZXdDb250ZXh0Ll9kYXRhID0gbWVyZ2VkRGF0YVxuICBpbml0RGF0YShuZXdDb250ZXh0KVxuICBpbml0Q29tcHV0ZWQobmV3Q29udGV4dClcbiAgbmV3Q29udGV4dC5fcmVhbFBhcmVudCA9IGNvbnRleHRcbiAgcmV0dXJuIG5ld0NvbnRleHRcbn1cblxuXG5cbi8qKiBXRUJQQUNLIEZPT1RFUiAqKlxuICoqIC9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9zcmMvZGVmYXVsdC92bS9jb21waWxlci5qc1xuICoqLyIsIi8qKlxuICogQGZpbGVPdmVydmlld1xuICogRGlyZWN0aXZlIFBhcnNlclxuICovXG5cbmltcG9ydCB7IGJpbmQsIHR5cG9mIH0gZnJvbSAnLi4vdXRpbCdcblxuaW1wb3J0IFdhdGNoZXIgZnJvbSAnLi4vY29yZS93YXRjaGVyJ1xuaW1wb3J0IGNvbmZpZyBmcm9tICcuLi9jb25maWcnXG5cbmNvbnN0IHsgbmF0aXZlQ29tcG9uZW50TWFwIH0gPSBjb25maWdcblxuY29uc3QgU0VUVEVSUyA9IHtcbiAgYXR0cjogJ3NldEF0dHInLFxuICBzdHlsZTogJ3NldFN0eWxlJyxcbiAgZXZlbnQ6ICdhZGRFdmVudCdcbn1cblxuLyoqXG4gKiBhcHBseSB0aGUgbmF0aXZlIGNvbXBvbmVudCdzIG9wdGlvbnMoc3BlY2lmaWVkIGJ5IHRlbXBsYXRlLnR5cGUpXG4gKiB0byB0aGUgdGVtcGxhdGVcbiAqL1xuZXhwb3J0IGZ1bmN0aW9uIGFwcGx5TmFpdHZlQ29tcG9uZW50T3B0aW9ucyAodGVtcGxhdGUpIHtcbiAgY29uc3QgeyB0eXBlIH0gPSB0ZW1wbGF0ZVxuICBjb25zdCBvcHRpb25zID0gbmF0aXZlQ29tcG9uZW50TWFwW3R5cGVdXG5cbiAgaWYgKHR5cGVvZiBvcHRpb25zID09PSAnb2JqZWN0Jykge1xuICAgIGZvciAoY29uc3Qga2V5IGluIG9wdGlvbnMpIHtcbiAgICAgIGlmICh0ZW1wbGF0ZVtrZXldID09IG51bGwpIHtcbiAgICAgICAgdGVtcGxhdGVba2V5XSA9IG9wdGlvbnNba2V5XVxuICAgICAgfVxuICAgICAgZWxzZSBpZiAodHlwb2YodGVtcGxhdGVba2V5XSkgPT09ICdvYmplY3QnICYmXG4gICAgICAgIHR5cG9mKG9wdGlvbnNba2V5XSkgPT09ICdvYmplY3QnKSB7XG4gICAgICAgIGZvciAoY29uc3Qgc3Via2V5IGluIG9wdGlvbnNba2V5XSkge1xuICAgICAgICAgIGlmICh0ZW1wbGF0ZVtrZXldW3N1YmtleV0gPT0gbnVsbCkge1xuICAgICAgICAgICAgdGVtcGxhdGVba2V5XVtzdWJrZXldID0gb3B0aW9uc1trZXldW3N1YmtleV1cbiAgICAgICAgICB9XG4gICAgICAgIH1cbiAgICAgIH1cbiAgICB9XG4gIH1cbn1cblxuLyoqXG4gKiBiaW5kIGFsbCBpZCwgYXR0ciwgY2xhc3NuYW1lcywgc3R5bGUsIGV2ZW50cyB0byBhbiBlbGVtZW50XG4gKi9cbmV4cG9ydCBmdW5jdGlvbiBiaW5kRWxlbWVudCAodm0sIGVsLCB0ZW1wbGF0ZSkge1xuICBzZXRJZCh2bSwgZWwsIHRlbXBsYXRlLmlkLCB2bSlcbiAgc2V0QXR0cih2bSwgZWwsIHRlbXBsYXRlLmF0dHIpXG4gIHNldENsYXNzKHZtLCBlbCwgdGVtcGxhdGUuY2xhc3NMaXN0KVxuICBzZXRTdHlsZSh2bSwgZWwsIHRlbXBsYXRlLnN0eWxlKVxuICBiaW5kRXZlbnRzKHZtLCBlbCwgdGVtcGxhdGUuZXZlbnRzKVxufVxuXG4vKipcbiAqIGJpbmQgYWxsIHByb3BzIHRvIHN1YiB2bSBhbmQgYmluZCBhbGwgc3R5bGUsIGV2ZW50cyB0byB0aGUgcm9vdCBlbGVtZW50XG4gKiBvZiB0aGUgc3ViIHZtIGlmIGl0IGRvZXNuJ3QgaGF2ZSBhIHJlcGxhY2VkIG11bHRpLW5vZGUgZnJhZ21lbnRcbiAqL1xuZXhwb3J0IGZ1bmN0aW9uIGJpbmRTdWJWbSAodm0sIHN1YlZtLCB0ZW1wbGF0ZSwgcmVwZWF0SXRlbSkge1xuICBzdWJWbSA9IHN1YlZtIHx8IHt9XG4gIHRlbXBsYXRlID0gdGVtcGxhdGUgfHwge31cblxuICBjb25zdCBvcHRpb25zID0gc3ViVm0uX29wdGlvbnMgfHwge31cblxuICAvLyBiaW5kIHByb3BzXG4gIGxldCBwcm9wcyA9IG9wdGlvbnMucHJvcHNcblxuICBpZiAoQXJyYXkuaXNBcnJheShwcm9wcykpIHtcbiAgICBwcm9wcyA9IHByb3BzLnJlZHVjZSgocmVzdWx0LCB2YWx1ZSkgPT4ge1xuICAgICAgcmVzdWx0W3ZhbHVlXSA9IHRydWVcbiAgICAgIHJldHVybiByZXN1bHRcbiAgICB9LCB7fSlcbiAgfVxuXG4gIG1lcmdlUHJvcHMocmVwZWF0SXRlbSwgcHJvcHMsIHZtLCBzdWJWbSlcbiAgbWVyZ2VQcm9wcyh0ZW1wbGF0ZS5hdHRyLCBwcm9wcywgdm0sIHN1YlZtKVxufVxuXG4vKipcbiAqIG1lcmdlIGNsYXNzIGFuZCBzdHlsZXMgZnJvbSB2bSB0byBzdWIgdm0uXG4gKi9cbmV4cG9ydCBmdW5jdGlvbiBiaW5kU3ViVm1BZnRlckluaXRpYWxpemVkICh2bSwgc3ViVm0sIHRlbXBsYXRlKSB7XG4gIG1lcmdlQ2xhc3NTdHlsZSh0ZW1wbGF0ZS5jbGFzc0xpc3QsIHZtLCBzdWJWbSlcbiAgbWVyZ2VTdHlsZSh0ZW1wbGF0ZS5zdHlsZSwgdm0sIHN1YlZtKVxufVxuXG4vKipcbiAqIEJpbmQgcHJvcHMgZnJvbSB2bSB0byBzdWIgdm0gYW5kIHdhdGNoIHRoZWlyIHVwZGF0ZXMuXG4gKi9cbmZ1bmN0aW9uIG1lcmdlUHJvcHMgKHRhcmdldCwgcHJvcHMsIHZtLCBzdWJWbSkge1xuICBpZiAoIXRhcmdldCkge1xuICAgIHJldHVyblxuICB9XG4gIGZvciAoY29uc3Qga2V5IGluIHRhcmdldCkge1xuICAgIGlmICghcHJvcHMgfHwgcHJvcHNba2V5XSkge1xuICAgICAgY29uc3QgdmFsdWUgPSB0YXJnZXRba2V5XVxuICAgICAgaWYgKHR5cGVvZiB2YWx1ZSA9PT0gJ2Z1bmN0aW9uJykge1xuICAgICAgICBjb25zdCByZXR1cm5WYWx1ZSA9IHdhdGNoKHZtLCB2YWx1ZSwgZnVuY3Rpb24gKHYpIHtcbiAgICAgICAgICBzdWJWbVtrZXldID0gdlxuICAgICAgICB9KVxuICAgICAgICBzdWJWbVtrZXldID0gcmV0dXJuVmFsdWVcbiAgICAgIH1cbiAgICAgIGVsc2Uge1xuICAgICAgICBzdWJWbVtrZXldID0gdmFsdWVcbiAgICAgIH1cbiAgICB9XG4gIH1cbn1cblxuLyoqXG4gKiBCaW5kIHN0eWxlIGZyb20gdm0gdG8gc3ViIHZtIGFuZCB3YXRjaCB0aGVpciB1cGRhdGVzLlxuICovXG5mdW5jdGlvbiBtZXJnZVN0eWxlICh0YXJnZXQsIHZtLCBzdWJWbSkge1xuICBmb3IgKGNvbnN0IGtleSBpbiB0YXJnZXQpIHtcbiAgICBjb25zdCB2YWx1ZSA9IHRhcmdldFtrZXldXG4gICAgaWYgKHR5cGVvZiB2YWx1ZSA9PT0gJ2Z1bmN0aW9uJykge1xuICAgICAgY29uc3QgcmV0dXJuVmFsdWUgPSB3YXRjaCh2bSwgdmFsdWUsIGZ1bmN0aW9uICh2KSB7XG4gICAgICAgIGlmIChzdWJWbS5fcm9vdEVsKSB7XG4gICAgICAgICAgc3ViVm0uX3Jvb3RFbC5zZXRTdHlsZShrZXksIHYpXG4gICAgICAgIH1cbiAgICAgIH0pXG4gICAgICBzdWJWbS5fcm9vdEVsLnNldFN0eWxlKGtleSwgcmV0dXJuVmFsdWUpXG4gICAgfVxuICAgIGVsc2Uge1xuICAgICAgaWYgKHN1YlZtLl9yb290RWwpIHtcbiAgICAgICAgc3ViVm0uX3Jvb3RFbC5zZXRTdHlsZShrZXksIHZhbHVlKVxuICAgICAgfVxuICAgIH1cbiAgfVxufVxuXG4vKipcbiAqIEJpbmQgY2xhc3MgJiBzdHlsZSBmcm9tIHZtIHRvIHN1YiB2bSBhbmQgd2F0Y2ggdGhlaXIgdXBkYXRlcy5cbiAqL1xuZnVuY3Rpb24gbWVyZ2VDbGFzc1N0eWxlICh0YXJnZXQsIHZtLCBzdWJWbSkge1xuICBjb25zdCBjc3MgPSB2bS5fb3B0aW9ucyAmJiB2bS5fb3B0aW9ucy5zdHlsZSB8fCB7fVxuXG4gIC8qIGlzdGFuYnVsIGlnbm9yZSBpZiAqL1xuICBpZiAoIXN1YlZtLl9yb290RWwpIHtcbiAgICByZXR1cm5cbiAgfVxuXG4gIGlmICh0eXBlb2YgdGFyZ2V0ID09PSAnZnVuY3Rpb24nKSB7XG4gICAgY29uc3QgdmFsdWUgPSB3YXRjaCh2bSwgdGFyZ2V0LCB2ID0+IHtcbiAgICAgIHNldENsYXNzU3R5bGUoc3ViVm0uX3Jvb3RFbCwgY3NzLCB2KVxuICAgIH0pXG4gICAgc2V0Q2xhc3NTdHlsZShzdWJWbS5fcm9vdEVsLCBjc3MsIHZhbHVlKVxuICB9XG4gIGVsc2UgaWYgKHRhcmdldCAhPSBudWxsKSB7XG4gICAgc2V0Q2xhc3NTdHlsZShzdWJWbS5fcm9vdEVsLCBjc3MsIHRhcmdldClcbiAgfVxufVxuXG4vKipcbiAqIGJpbmQgaWQgdG8gYW4gZWxlbWVudFxuICogZWFjaCBpZCBpcyB1bmlxdWUgaW4gYSB3aG9sZSB2bVxuICovXG5leHBvcnQgZnVuY3Rpb24gc2V0SWQgKHZtLCBlbCwgaWQsIHRhcmdldCkge1xuICBjb25zdCBtYXAgPSBPYmplY3QuY3JlYXRlKG51bGwpXG5cbiAgT2JqZWN0LmRlZmluZVByb3BlcnRpZXMobWFwLCB7XG4gICAgdm06IHtcbiAgICAgIHZhbHVlOiB0YXJnZXQsXG4gICAgICB3cml0YWJsZTogZmFsc2UsXG4gICAgICBjb25maWd1cmFibGU6IGZhbHNlXG4gICAgfSxcbiAgICBlbDoge1xuICAgICAgZ2V0OiAoKSA9PiBlbCB8fCB0YXJnZXQuX3Jvb3RFbCxcbiAgICAgIGNvbmZpZ3VyYWJsZTogZmFsc2VcbiAgICB9XG4gIH0pXG5cbiAgaWYgKHR5cGVvZiBpZCA9PT0gJ2Z1bmN0aW9uJykge1xuICAgIGNvbnN0IGhhbmRsZXIgPSBpZFxuICAgIGlkID0gaGFuZGxlci5jYWxsKHZtKVxuICAgIGlmIChpZCkge1xuICAgICAgdm0uX2lkc1tpZF0gPSBtYXBcbiAgICB9XG4gICAgd2F0Y2godm0sIGhhbmRsZXIsIChuZXdJZCkgPT4ge1xuICAgICAgaWYgKG5ld0lkKSB7XG4gICAgICAgIHZtLl9pZHNbbmV3SWRdID0gbWFwXG4gICAgICB9XG4gICAgfSlcbiAgfVxuICBlbHNlIGlmIChpZCAmJiB0eXBlb2YgaWQgPT09ICdzdHJpbmcnKSB7XG4gICAgdm0uX2lkc1tpZF0gPSBtYXBcbiAgfVxufVxuXG4vKipcbiAqIGJpbmQgYXR0ciB0byBhbiBlbGVtZW50XG4gKi9cbmZ1bmN0aW9uIHNldEF0dHIgKHZtLCBlbCwgYXR0cikge1xuICBiaW5kRGlyKHZtLCBlbCwgJ2F0dHInLCBhdHRyKVxufVxuXG5mdW5jdGlvbiBzZXRDbGFzc1N0eWxlIChlbCwgY3NzLCBjbGFzc0xpc3QpIHtcbiAgY29uc3QgY2xhc3NTdHlsZSA9IHt9XG4gIGNvbnN0IGxlbmd0aCA9IGNsYXNzTGlzdC5sZW5ndGhcblxuICBmb3IgKGxldCBpID0gMDsgaSA8IGxlbmd0aDsgaSsrKSB7XG4gICAgY29uc3Qgc3R5bGUgPSBjc3NbY2xhc3NMaXN0W2ldXVxuICAgIGlmIChzdHlsZSkge1xuICAgICAgZm9yIChjb25zdCBrZXkgaW4gc3R5bGUpIHtcbiAgICAgICAgY2xhc3NTdHlsZVtrZXldID0gc3R5bGVba2V5XVxuICAgICAgfVxuICAgIH1cbiAgfVxuICBlbC5zZXRDbGFzc1N0eWxlKGNsYXNzU3R5bGUpXG59XG5cbi8qKlxuICogYmluZCBjbGFzc25hbWVzIHRvIGFuIGVsZW1lbnRcbiAqL1xuZnVuY3Rpb24gc2V0Q2xhc3MgKHZtLCBlbCwgY2xhc3NMaXN0KSB7XG4gIGlmICh0eXBlb2YgY2xhc3NMaXN0ICE9PSAnZnVuY3Rpb24nICYmICFBcnJheS5pc0FycmF5KGNsYXNzTGlzdCkpIHtcbiAgICByZXR1cm5cbiAgfVxuICBpZiAoQXJyYXkuaXNBcnJheShjbGFzc0xpc3QpICYmICFjbGFzc0xpc3QubGVuZ3RoKSB7XG4gICAgZWwuc2V0Q2xhc3NTdHlsZSh7fSlcbiAgICByZXR1cm5cbiAgfVxuXG4gIGNvbnN0IHN0eWxlID0gdm0uX29wdGlvbnMgJiYgdm0uX29wdGlvbnMuc3R5bGUgfHwge31cbiAgaWYgKHR5cGVvZiBjbGFzc0xpc3QgPT09ICdmdW5jdGlvbicpIHtcbiAgICBjb25zdCB2YWx1ZSA9IHdhdGNoKHZtLCBjbGFzc0xpc3QsIHYgPT4ge1xuICAgICAgc2V0Q2xhc3NTdHlsZShlbCwgc3R5bGUsIHYpXG4gICAgfSlcbiAgICBzZXRDbGFzc1N0eWxlKGVsLCBzdHlsZSwgdmFsdWUpXG4gIH1cbiAgZWxzZSB7XG4gICAgc2V0Q2xhc3NTdHlsZShlbCwgc3R5bGUsIGNsYXNzTGlzdClcbiAgfVxufVxuXG4vKipcbiAqIGJpbmQgc3R5bGUgdG8gYW4gZWxlbWVudFxuICovXG5mdW5jdGlvbiBzZXRTdHlsZSAodm0sIGVsLCBzdHlsZSkge1xuICBiaW5kRGlyKHZtLCBlbCwgJ3N0eWxlJywgc3R5bGUpXG59XG5cbi8qKlxuICogYWRkIGFuIGV2ZW50IHR5cGUgYW5kIGhhbmRsZXIgdG8gYW4gZWxlbWVudCBhbmQgZ2VuZXJhdGUgYSBkb20gdXBkYXRlXG4gKi9cbmZ1bmN0aW9uIHNldEV2ZW50ICh2bSwgZWwsIHR5cGUsIGhhbmRsZXIpIHtcbiAgZWwuYWRkRXZlbnQodHlwZSwgYmluZChoYW5kbGVyLCB2bSkpXG59XG5cbi8qKlxuICogYWRkIGFsbCBldmVudHMgb2YgYW4gZWxlbWVudFxuICovXG5mdW5jdGlvbiBiaW5kRXZlbnRzICh2bSwgZWwsIGV2ZW50cykge1xuICBpZiAoIWV2ZW50cykge1xuICAgIHJldHVyblxuICB9XG4gIGNvbnN0IGtleXMgPSBPYmplY3Qua2V5cyhldmVudHMpXG4gIGxldCBpID0ga2V5cy5sZW5ndGhcbiAgd2hpbGUgKGktLSkge1xuICAgIGNvbnN0IGtleSA9IGtleXNbaV1cbiAgICBsZXQgaGFuZGxlciA9IGV2ZW50c1trZXldXG4gICAgaWYgKHR5cGVvZiBoYW5kbGVyID09PSAnc3RyaW5nJykge1xuICAgICAgaGFuZGxlciA9IHZtW2hhbmRsZXJdXG4gICAgICAvKiBpc3RhbmJ1bCBpZ25vcmUgaWYgKi9cbiAgICAgIGlmICghaGFuZGxlcikge1xuICAgICAgICBjb25zb2xlLmRlYnVnKGBbSlMgRnJhbWV3b3JrXSBUaGUgbWV0aG9kIFwiJHtoYW5kbGVyfVwiIGlzIG5vdCBkZWZpbmVkLmApXG4gICAgICB9XG4gICAgfVxuICAgIHNldEV2ZW50KHZtLCBlbCwga2V5LCBoYW5kbGVyKVxuICB9XG59XG5cbi8qKlxuICogc2V0IGEgc2VyaWVzIG9mIG1lbWJlcnMgYXMgYSBraW5kIG9mIGFuIGVsZW1lbnRcbiAqIGZvciBleGFtcGxlOiBzdHlsZSwgYXR0ciwgLi4uXG4gKiBpZiB0aGUgdmFsdWUgaXMgYSBmdW5jdGlvbiB0aGVuIGJpbmQgdGhlIGRhdGEgY2hhbmdlc1xuICovXG5mdW5jdGlvbiBiaW5kRGlyICh2bSwgZWwsIG5hbWUsIGRhdGEpIHtcbiAgaWYgKCFkYXRhKSB7XG4gICAgcmV0dXJuXG4gIH1cbiAgY29uc3Qga2V5cyA9IE9iamVjdC5rZXlzKGRhdGEpXG4gIGxldCBpID0ga2V5cy5sZW5ndGhcbiAgd2hpbGUgKGktLSkge1xuICAgIGNvbnN0IGtleSA9IGtleXNbaV1cbiAgICBjb25zdCB2YWx1ZSA9IGRhdGFba2V5XVxuICAgIGlmICh0eXBlb2YgdmFsdWUgPT09ICdmdW5jdGlvbicpIHtcbiAgICAgIGJpbmRLZXkodm0sIGVsLCBuYW1lLCBrZXksIHZhbHVlKVxuICAgIH1cbiAgICBlbHNlIHtcbiAgICAgIGVsW1NFVFRFUlNbbmFtZV1dKGtleSwgdmFsdWUpXG4gICAgfVxuICB9XG59XG5cbi8qKlxuICogYmluZCBkYXRhIGNoYW5nZXMgdG8gYSBjZXJ0YWluIGtleSB0byBhIG5hbWUgc2VyaWVzIGluIGFuIGVsZW1lbnRcbiAqL1xuZnVuY3Rpb24gYmluZEtleSAodm0sIGVsLCBuYW1lLCBrZXksIGNhbGMpIHtcbiAgY29uc3QgbWV0aG9kTmFtZSA9IFNFVFRFUlNbbmFtZV1cbiAgLy8gd2F0Y2ggdGhlIGNhbGMsIGFuZCByZXR1cm5zIGEgdmFsdWUgYnkgY2FsYy5jYWxsKClcbiAgY29uc3QgdmFsdWUgPSB3YXRjaCh2bSwgY2FsYywgKHZhbHVlKSA9PiB7XG4gICAgZnVuY3Rpb24gaGFuZGxlciAoKSB7XG4gICAgICBlbFttZXRob2ROYW1lXShrZXksIHZhbHVlKVxuICAgIH1cbiAgICBjb25zdCBkaWZmZXIgPSB2bSAmJiB2bS5fYXBwICYmIHZtLl9hcHAuZGlmZmVyXG4gICAgaWYgKGRpZmZlcikge1xuICAgICAgZGlmZmVyLmFwcGVuZCgnZWxlbWVudCcsIGVsLmRlcHRoLCBlbC5yZWYsIGhhbmRsZXIpXG4gICAgfVxuICAgIGVsc2Uge1xuICAgICAgaGFuZGxlcigpXG4gICAgfVxuICB9KVxuXG4gIGVsW21ldGhvZE5hbWVdKGtleSwgdmFsdWUpXG59XG5cbi8qKlxuICogd2F0Y2ggYSBjYWxjIGZ1bmN0aW9uIGFuZCBjYWxsYmFjayBpZiB0aGUgY2FsYyB2YWx1ZSBjaGFuZ2VzXG4gKi9cbmV4cG9ydCBmdW5jdGlvbiB3YXRjaCAodm0sIGNhbGMsIGNhbGxiYWNrKSB7XG4gIGNvbnN0IHdhdGNoZXIgPSBuZXcgV2F0Y2hlcih2bSwgY2FsYywgZnVuY3Rpb24gKHZhbHVlLCBvbGRWYWx1ZSkge1xuICAgIC8qIGlzdGFuYnVsIGlnbm9yZSBpZiAqL1xuICAgIGlmICh0eXBlb2YgdmFsdWUgIT09ICdvYmplY3QnICYmIHZhbHVlID09PSBvbGRWYWx1ZSkge1xuICAgICAgcmV0dXJuXG4gICAgfVxuICAgIGNhbGxiYWNrKHZhbHVlKVxuICB9KVxuXG4gIHJldHVybiB3YXRjaGVyLnZhbHVlXG59XG5cblxuXG4vKiogV0VCUEFDSyBGT09URVIgKipcbiAqKiAvVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvc3JjL2RlZmF1bHQvdm0vZGlyZWN0aXZlLmpzXG4gKiovIiwiLyoqXG4gKiBAZmlsZU92ZXJ2aWV3IERvY3VtZW50ICYgRWxlbWVudCBIZWxwZXJzLlxuICpcbiAqIHJlcXVpcmVkOlxuICogRG9jdW1lbnQjOiBjcmVhdGVFbGVtZW50LCBjcmVhdGVDb21tZW50LCBnZXRSZWZcbiAqIEVsZW1lbnQjOiBhcHBlbmRDaGlsZCwgaW5zZXJ0QmVmb3JlLCByZW1vdmVDaGlsZCwgbmV4dFNpYmxpbmdcbiAqL1xuXG4vKipcbiAqIENyZWF0ZSBhIGJvZHkgYnkgdHlwZVxuICogVXNpbmcgdGhpcy5fYXBwLmRvY1xuICpcbiAqIEBwYXJhbSAge3N0cmluZ30gdHlwZVxuICovXG5leHBvcnQgZnVuY3Rpb24gY3JlYXRlQm9keSAodm0sIHR5cGUpIHtcbiAgY29uc3QgZG9jID0gdm0uX2FwcC5kb2NcbiAgcmV0dXJuIGRvYy5jcmVhdGVCb2R5KHR5cGUpXG59XG5cbi8qKlxuICogQ3JlYXRlIGFuIGVsZW1lbnQgYnkgdHlwZVxuICogVXNpbmcgdGhpcy5fYXBwLmRvY1xuICpcbiAqIEBwYXJhbSAge3N0cmluZ30gdHlwZVxuICovXG5leHBvcnQgZnVuY3Rpb24gY3JlYXRlRWxlbWVudCAodm0sIHR5cGUpIHtcbiAgY29uc3QgZG9jID0gdm0uX2FwcC5kb2NcbiAgcmV0dXJuIGRvYy5jcmVhdGVFbGVtZW50KHR5cGUpXG59XG5cbi8qKlxuICogQ3JlYXRlIGFuZCByZXR1cm4gYSBmcmFnIGJsb2NrIGZvciBhbiBlbGVtZW50LlxuICogVGhlIGZyYWcgYmxvY2sgaGFzIGEgc3RhcnRlciwgZW5kZXIgYW5kIHRoZSBlbGVtZW50IGl0c2VsZi5cbiAqXG4gKiBAcGFyYW0gIHtvYmplY3R9IGVsZW1lbnRcbiAqL1xuZXhwb3J0IGZ1bmN0aW9uIGNyZWF0ZUJsb2NrICh2bSwgZWxlbWVudCkge1xuICBjb25zdCBzdGFydCA9IGNyZWF0ZUJsb2NrU3RhcnQodm0pXG4gIGNvbnN0IGVuZCA9IGNyZWF0ZUJsb2NrRW5kKHZtKVxuICBjb25zdCBibG9ja0lkID0gbGFzdGVzdEJsb2NrSWQrK1xuICBpZiAoZWxlbWVudC5lbGVtZW50KSB7XG4gICAgbGV0IHVwZGF0ZU1hcmsgPSBlbGVtZW50LnVwZGF0ZU1hcmtcbiAgICBpZiAodXBkYXRlTWFyaykge1xuICAgICAgaWYgKHVwZGF0ZU1hcmsuZWxlbWVudCkge1xuICAgICAgICB1cGRhdGVNYXJrID0gdXBkYXRlTWFyay5lbmRcbiAgICAgIH1cbiAgICAgIGVsZW1lbnQuZWxlbWVudC5pbnNlcnRBZnRlcihlbmQsIHVwZGF0ZU1hcmspXG4gICAgICBlbGVtZW50LmVsZW1lbnQuaW5zZXJ0QWZ0ZXIoc3RhcnQsIHVwZGF0ZU1hcmspXG4gICAgICBlbGVtZW50LnVwZGF0ZU1hcmsgPSBlbmRcbiAgICB9XG4gICAgZWxzZSB7XG4gICAgICBlbGVtZW50LmVsZW1lbnQuaW5zZXJ0QmVmb3JlKHN0YXJ0LCBlbGVtZW50LmVuZClcbiAgICAgIGVsZW1lbnQuZWxlbWVudC5pbnNlcnRCZWZvcmUoZW5kLCBlbGVtZW50LmVuZClcbiAgICB9XG4gICAgZWxlbWVudCA9IGVsZW1lbnQuZWxlbWVudFxuICB9XG4gIGVsc2Uge1xuICAgIGVsZW1lbnQuYXBwZW5kQ2hpbGQoc3RhcnQpXG4gICAgZWxlbWVudC5hcHBlbmRDaGlsZChlbmQpXG4gIH1cbiAgcmV0dXJuIHsgc3RhcnQsIGVuZCwgZWxlbWVudCwgYmxvY2tJZCB9XG59XG5cbmxldCBsYXN0ZXN0QmxvY2tJZCA9IDFcblxuLyoqXG4gKiBDcmVhdGUgYW5kIHJldHVybiBhIGJsb2NrIHN0YXJ0ZXIuXG4gKiBVc2luZyB0aGlzLl9hcHAuZG9jXG4gKi9cbmZ1bmN0aW9uIGNyZWF0ZUJsb2NrU3RhcnQgKHZtKSB7XG4gIGNvbnN0IGRvYyA9IHZtLl9hcHAuZG9jXG4gIGNvbnN0IGFuY2hvciA9IGRvYy5jcmVhdGVDb21tZW50KCdzdGFydCcpXG4gIHJldHVybiBhbmNob3Jcbn1cblxuLyoqXG4gKiBDcmVhdGUgYW5kIHJldHVybiBhIGJsb2NrIGVuZGVyLlxuICogVXNpbmcgdGhpcy5fYXBwLmRvY1xuICovXG5mdW5jdGlvbiBjcmVhdGVCbG9ja0VuZCAodm0pIHtcbiAgY29uc3QgZG9jID0gdm0uX2FwcC5kb2NcbiAgY29uc3QgYW5jaG9yID0gZG9jLmNyZWF0ZUNvbW1lbnQoJ2VuZCcpXG4gIHJldHVybiBhbmNob3Jcbn1cblxuLyoqXG4gKiBBdHRhY2ggdGFyZ2V0IHRvIGEgY2VydGFpbiBkZXN0IHVzaW5nIGFwcGVuZENoaWxkIGJ5IGRlZmF1bHQuXG4gKiBJZiB0aGUgZGVzdCBpcyBhIGZyYWcgYmxvY2sgdGhlbiBpbnNlcnQgYmVmb3JlIHRoZSBlbmRlci5cbiAqIElmIHRoZSB0YXJnZXQgaXMgYSBmcmFnIGJsb2NrIHRoZW4gYXR0YWNoIHRoZSBzdGFydGVyIGFuZCBlbmRlciBpbiBvcmRlci5cbiAqXG4gKiBAcGFyYW0gIHtvYmplY3R9IHRhcmdldFxuICogQHBhcmFtICB7b2JqZWN0fSBkZXN0XG4gKi9cbmV4cG9ydCBmdW5jdGlvbiBhdHRhY2hUYXJnZXQgKHZtLCB0YXJnZXQsIGRlc3QpIHtcbiAgaWYgKGRlc3QuZWxlbWVudCkge1xuICAgIGNvbnN0IGJlZm9yZSA9IGRlc3QuZW5kXG4gICAgY29uc3QgYWZ0ZXIgPSBkZXN0LnVwZGF0ZU1hcmtcbiAgICAvLyBwdXNoIG5ldyB0YXJnZXQgZm9yIHdhdGNoIGxpc3QgdXBkYXRlIGxhdGVyXG4gICAgaWYgKGRlc3QuY2hpbGRyZW4pIHtcbiAgICAgIGRlc3QuY2hpbGRyZW4ucHVzaCh0YXJnZXQpXG4gICAgfVxuICAgIC8vIGZvciBjaGVjayByZXBlYXQgY2FzZVxuICAgIGlmIChhZnRlcikge1xuICAgICAgY29uc3Qgc2lnbmFsID0gbW92ZVRhcmdldCh2bSwgdGFyZ2V0LCBhZnRlcilcbiAgICAgIGRlc3QudXBkYXRlTWFyayA9IHRhcmdldC5lbGVtZW50ID8gdGFyZ2V0LmVuZCA6IHRhcmdldFxuICAgICAgcmV0dXJuIHNpZ25hbFxuICAgIH1cbiAgICBlbHNlIGlmICh0YXJnZXQuZWxlbWVudCkge1xuICAgICAgZGVzdC5lbGVtZW50Lmluc2VydEJlZm9yZSh0YXJnZXQuc3RhcnQsIGJlZm9yZSlcbiAgICAgIGRlc3QuZWxlbWVudC5pbnNlcnRCZWZvcmUodGFyZ2V0LmVuZCwgYmVmb3JlKVxuICAgIH1cbiAgICBlbHNlIHtcbiAgICAgIHJldHVybiBkZXN0LmVsZW1lbnQuaW5zZXJ0QmVmb3JlKHRhcmdldCwgYmVmb3JlKVxuICAgIH1cbiAgfVxuICBlbHNlIHtcbiAgICBpZiAodGFyZ2V0LmVsZW1lbnQpIHtcbiAgICAgIGRlc3QuYXBwZW5kQ2hpbGQodGFyZ2V0LnN0YXJ0KVxuICAgICAgZGVzdC5hcHBlbmRDaGlsZCh0YXJnZXQuZW5kKVxuICAgIH1cbiAgICBlbHNlIHtcbiAgICAgIHJldHVybiBkZXN0LmFwcGVuZENoaWxkKHRhcmdldClcbiAgICB9XG4gIH1cbn1cblxuLyoqXG4gKiBNb3ZlIHRhcmdldCBiZWZvcmUgYSBjZXJ0YWluIGVsZW1lbnQuIFRoZSB0YXJnZXQgbWF5YmUgYmxvY2sgb3IgZWxlbWVudC5cbiAqXG4gKiBAcGFyYW0gIHtvYmplY3R9IHRhcmdldFxuICogQHBhcmFtICB7b2JqZWN0fSBiZWZvcmVcbiAqL1xuZXhwb3J0IGZ1bmN0aW9uIG1vdmVUYXJnZXQgKHZtLCB0YXJnZXQsIGFmdGVyKSB7XG4gIGlmICh0YXJnZXQuZWxlbWVudCkge1xuICAgIHJldHVybiBtb3ZlQmxvY2sodGFyZ2V0LCBhZnRlcilcbiAgfVxuICByZXR1cm4gbW92ZUVsZW1lbnQodGFyZ2V0LCBhZnRlcilcbn1cblxuLyoqXG4gKiBNb3ZlIGVsZW1lbnQgYmVmb3JlIGEgY2VydGFpbiBlbGVtZW50LlxuICpcbiAqIEBwYXJhbSAge29iamVjdH0gZWxlbWVudFxuICogQHBhcmFtICB7b2JqZWN0fSBiZWZvcmVcbiAqL1xuZnVuY3Rpb24gbW92ZUVsZW1lbnQgKGVsZW1lbnQsIGFmdGVyKSB7XG4gIGNvbnN0IHBhcmVudCA9IGFmdGVyLnBhcmVudE5vZGVcbiAgaWYgKHBhcmVudCkge1xuICAgIHJldHVybiBwYXJlbnQuaW5zZXJ0QWZ0ZXIoZWxlbWVudCwgYWZ0ZXIpXG4gIH1cbn1cblxuLyoqXG4gKiBNb3ZlIGFsbCBlbGVtZW50cyBvZiB0aGUgYmxvY2sgYmVmb3JlIGEgY2VydGFpbiBlbGVtZW50LlxuICpcbiAqIEBwYXJhbSAge29iamVjdH0gZnJhZ0Jsb2NrXG4gKiBAcGFyYW0gIHtvYmplY3R9IGJlZm9yZVxuICovXG5mdW5jdGlvbiBtb3ZlQmxvY2sgKGZyYWdCbG9jaywgYWZ0ZXIpIHtcbiAgY29uc3QgcGFyZW50ID0gYWZ0ZXIucGFyZW50Tm9kZVxuXG4gIGlmIChwYXJlbnQpIHtcbiAgICBsZXQgZWwgPSBmcmFnQmxvY2suc3RhcnRcbiAgICBsZXQgc2lnbmFsXG4gICAgY29uc3QgZ3JvdXAgPSBbZWxdXG5cbiAgICB3aGlsZSAoZWwgJiYgZWwgIT09IGZyYWdCbG9jay5lbmQpIHtcbiAgICAgIGVsID0gZWwubmV4dFNpYmxpbmdcbiAgICAgIGdyb3VwLnB1c2goZWwpXG4gICAgfVxuXG4gICAgbGV0IHRlbXAgPSBhZnRlclxuICAgIGdyb3VwLmV2ZXJ5KChlbCkgPT4ge1xuICAgICAgc2lnbmFsID0gcGFyZW50Lmluc2VydEFmdGVyKGVsLCB0ZW1wKVxuICAgICAgdGVtcCA9IGVsXG4gICAgICByZXR1cm4gc2lnbmFsICE9PSAtMVxuICAgIH0pXG5cbiAgICByZXR1cm4gc2lnbmFsXG4gIH1cbn1cblxuLyoqXG4gKiBSZW1vdmUgdGFyZ2V0IGZyb20gRE9NIHRyZWUuXG4gKiBJZiB0aGUgdGFyZ2V0IGlzIGEgZnJhZyBibG9jayB0aGVuIGNhbGwgX3JlbW92ZUJsb2NrXG4gKlxuICogQHBhcmFtICB7b2JqZWN0fSB0YXJnZXRcbiAqL1xuZXhwb3J0IGZ1bmN0aW9uIHJlbW92ZVRhcmdldCAodm0sIHRhcmdldCwgcHJlc2VydmVCbG9jayA9IGZhbHNlKSB7XG4gIGlmICh0YXJnZXQuZWxlbWVudCkge1xuICAgIHJlbW92ZUJsb2NrKHRhcmdldCwgcHJlc2VydmVCbG9jaylcbiAgfVxuICBlbHNlIHtcbiAgICByZW1vdmVFbGVtZW50KHRhcmdldClcbiAgfVxufVxuXG4vKipcbiAqIFJlbW92ZSBhIGNlcnRhaW4gZWxlbWVudC5cbiAqIFVzaW5nIHRoaXMuX2FwcC5kb2NcbiAqXG4gKiBAcGFyYW0gIHtvYmplY3R9IHRhcmdldFxuICovXG5mdW5jdGlvbiByZW1vdmVFbGVtZW50ICh0YXJnZXQpIHtcbiAgY29uc3QgcGFyZW50ID0gdGFyZ2V0LnBhcmVudE5vZGVcblxuICBpZiAocGFyZW50KSB7XG4gICAgcGFyZW50LnJlbW92ZUNoaWxkKHRhcmdldClcbiAgfVxufVxuXG4vKipcbiAqIFJlbW92ZSBhIGZyYWcgYmxvY2suXG4gKiBUaGUgc2Vjb25kIHBhcmFtIGRlY2lkZXMgd2hldGhlciB0aGUgYmxvY2sgc2VsZiBzaG91bGQgYmUgcmVtb3ZlZCB0b28uXG4gKlxuICogQHBhcmFtICB7b2JqZWN0fSAgZnJhZ0Jsb2NrXG4gKiBAcGFyYW0gIHtCb29sZWFufSBwcmVzZXJ2ZUJsb2NrPWZhbHNlXG4gKi9cbmZ1bmN0aW9uIHJlbW92ZUJsb2NrIChmcmFnQmxvY2ssIHByZXNlcnZlQmxvY2sgPSBmYWxzZSkge1xuICBjb25zdCByZXN1bHQgPSBbXVxuICBsZXQgZWwgPSBmcmFnQmxvY2suc3RhcnQubmV4dFNpYmxpbmdcblxuICB3aGlsZSAoZWwgJiYgZWwgIT09IGZyYWdCbG9jay5lbmQpIHtcbiAgICByZXN1bHQucHVzaChlbClcbiAgICBlbCA9IGVsLm5leHRTaWJsaW5nXG4gIH1cblxuICBpZiAoIXByZXNlcnZlQmxvY2spIHtcbiAgICByZW1vdmVFbGVtZW50KGZyYWdCbG9jay5zdGFydClcbiAgfVxuICByZXN1bHQuZm9yRWFjaCgoZWwpID0+IHtcbiAgICByZW1vdmVFbGVtZW50KGVsKVxuICB9KVxuICBpZiAoIXByZXNlcnZlQmxvY2spIHtcbiAgICByZW1vdmVFbGVtZW50KGZyYWdCbG9jay5lbmQpXG4gIH1cbn1cblxuXG5cblxuLyoqIFdFQlBBQ0sgRk9PVEVSICoqXG4gKiogL1VzZXJzL3poYW9qaW5qaWFuZy9TaXRlcy9wdWJsaWMvd2VleC9kaXN0L3dlZXgtanMtZnJhbWV3b3JrL3NyYy9kZWZhdWx0L3ZtL2RvbS1oZWxwZXIuanNcbiAqKi8iLCIvKipcbiAqIEBmaWxlT3ZlcnZpZXdcbiAqIEV2ZXJ5dGhpbmcgYWJvdXQgY29tcG9uZW50IGV2ZW50IHdoaWNoIGluY2x1ZGVzIGV2ZW50IG9iamVjdCwgZXZlbnQgbGlzdGVuZXIsXG4gKiBldmVudCBlbWl0dGVyIGFuZCBsaWZlY3ljbGUgaG9va3MuXG4gKi9cblxuLyoqXG4gKiBFdmVudCBvYmplY3QgZGVmaW5pdGlvbi4gQW4gZXZlbnQgb2JqZWN0IGhhcyBgdHlwZWAsIGB0aW1lc3RhbXBgIGFuZFxuICogYGRldGFpbGAgZnJvbSB3aGljaCBhIGNvbXBvbmVudCBlbWl0LiBUaGUgZXZlbnQgb2JqZWN0IGNvdWxkIGJlIGRpc3BhdGNoZWQgdG9cbiAqIHBhcmVudHMgb3IgYnJvYWRjYXN0ZWQgdG8gY2hpbGRyZW4gZXhjZXB0IGB0aGlzLnN0b3AoKWAgaXMgY2FsbGVkLlxuICogQHBhcmFtIHtzdHJpbmd9IHR5cGVcbiAqIEBwYXJhbSB7YW55fSAgICBkZXRhaWxcbiAqL1xuZnVuY3Rpb24gRXZ0ICh0eXBlLCBkZXRhaWwpIHtcbiAgaWYgKGRldGFpbCBpbnN0YW5jZW9mIEV2dCkge1xuICAgIHJldHVybiBkZXRhaWxcbiAgfVxuXG4gIHRoaXMudGltZXN0YW1wID0gRGF0ZS5ub3coKVxuICB0aGlzLmRldGFpbCA9IGRldGFpbFxuICB0aGlzLnR5cGUgPSB0eXBlXG5cbiAgbGV0IHNob3VsZFN0b3AgPSBmYWxzZVxuXG4gIC8qKlxuICAgKiBzdG9wIGRpc3BhdGNoIGFuZCBicm9hZGNhc3RcbiAgICovXG4gIHRoaXMuc3RvcCA9IGZ1bmN0aW9uICgpIHtcbiAgICBzaG91bGRTdG9wID0gdHJ1ZVxuICB9XG5cbiAgLyoqXG4gICAqIGNoZWNrIGlmIGl0IGNhbid0IGJlIGRpc3BhdGNoZWQgb3IgYnJvYWRjYXN0ZWRcbiAgICovXG4gIHRoaXMuaGFzU3RvcHBlZCA9IGZ1bmN0aW9uICgpIHtcbiAgICByZXR1cm4gc2hvdWxkU3RvcFxuICB9XG59XG5cbi8qKlxuICogRW1pdCBhbiBldmVudCBidXQgbm90IGJyb2FkY2FzdCBkb3duIG9yIGRpc3BhdGNoIHVwLlxuICogQHBhcmFtICB7c3RyaW5nfSB0eXBlXG4gKiBAcGFyYW0gIHthbnl9ICAgIGRldGFpbFxuICovXG5leHBvcnQgZnVuY3Rpb24gJGVtaXQgKHR5cGUsIGRldGFpbCkge1xuICBjb25zdCBldmVudHMgPSB0aGlzLl92bUV2ZW50c1xuICBjb25zdCBoYW5kbGVyTGlzdCA9IGV2ZW50c1t0eXBlXVxuICBpZiAoaGFuZGxlckxpc3QpIHtcbiAgICBjb25zdCBldnQgPSBuZXcgRXZ0KHR5cGUsIGRldGFpbClcbiAgICBoYW5kbGVyTGlzdC5mb3JFYWNoKChoYW5kbGVyKSA9PiB7XG4gICAgICBoYW5kbGVyLmNhbGwodGhpcywgZXZ0KVxuICAgIH0pXG4gIH1cbn1cblxuLyoqXG4gKiBFbWl0IGFuIGV2ZW50IGFuZCBkaXNwYXRjaCBpdCB1cC5cbiAqIEBwYXJhbSAge3N0cmluZ30gdHlwZVxuICogQHBhcmFtICB7YW55fSAgICBkZXRhaWxcbiAqL1xuZXhwb3J0IGZ1bmN0aW9uICRkaXNwYXRjaCAodHlwZSwgZGV0YWlsKSB7XG4gIGNvbnN0IGV2dCA9IG5ldyBFdnQodHlwZSwgZGV0YWlsKVxuICB0aGlzLiRlbWl0KHR5cGUsIGV2dClcblxuICBpZiAoIWV2dC5oYXNTdG9wcGVkKCkgJiYgdGhpcy5fcGFyZW50ICYmIHRoaXMuX3BhcmVudC4kZGlzcGF0Y2gpIHtcbiAgICB0aGlzLl9wYXJlbnQuJGRpc3BhdGNoKHR5cGUsIGV2dClcbiAgfVxufVxuXG4vKipcbiAqIEVtaXQgYW4gZXZlbnQgYW5kIGJyb2FkY2FzdCBpdCBkb3duLlxuICogQHBhcmFtICB7c3RyaW5nfSB0eXBlXG4gKiBAcGFyYW0gIHthbnl9ICAgIGRldGFpbFxuICovXG5leHBvcnQgZnVuY3Rpb24gJGJyb2FkY2FzdCAodHlwZSwgZGV0YWlsKSB7XG4gIGNvbnN0IGV2dCA9IG5ldyBFdnQodHlwZSwgZGV0YWlsKVxuICB0aGlzLiRlbWl0KHR5cGUsIGV2dClcblxuICBpZiAoIWV2dC5oYXNTdG9wcGVkKCkgJiYgdGhpcy5fY2hpbGRyZW5WbXMpIHtcbiAgICB0aGlzLl9jaGlsZHJlblZtcy5mb3JFYWNoKChzdWJWbSkgPT4ge1xuICAgICAgc3ViVm0uJGJyb2FkY2FzdCh0eXBlLCBldnQpXG4gICAgfSlcbiAgfVxufVxuXG4vKipcbiAqIEFkZCBldmVudCBsaXN0ZW5lci5cbiAqIEBwYXJhbSAge3N0cmluZ30gICB0eXBlXG4gKiBAcGFyYW0gIHtmdW5jdGlvbn0gaGFuZGxlclxuICovXG5leHBvcnQgZnVuY3Rpb24gJG9uICh0eXBlLCBoYW5kbGVyKSB7XG4gIGlmICghdHlwZSB8fCB0eXBlb2YgaGFuZGxlciAhPT0gJ2Z1bmN0aW9uJykge1xuICAgIHJldHVyblxuICB9XG4gIGNvbnN0IGV2ZW50cyA9IHRoaXMuX3ZtRXZlbnRzXG4gIGNvbnN0IGhhbmRsZXJMaXN0ID0gZXZlbnRzW3R5cGVdIHx8IFtdXG4gIGhhbmRsZXJMaXN0LnB1c2goaGFuZGxlcilcbiAgZXZlbnRzW3R5cGVdID0gaGFuZGxlckxpc3RcblxuICAvLyBmaXhlZCBvbGQgdmVyc2lvbiBsaWZlY3ljbGUgZGVzaWduXG4gIGlmICh0eXBlID09PSAnaG9vazpyZWFkeScgJiYgdGhpcy5fcmVhZHkpIHtcbiAgICB0aGlzLiRlbWl0KCdob29rOnJlYWR5JylcbiAgfVxufVxuXG4vKipcbiAqIFJlbW92ZSBldmVudCBsaXN0ZW5lci5cbiAqIEBwYXJhbSAge3N0cmluZ30gICB0eXBlXG4gKiBAcGFyYW0gIHtmdW5jdGlvbn0gaGFuZGxlclxuICovXG5leHBvcnQgZnVuY3Rpb24gJG9mZiAodHlwZSwgaGFuZGxlcikge1xuICBpZiAoIXR5cGUpIHtcbiAgICByZXR1cm5cbiAgfVxuICBjb25zdCBldmVudHMgPSB0aGlzLl92bUV2ZW50c1xuICBpZiAoIWhhbmRsZXIpIHtcbiAgICBkZWxldGUgZXZlbnRzW3R5cGVdXG4gICAgcmV0dXJuXG4gIH1cbiAgY29uc3QgaGFuZGxlckxpc3QgPSBldmVudHNbdHlwZV1cbiAgaWYgKCFoYW5kbGVyTGlzdCkge1xuICAgIHJldHVyblxuICB9XG4gIGhhbmRsZXJMaXN0LiRyZW1vdmUoaGFuZGxlcilcbn1cblxuY29uc3QgTElGRV9DWUNMRV9UWVBFUyA9IFsnaW5pdCcsICdjcmVhdGVkJywgJ3JlYWR5J11cblxuLyoqXG4gKiBJbml0IGV2ZW50czpcbiAqIDEuIGxpc3RlbiBgZXZlbnRzYCBpbiBjb21wb25lbnQgb3B0aW9ucyAmIGBleHRlcm5hbEV2ZW50c2AuXG4gKiAyLiBiaW5kIGxpZmVjeWNsZSBob29rcy5cbiAqIEBwYXJhbSAge1ZtfSAgICAgdm1cbiAqIEBwYXJhbSAge29iamVjdH0gZXh0ZXJuYWxFdmVudHNcbiAqL1xuZXhwb3J0IGZ1bmN0aW9uIGluaXRFdmVudHMgKHZtLCBleHRlcm5hbEV2ZW50cykge1xuICBjb25zdCBvcHRpb25zID0gdm0uX29wdGlvbnMgfHwge31cbiAgY29uc3QgZXZlbnRzID0gb3B0aW9ucy5ldmVudHMgfHwge31cbiAgZm9yIChjb25zdCB0eXBlMSBpbiBldmVudHMpIHtcbiAgICB2bS4kb24odHlwZTEsIGV2ZW50c1t0eXBlMV0pXG4gIH1cbiAgZm9yIChjb25zdCB0eXBlMiBpbiBleHRlcm5hbEV2ZW50cykge1xuICAgIHZtLiRvbih0eXBlMiwgZXh0ZXJuYWxFdmVudHNbdHlwZTJdKVxuICB9XG4gIExJRkVfQ1lDTEVfVFlQRVMuZm9yRWFjaCgodHlwZSkgPT4ge1xuICAgIHZtLiRvbihgaG9vazoke3R5cGV9YCwgb3B0aW9uc1t0eXBlXSlcbiAgfSlcbn1cblxuLyoqXG4gKiBCaW5kIGV2ZW50IHJlbGF0ZWQgbWV0aG9kcyB0byBWaWV3TW9kZWwgaW5zdGFuY2UuXG4gKiBAcGFyYW0gIHtWbX0gdm1cbiAqL1xuZXhwb3J0IGZ1bmN0aW9uIG1peGluRXZlbnRzICh2bSkge1xuICB2bS4kZW1pdCA9ICRlbWl0XG4gIHZtLiRkaXNwYXRjaCA9ICRkaXNwYXRjaFxuICB2bS4kYnJvYWRjYXN0ID0gJGJyb2FkY2FzdFxuICB2bS4kb24gPSAkb25cbiAgdm0uJG9mZiA9ICRvZmZcbn1cblxuXG5cbi8qKiBXRUJQQUNLIEZPT1RFUiAqKlxuICoqIC9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9zcmMvZGVmYXVsdC92bS9ldmVudHMuanNcbiAqKi8iLCJpbXBvcnQgc2VtdmVyIGZyb20gJ3NlbXZlcidcbmltcG9ydCB7IGlzUGxhaW5PYmplY3QsIHR5cG9mIH0gZnJvbSAnLi4vdXRpbCdcblxuLyoqXG4gKiBOb3JtYWxpemUgYSB2ZXJzaW9uIHN0cmluZy5cbiAqIEBwYXJhbSAge1N0cmluZ30gVmVyc2lvbi4gaWU6IDEsIDEuMCwgMS4wLjBcbiAqIEByZXR1cm4ge1N0cmluZ30gVmVyc2lvblxuICovXG5leHBvcnQgZnVuY3Rpb24gbm9ybWFsaXplVmVyc2lvbiAodikge1xuICBjb25zdCBpc1ZhbGlkID0gc2VtdmVyLnZhbGlkKHYpXG4gIGlmIChpc1ZhbGlkKSB7XG4gICAgcmV0dXJuIHZcbiAgfVxuXG4gIHYgPSB0eXBlb2YgKHYpID09PSAnc3RyaW5nJyA/IHYgOiAnJ1xuICBjb25zdCBzcGxpdCA9IHYuc3BsaXQoJy4nKVxuICBsZXQgaSA9IDBcbiAgY29uc3QgcmVzdWx0ID0gW11cblxuICB3aGlsZSAoaSA8IDMpIHtcbiAgICBjb25zdCBzID0gdHlwZW9mIChzcGxpdFtpXSkgPT09ICdzdHJpbmcnICYmIHNwbGl0W2ldID8gc3BsaXRbaV0gOiAnMCdcbiAgICByZXN1bHQucHVzaChzKVxuICAgIGkrK1xuICB9XG5cbiAgcmV0dXJuIHJlc3VsdC5qb2luKCcuJylcbn1cblxuLyoqXG4gKiBHZXQgaW5mb3JtYXRpb25zIGZyb20gZGlmZmVyZW50IGVycm9yIGtleS4gTGlrZTpcbiAqIC0gY29kZVxuICogLSBlcnJvck1lc3NhZ2VcbiAqIC0gZXJyb3JUeXBlXG4gKiAtIGlzRG93bmdyYWRlXG4gKiBAcGFyYW0gIHtzdHJpbmd9IGtleVxuICogQHBhcmFtICB7c3RyaW5nfSB2YWxcbiAqIEBwYXJhbSAge3N0cmluZ30gY3JpdGVyaWFcbiAqIEByZXR1cm4ge29iamVjdH1cbiAqL1xuZXhwb3J0IGZ1bmN0aW9uIGdldEVycm9yIChrZXksIHZhbCwgY3JpdGVyaWEpIHtcbiAgY29uc3QgcmVzdWx0ID0ge1xuICAgIGlzRG93bmdyYWRlOiB0cnVlLFxuICAgIGVycm9yVHlwZTogMSxcbiAgICBjb2RlOiAxMDAwXG4gIH1cbiAgY29uc3QgZ2V0TXNnID0gZnVuY3Rpb24gKGtleSwgdmFsLCBjcml0ZXJpYSkge1xuICAgIHJldHVybiAnRG93bmdyYWRlWycgKyBrZXkgKyAnXSA6OiBkZXZpY2VJbmZvICdcbiAgICAgICsgdmFsICsgJyBtYXRjaGVkIGNyaXRlcmlhICcgKyBjcml0ZXJpYVxuICB9XG4gIGNvbnN0IF9rZXkgPSBrZXkudG9Mb3dlckNhc2UoKVxuXG4gIHJlc3VsdC5lcnJvck1lc3NhZ2UgPSBnZXRNc2coa2V5LCB2YWwsIGNyaXRlcmlhKVxuXG4gIGlmIChfa2V5LmluZGV4T2YoJ29zdmVyc2lvbicpID49IDApIHtcbiAgICByZXN1bHQuY29kZSA9IDEwMDFcbiAgfVxuICBlbHNlIGlmIChfa2V5LmluZGV4T2YoJ2FwcHZlcnNpb24nKSA+PSAwKSB7XG4gICAgcmVzdWx0LmNvZGUgPSAxMDAyXG4gIH1cbiAgZWxzZSBpZiAoX2tleS5pbmRleE9mKCd3ZWV4dmVyc2lvbicpID49IDApIHtcbiAgICByZXN1bHQuY29kZSA9IDEwMDNcbiAgfVxuICBlbHNlIGlmIChfa2V5LmluZGV4T2YoJ2RldmljZW1vZGVsJykgPj0gMCkge1xuICAgIHJlc3VsdC5jb2RlID0gMTAwNFxuICB9XG5cbiAgcmV0dXJuIHJlc3VsdFxufVxuXG4vKipcbiAqIFdFRVggZnJhbWV3b3JrIGlucHV0KGRldmljZUluZm8pXG4gKiB7XG4gKiAgIHBsYXRmb3JtOiAnaU9TJyBvciAnYW5kcm9pZCdcbiAqICAgb3NWZXJzaW9uOiAnMS4wLjAnIG9yICcxLjAnIG9yICcxJ1xuICogICBhcHBWZXJzaW9uOiAnMS4wLjAnIG9yICcxLjAnIG9yICcxJ1xuICogICB3ZWV4VmVyc2lvbjogJzEuMC4wJyBvciAnMS4wJyBvciAnMSdcbiAqICAgZERldmljZU1vZGVsOiAnTU9ERUxfTkFNRSdcbiAqIH1cbiAqXG4gKiBkb3duZ3JhZGUgY29uZmlnKGNvbmZpZylcbiAqIHtcbiAqICAgaW9zOiB7XG4gKiAgICAgb3NWZXJzaW9uOiAnPjEuMC4wJyBvciAnPj0xLjAuMCcgb3IgJzwxLjAuMCcgb3IgJzw9MS4wLjAnIG9yICcxLjAuMCdcbiAqICAgICBhcHBWZXJzaW9uOiAnPjEuMC4wJyBvciAnPj0xLjAuMCcgb3IgJzwxLjAuMCcgb3IgJzw9MS4wLjAnIG9yICcxLjAuMCdcbiAqICAgICB3ZWV4VmVyc2lvbjogJz4xLjAuMCcgb3IgJz49MS4wLjAnIG9yICc8MS4wLjAnIG9yICc8PTEuMC4wJyBvciAnMS4wLjAnXG4gKiAgICAgZGV2aWNlTW9kZWw6IFsnbW9kZWxBJywgJ21vZGVsQicsIC4uLl1cbiAqICAgfSxcbiAqICAgYW5kcm9pZDoge1xuICogICAgIG9zVmVyc2lvbjogJz4xLjAuMCcgb3IgJz49MS4wLjAnIG9yICc8MS4wLjAnIG9yICc8PTEuMC4wJyBvciAnMS4wLjAnXG4gKiAgICAgYXBwVmVyc2lvbjogJz4xLjAuMCcgb3IgJz49MS4wLjAnIG9yICc8MS4wLjAnIG9yICc8PTEuMC4wJyBvciAnMS4wLjAnXG4gKiAgICAgd2VleFZlcnNpb246ICc+MS4wLjAnIG9yICc+PTEuMC4wJyBvciAnPDEuMC4wJyBvciAnPD0xLjAuMCcgb3IgJzEuMC4wJ1xuICogICAgIGRldmljZU1vZGVsOiBbJ21vZGVsQScsICdtb2RlbEInLCAuLi5dXG4gKiAgIH1cbiAqIH1cbiAqXG4gKlxuICogQHBhcmFtICB7b2JqZWN0fSBkZXZpY2VJbmZvIFdlZXggU0RLIGZyYW1ld29yayBpbnB1dFxuICogQHBhcmFtICB7b2JqZWN0fSBjb25maWcgICAgIHVzZXIgaW5wdXRcbiAqIEByZXR1cm4ge09iamVjdH0gICAgICAgICAgICB7IGlzRG93bmdyYWRlOiB0cnVlL2ZhbHNlLCBlcnJvck1lc3NhZ2UuLi4gfVxuICovXG5leHBvcnQgZnVuY3Rpb24gY2hlY2sgKGNvbmZpZywgZGV2aWNlSW5mbykge1xuICBkZXZpY2VJbmZvID0gZGV2aWNlSW5mbyB8fCBnbG9iYWwuV1hFbnZpcm9ubWVudFxuICBkZXZpY2VJbmZvID0gaXNQbGFpbk9iamVjdChkZXZpY2VJbmZvKSA/IGRldmljZUluZm8gOiB7fVxuXG4gIGxldCByZXN1bHQgPSB7XG4gICAgaXNEb3duZ3JhZGU6IGZhbHNlIC8vIGRlZmF1dGwgaXMgcGFzc1xuICB9XG5cbiAgaWYgKHR5cG9mKGNvbmZpZykgPT09ICdmdW5jdGlvbicpIHtcbiAgICBsZXQgY3VzdG9tRG93bmdyYWRlID0gY29uZmlnLmNhbGwodGhpcywgZGV2aWNlSW5mbywge1xuICAgICAgc2VtdmVyOiBzZW12ZXIsXG4gICAgICBub3JtYWxpemVWZXJzaW9uOiB0aGlzLm5vcm1hbGl6ZVZlcnNpb25cbiAgICB9KVxuXG4gICAgY3VzdG9tRG93bmdyYWRlID0gISFjdXN0b21Eb3duZ3JhZGVcblxuICAgIHJlc3VsdCA9IGN1c3RvbURvd25ncmFkZSA/IHRoaXMuZ2V0RXJyb3IoJ2N1c3RvbScsICcnLCAnY3VzdG9tIHBhcmFtcycpIDogcmVzdWx0XG4gIH1cbiAgZWxzZSB7XG4gICAgY29uZmlnID0gaXNQbGFpbk9iamVjdChjb25maWcpID8gY29uZmlnIDoge31cblxuICAgIGNvbnN0IHBsYXRmb3JtID0gZGV2aWNlSW5mby5wbGF0Zm9ybSB8fCAndW5rbm93J1xuICAgIGNvbnN0IGRQbGF0Zm9ybSA9IHBsYXRmb3JtLnRvTG93ZXJDYXNlKClcbiAgICBjb25zdCBjT2JqID0gY29uZmlnW2RQbGF0Zm9ybV0gfHwge31cblxuICAgIGZvciAoY29uc3QgaSBpbiBkZXZpY2VJbmZvKSB7XG4gICAgICBjb25zdCBrZXkgPSBpXG4gICAgICBjb25zdCBrZXlMb3dlciA9IGtleS50b0xvd2VyQ2FzZSgpXG4gICAgICBjb25zdCB2YWwgPSBkZXZpY2VJbmZvW2ldXG4gICAgICBjb25zdCBpc1ZlcnNpb24gPSBrZXlMb3dlci5pbmRleE9mKCd2ZXJzaW9uJykgPj0gMFxuICAgICAgY29uc3QgaXNEZXZpY2VNb2RlbCA9IGtleUxvd2VyLmluZGV4T2YoJ2RldmljZW1vZGVsJykgPj0gMFxuICAgICAgY29uc3QgY3JpdGVyaWEgPSBjT2JqW2ldXG5cbiAgICAgIGlmIChjcml0ZXJpYSAmJiBpc1ZlcnNpb24pIHtcbiAgICAgICAgY29uc3QgYyA9IHRoaXMubm9ybWFsaXplVmVyc2lvbihjcml0ZXJpYSlcbiAgICAgICAgY29uc3QgZCA9IHRoaXMubm9ybWFsaXplVmVyc2lvbihkZXZpY2VJbmZvW2ldKVxuXG4gICAgICAgIGlmIChzZW12ZXIuc2F0aXNmaWVzKGQsIGMpKSB7XG4gICAgICAgICAgcmVzdWx0ID0gdGhpcy5nZXRFcnJvcihrZXksIHZhbCwgY3JpdGVyaWEpXG4gICAgICAgICAgYnJlYWtcbiAgICAgICAgfVxuICAgICAgfVxuICAgICAgZWxzZSBpZiAoaXNEZXZpY2VNb2RlbCkge1xuICAgICAgICBjb25zdCBfY3JpdGVyaWEgPSB0eXBvZihjcml0ZXJpYSkgPT09ICdhcnJheScgPyBjcml0ZXJpYSA6IFtjcml0ZXJpYV1cbiAgICAgICAgaWYgKF9jcml0ZXJpYS5pbmRleE9mKHZhbCkgPj0gMCkge1xuICAgICAgICAgIHJlc3VsdCA9IHRoaXMuZ2V0RXJyb3Ioa2V5LCB2YWwsIGNyaXRlcmlhKVxuICAgICAgICAgIGJyZWFrXG4gICAgICAgIH1cbiAgICAgIH1cbiAgICB9XG4gIH1cblxuICByZXR1cm4gcmVzdWx0XG59XG5cblxuXG4vKiogV0VCUEFDSyBGT09URVIgKipcbiAqKiAvVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvc3JjL2RlZmF1bHQvYXBwL2Rvd25ncmFkZS5qc1xuICoqLyIsIi8qKlxuICogQGZpbGVPdmVydmlld1xuICogQSBzaW1wbGUgdmlydHVhbCBkb20gaW1wbGVtZW50YXRpb25cbiAqL1xuXG5pbXBvcnQgTGlzdGVuZXIgZnJvbSAnLi9saXN0ZW5lcidcbmltcG9ydCB7IGV4dGVuZCB9IGZyb20gJy4uL3NoYXJlZCdcblxuY29uc3QgREVGQVVMVF9UQUdfTkFNRSA9ICdkaXYnXG5cbmV4cG9ydCBjb25zdCBpbnN0YW5jZU1hcCA9IHt9XG5sZXQgbmV4dE5vZGVSZWYgPSAxXG5cbmV4cG9ydCBmdW5jdGlvbiBEb2N1bWVudCAoaWQsIHVybCwgaGFuZGxlcikge1xuICBpZCA9IGlkID8gaWQudG9TdHJpbmcoKSA6ICcnXG4gIHRoaXMuaWQgPSBpZFxuICB0aGlzLlVSTCA9IHVybFxuXG4gIGluc3RhbmNlTWFwW2lkXSA9IHRoaXNcbiAgdGhpcy5ub2RlTWFwID0ge31cbiAgdGhpcy5saXN0ZW5lciA9IG5ldyBMaXN0ZW5lcihpZCwgaGFuZGxlciB8fCBnZW5DYWxsVGFza3MoaWQpKVxuICB0aGlzLmNyZWF0ZURvY3VtZW50RWxlbWVudCgpXG59XG5cbmZ1bmN0aW9uIGdlbkNhbGxUYXNrcyAoaWQpIHtcbiAgcmV0dXJuICh0YXNrcykgPT4ge1xuICAgIGlmICghQXJyYXkuaXNBcnJheSh0YXNrcykpIHtcbiAgICAgIHRhc2tzID0gW3Rhc2tzXVxuICAgIH1cbiAgICByZXR1cm4gY2FsbE5hdGl2ZShpZCwgdGFza3MsICctMScpXG4gIH1cbn1cblxuRG9jdW1lbnQucHJvdG90eXBlLmRlc3Ryb3kgPSBmdW5jdGlvbiAoKSB7XG4gIGRlbGV0ZSB0aGlzLmxpc3RlbmVyXG4gIGRlbGV0ZSB0aGlzLm5vZGVNYXBcbiAgZGVsZXRlIGluc3RhbmNlTWFwW3RoaXMuaWRdXG59XG5cbkRvY3VtZW50LnByb3RvdHlwZS5vcGVuID0gZnVuY3Rpb24gKCkge1xuICB0aGlzLmxpc3RlbmVyLmJhdGNoZWQgPSBmYWxzZVxufVxuXG5Eb2N1bWVudC5wcm90b3R5cGUuY2xvc2UgPSBmdW5jdGlvbiAoKSB7XG4gIHRoaXMubGlzdGVuZXIuYmF0Y2hlZCA9IHRydWVcbn1cblxuRG9jdW1lbnQucHJvdG90eXBlLmNyZWF0ZURvY3VtZW50RWxlbWVudCA9IGZ1bmN0aW9uICgpIHtcbiAgaWYgKCF0aGlzLmRvY3VtZW50RWxlbWVudCkge1xuICAgIGNvbnN0IGVsID0gbmV3IEVsZW1lbnQoJ2RvY3VtZW50JylcbiAgICBlbC5kb2NJZCA9IHRoaXMuaWRcbiAgICBlbC5vd25lckRvY3VtZW50ID0gdGhpc1xuICAgIGVsLnJvbGUgPSAnZG9jdW1lbnRFbGVtZW50J1xuICAgIGVsLmRlcHRoID0gMFxuICAgIGVsLnJlZiA9ICdfZG9jdW1lbnRFbGVtZW50J1xuICAgIHRoaXMubm9kZU1hcC5fZG9jdW1lbnRFbGVtZW50ID0gZWxcbiAgICB0aGlzLmRvY3VtZW50RWxlbWVudCA9IGVsXG4gICAgZWwuYXBwZW5kQ2hpbGQgPSAobm9kZSkgPT4ge1xuICAgICAgYXBwZW5kQm9keSh0aGlzLCBub2RlKVxuICAgIH1cbiAgICBlbC5pbnNlcnRCZWZvcmUgPSAobm9kZSwgYmVmb3JlKSA9PiB7XG4gICAgICBhcHBlbmRCb2R5KHRoaXMsIG5vZGUsIGJlZm9yZSlcbiAgICB9XG4gIH1cblxuICByZXR1cm4gdGhpcy5kb2N1bWVudEVsZW1lbnRcbn1cblxuZnVuY3Rpb24gYXBwZW5kQm9keSAoZG9jLCBub2RlLCBiZWZvcmUpIHtcbiAgY29uc3QgeyBkb2N1bWVudEVsZW1lbnQgfSA9IGRvY1xuXG4gIGlmIChkb2N1bWVudEVsZW1lbnQucHVyZUNoaWxkcmVuLmxlbmd0aCA+IDAgfHwgbm9kZS5wYXJlbnROb2RlKSB7XG4gICAgcmV0dXJuXG4gIH1cbiAgY29uc3QgY2hpbGRyZW4gPSBkb2N1bWVudEVsZW1lbnQuY2hpbGRyZW5cbiAgY29uc3QgYmVmb3JlSW5kZXggPSBjaGlsZHJlbi5pbmRleE9mKGJlZm9yZSlcbiAgaWYgKGJlZm9yZUluZGV4IDwgMCkge1xuICAgIGNoaWxkcmVuLnB1c2gobm9kZSlcbiAgfVxuICBlbHNlIHtcbiAgICBjaGlsZHJlbi5zcGxpY2UoYmVmb3JlSW5kZXgsIDAsIG5vZGUpXG4gIH1cblxuICBpZiAobm9kZS5ub2RlVHlwZSA9PT0gMSkge1xuICAgIGlmIChub2RlLnJvbGUgPT09ICdib2R5Jykge1xuICAgICAgbm9kZS5kb2NJZCA9IGRvYy5pZFxuICAgICAgbm9kZS5vd25lckRvY3VtZW50ID0gZG9jXG4gICAgICBub2RlLnBhcmVudE5vZGUgPSBkb2N1bWVudEVsZW1lbnRcbiAgICB9XG4gICAgZWxzZSB7XG4gICAgICBub2RlLmNoaWxkcmVuLmZvckVhY2goY2hpbGQgPT4ge1xuICAgICAgICBjaGlsZC5wYXJlbnROb2RlID0gbm9kZVxuICAgICAgfSlcbiAgICAgIHNldEJvZHkoZG9jLCBub2RlKVxuICAgICAgbm9kZS5kb2NJZCA9IGRvYy5pZFxuICAgICAgbm9kZS5vd25lckRvY3VtZW50ID0gZG9jXG4gICAgICBsaW5rUGFyZW50KG5vZGUsIGRvY3VtZW50RWxlbWVudClcbiAgICAgIGRlbGV0ZSBkb2Mubm9kZU1hcFtub2RlLm5vZGVJZF1cbiAgICB9XG4gICAgZG9jdW1lbnRFbGVtZW50LnB1cmVDaGlsZHJlbi5wdXNoKG5vZGUpXG4gICAgZG9jLmxpc3RlbmVyLmNyZWF0ZUJvZHkobm9kZSlcbiAgfVxuICBlbHNlIHtcbiAgICBub2RlLnBhcmVudE5vZGUgPSBkb2N1bWVudEVsZW1lbnRcbiAgICBkb2Mubm9kZU1hcFtub2RlLnJlZl0gPSBub2RlXG4gIH1cbn1cblxuZnVuY3Rpb24gc2V0Qm9keSAoZG9jLCBlbCkge1xuICBlbC5yb2xlID0gJ2JvZHknXG4gIGVsLmRlcHRoID0gMVxuICBkZWxldGUgZG9jLm5vZGVNYXBbZWwubm9kZUlkXVxuICBlbC5yZWYgPSAnX3Jvb3QnXG4gIGRvYy5ub2RlTWFwLl9yb290ID0gZWxcbiAgZG9jLmJvZHkgPSBlbFxufVxuXG5Eb2N1bWVudC5wcm90b3R5cGUuY3JlYXRlQm9keSA9IGZ1bmN0aW9uICh0eXBlLCBwcm9wcykge1xuICBpZiAoIXRoaXMuYm9keSkge1xuICAgIGNvbnN0IGVsID0gbmV3IEVsZW1lbnQodHlwZSwgcHJvcHMpXG4gICAgc2V0Qm9keSh0aGlzLCBlbClcbiAgfVxuXG4gIHJldHVybiB0aGlzLmJvZHlcbn1cblxuRG9jdW1lbnQucHJvdG90eXBlLmNyZWF0ZUVsZW1lbnQgPSBmdW5jdGlvbiAodGFnTmFtZSwgcHJvcHMpIHtcbiAgcmV0dXJuIG5ldyBFbGVtZW50KHRhZ05hbWUsIHByb3BzKVxufVxuXG5Eb2N1bWVudC5wcm90b3R5cGUuY3JlYXRlQ29tbWVudCA9IGZ1bmN0aW9uICh0ZXh0KSB7XG4gIHJldHVybiBuZXcgQ29tbWVudCh0ZXh0KVxufVxuXG5Eb2N1bWVudC5wcm90b3R5cGUuZmlyZUV2ZW50ID0gZnVuY3Rpb24gKGVsLCB0eXBlLCBlLCBkb21DaGFuZ2VzKSB7XG4gIGlmICghZWwpIHtcbiAgICByZXR1cm5cbiAgfVxuICBlID0gZSB8fCB7fVxuICBlLnR5cGUgPSB0eXBlXG4gIGUudGFyZ2V0ID0gZWxcbiAgZS50aW1lc3RhbXAgPSBEYXRlLm5vdygpXG4gIGlmIChkb21DaGFuZ2VzKSB7XG4gICAgdXBkYXRlRWxlbWVudChlbCwgZG9tQ2hhbmdlcylcbiAgfVxuICByZXR1cm4gZWwuZmlyZUV2ZW50KHR5cGUsIGUpXG59XG5cbkRvY3VtZW50LnByb3RvdHlwZS5nZXRSZWYgPSBmdW5jdGlvbiAocmVmKSB7XG4gIHJldHVybiB0aGlzLm5vZGVNYXBbcmVmXVxufVxuXG5mdW5jdGlvbiB1cGRhdGVFbGVtZW50IChlbCwgY2hhbmdlcykge1xuICBjb25zdCBhdHRycyA9IGNoYW5nZXMuYXR0cnMgfHwge31cbiAgZm9yIChjb25zdCBuYW1lIGluIGF0dHJzKSB7XG4gICAgZWwuc2V0QXR0cihuYW1lLCBhdHRyc1tuYW1lXSwgdHJ1ZSlcbiAgfVxuICBjb25zdCBzdHlsZSA9IGNoYW5nZXMuc3R5bGUgfHwge31cbiAgZm9yIChjb25zdCBuYW1lIGluIHN0eWxlKSB7XG4gICAgZWwuc2V0U3R5bGUobmFtZSwgc3R5bGVbbmFtZV0sIHRydWUpXG4gIH1cbn1cblxuZXhwb3J0IGZ1bmN0aW9uIE5vZGUgKCkge1xuICB0aGlzLm5vZGVJZCA9IChuZXh0Tm9kZVJlZisrKS50b1N0cmluZygpXG4gIHRoaXMucmVmID0gdGhpcy5ub2RlSWRcbiAgdGhpcy5jaGlsZHJlbiA9IFtdXG4gIHRoaXMucHVyZUNoaWxkcmVuID0gW11cbiAgdGhpcy5wYXJlbnROb2RlID0gbnVsbFxuICB0aGlzLm5leHRTaWJsaW5nID0gbnVsbFxuICB0aGlzLnByZXZpb3VzU2libGluZyA9IG51bGxcbn1cblxuTm9kZS5wcm90b3R5cGUuZGVzdHJveSA9IGZ1bmN0aW9uICgpIHtcbiAgY29uc3QgZG9jID0gaW5zdGFuY2VNYXBbdGhpcy5kb2NJZF1cbiAgaWYgKGRvYykge1xuICAgIGRlbGV0ZSB0aGlzLmRvY0lkXG4gICAgZGVsZXRlIGRvYy5ub2RlTWFwW3RoaXMubm9kZUlkXVxuICB9XG4gIHRoaXMuY2hpbGRyZW4uZm9yRWFjaChjaGlsZCA9PiB7XG4gICAgY2hpbGQuZGVzdHJveSgpXG4gIH0pXG59XG5cbmV4cG9ydCBmdW5jdGlvbiBFbGVtZW50ICh0eXBlID0gREVGQVVMVF9UQUdfTkFNRSwgcHJvcHMpIHtcbiAgcHJvcHMgPSBwcm9wcyB8fCB7fVxuICB0aGlzLm5vZGVUeXBlID0gMVxuICB0aGlzLm5vZGVJZCA9IChuZXh0Tm9kZVJlZisrKS50b1N0cmluZygpXG4gIHRoaXMucmVmID0gdGhpcy5ub2RlSWRcbiAgdGhpcy50eXBlID0gdHlwZVxuICB0aGlzLmF0dHIgPSBwcm9wcy5hdHRyIHx8IHt9XG4gIHRoaXMuY2xhc3NTdHlsZSA9IHByb3BzLmNsYXNzU3R5bGUgfHwge31cbiAgdGhpcy5zdHlsZSA9IHByb3BzLnN0eWxlIHx8IHt9XG4gIHRoaXMuZXZlbnQgPSB7fVxuICB0aGlzLmNoaWxkcmVuID0gW11cbiAgdGhpcy5wdXJlQ2hpbGRyZW4gPSBbXVxufVxuXG5FbGVtZW50LnByb3RvdHlwZSA9IG5ldyBOb2RlKClcblxuRWxlbWVudC5wcm90b3R5cGUuYXBwZW5kQ2hpbGQgPSBmdW5jdGlvbiAobm9kZSkge1xuICBpZiAobm9kZS5wYXJlbnROb2RlICYmIG5vZGUucGFyZW50Tm9kZSAhPT0gdGhpcykge1xuICAgIHJldHVyblxuICB9XG4gIGlmICghbm9kZS5wYXJlbnROb2RlKSB7XG4gICAgbGlua1BhcmVudChub2RlLCB0aGlzKVxuICAgIGluc2VydEluZGV4KG5vZGUsIHRoaXMuY2hpbGRyZW4sIHRoaXMuY2hpbGRyZW4ubGVuZ3RoLCB0cnVlKVxuICAgIGlmICh0aGlzLmRvY0lkKSB7XG4gICAgICByZWdpc3Rlck5vZGUodGhpcy5kb2NJZCwgbm9kZSlcbiAgICB9XG4gICAgaWYgKG5vZGUubm9kZVR5cGUgPT09IDEpIHtcbiAgICAgIGluc2VydEluZGV4KG5vZGUsIHRoaXMucHVyZUNoaWxkcmVuLCB0aGlzLnB1cmVDaGlsZHJlbi5sZW5ndGgpXG4gICAgICBpZiAodGhpcy5kb2NJZCkge1xuICAgICAgICBjb25zdCBsaXN0ZW5lciA9IGluc3RhbmNlTWFwW3RoaXMuZG9jSWRdLmxpc3RlbmVyXG4gICAgICAgIHJldHVybiBsaXN0ZW5lci5hZGRFbGVtZW50KG5vZGUsIHRoaXMucmVmLCAtMSlcbiAgICAgIH1cbiAgICB9XG4gIH1cbiAgZWxzZSB7XG4gICAgbW92ZUluZGV4KG5vZGUsIHRoaXMuY2hpbGRyZW4sIHRoaXMuY2hpbGRyZW4ubGVuZ3RoLCB0cnVlKVxuICAgIGlmIChub2RlLm5vZGVUeXBlID09PSAxKSB7XG4gICAgICBjb25zdCBpbmRleCA9IG1vdmVJbmRleChub2RlLCB0aGlzLnB1cmVDaGlsZHJlbiwgdGhpcy5wdXJlQ2hpbGRyZW4ubGVuZ3RoKVxuICAgICAgaWYgKHRoaXMuZG9jSWQgJiYgaW5kZXggPj0gMCkge1xuICAgICAgICBjb25zdCBsaXN0ZW5lciA9IGluc3RhbmNlTWFwW3RoaXMuZG9jSWRdLmxpc3RlbmVyXG4gICAgICAgIHJldHVybiBsaXN0ZW5lci5tb3ZlRWxlbWVudChub2RlLnJlZiwgdGhpcy5yZWYsIGluZGV4KVxuICAgICAgfVxuICAgIH1cbiAgfVxufVxuXG5FbGVtZW50LnByb3RvdHlwZS5pbnNlcnRCZWZvcmUgPSBmdW5jdGlvbiAobm9kZSwgYmVmb3JlKSB7XG4gIGlmIChub2RlLnBhcmVudE5vZGUgJiYgbm9kZS5wYXJlbnROb2RlICE9PSB0aGlzKSB7XG4gICAgcmV0dXJuXG4gIH1cbiAgaWYgKG5vZGUgPT09IGJlZm9yZSB8fCBub2RlLm5leHRTaWJsaW5nID09PSBiZWZvcmUpIHtcbiAgICByZXR1cm5cbiAgfVxuICBpZiAoIW5vZGUucGFyZW50Tm9kZSkge1xuICAgIGxpbmtQYXJlbnQobm9kZSwgdGhpcylcbiAgICBpbnNlcnRJbmRleChub2RlLCB0aGlzLmNoaWxkcmVuLCB0aGlzLmNoaWxkcmVuLmluZGV4T2YoYmVmb3JlKSwgdHJ1ZSlcbiAgICBpZiAodGhpcy5kb2NJZCkge1xuICAgICAgcmVnaXN0ZXJOb2RlKHRoaXMuZG9jSWQsIG5vZGUpXG4gICAgfVxuICAgIGlmIChub2RlLm5vZGVUeXBlID09PSAxKSB7XG4gICAgICBjb25zdCBwdXJlQmVmb3JlID0gbmV4dEVsZW1lbnQoYmVmb3JlKVxuICAgICAgY29uc3QgaW5kZXggPSBpbnNlcnRJbmRleChcbiAgICAgICAgbm9kZSxcbiAgICAgICAgdGhpcy5wdXJlQ2hpbGRyZW4sXG4gICAgICAgIHB1cmVCZWZvcmVcbiAgICAgICAgICA/IHRoaXMucHVyZUNoaWxkcmVuLmluZGV4T2YocHVyZUJlZm9yZSlcbiAgICAgICAgICA6IHRoaXMucHVyZUNoaWxkcmVuLmxlbmd0aFxuICAgICAgKVxuICAgICAgaWYgKHRoaXMuZG9jSWQpIHtcbiAgICAgICAgY29uc3QgbGlzdGVuZXIgPSBpbnN0YW5jZU1hcFt0aGlzLmRvY0lkXS5saXN0ZW5lclxuICAgICAgICByZXR1cm4gbGlzdGVuZXIuYWRkRWxlbWVudChub2RlLCB0aGlzLnJlZiwgaW5kZXgpXG4gICAgICB9XG4gICAgfVxuICB9XG4gIGVsc2Uge1xuICAgIG1vdmVJbmRleChub2RlLCB0aGlzLmNoaWxkcmVuLCB0aGlzLmNoaWxkcmVuLmluZGV4T2YoYmVmb3JlKSwgdHJ1ZSlcbiAgICBpZiAobm9kZS5ub2RlVHlwZSA9PT0gMSkge1xuICAgICAgY29uc3QgcHVyZUJlZm9yZSA9IG5leHRFbGVtZW50KGJlZm9yZSlcbiAgICAgIGNvbnN0IGluZGV4ID0gbW92ZUluZGV4KFxuICAgICAgICBub2RlLFxuICAgICAgICB0aGlzLnB1cmVDaGlsZHJlbixcbiAgICAgICAgcHVyZUJlZm9yZVxuICAgICAgICAgID8gdGhpcy5wdXJlQ2hpbGRyZW4uaW5kZXhPZihwdXJlQmVmb3JlKVxuICAgICAgICAgIDogdGhpcy5wdXJlQ2hpbGRyZW4ubGVuZ3RoXG4gICAgICApXG4gICAgICBpZiAodGhpcy5kb2NJZCAmJiBpbmRleCA+PSAwKSB7XG4gICAgICAgIGNvbnN0IGxpc3RlbmVyID0gaW5zdGFuY2VNYXBbdGhpcy5kb2NJZF0ubGlzdGVuZXJcbiAgICAgICAgcmV0dXJuIGxpc3RlbmVyLm1vdmVFbGVtZW50KG5vZGUucmVmLCB0aGlzLnJlZiwgaW5kZXgpXG4gICAgICB9XG4gICAgfVxuICB9XG59XG5cbkVsZW1lbnQucHJvdG90eXBlLmluc2VydEFmdGVyID0gZnVuY3Rpb24gKG5vZGUsIGFmdGVyKSB7XG4gIGlmIChub2RlLnBhcmVudE5vZGUgJiYgbm9kZS5wYXJlbnROb2RlICE9PSB0aGlzKSB7XG4gICAgcmV0dXJuXG4gIH1cbiAgaWYgKG5vZGUgPT09IGFmdGVyIHx8IG5vZGUucHJldmlvdXNTaWJsaW5nID09PSBhZnRlcikge1xuICAgIHJldHVyblxuICB9XG4gIGlmICghbm9kZS5wYXJlbnROb2RlKSB7XG4gICAgbGlua1BhcmVudChub2RlLCB0aGlzKVxuICAgIGluc2VydEluZGV4KG5vZGUsIHRoaXMuY2hpbGRyZW4sIHRoaXMuY2hpbGRyZW4uaW5kZXhPZihhZnRlcikgKyAxLCB0cnVlKVxuICAgIGlmICh0aGlzLmRvY0lkKSB7XG4gICAgICByZWdpc3Rlck5vZGUodGhpcy5kb2NJZCwgbm9kZSlcbiAgICB9XG4gICAgaWYgKG5vZGUubm9kZVR5cGUgPT09IDEpIHtcbiAgICAgIGNvbnN0IGluZGV4ID0gaW5zZXJ0SW5kZXgoXG4gICAgICAgIG5vZGUsXG4gICAgICAgIHRoaXMucHVyZUNoaWxkcmVuLFxuICAgICAgICB0aGlzLnB1cmVDaGlsZHJlbi5pbmRleE9mKHByZXZpb3VzRWxlbWVudChhZnRlcikpICsgMVxuICAgICAgKVxuICAgICAgaWYgKHRoaXMuZG9jSWQpIHtcbiAgICAgICAgY29uc3QgbGlzdGVuZXIgPSBpbnN0YW5jZU1hcFt0aGlzLmRvY0lkXS5saXN0ZW5lclxuICAgICAgICByZXR1cm4gbGlzdGVuZXIuYWRkRWxlbWVudChub2RlLCB0aGlzLnJlZiwgaW5kZXgpXG4gICAgICB9XG4gICAgfVxuICB9XG4gIGVsc2Uge1xuICAgIG1vdmVJbmRleChub2RlLCB0aGlzLmNoaWxkcmVuLCB0aGlzLmNoaWxkcmVuLmluZGV4T2YoYWZ0ZXIpICsgMSwgdHJ1ZSlcbiAgICBpZiAobm9kZS5ub2RlVHlwZSA9PT0gMSkge1xuICAgICAgY29uc3QgaW5kZXggPSBtb3ZlSW5kZXgoXG4gICAgICAgIG5vZGUsXG4gICAgICAgIHRoaXMucHVyZUNoaWxkcmVuLFxuICAgICAgICB0aGlzLnB1cmVDaGlsZHJlbi5pbmRleE9mKHByZXZpb3VzRWxlbWVudChhZnRlcikpICsgMVxuICAgICAgKVxuICAgICAgaWYgKHRoaXMuZG9jSWQgJiYgaW5kZXggPj0gMCkge1xuICAgICAgICBjb25zdCBsaXN0ZW5lciA9IGluc3RhbmNlTWFwW3RoaXMuZG9jSWRdLmxpc3RlbmVyXG4gICAgICAgIHJldHVybiBsaXN0ZW5lci5tb3ZlRWxlbWVudChub2RlLnJlZiwgdGhpcy5yZWYsIGluZGV4KVxuICAgICAgfVxuICAgIH1cbiAgfVxufVxuXG5FbGVtZW50LnByb3RvdHlwZS5yZW1vdmVDaGlsZCA9IGZ1bmN0aW9uIChub2RlLCBwcmVzZXJ2ZWQpIHtcbiAgaWYgKG5vZGUucGFyZW50Tm9kZSkge1xuICAgIHJlbW92ZUluZGV4KG5vZGUsIHRoaXMuY2hpbGRyZW4sIHRydWUpXG4gICAgaWYgKG5vZGUubm9kZVR5cGUgPT09IDEpIHtcbiAgICAgIHJlbW92ZUluZGV4KG5vZGUsIHRoaXMucHVyZUNoaWxkcmVuKVxuICAgICAgaWYgKHRoaXMuZG9jSWQpIHtcbiAgICAgICAgY29uc3QgbGlzdGVuZXIgPSBpbnN0YW5jZU1hcFt0aGlzLmRvY0lkXS5saXN0ZW5lclxuICAgICAgICBsaXN0ZW5lci5yZW1vdmVFbGVtZW50KG5vZGUucmVmKVxuICAgICAgfVxuICAgIH1cbiAgfVxuICBpZiAoIXByZXNlcnZlZCkge1xuICAgIG5vZGUuZGVzdHJveSgpXG4gIH1cbn1cblxuRWxlbWVudC5wcm90b3R5cGUuY2xlYXIgPSBmdW5jdGlvbiAoKSB7XG4gIGlmICh0aGlzLmRvY0lkKSB7XG4gICAgY29uc3QgbGlzdGVuZXIgPSBpbnN0YW5jZU1hcFt0aGlzLmRvY0lkXS5saXN0ZW5lclxuICAgIHRoaXMucHVyZUNoaWxkcmVuLmZvckVhY2gobm9kZSA9PiB7XG4gICAgICBsaXN0ZW5lci5yZW1vdmVFbGVtZW50KG5vZGUucmVmKVxuICAgIH0pXG4gIH1cbiAgdGhpcy5jaGlsZHJlbi5mb3JFYWNoKG5vZGUgPT4ge1xuICAgIG5vZGUuZGVzdHJveSgpXG4gIH0pXG4gIHRoaXMuY2hpbGRyZW4ubGVuZ3RoID0gMFxuICB0aGlzLnB1cmVDaGlsZHJlbi5sZW5ndGggPSAwXG59XG5cbmZ1bmN0aW9uIG5leHRFbGVtZW50IChub2RlKSB7XG4gIHdoaWxlIChub2RlKSB7XG4gICAgaWYgKG5vZGUubm9kZVR5cGUgPT09IDEpIHtcbiAgICAgIHJldHVybiBub2RlXG4gICAgfVxuICAgIG5vZGUgPSBub2RlLm5leHRTaWJsaW5nXG4gIH1cbn1cblxuZnVuY3Rpb24gcHJldmlvdXNFbGVtZW50IChub2RlKSB7XG4gIHdoaWxlIChub2RlKSB7XG4gICAgaWYgKG5vZGUubm9kZVR5cGUgPT09IDEpIHtcbiAgICAgIHJldHVybiBub2RlXG4gICAgfVxuICAgIG5vZGUgPSBub2RlLnByZXZpb3VzU2libGluZ1xuICB9XG59XG5cbmZ1bmN0aW9uIGxpbmtQYXJlbnQgKG5vZGUsIHBhcmVudCkge1xuICBub2RlLnBhcmVudE5vZGUgPSBwYXJlbnRcbiAgaWYgKHBhcmVudC5kb2NJZCkge1xuICAgIG5vZGUuZG9jSWQgPSBwYXJlbnQuZG9jSWRcbiAgICBub2RlLm93bmVyRG9jdW1lbnQgPSBwYXJlbnQub3duZXJEb2N1bWVudFxuICAgIG5vZGUub3duZXJEb2N1bWVudC5ub2RlTWFwW25vZGUubm9kZUlkXSA9IG5vZGVcbiAgICBub2RlLmRlcHRoID0gcGFyZW50LmRlcHRoICsgMVxuICB9XG4gIG5vZGUuY2hpbGRyZW4uZm9yRWFjaChjaGlsZCA9PiB7XG4gICAgbGlua1BhcmVudChjaGlsZCwgbm9kZSlcbiAgfSlcbn1cblxuZnVuY3Rpb24gcmVnaXN0ZXJOb2RlIChkb2NJZCwgbm9kZSkge1xuICBjb25zdCBkb2MgPSBpbnN0YW5jZU1hcFtkb2NJZF1cbiAgZG9jLm5vZGVNYXBbbm9kZS5ub2RlSWRdID0gbm9kZVxufVxuXG5mdW5jdGlvbiBpbnNlcnRJbmRleCAodGFyZ2V0LCBsaXN0LCBuZXdJbmRleCwgY2hhbmdlU2libGluZykge1xuICBpZiAobmV3SW5kZXggPCAwKSB7XG4gICAgbmV3SW5kZXggPSAwXG4gIH1cbiAgY29uc3QgYmVmb3JlID0gbGlzdFtuZXdJbmRleCAtIDFdXG4gIGNvbnN0IGFmdGVyID0gbGlzdFtuZXdJbmRleF1cbiAgbGlzdC5zcGxpY2UobmV3SW5kZXgsIDAsIHRhcmdldClcbiAgaWYgKGNoYW5nZVNpYmxpbmcpIHtcbiAgICBiZWZvcmUgJiYgKGJlZm9yZS5uZXh0U2libGluZyA9IHRhcmdldClcbiAgICB0YXJnZXQucHJldmlvdXNTaWJsaW5nID0gYmVmb3JlXG4gICAgdGFyZ2V0Lm5leHRTaWJsaW5nID0gYWZ0ZXJcbiAgICBhZnRlciAmJiAoYWZ0ZXIucHJldmlvdXNTaWJsaW5nID0gdGFyZ2V0KVxuICB9XG4gIHJldHVybiBuZXdJbmRleFxufVxuXG5mdW5jdGlvbiBtb3ZlSW5kZXggKHRhcmdldCwgbGlzdCwgbmV3SW5kZXgsIGNoYW5nZVNpYmxpbmcpIHtcbiAgY29uc3QgaW5kZXggPSBsaXN0LmluZGV4T2YodGFyZ2V0KVxuICBpZiAoaW5kZXggPCAwKSB7XG4gICAgcmV0dXJuIC0xXG4gIH1cbiAgaWYgKGNoYW5nZVNpYmxpbmcpIHtcbiAgICBjb25zdCBiZWZvcmUgPSBsaXN0W2luZGV4IC0gMV1cbiAgICBjb25zdCBhZnRlciA9IGxpc3RbaW5kZXggKyAxXVxuICAgIGJlZm9yZSAmJiAoYmVmb3JlLm5leHRTaWJsaW5nID0gYWZ0ZXIpXG4gICAgYWZ0ZXIgJiYgKGFmdGVyLnByZXZpb3VzU2libGluZyA9IGJlZm9yZSlcbiAgfVxuICBsaXN0LnNwbGljZShpbmRleCwgMSlcbiAgbGV0IG5ld0luZGV4QWZ0ZXIgPSBuZXdJbmRleFxuICBpZiAoaW5kZXggPD0gbmV3SW5kZXgpIHtcbiAgICBuZXdJbmRleEFmdGVyID0gbmV3SW5kZXggLSAxXG4gIH1cbiAgY29uc3QgYmVmb3JlTmV3ID0gbGlzdFtuZXdJbmRleEFmdGVyIC0gMV1cbiAgY29uc3QgYWZ0ZXJOZXcgPSBsaXN0W25ld0luZGV4QWZ0ZXJdXG4gIGxpc3Quc3BsaWNlKG5ld0luZGV4QWZ0ZXIsIDAsIHRhcmdldClcbiAgaWYgKGNoYW5nZVNpYmxpbmcpIHtcbiAgICBiZWZvcmVOZXcgJiYgKGJlZm9yZU5ldy5uZXh0U2libGluZyA9IHRhcmdldClcbiAgICB0YXJnZXQucHJldmlvdXNTaWJsaW5nID0gYmVmb3JlTmV3XG4gICAgdGFyZ2V0Lm5leHRTaWJsaW5nID0gYWZ0ZXJOZXdcbiAgICBhZnRlck5ldyAmJiAoYWZ0ZXJOZXcucHJldmlvdXNTaWJsaW5nID0gdGFyZ2V0KVxuICB9XG4gIGlmIChpbmRleCA9PT0gbmV3SW5kZXhBZnRlcikge1xuICAgIHJldHVybiAtMVxuICB9XG4gIHJldHVybiBuZXdJbmRleFxufVxuXG5mdW5jdGlvbiByZW1vdmVJbmRleCAodGFyZ2V0LCBsaXN0LCBjaGFuZ2VTaWJsaW5nKSB7XG4gIGNvbnN0IGluZGV4ID0gbGlzdC5pbmRleE9mKHRhcmdldClcbiAgaWYgKGluZGV4IDwgMCkge1xuICAgIHJldHVyblxuICB9XG4gIGlmIChjaGFuZ2VTaWJsaW5nKSB7XG4gICAgY29uc3QgYmVmb3JlID0gbGlzdFtpbmRleCAtIDFdXG4gICAgY29uc3QgYWZ0ZXIgPSBsaXN0W2luZGV4ICsgMV1cbiAgICBiZWZvcmUgJiYgKGJlZm9yZS5uZXh0U2libGluZyA9IGFmdGVyKVxuICAgIGFmdGVyICYmIChhZnRlci5wcmV2aW91c1NpYmxpbmcgPSBiZWZvcmUpXG4gIH1cbiAgbGlzdC5zcGxpY2UoaW5kZXgsIDEpXG59XG5cbkVsZW1lbnQucHJvdG90eXBlLnNldEF0dHIgPSBmdW5jdGlvbiAoa2V5LCB2YWx1ZSwgc2lsZW50KSB7XG4gIGlmICh0aGlzLmF0dHJba2V5XSA9PT0gdmFsdWUpIHtcbiAgICByZXR1cm5cbiAgfVxuICB0aGlzLmF0dHJba2V5XSA9IHZhbHVlXG4gIGlmICghc2lsZW50ICYmIHRoaXMuZG9jSWQpIHtcbiAgICBjb25zdCBsaXN0ZW5lciA9IGluc3RhbmNlTWFwW3RoaXMuZG9jSWRdLmxpc3RlbmVyXG4gICAgbGlzdGVuZXIuc2V0QXR0cih0aGlzLnJlZiwga2V5LCB2YWx1ZSlcbiAgfVxufVxuXG5FbGVtZW50LnByb3RvdHlwZS5zZXRTdHlsZSA9IGZ1bmN0aW9uIChrZXksIHZhbHVlLCBzaWxlbnQpIHtcbiAgaWYgKHRoaXMuc3R5bGVba2V5XSA9PT0gdmFsdWUpIHtcbiAgICByZXR1cm5cbiAgfVxuICB0aGlzLnN0eWxlW2tleV0gPSB2YWx1ZVxuICBpZiAoIXNpbGVudCAmJiB0aGlzLmRvY0lkKSB7XG4gICAgY29uc3QgbGlzdGVuZXIgPSBpbnN0YW5jZU1hcFt0aGlzLmRvY0lkXS5saXN0ZW5lclxuICAgIGxpc3RlbmVyLnNldFN0eWxlKHRoaXMucmVmLCBrZXksIHZhbHVlKVxuICB9XG59XG5cbkVsZW1lbnQucHJvdG90eXBlLnNldENsYXNzU3R5bGUgPSBmdW5jdGlvbiAoY2xhc3NTdHlsZSkge1xuICB0aGlzLmNsYXNzU3R5bGUgPSBjbGFzc1N0eWxlXG4gIGlmICh0aGlzLmRvY0lkKSB7XG4gICAgY29uc3QgbGlzdGVuZXIgPSBpbnN0YW5jZU1hcFt0aGlzLmRvY0lkXS5saXN0ZW5lclxuICAgIGxpc3RlbmVyLnNldFN0eWxlcyh0aGlzLnJlZiwgdGhpcy50b1N0eWxlKCkpXG4gIH1cbn1cblxuRWxlbWVudC5wcm90b3R5cGUuYWRkRXZlbnQgPSBmdW5jdGlvbiAodHlwZSwgaGFuZGxlcikge1xuICBpZiAoIXRoaXMuZXZlbnRbdHlwZV0pIHtcbiAgICB0aGlzLmV2ZW50W3R5cGVdID0gaGFuZGxlclxuICAgIGlmICh0aGlzLmRvY0lkKSB7XG4gICAgICBjb25zdCBsaXN0ZW5lciA9IGluc3RhbmNlTWFwW3RoaXMuZG9jSWRdLmxpc3RlbmVyXG4gICAgICBsaXN0ZW5lci5hZGRFdmVudCh0aGlzLnJlZiwgdHlwZSlcbiAgICB9XG4gIH1cbn1cblxuRWxlbWVudC5wcm90b3R5cGUucmVtb3ZlRXZlbnQgPSBmdW5jdGlvbiAodHlwZSkge1xuICBpZiAodGhpcy5ldmVudFt0eXBlXSkge1xuICAgIGRlbGV0ZSB0aGlzLmV2ZW50W3R5cGVdXG4gICAgaWYgKHRoaXMuZG9jSWQpIHtcbiAgICAgIGNvbnN0IGxpc3RlbmVyID0gaW5zdGFuY2VNYXBbdGhpcy5kb2NJZF0ubGlzdGVuZXJcbiAgICAgIGxpc3RlbmVyLnJlbW92ZUV2ZW50KHRoaXMucmVmLCB0eXBlKVxuICAgIH1cbiAgfVxufVxuXG5FbGVtZW50LnByb3RvdHlwZS5maXJlRXZlbnQgPSBmdW5jdGlvbiAodHlwZSwgZSkge1xuICBjb25zdCBoYW5kbGVyID0gdGhpcy5ldmVudFt0eXBlXVxuICBpZiAoaGFuZGxlcikge1xuICAgIHJldHVybiBoYW5kbGVyLmNhbGwodGhpcywgZSlcbiAgfVxufVxuXG5FbGVtZW50LnByb3RvdHlwZS50b1N0eWxlID0gZnVuY3Rpb24gKCkge1xuICByZXR1cm4gZXh0ZW5kKHt9LCB0aGlzLmNsYXNzU3R5bGUsIHRoaXMuc3R5bGUpXG59XG5cbkVsZW1lbnQucHJvdG90eXBlLnRvSlNPTiA9IGZ1bmN0aW9uICgpIHtcbiAgY29uc3QgcmVzdWx0ID0ge1xuICAgIHJlZjogdGhpcy5yZWYudG9TdHJpbmcoKSxcbiAgICB0eXBlOiB0aGlzLnR5cGUsXG4gICAgYXR0cjogdGhpcy5hdHRyLFxuICAgIHN0eWxlOiB0aGlzLnRvU3R5bGUoKVxuICB9XG4gIGNvbnN0IGV2ZW50ID0gT2JqZWN0LmtleXModGhpcy5ldmVudClcbiAgaWYgKGV2ZW50Lmxlbmd0aCkge1xuICAgIHJlc3VsdC5ldmVudCA9IGV2ZW50XG4gIH1cbiAgaWYgKHRoaXMucHVyZUNoaWxkcmVuLmxlbmd0aCkge1xuICAgIHJlc3VsdC5jaGlsZHJlbiA9IHRoaXMucHVyZUNoaWxkcmVuLm1hcCgoY2hpbGQpID0+IGNoaWxkLnRvSlNPTigpKVxuICB9XG4gIHJldHVybiByZXN1bHRcbn1cblxuRWxlbWVudC5wcm90b3R5cGUudG9TdHJpbmcgPSBmdW5jdGlvbiAoKSB7XG4gIHJldHVybiAnPCcgKyB0aGlzLnR5cGUgK1xuICAgICcgYXR0cj0nICsgSlNPTi5zdHJpbmdpZnkodGhpcy5hdHRyKSArXG4gICAgJyBzdHlsZT0nICsgSlNPTi5zdHJpbmdpZnkodGhpcy50b1N0eWxlKCkpICsgJz4nICtcbiAgICB0aGlzLnB1cmVDaGlsZHJlbi5tYXAoKGNoaWxkKSA9PiBjaGlsZC50b1N0cmluZygpKS5qb2luKCcnKSArXG4gICAgJzwvJyArIHRoaXMudHlwZSArICc+J1xufVxuXG5leHBvcnQgZnVuY3Rpb24gQ29tbWVudCAodmFsdWUpIHtcbiAgdGhpcy5ub2RlVHlwZSA9IDhcbiAgdGhpcy5ub2RlSWQgPSAobmV4dE5vZGVSZWYrKykudG9TdHJpbmcoKVxuICB0aGlzLnJlZiA9IHRoaXMubm9kZUlkXG4gIHRoaXMudHlwZSA9ICdjb21tZW50J1xuICB0aGlzLnZhbHVlID0gdmFsdWVcbiAgdGhpcy5jaGlsZHJlbiA9IFtdXG4gIHRoaXMucHVyZUNoaWxkcmVuID0gW11cbn1cblxuQ29tbWVudC5wcm90b3R5cGUgPSBuZXcgTm9kZSgpXG5cbkNvbW1lbnQucHJvdG90eXBlLnRvU3RyaW5nID0gZnVuY3Rpb24gKCkge1xuICByZXR1cm4gJzwhLS0gJyArIHRoaXMudmFsdWUgKyAnIC0tPidcbn1cblxuXG5cbi8qKiBXRUJQQUNLIEZPT1RFUiAqKlxuICoqIC9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9zcmMvdmRvbS9pbmRleC5qc1xuICoqLyIsImV4cG9ydCBkZWZhdWx0IGZ1bmN0aW9uIExpc3RlbmVyIChpZCwgaGFuZGxlcikge1xuICB0aGlzLmlkID0gaWRcbiAgdGhpcy5iYXRjaGVkID0gZmFsc2VcbiAgdGhpcy51cGRhdGVzID0gW11cbiAgaWYgKHR5cGVvZiBoYW5kbGVyID09PSAnZnVuY3Rpb24nKSB7XG4gICAgdGhpcy5oYW5kbGVyID0gaGFuZGxlclxuICB9XG59XG5cbkxpc3RlbmVyLnByb3RvdHlwZS5jcmVhdGVGaW5pc2ggPSBmdW5jdGlvbiAoY2FsbGJhY2spIHtcbiAgY29uc3QgaGFuZGxlciA9IHRoaXMuaGFuZGxlclxuICByZXR1cm4gaGFuZGxlcihbY3JlYXRlQWN0aW9uKCdjcmVhdGVGaW5pc2gnLCBbXSldLCBjYWxsYmFjaylcbn1cblxuTGlzdGVuZXIucHJvdG90eXBlLnVwZGF0ZUZpbmlzaCA9IGZ1bmN0aW9uIChjYWxsYmFjaykge1xuICBjb25zdCBoYW5kbGVyID0gdGhpcy5oYW5kbGVyXG4gIHJldHVybiBoYW5kbGVyKFtjcmVhdGVBY3Rpb24oJ3VwZGF0ZUZpbmlzaCcsIFtdKV0sIGNhbGxiYWNrKVxufVxuXG5MaXN0ZW5lci5wcm90b3R5cGUucmVmcmVzaEZpbmlzaCA9IGZ1bmN0aW9uIChjYWxsYmFjaykge1xuICBjb25zdCBoYW5kbGVyID0gdGhpcy5oYW5kbGVyXG4gIHJldHVybiBoYW5kbGVyKFtjcmVhdGVBY3Rpb24oJ3JlZnJlc2hGaW5pc2gnLCBbXSldLCBjYWxsYmFjaylcbn1cblxuTGlzdGVuZXIucHJvdG90eXBlLmNyZWF0ZUJvZHkgPSBmdW5jdGlvbiAoZWxlbWVudCkge1xuICBjb25zdCBib2R5ID0gZWxlbWVudC50b0pTT04oKVxuICBjb25zdCBjaGlsZHJlbiA9IGJvZHkuY2hpbGRyZW5cbiAgZGVsZXRlIGJvZHkuY2hpbGRyZW5cbiAgY29uc3QgYWN0aW9ucyA9IFtjcmVhdGVBY3Rpb24oJ2NyZWF0ZUJvZHknLCBbYm9keV0pXVxuICBpZiAoY2hpbGRyZW4pIHtcbiAgICBhY3Rpb25zLnB1c2guYXBwbHkoYWN0aW9ucywgY2hpbGRyZW4ubWFwKGNoaWxkID0+IHtcbiAgICAgIHJldHVybiBjcmVhdGVBY3Rpb24oJ2FkZEVsZW1lbnQnLCBbYm9keS5yZWYsIGNoaWxkLCAtMV0pXG4gICAgfSkpXG4gIH1cbiAgcmV0dXJuIHRoaXMuYWRkQWN0aW9ucyhhY3Rpb25zKVxufVxuXG5MaXN0ZW5lci5wcm90b3R5cGUuYWRkRWxlbWVudCA9IGZ1bmN0aW9uIChlbGVtZW50LCByZWYsIGluZGV4KSB7XG4gIGlmICghKGluZGV4ID49IDApKSB7XG4gICAgaW5kZXggPSAtMVxuICB9XG4gIHJldHVybiB0aGlzLmFkZEFjdGlvbnMoY3JlYXRlQWN0aW9uKCdhZGRFbGVtZW50JywgW3JlZiwgZWxlbWVudC50b0pTT04oKSwgaW5kZXhdKSlcbn1cblxuTGlzdGVuZXIucHJvdG90eXBlLnJlbW92ZUVsZW1lbnQgPSBmdW5jdGlvbiAocmVmKSB7XG4gIGlmIChBcnJheS5pc0FycmF5KHJlZikpIHtcbiAgICBjb25zdCBhY3Rpb25zID0gcmVmLm1hcCgocikgPT4gY3JlYXRlQWN0aW9uKCdyZW1vdmVFbGVtZW50JywgW3JdKSlcbiAgICByZXR1cm4gdGhpcy5hZGRBY3Rpb25zKGFjdGlvbnMpXG4gIH1cbiAgcmV0dXJuIHRoaXMuYWRkQWN0aW9ucyhjcmVhdGVBY3Rpb24oJ3JlbW92ZUVsZW1lbnQnLCBbcmVmXSkpXG59XG5cbkxpc3RlbmVyLnByb3RvdHlwZS5tb3ZlRWxlbWVudCA9IGZ1bmN0aW9uICh0YXJnZXRSZWYsIHBhcmVudFJlZiwgaW5kZXgpIHtcbiAgcmV0dXJuIHRoaXMuYWRkQWN0aW9ucyhjcmVhdGVBY3Rpb24oJ21vdmVFbGVtZW50JywgW3RhcmdldFJlZiwgcGFyZW50UmVmLCBpbmRleF0pKVxufVxuXG5MaXN0ZW5lci5wcm90b3R5cGUuc2V0QXR0ciA9IGZ1bmN0aW9uIChyZWYsIGtleSwgdmFsdWUpIHtcbiAgY29uc3QgcmVzdWx0ID0ge31cbiAgcmVzdWx0W2tleV0gPSB2YWx1ZVxuICByZXR1cm4gdGhpcy5hZGRBY3Rpb25zKGNyZWF0ZUFjdGlvbigndXBkYXRlQXR0cnMnLCBbcmVmLCByZXN1bHRdKSlcbn1cblxuTGlzdGVuZXIucHJvdG90eXBlLnNldFN0eWxlID0gZnVuY3Rpb24gKHJlZiwga2V5LCB2YWx1ZSkge1xuICBjb25zdCByZXN1bHQgPSB7fVxuICByZXN1bHRba2V5XSA9IHZhbHVlXG4gIHJldHVybiB0aGlzLmFkZEFjdGlvbnMoY3JlYXRlQWN0aW9uKCd1cGRhdGVTdHlsZScsIFtyZWYsIHJlc3VsdF0pKVxufVxuXG5MaXN0ZW5lci5wcm90b3R5cGUuc2V0U3R5bGVzID0gZnVuY3Rpb24gKHJlZiwgc3R5bGUpIHtcbiAgcmV0dXJuIHRoaXMuYWRkQWN0aW9ucyhjcmVhdGVBY3Rpb24oJ3VwZGF0ZVN0eWxlJywgW3JlZiwgc3R5bGVdKSlcbn1cblxuTGlzdGVuZXIucHJvdG90eXBlLmFkZEV2ZW50ID0gZnVuY3Rpb24gKHJlZiwgdHlwZSkge1xuICByZXR1cm4gdGhpcy5hZGRBY3Rpb25zKGNyZWF0ZUFjdGlvbignYWRkRXZlbnQnLCBbcmVmLCB0eXBlXSkpXG59XG5cbkxpc3RlbmVyLnByb3RvdHlwZS5yZW1vdmVFdmVudCA9IGZ1bmN0aW9uIChyZWYsIHR5cGUpIHtcbiAgcmV0dXJuIHRoaXMuYWRkQWN0aW9ucyhjcmVhdGVBY3Rpb24oJ3JlbW92ZUV2ZW50JywgW3JlZiwgdHlwZV0pKVxufVxuXG5MaXN0ZW5lci5wcm90b3R5cGUuaGFuZGxlciA9IGZ1bmN0aW9uIChhY3Rpb25zLCBjYikge1xuICByZXR1cm4gY2IgJiYgY2IoKVxufVxuXG5MaXN0ZW5lci5wcm90b3R5cGUuYWRkQWN0aW9ucyA9IGZ1bmN0aW9uIChhY3Rpb25zKSB7XG4gIGNvbnN0IHVwZGF0ZXMgPSB0aGlzLnVwZGF0ZXNcbiAgY29uc3QgaGFuZGxlciA9IHRoaXMuaGFuZGxlclxuXG4gIGlmICghQXJyYXkuaXNBcnJheShhY3Rpb25zKSkge1xuICAgIGFjdGlvbnMgPSBbYWN0aW9uc11cbiAgfVxuXG4gIGlmICh0aGlzLmJhdGNoZWQpIHtcbiAgICB1cGRhdGVzLnB1c2guYXBwbHkodXBkYXRlcywgYWN0aW9ucylcbiAgfVxuICBlbHNlIHtcbiAgICByZXR1cm4gaGFuZGxlcihhY3Rpb25zKVxuICB9XG59XG5cbmV4cG9ydCBmdW5jdGlvbiBjcmVhdGVBY3Rpb24gKG5hbWUsIGFyZ3MpIHtcbiAgcmV0dXJuIHsgbW9kdWxlOiAnZG9tJywgbWV0aG9kOiBuYW1lLCBhcmdzOiBhcmdzIH1cbn1cblxuXG5cbi8qKiBXRUJQQUNLIEZPT1RFUiAqKlxuICoqIC9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9zcmMvdmRvbS9saXN0ZW5lci5qc1xuICoqLyIsIm1vZHVsZS5leHBvcnRzID0ge1xuXHRcIm5hbWVcIjogXCJ3ZWV4LWpzLWZyYW1ld29ya1wiLFxuXHRcInZlcnNpb25cIjogXCIwLjE1LjJcIixcblx0XCJzdWJ2ZXJzaW9uXCI6IHtcblx0XHRcImZyYW1ld29ya1wiOiBcIjAuMTUuMlwiLFxuXHRcdFwidHJhbnNmb3JtZXJcIjogXCI+PTAuMS41IDwwLjRcIlxuXHR9LFxuXHRcImRlc2NyaXB0aW9uXCI6IFwiV2VleCBKUyBGcmFtZXdvcmtcIixcblx0XCJrZXl3b3Jkc1wiOiBbXG5cdFx0XCJ3ZWV4XCIsXG5cdFx0XCJtdnZtXCIsXG5cdFx0XCJqYXZhc2NyaXB0XCIsXG5cdFx0XCJodG1sNVwiXG5cdF0sXG5cdFwiaG9tZXBhZ2VcIjogXCJodHRwczovL2FsaWJhYmEuZ2l0aHViLmlvL3dlZXhcIixcblx0XCJidWdzXCI6IHtcblx0XHRcInVybFwiOiBcImh0dHBzOi8vZ2l0aHViLmNvbS9hbGliYWJhL3dlZXgvaXNzdWVzXCJcblx0fSxcblx0XCJsaWNlbnNlXCI6IFwiQXBhY2hlLTIuMFwiLFxuXHRcImF1dGhvclwiOiBcIkppbmppYW5nIDx6aGFvamluamlhbmdAbWUuY29tPlwiLFxuXHRcIm1haW50YWluZXJzXCI6IFtcblx0XHRcInRlcnJ5a2luZ2NoYSA8dGVycnlraW5nY2hhQGdtYWlsLmNvbT5cIixcblx0XHRcIklza2VuSHVhbmcgPGlza2VuaHVhbmdAZ21haWwuY29tPlwiLFxuXHRcdFwieXVhbnlhbiA8eXVhbnlhbi5jYW9AZ21haWwuY29tPlwiXG5cdF0sXG5cdFwibWFpblwiOiBcImluZGV4LmpzXCIsXG5cdFwicmVwb3NpdG9yeVwiOiB7XG5cdFx0XCJ0eXBlXCI6IFwiZ2l0XCIsXG5cdFx0XCJ1cmxcIjogXCJnaXRAZ2l0aHViLmNvbTphbGliYWJhL3dlZXguZ2l0XCJcblx0fSxcblx0XCJzY3JpcHRzXCI6IHtcblx0XHRcInRlc3RcIjogXCJlY2hvIFxcXCJFcnJvcjogbm8gdGVzdCBzcGVjaWZpZWRcXFwiICYmIGV4aXQgMVwiXG5cdH0sXG5cdFwiZGVwZW5kZW5jaWVzXCI6IHtcblx0XHRcImNvcmUtanNcIjogXCJeMi40LjBcIixcblx0XHRcInNlbXZlclwiOiBcIl41LjEuMFwiXG5cdH1cbn07XG5cblxuLyoqKioqKioqKioqKioqKioqXG4gKiogV0VCUEFDSyBGT09URVJcbiAqKiAvVXNlcnMvemhhb2ppbmppYW5nL1NpdGVzL3B1YmxpYy93ZWV4L2Rpc3Qvd2VleC1qcy1mcmFtZXdvcmsvcGFja2FnZS5qc29uXG4gKiogbW9kdWxlIGlkID0gMTAxXG4gKiogbW9kdWxlIGNodW5rcyA9IDBcbiAqKi8iLCIvKipcbiAqIEBmaWxlT3ZlcnZpZXcgVGhlIGFwaSBmb3IgaW52b2tpbmcgd2l0aCBcIiRcIiBwcmVmaXhcbiAqL1xuaW1wb3J0IHsgZXh0ZW5kLCB0eXBvZiB9IGZyb20gJy4uL3V0aWwnXG5cbi8qKlxuICogPT09PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09PVxuICogY29tbW9uXG4gKiA9PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09XG4gKi9cblxuLyoqXG4gKiBAZGVwcmVjYXRlZCB1c2UgJHZtIGluc3RlYWRcbiAqIGZpbmQgdGhlIHZtIGJ5IGlkXG4gKiBOb3RlOiB0aGVyZSBpcyBvbmx5IG9uZSBpZCBpbiB3aG9sZSBjb21wb25lbnRcbiAqIEBwYXJhbSAge3N0cmluZ30gaWRcbiAqIEByZXR1cm4ge1ZtfVxuICovXG5leHBvcnQgZnVuY3Rpb24gJCAoaWQpIHtcbiAgY29uc29sZS53YXJuKCdbSlMgRnJhbWV3b3JrXSBWbSMkIGlzIGRlcHJlY2F0ZWQsIHBsZWFzZSB1c2UgVm0jJHZtIGluc3RlYWQnKVxuICBjb25zdCBpbmZvID0gdGhpcy5faWRzW2lkXVxuICBpZiAoaW5mbykge1xuICAgIHJldHVybiBpbmZvLnZtXG4gIH1cbn1cblxuLyoqXG4gKiBmaW5kIHRoZSBlbGVtZW50IGJ5IGlkXG4gKiBOb3RlOiB0aGVyZSBpcyBvbmx5IG9uZSBpZCBpbiB3aG9sZSBjb21wb25lbnRcbiAqIEBwYXJhbSAge3N0cmluZ30gaWRcbiAqIEByZXR1cm4ge0VsZW1lbnR9XG4gKi9cbmV4cG9ydCBmdW5jdGlvbiAkZWwgKGlkKSB7XG4gIGNvbnN0IGluZm8gPSB0aGlzLl9pZHNbaWRdXG4gIGlmIChpbmZvKSB7XG4gICAgcmV0dXJuIGluZm8uZWxcbiAgfVxufVxuXG4vKipcbiAqIGZpbmQgdGhlIHZtIG9mIHRoZSBjdXN0b20gY29tcG9uZW50IGJ5IGlkXG4gKiBOb3RlOiB0aGVyZSBpcyBvbmx5IG9uZSBpZCBpbiB3aG9sZSBjb21wb25lbnRcbiAqIEBwYXJhbSAge3N0cmluZ30gaWRcbiAqIEByZXR1cm4ge1ZtfVxuICovXG5leHBvcnQgZnVuY3Rpb24gJHZtIChpZCkge1xuICBjb25zdCBpbmZvID0gdGhpcy5faWRzW2lkXVxuICBpZiAoaW5mbykge1xuICAgIHJldHVybiBpbmZvLnZtXG4gIH1cbn1cblxuLyoqXG4gKiBGaXJlIHdoZW4gZGlmZmVyIHJlbmRlcmluZyBmaW5pc2hlZFxuICpcbiAqIEBwYXJhbSAge0Z1bmN0aW9ufSBmblxuICovXG5leHBvcnQgZnVuY3Rpb24gJHJlbmRlclRoZW4gKGZuKSB7XG4gIGNvbnN0IGFwcCA9IHRoaXMuX2FwcFxuICBjb25zdCBkaWZmZXIgPSBhcHAuZGlmZmVyXG4gIHJldHVybiBkaWZmZXIudGhlbigoKSA9PiB7XG4gICAgZm4oKVxuICB9KVxufVxuXG4vKipcbiAqIHNjcm9sbCBhbiBlbGVtZW50IHNwZWNpZmllZCBieSBpZCBpbnRvIHZpZXcsXG4gKiBtb3Jlb3ZlciBzcGVjaWZ5IGEgbnVtYmVyIG9mIG9mZnNldCBvcHRpb25hbGx5XG4gKiBAcGFyYW0gIHtzdHJpbmd9IGlkXG4gKiBAcGFyYW0gIHtudW1iZXJ9IG9mZnNldFxuICovXG5leHBvcnQgZnVuY3Rpb24gJHNjcm9sbFRvIChpZCwgb2Zmc2V0KSB7XG4gIGNvbnNvbGUud2FybignW0pTIEZyYW1ld29ya10gVm0jJHNjcm9sbFRvIGlzIGRlcHJlY2F0ZWQsICcgK1xuICAgICAgICAgICdwbGVhc2UgdXNlIFwicmVxdWlyZShcXCdAd2VleC1tb2R1bGUvZG9tXFwnKScgK1xuICAgICAgICAgICcuc2Nyb2xsVG8oZWwsIG9wdGlvbnMpXCIgaW5zdGVhZCcpXG4gIGNvbnN0IGVsID0gdGhpcy4kZWwoaWQpXG4gIGlmIChlbCkge1xuICAgIGNvbnN0IGRvbSA9IHRoaXMuX2FwcC5yZXF1aXJlTW9kdWxlKCdkb20nKVxuICAgIGRvbS5zY3JvbGxUb0VsZW1lbnQoZWwucmVmLCB7IG9mZnNldDogb2Zmc2V0IH0pXG4gIH1cbn1cblxuLyoqXG4gKiBwZXJmb3JtIHRyYW5zaXRpb24gYW5pbWF0aW9uIG9uIGFuIGVsZW1lbnQgc3BlY2lmaWVkIGJ5IGlkXG4gKiBAcGFyYW0gIHtzdHJpbmd9ICAgaWRcbiAqIEBwYXJhbSAge29iamVjdH0gICBvcHRpb25zXG4gKiBAcGFyYW0gIHtvYmplY3R9ICAgb3B0aW9ucy5zdHlsZXNcbiAqIEBwYXJhbSAge29iamVjdH0gICBvcHRpb25zLmR1cmF0aW9uKG1zKVxuICogQHBhcmFtICB7b2JqZWN0fSAgIFtvcHRpb25zLnRpbWluZ0Z1bmN0aW9uXVxuICogQHBhcmFtICB7b2JqZWN0fSAgIFtvcHRpb25zLmRlbGF5PTAobXMpXVxuICogQHBhcmFtICB7RnVuY3Rpb259IGNhbGxiYWNrXG4gKi9cbmV4cG9ydCBmdW5jdGlvbiAkdHJhbnNpdGlvbiAoaWQsIG9wdGlvbnMsIGNhbGxiYWNrKSB7XG4gIGNvbnN0IGVsID0gdGhpcy4kZWwoaWQpXG4gIGlmIChlbCAmJiBvcHRpb25zICYmIG9wdGlvbnMuc3R5bGVzKSB7XG4gICAgY29uc3QgYW5pbWF0aW9uID0gdGhpcy5fYXBwLnJlcXVpcmVNb2R1bGUoJ2FuaW1hdGlvbicpXG4gICAgYW5pbWF0aW9uLnRyYW5zaXRpb24oZWwucmVmLCBvcHRpb25zLCAoLi4uYXJncykgPT4ge1xuICAgICAgdGhpcy5fc2V0U3R5bGUoZWwsIG9wdGlvbnMuc3R5bGVzKVxuICAgICAgY2FsbGJhY2sgJiYgY2FsbGJhY2soLi4uYXJncylcbiAgICB9KVxuICB9XG59XG5cbi8qKlxuICogZ2V0IHNvbWUgY29uZmlnXG4gKiBAcmV0dXJuIHtvYmplY3R9IHNvbWUgY29uZmlnIGZvciBhcHAgaW5zdGFuY2VcbiAqIEBwcm9wZXJ0eSB7c3RyaW5nfSBidW5kbGVVcmxcbiAqIEBwcm9wZXJ0eSB7Ym9vbGVhbn0gZGVidWdcbiAqIEBwcm9wZXJ0eSB7b2JqZWN0fSBlbnZcbiAqIEBwcm9wZXJ0eSB7c3RyaW5nfSBlbnYud2VleFZlcnNpb24oZXguIDEuMC4wKVxuICogQHByb3BlcnR5IHtzdHJpbmd9IGVudi5hcHBOYW1lKGV4LiBUQi9UTSlcbiAqIEBwcm9wZXJ0eSB7c3RyaW5nfSBlbnYuYXBwVmVyc2lvbihleC4gNS4wLjApXG4gKiBAcHJvcGVydHkge3N0cmluZ30gZW52LnBsYXRmb3JtKGV4LiBpT1MvQW5kcm9pZClcbiAqIEBwcm9wZXJ0eSB7c3RyaW5nfSBlbnYub3NWZXJzaW9uKGV4LiA3LjAuMClcbiAqIEBwcm9wZXJ0eSB7c3RyaW5nfSBlbnYuZGV2aWNlTW9kZWwgKipuYXRpdmUgb25seSoqXG4gKiBAcHJvcGVydHkge251bWJlcn0gZW52LltkZXZpY2VXaWR0aD03NTBdXG4gKiBAcHJvcGVydHkge251bWJlcn0gZW52LmRldmljZUhlaWdodFxuICovXG5leHBvcnQgZnVuY3Rpb24gJGdldENvbmZpZyAoY2FsbGJhY2spIHtcbiAgY29uc3QgY29uZmlnID0gZXh0ZW5kKHtcbiAgICBlbnY6IGdsb2JhbC5XWEVudmlyb25tZW50IHx8IHt9XG4gIH0sIHRoaXMuX2FwcC5vcHRpb25zKVxuICBpZiAodHlwb2YoY2FsbGJhY2spID09PSAnZnVuY3Rpb24nKSB7XG4gICAgY29uc29sZS53YXJuKCdbSlMgRnJhbWV3b3JrXSB0aGUgY2FsbGJhY2sgb2YgVm0jJGdldENvbmZpZyhjYWxsYmFjaykgaXMgZGVwcmVjYXRlZCwgJyArXG4gICAgICAndGhpcyBhcGkgbm93IGNhbiBkaXJlY3RseSBSRVRVUk4gY29uZmlnIGluZm8uJylcbiAgICBjYWxsYmFjayhjb25maWcpXG4gIH1cbiAgcmV0dXJuIGNvbmZpZ1xufVxuXG4vKipcbiAqIEBkZXByZWNhdGVkXG4gKiByZXF1ZXN0IG5ldHdvcmsgdmlhIGh0dHAgcHJvdG9jb2xcbiAqIEBwYXJhbSAge29iamVjdH0gICBwYXJhbXNcbiAqIEBwYXJhbSAge0Z1bmN0aW9ufSBjYWxsYmFja1xuICovXG5leHBvcnQgZnVuY3Rpb24gJHNlbmRIdHRwIChwYXJhbXMsIGNhbGxiYWNrKSB7XG4gIGNvbnNvbGUud2FybignW0pTIEZyYW1ld29ya10gVm0jJHNlbmRIdHRwIGlzIGRlcHJlY2F0ZWQsICcgK1xuICAgICAgICAgICdwbGVhc2UgdXNlIFwicmVxdWlyZShcXCdAd2VleC1tb2R1bGUvc3RyZWFtXFwnKScgK1xuICAgICAgICAgICcuc2VuZEh0dHAocGFyYW1zLCBjYWxsYmFjaylcIiBpbnN0ZWFkJylcbiAgY29uc3Qgc3RyZWFtID0gdGhpcy5fYXBwLnJlcXVpcmVNb2R1bGUoJ3N0cmVhbScpXG4gIHN0cmVhbS5zZW5kSHR0cChwYXJhbXMsIGNhbGxiYWNrKVxufVxuXG4vKipcbiAqIEBkZXByZWNhdGVkXG4gKiBvcGVuIGEgdXJsXG4gKiBAcGFyYW0gIHtzdHJpbmd9IHVybFxuICovXG5leHBvcnQgZnVuY3Rpb24gJG9wZW5VUkwgKHVybCkge1xuICBjb25zb2xlLndhcm4oJ1tKUyBGcmFtZXdvcmtdIFZtIyRvcGVuVVJMIGlzIGRlcHJlY2F0ZWQsICcgK1xuICAgICAgICAgICdwbGVhc2UgdXNlIFwicmVxdWlyZShcXCdAd2VleC1tb2R1bGUvZXZlbnRcXCcpJyArXG4gICAgICAgICAgJy5vcGVuVVJMKHVybClcIiBpbnN0ZWFkJylcbiAgY29uc3QgZXZlbnQgPSB0aGlzLl9hcHAucmVxdWlyZU1vZHVsZSgnZXZlbnQnKVxuICBldmVudC5vcGVuVVJMKHVybClcbn1cblxuLyoqXG4gKiBAZGVwcmVjYXRlZFxuICogc2V0IGEgdGl0bGUgZm9yIHBhZ2VcbiAqIEBwYXJhbSAge3N0cmluZ30gdGl0bGVcbiAqL1xuZXhwb3J0IGZ1bmN0aW9uICRzZXRUaXRsZSAodGl0bGUpIHtcbiAgY29uc29sZS53YXJuKCdbSlMgRnJhbWV3b3JrXSBWbSMkc2V0VGl0bGUgaXMgZGVwcmVjYXRlZCwgJyArXG4gICAgICAgICAgJ3BsZWFzZSB1c2UgXCJyZXF1aXJlKFxcJ0B3ZWV4LW1vZHVsZS9wYWdlSW5mb1xcJyknICtcbiAgICAgICAgICAnLnNldFRpdGxlKHRpdGxlKVwiIGluc3RlYWQnKVxuICBjb25zdCBwYWdlSW5mbyA9IHRoaXMuX2FwcC5yZXF1aXJlTW9kdWxlKCdwYWdlSW5mbycpXG4gIHBhZ2VJbmZvLnNldFRpdGxlKHRpdGxlKVxufVxuXG4vKipcbiAqIEBkZXByZWNhdGVkIHVzZSBcInJlcXVpcmUoJ0B3ZWV4LW1vZHVsZS9tb2R1bGVOYW1lJykgaW5zdGVhZFwiXG4gKiBpbnZva2UgYSBuYXRpdmUgbWV0aG9kIGJ5IHNwZWNpZmluZyB0aGUgbmFtZSBvZiBtb2R1bGUgYW5kIG1ldGhvZFxuICogQHBhcmFtICB7c3RyaW5nfSBtb2R1bGVOYW1lXG4gKiBAcGFyYW0gIHtzdHJpbmd9IG1ldGhvZE5hbWVcbiAqIEBwYXJhbSAgey4uLip9IHRoZSByZXN0IGFyZ3VtZW50c1xuICovXG5leHBvcnQgZnVuY3Rpb24gJGNhbGwgKG1vZHVsZU5hbWUsIG1ldGhvZE5hbWUsIC4uLmFyZ3MpIHtcbiAgY29uc29sZS53YXJuKCdbSlMgRnJhbWV3b3JrXSBWbSMkY2FsbCBpcyBkZXByZWNhdGVkLCAnICtcbiAgICAncGxlYXNlIHVzZSBcInJlcXVpcmUoXFwnQHdlZXgtbW9kdWxlL21vZHVsZU5hbWVcXCcpXCIgaW5zdGVhZCcpXG4gIGNvbnN0IG1vZHVsZSA9IHRoaXMuX2FwcC5yZXF1aXJlTW9kdWxlKG1vZHVsZU5hbWUpXG4gIGlmIChtb2R1bGUgJiYgbW9kdWxlW21ldGhvZE5hbWVdKSB7XG4gICAgbW9kdWxlW21ldGhvZE5hbWVdKC4uLmFyZ3MpXG4gIH1cbn1cblxuXG5cbi8qKiBXRUJQQUNLIEZPT1RFUiAqKlxuICoqIC9Vc2Vycy96aGFvamluamlhbmcvU2l0ZXMvcHVibGljL3dlZXgvZGlzdC93ZWV4LWpzLWZyYW1ld29yay9zcmMvZGVmYXVsdC9hcGkvbWV0aG9kcy5qc1xuICoqLyIsIi8qKlxuICogQGZpbGVPdmVydmlldyBUaGUgYXBpIGZvciBpbnZva2luZyB3aXRoIFwiJFwiIHByZWZpeFxuICovXG5cbi8qKlxuICogPT09PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09PVxuICogcHJpdmF0ZSBmb3IgYWxpXG4gKiA9PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09XG4gKi9cblxuLyoqXG4gKiBpbnZva2UgdXNlci10cmFjayBvbiBUYW9iYW8gTW9ibGllXG4gKiBAcGFyYW0ge3N0cmluZ30gdHlwZe+8mmVudGVyLCBjbGljaywgZXhwb3NlXG4gKiBAcGFyYW0ge3N0cmluZ30gbmFtZVxuICogQHBhcmFtIHtzdHJpbmd9IGNvbU5hbWVcbiAqIEBwYXJhbSB7b2JqZWN0fSBwYXJhbVxuKi9cbmV4cG9ydCBmdW5jdGlvbiAkdXNlclRyYWNrKHR5cGUsIG5hbWUsIGNvbU5hbWUsIHBhcmFtKSB7XG4gIGNvbnNvbGUud2FybignW0pTIEZyYW1ld29ya10gVm0jJHVzZXJUcmFjayBpcyBkZXByZWNhdGVkLCAnICtcbiAgICAgICAgICAncGxlYXNlIHVzZSBcInJlcXVpcmUoXFwnQHdlZXgtbW9kdWxlL3VzZXJUcmFja1xcJyknICtcbiAgICAgICAgICAnLmNvbW1pdCh0eXBlLCBuYW1lLCBjb21OYW1lLCBwYXJhbSlcIiBpbnN0ZWFkJylcbiAgY29uc3QgdXNlclRyYWNrID0gdGhpcy5fYXBwLnJlcXVpcmVNb2R1bGUoJ3VzZXJUcmFjaycpXG4gIHVzZXJUcmFjay5jb21taXQodHlwZSwgbmFtZSwgY29tTmFtZSwgcGFyYW0pXG59XG5cbi8qKlxuICogcmVxdWVzdCBhIHJlc3RmdWxsIGFwaSB2aWEgdGhlIG10b3AgZ2F0ZXdheVxuICogQHBhcmFtICB7b2JqZWN0fSAgIHBhcmFtc1xuICogQHBhcmFtICB7RnVuY3Rpb259IGNhbGxiYWNrXG4gKi9cbmV4cG9ydCBmdW5jdGlvbiAkc2VuZE10b3AocGFyYW1zLCBjYWxsYmFjaykge1xuICBjb25zb2xlLndhcm4oJ1tKUyBGcmFtZXdvcmtdIFZtIyRzZW5kTXRvcCBpcyBkZXByZWNhdGVkLCAnICtcbiAgICAgICAgICAncGxlYXNlIHVzZSBcInJlcXVpcmUoXFwnQHdlZXgtbW9kdWxlL3N0cmVhbVxcJyknICtcbiAgICAgICAgICAnLnNlbmRNdG9wKHBhcmFtcywgY2FsbGJhY2spXCIgaW5zdGVhZCcpXG4gIC8qIGlzdGFuYnVsIGlnbm9yZSBlbHNlICovXG4gIGlmICh0eXBlb2Ygd2luZG93ID09PSAndW5kZWZpbmVkJykge1xuICAgIC8vIGluIG5hdGl2Ze+8jHVzZSB3aW5kdmFuZVxuICAgIGNvbnN0IHdpbmR2YW5lID0gdGhpcy5fYXBwLnJlcXVpcmVNb2R1bGUoJ3dpbmR2YW5lJylcbiAgICB3aW5kdmFuZS5jYWxsKHtcbiAgICAgIGNsYXNzOiAnTXRvcFdWUGx1Z2luJyxcbiAgICAgIG1ldGhvZDogJ3NlbmQnLFxuICAgICAgZGF0YTogcGFyYW1zXG4gICAgfSwgY2FsbGJhY2spXG4gIH0gZWxzZSB7XG4gICAgLy8gaW4gd2ViIGJyd29zZXLvvIx1c2Ugc3RyZWFtLnNlbmRNdG9wXG4gICAgY29uc3Qgc3RyZWFtID0gdGhpcy5fYXBwLnJlcXVpcmVNb2R1bGUoJ3N0cmVhbScpXG4gICAgc3RyZWFtLnNlbmRNdG9wKHBhcmFtcywgY2FsbGJhY2spXG4gIH1cbn1cblxuLyoqXG4gKiByZXF1ZXN0IGEgbmF0aXZlIGFwaSB2aWEgd2luZHZhbmUgcHJvdG9jb2xcbiAqIEBwYXJhbSAge29iamVjdH0gICBwYXJhbXNcbiAqIEBwYXJhbSAge0Z1bmN0aW9ufSBjYWxsYmFja1xuICovXG5leHBvcnQgZnVuY3Rpb24gJGNhbGxXaW5kdmFuZShwYXJhbXMsIGNhbGxiYWNrKSB7XG4gIGNvbnNvbGUud2FybignW0pTIEZyYW1ld29ya10gVm0jJGNhbGxXaW5kdmFuZSBpcyBkZXByZWNhdGVkLCAnICtcbiAgICAgICAgICAncGxlYXNlIHVzZSBcInJlcXVpcmUoXFwnQHdlZXgtbW9kdWxlL3dpbmR2YW5lXFwnKScgK1xuICAgICAgICAgICcuY2FsbChwYXJhbXMsIGNhbGxiYWNrKVwiIGluc3RlYWQnKVxuICBjb25zdCB3aW5kdmFuZSA9IHRoaXMuX2FwcC5yZXF1aXJlTW9kdWxlKCd3aW5kdmFuZScpXG4gIHdpbmR2YW5lLmNhbGwocGFyYW1zLCBjYWxsYmFjaylcbn1cblxuLyoqXG4gKiBzZXQgc3BtIGZvciB0aGUgcGFnZVxuICogQHBhcmFtICB7c3RyaW5nfSBhXG4gKiBAcGFyYW0gIHtzdHJpbmd9IGJcbiAqL1xuZXhwb3J0IGZ1bmN0aW9uICRzZXRTcG0oYSwgYikge1xuICBjb25zb2xlLndhcm4oJ1tKUyBGcmFtZXdvcmtdIFZtIyRzZXRTcG0gaXMgZGVwcmVjYXRlZCwgJyArXG4gICAgICAgICAgJ3BsZWFzZSB1c2UgXCJyZXF1aXJlKFxcJ0B3ZWV4LW1vZHVsZS9wYWdlSW5mb1xcJyknICtcbiAgICAgICAgICAnLnNldFNwbShhLCBiKVwiIGluc3RlYWQnKVxuICBjb25zdCBwYWdlSW5mbyA9IHRoaXMuX2FwcC5yZXF1aXJlTW9kdWxlKCdwYWdlSW5mbycpXG4gIHBhZ2VJbmZvLnNldFNwbShhLCBiKVxufVxuXG4vKipcbiAqIGdldCB0aGUgaW5mb3JtYXRpb24gb2YgdGhlIGN1cnJlbnQgbG9naW5lZCB1c2VyXG4gKiBAcGFyYW0gIHtGdW5jdGlvbn0gY2FsbGJhY2tcbiAqL1xuZXhwb3J0IGZ1bmN0aW9uICRnZXRVc2VySW5mbyhjYWxsYmFjaykge1xuICBjb25zb2xlLndhcm4oJ1tKUyBGcmFtZXdvcmtdIFZtIyRnZXRVc2VySW5mbyBpcyBkZXByZWNhdGVkLCAnICtcbiAgICAgICAgICAncGxlYXNlIHVzZSBcInJlcXVpcmUoXFwnQHdlZXgtbW9kdWxlL3VzZXJcXCcpJyArXG4gICAgICAgICAgJy5nZXRVc2VySW5mbyhjYWxsYmFjaylcIiBpbnN0ZWFkJylcbiAgY29uc3QgdXNlciA9IHRoaXMuX2FwcC5yZXF1aXJlTW9kdWxlKCd1c2VyJylcbiAgdXNlci5nZXRVc2VySW5mbyhjYWxsYmFjaylcbn1cblxuLyoqXG4gKiBwZXJmb3JtIGxvZ2luXG4gKiBAcGFyYW0gIHtGdW5jdGlvbn0gY2FsbGJhY2tcbiAqL1xuZXhwb3J0IGZ1bmN0aW9uICRsb2dpbihjYWxsYmFjaykge1xuICBjb25zb2xlLndhcm4oJ1tKUyBGcmFtZXdvcmtdIFZtIyRsb2dpbiBpcyBkZXByZWNhdGVkLCAnICtcbiAgICAgICAgICAncGxlYXNlIHVzZSBcInJlcXVpcmUoXFwnQHdlZXgtbW9kdWxlL3VzZXJcXCcpJyArXG4gICAgICAgICAgJy5sb2dpbihjYWxsYmFjaylcIiBpbnN0ZWFkJylcbiAgY29uc3QgdXNlciA9IHRoaXMuX2FwcC5yZXF1aXJlTW9kdWxlKCd1c2VyJylcbiAgdXNlci5sb2dpbihjYWxsYmFjaylcbn1cblxuLyoqXG4gKiBwZXJmb3JtIGxvZ291dFxuICogQHBhcmFtICB7RnVuY3Rpb259IGNhbGxiYWNrXG4gKi9cbmV4cG9ydCBmdW5jdGlvbiAkbG9nb3V0KGNhbGxiYWNrKSB7XG4gIGNvbnNvbGUud2FybignW0pTIEZyYW1ld29ya10gVm0jJGxvZ291dCBpcyBkZXByZWNhdGVkLCAnICtcbiAgICAgICAgICAncGxlYXNlIHVzZSBcInJlcXVpcmUoXFwnQHdlZXgtbW9kdWxlL3VzZXJcXCcpJyArXG4gICAgICAgICAgJy5sb2dvdXQoY2FsbGJhY2spXCIgaW5zdGVhZCcpXG4gIGNvbnN0IHVzZXIgPSB0aGlzLl9hcHAucmVxdWlyZU1vZHVsZSgndXNlcicpXG4gIHVzZXIubG9nb3V0KGNhbGxiYWNrKVxufVxuXG5cblxuLyoqIFdFQlBBQ0sgRk9PVEVSICoqXG4gKiogLi9saWIvYXBpL21ldGhvZHMuanNcbiAqKi8iXSwic291cmNlUm9vdCI6IiJ9
