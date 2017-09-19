(this.nativeLog || function(s) {console.log(s)})('START JS FRAMEWORK 0.21.11, Build 2017-09-19 15:53.');
;(this.getJSFMVersion = function(){return "0.21.11"});
var global = this, process = { env: {} };var setTimeout = global.setTimeout;

(function (global, factory) {
	typeof exports === 'object' && typeof module !== 'undefined' ? factory() :
	typeof define === 'function' && define.amd ? define(factory) :
	(factory());
}(this, (function () { 'use strict';

var subversion = {"browser":"0.5.0","framework":"0.21.11","vue-render":"0.12.3","transformer":">=0.1.5 <0.5"};

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

/* eslint-disable */

// Production steps of ECMA-262, Edition 6, 22.1.2.1
// Reference: https://people.mozilla.org/~jorendorff/es6-draft.html#sec-array.from

/* istanbul ignore if */
if (!Array.from) {
  Array.from = (function() {
    var toStr = Object.prototype.toString;
    var isCallable = function(fn) {
      return typeof fn === 'function' || toStr.call(fn) === '[object Function]';
    };
    var toInteger = function(value) {
      var number = Number(value);
      if (isNaN(number)) {
        return 0;
      }
      if (number === 0 || !isFinite(number)) {
        return number;
      }
      return (number > 0 ? 1 : -1) * Math.floor(Math.abs(number));
    };
    var maxSafeInteger = Math.pow(2, 53) - 1;
    var toLength = function(value) {
      var len = toInteger(value);
      return Math.min(Math.max(len, 0), maxSafeInteger);
    };

    // The length property of the from method is 1.
    return function from(arrayLike/*, mapFn, thisArg */) {
      // 1. Let C be the this value.
      var C = this;

      // 2. Let items be ToObject(arrayLike).
      var items = Object(arrayLike);

      // 3. ReturnIfAbrupt(items).
      if (arrayLike == null) {
        throw new TypeError('Array.from requires an array-like object - not null or undefined');
      }

      // 4. If mapfn is undefined, then let mapping be false.
      var mapFn = arguments.length > 1 ? arguments[1] : void undefined;
      var T;
      if (typeof mapFn !== 'undefined') {
        // 5. else
        // 5. a If IsCallable(mapfn) is false, throw a TypeError exception.
        if (!isCallable(mapFn)) {
          throw new TypeError('Array.from: when provided, the second argument must be a function');
        }

        // 5. b. If thisArg was supplied, let T be thisArg; else let T be undefined.
        if (arguments.length > 2) {
          T = arguments[2];
        }
      }

      // 10. Let lenValue be Get(items, "length").
      // 11. Let len be ToLength(lenValue).
      var len = toLength(items.length);

      // 13. If IsConstructor(C) is true, then
      // 13. a. Let A be the result of calling the [[Construct]] internal method of C with an argument list containing the single item len.
      // 14. a. Else, Let A be ArrayCreate(len).
      var A = isCallable(C) ? Object(new C(len)) : new Array(len);

      // 16. Let k be 0.
      var k = 0;
      // 17. Repeat, while k < len鈥� (also steps a - h)
      var kValue;
      while (k < len) {
        kValue = items[k];
        if (mapFn) {
          A[k] = typeof T === 'undefined' ? mapFn(kValue, k) : mapFn.call(T, kValue, k);
        } else {
          A[k] = kValue;
        }
        k += 1;
      }
      // 18. Let putStatus be Put(A, "length", len, true).
      A.length = len;
      // 20. Return A.
      return A;
    };
  }());
}

var commonjsGlobal = typeof window !== 'undefined' ? window : typeof global !== 'undefined' ? global : typeof self !== 'undefined' ? self : {};



function unwrapExports (x) {
	return x && x.__esModule ? x['default'] : x;
}

function createCommonjsModule(fn, module) {
	return module = { exports: {} }, fn(module, module.exports), module.exports;
}

var _global = createCommonjsModule(function (module) {
// https://github.com/zloirock/core-js/issues/86#issuecomment-115759028
var global = module.exports = typeof window != 'undefined' && window.Math == Math
  ? window : typeof self != 'undefined' && self.Math == Math ? self
  // eslint-disable-next-line no-new-func
  : Function('return this')();
if (typeof __g == 'number') { __g = global; } // eslint-disable-line no-undef
});

var _core = createCommonjsModule(function (module) {
var core = module.exports = { version: '2.5.0' };
if (typeof __e == 'number') { __e = core; } // eslint-disable-line no-undef
});

var _isObject = function (it) {
  return typeof it === 'object' ? it !== null : typeof it === 'function';
};

var isObject = _isObject;
var _anObject = function (it) {
  if (!isObject(it)) { throw TypeError(it + ' is not an object!'); }
  return it;
};

var _fails = function (exec) {
  try {
    return !!exec();
  } catch (e) {
    return true;
  }
};

// Thank's IE8 for his funny defineProperty
var _descriptors = !_fails(function () {
  return Object.defineProperty({}, 'a', { get: function () { return 7; } }).a != 7;
});

var isObject$1 = _isObject;
var document$1 = _global.document;
// typeof document.createElement is 'object' in old IE
var is = isObject$1(document$1) && isObject$1(document$1.createElement);
var _domCreate = function (it) {
  return is ? document$1.createElement(it) : {};
};

var _ie8DomDefine = !_descriptors && !_fails(function () {
  return Object.defineProperty(_domCreate('div'), 'a', { get: function () { return 7; } }).a != 7;
});

// 7.1.1 ToPrimitive(input [, PreferredType])
var isObject$2 = _isObject;
// instead of the ES6 spec version, we didn't implement @@toPrimitive case
// and the second argument - flag - preferred type is a string
var _toPrimitive = function (it, S) {
  if (!isObject$2(it)) { return it; }
  var fn, val;
  if (S && typeof (fn = it.toString) == 'function' && !isObject$2(val = fn.call(it))) { return val; }
  if (typeof (fn = it.valueOf) == 'function' && !isObject$2(val = fn.call(it))) { return val; }
  if (!S && typeof (fn = it.toString) == 'function' && !isObject$2(val = fn.call(it))) { return val; }
  throw TypeError("Can't convert object to primitive value");
};

var anObject = _anObject;
var IE8_DOM_DEFINE = _ie8DomDefine;
var toPrimitive = _toPrimitive;
var dP$1 = Object.defineProperty;

var f = _descriptors ? Object.defineProperty : function defineProperty(O, P, Attributes) {
  anObject(O);
  P = toPrimitive(P, true);
  anObject(Attributes);
  if (IE8_DOM_DEFINE) { try {
    return dP$1(O, P, Attributes);
  } catch (e) { /* empty */ } }
  if ('get' in Attributes || 'set' in Attributes) { throw TypeError('Accessors not supported!'); }
  if ('value' in Attributes) { O[P] = Attributes.value; }
  return O;
};

var _objectDp = {
	f: f
};

var _propertyDesc = function (bitmap, value) {
  return {
    enumerable: !(bitmap & 1),
    configurable: !(bitmap & 2),
    writable: !(bitmap & 4),
    value: value
  };
};

var dP = _objectDp;
var createDesc = _propertyDesc;
var _hide = _descriptors ? function (object, key, value) {
  return dP.f(object, key, createDesc(1, value));
} : function (object, key, value) {
  object[key] = value;
  return object;
};

var hasOwnProperty = {}.hasOwnProperty;
var _has = function (it, key) {
  return hasOwnProperty.call(it, key);
};

var id = 0;
var px = Math.random();
var _uid = function (key) {
  return 'Symbol('.concat(key === undefined ? '' : key, ')_', (++id + px).toString(36));
};

var _redefine = createCommonjsModule(function (module) {
var global = _global;
var hide = _hide;
var has = _has;
var SRC = _uid('src');
var TO_STRING = 'toString';
var $toString = Function[TO_STRING];
var TPL = ('' + $toString).split(TO_STRING);

_core.inspectSource = function (it) {
  return $toString.call(it);
};

(module.exports = function (O, key, val, safe) {
  var isFunction = typeof val == 'function';
  if (isFunction) { has(val, 'name') || hide(val, 'name', key); }
  if (O[key] === val) { return; }
  if (isFunction) { has(val, SRC) || hide(val, SRC, O[key] ? '' + O[key] : TPL.join(String(key))); }
  if (O === global) {
    O[key] = val;
  } else if (!safe) {
    delete O[key];
    hide(O, key, val);
  } else if (O[key]) {
    O[key] = val;
  } else {
    hide(O, key, val);
  }
// add fake Function#toString for correct work wrapped methods / constructors with methods like LoDash isNative
})(Function.prototype, TO_STRING, function toString() {
  return typeof this == 'function' && this[SRC] || $toString.call(this);
});
});

var _aFunction = function (it) {
  if (typeof it != 'function') { throw TypeError(it + ' is not a function!'); }
  return it;
};

// optional / simple context binding
var aFunction = _aFunction;
var _ctx = function (fn, that, length) {
  aFunction(fn);
  if (that === undefined) { return fn; }
  switch (length) {
    case 1: return function (a) {
      return fn.call(that, a);
    };
    case 2: return function (a, b) {
      return fn.call(that, a, b);
    };
    case 3: return function (a, b, c) {
      return fn.call(that, a, b, c);
    };
  }
  return function (/* ...args */) {
    return fn.apply(that, arguments);
  };
};

var global$1 = _global;
var core = _core;
var hide = _hide;
var redefine = _redefine;
var ctx = _ctx;
var PROTOTYPE = 'prototype';

var $export$1 = function (type, name, source) {
  var IS_FORCED = type & $export$1.F;
  var IS_GLOBAL = type & $export$1.G;
  var IS_STATIC = type & $export$1.S;
  var IS_PROTO = type & $export$1.P;
  var IS_BIND = type & $export$1.B;
  var target = IS_GLOBAL ? global$1 : IS_STATIC ? global$1[name] || (global$1[name] = {}) : (global$1[name] || {})[PROTOTYPE];
  var exports = IS_GLOBAL ? core : core[name] || (core[name] = {});
  var expProto = exports[PROTOTYPE] || (exports[PROTOTYPE] = {});
  var key, own, out, exp;
  if (IS_GLOBAL) { source = name; }
  for (key in source) {
    // contains in native
    own = !IS_FORCED && target && target[key] !== undefined;
    // export native or passed
    out = (own ? target : source)[key];
    // bind timers to global for call from export context
    exp = IS_BIND && own ? ctx(out, global$1) : IS_PROTO && typeof out == 'function' ? ctx(Function.call, out) : out;
    // extend global
    if (target) { redefine(target, key, out, type & $export$1.U); }
    // export
    if (exports[key] != out) { hide(exports, key, exp); }
    if (IS_PROTO && expProto[key] != out) { expProto[key] = out; }
  }
};
global$1.core = core;
// type bitmap
$export$1.F = 1;   // forced
$export$1.G = 2;   // global
$export$1.S = 4;   // static
$export$1.P = 8;   // proto
$export$1.B = 16;  // bind
$export$1.W = 32;  // wrap
$export$1.U = 64;  // safe
$export$1.R = 128; // real proto method for `library`
var _export = $export$1;

var toString$1 = {}.toString;

var _cof = function (it) {
  return toString$1.call(it).slice(8, -1);
};

// fallback for non-array-like ES3 and non-enumerable old V8 strings
var cof = _cof;
// eslint-disable-next-line no-prototype-builtins
var _iobject = Object('z').propertyIsEnumerable(0) ? Object : function (it) {
  return cof(it) == 'String' ? it.split('') : Object(it);
};

// 7.2.1 RequireObjectCoercible(argument)
var _defined = function (it) {
  if (it == undefined) { throw TypeError("Can't call method on  " + it); }
  return it;
};

// to indexed object, toObject with fallback for non-array-like ES3 strings
var IObject$1 = _iobject;
var defined = _defined;
var _toIobject = function (it) {
  return IObject$1(defined(it));
};

// 7.1.4 ToInteger
var ceil = Math.ceil;
var floor = Math.floor;
var _toInteger = function (it) {
  return isNaN(it = +it) ? 0 : (it > 0 ? floor : ceil)(it);
};

// 7.1.15 ToLength
var toInteger = _toInteger;
var min = Math.min;
var _toLength = function (it) {
  return it > 0 ? min(toInteger(it), 0x1fffffffffffff) : 0; // pow(2, 53) - 1 == 9007199254740991
};

var toInteger$1 = _toInteger;
var max = Math.max;
var min$1 = Math.min;
var _toAbsoluteIndex = function (index, length) {
  index = toInteger$1(index);
  return index < 0 ? max(index + length, 0) : min$1(index, length);
};

// false -> Array#indexOf
// true  -> Array#includes
var toIObject$1 = _toIobject;
var toLength = _toLength;
var toAbsoluteIndex = _toAbsoluteIndex;
var _arrayIncludes = function (IS_INCLUDES) {
  return function ($this, el, fromIndex) {
    var O = toIObject$1($this);
    var length = toLength(O.length);
    var index = toAbsoluteIndex(fromIndex, length);
    var value;
    // Array#includes uses SameValueZero equality algorithm
    // eslint-disable-next-line no-self-compare
    if (IS_INCLUDES && el != el) { while (length > index) {
      value = O[index++];
      // eslint-disable-next-line no-self-compare
      if (value != value) { return true; }
    // Array#indexOf ignores holes, Array#includes - not
    } } else { for (;length > index; index++) { if (IS_INCLUDES || index in O) {
      if (O[index] === el) { return IS_INCLUDES || index || 0; }
    } } } return !IS_INCLUDES && -1;
  };
};

var global$2 = _global;
var SHARED = '__core-js_shared__';
var store = global$2[SHARED] || (global$2[SHARED] = {});
var _shared = function (key) {
  return store[key] || (store[key] = {});
};

var shared = _shared('keys');
var uid = _uid;
var _sharedKey = function (key) {
  return shared[key] || (shared[key] = uid(key));
};

var has$1 = _has;
var toIObject = _toIobject;
var arrayIndexOf = _arrayIncludes(false);
var IE_PROTO = _sharedKey('IE_PROTO');

var _objectKeysInternal = function (object, names) {
  var O = toIObject(object);
  var i = 0;
  var result = [];
  var key;
  for (key in O) { if (key != IE_PROTO) { has$1(O, key) && result.push(key); } }
  // Don't enum bug & hidden keys
  while (names.length > i) { if (has$1(O, key = names[i++])) {
    ~arrayIndexOf(result, key) || result.push(key);
  } }
  return result;
};

// IE 8- don't enum bug keys
var _enumBugKeys = (
  'constructor,hasOwnProperty,isPrototypeOf,propertyIsEnumerable,toLocaleString,toString,valueOf'
).split(',');

// 19.1.2.14 / 15.2.3.14 Object.keys(O)
var $keys = _objectKeysInternal;
var enumBugKeys = _enumBugKeys;

var _objectKeys = Object.keys || function keys(O) {
  return $keys(O, enumBugKeys);
};

var f$1 = Object.getOwnPropertySymbols;

var _objectGops = {
	f: f$1
};

var f$2 = {}.propertyIsEnumerable;

var _objectPie = {
	f: f$2
};

// 7.1.13 ToObject(argument)
var defined$1 = _defined;
var _toObject = function (it) {
  return Object(defined$1(it));
};

// 19.1.2.1 Object.assign(target, source, ...)
var getKeys = _objectKeys;
var gOPS = _objectGops;
var pIE = _objectPie;
var toObject = _toObject;
var IObject = _iobject;
var $assign = Object.assign;

// should work with symbols and should have deterministic property order (V8 bug)
var _objectAssign = !$assign || _fails(function () {
  var A = {};
  var B = {};
  // eslint-disable-next-line no-undef
  var S = Symbol();
  var K = 'abcdefghijklmnopqrst';
  A[S] = 7;
  K.split('').forEach(function (k) { B[k] = k; });
  return $assign({}, A)[S] != 7 || Object.keys($assign({}, B)).join('') != K;
}) ? function assign(target, source) {
  var arguments$1 = arguments;
 // eslint-disable-line no-unused-vars
  var T = toObject(target);
  var aLen = arguments.length;
  var index = 1;
  var getSymbols = gOPS.f;
  var isEnum = pIE.f;
  while (aLen > index) {
    var S = IObject(arguments$1[index++]);
    var keys = getSymbols ? getKeys(S).concat(getSymbols(S)) : getKeys(S);
    var length = keys.length;
    var j = 0;
    var key;
    while (length > j) { if (isEnum.call(S, key = keys[j++])) { T[key] = S[key]; } }
  } return T;
} : $assign;

// 19.1.3.1 Object.assign(target, source)
var $export = _export;

$export($export.S + $export.F, 'Object', { assign: _objectAssign });

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

/* eslint-disable */

// https://gist.github.com/WebReflection/5593554

/* istanbul ignore if */
if (!Object.setPrototypeOf) {
  Object.setPrototypeOf = (function(Object, magic) {
    var set;
    function setPrototypeOf(O, proto) {
      set.call(O, proto);
      return O;
    }
    try {
      // this works already in Firefox and Safari
      set = Object.getOwnPropertyDescriptor(Object.prototype, magic).set;
      set.call({}, null);
    } catch (e) {
      if (
        // IE < 11 cannot be shimmed
        Object.prototype !== {}[magic] ||
        // neither can any browser that actually
        // implemented __proto__ correctly
        // (all but old V8 will return here)
        {__proto__: null}.__proto__ === void 0
        // this case means null objects cannot be passed
        // through setPrototypeOf in a reliable way
        // which means here a **Sham** is needed instead
      ) {
        return;
      }
      // nodejs 0.8 and 0.10 are (buggy and..) fine here
      // probably Chrome or some old Mobile stock browser
      set = function(proto) {
        this[magic] = proto;
      };
      // please note that this will **not** work
      // in those browsers that do not inherit
      // __proto__ by mistake from Object.prototype
      // in these cases we should probably throw an error
      // or at least be informed about the issue
      setPrototypeOf.polyfill = setPrototypeOf(
        setPrototypeOf({}, null),
        Object.prototype
      ) instanceof Object;
      // setPrototypeOf.polyfill === true means it works as meant
      // setPrototypeOf.polyfill === false means it's not 100% reliable
      // setPrototypeOf.polyfill === undefined
      // or
      // setPrototypeOf.polyfill ==  null means it's not a polyfill
      // which means it works as expected
      // we can even delete Object.prototype.__proto__;
    }
    return setPrototypeOf;
  }(Object, '__proto__'));
}

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

// fix Promise Problem on JSContext of iOS7~8
// @see https://bugs.webkit.org/show_bug.cgi?id=135866

var ref = commonjsGlobal;
var WXEnvironment$1 = ref.WXEnvironment;

/* istanbul ignore next */
if (WXEnvironment$1 && WXEnvironment$1.platform === 'iOS') {
  commonjsGlobal.Promise = undefined;
}

var _wks = createCommonjsModule(function (module) {
var store = _shared('wks');
var uid = _uid;
var Symbol = _global.Symbol;
var USE_SYMBOL = typeof Symbol == 'function';

var $exports = module.exports = function (name) {
  return store[name] || (store[name] =
    USE_SYMBOL && Symbol[name] || (USE_SYMBOL ? Symbol : uid)('Symbol.' + name));
};

$exports.store = store;
});

// getting tag from 19.1.3.6 Object.prototype.toString()
var cof$1 = _cof;
var TAG = _wks('toStringTag');
// ES3 wrong here
var ARG = cof$1(function () { return arguments; }()) == 'Arguments';

// fallback for IE11 Script Access Denied error
var tryGet = function (it, key) {
  try {
    return it[key];
  } catch (e) { /* empty */ }
};

var _classof = function (it) {
  var O, T, B;
  return it === undefined ? 'Undefined' : it === null ? 'Null'
    // @@toStringTag case
    : typeof (T = tryGet(O = Object(it), TAG)) == 'string' ? T
    // builtinTag case
    : ARG ? cof$1(O)
    // ES3 arguments fallback
    : (B = cof$1(O)) == 'Object' && typeof O.callee == 'function' ? 'Arguments' : B;
};

// 19.1.3.6 Object.prototype.toString()
var classof = _classof;
var test = {};
test[_wks('toStringTag')] = 'z';
if (test + '' != '[object z]') {
  _redefine(Object.prototype, 'toString', function toString() {
    return '[object ' + classof(this) + ']';
  }, true);
}

var toInteger$2 = _toInteger;
var defined$2 = _defined;
// true  -> String#at
// false -> String#codePointAt
var _stringAt = function (TO_STRING) {
  return function (that, pos) {
    var s = String(defined$2(that));
    var i = toInteger$2(pos);
    var l = s.length;
    var a, b;
    if (i < 0 || i >= l) { return TO_STRING ? '' : undefined; }
    a = s.charCodeAt(i);
    return a < 0xd800 || a > 0xdbff || i + 1 === l || (b = s.charCodeAt(i + 1)) < 0xdc00 || b > 0xdfff
      ? TO_STRING ? s.charAt(i) : a
      : TO_STRING ? s.slice(i, i + 2) : (a - 0xd800 << 10) + (b - 0xdc00) + 0x10000;
  };
};

var _library = false;

var _iterators = {};

var dP$2 = _objectDp;
var anObject$2 = _anObject;
var getKeys$1 = _objectKeys;

var _objectDps = _descriptors ? Object.defineProperties : function defineProperties(O, Properties) {
  anObject$2(O);
  var keys = getKeys$1(Properties);
  var length = keys.length;
  var i = 0;
  var P;
  while (length > i) { dP$2.f(O, P = keys[i++], Properties[P]); }
  return O;
};

var document$2 = _global.document;
var _html = document$2 && document$2.documentElement;

// 19.1.2.2 / 15.2.3.5 Object.create(O [, Properties])
var anObject$1 = _anObject;
var dPs = _objectDps;
var enumBugKeys$1 = _enumBugKeys;
var IE_PROTO$1 = _sharedKey('IE_PROTO');
var Empty = function () { /* empty */ };
var PROTOTYPE$1 = 'prototype';

// Create object with fake `null` prototype: use iframe Object with cleared prototype
var createDict = function () {
  // Thrash, waste and sodomy: IE GC bug
  var iframe = _domCreate('iframe');
  var i = enumBugKeys$1.length;
  var lt = '<';
  var gt = '>';
  var iframeDocument;
  iframe.style.display = 'none';
  _html.appendChild(iframe);
  iframe.src = 'javascript:'; // eslint-disable-line no-script-url
  // createDict = iframe.contentWindow.Object;
  // html.removeChild(iframe);
  iframeDocument = iframe.contentWindow.document;
  iframeDocument.open();
  iframeDocument.write(lt + 'script' + gt + 'document.F=Object' + lt + '/script' + gt);
  iframeDocument.close();
  createDict = iframeDocument.F;
  while (i--) { delete createDict[PROTOTYPE$1][enumBugKeys$1[i]]; }
  return createDict();
};

var _objectCreate = Object.create || function create(O, Properties) {
  var result;
  if (O !== null) {
    Empty[PROTOTYPE$1] = anObject$1(O);
    result = new Empty();
    Empty[PROTOTYPE$1] = null;
    // add "__proto__" for Object.getPrototypeOf polyfill
    result[IE_PROTO$1] = O;
  } else { result = createDict(); }
  return Properties === undefined ? result : dPs(result, Properties);
};

var def = _objectDp.f;
var has$3 = _has;
var TAG$1 = _wks('toStringTag');

var _setToStringTag = function (it, tag, stat) {
  if (it && !has$3(it = stat ? it : it.prototype, TAG$1)) { def(it, TAG$1, { configurable: true, value: tag }); }
};

var create$1 = _objectCreate;
var descriptor = _propertyDesc;
var setToStringTag$1 = _setToStringTag;
var IteratorPrototype = {};

// 25.1.2.1.1 %IteratorPrototype%[@@iterator]()
_hide(IteratorPrototype, _wks('iterator'), function () { return this; });

var _iterCreate = function (Constructor, NAME, next) {
  Constructor.prototype = create$1(IteratorPrototype, { next: descriptor(1, next) });
  setToStringTag$1(Constructor, NAME + ' Iterator');
};

// 19.1.2.9 / 15.2.3.2 Object.getPrototypeOf(O)
var has$4 = _has;
var toObject$1 = _toObject;
var IE_PROTO$2 = _sharedKey('IE_PROTO');
var ObjectProto = Object.prototype;

var _objectGpo = Object.getPrototypeOf || function (O) {
  O = toObject$1(O);
  if (has$4(O, IE_PROTO$2)) { return O[IE_PROTO$2]; }
  if (typeof O.constructor == 'function' && O instanceof O.constructor) {
    return O.constructor.prototype;
  } return O instanceof Object ? ObjectProto : null;
};

var LIBRARY = _library;
var $export$2 = _export;
var redefine$1 = _redefine;
var hide$1 = _hide;
var has$2 = _has;
var Iterators = _iterators;
var $iterCreate = _iterCreate;
var setToStringTag = _setToStringTag;
var getPrototypeOf = _objectGpo;
var ITERATOR = _wks('iterator');
var BUGGY = !([].keys && 'next' in [].keys()); // Safari has buggy iterators w/o `next`
var FF_ITERATOR = '@@iterator';
var KEYS = 'keys';
var VALUES = 'values';

var returnThis = function () { return this; };

var _iterDefine = function (Base, NAME, Constructor, next, DEFAULT, IS_SET, FORCED) {
  $iterCreate(Constructor, NAME, next);
  var getMethod = function (kind) {
    if (!BUGGY && kind in proto) { return proto[kind]; }
    switch (kind) {
      case KEYS: return function keys() { return new Constructor(this, kind); };
      case VALUES: return function values() { return new Constructor(this, kind); };
    } return function entries() { return new Constructor(this, kind); };
  };
  var TAG = NAME + ' Iterator';
  var DEF_VALUES = DEFAULT == VALUES;
  var VALUES_BUG = false;
  var proto = Base.prototype;
  var $native = proto[ITERATOR] || proto[FF_ITERATOR] || DEFAULT && proto[DEFAULT];
  var $default = $native || getMethod(DEFAULT);
  var $entries = DEFAULT ? !DEF_VALUES ? $default : getMethod('entries') : undefined;
  var $anyNative = NAME == 'Array' ? proto.entries || $native : $native;
  var methods, key, IteratorPrototype;
  // Fix native
  if ($anyNative) {
    IteratorPrototype = getPrototypeOf($anyNative.call(new Base()));
    if (IteratorPrototype !== Object.prototype && IteratorPrototype.next) {
      // Set @@toStringTag to native iterators
      setToStringTag(IteratorPrototype, TAG, true);
      // fix for some old engines
      if (!LIBRARY && !has$2(IteratorPrototype, ITERATOR)) { hide$1(IteratorPrototype, ITERATOR, returnThis); }
    }
  }
  // fix Array#{values, @@iterator}.name in V8 / FF
  if (DEF_VALUES && $native && $native.name !== VALUES) {
    VALUES_BUG = true;
    $default = function values() { return $native.call(this); };
  }
  // Define iterator
  if ((!LIBRARY || FORCED) && (BUGGY || VALUES_BUG || !proto[ITERATOR])) {
    hide$1(proto, ITERATOR, $default);
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
    if (FORCED) { for (key in methods) {
      if (!(key in proto)) { redefine$1(proto, key, methods[key]); }
    } } else { $export$2($export$2.P + $export$2.F * (BUGGY || VALUES_BUG), NAME, methods); }
  }
  return methods;
};

var $at = _stringAt(true);

// 21.1.3.27 String.prototype[@@iterator]()
_iterDefine(String, 'String', function (iterated) {
  this._t = String(iterated); // target
  this._i = 0;                // next index
// 21.1.5.2.1 %StringIteratorPrototype%.next()
}, function () {
  var O = this._t;
  var index = this._i;
  var point;
  if (index >= O.length) { return { value: undefined, done: true }; }
  point = $at(O, index);
  this._i += point.length;
  return { value: point, done: false };
});

// 22.1.3.31 Array.prototype[@@unscopables]
var UNSCOPABLES = _wks('unscopables');
var ArrayProto = Array.prototype;
if (ArrayProto[UNSCOPABLES] == undefined) { _hide(ArrayProto, UNSCOPABLES, {}); }
var _addToUnscopables = function (key) {
  ArrayProto[UNSCOPABLES][key] = true;
};

var _iterStep = function (done, value) {
  return { value: value, done: !!done };
};

var addToUnscopables = _addToUnscopables;
var step = _iterStep;
var Iterators$2 = _iterators;
var toIObject$2 = _toIobject;

// 22.1.3.4 Array.prototype.entries()
// 22.1.3.13 Array.prototype.keys()
// 22.1.3.29 Array.prototype.values()
// 22.1.3.30 Array.prototype[@@iterator]()
var es6_array_iterator = _iterDefine(Array, 'Array', function (iterated, kind) {
  this._t = toIObject$2(iterated); // target
  this._i = 0;                   // next index
  this._k = kind;                // kind
// 22.1.5.2.1 %ArrayIteratorPrototype%.next()
}, function () {
  var O = this._t;
  var kind = this._k;
  var index = this._i++;
  if (!O || index >= O.length) {
    this._t = undefined;
    return step(1);
  }
  if (kind == 'keys') { return step(0, index); }
  if (kind == 'values') { return step(0, O[index]); }
  return step(0, [index, O[index]]);
}, 'values');

// argumentsList[@@iterator] is %ArrayProto_values% (9.4.4.6, 9.4.4.7)
Iterators$2.Arguments = Iterators$2.Array;

addToUnscopables('keys');
addToUnscopables('values');
addToUnscopables('entries');

var $iterators = es6_array_iterator;
var getKeys$2 = _objectKeys;
var redefine$2 = _redefine;
var global$3 = _global;
var hide$2 = _hide;
var Iterators$1 = _iterators;
var wks = _wks;
var ITERATOR$1 = wks('iterator');
var TO_STRING_TAG = wks('toStringTag');
var ArrayValues = Iterators$1.Array;

var DOMIterables = {
  CSSRuleList: true, // TODO: Not spec compliant, should be false.
  CSSStyleDeclaration: false,
  CSSValueList: false,
  ClientRectList: false,
  DOMRectList: false,
  DOMStringList: false,
  DOMTokenList: true,
  DataTransferItemList: false,
  FileList: false,
  HTMLAllCollection: false,
  HTMLCollection: false,
  HTMLFormElement: false,
  HTMLSelectElement: false,
  MediaList: true, // TODO: Not spec compliant, should be false.
  MimeTypeArray: false,
  NamedNodeMap: false,
  NodeList: true,
  PaintRequestList: false,
  Plugin: false,
  PluginArray: false,
  SVGLengthList: false,
  SVGNumberList: false,
  SVGPathSegList: false,
  SVGPointList: false,
  SVGStringList: false,
  SVGTransformList: false,
  SourceBufferList: false,
  StyleSheetList: true, // TODO: Not spec compliant, should be false.
  TextTrackCueList: false,
  TextTrackList: false,
  TouchList: false
};

for (var collections = getKeys$2(DOMIterables), i = 0; i < collections.length; i++) {
  var NAME = collections[i];
  var explicit = DOMIterables[NAME];
  var Collection = global$3[NAME];
  var proto = Collection && Collection.prototype;
  var key;
  if (proto) {
    if (!proto[ITERATOR$1]) { hide$2(proto, ITERATOR$1, ArrayValues); }
    if (!proto[TO_STRING_TAG]) { hide$2(proto, TO_STRING_TAG, NAME); }
    Iterators$1[NAME] = ArrayValues;
    if (explicit) { for (key in $iterators) { if (!proto[key]) { redefine$2(proto, key, $iterators[key], true); } } }
  }
}

var _anInstance = function (it, Constructor, name, forbiddenField) {
  if (!(it instanceof Constructor) || (forbiddenField !== undefined && forbiddenField in it)) {
    throw TypeError(name + ': incorrect invocation!');
  } return it;
};

// call something on iterator step with safe closing on error
var anObject$3 = _anObject;
var _iterCall = function (iterator, fn, value, entries) {
  try {
    return entries ? fn(anObject$3(value)[0], value[1]) : fn(value);
  // 7.4.6 IteratorClose(iterator, completion)
  } catch (e) {
    var ret = iterator['return'];
    if (ret !== undefined) { anObject$3(ret.call(iterator)); }
    throw e;
  }
};

// check on default Array iterator
var Iterators$3 = _iterators;
var ITERATOR$2 = _wks('iterator');
var ArrayProto$1 = Array.prototype;

var _isArrayIter = function (it) {
  return it !== undefined && (Iterators$3.Array === it || ArrayProto$1[ITERATOR$2] === it);
};

var classof$2 = _classof;
var ITERATOR$3 = _wks('iterator');
var Iterators$4 = _iterators;
var core_getIteratorMethod = _core.getIteratorMethod = function (it) {
  if (it != undefined) { return it[ITERATOR$3]
    || it['@@iterator']
    || Iterators$4[classof$2(it)]; }
};

var _forOf = createCommonjsModule(function (module) {
var ctx = _ctx;
var call = _iterCall;
var isArrayIter = _isArrayIter;
var anObject = _anObject;
var toLength = _toLength;
var getIterFn = core_getIteratorMethod;
var BREAK = {};
var RETURN = {};
var exports = module.exports = function (iterable, entries, fn, that, ITERATOR) {
  var iterFn = ITERATOR ? function () { return iterable; } : getIterFn(iterable);
  var f = ctx(fn, that, entries ? 2 : 1);
  var index = 0;
  var length, step, iterator, result;
  if (typeof iterFn != 'function') { throw TypeError(iterable + ' is not iterable!'); }
  // fast case for arrays with default iterator
  if (isArrayIter(iterFn)) { for (length = toLength(iterable.length); length > index; index++) {
    result = entries ? f(anObject(step = iterable[index])[0], step[1]) : f(iterable[index]);
    if (result === BREAK || result === RETURN) { return result; }
  } } else { for (iterator = iterFn.call(iterable); !(step = iterator.next()).done;) {
    result = call(iterator, f, step.value, entries);
    if (result === BREAK || result === RETURN) { return result; }
  } }
};
exports.BREAK = BREAK;
exports.RETURN = RETURN;
});

// 7.3.20 SpeciesConstructor(O, defaultConstructor)
var anObject$4 = _anObject;
var aFunction$2 = _aFunction;
var SPECIES = _wks('species');
var _speciesConstructor = function (O, D) {
  var C = anObject$4(O).constructor;
  var S;
  return C === undefined || (S = anObject$4(C)[SPECIES]) == undefined ? D : aFunction$2(S);
};

// fast apply, http://jsperf.lnkit.com/fast-apply/5
var _invoke = function (fn, args, that) {
  var un = that === undefined;
  switch (args.length) {
    case 0: return un ? fn()
                      : fn.call(that);
    case 1: return un ? fn(args[0])
                      : fn.call(that, args[0]);
    case 2: return un ? fn(args[0], args[1])
                      : fn.call(that, args[0], args[1]);
    case 3: return un ? fn(args[0], args[1], args[2])
                      : fn.call(that, args[0], args[1], args[2]);
    case 4: return un ? fn(args[0], args[1], args[2], args[3])
                      : fn.call(that, args[0], args[1], args[2], args[3]);
  } return fn.apply(that, args);
};

var ctx$2 = _ctx;
var invoke = _invoke;
var html = _html;
var cel = _domCreate;
var global$5 = _global;
var process$2 = global$5.process;
var setTask = global$5.setImmediate;
var clearTask = global$5.clearImmediate;
var MessageChannel = global$5.MessageChannel;
var Dispatch = global$5.Dispatch;
var counter = 0;
var queue = {};
var ONREADYSTATECHANGE = 'onreadystatechange';
var defer;
var channel;
var port;
var run$1 = function () {
  var id = +this;
  // eslint-disable-next-line no-prototype-builtins
  if (queue.hasOwnProperty(id)) {
    var fn = queue[id];
    delete queue[id];
    fn();
  }
};
var listener = function (event) {
  run$1.call(event.data);
};
// Node.js 0.9+ & IE10+ has setImmediate, otherwise:
if (!setTask || !clearTask) {
  setTask = function setImmediate(fn) {
    var arguments$1 = arguments;

    var args = [];
    var i = 1;
    while (arguments.length > i) { args.push(arguments$1[i++]); }
    queue[++counter] = function () {
      // eslint-disable-next-line no-new-func
      invoke(typeof fn == 'function' ? fn : Function(fn), args);
    };
    defer(counter);
    return counter;
  };
  clearTask = function clearImmediate(id) {
    delete queue[id];
  };
  // Node.js 0.8-
  if (_cof(process$2) == 'process') {
    defer = function (id) {
      process$2.nextTick(ctx$2(run$1, id, 1));
    };
  // Sphere (JS game engine) Dispatch API
  } else if (Dispatch && Dispatch.now) {
    defer = function (id) {
      Dispatch.now(ctx$2(run$1, id, 1));
    };
  // Browsers with MessageChannel, includes WebWorkers
  } else if (MessageChannel) {
    channel = new MessageChannel();
    port = channel.port2;
    channel.port1.onmessage = listener;
    defer = ctx$2(port.postMessage, port, 1);
  // Browsers with postMessage, skip WebWorkers
  // IE8 has postMessage, but it's sync & typeof its postMessage is 'object'
  } else if (global$5.addEventListener && typeof postMessage == 'function' && !global$5.importScripts) {
    defer = function (id) {
      global$5.postMessage(id + '', '*');
    };
    global$5.addEventListener('message', listener, false);
  // IE8-
  } else if (ONREADYSTATECHANGE in cel('script')) {
    defer = function (id) {
      html.appendChild(cel('script'))[ONREADYSTATECHANGE] = function () {
        html.removeChild(this);
        run$1.call(id);
      };
    };
  // Rest old browsers
  } else {
    defer = function (id) {
      setTimeout(ctx$2(run$1, id, 1), 0);
    };
  }
}
var _task = {
  set: setTask,
  clear: clearTask
};

var global$6 = _global;
var macrotask = _task.set;
var Observer = global$6.MutationObserver || global$6.WebKitMutationObserver;
var process$3 = global$6.process;
var Promise$1 = global$6.Promise;
var isNode$1 = _cof(process$3) == 'process';

var _microtask = function () {
  var head, last, notify;

  var flush = function () {
    var parent, fn;
    if (isNode$1 && (parent = process$3.domain)) { parent.exit(); }
    while (head) {
      fn = head.fn;
      head = head.next;
      try {
        fn();
      } catch (e) {
        if (head) { notify(); }
        else { last = undefined; }
        throw e;
      }
    } last = undefined;
    if (parent) { parent.enter(); }
  };

  // Node.js
  if (isNode$1) {
    notify = function () {
      process$3.nextTick(flush);
    };
  // browsers with MutationObserver
  } else if (Observer) {
    var toggle = true;
    var node = document.createTextNode('');
    new Observer(flush).observe(node, { characterData: true }); // eslint-disable-line no-new
    notify = function () {
      node.data = toggle = !toggle;
    };
  // environments with maybe non-completely correct, but existent Promise
  } else if (Promise$1 && Promise$1.resolve) {
    var promise = Promise$1.resolve();
    notify = function () {
      promise.then(flush);
    };
  // for other environments - macrotask based on:
  // - setImmediate
  // - MessageChannel
  // - window.postMessag
  // - onreadystatechange
  // - setTimeout
  } else {
    notify = function () {
      // strange IE + webpack dev server bug - use .call(global)
      macrotask.call(global$6, flush);
    };
  }

  return function (fn) {
    var task = { fn: fn, next: undefined };
    if (last) { last.next = task; }
    if (!head) {
      head = task;
      notify();
    } last = task;
  };
};

// 25.4.1.5 NewPromiseCapability(C)
var aFunction$3 = _aFunction;

function PromiseCapability(C) {
  var resolve, reject;
  this.promise = new C(function ($$resolve, $$reject) {
    if (resolve !== undefined || reject !== undefined) { throw TypeError('Bad Promise constructor'); }
    resolve = $$resolve;
    reject = $$reject;
  });
  this.resolve = aFunction$3(resolve);
  this.reject = aFunction$3(reject);
}

var f$3 = function (C) {
  return new PromiseCapability(C);
};

var _newPromiseCapability = {
	f: f$3
};

var _perform = function (exec) {
  try {
    return { e: false, v: exec() };
  } catch (e) {
    return { e: true, v: e };
  }
};

var newPromiseCapability$1 = _newPromiseCapability;

var _promiseResolve = function (C, x) {
  var promiseCapability = newPromiseCapability$1.f(C);
  var resolve = promiseCapability.resolve;
  resolve(x);
  return promiseCapability.promise;
};

var redefine$3 = _redefine;
var _redefineAll = function (target, src, safe) {
  for (var key in src) { redefine$3(target, key, src[key], safe); }
  return target;
};

var global$7 = _global;
var dP$3 = _objectDp;
var DESCRIPTORS = _descriptors;
var SPECIES$1 = _wks('species');

var _setSpecies = function (KEY) {
  var C = global$7[KEY];
  if (DESCRIPTORS && C && !C[SPECIES$1]) { dP$3.f(C, SPECIES$1, {
    configurable: true,
    get: function () { return this; }
  }); }
};

var ITERATOR$4 = _wks('iterator');
var SAFE_CLOSING = false;

try {
  var riter = [7][ITERATOR$4]();
  riter['return'] = function () { SAFE_CLOSING = true; };
  // eslint-disable-next-line no-throw-literal
  Array.from(riter, function () { throw 2; });
} catch (e) { /* empty */ }

var _iterDetect = function (exec, skipClosing) {
  if (!skipClosing && !SAFE_CLOSING) { return false; }
  var safe = false;
  try {
    var arr = [7];
    var iter = arr[ITERATOR$4]();
    iter.next = function () { return { done: safe = true }; };
    arr[ITERATOR$4] = function () { return iter; };
    exec(arr);
  } catch (e) { /* empty */ }
  return safe;
};

var LIBRARY$1 = _library;
var global$4 = _global;
var ctx$1 = _ctx;
var classof$1 = _classof;
var $export$3 = _export;
var isObject$3 = _isObject;
var aFunction$1 = _aFunction;
var anInstance = _anInstance;
var forOf = _forOf;
var speciesConstructor = _speciesConstructor;
var task = _task.set;
var microtask = _microtask();
var newPromiseCapabilityModule = _newPromiseCapability;
var perform = _perform;
var promiseResolve = _promiseResolve;
var PROMISE = 'Promise';
var TypeError$1 = global$4.TypeError;
var process$1 = global$4.process;
var $Promise = global$4[PROMISE];
var isNode = classof$1(process$1) == 'process';
var empty = function () { /* empty */ };
var Internal;
var newGenericPromiseCapability;
var OwnPromiseCapability;
var Wrapper;
var newPromiseCapability = newGenericPromiseCapability = newPromiseCapabilityModule.f;

var USE_NATIVE = !!function () {
  try {
    // correct subclassing with @@species support
    var promise = $Promise.resolve(1);
    var FakePromise = (promise.constructor = {})[_wks('species')] = function (exec) {
      exec(empty, empty);
    };
    // unhandled rejections tracking support, NodeJS Promise without it fails @@species test
    return (isNode || typeof PromiseRejectionEvent == 'function') && promise.then(empty) instanceof FakePromise;
  } catch (e) { /* empty */ }
}();

// helpers
var sameConstructor = LIBRARY$1 ? function (a, b) {
  // with library wrapper special case
  return a === b || a === $Promise && b === Wrapper;
} : function (a, b) {
  return a === b;
};
var isThenable = function (it) {
  var then;
  return isObject$3(it) && typeof (then = it.then) == 'function' ? then : false;
};
var notify$1 = function (promise, isReject) {
  if (promise._n) { return; }
  promise._n = true;
  var chain = promise._c;
  microtask(function () {
    var value = promise._v;
    var ok = promise._s == 1;
    var i = 0;
    var run = function (reaction) {
      var handler = ok ? reaction.ok : reaction.fail;
      var resolve = reaction.resolve;
      var reject = reaction.reject;
      var domain = reaction.domain;
      var result, then;
      try {
        if (handler) {
          if (!ok) {
            if (promise._h == 2) { onHandleUnhandled(promise); }
            promise._h = 1;
          }
          if (handler === true) { result = value; }
          else {
            if (domain) { domain.enter(); }
            result = handler(value);
            if (domain) { domain.exit(); }
          }
          if (result === reaction.promise) {
            reject(TypeError$1('Promise-chain cycle'));
          } else if (then = isThenable(result)) {
            then.call(result, resolve, reject);
          } else { resolve(result); }
        } else { reject(value); }
      } catch (e) {
        reject(e);
      }
    };
    while (chain.length > i) { run(chain[i++]); } // variable length - can't use forEach
    promise._c = [];
    promise._n = false;
    if (isReject && !promise._h) { onUnhandled(promise); }
  });
};
var onUnhandled = function (promise) {
  task.call(global$4, function () {
    var value = promise._v;
    var unhandled = isUnhandled(promise);
    var result, handler, console;
    if (unhandled) {
      result = perform(function () {
        if (isNode) {
          process$1.emit('unhandledRejection', value, promise);
        } else if (handler = global$4.onunhandledrejection) {
          handler({ promise: promise, reason: value });
        } else if ((console = global$4.console) && console.error) {
          console.error('Unhandled promise rejection', value);
        }
      });
      // Browsers should not trigger `rejectionHandled` event if it was handled here, NodeJS - should
      promise._h = isNode || isUnhandled(promise) ? 2 : 1;
    } promise._a = undefined;
    if (unhandled && result.e) { throw result.v; }
  });
};
var isUnhandled = function (promise) {
  if (promise._h == 1) { return false; }
  var chain = promise._a || promise._c;
  var i = 0;
  var reaction;
  while (chain.length > i) {
    reaction = chain[i++];
    if (reaction.fail || !isUnhandled(reaction.promise)) { return false; }
  } return true;
};
var onHandleUnhandled = function (promise) {
  task.call(global$4, function () {
    var handler;
    if (isNode) {
      process$1.emit('rejectionHandled', promise);
    } else if (handler = global$4.onrejectionhandled) {
      handler({ promise: promise, reason: promise._v });
    }
  });
};
var $reject = function (value) {
  var promise = this;
  if (promise._d) { return; }
  promise._d = true;
  promise = promise._w || promise; // unwrap
  promise._v = value;
  promise._s = 2;
  if (!promise._a) { promise._a = promise._c.slice(); }
  notify$1(promise, true);
};
var $resolve = function (value) {
  var promise = this;
  var then;
  if (promise._d) { return; }
  promise._d = true;
  promise = promise._w || promise; // unwrap
  try {
    if (promise === value) { throw TypeError$1("Promise can't be resolved itself"); }
    if (then = isThenable(value)) {
      microtask(function () {
        var wrapper = { _w: promise, _d: false }; // wrap
        try {
          then.call(value, ctx$1($resolve, wrapper, 1), ctx$1($reject, wrapper, 1));
        } catch (e) {
          $reject.call(wrapper, e);
        }
      });
    } else {
      promise._v = value;
      promise._s = 1;
      notify$1(promise, false);
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
    aFunction$1(executor);
    Internal.call(this);
    try {
      executor(ctx$1($resolve, this, 1), ctx$1($reject, this, 1));
    } catch (err) {
      $reject.call(this, err);
    }
  };
  // eslint-disable-next-line no-unused-vars
  Internal = function Promise(executor) {
    this._c = [];             // <- awaiting reactions
    this._a = undefined;      // <- checked in isUnhandled reactions
    this._s = 0;              // <- state
    this._d = false;          // <- done
    this._v = undefined;      // <- value
    this._h = 0;              // <- rejection state, 0 - default, 1 - handled, 2 - unhandled
    this._n = false;          // <- notify
  };
  Internal.prototype = _redefineAll($Promise.prototype, {
    // 25.4.5.3 Promise.prototype.then(onFulfilled, onRejected)
    then: function then(onFulfilled, onRejected) {
      var reaction = newPromiseCapability(speciesConstructor(this, $Promise));
      reaction.ok = typeof onFulfilled == 'function' ? onFulfilled : true;
      reaction.fail = typeof onRejected == 'function' && onRejected;
      reaction.domain = isNode ? process$1.domain : undefined;
      this._c.push(reaction);
      if (this._a) { this._a.push(reaction); }
      if (this._s) { notify$1(this, false); }
      return reaction.promise;
    },
    // 25.4.5.1 Promise.prototype.catch(onRejected)
    'catch': function (onRejected) {
      return this.then(undefined, onRejected);
    }
  });
  OwnPromiseCapability = function () {
    var promise = new Internal();
    this.promise = promise;
    this.resolve = ctx$1($resolve, promise, 1);
    this.reject = ctx$1($reject, promise, 1);
  };
  newPromiseCapabilityModule.f = newPromiseCapability = function (C) {
    return sameConstructor($Promise, C)
      ? new OwnPromiseCapability(C)
      : newGenericPromiseCapability(C);
  };
}

$export$3($export$3.G + $export$3.W + $export$3.F * !USE_NATIVE, { Promise: $Promise });
_setToStringTag($Promise, PROMISE);
_setSpecies(PROMISE);
Wrapper = _core[PROMISE];

// statics
$export$3($export$3.S + $export$3.F * !USE_NATIVE, PROMISE, {
  // 25.4.4.5 Promise.reject(r)
  reject: function reject(r) {
    var capability = newPromiseCapability(this);
    var $$reject = capability.reject;
    $$reject(r);
    return capability.promise;
  }
});
$export$3($export$3.S + $export$3.F * (LIBRARY$1 || !USE_NATIVE), PROMISE, {
  // 25.4.4.6 Promise.resolve(x)
  resolve: function resolve(x) {
    // instanceof instead of internal slot check because we should fix it without replacement native Promise core
    if (x instanceof $Promise && sameConstructor(x.constructor, this)) { return x; }
    return promiseResolve(this, x);
  }
});
$export$3($export$3.S + $export$3.F * !(USE_NATIVE && _iterDetect(function (iter) {
  $Promise.all(iter)['catch'](empty);
})), PROMISE, {
  // 25.4.4.1 Promise.all(iterable)
  all: function all(iterable) {
    var C = this;
    var capability = newPromiseCapability(C);
    var resolve = capability.resolve;
    var reject = capability.reject;
    var result = perform(function () {
      var values = [];
      var index = 0;
      var remaining = 1;
      forOf(iterable, false, function (promise) {
        var $index = index++;
        var alreadyCalled = false;
        values.push(undefined);
        remaining++;
        C.resolve(promise).then(function (value) {
          if (alreadyCalled) { return; }
          alreadyCalled = true;
          values[$index] = value;
          --remaining || resolve(values);
        }, reject);
      });
      --remaining || resolve(values);
    });
    if (result.e) { reject(result.v); }
    return capability.promise;
  },
  // 25.4.4.4 Promise.race(iterable)
  race: function race(iterable) {
    var C = this;
    var capability = newPromiseCapability(C);
    var reject = capability.reject;
    var result = perform(function () {
      forOf(iterable, false, function (promise) {
        C.resolve(promise).then(capability.resolve, reject);
      });
    });
    if (result.e) { reject(result.v); }
    return capability.promise;
  }
});

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

/**
 * @fileOverview
 * This file will hack `console` methods by `WXEnvironment.logLevel`.
 * So we can control how many and which messages will be sent by change the log level.
 * Additionally in native platform the message content must be primitive values and
 * using `nativeLog(...args, logLevelMark)` so we create a new `console` object in
 * global add a format process for its methods.
 */

var LEVELS = ['off', 'error', 'warn', 'info', 'log', 'debug'];
var levelMap = {};

var originalConsole = global.console;

/**
 * Hack console for native environment.
 */
function setNativeConsole () {
  generateLevelMap();

  /* istanbul ignore next */
  // mock console in native environment
  if (global.WXEnvironment && global.WXEnvironment.platform !== 'Web') {
    global.console = {
      debug: function () {
        var args = [], len = arguments.length;
        while ( len-- ) args[ len ] = arguments[ len ];

        if (checkLevel('debug')) { global.nativeLog.apply(global, format(args).concat( ['__DEBUG'] )); }
      },
      log: function () {
        var args = [], len = arguments.length;
        while ( len-- ) args[ len ] = arguments[ len ];

        if (checkLevel('log')) { global.nativeLog.apply(global, format(args).concat( ['__LOG'] )); }
      },
      info: function () {
        var args = [], len = arguments.length;
        while ( len-- ) args[ len ] = arguments[ len ];

        if (checkLevel('info')) { global.nativeLog.apply(global, format(args).concat( ['__INFO'] )); }
      },
      warn: function () {
        var args = [], len = arguments.length;
        while ( len-- ) args[ len ] = arguments[ len ];

        if (checkLevel('warn')) { global.nativeLog.apply(global, format(args).concat( ['__WARN'] )); }
      },
      error: function () {
        var args = [], len = arguments.length;
        while ( len-- ) args[ len ] = arguments[ len ];

        if (checkLevel('error')) { global.nativeLog.apply(global, format(args).concat( ['__ERROR'] )); }
      }
    };
  }

  // Web or Node
  else {
    var debug = console.debug;
    var log = console.log;
    var info = console.info;
    var warn = console.warn;
    var error = console.error;
    console.__ori__ = { debug: debug, log: log, info: info, warn: warn, error: error };
    console.debug = function () {
      var args = [], len = arguments.length;
      while ( len-- ) args[ len ] = arguments[ len ];

      if (checkLevel('debug')) { console.__ori__.debug.apply(console, args); }
    };
    console.log = function () {
      var args = [], len = arguments.length;
      while ( len-- ) args[ len ] = arguments[ len ];

      if (checkLevel('log')) { console.__ori__.log.apply(console, args); }
    };
    console.info = function () {
      var args = [], len = arguments.length;
      while ( len-- ) args[ len ] = arguments[ len ];

      if (checkLevel('info')) { console.__ori__.info.apply(console, args); }
    };
    console.warn = function () {
      var args = [], len = arguments.length;
      while ( len-- ) args[ len ] = arguments[ len ];

      if (checkLevel('warn')) { console.__ori__.warn.apply(console, args); }
    };
    console.error = function () {
      var args = [], len = arguments.length;
      while ( len-- ) args[ len ] = arguments[ len ];

      if (checkLevel('error')) { console.__ori__.error.apply(console, args); }
    };
  }
}

/**
 * Reset hacked console to original.
 */
/* istanbul ignore next */
function resetNativeConsole () {
  levelMap = {};
  global.console = originalConsole;
}

/**
 * Generate map for which types of message will be sent in a certain message level
 * as the order of LEVELS.
 */
function generateLevelMap () {
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
function checkLevel (type) {
  var logLevel = (global.WXEnvironment && global.WXEnvironment.logLevel) || 'log';
  return levelMap[logLevel] && levelMap[logLevel][type]
}

/**
 * Convert all log arguments into primitive values.
 * @param  {array} args
 * @return {array}
 */
/* istanbul ignore next */
function format (args) {
  return args.map(function (v) {
    var type = Object.prototype.toString.call(v);
    if (type.toLowerCase() === '[object object]') {
      v = JSON.stringify(v);
    }
    else {
      v = String(v);
    }
    return v
  })
}

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

/**
 * @fileOverview
 * Polyfill `setTimeout` on Android V8 using native method
 * `setTimeoutNative(callbackId, time)` and JS method
 * `setTimeoutCallback(callbackId)`.
 * This polyfill is only used in virtual-DOM diff & flush agorithm. Not
 * accessed by JS Bundle code (The timer APIs polyfill for JS Bundle is in
 * `html5/default/app/ctrl.js`).
 */

var originalSetTimeout = global.setTimeout;
var setTimeoutNative = global.setTimeoutNative;

/**
 * Set up native timer
 */
/* istanbul ignore next */
function setNativeTimer () {
  if (typeof setTimeout === 'undefined' &&
  typeof setTimeoutNative === 'function') {
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
  }
}

/* istanbul ignore next */
function resetNativeTimer () {
  global.setTimeout = originalSetTimeout;
  global.setTimeoutCallback = null;
}

setNativeTimer();

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
/**
 * Freeze the prototype of javascript build-in objects.
 */
/* istanbul ignore next */
function freezePrototype$1 () {
  Object.freeze(Object);
  Object.freeze(Array);

  // Object.freeze(Object.prototype)
  freezeObjectProto();
  Object.freeze(Array.prototype);
  Object.freeze(String.prototype);
  Object.freeze(Number.prototype);
  Object.freeze(Boolean.prototype);

  // Object.freeze(Error.prototype)
  freezeErrorProto();
  Object.freeze(Date.prototype);
  Object.freeze(RegExp.prototype);
}

function freezeObjectProto () {
  var proto = Object.prototype;
  var protoName = 'Object.prototype';
  freezeProtoProperty(proto, '__defineGetter__', protoName);
  freezeProtoProperty(proto, '__defineSetter__', protoName);
  freezeProtoProperty(proto, '__lookupGetter__', protoName);
  freezeProtoProperty(proto, '__lookupSetter__', protoName);
  freezeProtoProperty(proto, 'constructor', protoName);
  freezeProtoProperty(proto, 'hasOwnProperty', protoName);
  freezeProtoProperty(proto, 'isPrototypeOf', protoName);
  freezeProtoProperty(proto, 'propertyIsEnumerable', protoName);
  freezeProtoProperty(proto, 'toLocaleString', protoName);
  freezeProtoProperty(proto, 'toString', protoName);
  freezeProtoProperty(proto, 'valueOf', protoName);
  Object.seal(proto);
}

function freezeErrorProto () {
  var proto = Error.prototype;
  var protoName = 'Error.prototype';
  freezeProtoProperty(proto, 'name', protoName);
  freezeProtoProperty(proto, 'message', protoName);
  freezeProtoProperty(proto, 'toString', protoName);
  freezeProtoProperty(proto, 'constructor', protoName);
  Object.seal(proto);
}

function freezeProtoProperty (proto, propertyName, protoName) {
  if (!proto.hasOwnProperty(propertyName)) {
    return
  }

  var origin = proto[propertyName];
  Object.defineProperty(proto, propertyName, {
    get: function () {
      return origin
    },
    set: function (value) {
      if (this === proto) {
        throw Error(("Cannot assign to read only property " + propertyName + " of " + protoName))
      }

      Object.defineProperty(this, propertyName, {
        value: value,
        writable: true
      });

      return value
    }
  });
}

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
// import promise hack and polyfills

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

// Get a unique id.
var uniqueId = (function () {
  var nextNodeRef = 1;
  return function () { return String(nextNodeRef++); }
})();

function typof (v) {
  var type = Object.prototype.toString.call(v);
  return type.substring(8, type.length - 1)
}

function bufferToBase64 (buffer) {
  if (typeof btoa !== 'function') {
    return ''
  }
  var string = Array.prototype.map.call(
    new Uint8Array(buffer),
    function (code) { return String.fromCharCode(code); }
  ).join('');
  return btoa(string) // eslint-disable-line no-undef
}

function base64ToBuffer (base64) {
  if (typeof atob !== 'function') {
    return new ArrayBuffer(0)
  }
  var string = atob(base64); // eslint-disable-line no-undef
  var array = new Uint8Array(string.length);
  Array.prototype.forEach.call(string, function (ch, i) {
    array[i] = ch.charCodeAt(0);
  });
  return array.buffer
}

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

/**
 * Normalize a primitive value.
 * @param  {any}        v
 * @return {primitive}
 */
function normalizePrimitive (v) {
  var type = typof(v);

  switch (type) {
    case 'Undefined':
    case 'Null':
      return ''

    case 'RegExp':
      return v.toString()
    case 'Date':
      return v.toISOString()

    case 'Number':
    case 'String':
    case 'Boolean':
    case 'Array':
    case 'Object':
      return v

    case 'ArrayBuffer':
      return {
        '@type': 'binary',
        dataType: type,
        base64: bufferToBase64(v)
      }

    case 'Int8Array':
    case 'Uint8Array':
    case 'Uint8ClampedArray':
    case 'Int16Array':
    case 'Uint16Array':
    case 'Int32Array':
    case 'Uint32Array':
    case 'Float32Array':
    case 'Float64Array':
      return {
        '@type': 'binary',
        dataType: type,
        base64: bufferToBase64(v.buffer)
      }

    default:
      return JSON.stringify(v)
  }
}

function decodePrimitive (data) {
  if (typof(data) === 'Object') {
    // decode base64 into binary
    if (data['@type'] && data['@type'] === 'binary') {
      return base64ToBuffer(data.base64 || '')
    }

    var realData = {};
    for (var key in data) {
      realData[key] = decodePrimitive(data[key]);
    }
    return realData
  }
  if (typof(data) === 'Array') {
    return data.map(decodePrimitive)
  }
  return data
}

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

/**
 * For general callback management of a certain Weex instance.
 * Because function can not passed into native, so we create callback
 * callback id for each function and pass the callback id into native
 * in fact. And when a callback called from native, we can find the real
 * callback through the callback id we have passed before.
 */
var CallbackManager = function CallbackManager (instanceId) {
  this.instanceId = instanceId;
  this.lastCallbackId = 0;
  this.callbacks = {};
};
CallbackManager.prototype.add = function add (callback) {
  this.lastCallbackId++;
  this.callbacks[this.lastCallbackId] = callback;
  return this.lastCallbackId
};
CallbackManager.prototype.remove = function remove (callbackId) {
  var callback = this.callbacks[callbackId];
  delete this.callbacks[callbackId];
  return callback
};
CallbackManager.prototype.consume = function consume (callbackId, data, ifKeepAlive) {
  var callback = this.callbacks[callbackId];
  if (typeof ifKeepAlive === 'undefined' || ifKeepAlive === false) {
    delete this.callbacks[callbackId];
  }
  if (typeof callback === 'function') {
    return callback(decodePrimitive(data))
  }
  return new Error(("invalid callback id \"" + callbackId + "\""))
};
CallbackManager.prototype.close = function close () {
  this.callbacks = {};
};

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

var docMap = {};

/**
 * Add a document object into docMap.
 * @param {string} id
 * @param {object} document
 */
function addDoc (id, doc) {
  if (id) {
    docMap[id] = doc;
  }
}

/**
 * Get the document object by id.
 * @param {string} id
 */
function getDoc (id) {
  return docMap[id]
}

/**
 * Remove the document from docMap by id.
 * @param {string} id
 */
function removeDoc (id) {
  delete docMap[id];
}

/**
 * @deprecated
 * Get listener by document id.
 * @param {string} id
 * @return {object} listener
 */


/**
 * Get TaskCenter instance by id.
 * @param {string} id
 * @return {object} TaskCenter
 */
function getTaskCenter (id) {
  var doc = docMap[id];
  if (doc && doc.taskCenter) {
    return doc.taskCenter
  }
  return null
}

/**
 * Append body node to documentElement.
 * @param {object} document
 * @param {object} node
 * @param {object} before
 */
function appendBody (doc, node, before) {
  var documentElement = doc.documentElement;

  if (documentElement.pureChildren.length > 0 || node.parentNode) {
    return
  }
  var children = documentElement.children;
  var beforeIndex = children.indexOf(before);
  if (beforeIndex < 0) {
    children.push(node);
  }
  else {
    children.splice(beforeIndex, 0, node);
  }

  if (node.nodeType === 1) {
    if (node.role === 'body') {
      node.docId = doc.id;
      node.ownerDocument = doc;
      node.parentNode = documentElement;
      linkParent(node, documentElement);
    }
    else {
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
    sendBody(doc, node);
  }
  else {
    node.parentNode = documentElement;
    doc.nodeMap[node.ref] = node;
  }
}

function sendBody (doc, node) {
  var body = node.toJSON();
  var children = body.children;
  delete body.children;
  var result = doc.taskCenter.send('dom', { action: 'createBody' }, [body]);
  if (children) {
    children.forEach(function (child) {
      result = doc.taskCenter.send('dom', { action: 'addElement' }, [body.ref, child, -1]);
    });
  }
  return result
}

/**
 * Set up body node.
 * @param {object} document
 * @param {object} element
 */
function setBody (doc, el) {
  el.role = 'body';
  el.depth = 1;
  delete doc.nodeMap[el.nodeId];
  el.ref = '_root';
  doc.nodeMap._root = el;
  doc.body = el;
}

/**
 * Establish the connection between parent and child node.
 * @param {object} child node
 * @param {object} parent node
 */
function linkParent (node, parent) {
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

/**
 * Get the next sibling element.
 * @param {object} node
 */
function nextElement (node) {
  while (node) {
    if (node.nodeType === 1) {
      return node
    }
    node = node.nextSibling;
  }
}

/**
 * Get the previous sibling element.
 * @param {object} node
 */
function previousElement (node) {
  while (node) {
    if (node.nodeType === 1) {
      return node
    }
    node = node.previousSibling;
  }
}

/**
 * Insert a node into list at the specified index.
 * @param {object} target node
 * @param {array} list
 * @param {number} newIndex
 * @param {boolean} changeSibling
 * @return {number} newIndex
 */
function insertIndex (target, list, newIndex, changeSibling) {
  /* istanbul ignore next */
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
  return newIndex
}

/**
 * Move the node to a new index in list.
 * @param {object} target node
 * @param {array} list
 * @param {number} newIndex
 * @param {boolean} changeSibling
 * @return {number} newIndex
 */
function moveIndex (target, list, newIndex, changeSibling) {
  var index = list.indexOf(target);
  /* istanbul ignore next */
  if (index < 0) {
    return -1
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
    return -1
  }
  return newIndex
}

/**
 * Remove the node from list.
 * @param {object} target node
 * @param {array} list
 * @param {boolean} changeSibling
 */
function removeIndex (target, list, changeSibling) {
  var index = list.indexOf(target);
  /* istanbul ignore next */
  if (index < 0) {
    return
  }
  if (changeSibling) {
    var before = list[index - 1];
    var after = list[index + 1];
    before && (before.nextSibling = after);
    after && (after.previousSibling = before);
  }
  list.splice(index, 1);
}

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

var Node = function Node () {
  this.nodeId = uniqueId();
  this.ref = this.nodeId;
  this.children = [];
  this.pureChildren = [];
  this.parentNode = null;
  this.nextSibling = null;
  this.previousSibling = null;
};

/**
* Destroy current node, and remove itself form nodeMap.
*/
Node.prototype.destroy = function destroy () {
  var doc = getDoc(this.docId);
  if (doc) {
    delete this.docId;
    delete doc.nodeMap[this.nodeId];
  }
  this.children.forEach(function (child) {
    child.destroy();
  });
};

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
var Element$2; // TODO: import Element

// deprecated
function setElement (El) {
  Element$2 = El;
}

/**
 * A map which stores all type of elements.
 * @type {Object}
 */
var registeredElements = {};

/**
 * Register an extended element type with component methods.
 * @param  {string} type    component type
 * @param  {array}  methods a list of method names
 */
function registerElement (type, methods) {
  // Skip when no special component methods.
  if (!methods || !methods.length) {
    return
  }

  // Init constructor.
  var WeexElement = (function (Element) {
    function WeexElement () {
      Element.apply(this, arguments);
    }if ( Element ) WeexElement.__proto__ = Element;
    WeexElement.prototype = Object.create( Element && Element.prototype );
    WeexElement.prototype.constructor = WeexElement;

    

    return WeexElement;
  }(Element$2));

  // Add methods to prototype.
  methods.forEach(function (methodName) {
    WeexElement.prototype[methodName] = function () {
      var args = [], len = arguments.length;
      while ( len-- ) args[ len ] = arguments[ len ];

      var taskCenter = getTaskCenter(this.docId);
      if (taskCenter) {
        return taskCenter.send('component', {
          ref: this.ref,
          component: type,
          method: methodName
        }, args)
      }
    };
  });

  // Add to element type map.
  registeredElements[type] = WeexElement;
}



function getWeexElement (type) {
  return registeredElements[type]
}



/**
 * Clear all element types. Only for testing.
 */

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

var DEFAULT_TAG_NAME = 'div';
var BUBBLE_EVENTS = [
  'click', 'longpress', 'touchstart', 'touchmove', 'touchend',
  'panstart', 'panmove', 'panend', 'horizontalpan', 'verticalpan', 'swipe'
];

function registerNode (docId, node) {
  var doc = getDoc(docId);
  doc.nodeMap[node.nodeId] = node;
}

var Element = (function (Node$$1) {
  function Element (type, props, isExtended) {
    Node$$1.call(this);

    // TODO: deprecated
    var WeexElement = getWeexElement(type);
    if (WeexElement && !isExtended) {
      return new WeexElement(type, props, true)
    }

    props = props || {};
    this.nodeType = 1;
    this.nodeId = uniqueId();
    this.ref = this.nodeId;
    this.type = type || DEFAULT_TAG_NAME;
    this.attr = props.attr || {};
    this.style = props.style || {};
    this.classStyle = props.classStyle || {};
    this.event = {};
    this.children = [];
    this.pureChildren = [];
  }

  if ( Node$$1 ) Element.__proto__ = Node$$1;
  Element.prototype = Object.create( Node$$1 && Node$$1.prototype );
  Element.prototype.constructor = Element;

  /**
   * Append a child node.
   * @param {object} node
   * @return {undefined | number} the signal sent by native
   */
  Element.prototype.appendChild = function appendChild (node) {
    if (node.parentNode && node.parentNode !== this) {
      return
    }
    /* istanbul ignore else */
    if (!node.parentNode) {
      linkParent(node, this);
      insertIndex(node, this.children, this.children.length, true);
      if (this.docId) {
        registerNode(this.docId, node);
      }
      if (node.nodeType === 1) {
        insertIndex(node, this.pureChildren, this.pureChildren.length);
        var taskCenter = getTaskCenter(this.docId);
        if (taskCenter) {
          return taskCenter.send(
            'dom',
            { action: 'addElement' },
            [this.ref, node.toJSON(), -1]
          )
        }
      }
    }
    else {
      moveIndex(node, this.children, this.children.length, true);
      if (node.nodeType === 1) {
        var index = moveIndex(node, this.pureChildren, this.pureChildren.length);
        var taskCenter$1 = getTaskCenter(this.docId);
        if (taskCenter$1 && index >= 0) {
          return taskCenter$1.send(
            'dom',
            { action: 'moveElement' },
            [node.ref, this.ref, index]
          )
        }
      }
    }
  };

  /**
   * Insert a node before specified node.
   * @param {object} node
   * @param {object} before
   * @return {undefined | number} the signal sent by native
   */
  Element.prototype.insertBefore = function insertBefore (node, before) {
    if (node.parentNode && node.parentNode !== this) {
      return
    }
    if (node === before || (node.nextSibling && node.nextSibling === before)) {
      return
    }
    if (!node.parentNode) {
      linkParent(node, this);
      insertIndex(node, this.children, this.children.indexOf(before), true);
      if (this.docId) {
        registerNode(this.docId, node);
      }
      if (node.nodeType === 1) {
        var pureBefore = nextElement(before);
        var index = insertIndex(
          node,
          this.pureChildren,
          pureBefore
          ? this.pureChildren.indexOf(pureBefore)
          : this.pureChildren.length
        );
        var taskCenter = getTaskCenter(this.docId);
        if (taskCenter) {
          return taskCenter.send(
            'dom',
            { action: 'addElement' },
            [this.ref, node.toJSON(), index]
          )
        }
      }
    }
    else {
      moveIndex(node, this.children, this.children.indexOf(before), true);
      if (node.nodeType === 1) {
        var pureBefore$1 = nextElement(before);
        /* istanbul ignore next */
        var index$1 = moveIndex(
          node,
          this.pureChildren,
          pureBefore$1
          ? this.pureChildren.indexOf(pureBefore$1)
          : this.pureChildren.length
        );
        var taskCenter$1 = getTaskCenter(this.docId);
        if (taskCenter$1 && index$1 >= 0) {
          return taskCenter$1.send(
            'dom',
            { action: 'moveElement' },
            [node.ref, this.ref, index$1]
          )
        }
      }
    }
  };

  /**
   * Insert a node after specified node.
   * @param {object} node
   * @param {object} after
   * @return {undefined | number} the signal sent by native
   */
  Element.prototype.insertAfter = function insertAfter (node, after) {
    if (node.parentNode && node.parentNode !== this) {
      return
    }
    if (node === after || (node.previousSibling && node.previousSibling === after)) {
      return
    }
    if (!node.parentNode) {
      linkParent(node, this);
      insertIndex(node, this.children, this.children.indexOf(after) + 1, true);
      /* istanbul ignore else */
      if (this.docId) {
        registerNode(this.docId, node);
      }
      if (node.nodeType === 1) {
        var index = insertIndex(
          node,
          this.pureChildren,
          this.pureChildren.indexOf(previousElement(after)) + 1
        );
        var taskCenter = getTaskCenter(this.docId);
        /* istanbul ignore else */
        if (taskCenter) {
          return taskCenter.send(
            'dom',
            { action: 'addElement' },
            [this.ref, node.toJSON(), index]
          )
        }
      }
    }
    else {
      moveIndex(node, this.children, this.children.indexOf(after) + 1, true);
      if (node.nodeType === 1) {
        var index$1 = moveIndex(
          node,
          this.pureChildren,
          this.pureChildren.indexOf(previousElement(after)) + 1
        );
        var taskCenter$1 = getTaskCenter(this.docId);
        if (taskCenter$1 && index$1 >= 0) {
          return taskCenter$1.send(
            'dom',
            { action: 'moveElement' },
            [node.ref, this.ref, index$1]
          )
        }
      }
    }
  };

  /**
   * Remove a child node, and decide whether it should be destroyed.
   * @param {object} node
   * @param {boolean} preserved
   */
  Element.prototype.removeChild = function removeChild (node, preserved) {
    if (node.parentNode) {
      removeIndex(node, this.children, true);
      if (node.nodeType === 1) {
        removeIndex(node, this.pureChildren);
        var taskCenter = getTaskCenter(this.docId);
        if (taskCenter) {
          taskCenter.send(
            'dom',
            { action: 'removeElement' },
            [node.ref]
          );
        }
      }
    }
    if (!preserved) {
      node.destroy();
    }
  };

  /**
   * Clear all child nodes.
   */
  Element.prototype.clear = function clear () {
    var taskCenter = getTaskCenter(this.docId);
    /* istanbul ignore else */
    if (taskCenter) {
      this.pureChildren.forEach(function (node) {
        taskCenter.send(
          'dom',
          { action: 'removeElement' },
          [node.ref]
        );
      });
    }
    this.children.forEach(function (node) {
      node.destroy();
    });
    this.children.length = 0;
    this.pureChildren.length = 0;
  };

  /**
   * Set an attribute, and decide whether the task should be send to native.
   * @param {string} key
   * @param {string | number} value
   * @param {boolean} silent
   */
  Element.prototype.setAttr = function setAttr (key, value, silent) {
    if (!this.attr || this.attr[key] === value && silent !== false) {
      return
    }
    this.attr[key] = value;
    var taskCenter = getTaskCenter(this.docId);
    if (!silent && taskCenter) {
      var result = {};
      result[key] = value;
      taskCenter.send(
        'dom',
        { action: 'updateAttrs' },
        [this.ref, result]
      );
    }
  };

  /**
   * Set a style property, and decide whether the task should be send to native.
   * @param {string} key
   * @param {string | number} value
   * @param {boolean} silent
   */
  Element.prototype.setStyle = function setStyle (key, value, silent) {
    if (this.style[key] === value && silent !== false) {
      return
    }
    this.style[key] = value;
    var taskCenter = getTaskCenter(this.docId);
    if (!silent && taskCenter) {
      var result = {};
      result[key] = value;
      taskCenter.send(
        'dom',
        { action: 'updateStyle' },
        [this.ref, result]
      );
    }
  };

  /**
   * Set style properties from class.
   * @param {object} classStyle
   */
  Element.prototype.setClassStyle = function setClassStyle (classStyle) {
    var this$1 = this;

    if (!classStyle) {
      return
    }

    // reset previous class style to empty string
    for (var key in this.classStyle) {
      this$1.classStyle[key] = '';
    }

    Object.assign(this.classStyle, classStyle);
    var taskCenter = getTaskCenter(this.docId);
    if (taskCenter) {
      taskCenter.send(
        'dom',
        { action: 'updateStyle' },
        [this.ref, this.toStyle()]
      );
    }
  };

  /**
   * Add an event handler.
   * @param {string} type event type
   * @param {function} handler event handler
   * @param {object} params event handler parameters
   */
  Element.prototype.addEvent = function addEvent (type, handler, params) {
    if (!this.event[type]) {
      this.event[type] = { handler: handler, params: params };
      var taskCenter = getTaskCenter(this.docId);
      if (taskCenter) {
        taskCenter.send(
          'dom',
          { action: 'addEvent' },
          [this.ref, type]
        );
      }
    }
  };

  /**
   * Remove an event handler.
   * @param {string} event type
   */
  Element.prototype.removeEvent = function removeEvent (type) {
    if (this.event[type]) {
      delete this.event[type];
      var taskCenter = getTaskCenter(this.docId);
      if (taskCenter) {
        taskCenter.send(
          'dom',
          { action: 'removeEvent' },
          [this.ref, type]
        );
      }
    }
  };

  /**
   * Fire an event manually.
   * @param {string} type type
   * @param {function} e handler
   * @param {boolean} isBubble whether or not event bubble
   * @param {object} options additional options
   * @return {} anything returned by handler function
   */
  Element.prototype.fireEvent = function fireEvent (type, e, isBubble, options) {
    var result = null;
    var isStopPropagation = false;
    var eventDesc = this.event[type];
    if (eventDesc && e) {
      var handler = eventDesc.handler;
      e.stopPropagation = function () {
        isStopPropagation = true;
      };
      if (options && options.params) {
        result = handler.call.apply(handler, [ this ].concat( options.params, [e] ));
      }
      else {
        result = handler.call(this, e);
      }
    }

    // TODO: bubble in next tick
    if (!isStopPropagation
      && isBubble
      && BUBBLE_EVENTS.includes(type)
      && this.parentNode
      && this.parentNode.fireEvent) {
      e.currentTarget = this.parentNode;
      this.parentNode.fireEvent(type, e, isBubble);
    }

    return result
  };

  /**
   * Get all styles of current element.
   * @return {object} style
   */
  Element.prototype.toStyle = function toStyle () {
    return Object.assign({}, this.classStyle, this.style)
  };

  /**
   * Convert current element to JSON like object.
   * @return {object} element
   */
  Element.prototype.toJSON = function toJSON () {
    var this$1 = this;

    var result = {
      ref: this.ref.toString(),
      type: this.type,
      attr: this.attr,
      style: this.toStyle()
    };
    var event = [];
    for (var eventType in this.event) {
      var ref = this$1.event[eventType];
      var params = ref.params;
      if (!params) {
        event.push(eventType);
      }
      else {
        event.push({ type: eventType, params: params });
      }
    }
    if (event.length) {
      result.event = event;
    }
    if (this.pureChildren.length) {
      result.children = this.pureChildren.map(function (child) { return child.toJSON(); });
    }
    return result
  };

  /**
   * Convert to HTML element tag string.
   * @return {stirng} html
   */
  Element.prototype.toString = function toString () {
    return '<' + this.type +
    ' attr=' + JSON.stringify(this.attr) +
    ' style=' + JSON.stringify(this.toStyle()) + '>' +
    this.pureChildren.map(function (child) { return child.toString(); }).join('') +
    '</' + this.type + '>'
  };

  return Element;
}(Node));

setElement(Element);

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

var fallback = function () {};

// The API of TaskCenter would be re-design.
var TaskCenter = function TaskCenter (id, sendTasks) {
  Object.defineProperty(this, 'instanceId', {
    enumerable: true,
    value: id
  });
  Object.defineProperty(this, 'callbackManager', {
    enumerable: true,
    value: new CallbackManager(id)
  });
  fallback = sendTasks || function () {};
};

TaskCenter.prototype.callback = function callback (callbackId, data, ifKeepAlive) {
  return this.callbackManager.consume(callbackId, data, ifKeepAlive)
};

TaskCenter.prototype.destroyCallback = function destroyCallback () {
  return this.callbackManager.close()
};

/**
 * Normalize a value. Specially, if the value is a function, then generate a function id
 * and save it to `CallbackManager`, at last return the function id.
 * @param{any}      v
 * @return {primitive}
 */
TaskCenter.prototype.normalize = function normalize (v) {
  var type = typof(v);
  if (v && v instanceof Element) {
    return v.ref
  }
  if (v && v._isVue && v.$el instanceof Element) {
    return v.$el.ref
  }
  if (type === 'Function') {
    return this.callbackManager.add(v).toString()
  }
  return normalizePrimitive(v)
};

TaskCenter.prototype.send = function send (type, params, args, options) {
    var this$1 = this;

  var action = params.action;
    var component = params.component;
    var ref = params.ref;
    var module = params.module;
    var method = params.method;

  args = args.map(function (arg) { return this$1.normalize(arg); });

  switch (type) {
    case 'dom':
      return this[action](this.instanceId, args)
    case 'component':
      return this.componentHandler(this.instanceId, ref, method, args, Object.assign({ component: component }, options))
    default:
      return this.moduleHandler(this.instanceId, module, method, args, options)
  }
};

TaskCenter.prototype.callDOM = function callDOM (action, args) {
  return this[action](this.instanceId, args)
};

TaskCenter.prototype.callComponent = function callComponent (ref, method, args, options) {
  return this.componentHandler(this.instanceId, ref, method, args, options)
};

TaskCenter.prototype.callModule = function callModule (module, method, args, options) {
  return this.moduleHandler(this.instanceId, module, method, args, options)
};

function init$2 () {
  var DOM_METHODS = {
    createFinish: global.callCreateFinish,
    updateFinish: global.callUpdateFinish,
    refreshFinish: global.callRefreshFinish,

    createBody: global.callCreateBody,

    addElement: global.callAddElement,
    removeElement: global.callRemoveElement,
    moveElement: global.callMoveElement,
    updateAttrs: global.callUpdateAttrs,
    updateStyle: global.callUpdateStyle,

    addEvent: global.callAddEvent,
    removeEvent: global.callRemoveEvent
  };
  var proto = TaskCenter.prototype;

  var loop = function ( name ) {
    var method = DOM_METHODS[name];
    proto[name] = method ?
      function (id, args) { return method.apply(void 0, [ id ].concat( args )); } :
      function (id, args) { return fallback(id, [{ module: 'dom', method: name, args: args }], '-1'); };
  };

  for (var name in DOM_METHODS) loop( name );

  proto.componentHandler = global.callNativeComponent ||
    (function (id, ref, method, args, options) { return fallback(id, [{ component: options.component, ref: ref, method: method, args: args }]); });

  proto.moduleHandler = global.callNativeModule ||
    (function (id, module, method, args) { return fallback(id, [{ module: module, method: method, args: args }]); });
}

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

// JS Services
var services = [];

/**
 * Register a JavaScript service.
 * A JavaScript service options could have a set of lifecycle methods
 * for each Weex instance. For example: create, refresh, destroy.
 * For the JS framework maintainer if you want to supply some features
 * which need to work well in different Weex instances, even in different
 * frameworks separately. You can make a JavaScript service to init
 * its variables or classes for each Weex instance when it's created
 * and recycle them when it's destroyed.
 * @param {object} options Could have { create, refresh, destroy }
 *                         lifecycle methods. In create method it should
 *                         return an object of what variables or classes
 *                         would be injected into the Weex instance.
 */
function registerService (name, options) {
  if (hasService(name)) {
    console.warn(("Service \"" + name + "\" has been registered already!"));
  }
  else {
    options = Object.assign({}, options);
    services.push({ name: name, options: options });
  }
}

/**
 * Unregister a JavaScript service by name
 * @param {string} name
 */
function unregisterService (name) {
  services.some(function (service, index) {
    if (service.name === name) {
      services.splice(index, 1);
      return true
    }
  });
}

/**
 * Check if a JavaScript service with a certain name existed.
 * @param  {string}  name
 * @return {Boolean}
 */
function hasService (name) {
  return services.map(function (service) { return service.name; }).indexOf(name) >= 0
}

/**
 * Generate service map
 */
function createServices (id, env, config) {
  // Init JavaScript services for this instance.
  var serviceMap = Object.create(null);
  serviceMap.service = Object.create(null);
  services.forEach(function (ref) {
    var name = ref.name;
    var options = ref.options;

    {
      console.debug(("[JS Runtime] create service " + name + "."));
    }
    var create = options.create;
    if (create) {
      var result = create(id, env, config);
      Object.assign(serviceMap.service, result);
      Object.assign(serviceMap, result.instance);
    }
  });
  delete serviceMap.service.instance;
  Object.freeze(serviceMap.service);
  return serviceMap
}

function refreshServices (id, env, config) {
  services.forEach(function (service) {
    var refresh = service.options.refresh;
    if (typeof refresh === 'function') {
      refresh(id, env, config);
    }
  });
}

function destroyServices (id, env, config) {
  services.forEach(function (service) {
    var destroy = service.options.destroy;
    if (typeof destroy === 'function') {
      destroy(id, env, config);
    }
  });
}

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

var weexModules = {};

/**
 * Register native modules information.
 * @param {object} newModules
 */
function registerModules (newModules) {
  var loop = function ( name ) {
    if (!weexModules[name]) {
      weexModules[name] = {};
    }
    newModules[name].forEach(function (method) {
      if (typeof method === 'string') {
        weexModules[name][method] = true;
      }
      else {
        weexModules[name][method.name] = method.args;
      }
    });
  };

  for (var name in newModules) loop( name );
}

/**
 * Check whether the module or the method has been registered.
 * @param {String} module name
 * @param {String} method name (optional)
 */
function isRegisteredModule (name, method) {
  if (typeof method === 'string') {
    return !!(weexModules[name] && weexModules[name][method])
  }
  return !!weexModules[name]
}

function getModuleDescription (name) {
  return weexModules[name]
}

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

var weexComponents = {};

/**
 * Register native components information.
 * @param {array} newComponents
 */
function registerComponents (newComponents) {
  if (Array.isArray(newComponents)) {
    newComponents.forEach(function (component) {
      if (!component) {
        return
      }
      if (typeof component === 'string') {
        weexComponents[component] = true;
      }
      else if (typeof component === 'object' && typeof component.type === 'string') {
        weexComponents[component.type] = component;
        registerElement(component.type, component.methods);
      }
    });
  }
}

/**
 * Check whether the component has been registered.
 * @param {String} component name
 */
function isRegisteredComponent (name) {
  return !!weexComponents[name]
}

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

var Comment = (function (Node$$1) {
  function Comment (value) {
    Node$$1.call(this);

    this.nodeType = 8;
    this.nodeId = uniqueId();
    this.ref = this.nodeId;
    this.type = 'comment';
    this.value = value;
    this.children = [];
    this.pureChildren = [];
  }

  if ( Node$$1 ) Comment.__proto__ = Node$$1;
  Comment.prototype = Object.create( Node$$1 && Node$$1.prototype );
  Comment.prototype.constructor = Comment;

  /**
  * Convert to HTML comment string.
  * @return {stirng} html
  */
  Comment.prototype.toString = function toString () {
    return '<!-- ' + this.value + ' -->'
  };

  return Comment;
}(Node));

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

/**
* Create the action object.
* @param {string} name
* @param {array} arguments
* @return {object} action
*/
function createAction (name, args) {
  if ( args === void 0 ) args = [];

  return { module: 'dom', method: name, args: args }
}

var Listener = function Listener (id, handler) {
  this.id = id;
  this.batched = false;
  this.updates = [];
  if (typeof handler === 'function') {
    Object.defineProperty(this, 'handler', {
      configurable: true,
      enumerable: true,
      writable: true,
      value: handler
    });
  }
  else {
    console.error('[JS Runtime] invalid parameter, handler must be a function');
  }
};

/**
 * Send the "createFinish" signal.
 * @param {function} callback
 * @return {undefined | number} the signal sent by native
 */
Listener.prototype.createFinish = function createFinish (callback) {
  var handler = this.handler;
  return handler([createAction('createFinish')], callback)
};

/**
 * Send the "updateFinish" signal.
 * @param {function} callback
 * @return {undefined | number} the signal sent by native
 */
Listener.prototype.updateFinish = function updateFinish (callback) {
  var handler = this.handler;
  return handler([createAction('updateFinish')], callback)
};

/**
 * Send the "refreshFinish" signal.
 * @param {function} callback
 * @return {undefined | number} the signal sent by native
 */
Listener.prototype.refreshFinish = function refreshFinish (callback) {
  var handler = this.handler;
  return handler([createAction('refreshFinish')], callback)
};

/**
 * Send the "createBody" signal.
 * @param {object} element
 * @return {undefined | number} the signal sent by native
 */
Listener.prototype.createBody = function createBody (element) {
  var body = element.toJSON();
  var children = body.children;
  delete body.children;
  var actions = [createAction('createBody', [body])];
  if (children) {
    actions.push.apply(actions, children.map(function (child) {
      return createAction('addElement', [body.ref, child, -1])
    }));
  }
  return this.addActions(actions)
};

/**
 * Send the "addElement" signal.
 * @param {object} element
 * @param {stirng} reference id
 * @param {number} index
 * @return {undefined | number} the signal sent by native
 */
Listener.prototype.addElement = function addElement (element, ref, index) {
  if (!(index >= 0)) {
    index = -1;
  }
  return this.addActions(createAction('addElement', [ref, element.toJSON(), index]))
};

/**
 * Send the "removeElement" signal.
 * @param {stirng} reference id
 * @return {undefined | number} the signal sent by native
 */
Listener.prototype.removeElement = function removeElement (ref) {
  if (Array.isArray(ref)) {
    var actions = ref.map(function (r) { return createAction('removeElement', [r]); });
    return this.addActions(actions)
  }
  return this.addActions(createAction('removeElement', [ref]))
};

/**
 * Send the "moveElement" signal.
 * @param {stirng} target reference id
 * @param {stirng} parent reference id
 * @param {number} index
 * @return {undefined | number} the signal sent by native
 */
Listener.prototype.moveElement = function moveElement (targetRef, parentRef, index) {
  return this.addActions(createAction('moveElement', [targetRef, parentRef, index]))
};

/**
 * Send the "updateAttrs" signal.
 * @param {stirng} reference id
 * @param {stirng} key
 * @param {stirng} value
 * @return {undefined | number} the signal sent by native
 */
Listener.prototype.setAttr = function setAttr (ref, key, value) {
  var result = {};
  result[key] = value;
  return this.addActions(createAction('updateAttrs', [ref, result]))
};

/**
 * Send the "updateStyle" signal, update a sole style.
 * @param {stirng} reference id
 * @param {stirng} key
 * @param {stirng} value
 * @return {undefined | number} the signal sent by native
 */
Listener.prototype.setStyle = function setStyle (ref, key, value) {
  var result = {};
  result[key] = value;
  return this.addActions(createAction('updateStyle', [ref, result]))
};

/**
 * Send the "updateStyle" signal.
 * @param {stirng} reference id
 * @param {object} style
 * @return {undefined | number} the signal sent by native
 */
Listener.prototype.setStyles = function setStyles (ref, style) {
  return this.addActions(createAction('updateStyle', [ref, style]))
};

/**
 * Send the "addEvent" signal.
 * @param {stirng} reference id
 * @param {string} event type
 * @return {undefined | number} the signal sent by native
 */
Listener.prototype.addEvent = function addEvent (ref, type) {
  return this.addActions(createAction('addEvent', [ref, type]))
};

/**
 * Send the "removeEvent" signal.
 * @param {stirng} reference id
 * @param {string} event type
 * @return {undefined | number} the signal sent by native
 */
Listener.prototype.removeEvent = function removeEvent (ref, type) {
  return this.addActions(createAction('removeEvent', [ref, type]))
};

/**
 * Default handler.
 * @param {object | array} actions
 * @param {function} callback
 * @return {} anything returned by callback function
 */
Listener.prototype.handler = function handler (actions, cb) {
  return cb && cb()
};

/**
 * Add actions into updates.
 * @param {object | array} actions
 * @return {undefined | number} the signal sent by native
 */
Listener.prototype.addActions = function addActions (actions) {
  var updates = this.updates;
  var handler = this.handler;

  if (!Array.isArray(actions)) {
    actions = [actions];
  }

  if (this.batched) {
    updates.push.apply(updates, actions);
  }
  else {
    return handler(actions)
  }
};

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

/**
 * @fileOverview
 * Task handler for communication between javascript and native.
 */

var handlerMap = {
  createBody: 'callCreateBody',
  addElement: 'callAddElement',
  removeElement: 'callRemoveElement',
  moveElement: 'callMoveElement',
  updateAttrs: 'callUpdateAttrs',
  updateStyle: 'callUpdateStyle',
  addEvent: 'callAddEvent',
  removeEvent: 'callRemoveEvent'
};

/**
 * Create a task handler.
 * @param {string} id
 * @param {function} handler
 * @return {function} taskHandler
 */
function createHandler (id, handler) {
  var defaultHandler = handler || global.callNative;

  /* istanbul ignore if */
  if (typeof defaultHandler !== 'function') {
    console.error('[JS Runtime] no default handler');
  }

  return function taskHandler (tasks) {
    /* istanbul ignore if */
    if (!Array.isArray(tasks)) {
      tasks = [tasks];
    }
    for (var i = 0; i < tasks.length; i++) {
      var returnValue = dispatchTask(id, tasks[i], defaultHandler);
      if (returnValue === -1) {
        return returnValue
      }
    }
  }
}

/**
 * Check if there is a corresponding available handler in the environment.
 * @param {string} module
 * @param {string} method
 * @return {boolean}
 */
function hasAvailableHandler (module, method) {
  return module === 'dom'
    && handlerMap[method]
    && typeof global[handlerMap[method]] === 'function'
}

/**
 * Dispatch the task to the specified handler.
 * @param {string} id
 * @param {object} task
 * @param {function} defaultHandler
 * @return {number} signal returned from native
 */
function dispatchTask (id, task, defaultHandler) {
  var module = task.module;
  var method = task.method;
  var args = task.args;

  if (hasAvailableHandler(module, method)) {
    return global[handlerMap[method]].apply(global, [ id ].concat( args, ['-1'] ))
  }

  return defaultHandler(id, [task], '-1')
}

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

/**
 * Update all changes for an element.
 * @param {object} element
 * @param {object} changes
 */
function updateElement (el, changes) {
  var attrs = changes.attrs || {};
  for (var name in attrs) {
    el.setAttr(name, attrs[name], true);
  }
  var style = changes.style || {};
  for (var name$1 in style) {
    el.setStyle(name$1, style[name$1], true);
  }
}

var Document = function Document (id, url, handler) {
  id = id ? id.toString() : '';
  this.id = id;
  this.URL = url;

  addDoc(id, this);
  this.nodeMap = {};
  var L = Document.Listener || Listener;
  this.listener = new L(id, handler || createHandler(id, Document.handler)); // deprecated
  this.taskCenter = new TaskCenter(id, handler ? function (id) {
    var args = [], len = arguments.length - 1;
    while ( len-- > 0 ) args[ len ] = arguments[ len + 1 ];

    return handler.apply(void 0, args);
  } : Document.handler);
  this.createDocumentElement();
};

/**
* Get the node from nodeMap.
* @param {string} reference id
* @return {object} node
*/
Document.prototype.getRef = function getRef (ref) {
  return this.nodeMap[ref]
};

/**
* Turn on batched updates.
*/
Document.prototype.open = function open () {
  this.listener.batched = false;
};

/**
* Turn off batched updates.
*/
Document.prototype.close = function close () {
  this.listener.batched = true;
};

/**
* Create the document element.
* @return {object} documentElement
*/
Document.prototype.createDocumentElement = function createDocumentElement () {
    var this$1 = this;

  if (!this.documentElement) {
    var el = new Element('document');
    el.docId = this.id;
    el.ownerDocument = this;
    el.role = 'documentElement';
    el.depth = 0;
    el.ref = '_documentElement';
    this.nodeMap._documentElement = el;
    this.documentElement = el;

    Object.defineProperty(el, 'appendChild', {
      configurable: true,
      enumerable: true,
      writable: true,
      value: function (node) {
        appendBody(this$1, node);
      }
    });

    Object.defineProperty(el, 'insertBefore', {
      configurable: true,
      enumerable: true,
      writable: true,
      value: function (node, before) {
        appendBody(this$1, node, before);
      }
    });
  }

  return this.documentElement
};

/**
* Create the body element.
* @param {string} type
* @param {objct} props
* @return {object} body element
*/
Document.prototype.createBody = function createBody (type, props) {
  if (!this.body) {
    var el = new Element(type, props);
    setBody(this, el);
  }

  return this.body
};

/**
* Create an element.
* @param {string} tagName
* @param {objct} props
* @return {object} element
*/
Document.prototype.createElement = function createElement (tagName, props) {
  var WeexElement = getWeexElement(tagName);
  if (WeexElement) {
    return new WeexElement(tagName, props)
  }
  return new Element(tagName, props)
};

/**
* Create an comment.
* @param {string} text
* @return {object} comment
*/
Document.prototype.createComment = function createComment (text) {
  return new Comment(text)
};

/**
* Fire an event on specified element manually.
* @param {object} element
* @param {string} type event type
* @param {object} eventevent object
* @param {object} domChanges
 * @param {object} options additional options
* @return {} anything returned by handler function
*/
Document.prototype.fireEvent = function fireEvent (el, type, event, domChanges, options) {
  if (!el) {
    return
  }
  event = event || {};
  event.type = type;
  event.target = el;
  event.currentTarget = el;
  event.timestamp = Date.now();
  if (domChanges) {
    updateElement(el, domChanges);
  }
  var isBubble = this.getRef('_root').attr['bubble'] === 'true';
  return el.fireEvent(type, event, isBubble, options)
};

/**
* Destroy current document, and remove itself form docMap.
*/
Document.prototype.destroy = function destroy () {
  this.taskCenter.destroyCallback();
  delete this.listener;
  delete this.nodeMap;
  delete this.taskCenter;
  removeDoc(this.id);
};

// default task handler
Document.handler = null;

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

var moduleProxys = {};

function setId (weex, id) {
  Object.defineProperty(weex, '[[CurrentInstanceId]]', { value: id });
}

function getId (weex) {
  return weex['[[CurrentInstanceId]]']
}

function moduleGetter$1 (module, method, taskCenter) {
  return function () {
    var args = [], len = arguments.length;
    while ( len-- ) args[ len ] = arguments[ len ];

    return taskCenter.send('module', { module: module, method: method }, args);
  }
}

var WeexInstance = function WeexInstance (id, config) {
  setId(this, id);
  this.config = config || {};
  this.document = new Document(id, this.config.bundleUrl);
  this.requireModule = this.requireModule.bind(this);
  this.isRegisteredModule = isRegisteredModule;
  this.isRegisteredComponent = isRegisteredComponent;
};

WeexInstance.prototype.requireModule = function requireModule (moduleName) {
  var id = getId(this);
  if (!(id && this.document && this.document.taskCenter)) {
    console.error(("[JS Framework] invalid instance id \"" + id + "\""));
    return
  }

  // warn for unknown module
  if (!isRegisteredModule(moduleName)) {
    console.warn(("[JS Framework] using unregistered weex module \"" + moduleName + "\""));
    return
  }

  // create new module proxy
  if (!moduleProxys[moduleName]) {
    var moduleDefine = getModuleDescription(moduleName);
    var taskCenter = this.document.taskCenter;

    // create registered module apis
    var moduleApis = {};
    var loop = function ( methodName ) {
      Object.defineProperty(moduleApis, methodName, {
        enumerable: true,
        configurable: true,
        get: function () { return moduleGetter$1(moduleName, methodName, taskCenter); },
        set: function set (fn) {
          if (typeof fn === 'function') {
            return taskCenter.send('module', {
              module: moduleName,
              method: methodName
            }, [fn])
          }
        }
      });
    };

      for (var methodName in moduleDefine) loop( methodName );

    // create module Proxy
    if (typeof Proxy === 'function') {
      moduleProxys[moduleName] = new Proxy(moduleApis, {
        get: function get (target, methodName) {
          if (methodName in target) {
            return target[methodName]
          }
          console.warn(("[JS Framework] using unregistered method \"" + moduleName + "." + methodName + "\""));
          return moduleGetter$1(moduleName, methodName, taskCenter)
        }
      });
    }
    else {
      moduleProxys[moduleName] = moduleApis;
    }
  }

  return moduleProxys[moduleName]
};

WeexInstance.prototype.supports = function supports (condition) {
  if (typeof condition !== 'string') { return null }

  var res = condition.match(/^@(\w+)\/(\w+)(\.(\w+))?$/i);
  if (res) {
    var type = res[1];
    var name = res[2];
    var method = res[4];
    switch (type) {
      case 'module': return isRegisteredModule(name, method)
      case 'component': return isRegisteredComponent(name)
    }
  }

  return null
};

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

var versionRegExp = /^\s*\/\/ *(\{[^}]*\}) *\r?\n/;

/**
 * Detect a JS Bundle code and make sure which framework it's based to. Each JS
 * Bundle should make sure that it starts with a line of JSON comment and is
 * more that one line.
 * @param  {string} code
 * @return {object}
 */
function getBundleType (code) {
  var result = versionRegExp.exec(code);
  if (result) {
    try {
      var info = JSON.parse(result[1]);
      return info.framework
    }
    catch (e) {}
  }
  return 'Weex' // default bundle type
}

var instanceMap = {};

function getFrameworkType (id) {
  if (instanceMap[id]) {
    return instanceMap[id].framework
  }
  return 'Weex'
}

/**
 * Check which framework a certain JS Bundle code based to. And create instance
 * by this framework.
 * @param {string} id
 * @param {string} code
 * @param {object} config
 * @param {object} data
 */
function createInstance (id, code, config, data) {
  if (instanceMap[id]) {
    return new Error(("invalid instance id \"" + id + "\""))
  }

  // Init instance info.
  var bundleType = getBundleType(code);
  var runtimeConfig = getRuntimeConfig();

  // Init instance config.
  config = JSON.parse(JSON.stringify(config || {}));
  config.env = JSON.parse(JSON.stringify(global.WXEnvironment || {}));

  var weex = new WeexInstance(id, config);
  Object.freeze(weex);

  var runtimeEnv = {
    weex: weex, // TODO: deprecated
    config: config, // TODO: deprecated
    created: Date.now(),
    framework: bundleType
  };

  // TODO: deprecated, no need to pass services to runtime env
  runtimeEnv.services = createServices(id, runtimeEnv, runtimeConfig);
  instanceMap[id] = runtimeEnv;

  var runtimeContext = Object.create(null);
  Object.assign(runtimeContext, runtimeEnv.services, { weex: weex });

  var framework = runtimeConfig.frameworks[bundleType];
  if (!framework) {
    return new Error(("invalid bundle type \"" + bundleType + "\"."))
  }

  {
    console.debug(("[JS Framework] create an " + bundleType + " instance"));
  }

  // run create instance
  if (typeof framework.prepareInstanceContext === 'function') {
    var instanceContext = framework.prepareInstanceContext(runtimeContext);
    return runInContext(code, instanceContext)
  }
  return framework.createInstance(id, code, config, data, runtimeEnv)
}

function runInContext (code, context) {
  var keys = [];
  var args = [];
  for (var key in context) {
    keys.push(key);
    args.push(context[key]);
  }

  var bundle = "\n    (function (global) {\n      \"use strict\";\n      " + code + "\n    })(Object.create(this))\n  ";

  return (new (Function.prototype.bind.apply( Function, [ null ].concat( keys, [bundle]) ))).apply(void 0, args)
}

function refreshInstance (id) {
  var args = [], len = arguments.length - 1;
  while ( len-- > 0 ) args[ len ] = arguments[ len + 1 ];

  var type = getFrameworkType(id);
  var runtimeConfig = getRuntimeConfig();
  refreshServices(id, {
    info: { framework: type },
    runtime: runtimeConfig
  });

  var fm = runtimeConfig.frameworks[type];
  if (!fm) {
    return new Error(("refreshInstance: the instance id of \"" + type + "\" is not found."))
  }
  return fm.refreshInstance.apply(fm, [ id ].concat( args ))
}

function destroyInstance (id) {
  var args = [], len = arguments.length - 1;
  while ( len-- > 0 ) args[ len ] = arguments[ len + 1 ];

  var type = getFrameworkType(id);
  var runtimeConfig = getRuntimeConfig();
  destroyServices(id, {
    info: { framework: type },
    runtime: runtimeConfig
  });

  var fm = runtimeConfig.frameworks[type];
  if (!fm) {
    return new Error(("destroyInstance: the instance id of \"" + type + "\" is not found."))
  }
  delete instanceMap[id];
  return fm.destroyInstance.apply(fm, [ id ].concat( args ))
}

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

var runtimeConfig = {};

function getRuntimeConfig () {
  return runtimeConfig
}

var methods = {
  createInstance: createInstance,
  refreshInstance: refreshInstance,
  destroyInstance: destroyInstance,
  registerService: registerService,
  unregisterService: unregisterService
};

/**
 * Register methods which init each frameworks.
 * @param {string} methodName
 * @param {function} sharedMethod
 */
function adaptMethod (methodName, sharedMethod) {
  methods[methodName] = function () {
    var args = [], len = arguments.length;
    while ( len-- ) args[ len ] = arguments[ len ];

    if (typeof sharedMethod === 'function') {
      sharedMethod.apply(void 0, args);
    }

    // TODO: deprecated
    for (var name in runtimeConfig.frameworks) {
      var framework = runtimeConfig.frameworks[name];
      if (framework && framework[methodName]) {
        framework[methodName].apply(framework, args);
      }
    }
  };
}

/**
 * Register methods which will be called for each instance.
 * @param {string} methodName
 */
// TODO: move to instance.js
function genInstance (methodName) {
  methods[methodName] = function () {
    var args = [], len = arguments.length;
    while ( len-- ) args[ len ] = arguments[ len ];

    var id = args[0];
    var type = getFrameworkType(id);
    var framework = runtimeConfig.frameworks[type];
    if (type && framework) {
      return framework[methodName].apply(framework, args)
    }
    return new Error(("invalid instance id \"" + id + "\""))
  };
}

function init$1 (config) {
  Object.assign(runtimeConfig, config);

  init$2();

  // Init each framework by `init` method and `config` which contains three
  // virtual-DOM Class: `Document`, `Element` & `Comment`, and a JS bridge method:
  // `sendTasks(...args)`.
  var frameworks = runtimeConfig.frameworks || {};
  for (var name in frameworks) {
    var framework = frameworks[name];
    framework.init(config);
  }

  adaptMethod('registerComponents', registerComponents);
  adaptMethod('registerModules', registerModules);
  adaptMethod('registerMethods')

  // TODO: deprecated
  ; ['receiveTasks', 'getRoot'].forEach(genInstance);

  // adapt instance
  methods.callJS = methods.receiveTasks;

  return methods
}

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

var config = {
  Document: Document, Element: Element, Comment: Comment, Listener: Listener,
  TaskCenter: TaskCenter,
  sendTasks: function sendTasks () {
    var args = [], len = arguments.length;
    while ( len-- ) args[ len ] = arguments[ len ];

    if (typeof callNative === 'function') {
      return callNative.apply(void 0, args)
    }
    return (global.callNative || (function () {})).apply(void 0, args)
  }
};

Document.handler = config.sendTasks;

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

/**
 * @fileOverview
 * Register framework(s) in JS runtime. Weex supply two layers for 3rd-party
 * framework(s): one is the instance management layer, another is the
 * virtual-DOM layer.
 */

/* istanbul ignore next */
function freezePrototype$$1 () {
  freezePrototype$1();

  // Object.freeze(config.Element)
  Object.freeze(config.Comment);
  Object.freeze(config.Listener);
  Object.freeze(config.Document.prototype);
  // Object.freeze(config.Element.prototype)
  Object.freeze(config.Comment.prototype);
  Object.freeze(config.Listener.prototype);
}

var runtime = {
  setNativeConsole: setNativeConsole,
  resetNativeConsole: resetNativeConsole,
  setNativeTimer: setNativeTimer,
  resetNativeTimer: resetNativeTimer,
  service: { registerService: registerService, unregisterService: unregisterService, hasService: hasService },
  freezePrototype: freezePrototype$$1,
  init: init$1,
  config: config
};

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

/**
 * Implement the MessageEvent.
 *
 * This type has been simplified.
 * https://html.spec.whatwg.org/multipage/comms.html#messageevent
 * https://dom.spec.whatwg.org/#interface-event
 */
var MessageEvent = function MessageEvent (type, dict) {
  if ( dict === void 0 ) dict = {};

  this.type = type || 'message';

  this.data = dict.data || null;
  this.origin = dict.origin || '';
  this.source = dict.source || null;
  this.ports = dict.ports || [];

  // inherit properties
  this.target = null;
  this.timeStamp = Date.now();
};

var channels = {};

/**
 * Implement the BroadcastChannel API.
 * This api can be used to achieve inter-instance communications.
 *
 * https://html.spec.whatwg.org/multipage/comms.html#broadcasting-to-other-browsing-contexts
 */
var BroadcastChannel$1 = function BroadcastChannel$1 (name) {
  // the name property is readonly
  Object.defineProperty(this, 'name', {
    configurable: false,
    enumerable: true,
    writable: false,
    value: String(name)
  });

  this._closed = false;
  this.onmessage = null;

  if (!channels[this.name]) {
    channels[this.name] = [];
  }
  channels[this.name].push(this);
};

/**
 * Sends the given message to other BroadcastChannel objects set up for this channel.
 * @param {any} message
 */
BroadcastChannel$1.prototype.postMessage = function postMessage (message) {
    var this$1 = this;

  if (this._closed) {
    throw new Error(("BroadcastChannel \"" + (this.name) + "\" is closed."))
  }

  var subscribers = channels[this.name];
  if (subscribers && subscribers.length) {
    for (var i = 0; i < subscribers.length; ++i) {
      var member = subscribers[i];
      if (member._closed || member === this$1) { continue }
      if (typeof member.onmessage === 'function') {
        member.onmessage(new MessageEvent('message', { data: message }));
      }
    }
  }
};

/**
 * Closes the BroadcastChannel object, opening it up to garbage collection.
 */
BroadcastChannel$1.prototype.close = function close () {
    var this$1 = this;

  if (this._closed) {
    return
  }

  // remove itself from channels.
  if (channels[this.name]) {
    var subscribers = channels[this.name].filter(function (x) { return x !== this$1; });
    if (subscribers.length) {
      channels[this.name] = subscribers;
    }
    else {
      delete channels[this.name];
    }
  }

  this._closed = true;
};

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

var listeners = {};

var BroadcastChannel$$1 = {
  create: function create (id) {
    if (typeof global.BroadcastChannel === 'function') {
      return {}
    }

    listeners[id] = [];
    var InstanceBroadcastChannel = (function (BroadcastChannel$$1) {
      function InstanceBroadcastChannel () {
        var args = [], len = arguments.length;
        while ( len-- ) args[ len ] = arguments[ len ];

        BroadcastChannel$$1.apply(this, args);
        listeners[id].push(this);
      }

      if ( BroadcastChannel$$1 ) InstanceBroadcastChannel.__proto__ = BroadcastChannel$$1;
      InstanceBroadcastChannel.prototype = Object.create( BroadcastChannel$$1 && BroadcastChannel$$1.prototype );
      InstanceBroadcastChannel.prototype.constructor = InstanceBroadcastChannel;

      return InstanceBroadcastChannel;
    }(BroadcastChannel$1));
    return {
      instance: {
        BroadcastChannel: InstanceBroadcastChannel
      }
    }
  },

  destroy: function destroy (id) {
    if (id && listeners[id]) {
      listeners[id].forEach(function (member) { return member.close(); });
      delete listeners[id];
    }
  }
};

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
var services$1 = {
  BroadcastChannel: BroadcastChannel$$1
};

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

/**
 * Setup frameworks with runtime.
 * You can package more frameworks by
 *  passing them as arguments.
 */
var setup = function (frameworks) {
  var init = runtime.init;
  var config = runtime.config;
  config.frameworks = frameworks;
  var native = subversion.native;
  var transformer = subversion.transformer;

  for (var serviceName in services$1) {
    runtime.service.registerService(serviceName, services$1[serviceName]);
  }

  runtime.freezePrototype();
  runtime.setNativeConsole();

  // register framework meta info
  global.frameworkVersion = native;
  global.transformerVersion = transformer;

  // init frameworks
  var globalMethods = init(config);

  // set global methods
  var loop = function ( methodName ) {
    global[methodName] = function () {
      var args = [], len = arguments.length;
      while ( len-- ) args[ len ] = arguments[ len ];

      var ret = globalMethods[methodName].apply(globalMethods, args);
      if (ret instanceof Error) {
        console.error(ret.toString());
      }
      return ret
    };
  };

  for (var methodName in globalMethods) loop( methodName );
};

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
var config$2 = {};

var instanceMap$1 = {};

function init$3 (cfg) {
  config$2.Document = cfg.Document;
  config$2.Element = cfg.Element;
  config$2.Comment = cfg.Comment;
  config$2.sendTasks = cfg.sendTasks;
}

function createInstance$1 (id, code, options, data, serviceObjects) {
  if ( options === void 0 ) options = {};
  if ( data === void 0 ) data = {};
  if ( serviceObjects === void 0 ) serviceObjects = {};

  var document = new config$2.Document(id, options.bundleUrl);
  var callbacks = {};
  var instance = { id: id, data: data, document: document, callbacks: callbacks };

  var lastCallbackId = 0;
  document.addCallback = function (func) {
    lastCallbackId++;
    callbacks[lastCallbackId] = func;
    return lastCallbackId
  };
  document.handleCallback = function (funcId, data, ifLast) {
    var callback = callbacks[funcId];
    if (ifLast) {
      delete callbacks[funcId];
    }
    return callback(data)
  };
  instanceMap$1[id] = instance;

  var globalObjects = Object.assign({
    Document: config$2.Document,
    Element: config$2.Element,
    Comment: config$2.Comment,
    sendTasks: function (tasks) { return config$2.sendTasks(id, tasks, -1); },
    options: options,
    document: document
  }, serviceObjects);

  var globalKeys = [];
  var globalValues = [];
  for (var key in globalObjects) {
    globalKeys.push(key);
    globalValues.push(globalObjects[key]);
  }
  globalKeys.push(code);

  var result = new (Function.prototype.bind.apply( Function, [ null ].concat( globalKeys) ));
  result.apply(void 0, globalValues);

  config$2.sendTasks(id, [{ module: 'dom', method: 'createFinish', args: [] }], -1);

  return instance
}

function destroyInstance$1 (id) {
  delete instanceMap$1[id];
}

function getRoot (id) {
  return instanceMap$1[id].document.body.toJSON()
}

function receiveTasks (id, tasks) {
  var jsHandlers = {
    fireEvent: function (id, ref, type, data, domChanges) {
      var ref$1 = instanceMap$1[id];
      var document = ref$1.document;
      var el = document.getRef(ref);
      return document.fireEvent(el, type, data, domChanges)
    },

    callback: function (id, funcId, data, ifLast) {
      var ref = instanceMap$1[id];
      var document = ref.document;
      return document.handleCallback(funcId, data, ifLast)
    }
  };

  var ref = instanceMap$1[id] || {};
  var document = ref.document;
  if (document && Array.isArray(tasks)) {
    var results = [];
    tasks.forEach(function (task) {
      var handler = jsHandlers[task.method];
      var args = [].concat( task.args );
      if (typeof handler === 'function') {
        args.unshift(id);
        results.push(handler.apply(void 0, args));
      }
    });
    return results
  }
}


var Vanilla = Object.freeze({
	init: init$3,
	createInstance: createInstance$1,
	destroyInstance: destroyInstance$1,
	getRoot: getRoot,
	receiveTasks: receiveTasks
});

var factory = createCommonjsModule(function (module) {
'use strict';

module.exports = function weexFactory (exports, renderer) {

/*  */

// these helpers produces better vm code in JS engines due to their
// explicitness and function inlining
function isUndef (v) {
  return v === undefined || v === null
}

function isDef (v) {
  return v !== undefined && v !== null
}

function isTrue (v) {
  return v === true
}

function isFalse (v) {
  return v === false
}

/**
 * Check if value is primitive
 */
function isPrimitive (value) {
  return (
    typeof value === 'string' ||
    typeof value === 'number' ||
    typeof value === 'boolean'
  )
}

/**
 * Quick object check - this is primarily used to tell
 * Objects from primitive values when we know the value
 * is a JSON-compliant type.
 */
function isObject (obj) {
  return obj !== null && typeof obj === 'object'
}

var _toString = Object.prototype.toString;

/**
 * Strict object type check. Only returns true
 * for plain JavaScript objects.
 */
function isPlainObject (obj) {
  return _toString.call(obj) === '[object Object]'
}

function isRegExp (v) {
  return _toString.call(v) === '[object RegExp]'
}

/**
 * Check if val is a valid array index.
 */
function isValidArrayIndex (val) {
  var n = parseFloat(val);
  return n >= 0 && Math.floor(n) === n && isFinite(val)
}

/**
 * Convert a value to a string that is actually rendered.
 */
function toString (val) {
  return val == null
    ? ''
    : typeof val === 'object'
      ? JSON.stringify(val, null, 2)
      : String(val)
}

/**
 * Convert a input value to a number for persistence.
 * If the conversion fails, return original string.
 */
function toNumber (val) {
  var n = parseFloat(val);
  return isNaN(n) ? val : n
}

/**
 * Make a map and return a function for checking if a key
 * is in that map.
 */
function makeMap (
  str,
  expectsLowerCase
) {
  var map = Object.create(null);
  var list = str.split(',');
  for (var i = 0; i < list.length; i++) {
    map[list[i]] = true;
  }
  return expectsLowerCase
    ? function (val) { return map[val.toLowerCase()]; }
    : function (val) { return map[val]; }
}

/**
 * Check if a tag is a built-in tag.
 */
var isBuiltInTag = makeMap('slot,component', true);

/**
 * Check if a attribute is a reserved attribute.
 */
var isReservedAttribute = makeMap('key,ref,slot,is');

/**
 * Remove an item from an array
 */
function remove (arr, item) {
  if (arr.length) {
    var index = arr.indexOf(item);
    if (index > -1) {
      return arr.splice(index, 1)
    }
  }
}

/**
 * Check whether the object has the property.
 */
var hasOwnProperty = Object.prototype.hasOwnProperty;
function hasOwn (obj, key) {
  return hasOwnProperty.call(obj, key)
}

/**
 * Create a cached version of a pure function.
 */
function cached (fn) {
  var cache = Object.create(null);
  return (function cachedFn (str) {
    var hit = cache[str];
    return hit || (cache[str] = fn(str))
  })
}

/**
 * Camelize a hyphen-delimited string.
 */
var camelizeRE = /-(\w)/g;
var camelize = cached(function (str) {
  return str.replace(camelizeRE, function (_, c) { return c ? c.toUpperCase() : ''; })
});

/**
 * Capitalize a string.
 */
var capitalize = cached(function (str) {
  return str.charAt(0).toUpperCase() + str.slice(1)
});

/**
 * Hyphenate a camelCase string.
 */
var hyphenateRE = /\B([A-Z])/g;
var hyphenate = cached(function (str) {
  return str.replace(hyphenateRE, '-$1').toLowerCase()
});

/**
 * Simple bind, faster than native
 */
function bind (fn, ctx) {
  function boundFn (a) {
    var l = arguments.length;
    return l
      ? l > 1
        ? fn.apply(ctx, arguments)
        : fn.call(ctx, a)
      : fn.call(ctx)
  }
  // record original fn length
  boundFn._length = fn.length;
  return boundFn
}

/**
 * Convert an Array-like object to a real Array.
 */
function toArray (list, start) {
  start = start || 0;
  var i = list.length - start;
  var ret = new Array(i);
  while (i--) {
    ret[i] = list[i + start];
  }
  return ret
}

/**
 * Mix properties into target object.
 */
function extend (to, _from) {
  for (var key in _from) {
    to[key] = _from[key];
  }
  return to
}

/**
 * Merge an Array of Objects into a single Object.
 */
function toObject (arr) {
  var res = {};
  for (var i = 0; i < arr.length; i++) {
    if (arr[i]) {
      extend(res, arr[i]);
    }
  }
  return res
}

/**
 * Perform no operation.
 * Stubbing args to make Flow happy without leaving useless transpiled code
 * with ...rest (https://flow.org/blog/2017/05/07/Strict-Function-Call-Arity/)
 */
function noop (a, b, c) {}

/**
 * Always return false.
 */
var no = function (a, b, c) { return false; };

/**
 * Return same value
 */
var identity = function (_) { return _; };

/**
 * Generate a static keys string from compiler modules.
 */


/**
 * Check if two values are loosely equal - that is,
 * if they are plain objects, do they have the same shape?
 */
function looseEqual (a, b) {
  if (a === b) { return true }
  var isObjectA = isObject(a);
  var isObjectB = isObject(b);
  if (isObjectA && isObjectB) {
    try {
      var isArrayA = Array.isArray(a);
      var isArrayB = Array.isArray(b);
      if (isArrayA && isArrayB) {
        return a.length === b.length && a.every(function (e, i) {
          return looseEqual(e, b[i])
        })
      } else if (!isArrayA && !isArrayB) {
        var keysA = Object.keys(a);
        var keysB = Object.keys(b);
        return keysA.length === keysB.length && keysA.every(function (key) {
          return looseEqual(a[key], b[key])
        })
      } else {
        /* istanbul ignore next */
        return false
      }
    } catch (e) {
      /* istanbul ignore next */
      return false
    }
  } else if (!isObjectA && !isObjectB) {
    return String(a) === String(b)
  } else {
    return false
  }
}

function looseIndexOf (arr, val) {
  for (var i = 0; i < arr.length; i++) {
    if (looseEqual(arr[i], val)) { return i }
  }
  return -1
}

/**
 * Ensure a function is called only once.
 */
function once (fn) {
  var called = false;
  return function () {
    if (!called) {
      called = true;
      fn.apply(this, arguments);
    }
  }
}

var SSR_ATTR = 'data-server-rendered';

var ASSET_TYPES = [
  'component',
  'directive',
  'filter'
];

var LIFECYCLE_HOOKS = [
  'beforeCreate',
  'created',
  'beforeMount',
  'mounted',
  'beforeUpdate',
  'updated',
  'beforeDestroy',
  'destroyed',
  'activated',
  'deactivated'
];

/*  */

var config = ({
  /**
   * Option merge strategies (used in core/util/options)
   */
  optionMergeStrategies: Object.create(null),

  /**
   * Whether to suppress warnings.
   */
  silent: false,

  /**
   * Show production mode tip message on boot?
   */
  productionTip: "development" !== 'production',

  /**
   * Whether to enable devtools
   */
  devtools: "development" !== 'production',

  /**
   * Whether to record perf
   */
  performance: false,

  /**
   * Error handler for watcher errors
   */
  errorHandler: null,

  /**
   * Warn handler for watcher warns
   */
  warnHandler: null,

  /**
   * Ignore certain custom elements
   */
  ignoredElements: [],

  /**
   * Custom user key aliases for v-on
   */
  keyCodes: Object.create(null),

  /**
   * Check if a tag is reserved so that it cannot be registered as a
   * component. This is platform-dependent and may be overwritten.
   */
  isReservedTag: no,

  /**
   * Check if an attribute is reserved so that it cannot be used as a component
   * prop. This is platform-dependent and may be overwritten.
   */
  isReservedAttr: no,

  /**
   * Check if a tag is an unknown element.
   * Platform-dependent.
   */
  isUnknownElement: no,

  /**
   * Get the namespace of an element
   */
  getTagNamespace: noop,

  /**
   * Parse the real tag name for the specific platform.
   */
  parsePlatformTagName: identity,

  /**
   * Check if an attribute must be bound using property, e.g. value
   * Platform-dependent.
   */
  mustUseProp: no,

  /**
   * Exposed for legacy reasons
   */
  _lifecycleHooks: LIFECYCLE_HOOKS
});

/*  */

var emptyObject = Object.freeze({});

/**
 * Check if a string starts with $ or _
 */
function isReserved (str) {
  var c = (str + '').charCodeAt(0);
  return c === 0x24 || c === 0x5F
}

/**
 * Define a property.
 */
function def (obj, key, val, enumerable) {
  Object.defineProperty(obj, key, {
    value: val,
    enumerable: !!enumerable,
    writable: true,
    configurable: true
  });
}

/**
 * Parse simple path.
 */
var bailRE = /[^\w.$]/;
function parsePath (path) {
  if (bailRE.test(path)) {
    return
  }
  var segments = path.split('.');
  return function (obj) {
    for (var i = 0; i < segments.length; i++) {
      if (!obj) { return }
      obj = obj[segments[i]];
    }
    return obj
  }
}

/*  */

var warn = noop;
var tip = noop;
var formatComponentName = (null); // work around flow check

{
  var hasConsole = typeof console !== 'undefined';
  var classifyRE = /(?:^|[-_])(\w)/g;
  var classify = function (str) { return str
    .replace(classifyRE, function (c) { return c.toUpperCase(); })
    .replace(/[-_]/g, ''); };

  warn = function (msg, vm) {
    var trace = vm ? generateComponentTrace(vm) : '';

    if (config.warnHandler) {
      config.warnHandler.call(null, msg, vm, trace);
    } else if (hasConsole && (!config.silent)) {
      console.error(("[Vue warn]: " + msg + trace));
    }
  };

  tip = function (msg, vm) {
    if (hasConsole && (!config.silent)) {
      console.warn("[Vue tip]: " + msg + (
        vm ? generateComponentTrace(vm) : ''
      ));
    }
  };

  formatComponentName = function (vm, includeFile) {
    if (vm.$root === vm) {
      return '<Root>'
    }
    var name = typeof vm === 'string'
      ? vm
      : typeof vm === 'function' && vm.options
        ? vm.options.name
        : vm._isVue
          ? vm.$options.name || vm.$options._componentTag
          : vm.name;

    var file = vm._isVue && vm.$options.__file;
    if (!name && file) {
      var match = file.match(/([^/\\]+)\.vue$/);
      name = match && match[1];
    }

    return (
      (name ? ("<" + (classify(name)) + ">") : "<Anonymous>") +
      (file && includeFile !== false ? (" at " + file) : '')
    )
  };

  var repeat = function (str, n) {
    var res = '';
    while (n) {
      if (n % 2 === 1) { res += str; }
      if (n > 1) { str += str; }
      n >>= 1;
    }
    return res
  };

  var generateComponentTrace = function (vm) {
    if (vm._isVue && vm.$parent) {
      var tree = [];
      var currentRecursiveSequence = 0;
      while (vm) {
        if (tree.length > 0) {
          var last = tree[tree.length - 1];
          if (last.constructor === vm.constructor) {
            currentRecursiveSequence++;
            vm = vm.$parent;
            continue
          } else if (currentRecursiveSequence > 0) {
            tree[tree.length - 1] = [last, currentRecursiveSequence];
            currentRecursiveSequence = 0;
          }
        }
        tree.push(vm);
        vm = vm.$parent;
      }
      return '\n\nfound in\n\n' + tree
        .map(function (vm, i) { return ("" + (i === 0 ? '---> ' : repeat(' ', 5 + i * 2)) + (Array.isArray(vm)
            ? ((formatComponentName(vm[0])) + "... (" + (vm[1]) + " recursive calls)")
            : formatComponentName(vm))); })
        .join('\n')
    } else {
      return ("\n\n(found in " + (formatComponentName(vm)) + ")")
    }
  };
}

/*  */

function handleError (err, vm, info) {
  if (config.errorHandler) {
    config.errorHandler.call(null, err, vm, info);
  } else {
    {
      warn(("Error in " + info + ": \"" + (err.toString()) + "\""), vm);
    }
    /* istanbul ignore else */
    if (inBrowser && typeof console !== 'undefined') {
      console.error(err);
    } else {
      throw err
    }
  }
}

/*  */
/* globals MutationObserver */

// can we use __proto__?
var hasProto = '__proto__' in {};

// Browser environment sniffing
var inBrowser = typeof window !== 'undefined';
var UA = inBrowser && window.navigator.userAgent.toLowerCase();
var isIE = UA && /msie|trident/.test(UA);
var isIE9 = UA && UA.indexOf('msie 9.0') > 0;
var isEdge = UA && UA.indexOf('edge/') > 0;
var isAndroid = UA && UA.indexOf('android') > 0;
var isIOS = UA && /iphone|ipad|ipod|ios/.test(UA);
var isChrome = UA && /chrome\/\d+/.test(UA) && !isEdge;

// Firefox has a "watch" function on Object.prototype...
var nativeWatch = ({}).watch;

var supportsPassive = false;
if (inBrowser) {
  try {
    var opts = {};
    Object.defineProperty(opts, 'passive', ({
      get: function get () {
        /* istanbul ignore next */
        supportsPassive = true;
      }
    })); // https://github.com/facebook/flow/issues/285
    window.addEventListener('test-passive', null, opts);
  } catch (e) {}
}

// this needs to be lazy-evaled because vue may be required before
// vue-server-renderer can set VUE_ENV
var _isServer;
var isServerRendering = function () {
  if (_isServer === undefined) {
    /* istanbul ignore if */
    if (!inBrowser && typeof commonjsGlobal !== 'undefined') {
      // detect presence of vue-server-renderer and avoid
      // Webpack shimming the process
      _isServer = commonjsGlobal['process'].env.VUE_ENV === 'server';
    } else {
      _isServer = false;
    }
  }
  return _isServer
};

// detect devtools
var devtools = inBrowser && window.__VUE_DEVTOOLS_GLOBAL_HOOK__;

/* istanbul ignore next */
function isNative (Ctor) {
  return typeof Ctor === 'function' && /native code/.test(Ctor.toString())
}

var hasSymbol =
  typeof Symbol !== 'undefined' && isNative(Symbol) &&
  typeof Reflect !== 'undefined' && isNative(Reflect.ownKeys);

/**
 * Defer a task to execute it asynchronously.
 */
var nextTick = (function () {
  var callbacks = [];
  var pending = false;
  var timerFunc;

  function nextTickHandler () {
    pending = false;
    var copies = callbacks.slice(0);
    callbacks.length = 0;
    for (var i = 0; i < copies.length; i++) {
      copies[i]();
    }
  }

  // the nextTick behavior leverages the microtask queue, which can be accessed
  // via either native Promise.then or MutationObserver.
  // MutationObserver has wider support, however it is seriously bugged in
  // UIWebView in iOS >= 9.3.3 when triggered in touch event handlers. It
  // completely stops working after triggering a few times... so, if native
  // Promise is available, we will use it:
  /* istanbul ignore if */
  if (typeof Promise !== 'undefined' && isNative(Promise)) {
    var p = Promise.resolve();
    var logError = function (err) { console.error(err); };
    timerFunc = function () {
      p.then(nextTickHandler).catch(logError);
      // in problematic UIWebViews, Promise.then doesn't completely break, but
      // it can get stuck in a weird state where callbacks are pushed into the
      // microtask queue but the queue isn't being flushed, until the browser
      // needs to do some other work, e.g. handle a timer. Therefore we can
      // "force" the microtask queue to be flushed by adding an empty timer.
      if (isIOS) { setTimeout(noop); }
    };
  } else if (!isIE && typeof MutationObserver !== 'undefined' && (
    isNative(MutationObserver) ||
    // PhantomJS and iOS 7.x
    MutationObserver.toString() === '[object MutationObserverConstructor]'
  )) {
    // use MutationObserver where native Promise is not available,
    // e.g. PhantomJS, iOS7, Android 4.4
    var counter = 1;
    var observer = new MutationObserver(nextTickHandler);
    var textNode = document.createTextNode(String(counter));
    observer.observe(textNode, {
      characterData: true
    });
    timerFunc = function () {
      counter = (counter + 1) % 2;
      textNode.data = String(counter);
    };
  } else {
    // fallback to setTimeout
    /* istanbul ignore next */
    timerFunc = function () {
      setTimeout(nextTickHandler, 0);
    };
  }

  return function queueNextTick (cb, ctx) {
    var _resolve;
    callbacks.push(function () {
      if (cb) {
        try {
          cb.call(ctx);
        } catch (e) {
          handleError(e, ctx, 'nextTick');
        }
      } else if (_resolve) {
        _resolve(ctx);
      }
    });
    if (!pending) {
      pending = true;
      timerFunc();
    }
    if (!cb && typeof Promise !== 'undefined') {
      return new Promise(function (resolve, reject) {
        _resolve = resolve;
      })
    }
  }
})();

var _Set;
/* istanbul ignore if */
if (typeof Set !== 'undefined' && isNative(Set)) {
  // use native Set when available.
  _Set = Set;
} else {
  // a non-standard Set polyfill that only works with primitive keys.
  _Set = (function () {
    function Set () {
      this.set = Object.create(null);
    }
    Set.prototype.has = function has (key) {
      return this.set[key] === true
    };
    Set.prototype.add = function add (key) {
      this.set[key] = true;
    };
    Set.prototype.clear = function clear () {
      this.set = Object.create(null);
    };

    return Set;
  }());
}

/*  */


var uid$1 = 0;

/**
 * A dep is an observable that can have multiple
 * directives subscribing to it.
 */
var Dep = function Dep () {
  this.id = uid$1++;
  this.subs = [];
};

Dep.prototype.addSub = function addSub (sub) {
  this.subs.push(sub);
};

Dep.prototype.removeSub = function removeSub (sub) {
  remove(this.subs, sub);
};

Dep.prototype.depend = function depend () {
  if (Dep.target) {
    Dep.target.addDep(this);
  }
};

Dep.prototype.notify = function notify () {
  // stabilize the subscriber list first
  var subs = this.subs.slice();
  for (var i = 0, l = subs.length; i < l; i++) {
    subs[i].update();
  }
};

// the current target watcher being evaluated.
// this is globally unique because there could be only one
// watcher being evaluated at any time.
Dep.target = null;
var targetStack = [];

function pushTarget (_target) {
  if (Dep.target) { targetStack.push(Dep.target); }
  Dep.target = _target;
}

function popTarget () {
  Dep.target = targetStack.pop();
}

/*
 * not type checking this file because flow doesn't play well with
 * dynamically accessing methods on Array prototype
 */

var arrayProto = Array.prototype;
var arrayMethods = Object.create(arrayProto);[
  'push',
  'pop',
  'shift',
  'unshift',
  'splice',
  'sort',
  'reverse'
]
.forEach(function (method) {
  // cache original method
  var original = arrayProto[method];
  def(arrayMethods, method, function mutator () {
    var arguments$1 = arguments;

    var args = [], len = arguments.length;
    while ( len-- ) { args[ len ] = arguments$1[ len ]; }

    var result = original.apply(this, args);
    var ob = this.__ob__;
    var inserted;
    switch (method) {
      case 'push':
      case 'unshift':
        inserted = args;
        break
      case 'splice':
        inserted = args.slice(2);
        break
    }
    if (inserted) { ob.observeArray(inserted); }
    // notify change
    ob.dep.notify();
    return result
  });
});

/*  */

var arrayKeys = Object.getOwnPropertyNames(arrayMethods);

/**
 * By default, when a reactive property is set, the new value is
 * also converted to become reactive. However when passing down props,
 * we don't want to force conversion because the value may be a nested value
 * under a frozen data structure. Converting it would defeat the optimization.
 */
var observerState = {
  shouldConvert: true
};

/**
 * Observer class that are attached to each observed
 * object. Once attached, the observer converts target
 * object's property keys into getter/setters that
 * collect dependencies and dispatches updates.
 */
var Observer = function Observer (value) {
  this.value = value;
  this.dep = new Dep();
  this.vmCount = 0;
  def(value, '__ob__', this);
  if (Array.isArray(value)) {
    var augment = hasProto
      ? protoAugment
      : copyAugment;
    augment(value, arrayMethods, arrayKeys);
    this.observeArray(value);
  } else {
    this.walk(value);
  }
};

/**
 * Walk through each property and convert them into
 * getter/setters. This method should only be called when
 * value type is Object.
 */
Observer.prototype.walk = function walk (obj) {
  var keys = Object.keys(obj);
  for (var i = 0; i < keys.length; i++) {
    defineReactive$$1(obj, keys[i], obj[keys[i]]);
  }
};

/**
 * Observe a list of Array items.
 */
Observer.prototype.observeArray = function observeArray (items) {
  for (var i = 0, l = items.length; i < l; i++) {
    observe(items[i]);
  }
};

// helpers

/**
 * Augment an target Object or Array by intercepting
 * the prototype chain using __proto__
 */
function protoAugment (target, src, keys) {
  /* eslint-disable no-proto */
  target.__proto__ = src;
  /* eslint-enable no-proto */
}

/**
 * Augment an target Object or Array by defining
 * hidden properties.
 */
/* istanbul ignore next */
function copyAugment (target, src, keys) {
  for (var i = 0, l = keys.length; i < l; i++) {
    var key = keys[i];
    def(target, key, src[key]);
  }
}

/**
 * Attempt to create an observer instance for a value,
 * returns the new observer if successfully observed,
 * or the existing observer if the value already has one.
 */
function observe (value, asRootData) {
  if (!isObject(value)) {
    return
  }
  var ob;
  if (hasOwn(value, '__ob__') && value.__ob__ instanceof Observer) {
    ob = value.__ob__;
  } else if (
    observerState.shouldConvert &&
    !isServerRendering() &&
    (Array.isArray(value) || isPlainObject(value)) &&
    Object.isExtensible(value) &&
    !value._isVue
  ) {
    ob = new Observer(value);
  }
  if (asRootData && ob) {
    ob.vmCount++;
  }
  return ob
}

/**
 * Define a reactive property on an Object.
 */
function defineReactive$$1 (
  obj,
  key,
  val,
  customSetter,
  shallow
) {
  var dep = new Dep();

  var property = Object.getOwnPropertyDescriptor(obj, key);
  if (property && property.configurable === false) {
    return
  }

  // cater for pre-defined getter/setters
  var getter = property && property.get;
  var setter = property && property.set;

  var childOb = !shallow && observe(val);
  Object.defineProperty(obj, key, {
    enumerable: true,
    configurable: true,
    get: function reactiveGetter () {
      var value = getter ? getter.call(obj) : val;
      if (Dep.target) {
        dep.depend();
        if (childOb) {
          childOb.dep.depend();
          if (Array.isArray(value)) {
            dependArray(value);
          }
        }
      }
      return value
    },
    set: function reactiveSetter (newVal) {
      var value = getter ? getter.call(obj) : val;
      /* eslint-disable no-self-compare */
      if (newVal === value || (newVal !== newVal && value !== value)) {
        return
      }
      /* eslint-enable no-self-compare */
      if ("development" !== 'production' && customSetter) {
        customSetter();
      }
      if (setter) {
        setter.call(obj, newVal);
      } else {
        val = newVal;
      }
      childOb = !shallow && observe(newVal);
      dep.notify();
    }
  });
}

/**
 * Set a property on an object. Adds the new property and
 * triggers change notification if the property doesn't
 * already exist.
 */
function set (target, key, val) {
  if (Array.isArray(target) && isValidArrayIndex(key)) {
    target.length = Math.max(target.length, key);
    target.splice(key, 1, val);
    return val
  }
  if (hasOwn(target, key)) {
    target[key] = val;
    return val
  }
  var ob = (target).__ob__;
  if (target._isVue || (ob && ob.vmCount)) {
    "development" !== 'production' && warn(
      'Avoid adding reactive properties to a Vue instance or its root $data ' +
      'at runtime - declare it upfront in the data option.'
    );
    return val
  }
  if (!ob) {
    target[key] = val;
    return val
  }
  defineReactive$$1(ob.value, key, val);
  ob.dep.notify();
  return val
}

/**
 * Delete a property and trigger change if necessary.
 */
function del (target, key) {
  if (Array.isArray(target) && isValidArrayIndex(key)) {
    target.splice(key, 1);
    return
  }
  var ob = (target).__ob__;
  if (target._isVue || (ob && ob.vmCount)) {
    "development" !== 'production' && warn(
      'Avoid deleting properties on a Vue instance or its root $data ' +
      '- just set it to null.'
    );
    return
  }
  if (!hasOwn(target, key)) {
    return
  }
  delete target[key];
  if (!ob) {
    return
  }
  ob.dep.notify();
}

/**
 * Collect dependencies on array elements when the array is touched, since
 * we cannot intercept array element access like property getters.
 */
function dependArray (value) {
  for (var e = (void 0), i = 0, l = value.length; i < l; i++) {
    e = value[i];
    e && e.__ob__ && e.__ob__.dep.depend();
    if (Array.isArray(e)) {
      dependArray(e);
    }
  }
}

/*  */

/**
 * Option overwriting strategies are functions that handle
 * how to merge a parent option value and a child option
 * value into the final value.
 */
var strats = config.optionMergeStrategies;

/**
 * Options with restrictions
 */
{
  strats.el = strats.propsData = function (parent, child, vm, key) {
    if (!vm) {
      warn(
        "option \"" + key + "\" can only be used during instance " +
        'creation with the `new` keyword.'
      );
    }
    return defaultStrat(parent, child)
  };
}

/**
 * Helper that recursively merges two data objects together.
 */
function mergeData (to, from) {
  if (!from) { return to }
  var key, toVal, fromVal;
  var keys = Object.keys(from);
  for (var i = 0; i < keys.length; i++) {
    key = keys[i];
    toVal = to[key];
    fromVal = from[key];
    if (!hasOwn(to, key)) {
      set(to, key, fromVal);
    } else if (isPlainObject(toVal) && isPlainObject(fromVal)) {
      mergeData(toVal, fromVal);
    }
  }
  return to
}

/**
 * Data
 */
function mergeDataOrFn (
  parentVal,
  childVal,
  vm
) {
  if (!vm) {
    // in a Vue.extend merge, both should be functions
    if (!childVal) {
      return parentVal
    }
    if (!parentVal) {
      return childVal
    }
    // when parentVal & childVal are both present,
    // we need to return a function that returns the
    // merged result of both functions... no need to
    // check if parentVal is a function here because
    // it has to be a function to pass previous merges.
    return function mergedDataFn () {
      return mergeData(
        typeof childVal === 'function' ? childVal.call(this) : childVal,
        typeof parentVal === 'function' ? parentVal.call(this) : parentVal
      )
    }
  } else if (parentVal || childVal) {
    return function mergedInstanceDataFn () {
      // instance merge
      var instanceData = typeof childVal === 'function'
        ? childVal.call(vm)
        : childVal;
      var defaultData = typeof parentVal === 'function'
        ? parentVal.call(vm)
        : parentVal;
      if (instanceData) {
        return mergeData(instanceData, defaultData)
      } else {
        return defaultData
      }
    }
  }
}

strats.data = function (
  parentVal,
  childVal,
  vm
) {
  if (!vm) {
    if (childVal && typeof childVal !== 'function') {
      "development" !== 'production' && warn(
        'The "data" option should be a function ' +
        'that returns a per-instance value in component ' +
        'definitions.',
        vm
      );

      return parentVal
    }
    return mergeDataOrFn.call(this, parentVal, childVal)
  }

  return mergeDataOrFn(parentVal, childVal, vm)
};

/**
 * Hooks and props are merged as arrays.
 */
function mergeHook (
  parentVal,
  childVal
) {
  return childVal
    ? parentVal
      ? parentVal.concat(childVal)
      : Array.isArray(childVal)
        ? childVal
        : [childVal]
    : parentVal
}

LIFECYCLE_HOOKS.forEach(function (hook) {
  strats[hook] = mergeHook;
});

/**
 * Assets
 *
 * When a vm is present (instance creation), we need to do
 * a three-way merge between constructor options, instance
 * options and parent options.
 */
function mergeAssets (parentVal, childVal) {
  var res = Object.create(parentVal || null);
  return childVal
    ? extend(res, childVal)
    : res
}

ASSET_TYPES.forEach(function (type) {
  strats[type + 's'] = mergeAssets;
});

/**
 * Watchers.
 *
 * Watchers hashes should not overwrite one
 * another, so we merge them as arrays.
 */
strats.watch = function (parentVal, childVal) {
  // work around Firefox's Object.prototype.watch...
  if (parentVal === nativeWatch) { parentVal = undefined; }
  if (childVal === nativeWatch) { childVal = undefined; }
  /* istanbul ignore if */
  if (!childVal) { return Object.create(parentVal || null) }
  if (!parentVal) { return childVal }
  var ret = {};
  extend(ret, parentVal);
  for (var key in childVal) {
    var parent = ret[key];
    var child = childVal[key];
    if (parent && !Array.isArray(parent)) {
      parent = [parent];
    }
    ret[key] = parent
      ? parent.concat(child)
      : Array.isArray(child) ? child : [child];
  }
  return ret
};

/**
 * Other object hashes.
 */
strats.props =
strats.methods =
strats.inject =
strats.computed = function (parentVal, childVal) {
  if (!parentVal) { return childVal }
  var ret = Object.create(null);
  extend(ret, parentVal);
  if (childVal) { extend(ret, childVal); }
  return ret
};
strats.provide = mergeDataOrFn;

/**
 * Default strategy.
 */
var defaultStrat = function (parentVal, childVal) {
  return childVal === undefined
    ? parentVal
    : childVal
};

/**
 * Validate component names
 */
function checkComponents (options) {
  for (var key in options.components) {
    var lower = key.toLowerCase();
    if (isBuiltInTag(lower) || config.isReservedTag(lower)) {
      warn(
        'Do not use built-in or reserved HTML elements as component ' +
        'id: ' + key
      );
    }
  }
}

/**
 * Ensure all props option syntax are normalized into the
 * Object-based format.
 */
function normalizeProps (options) {
  var props = options.props;
  if (!props) { return }
  var res = {};
  var i, val, name;
  if (Array.isArray(props)) {
    i = props.length;
    while (i--) {
      val = props[i];
      if (typeof val === 'string') {
        name = camelize(val);
        res[name] = { type: null };
      } else {
        warn('props must be strings when using array syntax.');
      }
    }
  } else if (isPlainObject(props)) {
    for (var key in props) {
      val = props[key];
      name = camelize(key);
      res[name] = isPlainObject(val)
        ? val
        : { type: val };
    }
  }
  options.props = res;
}

/**
 * Normalize all injections into Object-based format
 */
function normalizeInject (options) {
  var inject = options.inject;
  if (Array.isArray(inject)) {
    var normalized = options.inject = {};
    for (var i = 0; i < inject.length; i++) {
      normalized[inject[i]] = inject[i];
    }
  }
}

/**
 * Normalize raw function directives into object format.
 */
function normalizeDirectives (options) {
  var dirs = options.directives;
  if (dirs) {
    for (var key in dirs) {
      var def = dirs[key];
      if (typeof def === 'function') {
        dirs[key] = { bind: def, update: def };
      }
    }
  }
}

/**
 * Merge two option objects into a new one.
 * Core utility used in both instantiation and inheritance.
 */
function mergeOptions (
  parent,
  child,
  vm
) {
  {
    checkComponents(child);
  }

  if (typeof child === 'function') {
    child = child.options;
  }

  normalizeProps(child);
  normalizeInject(child);
  normalizeDirectives(child);
  var extendsFrom = child.extends;
  if (extendsFrom) {
    parent = mergeOptions(parent, extendsFrom, vm);
  }
  if (child.mixins) {
    for (var i = 0, l = child.mixins.length; i < l; i++) {
      parent = mergeOptions(parent, child.mixins[i], vm);
    }
  }
  var options = {};
  var key;
  for (key in parent) {
    mergeField(key);
  }
  for (key in child) {
    if (!hasOwn(parent, key)) {
      mergeField(key);
    }
  }
  function mergeField (key) {
    var strat = strats[key] || defaultStrat;
    options[key] = strat(parent[key], child[key], vm, key);
  }
  return options
}

/**
 * Resolve an asset.
 * This function is used because child instances need access
 * to assets defined in its ancestor chain.
 */
function resolveAsset (
  options,
  type,
  id,
  warnMissing
) {
  /* istanbul ignore if */
  if (typeof id !== 'string') {
    return
  }
  var assets = options[type];
  // check local registration variations first
  if (hasOwn(assets, id)) { return assets[id] }
  var camelizedId = camelize(id);
  if (hasOwn(assets, camelizedId)) { return assets[camelizedId] }
  var PascalCaseId = capitalize(camelizedId);
  if (hasOwn(assets, PascalCaseId)) { return assets[PascalCaseId] }
  // fallback to prototype chain
  var res = assets[id] || assets[camelizedId] || assets[PascalCaseId];
  if ("development" !== 'production' && warnMissing && !res) {
    warn(
      'Failed to resolve ' + type.slice(0, -1) + ': ' + id,
      options
    );
  }
  return res
}

/*  */

function validateProp (
  key,
  propOptions,
  propsData,
  vm
) {
  var prop = propOptions[key];
  var absent = !hasOwn(propsData, key);
  var value = propsData[key];
  // handle boolean props
  if (isType(Boolean, prop.type)) {
    if (absent && !hasOwn(prop, 'default')) {
      value = false;
    } else if (!isType(String, prop.type) && (value === '' || value === hyphenate(key))) {
      value = true;
    }
  }
  // check default value
  if (value === undefined) {
    value = getPropDefaultValue(vm, prop, key);
    // since the default value is a fresh copy,
    // make sure to observe it.
    var prevShouldConvert = observerState.shouldConvert;
    observerState.shouldConvert = true;
    observe(value);
    observerState.shouldConvert = prevShouldConvert;
  }
  {
    assertProp(prop, key, value, vm, absent);
  }
  return value
}

/**
 * Get the default value of a prop.
 */
function getPropDefaultValue (vm, prop, key) {
  // no default, return undefined
  if (!hasOwn(prop, 'default')) {
    return undefined
  }
  var def = prop.default;
  // warn against non-factory defaults for Object & Array
  if ("development" !== 'production' && isObject(def)) {
    warn(
      'Invalid default value for prop "' + key + '": ' +
      'Props with type Object/Array must use a factory function ' +
      'to return the default value.',
      vm
    );
  }
  // the raw prop value was also undefined from previous render,
  // return previous default value to avoid unnecessary watcher trigger
  if (vm && vm.$options.propsData &&
    vm.$options.propsData[key] === undefined &&
    vm._props[key] !== undefined
  ) {
    return vm._props[key]
  }
  // call factory function for non-Function types
  // a value is Function if its prototype is function even across different execution context
  return typeof def === 'function' && getType(prop.type) !== 'Function'
    ? def.call(vm)
    : def
}

/**
 * Assert whether a prop is valid.
 */
function assertProp (
  prop,
  name,
  value,
  vm,
  absent
) {
  if (prop.required && absent) {
    warn(
      'Missing required prop: "' + name + '"',
      vm
    );
    return
  }
  if (value == null && !prop.required) {
    return
  }
  var type = prop.type;
  var valid = !type || type === true;
  var expectedTypes = [];
  if (type) {
    if (!Array.isArray(type)) {
      type = [type];
    }
    for (var i = 0; i < type.length && !valid; i++) {
      var assertedType = assertType(value, type[i]);
      expectedTypes.push(assertedType.expectedType || '');
      valid = assertedType.valid;
    }
  }
  if (!valid) {
    warn(
      'Invalid prop: type check failed for prop "' + name + '".' +
      ' Expected ' + expectedTypes.map(capitalize).join(', ') +
      ', got ' + Object.prototype.toString.call(value).slice(8, -1) + '.',
      vm
    );
    return
  }
  var validator = prop.validator;
  if (validator) {
    if (!validator(value)) {
      warn(
        'Invalid prop: custom validator check failed for prop "' + name + '".',
        vm
      );
    }
  }
}

var simpleCheckRE = /^(String|Number|Boolean|Function|Symbol)$/;

function assertType (value, type) {
  var valid;
  var expectedType = getType(type);
  if (simpleCheckRE.test(expectedType)) {
    var t = typeof value;
    valid = t === expectedType.toLowerCase();
    // for primitive wrapper objects
    if (!valid && t === 'object') {
      valid = value instanceof type;
    }
  } else if (expectedType === 'Object') {
    valid = isPlainObject(value);
  } else if (expectedType === 'Array') {
    valid = Array.isArray(value);
  } else {
    valid = value instanceof type;
  }
  return {
    valid: valid,
    expectedType: expectedType
  }
}

/**
 * Use function string name to check built-in types,
 * because a simple equality check will fail when running
 * across different vms / iframes.
 */
function getType (fn) {
  var match = fn && fn.toString().match(/^\s*function (\w+)/);
  return match ? match[1] : ''
}

function isType (type, fn) {
  if (!Array.isArray(fn)) {
    return getType(fn) === getType(type)
  }
  for (var i = 0, len = fn.length; i < len; i++) {
    if (getType(fn[i]) === getType(type)) {
      return true
    }
  }
  /* istanbul ignore next */
  return false
}

/*  */

/* not type checking this file because flow doesn't play well with Proxy */

var initProxy;

{
  var allowedGlobals = makeMap(
    'Infinity,undefined,NaN,isFinite,isNaN,' +
    'parseFloat,parseInt,decodeURI,decodeURIComponent,encodeURI,encodeURIComponent,' +
    'Math,Number,Date,Array,Object,Boolean,String,RegExp,Map,Set,JSON,Intl,' +
    'require' // for Webpack/Browserify
  );

  var warnNonPresent = function (target, key) {
    warn(
      "Property or method \"" + key + "\" is not defined on the instance but " +
      "referenced during render. Make sure to declare reactive data " +
      "properties in the data option.",
      target
    );
  };

  var hasProxy =
    typeof Proxy !== 'undefined' &&
    Proxy.toString().match(/native code/);

  if (hasProxy) {
    var isBuiltInModifier = makeMap('stop,prevent,self,ctrl,shift,alt,meta');
    config.keyCodes = new Proxy(config.keyCodes, {
      set: function set (target, key, value) {
        if (isBuiltInModifier(key)) {
          warn(("Avoid overwriting built-in modifier in config.keyCodes: ." + key));
          return false
        } else {
          target[key] = value;
          return true
        }
      }
    });
  }

  var hasHandler = {
    has: function has (target, key) {
      var has = key in target;
      var isAllowed = allowedGlobals(key) || key.charAt(0) === '_';
      if (!has && !isAllowed) {
        warnNonPresent(target, key);
      }
      return has || !isAllowed
    }
  };

  var getHandler = {
    get: function get (target, key) {
      if (typeof key === 'string' && !(key in target)) {
        warnNonPresent(target, key);
      }
      return target[key]
    }
  };

  initProxy = function initProxy (vm) {
    if (hasProxy) {
      // determine which proxy handler to use
      var options = vm.$options;
      var handlers = options.render && options.render._withStripped
        ? getHandler
        : hasHandler;
      vm._renderProxy = new Proxy(vm, handlers);
    } else {
      vm._renderProxy = vm;
    }
  };
}

var mark;
var measure;

{
  var perf = inBrowser && window.performance;
  /* istanbul ignore if */
  if (
    perf &&
    perf.mark &&
    perf.measure &&
    perf.clearMarks &&
    perf.clearMeasures
  ) {
    mark = function (tag) { return perf.mark(tag); };
    measure = function (name, startTag, endTag) {
      perf.measure(name, startTag, endTag);
      perf.clearMarks(startTag);
      perf.clearMarks(endTag);
      perf.clearMeasures(name);
    };
  }
}

/*  */

var VNode = function VNode (
  tag,
  data,
  children,
  text,
  elm,
  context,
  componentOptions,
  asyncFactory
) {
  this.tag = tag;
  this.data = data;
  this.children = children;
  this.text = text;
  this.elm = elm;
  this.ns = undefined;
  this.context = context;
  this.functionalContext = undefined;
  this.key = data && data.key;
  this.componentOptions = componentOptions;
  this.componentInstance = undefined;
  this.parent = undefined;
  this.raw = false;
  this.isStatic = false;
  this.isRootInsert = true;
  this.isComment = false;
  this.isCloned = false;
  this.isOnce = false;
  this.asyncFactory = asyncFactory;
  this.asyncMeta = undefined;
  this.isAsyncPlaceholder = false;
};

var prototypeAccessors = { child: {} };

// DEPRECATED: alias for componentInstance for backwards compat.
/* istanbul ignore next */
prototypeAccessors.child.get = function () {
  return this.componentInstance
};

Object.defineProperties( VNode.prototype, prototypeAccessors );

var createEmptyVNode = function (text) {
  if ( text === void 0 ) { text = ''; }

  var node = new VNode();
  node.text = text;
  node.isComment = true;
  return node
};

function createTextVNode (val) {
  return new VNode(undefined, undefined, undefined, String(val))
}

// optimized shallow clone
// used for static nodes and slot nodes because they may be reused across
// multiple renders, cloning them avoids errors when DOM manipulations rely
// on their elm reference.
function cloneVNode (vnode, deep) {
  var cloned = new VNode(
    vnode.tag,
    vnode.data,
    vnode.children,
    vnode.text,
    vnode.elm,
    vnode.context,
    vnode.componentOptions,
    vnode.asyncFactory
  );
  cloned.ns = vnode.ns;
  cloned.isStatic = vnode.isStatic;
  cloned.key = vnode.key;
  cloned.isComment = vnode.isComment;
  cloned.isCloned = true;
  if (deep && vnode.children) {
    cloned.children = cloneVNodes(vnode.children);
  }
  return cloned
}

function cloneVNodes (vnodes, deep) {
  var len = vnodes.length;
  var res = new Array(len);
  for (var i = 0; i < len; i++) {
    res[i] = cloneVNode(vnodes[i], deep);
  }
  return res
}

/*  */

var normalizeEvent = cached(function (name) {
  var passive = name.charAt(0) === '&';
  name = passive ? name.slice(1) : name;
  var once$$1 = name.charAt(0) === '~'; // Prefixed last, checked first
  name = once$$1 ? name.slice(1) : name;
  var capture = name.charAt(0) === '!';
  name = capture ? name.slice(1) : name;
  var plain = !(passive || once$$1 || capture);
  return {
    name: name,
    plain: plain,
    once: once$$1,
    capture: capture,
    passive: passive
  }
});

function createFnInvoker (fns) {
  function invoker () {
    var arguments$1 = arguments;

    var fns = invoker.fns;
    if (Array.isArray(fns)) {
      var cloned = fns.slice();
      for (var i = 0; i < cloned.length; i++) {
        cloned[i].apply(null, arguments$1);
      }
    } else {
      // return handler return value for single handlers
      return fns.apply(null, arguments)
    }
  }
  invoker.fns = fns;
  return invoker
}

// #6552
function prioritizePlainEvents (a, b) {
  return a.plain ? -1 : b.plain ? 1 : 0
}

function updateListeners (
  on,
  oldOn,
  add,
  remove$$1,
  vm
) {
  var name, def$$1, cur, old, event;
  var toAdd = [];
  var hasModifier = false;
  for (name in on) {
    def$$1 = cur = on[name];
    old = oldOn[name];
    event = normalizeEvent(name);
    if (isPlainObject(def$$1)) {
      cur = def$$1.handler;
      event.params = def$$1.params;
    }
    if (!event.plain) { hasModifier = true; }
    if (isUndef(cur)) {
      "development" !== 'production' && warn(
        "Invalid handler for event \"" + (event.name) + "\": got " + String(cur),
        vm
      );
    } else if (isUndef(old)) {
      if (isUndef(cur.fns)) {
        cur = on[name] = createFnInvoker(cur);
      }
      event.handler = cur;
      toAdd.push(event);
    } else if (cur !== old) {
      old.fns = cur;
      on[name] = old;
    }
  }
  if (toAdd.length) {
    if (hasModifier) { toAdd.sort(prioritizePlainEvents); }
    for (var i = 0; i < toAdd.length; i++) {
      var event$1 = toAdd[i];
      add(event$1.name, event$1.handler, event$1.once, event$1.capture, event$1.passive, event$1.params);
    }
  }
  for (name in oldOn) {
    if (isUndef(on[name])) {
      event = normalizeEvent(name);
      remove$$1(event.name, oldOn[name], event.capture);
    }
  }
}

/*  */

function mergeVNodeHook (def, hookKey, hook) {
  var invoker;
  var oldHook = def[hookKey];

  function wrappedHook () {
    hook.apply(this, arguments);
    // important: remove merged hook to ensure it's called only once
    // and prevent memory leak
    remove(invoker.fns, wrappedHook);
  }

  if (isUndef(oldHook)) {
    // no existing hook
    invoker = createFnInvoker([wrappedHook]);
  } else {
    /* istanbul ignore if */
    if (isDef(oldHook.fns) && isTrue(oldHook.merged)) {
      // already a merged invoker
      invoker = oldHook;
      invoker.fns.push(wrappedHook);
    } else {
      // existing plain hook
      invoker = createFnInvoker([oldHook, wrappedHook]);
    }
  }

  invoker.merged = true;
  def[hookKey] = invoker;
}

/*  */

function extractPropsFromVNodeData (
  data,
  Ctor,
  tag
) {
  // we are only extracting raw values here.
  // validation and default values are handled in the child
  // component itself.
  var propOptions = Ctor.options.props;
  if (isUndef(propOptions)) {
    return
  }
  var res = {};
  var attrs = data.attrs;
  var props = data.props;
  if (isDef(attrs) || isDef(props)) {
    for (var key in propOptions) {
      var altKey = hyphenate(key);
      {
        var keyInLowerCase = key.toLowerCase();
        if (
          key !== keyInLowerCase &&
          attrs && hasOwn(attrs, keyInLowerCase)
        ) {
          tip(
            "Prop \"" + keyInLowerCase + "\" is passed to component " +
            (formatComponentName(tag || Ctor)) + ", but the declared prop name is" +
            " \"" + key + "\". " +
            "Note that HTML attributes are case-insensitive and camelCased " +
            "props need to use their kebab-case equivalents when using in-DOM " +
            "templates. You should probably use \"" + altKey + "\" instead of \"" + key + "\"."
          );
        }
      }
      checkProp(res, props, key, altKey, true) ||
      checkProp(res, attrs, key, altKey, false);
    }
  }
  return res
}

function checkProp (
  res,
  hash,
  key,
  altKey,
  preserve
) {
  if (isDef(hash)) {
    if (hasOwn(hash, key)) {
      res[key] = hash[key];
      if (!preserve) {
        delete hash[key];
      }
      return true
    } else if (hasOwn(hash, altKey)) {
      res[key] = hash[altKey];
      if (!preserve) {
        delete hash[altKey];
      }
      return true
    }
  }
  return false
}

/*  */

// The template compiler attempts to minimize the need for normalization by
// statically analyzing the template at compile time.
//
// For plain HTML markup, normalization can be completely skipped because the
// generated render function is guaranteed to return Array<VNode>. There are
// two cases where extra normalization is needed:

// 1. When the children contains components - because a functional component
// may return an Array instead of a single root. In this case, just a simple
// normalization is needed - if any child is an Array, we flatten the whole
// thing with Array.prototype.concat. It is guaranteed to be only 1-level deep
// because functional components already normalize their own children.
function simpleNormalizeChildren (children) {
  for (var i = 0; i < children.length; i++) {
    if (Array.isArray(children[i])) {
      return Array.prototype.concat.apply([], children)
    }
  }
  return children
}

// 2. When the children contains constructs that always generated nested Arrays,
// e.g. <template>, <slot>, v-for, or when the children is provided by user
// with hand-written render functions / JSX. In such cases a full normalization
// is needed to cater to all possible types of children values.
function normalizeChildren (children) {
  return isPrimitive(children)
    ? [createTextVNode(children)]
    : Array.isArray(children)
      ? normalizeArrayChildren(children)
      : undefined
}

function isTextNode (node) {
  return isDef(node) && isDef(node.text) && isFalse(node.isComment)
}

function normalizeArrayChildren (children, nestedIndex) {
  var res = [];
  var i, c, last;
  for (i = 0; i < children.length; i++) {
    c = children[i];
    if (isUndef(c) || typeof c === 'boolean') { continue }
    last = res[res.length - 1];
    //  nested
    if (Array.isArray(c)) {
      res.push.apply(res, normalizeArrayChildren(c, ((nestedIndex || '') + "_" + i)));
    } else if (isPrimitive(c)) {
      if (isTextNode(last)) {
        // merge adjacent text nodes
        // this is necessary for SSR hydration because text nodes are
        // essentially merged when rendered to HTML strings
        (last).text += String(c);
      } else if (c !== '') {
        // convert primitive to vnode
        res.push(createTextVNode(c));
      }
    } else {
      if (isTextNode(c) && isTextNode(last)) {
        // merge adjacent text nodes
        res[res.length - 1] = createTextVNode(last.text + c.text);
      } else {
        // default key for nested array children (likely generated by v-for)
        if (isTrue(children._isVList) &&
          isDef(c.tag) &&
          isUndef(c.key) &&
          isDef(nestedIndex)) {
          c.key = "__vlist" + nestedIndex + "_" + i + "__";
        }
        res.push(c);
      }
    }
  }
  return res
}

/*  */

function ensureCtor (comp, base) {
  if (comp.__esModule && comp.default) {
    comp = comp.default;
  }
  return isObject(comp)
    ? base.extend(comp)
    : comp
}

function createAsyncPlaceholder (
  factory,
  data,
  context,
  children,
  tag
) {
  var node = createEmptyVNode();
  node.asyncFactory = factory;
  node.asyncMeta = { data: data, context: context, children: children, tag: tag };
  return node
}

function resolveAsyncComponent (
  factory,
  baseCtor,
  context
) {
  if (isTrue(factory.error) && isDef(factory.errorComp)) {
    return factory.errorComp
  }

  if (isDef(factory.resolved)) {
    return factory.resolved
  }

  if (isTrue(factory.loading) && isDef(factory.loadingComp)) {
    return factory.loadingComp
  }

  if (isDef(factory.contexts)) {
    // already pending
    factory.contexts.push(context);
  } else {
    var contexts = factory.contexts = [context];
    var sync = true;

    var forceRender = function () {
      for (var i = 0, l = contexts.length; i < l; i++) {
        contexts[i].$forceUpdate();
      }
    };

    var resolve = once(function (res) {
      // cache resolved
      factory.resolved = ensureCtor(res, baseCtor);
      // invoke callbacks only if this is not a synchronous resolve
      // (async resolves are shimmed as synchronous during SSR)
      if (!sync) {
        forceRender();
      }
    });

    var reject = once(function (reason) {
      "development" !== 'production' && warn(
        "Failed to resolve async component: " + (String(factory)) +
        (reason ? ("\nReason: " + reason) : '')
      );
      if (isDef(factory.errorComp)) {
        factory.error = true;
        forceRender();
      }
    });

    var res = factory(resolve, reject);

    if (isObject(res)) {
      if (typeof res.then === 'function') {
        // () => Promise
        if (isUndef(factory.resolved)) {
          res.then(resolve, reject);
        }
      } else if (isDef(res.component) && typeof res.component.then === 'function') {
        res.component.then(resolve, reject);

        if (isDef(res.error)) {
          factory.errorComp = ensureCtor(res.error, baseCtor);
        }

        if (isDef(res.loading)) {
          factory.loadingComp = ensureCtor(res.loading, baseCtor);
          if (res.delay === 0) {
            factory.loading = true;
          } else {
            setTimeout(function () {
              if (isUndef(factory.resolved) && isUndef(factory.error)) {
                factory.loading = true;
                forceRender();
              }
            }, res.delay || 200);
          }
        }

        if (isDef(res.timeout)) {
          setTimeout(function () {
            if (isUndef(factory.resolved)) {
              reject(
                ("timeout (" + (res.timeout) + "ms)")
              );
            }
          }, res.timeout);
        }
      }
    }

    sync = false;
    // return in case resolved synchronously
    return factory.loading
      ? factory.loadingComp
      : factory.resolved
  }
}

/*  */

function isAsyncPlaceholder (node) {
  return node.isComment && node.asyncFactory
}

/*  */

function getFirstComponentChild (children) {
  if (Array.isArray(children)) {
    for (var i = 0; i < children.length; i++) {
      var c = children[i];
      if (isDef(c) && (isDef(c.componentOptions) || isAsyncPlaceholder(c))) {
        return c
      }
    }
  }
}

/*  */

/*  */

function initEvents (vm) {
  vm._events = Object.create(null);
  vm._hasHookEvent = false;
  // init parent attached events
  var listeners = vm.$options._parentListeners;
  if (listeners) {
    updateComponentListeners(vm, listeners);
  }
}

var target;

function add (event, fn, once$$1) {
  if (once$$1) {
    target.$once(event, fn);
  } else {
    target.$on(event, fn);
  }
}

function remove$1 (event, fn) {
  target.$off(event, fn);
}

function updateComponentListeners (
  vm,
  listeners,
  oldListeners
) {
  target = vm;
  updateListeners(listeners, oldListeners || {}, add, remove$1, vm);
}

function eventsMixin (Vue) {
  var hookRE = /^hook:/;
  Vue.prototype.$on = function (event, fn) {
    var this$1 = this;

    var vm = this;
    if (Array.isArray(event)) {
      for (var i = 0, l = event.length; i < l; i++) {
        this$1.$on(event[i], fn);
      }
    } else {
      (vm._events[event] || (vm._events[event] = [])).push(fn);
      // optimize hook:event cost by using a boolean flag marked at registration
      // instead of a hash lookup
      if (hookRE.test(event)) {
        vm._hasHookEvent = true;
      }
    }
    return vm
  };

  Vue.prototype.$once = function (event, fn) {
    var vm = this;
    function on () {
      vm.$off(event, on);
      fn.apply(vm, arguments);
    }
    on.fn = fn;
    vm.$on(event, on);
    return vm
  };

  Vue.prototype.$off = function (event, fn) {
    var this$1 = this;

    var vm = this;
    // all
    if (!arguments.length) {
      vm._events = Object.create(null);
      return vm
    }
    // array of events
    if (Array.isArray(event)) {
      for (var i = 0, l = event.length; i < l; i++) {
        this$1.$off(event[i], fn);
      }
      return vm
    }
    // specific event
    var cbs = vm._events[event];
    if (!cbs) {
      return vm
    }
    if (arguments.length === 1) {
      vm._events[event] = null;
      return vm
    }
    if (fn) {
      // specific handler
      var cb;
      var i$1 = cbs.length;
      while (i$1--) {
        cb = cbs[i$1];
        if (cb === fn || cb.fn === fn) {
          cbs.splice(i$1, 1);
          break
        }
      }
    }
    return vm
  };

  Vue.prototype.$emit = function (event) {
    var vm = this;
    {
      var lowerCaseEvent = event.toLowerCase();
      if (lowerCaseEvent !== event && vm._events[lowerCaseEvent]) {
        tip(
          "Event \"" + lowerCaseEvent + "\" is emitted in component " +
          (formatComponentName(vm)) + " but the handler is registered for \"" + event + "\". " +
          "Note that HTML attributes are case-insensitive and you cannot use " +
          "v-on to listen to camelCase events when using in-DOM templates. " +
          "You should probably use \"" + (hyphenate(event)) + "\" instead of \"" + event + "\"."
        );
      }
    }
    var cbs = vm._events[event];
    if (cbs) {
      cbs = cbs.length > 1 ? toArray(cbs) : cbs;
      var args = toArray(arguments, 1);
      for (var i = 0, l = cbs.length; i < l; i++) {
        try {
          cbs[i].apply(vm, args);
        } catch (e) {
          handleError(e, vm, ("event handler for \"" + event + "\""));
        }
      }
    }
    return vm
  };
}

/*  */

/**
 * Runtime helper for resolving raw children VNodes into a slot object.
 */
function resolveSlots (
  children,
  context
) {
  var slots = {};
  if (!children) {
    return slots
  }
  var defaultSlot = [];
  for (var i = 0, l = children.length; i < l; i++) {
    var child = children[i];
    var data = child.data;
    // remove slot attribute if the node is resolved as a Vue slot node
    if (data && data.attrs && data.attrs.slot) {
      delete data.attrs.slot;
    }
    // named slots should only be respected if the vnode was rendered in the
    // same context.
    if ((child.context === context || child.functionalContext === context) &&
      data && data.slot != null
    ) {
      var name = child.data.slot;
      var slot = (slots[name] || (slots[name] = []));
      if (child.tag === 'template') {
        slot.push.apply(slot, child.children);
      } else {
        slot.push(child);
      }
    } else {
      defaultSlot.push(child);
    }
  }
  // ignore whitespace
  if (!defaultSlot.every(isWhitespace)) {
    slots.default = defaultSlot;
  }
  return slots
}

function isWhitespace (node) {
  return node.isComment || node.text === ' '
}

function resolveScopedSlots (
  fns, // see flow/vnode
  res
) {
  res = res || {};
  for (var i = 0; i < fns.length; i++) {
    if (Array.isArray(fns[i])) {
      resolveScopedSlots(fns[i], res);
    } else {
      res[fns[i].key] = fns[i].fn;
    }
  }
  return res
}

/*  */

var activeInstance = null;
var isUpdatingChildComponent = false;

function initLifecycle (vm) {
  var options = vm.$options;

  // locate first non-abstract parent
  var parent = options.parent;
  if (parent && !options.abstract) {
    while (parent.$options.abstract && parent.$parent) {
      parent = parent.$parent;
    }
    parent.$children.push(vm);
  }

  vm.$parent = parent;
  vm.$root = parent ? parent.$root : vm;

  vm.$children = [];
  vm.$refs = {};

  vm._watcher = null;
  vm._inactive = null;
  vm._directInactive = false;
  vm._isMounted = false;
  vm._isDestroyed = false;
  vm._isBeingDestroyed = false;
}

function lifecycleMixin (Vue) {
  Vue.prototype._update = function (vnode, hydrating) {
    var vm = this;
    if (vm._isMounted) {
      callHook(vm, 'beforeUpdate');
    }
    var prevEl = vm.$el;
    var prevVnode = vm._vnode;
    var prevActiveInstance = activeInstance;
    activeInstance = vm;
    vm._vnode = vnode;
    // Vue.prototype.__patch__ is injected in entry points
    // based on the rendering backend used.
    if (!prevVnode) {
      // initial render
      vm.$el = vm.__patch__(
        vm.$el, vnode, hydrating, false /* removeOnly */,
        vm.$options._parentElm,
        vm.$options._refElm
      );
      // no need for the ref nodes after initial patch
      // this prevents keeping a detached DOM tree in memory (#5851)
      vm.$options._parentElm = vm.$options._refElm = null;
    } else {
      // updates
      vm.$el = vm.__patch__(prevVnode, vnode);
    }
    activeInstance = prevActiveInstance;
    // update __vue__ reference
    if (prevEl) {
      prevEl.__vue__ = null;
    }
    if (vm.$el) {
      vm.$el.__vue__ = vm;
    }
    // if parent is an HOC, update its $el as well
    if (vm.$vnode && vm.$parent && vm.$vnode === vm.$parent._vnode) {
      vm.$parent.$el = vm.$el;
    }
    // updated hook is called by the scheduler to ensure that children are
    // updated in a parent's updated hook.
  };

  Vue.prototype.$forceUpdate = function () {
    var vm = this;
    if (vm._watcher) {
      vm._watcher.update();
    }
  };

  Vue.prototype.$destroy = function () {
    var vm = this;
    if (vm._isBeingDestroyed) {
      return
    }
    callHook(vm, 'beforeDestroy');
    vm._isBeingDestroyed = true;
    // remove self from parent
    var parent = vm.$parent;
    if (parent && !parent._isBeingDestroyed && !vm.$options.abstract) {
      remove(parent.$children, vm);
    }
    // teardown watchers
    if (vm._watcher) {
      vm._watcher.teardown();
    }
    var i = vm._watchers.length;
    while (i--) {
      vm._watchers[i].teardown();
    }
    // remove reference from data ob
    // frozen object may not have observer.
    if (vm._data.__ob__) {
      vm._data.__ob__.vmCount--;
    }
    // call the last hook...
    vm._isDestroyed = true;
    // invoke destroy hooks on current rendered tree
    vm.__patch__(vm._vnode, null);
    // fire destroyed hook
    callHook(vm, 'destroyed');
    // turn off all instance listeners.
    vm.$off();
    // remove __vue__ reference
    if (vm.$el) {
      vm.$el.__vue__ = null;
    }
  };
}

function mountComponent (
  vm,
  el,
  hydrating
) {
  vm.$el = el;
  if (!vm.$options.render) {
    vm.$options.render = createEmptyVNode;
    {
      /* istanbul ignore if */
      if ((vm.$options.template && vm.$options.template.charAt(0) !== '#') ||
        vm.$options.el || el) {
        warn(
          'You are using the runtime-only build of Vue where the template ' +
          'compiler is not available. Either pre-compile the templates into ' +
          'render functions, or use the compiler-included build.',
          vm
        );
      } else {
        warn(
          'Failed to mount component: template or render function not defined.',
          vm
        );
      }
    }
  }
  callHook(vm, 'beforeMount');

  var updateComponent;
  /* istanbul ignore if */
  if ("development" !== 'production' && config.performance && mark) {
    updateComponent = function () {
      var name = vm._name;
      var id = vm._uid;
      var startTag = "vue-perf-start:" + id;
      var endTag = "vue-perf-end:" + id;

      mark(startTag);
      var vnode = vm._render();
      mark(endTag);
      measure((name + " render"), startTag, endTag);

      mark(startTag);
      vm._update(vnode, hydrating);
      mark(endTag);
      measure((name + " patch"), startTag, endTag);
    };
  } else {
    updateComponent = function () {
      vm._update(vm._render(), hydrating);
    };
  }

  vm._watcher = new Watcher(vm, updateComponent, noop);
  hydrating = false;

  // manually mounted instance, call mounted on self
  // mounted is called for render-created child components in its inserted hook
  if (vm.$vnode == null) {
    vm._isMounted = true;
    callHook(vm, 'mounted');
  }
  return vm
}

function updateChildComponent (
  vm,
  propsData,
  listeners,
  parentVnode,
  renderChildren
) {
  {
    isUpdatingChildComponent = true;
  }

  // determine whether component has slot children
  // we need to do this before overwriting $options._renderChildren
  var hasChildren = !!(
    renderChildren ||               // has new static slots
    vm.$options._renderChildren ||  // has old static slots
    parentVnode.data.scopedSlots || // has new scoped slots
    vm.$scopedSlots !== emptyObject // has old scoped slots
  );

  vm.$options._parentVnode = parentVnode;
  vm.$vnode = parentVnode; // update vm's placeholder node without re-render

  if (vm._vnode) { // update child tree's parent
    vm._vnode.parent = parentVnode;
  }
  vm.$options._renderChildren = renderChildren;

  // update $attrs and $listeners hash
  // these are also reactive so they may trigger child update if the child
  // used them during render
  vm.$attrs = (parentVnode.data && parentVnode.data.attrs) || emptyObject;
  vm.$listeners = listeners || emptyObject;

  // update props
  if (propsData && vm.$options.props) {
    observerState.shouldConvert = false;
    var props = vm._props;
    var propKeys = vm.$options._propKeys || [];
    for (var i = 0; i < propKeys.length; i++) {
      var key = propKeys[i];
      props[key] = validateProp(key, vm.$options.props, propsData, vm);
    }
    observerState.shouldConvert = true;
    // keep a copy of raw propsData
    vm.$options.propsData = propsData;
  }

  // update listeners
  if (listeners) {
    var oldListeners = vm.$options._parentListeners;
    vm.$options._parentListeners = listeners;
    updateComponentListeners(vm, listeners, oldListeners);
  }
  // resolve slots + force update if has children
  if (hasChildren) {
    vm.$slots = resolveSlots(renderChildren, parentVnode.context);
    vm.$forceUpdate();
  }

  {
    isUpdatingChildComponent = false;
  }
}

function isInInactiveTree (vm) {
  while (vm && (vm = vm.$parent)) {
    if (vm._inactive) { return true }
  }
  return false
}

function activateChildComponent (vm, direct) {
  if (direct) {
    vm._directInactive = false;
    if (isInInactiveTree(vm)) {
      return
    }
  } else if (vm._directInactive) {
    return
  }
  if (vm._inactive || vm._inactive === null) {
    vm._inactive = false;
    for (var i = 0; i < vm.$children.length; i++) {
      activateChildComponent(vm.$children[i]);
    }
    callHook(vm, 'activated');
  }
}

function deactivateChildComponent (vm, direct) {
  if (direct) {
    vm._directInactive = true;
    if (isInInactiveTree(vm)) {
      return
    }
  }
  if (!vm._inactive) {
    vm._inactive = true;
    for (var i = 0; i < vm.$children.length; i++) {
      deactivateChildComponent(vm.$children[i]);
    }
    callHook(vm, 'deactivated');
  }
}

function callHook (vm, hook) {
  var handlers = vm.$options[hook];
  if (handlers) {
    for (var i = 0, j = handlers.length; i < j; i++) {
      try {
        handlers[i].call(vm);
      } catch (e) {
        handleError(e, vm, (hook + " hook"));
      }
    }
  }
  if (vm._hasHookEvent) {
    vm.$emit('hook:' + hook);
  }
}

/*  */


var MAX_UPDATE_COUNT = 100;

var queue = [];
var activatedChildren = [];
var has = {};
var circular = {};
var waiting = false;
var flushing = false;
var index = 0;

/**
 * Reset the scheduler's state.
 */
function resetSchedulerState () {
  index = queue.length = activatedChildren.length = 0;
  has = {};
  {
    circular = {};
  }
  waiting = flushing = false;
}

/**
 * Flush both queues and run the watchers.
 */
function flushSchedulerQueue () {
  flushing = true;
  var watcher, id;

  // Sort queue before flush.
  // This ensures that:
  // 1. Components are updated from parent to child. (because parent is always
  //    created before the child)
  // 2. A component's user watchers are run before its render watcher (because
  //    user watchers are created before the render watcher)
  // 3. If a component is destroyed during a parent component's watcher run,
  //    its watchers can be skipped.
  queue.sort(function (a, b) { return a.id - b.id; });

  // do not cache length because more watchers might be pushed
  // as we run existing watchers
  for (index = 0; index < queue.length; index++) {
    watcher = queue[index];
    id = watcher.id;
    has[id] = null;
    watcher.run();
    // in dev build, check and stop circular updates.
    if ("development" !== 'production' && has[id] != null) {
      circular[id] = (circular[id] || 0) + 1;
      if (circular[id] > MAX_UPDATE_COUNT) {
        warn(
          'You may have an infinite update loop ' + (
            watcher.user
              ? ("in watcher with expression \"" + (watcher.expression) + "\"")
              : "in a component render function."
          ),
          watcher.vm
        );
        break
      }
    }
  }

  // keep copies of post queues before resetting state
  var activatedQueue = activatedChildren.slice();
  var updatedQueue = queue.slice();

  resetSchedulerState();

  // call component updated and activated hooks
  callActivatedHooks(activatedQueue);
  callUpdatedHooks(updatedQueue);

  // devtool hook
  /* istanbul ignore if */
  if (devtools && config.devtools) {
    devtools.emit('flush');
  }
}

function callUpdatedHooks (queue) {
  var i = queue.length;
  while (i--) {
    var watcher = queue[i];
    var vm = watcher.vm;
    if (vm._watcher === watcher && vm._isMounted) {
      callHook(vm, 'updated');
    }
  }
}

/**
 * Queue a kept-alive component that was activated during patch.
 * The queue will be processed after the entire tree has been patched.
 */
function queueActivatedComponent (vm) {
  // setting _inactive to false here so that a render function can
  // rely on checking whether it's in an inactive tree (e.g. router-view)
  vm._inactive = false;
  activatedChildren.push(vm);
}

function callActivatedHooks (queue) {
  for (var i = 0; i < queue.length; i++) {
    queue[i]._inactive = true;
    activateChildComponent(queue[i], true /* true */);
  }
}

/**
 * Push a watcher into the watcher queue.
 * Jobs with duplicate IDs will be skipped unless it's
 * pushed when the queue is being flushed.
 */
function queueWatcher (watcher) {
  var id = watcher.id;
  if (has[id] == null) {
    has[id] = true;
    if (!flushing) {
      queue.push(watcher);
    } else {
      // if already flushing, splice the watcher based on its id
      // if already past its id, it will be run next immediately.
      var i = queue.length - 1;
      while (i > index && queue[i].id > watcher.id) {
        i--;
      }
      queue.splice(i + 1, 0, watcher);
    }
    // queue the flush
    if (!waiting) {
      waiting = true;
      nextTick(flushSchedulerQueue);
    }
  }
}

/*  */

var uid$2 = 0;

/**
 * A watcher parses an expression, collects dependencies,
 * and fires callback when the expression value changes.
 * This is used for both the $watch() api and directives.
 */
var Watcher = function Watcher (
  vm,
  expOrFn,
  cb,
  options
) {
  this.vm = vm;
  vm._watchers.push(this);
  // options
  if (options) {
    this.deep = !!options.deep;
    this.user = !!options.user;
    this.lazy = !!options.lazy;
    this.sync = !!options.sync;
  } else {
    this.deep = this.user = this.lazy = this.sync = false;
  }
  this.cb = cb;
  this.id = ++uid$2; // uid for batching
  this.active = true;
  this.dirty = this.lazy; // for lazy watchers
  this.deps = [];
  this.newDeps = [];
  this.depIds = new _Set();
  this.newDepIds = new _Set();
  this.expression = expOrFn.toString();
  // parse expression for getter
  if (typeof expOrFn === 'function') {
    this.getter = expOrFn;
  } else {
    this.getter = parsePath(expOrFn);
    if (!this.getter) {
      this.getter = function () {};
      "development" !== 'production' && warn(
        "Failed watching path: \"" + expOrFn + "\" " +
        'Watcher only accepts simple dot-delimited paths. ' +
        'For full control, use a function instead.',
        vm
      );
    }
  }
  this.value = this.lazy
    ? undefined
    : this.get();
};

/**
 * Evaluate the getter, and re-collect dependencies.
 */
Watcher.prototype.get = function get () {
  pushTarget(this);
  var value;
  var vm = this.vm;
  try {
    value = this.getter.call(vm, vm);
  } catch (e) {
    if (this.user) {
      handleError(e, vm, ("getter for watcher \"" + (this.expression) + "\""));
    } else {
      throw e
    }
  } finally {
    // "touch" every property so they are all tracked as
    // dependencies for deep watching
    if (this.deep) {
      traverse(value);
    }
    popTarget();
    this.cleanupDeps();
  }
  return value
};

/**
 * Add a dependency to this directive.
 */
Watcher.prototype.addDep = function addDep (dep) {
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
Watcher.prototype.cleanupDeps = function cleanupDeps () {
    var this$1 = this;

  var i = this.deps.length;
  while (i--) {
    var dep = this$1.deps[i];
    if (!this$1.newDepIds.has(dep.id)) {
      dep.removeSub(this$1);
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
 */
Watcher.prototype.update = function update () {
  /* istanbul ignore else */
  if (this.lazy) {
    this.dirty = true;
  } else if (this.sync) {
    this.run();
  } else {
    queueWatcher(this);
  }
};

/**
 * Scheduler job interface.
 * Will be called by the scheduler.
 */
Watcher.prototype.run = function run () {
  if (this.active) {
    var value = this.get();
    if (
      value !== this.value ||
      // Deep watchers and watchers on Object/Arrays should fire even
      // when the value is the same, because the value may
      // have mutated.
      isObject(value) ||
      this.deep
    ) {
      // set new value
      var oldValue = this.value;
      this.value = value;
      if (this.user) {
        try {
          this.cb.call(this.vm, value, oldValue);
        } catch (e) {
          handleError(e, this.vm, ("callback for watcher \"" + (this.expression) + "\""));
        }
      } else {
        this.cb.call(this.vm, value, oldValue);
      }
    }
  }
};

/**
 * Evaluate the value of the watcher.
 * This only gets called for lazy watchers.
 */
Watcher.prototype.evaluate = function evaluate () {
  this.value = this.get();
  this.dirty = false;
};

/**
 * Depend on all deps collected by this watcher.
 */
Watcher.prototype.depend = function depend () {
    var this$1 = this;

  var i = this.deps.length;
  while (i--) {
    this$1.deps[i].depend();
  }
};

/**
 * Remove self from all dependencies' subscriber list.
 */
Watcher.prototype.teardown = function teardown () {
    var this$1 = this;

  if (this.active) {
    // remove self from vm's watcher list
    // this is a somewhat expensive operation so we skip it
    // if the vm is being destroyed.
    if (!this.vm._isBeingDestroyed) {
      remove(this.vm._watchers, this);
    }
    var i = this.deps.length;
    while (i--) {
      this$1.deps[i].removeSub(this$1);
    }
    this.active = false;
  }
};

/**
 * Recursively traverse an object to evoke all converted
 * getters, so that every nested property inside the object
 * is collected as a "deep" dependency.
 */
var seenObjects = new _Set();
function traverse (val) {
  seenObjects.clear();
  _traverse(val, seenObjects);
}

function _traverse (val, seen) {
  var i, keys;
  var isA = Array.isArray(val);
  if ((!isA && !isObject(val)) || !Object.isExtensible(val)) {
    return
  }
  if (val.__ob__) {
    var depId = val.__ob__.dep.id;
    if (seen.has(depId)) {
      return
    }
    seen.add(depId);
  }
  if (isA) {
    i = val.length;
    while (i--) { _traverse(val[i], seen); }
  } else {
    keys = Object.keys(val);
    i = keys.length;
    while (i--) { _traverse(val[keys[i]], seen); }
  }
}

/*  */

var sharedPropertyDefinition = {
  enumerable: true,
  configurable: true,
  get: noop,
  set: noop
};

function proxy (target, sourceKey, key) {
  sharedPropertyDefinition.get = function proxyGetter () {
    return this[sourceKey][key]
  };
  sharedPropertyDefinition.set = function proxySetter (val) {
    this[sourceKey][key] = val;
  };
  Object.defineProperty(target, key, sharedPropertyDefinition);
}

function initState (vm) {
  vm._watchers = [];
  var opts = vm.$options;
  if (opts.props) { initProps(vm, opts.props); }
  if (opts.methods) { initMethods(vm, opts.methods); }
  if (opts.data) {
    initData(vm);
  } else {
    observe(vm._data = {}, true /* asRootData */);
  }
  if (opts.computed) { initComputed(vm, opts.computed); }
  if (opts.watch && opts.watch !== nativeWatch) {
    initWatch(vm, opts.watch);
  }
}

function checkOptionType (vm, name) {
  var option = vm.$options[name];
  if (!isPlainObject(option)) {
    warn(
      ("component option \"" + name + "\" should be an object."),
      vm
    );
  }
}

function initProps (vm, propsOptions) {
  var propsData = vm.$options.propsData || {};
  var props = vm._props = {};
  // cache prop keys so that future props updates can iterate using Array
  // instead of dynamic object key enumeration.
  var keys = vm.$options._propKeys = [];
  var isRoot = !vm.$parent;
  // root instance props should be converted
  observerState.shouldConvert = isRoot;
  var loop = function ( key ) {
    keys.push(key);
    var value = validateProp(key, propsOptions, propsData, vm);
    /* istanbul ignore else */
    {
      if (isReservedAttribute(key) || config.isReservedAttr(key)) {
        warn(
          ("\"" + key + "\" is a reserved attribute and cannot be used as component prop."),
          vm
        );
      }
      defineReactive$$1(props, key, value, function () {
        if (vm.$parent && !isUpdatingChildComponent) {
          warn(
            "Avoid mutating a prop directly since the value will be " +
            "overwritten whenever the parent component re-renders. " +
            "Instead, use a data or computed property based on the prop's " +
            "value. Prop being mutated: \"" + key + "\"",
            vm
          );
        }
      });
    }
    // static props are already proxied on the component's prototype
    // during Vue.extend(). We only need to proxy props defined at
    // instantiation here.
    if (!(key in vm)) {
      proxy(vm, "_props", key);
    }
  };

  for (var key in propsOptions) { loop( key ); }
  observerState.shouldConvert = true;
}

function initData (vm) {
  var data = vm.$options.data;
  data = vm._data = typeof data === 'function'
    ? getData(data, vm)
    : data || {};
  if (!isPlainObject(data)) {
    data = {};
    "development" !== 'production' && warn(
      'data functions should return an object:\n' +
      'https://vuejs.org/v2/guide/components.html#data-Must-Be-a-Function',
      vm
    );
  }
  // proxy data on instance
  var keys = Object.keys(data);
  var props = vm.$options.props;
  var methods = vm.$options.methods;
  var i = keys.length;
  while (i--) {
    var key = keys[i];
    {
      if (methods && hasOwn(methods, key)) {
        warn(
          ("Method \"" + key + "\" has already been defined as a data property."),
          vm
        );
      }
    }
    if (props && hasOwn(props, key)) {
      "development" !== 'production' && warn(
        "The data property \"" + key + "\" is already declared as a prop. " +
        "Use prop default value instead.",
        vm
      );
    } else if (!isReserved(key)) {
      proxy(vm, "_data", key);
    }
  }
  // observe data
  observe(data, true /* asRootData */);
}

function getData (data, vm) {
  try {
    return data.call(vm)
  } catch (e) {
    handleError(e, vm, "data()");
    return {}
  }
}

var computedWatcherOptions = { lazy: true };

function initComputed (vm, computed) {
  "development" !== 'production' && checkOptionType(vm, 'computed');
  var watchers = vm._computedWatchers = Object.create(null);
  // computed properties are just getters during SSR
  var isSSR = isServerRendering();

  for (var key in computed) {
    var userDef = computed[key];
    var getter = typeof userDef === 'function' ? userDef : userDef.get;
    if ("development" !== 'production' && getter == null) {
      warn(
        ("Getter is missing for computed property \"" + key + "\"."),
        vm
      );
    }

    if (!isSSR) {
      // create internal watcher for the computed property.
      watchers[key] = new Watcher(
        vm,
        getter || noop,
        noop,
        computedWatcherOptions
      );
    }

    // component-defined computed properties are already defined on the
    // component prototype. We only need to define computed properties defined
    // at instantiation here.
    if (!(key in vm)) {
      defineComputed(vm, key, userDef);
    } else {
      if (key in vm.$data) {
        warn(("The computed property \"" + key + "\" is already defined in data."), vm);
      } else if (vm.$options.props && key in vm.$options.props) {
        warn(("The computed property \"" + key + "\" is already defined as a prop."), vm);
      }
    }
  }
}

function defineComputed (
  target,
  key,
  userDef
) {
  var shouldCache = !isServerRendering();
  if (typeof userDef === 'function') {
    sharedPropertyDefinition.get = shouldCache
      ? createComputedGetter(key)
      : userDef;
    sharedPropertyDefinition.set = noop;
  } else {
    sharedPropertyDefinition.get = userDef.get
      ? shouldCache && userDef.cache !== false
        ? createComputedGetter(key)
        : userDef.get
      : noop;
    sharedPropertyDefinition.set = userDef.set
      ? userDef.set
      : noop;
  }
  if ("development" !== 'production' &&
      sharedPropertyDefinition.set === noop) {
    sharedPropertyDefinition.set = function () {
      warn(
        ("Computed property \"" + key + "\" was assigned to but it has no setter."),
        this
      );
    };
  }
  Object.defineProperty(target, key, sharedPropertyDefinition);
}

function createComputedGetter (key) {
  return function computedGetter () {
    var watcher = this._computedWatchers && this._computedWatchers[key];
    if (watcher) {
      if (watcher.dirty) {
        watcher.evaluate();
      }
      if (Dep.target) {
        watcher.depend();
      }
      return watcher.value
    }
  }
}

function initMethods (vm, methods) {
  "development" !== 'production' && checkOptionType(vm, 'methods');
  var props = vm.$options.props;
  for (var key in methods) {
    {
      if (methods[key] == null) {
        warn(
          "Method \"" + key + "\" has an undefined value in the component definition. " +
          "Did you reference the function correctly?",
          vm
        );
      }
      if (props && hasOwn(props, key)) {
        warn(
          ("Method \"" + key + "\" has already been defined as a prop."),
          vm
        );
      }
      if ((key in vm) && isReserved(key)) {
        warn(
          "Method \"" + key + "\" conflicts with an existing Vue instance method. " +
          "Avoid defining component methods that start with _ or $."
        );
      }
    }
    vm[key] = methods[key] == null ? noop : bind(methods[key], vm);
  }
}

function initWatch (vm, watch) {
  "development" !== 'production' && checkOptionType(vm, 'watch');
  for (var key in watch) {
    var handler = watch[key];
    if (Array.isArray(handler)) {
      for (var i = 0; i < handler.length; i++) {
        createWatcher(vm, key, handler[i]);
      }
    } else {
      createWatcher(vm, key, handler);
    }
  }
}

function createWatcher (
  vm,
  keyOrFn,
  handler,
  options
) {
  if (isPlainObject(handler)) {
    options = handler;
    handler = handler.handler;
  }
  if (typeof handler === 'string') {
    handler = vm[handler];
  }
  return vm.$watch(keyOrFn, handler, options)
}

function stateMixin (Vue) {
  // flow somehow has problems with directly declared definition object
  // when using Object.defineProperty, so we have to procedurally build up
  // the object here.
  var dataDef = {};
  dataDef.get = function () { return this._data };
  var propsDef = {};
  propsDef.get = function () { return this._props };
  {
    dataDef.set = function (newData) {
      warn(
        'Avoid replacing instance root $data. ' +
        'Use nested data properties instead.',
        this
      );
    };
    propsDef.set = function () {
      warn("$props is readonly.", this);
    };
  }
  Object.defineProperty(Vue.prototype, '$data', dataDef);
  Object.defineProperty(Vue.prototype, '$props', propsDef);

  Vue.prototype.$set = set;
  Vue.prototype.$delete = del;

  Vue.prototype.$watch = function (
    expOrFn,
    cb,
    options
  ) {
    var vm = this;
    if (isPlainObject(cb)) {
      return createWatcher(vm, expOrFn, cb, options)
    }
    options = options || {};
    options.user = true;
    var watcher = new Watcher(vm, expOrFn, cb, options);
    if (options.immediate) {
      cb.call(vm, watcher.value);
    }
    return function unwatchFn () {
      watcher.teardown();
    }
  };
}

/*  */

function initProvide (vm) {
  var provide = vm.$options.provide;
  if (provide) {
    vm._provided = typeof provide === 'function'
      ? provide.call(vm)
      : provide;
  }
}

function initInjections (vm) {
  var result = resolveInject(vm.$options.inject, vm);
  if (result) {
    observerState.shouldConvert = false;
    Object.keys(result).forEach(function (key) {
      /* istanbul ignore else */
      {
        defineReactive$$1(vm, key, result[key], function () {
          warn(
            "Avoid mutating an injected value directly since the changes will be " +
            "overwritten whenever the provided component re-renders. " +
            "injection being mutated: \"" + key + "\"",
            vm
          );
        });
      }
    });
    observerState.shouldConvert = true;
  }
}

function resolveInject (inject, vm) {
  if (inject) {
    // inject is :any because flow is not smart enough to figure out cached
    var result = Object.create(null);
    var keys = hasSymbol
        ? Reflect.ownKeys(inject).filter(function (key) {
          /* istanbul ignore next */
          return Object.getOwnPropertyDescriptor(inject, key).enumerable
        })
        : Object.keys(inject);

    for (var i = 0; i < keys.length; i++) {
      var key = keys[i];
      var provideKey = inject[key];
      var source = vm;
      while (source) {
        if (source._provided && provideKey in source._provided) {
          result[key] = source._provided[provideKey];
          break
        }
        source = source.$parent;
      }
      if ("development" !== 'production' && !source) {
        warn(("Injection \"" + key + "\" not found"), vm);
      }
    }
    return result
  }
}

/*  */

function createFunctionalComponent (
  Ctor,
  propsData,
  data,
  context,
  children
) {
  var props = {};
  var propOptions = Ctor.options.props;
  if (isDef(propOptions)) {
    for (var key in propOptions) {
      props[key] = validateProp(key, propOptions, propsData || emptyObject);
    }
  } else {
    if (isDef(data.attrs)) { mergeProps(props, data.attrs); }
    if (isDef(data.props)) { mergeProps(props, data.props); }
  }
  // ensure the createElement function in functional components
  // gets a unique context - this is necessary for correct named slot check
  var _context = Object.create(context);
  var h = function (a, b, c, d) { return createElement(_context, a, b, c, d, true); };
  var vnode = Ctor.options.render.call(null, h, {
    data: data,
    props: props,
    children: children,
    parent: context,
    listeners: data.on || emptyObject,
    injections: resolveInject(Ctor.options.inject, context),
    slots: function () { return resolveSlots(children, context); }
  });
  if (vnode instanceof VNode) {
    vnode.functionalContext = context;
    vnode.functionalOptions = Ctor.options;
    if (data.slot) {
      (vnode.data || (vnode.data = {})).slot = data.slot;
    }
  }
  return vnode
}

function mergeProps (to, from) {
  for (var key in from) {
    to[camelize(key)] = from[key];
  }
}

/*  */

// hooks to be invoked on component VNodes during patch
var componentVNodeHooks = {
  init: function init (
    vnode,
    hydrating,
    parentElm,
    refElm
  ) {
    if (!vnode.componentInstance || vnode.componentInstance._isDestroyed) {
      var child = vnode.componentInstance = createComponentInstanceForVnode(
        vnode,
        activeInstance,
        parentElm,
        refElm
      );
      child.$mount(hydrating ? vnode.elm : undefined, hydrating);
    } else if (vnode.data.keepAlive) {
      // kept-alive components, treat as a patch
      var mountedNode = vnode; // work around flow
      componentVNodeHooks.prepatch(mountedNode, mountedNode);
    }
  },

  prepatch: function prepatch (oldVnode, vnode) {
    var options = vnode.componentOptions;
    var child = vnode.componentInstance = oldVnode.componentInstance;
    updateChildComponent(
      child,
      options.propsData, // updated props
      options.listeners, // updated listeners
      vnode, // new parent vnode
      options.children // new children
    );
  },

  insert: function insert (vnode) {
    var context = vnode.context;
    var componentInstance = vnode.componentInstance;
    if (!componentInstance._isMounted) {
      componentInstance._isMounted = true;
      callHook(componentInstance, 'mounted');
    }
    if (vnode.data.keepAlive) {
      if (context._isMounted) {
        // vue-router#1212
        // During updates, a kept-alive component's child components may
        // change, so directly walking the tree here may call activated hooks
        // on incorrect children. Instead we push them into a queue which will
        // be processed after the whole patch process ended.
        queueActivatedComponent(componentInstance);
      } else {
        activateChildComponent(componentInstance, true /* direct */);
      }
    }
  },

  destroy: function destroy (vnode) {
    var componentInstance = vnode.componentInstance;
    if (!componentInstance._isDestroyed) {
      if (!vnode.data.keepAlive) {
        componentInstance.$destroy();
      } else {
        deactivateChildComponent(componentInstance, true /* direct */);
      }
    }
  }
};

var hooksToMerge = Object.keys(componentVNodeHooks);

function createComponent (
  Ctor,
  data,
  context,
  children,
  tag
) {
  if (isUndef(Ctor)) {
    return
  }

  var baseCtor = context.$options._base;

  // plain options object: turn it into a constructor
  if (isObject(Ctor)) {
    Ctor = baseCtor.extend(Ctor);
  }

  // if at this stage it's not a constructor or an async component factory,
  // reject.
  if (typeof Ctor !== 'function') {
    {
      warn(("Invalid Component definition: " + (String(Ctor))), context);
    }
    return
  }

  // async component
  var asyncFactory;
  if (isUndef(Ctor.cid)) {
    asyncFactory = Ctor;
    Ctor = resolveAsyncComponent(asyncFactory, baseCtor, context);
    if (Ctor === undefined) {
      // return a placeholder node for async component, which is rendered
      // as a comment node but preserves all the raw information for the node.
      // the information will be used for async server-rendering and hydration.
      return createAsyncPlaceholder(
        asyncFactory,
        data,
        context,
        children,
        tag
      )
    }
  }

  data = data || {};

  // resolve constructor options in case global mixins are applied after
  // component constructor creation
  resolveConstructorOptions(Ctor);

  // transform component v-model data into props & events
  if (isDef(data.model)) {
    transformModel(Ctor.options, data);
  }

  // extract props
  var propsData = extractPropsFromVNodeData(data, Ctor, tag);

  // functional component
  if (isTrue(Ctor.options.functional)) {
    return createFunctionalComponent(Ctor, propsData, data, context, children)
  }

  // extract listeners, since these needs to be treated as
  // child component listeners instead of DOM listeners
  var listeners = data.on;
  // replace with listeners with .native modifier
  // so it gets processed during parent component patch.
  data.on = data.nativeOn;

  if (isTrue(Ctor.options.abstract)) {
    // abstract components do not keep anything
    // other than props & listeners & slot

    // work around flow
    var slot = data.slot;
    data = {};
    if (slot) {
      data.slot = slot;
    }
  }

  // merge component management hooks onto the placeholder node
  mergeHooks(data);

  // return a placeholder vnode
  var name = Ctor.options.name || tag;
  var vnode = new VNode(
    ("vue-component-" + (Ctor.cid) + (name ? ("-" + name) : '')),
    data, undefined, undefined, undefined, context,
    { Ctor: Ctor, propsData: propsData, listeners: listeners, tag: tag, children: children },
    asyncFactory
  );
  return vnode
}

function createComponentInstanceForVnode (
  vnode, // we know it's MountedComponentVNode but flow doesn't
  parent, // activeInstance in lifecycle state
  parentElm,
  refElm
) {
  var vnodeComponentOptions = vnode.componentOptions;
  var options = {
    _isComponent: true,
    parent: parent,
    propsData: vnodeComponentOptions.propsData,
    _componentTag: vnodeComponentOptions.tag,
    _parentVnode: vnode,
    _parentListeners: vnodeComponentOptions.listeners,
    _renderChildren: vnodeComponentOptions.children,
    _parentElm: parentElm || null,
    _refElm: refElm || null
  };
  // check inline-template render functions
  var inlineTemplate = vnode.data.inlineTemplate;
  if (isDef(inlineTemplate)) {
    options.render = inlineTemplate.render;
    options.staticRenderFns = inlineTemplate.staticRenderFns;
  }
  return new vnodeComponentOptions.Ctor(options)
}

function mergeHooks (data) {
  if (!data.hook) {
    data.hook = {};
  }
  for (var i = 0; i < hooksToMerge.length; i++) {
    var key = hooksToMerge[i];
    var fromParent = data.hook[key];
    var ours = componentVNodeHooks[key];
    data.hook[key] = fromParent ? mergeHook$1(ours, fromParent) : ours;
  }
}

function mergeHook$1 (one, two) {
  return function (a, b, c, d) {
    one(a, b, c, d);
    two(a, b, c, d);
  }
}

// transform component v-model info (value and callback) into
// prop and event handler respectively.
function transformModel (options, data) {
  var prop = (options.model && options.model.prop) || 'value';
  var event = (options.model && options.model.event) || 'input';(data.props || (data.props = {}))[prop] = data.model.value;
  var on = data.on || (data.on = {});
  if (isDef(on[event])) {
    on[event] = [data.model.callback].concat(on[event]);
  } else {
    on[event] = data.model.callback;
  }
}

/*  */

var SIMPLE_NORMALIZE = 1;
var ALWAYS_NORMALIZE = 2;

// wrapper function for providing a more flexible interface
// without getting yelled at by flow
function createElement (
  context,
  tag,
  data,
  children,
  normalizationType,
  alwaysNormalize
) {
  if (Array.isArray(data) || isPrimitive(data)) {
    normalizationType = children;
    children = data;
    data = undefined;
  }
  if (isTrue(alwaysNormalize)) {
    normalizationType = ALWAYS_NORMALIZE;
  }
  return _createElement(context, tag, data, children, normalizationType)
}

function _createElement (
  context,
  tag,
  data,
  children,
  normalizationType
) {
  if (isDef(data) && isDef((data).__ob__)) {
    "development" !== 'production' && warn(
      "Avoid using observed data object as vnode data: " + (JSON.stringify(data)) + "\n" +
      'Always create fresh vnode data objects in each render!',
      context
    );
    return createEmptyVNode()
  }
  // object syntax in v-bind
  if (isDef(data) && isDef(data.is)) {
    tag = data.is;
  }
  if (!tag) {
    // in case of component :is set to falsy value
    return createEmptyVNode()
  }
  // warn against non-primitive key
  if ("development" !== 'production' &&
    isDef(data) && isDef(data.key) && !isPrimitive(data.key)
  ) {
    warn(
      'Avoid using non-primitive value as key, ' +
      'use string/number value instead.',
      context
    );
  }
  // support single function children as default scoped slot
  if (Array.isArray(children) &&
    typeof children[0] === 'function'
  ) {
    data = data || {};
    data.scopedSlots = { default: children[0] };
    children.length = 0;
  }
  if (normalizationType === ALWAYS_NORMALIZE) {
    children = normalizeChildren(children);
  } else if (normalizationType === SIMPLE_NORMALIZE) {
    children = simpleNormalizeChildren(children);
  }
  var vnode, ns;
  if (typeof tag === 'string') {
    var Ctor;
    ns = (context.$vnode && context.$vnode.ns) || config.getTagNamespace(tag);
    if (config.isReservedTag(tag)) {
      // platform built-in elements
      vnode = new VNode(
        config.parsePlatformTagName(tag), data, children,
        undefined, undefined, context
      );
    } else if (isDef(Ctor = resolveAsset(context.$options, 'components', tag))) {
      // component
      vnode = createComponent(Ctor, data, context, children, tag);
    } else {
      // unknown or unlisted namespaced elements
      // check at runtime because it may get assigned a namespace when its
      // parent normalizes children
      vnode = new VNode(
        tag, data, children,
        undefined, undefined, context
      );
    }
  } else {
    // direct component options / constructor
    vnode = createComponent(tag, data, context, children);
  }
  if (isDef(vnode)) {
    if (ns) { applyNS(vnode, ns); }
    return vnode
  } else {
    return createEmptyVNode()
  }
}

function applyNS (vnode, ns) {
  vnode.ns = ns;
  if (vnode.tag === 'foreignObject') {
    // use default namespace inside foreignObject
    return
  }
  if (isDef(vnode.children)) {
    for (var i = 0, l = vnode.children.length; i < l; i++) {
      var child = vnode.children[i];
      if (isDef(child.tag) && isUndef(child.ns)) {
        applyNS(child, ns);
      }
    }
  }
}

/*  */

/**
 * Runtime helper for rendering v-for lists.
 */
function renderList (
  val,
  render
) {
  var ret, i, l, keys, key;
  if (Array.isArray(val) || typeof val === 'string') {
    ret = new Array(val.length);
    for (i = 0, l = val.length; i < l; i++) {
      ret[i] = render(val[i], i);
    }
  } else if (typeof val === 'number') {
    ret = new Array(val);
    for (i = 0; i < val; i++) {
      ret[i] = render(i + 1, i);
    }
  } else if (isObject(val)) {
    keys = Object.keys(val);
    ret = new Array(keys.length);
    for (i = 0, l = keys.length; i < l; i++) {
      key = keys[i];
      ret[i] = render(val[key], key, i);
    }
  }
  if (isDef(ret)) {
    (ret)._isVList = true;
  }
  return ret
}

/*  */

/**
 * Runtime helper for rendering <slot>
 */
function renderSlot (
  name,
  fallback,
  props,
  bindObject
) {
  var scopedSlotFn = this.$scopedSlots[name];
  if (scopedSlotFn) { // scoped slot
    props = props || {};
    if (bindObject) {
      props = extend(extend({}, bindObject), props);
    }
    return scopedSlotFn(props) || fallback
  } else {
    var slotNodes = this.$slots[name];
    // warn duplicate slot usage
    if (slotNodes && "development" !== 'production') {
      slotNodes._rendered && warn(
        "Duplicate presence of slot \"" + name + "\" found in the same render tree " +
        "- this will likely cause render errors.",
        this
      );
      slotNodes._rendered = true;
    }
    return slotNodes || fallback
  }
}

/*  */

/**
 * Runtime helper for resolving filters
 */
function resolveFilter (id) {
  return resolveAsset(this.$options, 'filters', id, true) || identity
}

/*  */

/**
 * Runtime helper for checking keyCodes from config.
 */
function checkKeyCodes (
  eventKeyCode,
  key,
  builtInAlias
) {
  var keyCodes = config.keyCodes[key] || builtInAlias;
  if (Array.isArray(keyCodes)) {
    return keyCodes.indexOf(eventKeyCode) === -1
  } else {
    return keyCodes !== eventKeyCode
  }
}

/*  */

/**
 * Runtime helper for merging v-bind="object" into a VNode's data.
 */
function bindObjectProps (
  data,
  tag,
  value,
  asProp,
  isSync
) {
  if (value) {
    if (!isObject(value)) {
      "development" !== 'production' && warn(
        'v-bind without argument expects an Object or Array value',
        this
      );
    } else {
      if (Array.isArray(value)) {
        value = toObject(value);
      }
      var hash;
      var loop = function ( key ) {
        if (
          key === 'class' ||
          key === 'style' ||
          isReservedAttribute(key)
        ) {
          hash = data;
        } else {
          var type = data.attrs && data.attrs.type;
          hash = asProp || config.mustUseProp(tag, type, key)
            ? data.domProps || (data.domProps = {})
            : data.attrs || (data.attrs = {});
        }
        if (!(key in hash)) {
          hash[key] = value[key];

          if (isSync) {
            var on = data.on || (data.on = {});
            on[("update:" + key)] = function ($event) {
              value[key] = $event;
            };
          }
        }
      };

      for (var key in value) { loop( key ); }
    }
  }
  return data
}

/*  */

/**
 * Runtime helper for rendering static trees.
 */
function renderStatic (
  index,
  isInFor
) {
  var tree = this._staticTrees[index];
  // if has already-rendered static tree and not inside v-for,
  // we can reuse the same tree by doing a shallow clone.
  if (tree && !isInFor) {
    return Array.isArray(tree)
      ? cloneVNodes(tree)
      : cloneVNode(tree)
  }
  // otherwise, render a fresh tree.
  tree = this._staticTrees[index] =
    this.$options.staticRenderFns[index].call(this._renderProxy);
  markStatic(tree, ("__static__" + index), false);
  return tree
}

/**
 * Runtime helper for v-once.
 * Effectively it means marking the node as static with a unique key.
 */
function markOnce (
  tree,
  index,
  key
) {
  markStatic(tree, ("__once__" + index + (key ? ("_" + key) : "")), true);
  return tree
}

function markStatic (
  tree,
  key,
  isOnce
) {
  if (Array.isArray(tree)) {
    for (var i = 0; i < tree.length; i++) {
      if (tree[i] && typeof tree[i] !== 'string') {
        markStaticNode(tree[i], (key + "_" + i), isOnce);
      }
    }
  } else {
    markStaticNode(tree, key, isOnce);
  }
}

function markStaticNode (node, key, isOnce) {
  node.isStatic = true;
  node.key = key;
  node.isOnce = isOnce;
}

/*  */

function bindObjectListeners (data, value) {
  if (value) {
    if (!isPlainObject(value)) {
      "development" !== 'production' && warn(
        'v-on without argument expects an Object value',
        this
      );
    } else {
      var on = data.on = data.on ? extend({}, data.on) : {};
      for (var key in value) {
        var existing = on[key];
        var ours = value[key];
        on[key] = existing ? [].concat(ours, existing) : ours;
      }
    }
  }
  return data
}

/*  */

function initRender (vm) {
  vm._vnode = null; // the root of the child tree
  vm._staticTrees = null;
  var parentVnode = vm.$vnode = vm.$options._parentVnode; // the placeholder node in parent tree
  var renderContext = parentVnode && parentVnode.context;
  vm.$slots = resolveSlots(vm.$options._renderChildren, renderContext);
  vm.$scopedSlots = emptyObject;
  // bind the createElement fn to this instance
  // so that we get proper render context inside it.
  // args order: tag, data, children, normalizationType, alwaysNormalize
  // internal version is used by render functions compiled from templates
  vm._c = function (a, b, c, d) { return createElement(vm, a, b, c, d, false); };
  // normalization is always applied for the public version, used in
  // user-written render functions.
  vm.$createElement = function (a, b, c, d) { return createElement(vm, a, b, c, d, true); };

  // $attrs & $listeners are exposed for easier HOC creation.
  // they need to be reactive so that HOCs using them are always updated
  var parentData = parentVnode && parentVnode.data;

  /* istanbul ignore else */
  {
    defineReactive$$1(vm, '$attrs', parentData && parentData.attrs || emptyObject, function () {
      !isUpdatingChildComponent && warn("$attrs is readonly.", vm);
    }, true);
    defineReactive$$1(vm, '$listeners', vm.$options._parentListeners || emptyObject, function () {
      !isUpdatingChildComponent && warn("$listeners is readonly.", vm);
    }, true);
  }
}

function renderMixin (Vue) {
  Vue.prototype.$nextTick = function (fn) {
    return nextTick(fn, this)
  };

  Vue.prototype._render = function () {
    var vm = this;
    var ref = vm.$options;
    var render = ref.render;
    var staticRenderFns = ref.staticRenderFns;
    var _parentVnode = ref._parentVnode;

    if (vm._isMounted) {
      // if the parent didn't update, the slot nodes will be the ones from
      // last render. They need to be cloned to ensure "freshness" for this render.
      for (var key in vm.$slots) {
        var slot = vm.$slots[key];
        if (slot._rendered) {
          vm.$slots[key] = cloneVNodes(slot, true /* deep */);
        }
      }
    }

    vm.$scopedSlots = (_parentVnode && _parentVnode.data.scopedSlots) || emptyObject;

    if (staticRenderFns && !vm._staticTrees) {
      vm._staticTrees = [];
    }
    // set parent vnode. this allows render functions to have access
    // to the data on the placeholder node.
    vm.$vnode = _parentVnode;
    // render self
    var vnode;
    try {
      vnode = render.call(vm._renderProxy, vm.$createElement);
    } catch (e) {
      handleError(e, vm, "render function");
      // return error render result,
      // or previous vnode to prevent render error causing blank component
      /* istanbul ignore else */
      {
        vnode = vm.$options.renderError
          ? vm.$options.renderError.call(vm._renderProxy, vm.$createElement, e)
          : vm._vnode;
      }
    }
    // return empty vnode in case the render function errored out
    if (!(vnode instanceof VNode)) {
      if ("development" !== 'production' && Array.isArray(vnode)) {
        warn(
          'Multiple root nodes returned from render function. Render function ' +
          'should return a single root node.',
          vm
        );
      }
      vnode = createEmptyVNode();
    }
    // set parent
    vnode.parent = _parentVnode;
    return vnode
  };

  // internal render helpers.
  // these are exposed on the instance prototype to reduce generated render
  // code size.
  Vue.prototype._o = markOnce;
  Vue.prototype._n = toNumber;
  Vue.prototype._s = toString;
  Vue.prototype._l = renderList;
  Vue.prototype._t = renderSlot;
  Vue.prototype._q = looseEqual;
  Vue.prototype._i = looseIndexOf;
  Vue.prototype._m = renderStatic;
  Vue.prototype._f = resolveFilter;
  Vue.prototype._k = checkKeyCodes;
  Vue.prototype._b = bindObjectProps;
  Vue.prototype._v = createTextVNode;
  Vue.prototype._e = createEmptyVNode;
  Vue.prototype._u = resolveScopedSlots;
  Vue.prototype._g = bindObjectListeners;
}

/*  */

var uid = 0;

function initMixin (Vue) {
  Vue.prototype._init = function (options) {
    var vm = this;
    // a uid
    vm._uid = uid++;

    var startTag, endTag;
    /* istanbul ignore if */
    if ("development" !== 'production' && config.performance && mark) {
      startTag = "vue-perf-init:" + (vm._uid);
      endTag = "vue-perf-end:" + (vm._uid);
      mark(startTag);
    }

    // a flag to avoid this being observed
    vm._isVue = true;
    // merge options
    if (options && options._isComponent) {
      // optimize internal component instantiation
      // since dynamic options merging is pretty slow, and none of the
      // internal component options needs special treatment.
      initInternalComponent(vm, options);
    } else {
      vm.$options = mergeOptions(
        resolveConstructorOptions(vm.constructor),
        options || {},
        vm
      );
    }
    /* istanbul ignore else */
    {
      initProxy(vm);
    }
    // expose real self
    vm._self = vm;
    initLifecycle(vm);
    initEvents(vm);
    initRender(vm);
    callHook(vm, 'beforeCreate');
    initInjections(vm); // resolve injections before data/props
    initState(vm);
    initProvide(vm); // resolve provide after data/props
    callHook(vm, 'created');

    /* istanbul ignore if */
    if ("development" !== 'production' && config.performance && mark) {
      vm._name = formatComponentName(vm, false);
      mark(endTag);
      measure(((vm._name) + " init"), startTag, endTag);
    }

    if (vm.$options.el) {
      vm.$mount(vm.$options.el);
    }
  };
}

function initInternalComponent (vm, options) {
  var opts = vm.$options = Object.create(vm.constructor.options);
  // doing this because it's faster than dynamic enumeration.
  opts.parent = options.parent;
  opts.propsData = options.propsData;
  opts._parentVnode = options._parentVnode;
  opts._parentListeners = options._parentListeners;
  opts._renderChildren = options._renderChildren;
  opts._componentTag = options._componentTag;
  opts._parentElm = options._parentElm;
  opts._refElm = options._refElm;
  if (options.render) {
    opts.render = options.render;
    opts.staticRenderFns = options.staticRenderFns;
  }
}

function resolveConstructorOptions (Ctor) {
  var options = Ctor.options;
  if (Ctor.super) {
    var superOptions = resolveConstructorOptions(Ctor.super);
    var cachedSuperOptions = Ctor.superOptions;
    if (superOptions !== cachedSuperOptions) {
      // super option changed,
      // need to resolve new options.
      Ctor.superOptions = superOptions;
      // check if there are any late-modified/attached options (#4976)
      var modifiedOptions = resolveModifiedOptions(Ctor);
      // update base extend options
      if (modifiedOptions) {
        extend(Ctor.extendOptions, modifiedOptions);
      }
      options = Ctor.options = mergeOptions(superOptions, Ctor.extendOptions);
      if (options.name) {
        options.components[options.name] = Ctor;
      }
    }
  }
  return options
}

function resolveModifiedOptions (Ctor) {
  var modified;
  var latest = Ctor.options;
  var extended = Ctor.extendOptions;
  var sealed = Ctor.sealedOptions;
  for (var key in latest) {
    if (latest[key] !== sealed[key]) {
      if (!modified) { modified = {}; }
      modified[key] = dedupe(latest[key], extended[key], sealed[key]);
    }
  }
  return modified
}

function dedupe (latest, extended, sealed) {
  // compare latest and sealed to ensure lifecycle hooks won't be duplicated
  // between merges
  if (Array.isArray(latest)) {
    var res = [];
    sealed = Array.isArray(sealed) ? sealed : [sealed];
    extended = Array.isArray(extended) ? extended : [extended];
    for (var i = 0; i < latest.length; i++) {
      // push original options and not sealed options to exclude duplicated options
      if (extended.indexOf(latest[i]) >= 0 || sealed.indexOf(latest[i]) < 0) {
        res.push(latest[i]);
      }
    }
    return res
  } else {
    return latest
  }
}

function Vue$2 (options) {
  if ("development" !== 'production' &&
    !(this instanceof Vue$2)
  ) {
    warn('Vue is a constructor and should be called with the `new` keyword');
  }
  this._init(options);
}

initMixin(Vue$2);
stateMixin(Vue$2);
eventsMixin(Vue$2);
lifecycleMixin(Vue$2);
renderMixin(Vue$2);

/*  */

function initUse (Vue) {
  Vue.use = function (plugin) {
    var installedPlugins = (this._installedPlugins || (this._installedPlugins = []));
    if (installedPlugins.indexOf(plugin) > -1) {
      return this
    }

    // additional parameters
    var args = toArray(arguments, 1);
    args.unshift(this);
    if (typeof plugin.install === 'function') {
      plugin.install.apply(plugin, args);
    } else if (typeof plugin === 'function') {
      plugin.apply(null, args);
    }
    installedPlugins.push(plugin);
    return this
  };
}

/*  */

function initMixin$1 (Vue) {
  Vue.mixin = function (mixin) {
    this.options = mergeOptions(this.options, mixin);
    return this
  };
}

/*  */

function initExtend (Vue) {
  /**
   * Each instance constructor, including Vue, has a unique
   * cid. This enables us to create wrapped "child
   * constructors" for prototypal inheritance and cache them.
   */
  Vue.cid = 0;
  var cid = 1;

  /**
   * Class inheritance
   */
  Vue.extend = function (extendOptions) {
    extendOptions = extendOptions || {};
    var Super = this;
    var SuperId = Super.cid;
    var cachedCtors = extendOptions._Ctor || (extendOptions._Ctor = {});
    if (cachedCtors[SuperId]) {
      return cachedCtors[SuperId]
    }

    var name = extendOptions.name || Super.options.name;
    {
      if (!/^[a-zA-Z][\w-]*$/.test(name)) {
        warn(
          'Invalid component name: "' + name + '". Component names ' +
          'can only contain alphanumeric characters and the hyphen, ' +
          'and must start with a letter.'
        );
      }
    }

    var Sub = function VueComponent (options) {
      this._init(options);
    };
    Sub.prototype = Object.create(Super.prototype);
    Sub.prototype.constructor = Sub;
    Sub.cid = cid++;
    Sub.options = mergeOptions(
      Super.options,
      extendOptions
    );
    Sub['super'] = Super;

    // For props and computed properties, we define the proxy getters on
    // the Vue instances at extension time, on the extended prototype. This
    // avoids Object.defineProperty calls for each instance created.
    if (Sub.options.props) {
      initProps$1(Sub);
    }
    if (Sub.options.computed) {
      initComputed$1(Sub);
    }

    // allow further extension/mixin/plugin usage
    Sub.extend = Super.extend;
    Sub.mixin = Super.mixin;
    Sub.use = Super.use;

    // create asset registers, so extended classes
    // can have their private assets too.
    ASSET_TYPES.forEach(function (type) {
      Sub[type] = Super[type];
    });
    // enable recursive self-lookup
    if (name) {
      Sub.options.components[name] = Sub;
    }

    // keep a reference to the super options at extension time.
    // later at instantiation we can check if Super's options have
    // been updated.
    Sub.superOptions = Super.options;
    Sub.extendOptions = extendOptions;
    Sub.sealedOptions = extend({}, Sub.options);

    // cache constructor
    cachedCtors[SuperId] = Sub;
    return Sub
  };
}

function initProps$1 (Comp) {
  var props = Comp.options.props;
  for (var key in props) {
    proxy(Comp.prototype, "_props", key);
  }
}

function initComputed$1 (Comp) {
  var computed = Comp.options.computed;
  for (var key in computed) {
    defineComputed(Comp.prototype, key, computed[key]);
  }
}

/*  */

function initAssetRegisters (Vue) {
  /**
   * Create asset registration methods.
   */
  ASSET_TYPES.forEach(function (type) {
    Vue[type] = function (
      id,
      definition
    ) {
      if (!definition) {
        return this.options[type + 's'][id]
      } else {
        /* istanbul ignore if */
        {
          if (type === 'component' && config.isReservedTag(id)) {
            warn(
              'Do not use built-in or reserved HTML elements as component ' +
              'id: ' + id
            );
          }
        }
        if (type === 'component' && isPlainObject(definition)) {
          definition.name = definition.name || id;
          definition = this.options._base.extend(definition);
        }
        if (type === 'directive' && typeof definition === 'function') {
          definition = { bind: definition, update: definition };
        }
        this.options[type + 's'][id] = definition;
        return definition
      }
    };
  });
}

/*  */

var patternTypes = [String, RegExp, Array];

function getComponentName (opts) {
  return opts && (opts.Ctor.options.name || opts.tag)
}

function matches (pattern, name) {
  if (Array.isArray(pattern)) {
    return pattern.indexOf(name) > -1
  } else if (typeof pattern === 'string') {
    return pattern.split(',').indexOf(name) > -1
  } else if (isRegExp(pattern)) {
    return pattern.test(name)
  }
  /* istanbul ignore next */
  return false
}

function pruneCache (cache, current, filter) {
  for (var key in cache) {
    var cachedNode = cache[key];
    if (cachedNode) {
      var name = getComponentName(cachedNode.componentOptions);
      if (name && !filter(name)) {
        if (cachedNode !== current) {
          pruneCacheEntry(cachedNode);
        }
        cache[key] = null;
      }
    }
  }
}

function pruneCacheEntry (vnode) {
  if (vnode) {
    vnode.componentInstance.$destroy();
  }
}

var KeepAlive = {
  name: 'keep-alive',
  abstract: true,

  props: {
    include: patternTypes,
    exclude: patternTypes
  },

  created: function created () {
    this.cache = Object.create(null);
  },

  destroyed: function destroyed () {
    var this$1 = this;

    for (var key in this$1.cache) {
      pruneCacheEntry(this$1.cache[key]);
    }
  },

  watch: {
    include: function include (val) {
      pruneCache(this.cache, this._vnode, function (name) { return matches(val, name); });
    },
    exclude: function exclude (val) {
      pruneCache(this.cache, this._vnode, function (name) { return !matches(val, name); });
    }
  },

  render: function render () {
    var vnode = getFirstComponentChild(this.$slots.default);
    var componentOptions = vnode && vnode.componentOptions;
    if (componentOptions) {
      // check pattern
      var name = getComponentName(componentOptions);
      if (name && (
        (this.include && !matches(this.include, name)) ||
        (this.exclude && matches(this.exclude, name))
      )) {
        return vnode
      }
      var key = vnode.key == null
        // same constructor may get registered as different local components
        // so cid alone is not enough (#3269)
        ? componentOptions.Ctor.cid + (componentOptions.tag ? ("::" + (componentOptions.tag)) : '')
        : vnode.key;
      if (this.cache[key]) {
        vnode.componentInstance = this.cache[key].componentInstance;
      } else {
        this.cache[key] = vnode;
      }
      vnode.data.keepAlive = true;
    }
    return vnode
  }
};

var builtInComponents = {
  KeepAlive: KeepAlive
};

/*  */

function initGlobalAPI (Vue) {
  // config
  var configDef = {};
  configDef.get = function () { return config; };
  {
    configDef.set = function () {
      warn(
        'Do not replace the Vue.config object, set individual fields instead.'
      );
    };
  }
  Object.defineProperty(Vue, 'config', configDef);

  // exposed util methods.
  // NOTE: these are not considered part of the public API - avoid relying on
  // them unless you are aware of the risk.
  Vue.util = {
    warn: warn,
    extend: extend,
    mergeOptions: mergeOptions,
    defineReactive: defineReactive$$1
  };

  Vue.set = set;
  Vue.delete = del;
  Vue.nextTick = nextTick;

  Vue.options = Object.create(null);
  ASSET_TYPES.forEach(function (type) {
    Vue.options[type + 's'] = Object.create(null);
  });

  // this is used to identify the "base" constructor to extend all plain-object
  // components with in Weex's multi-instance scenarios.
  Vue.options._base = Vue;

  extend(Vue.options.components, builtInComponents);

  initUse(Vue);
  initMixin$1(Vue);
  initExtend(Vue);
  initAssetRegisters(Vue);
}

initGlobalAPI(Vue$2);

Object.defineProperty(Vue$2.prototype, '$isServer', {
  get: isServerRendering
});

Object.defineProperty(Vue$2.prototype, '$ssrContext', {
  get: function get () {
    /* istanbul ignore next */
    return this.$vnode && this.$vnode.ssrContext
  }
});

Vue$2.version = '2.4.3';

/* globals renderer */
// renderer is injected by weex factory wrapper

var namespaceMap = {};

function createElement$1 (tagName) {
  return new renderer.Element(tagName)
}

function createElementNS (namespace, tagName) {
  return new renderer.Element(namespace + ':' + tagName)
}

function createTextNode (text) {
  return new renderer.TextNode(text)
}

function createComment (text) {
  return new renderer.Comment(text)
}

function insertBefore (node, target, before) {
  if (target.nodeType === 3) {
    if (node.type === 'text') {
      node.setAttr('value', target.text);
      target.parentNode = node;
    } else {
      var text = createElement$1('text');
      text.setAttr('value', target.text);
      node.insertBefore(text, before);
    }
    return
  }
  node.insertBefore(target, before);
}

function removeChild (node, child) {
  if (child.nodeType === 3) {
    node.setAttr('value', '');
    return
  }
  node.removeChild(child);
}

function appendChild (node, child) {
  if (child.nodeType === 3) {
    if (node.type === 'text') {
      node.setAttr('value', child.text);
      child.parentNode = node;
    } else {
      var text = createElement$1('text');
      text.setAttr('value', child.text);
      node.appendChild(text);
    }
    return
  }

  node.appendChild(child);
}

function parentNode (node) {
  return node.parentNode
}

function nextSibling (node) {
  return node.nextSibling
}

function tagName (node) {
  return node.type
}

function setTextContent (node, text) {
  node.parentNode.setAttr('value', text);
}

function setAttribute (node, key, val) {
  node.setAttr(key, val);
}


var nodeOps = Object.freeze({
	namespaceMap: namespaceMap,
	createElement: createElement$1,
	createElementNS: createElementNS,
	createTextNode: createTextNode,
	createComment: createComment,
	insertBefore: insertBefore,
	removeChild: removeChild,
	appendChild: appendChild,
	parentNode: parentNode,
	nextSibling: nextSibling,
	tagName: tagName,
	setTextContent: setTextContent,
	setAttribute: setAttribute
});

/*  */

var ref = {
  create: function create (_, vnode) {
    registerRef(vnode);
  },
  update: function update (oldVnode, vnode) {
    if (oldVnode.data.ref !== vnode.data.ref) {
      registerRef(oldVnode, true);
      registerRef(vnode);
    }
  },
  destroy: function destroy (vnode) {
    registerRef(vnode, true);
  }
};

function registerRef (vnode, isRemoval) {
  var key = vnode.data.ref;
  if (!key) { return }

  var vm = vnode.context;
  var ref = vnode.componentInstance || vnode.elm;
  var refs = vm.$refs;
  if (isRemoval) {
    if (Array.isArray(refs[key])) {
      remove(refs[key], ref);
    } else if (refs[key] === ref) {
      refs[key] = undefined;
    }
  } else {
    if (vnode.data.refInFor) {
      if (!Array.isArray(refs[key])) {
        refs[key] = [ref];
      } else if (refs[key].indexOf(ref) < 0) {
        // $flow-disable-line
        refs[key].push(ref);
      }
    } else {
      refs[key] = ref;
    }
  }
}

/*  */



var isHTMLTag = makeMap(
  'html,body,base,head,link,meta,style,title,' +
  'address,article,aside,footer,header,h1,h2,h3,h4,h5,h6,hgroup,nav,section,' +
  'div,dd,dl,dt,figcaption,figure,picture,hr,img,li,main,ol,p,pre,ul,' +
  'a,b,abbr,bdi,bdo,br,cite,code,data,dfn,em,i,kbd,mark,q,rp,rt,rtc,ruby,' +
  's,samp,small,span,strong,sub,sup,time,u,var,wbr,area,audio,map,track,video,' +
  'embed,object,param,source,canvas,script,noscript,del,ins,' +
  'caption,col,colgroup,table,thead,tbody,td,th,tr,' +
  'button,datalist,fieldset,form,input,label,legend,meter,optgroup,option,' +
  'output,progress,select,textarea,' +
  'details,dialog,menu,menuitem,summary,' +
  'content,element,shadow,template,blockquote,iframe,tfoot'
);

// this map is intentionally selective, only covering SVG elements that may
// contain child elements.
var isSVG = makeMap(
  'svg,animate,circle,clippath,cursor,defs,desc,ellipse,filter,font-face,' +
  'foreignObject,g,glyph,image,line,marker,mask,missing-glyph,path,pattern,' +
  'polygon,polyline,rect,switch,symbol,text,textpath,tspan,use,view',
  true
);









var isTextInputType = makeMap('text,number,password,search,email,tel,url');

/**
 * Virtual DOM patching algorithm based on Snabbdom by
 * Simon Friis Vindum (@paldepind)
 * Licensed under the MIT License
 * https://github.com/paldepind/snabbdom/blob/master/LICENSE
 *
 * modified by Evan You (@yyx990803)
 *
 * Not type-checking this because this file is perf-critical and the cost
 * of making flow understand it is not worth it.
 */

var emptyNode = new VNode('', {}, []);

var hooks = ['create', 'activate', 'update', 'remove', 'destroy'];

function sameVnode (a, b) {
  return (
    a.key === b.key && (
      (
        a.tag === b.tag &&
        a.isComment === b.isComment &&
        isDef(a.data) === isDef(b.data) &&
        sameInputType(a, b)
      ) || (
        isTrue(a.isAsyncPlaceholder) &&
        a.asyncFactory === b.asyncFactory &&
        isUndef(b.asyncFactory.error)
      )
    )
  )
}

function sameInputType (a, b) {
  if (a.tag !== 'input') { return true }
  var i;
  var typeA = isDef(i = a.data) && isDef(i = i.attrs) && i.type;
  var typeB = isDef(i = b.data) && isDef(i = i.attrs) && i.type;
  return typeA === typeB || isTextInputType(typeA) && isTextInputType(typeB)
}

function createKeyToOldIdx (children, beginIdx, endIdx) {
  var i, key;
  var map = {};
  for (i = beginIdx; i <= endIdx; ++i) {
    key = children[i].key;
    if (isDef(key)) { map[key] = i; }
  }
  return map
}

function createPatchFunction (backend) {
  var i, j;
  var cbs = {};

  var modules = backend.modules;
  var nodeOps = backend.nodeOps;

  for (i = 0; i < hooks.length; ++i) {
    cbs[hooks[i]] = [];
    for (j = 0; j < modules.length; ++j) {
      if (isDef(modules[j][hooks[i]])) {
        cbs[hooks[i]].push(modules[j][hooks[i]]);
      }
    }
  }

  function emptyNodeAt (elm) {
    return new VNode(nodeOps.tagName(elm).toLowerCase(), {}, [], undefined, elm)
  }

  function createRmCb (childElm, listeners) {
    function remove$$1 () {
      if (--remove$$1.listeners === 0) {
        removeNode(childElm);
      }
    }
    remove$$1.listeners = listeners;
    return remove$$1
  }

  function removeNode (el) {
    var parent = nodeOps.parentNode(el);
    // element may have already been removed due to v-html / v-text
    if (isDef(parent)) {
      nodeOps.removeChild(parent, el);
    }
  }

  var inPre = 0;
  function createElm (vnode, insertedVnodeQueue, parentElm, refElm, nested) {
    vnode.isRootInsert = !nested; // for transition enter check
    if (createComponent(vnode, insertedVnodeQueue, parentElm, refElm)) {
      return
    }

    var data = vnode.data;
    var children = vnode.children;
    var tag = vnode.tag;
    if (isDef(tag)) {
      {
        if (data && data.pre) {
          inPre++;
        }
        if (
          !inPre &&
          !vnode.ns &&
          !(config.ignoredElements.length && config.ignoredElements.indexOf(tag) > -1) &&
          config.isUnknownElement(tag)
        ) {
          warn(
            'Unknown custom element: <' + tag + '> - did you ' +
            'register the component correctly? For recursive components, ' +
            'make sure to provide the "name" option.',
            vnode.context
          );
        }
      }
      vnode.elm = vnode.ns
        ? nodeOps.createElementNS(vnode.ns, tag)
        : nodeOps.createElement(tag, vnode);
      setScope(vnode);

      /* istanbul ignore if */
      {
        // in Weex, the default insertion order is parent-first.
        // List items can be optimized to use children-first insertion
        // with append="tree".
        var appendAsTree = isDef(data) && isTrue(data.appendAsTree);
        if (!appendAsTree) {
          if (isDef(data)) {
            invokeCreateHooks(vnode, insertedVnodeQueue);
          }
          insert(parentElm, vnode.elm, refElm);
        }
        createChildren(vnode, children, insertedVnodeQueue);
        if (appendAsTree) {
          if (isDef(data)) {
            invokeCreateHooks(vnode, insertedVnodeQueue);
          }
          insert(parentElm, vnode.elm, refElm);
        }
      }

      if ("development" !== 'production' && data && data.pre) {
        inPre--;
      }
    } else if (isTrue(vnode.isComment)) {
      vnode.elm = nodeOps.createComment(vnode.text);
      insert(parentElm, vnode.elm, refElm);
    } else {
      vnode.elm = nodeOps.createTextNode(vnode.text);
      insert(parentElm, vnode.elm, refElm);
    }
  }

  function createComponent (vnode, insertedVnodeQueue, parentElm, refElm) {
    var i = vnode.data;
    if (isDef(i)) {
      var isReactivated = isDef(vnode.componentInstance) && i.keepAlive;
      if (isDef(i = i.hook) && isDef(i = i.init)) {
        i(vnode, false /* hydrating */, parentElm, refElm);
      }
      // after calling the init hook, if the vnode is a child component
      // it should've created a child instance and mounted it. the child
      // component also has set the placeholder vnode's elm.
      // in that case we can just return the element and be done.
      if (isDef(vnode.componentInstance)) {
        initComponent(vnode, insertedVnodeQueue);
        if (isTrue(isReactivated)) {
          reactivateComponent(vnode, insertedVnodeQueue, parentElm, refElm);
        }
        return true
      }
    }
  }

  function initComponent (vnode, insertedVnodeQueue) {
    if (isDef(vnode.data.pendingInsert)) {
      insertedVnodeQueue.push.apply(insertedVnodeQueue, vnode.data.pendingInsert);
      vnode.data.pendingInsert = null;
    }
    vnode.elm = vnode.componentInstance.$el;
    if (isPatchable(vnode)) {
      invokeCreateHooks(vnode, insertedVnodeQueue);
      setScope(vnode);
    } else {
      // empty component root.
      // skip all element-related modules except for ref (#3455)
      registerRef(vnode);
      // make sure to invoke the insert hook
      insertedVnodeQueue.push(vnode);
    }
  }

  function reactivateComponent (vnode, insertedVnodeQueue, parentElm, refElm) {
    var i;
    // hack for #4339: a reactivated component with inner transition
    // does not trigger because the inner node's created hooks are not called
    // again. It's not ideal to involve module-specific logic in here but
    // there doesn't seem to be a better way to do it.
    var innerNode = vnode;
    while (innerNode.componentInstance) {
      innerNode = innerNode.componentInstance._vnode;
      if (isDef(i = innerNode.data) && isDef(i = i.transition)) {
        for (i = 0; i < cbs.activate.length; ++i) {
          cbs.activate[i](emptyNode, innerNode);
        }
        insertedVnodeQueue.push(innerNode);
        break
      }
    }
    // unlike a newly created component,
    // a reactivated keep-alive component doesn't insert itself
    insert(parentElm, vnode.elm, refElm);
  }

  function insert (parent, elm, ref$$1) {
    if (isDef(parent)) {
      if (isDef(ref$$1)) {
        if (ref$$1.parentNode === parent) {
          nodeOps.insertBefore(parent, elm, ref$$1);
        }
      } else {
        nodeOps.appendChild(parent, elm);
      }
    }
  }

  function createChildren (vnode, children, insertedVnodeQueue) {
    if (Array.isArray(children)) {
      for (var i = 0; i < children.length; ++i) {
        createElm(children[i], insertedVnodeQueue, vnode.elm, null, true);
      }
    } else if (isPrimitive(vnode.text)) {
      nodeOps.appendChild(vnode.elm, nodeOps.createTextNode(vnode.text));
    }
  }

  function isPatchable (vnode) {
    while (vnode.componentInstance) {
      vnode = vnode.componentInstance._vnode;
    }
    return isDef(vnode.tag)
  }

  function invokeCreateHooks (vnode, insertedVnodeQueue) {
    for (var i$1 = 0; i$1 < cbs.create.length; ++i$1) {
      cbs.create[i$1](emptyNode, vnode);
    }
    i = vnode.data.hook; // Reuse variable
    if (isDef(i)) {
      if (isDef(i.create)) { i.create(emptyNode, vnode); }
      if (isDef(i.insert)) { insertedVnodeQueue.push(vnode); }
    }
  }

  // set scope id attribute for scoped CSS.
  // this is implemented as a special case to avoid the overhead
  // of going through the normal attribute patching process.
  function setScope (vnode) {
    var i;
    var ancestor = vnode;
    while (ancestor) {
      if (isDef(i = ancestor.context) && isDef(i = i.$options._scopeId)) {
        nodeOps.setAttribute(vnode.elm, i, '');
      }
      ancestor = ancestor.parent;
    }
    // for slot content they should also get the scopeId from the host instance.
    if (isDef(i = activeInstance) &&
      i !== vnode.context &&
      isDef(i = i.$options._scopeId)
    ) {
      nodeOps.setAttribute(vnode.elm, i, '');
    }
  }

  function addVnodes (parentElm, refElm, vnodes, startIdx, endIdx, insertedVnodeQueue) {
    for (; startIdx <= endIdx; ++startIdx) {
      createElm(vnodes[startIdx], insertedVnodeQueue, parentElm, refElm);
    }
  }

  function invokeDestroyHook (vnode) {
    var i, j;
    var data = vnode.data;
    if (isDef(data)) {
      if (isDef(i = data.hook) && isDef(i = i.destroy)) { i(vnode); }
      for (i = 0; i < cbs.destroy.length; ++i) { cbs.destroy[i](vnode); }
    }
    if (isDef(i = vnode.children)) {
      for (j = 0; j < vnode.children.length; ++j) {
        invokeDestroyHook(vnode.children[j]);
      }
    }
  }

  function removeVnodes (parentElm, vnodes, startIdx, endIdx) {
    for (; startIdx <= endIdx; ++startIdx) {
      var ch = vnodes[startIdx];
      if (isDef(ch)) {
        if (isDef(ch.tag)) {
          removeAndInvokeRemoveHook(ch);
          invokeDestroyHook(ch);
        } else { // Text node
          removeNode(ch.elm);
        }
      }
    }
  }

  function removeAndInvokeRemoveHook (vnode, rm) {
    if (isDef(rm) || isDef(vnode.data)) {
      var i;
      var listeners = cbs.remove.length + 1;
      if (isDef(rm)) {
        // we have a recursively passed down rm callback
        // increase the listeners count
        rm.listeners += listeners;
      } else {
        // directly removing
        rm = createRmCb(vnode.elm, listeners);
      }
      // recursively invoke hooks on child component root node
      if (isDef(i = vnode.componentInstance) && isDef(i = i._vnode) && isDef(i.data)) {
        removeAndInvokeRemoveHook(i, rm);
      }
      for (i = 0; i < cbs.remove.length; ++i) {
        cbs.remove[i](vnode, rm);
      }
      if (isDef(i = vnode.data.hook) && isDef(i = i.remove)) {
        i(vnode, rm);
      } else {
        rm();
      }
    } else {
      removeNode(vnode.elm);
    }
  }

  function updateChildren (parentElm, oldCh, newCh, insertedVnodeQueue, removeOnly) {
    var oldStartIdx = 0;
    var newStartIdx = 0;
    var oldEndIdx = oldCh.length - 1;
    var oldStartVnode = oldCh[0];
    var oldEndVnode = oldCh[oldEndIdx];
    var newEndIdx = newCh.length - 1;
    var newStartVnode = newCh[0];
    var newEndVnode = newCh[newEndIdx];
    var oldKeyToIdx, idxInOld, elmToMove, refElm;

    // removeOnly is a special flag used only by <transition-group>
    // to ensure removed elements stay in correct relative positions
    // during leaving transitions
    var canMove = !removeOnly;

    while (oldStartIdx <= oldEndIdx && newStartIdx <= newEndIdx) {
      if (isUndef(oldStartVnode)) {
        oldStartVnode = oldCh[++oldStartIdx]; // Vnode has been moved left
      } else if (isUndef(oldEndVnode)) {
        oldEndVnode = oldCh[--oldEndIdx];
      } else if (sameVnode(oldStartVnode, newStartVnode)) {
        patchVnode(oldStartVnode, newStartVnode, insertedVnodeQueue);
        oldStartVnode = oldCh[++oldStartIdx];
        newStartVnode = newCh[++newStartIdx];
      } else if (sameVnode(oldEndVnode, newEndVnode)) {
        patchVnode(oldEndVnode, newEndVnode, insertedVnodeQueue);
        oldEndVnode = oldCh[--oldEndIdx];
        newEndVnode = newCh[--newEndIdx];
      } else if (sameVnode(oldStartVnode, newEndVnode)) { // Vnode moved right
        patchVnode(oldStartVnode, newEndVnode, insertedVnodeQueue);
        canMove && nodeOps.insertBefore(parentElm, oldStartVnode.elm, nodeOps.nextSibling(oldEndVnode.elm));
        oldStartVnode = oldCh[++oldStartIdx];
        newEndVnode = newCh[--newEndIdx];
      } else if (sameVnode(oldEndVnode, newStartVnode)) { // Vnode moved left
        patchVnode(oldEndVnode, newStartVnode, insertedVnodeQueue);
        canMove && nodeOps.insertBefore(parentElm, oldEndVnode.elm, oldStartVnode.elm);
        oldEndVnode = oldCh[--oldEndIdx];
        newStartVnode = newCh[++newStartIdx];
      } else {
        if (isUndef(oldKeyToIdx)) { oldKeyToIdx = createKeyToOldIdx(oldCh, oldStartIdx, oldEndIdx); }
        idxInOld = isDef(newStartVnode.key)
          ? oldKeyToIdx[newStartVnode.key]
          : findIdxInOld(newStartVnode, oldCh, oldStartIdx, oldEndIdx);
        if (isUndef(idxInOld)) { // New element
          createElm(newStartVnode, insertedVnodeQueue, parentElm, oldStartVnode.elm);
        } else {
          elmToMove = oldCh[idxInOld];
          /* istanbul ignore if */
          if ("development" !== 'production' && !elmToMove) {
            warn(
              'It seems there are duplicate keys that is causing an update error. ' +
              'Make sure each v-for item has a unique key.'
            );
          }
          if (sameVnode(elmToMove, newStartVnode)) {
            patchVnode(elmToMove, newStartVnode, insertedVnodeQueue);
            oldCh[idxInOld] = undefined;
            canMove && nodeOps.insertBefore(parentElm, elmToMove.elm, oldStartVnode.elm);
          } else {
            // same key but different element. treat as new element
            createElm(newStartVnode, insertedVnodeQueue, parentElm, oldStartVnode.elm);
          }
        }
        newStartVnode = newCh[++newStartIdx];
      }
    }
    if (oldStartIdx > oldEndIdx) {
      refElm = isUndef(newCh[newEndIdx + 1]) ? null : newCh[newEndIdx + 1].elm;
      addVnodes(parentElm, refElm, newCh, newStartIdx, newEndIdx, insertedVnodeQueue);
    } else if (newStartIdx > newEndIdx) {
      removeVnodes(parentElm, oldCh, oldStartIdx, oldEndIdx);
    }
  }

  function findIdxInOld (node, oldCh, start, end) {
    for (var i = start; i < end; i++) {
      var c = oldCh[i];
      if (isDef(c) && sameVnode(node, c)) { return i }
    }
  }

  function patchVnode (oldVnode, vnode, insertedVnodeQueue, removeOnly) {
    if (oldVnode === vnode) {
      return
    }

    var elm = vnode.elm = oldVnode.elm;

    if (isTrue(oldVnode.isAsyncPlaceholder)) {
      if (isDef(vnode.asyncFactory.resolved)) {
        hydrate(oldVnode.elm, vnode, insertedVnodeQueue);
      } else {
        vnode.isAsyncPlaceholder = true;
      }
      return
    }

    // reuse element for static trees.
    // note we only do this if the vnode is cloned -
    // if the new node is not cloned it means the render functions have been
    // reset by the hot-reload-api and we need to do a proper re-render.
    if (isTrue(vnode.isStatic) &&
      isTrue(oldVnode.isStatic) &&
      vnode.key === oldVnode.key &&
      (isTrue(vnode.isCloned) || isTrue(vnode.isOnce))
    ) {
      vnode.componentInstance = oldVnode.componentInstance;
      return
    }

    var i;
    var data = vnode.data;
    if (isDef(data) && isDef(i = data.hook) && isDef(i = i.prepatch)) {
      i(oldVnode, vnode);
    }

    var oldCh = oldVnode.children;
    var ch = vnode.children;
    if (isDef(data) && isPatchable(vnode)) {
      for (i = 0; i < cbs.update.length; ++i) { cbs.update[i](oldVnode, vnode); }
      if (isDef(i = data.hook) && isDef(i = i.update)) { i(oldVnode, vnode); }
    }
    if (isUndef(vnode.text)) {
      if (isDef(oldCh) && isDef(ch)) {
        if (oldCh !== ch) { updateChildren(elm, oldCh, ch, insertedVnodeQueue, removeOnly); }
      } else if (isDef(ch)) {
        if (isDef(oldVnode.text)) { nodeOps.setTextContent(elm, ''); }
        addVnodes(elm, null, ch, 0, ch.length - 1, insertedVnodeQueue);
      } else if (isDef(oldCh)) {
        removeVnodes(elm, oldCh, 0, oldCh.length - 1);
      } else if (isDef(oldVnode.text)) {
        nodeOps.setTextContent(elm, '');
      }
    } else if (oldVnode.text !== vnode.text) {
      nodeOps.setTextContent(elm, vnode.text);
    }
    if (isDef(data)) {
      if (isDef(i = data.hook) && isDef(i = i.postpatch)) { i(oldVnode, vnode); }
    }
  }

  function invokeInsertHook (vnode, queue, initial) {
    // delay insert hooks for component root nodes, invoke them after the
    // element is really inserted
    if (isTrue(initial) && isDef(vnode.parent)) {
      vnode.parent.data.pendingInsert = queue;
    } else {
      for (var i = 0; i < queue.length; ++i) {
        queue[i].data.hook.insert(queue[i]);
      }
    }
  }

  var bailed = false;
  // list of modules that can skip create hook during hydration because they
  // are already rendered on the client or has no need for initialization
  var isRenderedModule = makeMap('attrs,style,class,staticClass,staticStyle,key');

  // Note: this is a browser-only function so we can assume elms are DOM nodes.
  function hydrate (elm, vnode, insertedVnodeQueue) {
    if (isTrue(vnode.isComment) && isDef(vnode.asyncFactory)) {
      vnode.elm = elm;
      vnode.isAsyncPlaceholder = true;
      return true
    }
    {
      if (!assertNodeMatch(elm, vnode)) {
        return false
      }
    }
    vnode.elm = elm;
    var tag = vnode.tag;
    var data = vnode.data;
    var children = vnode.children;
    if (isDef(data)) {
      if (isDef(i = data.hook) && isDef(i = i.init)) { i(vnode, true /* hydrating */); }
      if (isDef(i = vnode.componentInstance)) {
        // child component. it should have hydrated its own tree.
        initComponent(vnode, insertedVnodeQueue);
        return true
      }
    }
    if (isDef(tag)) {
      if (isDef(children)) {
        // empty element, allow client to pick up and populate children
        if (!elm.hasChildNodes()) {
          createChildren(vnode, children, insertedVnodeQueue);
        } else {
          // v-html and domProps: innerHTML
          if (isDef(i = data) && isDef(i = i.domProps) && isDef(i = i.innerHTML)) {
            if (i !== elm.innerHTML) {
              /* istanbul ignore if */
              if ("development" !== 'production' &&
                typeof console !== 'undefined' &&
                !bailed
              ) {
                bailed = true;
                console.warn('Parent: ', elm);
                console.warn('server innerHTML: ', i);
                console.warn('client innerHTML: ', elm.innerHTML);
              }
              return false
            }
          } else {
            // iterate and compare children lists
            var childrenMatch = true;
            var childNode = elm.firstChild;
            for (var i$1 = 0; i$1 < children.length; i$1++) {
              if (!childNode || !hydrate(childNode, children[i$1], insertedVnodeQueue)) {
                childrenMatch = false;
                break
              }
              childNode = childNode.nextSibling;
            }
            // if childNode is not null, it means the actual childNodes list is
            // longer than the virtual children list.
            if (!childrenMatch || childNode) {
              /* istanbul ignore if */
              if ("development" !== 'production' &&
                typeof console !== 'undefined' &&
                !bailed
              ) {
                bailed = true;
                console.warn('Parent: ', elm);
                console.warn('Mismatching childNodes vs. VNodes: ', elm.childNodes, children);
              }
              return false
            }
          }
        }
      }
      if (isDef(data)) {
        for (var key in data) {
          if (!isRenderedModule(key)) {
            invokeCreateHooks(vnode, insertedVnodeQueue);
            break
          }
        }
      }
    } else if (elm.data !== vnode.text) {
      elm.data = vnode.text;
    }
    return true
  }

  function assertNodeMatch (node, vnode) {
    if (isDef(vnode.tag)) {
      return (
        vnode.tag.indexOf('vue-component') === 0 ||
        vnode.tag.toLowerCase() === (node.tagName && node.tagName.toLowerCase())
      )
    } else {
      return node.nodeType === (vnode.isComment ? 8 : 3)
    }
  }

  return function patch (oldVnode, vnode, hydrating, removeOnly, parentElm, refElm) {
    if (isUndef(vnode)) {
      if (isDef(oldVnode)) { invokeDestroyHook(oldVnode); }
      return
    }

    var isInitialPatch = false;
    var insertedVnodeQueue = [];

    if (isUndef(oldVnode)) {
      // empty mount (likely as component), create new root element
      isInitialPatch = true;
      createElm(vnode, insertedVnodeQueue, parentElm, refElm);
    } else {
      var isRealElement = isDef(oldVnode.nodeType);
      if (!isRealElement && sameVnode(oldVnode, vnode)) {
        // patch existing root node
        patchVnode(oldVnode, vnode, insertedVnodeQueue, removeOnly);
      } else {
        if (isRealElement) {
          // mounting to a real element
          // check if this is server-rendered content and if we can perform
          // a successful hydration.
          if (oldVnode.nodeType === 1 && oldVnode.hasAttribute(SSR_ATTR)) {
            oldVnode.removeAttribute(SSR_ATTR);
            hydrating = true;
          }
          if (isTrue(hydrating)) {
            if (hydrate(oldVnode, vnode, insertedVnodeQueue)) {
              invokeInsertHook(vnode, insertedVnodeQueue, true);
              return oldVnode
            } else {
              warn(
                'The client-side rendered virtual DOM tree is not matching ' +
                'server-rendered content. This is likely caused by incorrect ' +
                'HTML markup, for example nesting block-level elements inside ' +
                '<p>, or missing <tbody>. Bailing hydration and performing ' +
                'full client-side render.'
              );
            }
          }
          // either not server-rendered, or hydration failed.
          // create an empty node and replace it
          oldVnode = emptyNodeAt(oldVnode);
        }
        // replacing existing element
        var oldElm = oldVnode.elm;
        var parentElm$1 = nodeOps.parentNode(oldElm);
        createElm(
          vnode,
          insertedVnodeQueue,
          // extremely rare edge case: do not insert if old element is in a
          // leaving transition. Only happens when combining transition +
          // keep-alive + HOCs. (#4590)
          oldElm._leaveCb ? null : parentElm$1,
          nodeOps.nextSibling(oldElm)
        );

        if (isDef(vnode.parent)) {
          // component root element replaced.
          // update parent placeholder node element, recursively
          var ancestor = vnode.parent;
          var patchable = isPatchable(vnode);
          while (ancestor) {
            for (var i = 0; i < cbs.destroy.length; ++i) {
              cbs.destroy[i](ancestor);
            }
            ancestor.elm = vnode.elm;
            if (patchable) {
              for (var i$1 = 0; i$1 < cbs.create.length; ++i$1) {
                cbs.create[i$1](emptyNode, ancestor);
              }
              // #6513
              // invoke insert hooks that may have been merged by create hooks.
              // e.g. for directives that uses the "inserted" hook.
              var insert = ancestor.data.hook.insert;
              if (insert.merged) {
                // start at index 1 to avoid re-invoking component mounted hook
                for (var i$2 = 1; i$2 < insert.fns.length; i$2++) {
                  insert.fns[i$2]();
                }
              }
            }
            ancestor = ancestor.parent;
          }
        }

        if (isDef(parentElm$1)) {
          removeVnodes(parentElm$1, [oldVnode], 0, 0);
        } else if (isDef(oldVnode.tag)) {
          invokeDestroyHook(oldVnode);
        }
      }
    }

    invokeInsertHook(vnode, insertedVnodeQueue, isInitialPatch);
    return vnode.elm
  }
}

/*  */

var directives = {
  create: updateDirectives,
  update: updateDirectives,
  destroy: function unbindDirectives (vnode) {
    updateDirectives(vnode, emptyNode);
  }
};

function updateDirectives (oldVnode, vnode) {
  if (oldVnode.data.directives || vnode.data.directives) {
    _update(oldVnode, vnode);
  }
}

function _update (oldVnode, vnode) {
  var isCreate = oldVnode === emptyNode;
  var isDestroy = vnode === emptyNode;
  var oldDirs = normalizeDirectives$1(oldVnode.data.directives, oldVnode.context);
  var newDirs = normalizeDirectives$1(vnode.data.directives, vnode.context);

  var dirsWithInsert = [];
  var dirsWithPostpatch = [];

  var key, oldDir, dir;
  for (key in newDirs) {
    oldDir = oldDirs[key];
    dir = newDirs[key];
    if (!oldDir) {
      // new directive, bind
      callHook$1(dir, 'bind', vnode, oldVnode);
      if (dir.def && dir.def.inserted) {
        dirsWithInsert.push(dir);
      }
    } else {
      // existing directive, update
      dir.oldValue = oldDir.value;
      callHook$1(dir, 'update', vnode, oldVnode);
      if (dir.def && dir.def.componentUpdated) {
        dirsWithPostpatch.push(dir);
      }
    }
  }

  if (dirsWithInsert.length) {
    var callInsert = function () {
      for (var i = 0; i < dirsWithInsert.length; i++) {
        callHook$1(dirsWithInsert[i], 'inserted', vnode, oldVnode);
      }
    };
    if (isCreate) {
      mergeVNodeHook(vnode.data.hook || (vnode.data.hook = {}), 'insert', callInsert);
    } else {
      callInsert();
    }
  }

  if (dirsWithPostpatch.length) {
    mergeVNodeHook(vnode.data.hook || (vnode.data.hook = {}), 'postpatch', function () {
      for (var i = 0; i < dirsWithPostpatch.length; i++) {
        callHook$1(dirsWithPostpatch[i], 'componentUpdated', vnode, oldVnode);
      }
    });
  }

  if (!isCreate) {
    for (key in oldDirs) {
      if (!newDirs[key]) {
        // no longer present, unbind
        callHook$1(oldDirs[key], 'unbind', oldVnode, oldVnode, isDestroy);
      }
    }
  }
}

var emptyModifiers = Object.create(null);

function normalizeDirectives$1 (
  dirs,
  vm
) {
  var res = Object.create(null);
  if (!dirs) {
    return res
  }
  var i, dir;
  for (i = 0; i < dirs.length; i++) {
    dir = dirs[i];
    if (!dir.modifiers) {
      dir.modifiers = emptyModifiers;
    }
    res[getRawDirName(dir)] = dir;
    dir.def = resolveAsset(vm.$options, 'directives', dir.name, true);
  }
  return res
}

function getRawDirName (dir) {
  return dir.rawName || ((dir.name) + "." + (Object.keys(dir.modifiers || {}).join('.')))
}

function callHook$1 (dir, hook, vnode, oldVnode, isDestroy) {
  var fn = dir.def && dir.def[hook];
  if (fn) {
    try {
      fn(vnode.elm, dir, vnode, oldVnode, isDestroy);
    } catch (e) {
      handleError(e, vnode.context, ("directive " + (dir.name) + " " + hook + " hook"));
    }
  }
}

var baseModules = [
  ref,
  directives
];

/*  */

function updateAttrs (oldVnode, vnode) {
  if (!oldVnode.data.attrs && !vnode.data.attrs) {
    return
  }
  var key, cur, old;
  var elm = vnode.elm;
  var oldAttrs = oldVnode.data.attrs || {};
  var attrs = vnode.data.attrs || {};
  // clone observed objects, as the user probably wants to mutate it
  if (attrs.__ob__) {
    attrs = vnode.data.attrs = extend({}, attrs);
  }

  for (key in attrs) {
    cur = attrs[key];
    old = oldAttrs[key];
    if (old !== cur) {
      elm.setAttr(key, cur);
    }
  }
  for (key in oldAttrs) {
    if (attrs[key] == null) {
      elm.setAttr(key);
    }
  }
}

var attrs = {
  create: updateAttrs,
  update: updateAttrs
};

/*  */

function updateClass (oldVnode, vnode) {
  var el = vnode.elm;
  var ctx = vnode.context;

  var data = vnode.data;
  var oldData = oldVnode.data;
  if (!data.staticClass &&
    !data.class &&
    (!oldData || (!oldData.staticClass && !oldData.class))
  ) {
    return
  }

  var oldClassList = [];
  // unlike web, weex vnode staticClass is an Array
  var oldStaticClass = oldData.staticClass;
  if (oldStaticClass) {
    oldClassList.push.apply(oldClassList, oldStaticClass);
  }
  if (oldData.class) {
    oldClassList.push.apply(oldClassList, oldData.class);
  }

  var classList = [];
  // unlike web, weex vnode staticClass is an Array
  var staticClass = data.staticClass;
  if (staticClass) {
    classList.push.apply(classList, staticClass);
  }
  if (data.class) {
    classList.push.apply(classList, data.class);
  }

  var style = getStyle(oldClassList, classList, ctx);
  for (var key in style) {
    el.setStyle(key, style[key]);
  }
}

function getStyle (oldClassList, classList, ctx) {
  // style is a weex-only injected object
  // compiled from <style> tags in weex files
  var stylesheet = ctx.$options.style || {};
  var result = {};
  classList.forEach(function (name) {
    var style = stylesheet[name];
    extend(result, style);
  });
  oldClassList.forEach(function (name) {
    var style = stylesheet[name];
    for (var key in style) {
      if (!result.hasOwnProperty(key)) {
        result[key] = '';
      }
    }
  });
  return result
}

var klass = {
  create: updateClass,
  update: updateClass
};

/*  */

var target$1;

function add$1 (
  event,
  handler,
  once,
  capture,
  passive,
  params
) {
  if (capture) {
    console.log('Weex do not support event in bubble phase.');
    return
  }
  if (once) {
    var oldHandler = handler;
    var _target = target$1; // save current target element in closure
    handler = function (ev) {
      var res = arguments.length === 1
        ? oldHandler(ev)
        : oldHandler.apply(null, arguments);
      if (res !== null) {
        remove$2(event, null, null, _target);
      }
    };
  }
  target$1.addEvent(event, handler, params);
}

function remove$2 (
  event,
  handler,
  capture,
  _target
) {
  (_target || target$1).removeEvent(event);
}

function updateDOMListeners (oldVnode, vnode) {
  if (!oldVnode.data.on && !vnode.data.on) {
    return
  }
  var on = vnode.data.on || {};
  var oldOn = oldVnode.data.on || {};
  target$1 = vnode.elm;
  updateListeners(on, oldOn, add$1, remove$2, vnode.context);
}

var events = {
  create: updateDOMListeners,
  update: updateDOMListeners
};

/*  */

var normalize = cached(camelize);

function createStyle (oldVnode, vnode) {
  if (!vnode.data.staticStyle) {
    updateStyle(oldVnode, vnode);
    return
  }
  var elm = vnode.elm;
  var staticStyle = vnode.data.staticStyle;
  for (var name in staticStyle) {
    if (staticStyle[name]) {
      elm.setStyle(normalize(name), staticStyle[name]);
    }
  }
  updateStyle(oldVnode, vnode);
}

function updateStyle (oldVnode, vnode) {
  if (!oldVnode.data.style && !vnode.data.style) {
    return
  }
  var cur, name;
  var elm = vnode.elm;
  var oldStyle = oldVnode.data.style || {};
  var style = vnode.data.style || {};

  var needClone = style.__ob__;

  // handle array syntax
  if (Array.isArray(style)) {
    style = vnode.data.style = toObject$1(style);
  }

  // clone the style for future updates,
  // in case the user mutates the style object in-place.
  if (needClone) {
    style = vnode.data.style = extend({}, style);
  }

  for (name in oldStyle) {
    if (!style[name]) {
      elm.setStyle(normalize(name), '');
    }
  }
  for (name in style) {
    cur = style[name];
    elm.setStyle(normalize(name), cur);
  }
}

function toObject$1 (arr) {
  var res = {};
  for (var i = 0; i < arr.length; i++) {
    if (arr[i]) {
      extend(res, arr[i]);
    }
  }
  return res
}

var style = {
  create: createStyle,
  update: updateStyle
};

/*  */

/**
 * Add class with compatibility for SVG since classList is not supported on
 * SVG elements in IE
 */


/**
 * Remove class with compatibility for SVG since classList is not supported on
 * SVG elements in IE
 */

/*  */

function resolveTransition (def$$1) {
  if (!def$$1) {
    return
  }
  /* istanbul ignore else */
  if (typeof def$$1 === 'object') {
    var res = {};
    if (def$$1.css !== false) {
      extend(res, autoCssTransition(def$$1.name || 'v'));
    }
    extend(res, def$$1);
    return res
  } else if (typeof def$$1 === 'string') {
    return autoCssTransition(def$$1)
  }
}

var autoCssTransition = cached(function (name) {
  return {
    enterClass: (name + "-enter"),
    enterToClass: (name + "-enter-to"),
    enterActiveClass: (name + "-enter-active"),
    leaveClass: (name + "-leave"),
    leaveToClass: (name + "-leave-to"),
    leaveActiveClass: (name + "-leave-active")
  }
});


// Transition property/event sniffing




// binding to window is necessary to make hot reload work in IE in strict mode
var raf = inBrowser && window.requestAnimationFrame
  ? window.requestAnimationFrame.bind(window)
  : setTimeout;

var transition = {
  create: enter,
  activate: enter,
  remove: leave
};

function enter (_, vnode) {
  var el = vnode.elm;

  // call leave callback now
  if (el._leaveCb) {
    el._leaveCb.cancelled = true;
    el._leaveCb();
  }

  var data = resolveTransition(vnode.data.transition);
  if (!data) {
    return
  }

  /* istanbul ignore if */
  if (el._enterCb) {
    return
  }

  var enterClass = data.enterClass;
  var enterToClass = data.enterToClass;
  var enterActiveClass = data.enterActiveClass;
  var appearClass = data.appearClass;
  var appearToClass = data.appearToClass;
  var appearActiveClass = data.appearActiveClass;
  var beforeEnter = data.beforeEnter;
  var enter = data.enter;
  var afterEnter = data.afterEnter;
  var enterCancelled = data.enterCancelled;
  var beforeAppear = data.beforeAppear;
  var appear = data.appear;
  var afterAppear = data.afterAppear;
  var appearCancelled = data.appearCancelled;

  var context = activeInstance;
  var transitionNode = activeInstance.$vnode;
  while (transitionNode && transitionNode.parent) {
    transitionNode = transitionNode.parent;
    context = transitionNode.context;
  }

  var isAppear = !context._isMounted || !vnode.isRootInsert;

  if (isAppear && !appear && appear !== '') {
    return
  }

  var startClass = isAppear ? appearClass : enterClass;
  var toClass = isAppear ? appearToClass : enterToClass;
  var activeClass = isAppear ? appearActiveClass : enterActiveClass;
  var beforeEnterHook = isAppear ? (beforeAppear || beforeEnter) : beforeEnter;
  var enterHook = isAppear ? (typeof appear === 'function' ? appear : enter) : enter;
  var afterEnterHook = isAppear ? (afterAppear || afterEnter) : afterEnter;
  var enterCancelledHook = isAppear ? (appearCancelled || enterCancelled) : enterCancelled;

  var userWantsControl =
    enterHook &&
    // enterHook may be a bound method which exposes
    // the length of original fn as _length
    (enterHook._length || enterHook.length) > 1;

  var stylesheet = vnode.context.$options.style || {};
  var startState = stylesheet[startClass];
  var transitionProperties = (stylesheet['@TRANSITION'] && stylesheet['@TRANSITION'][activeClass]) || {};
  var endState = getEnterTargetState(el, stylesheet, startClass, toClass, activeClass, vnode.context);
  var needAnimation = Object.keys(endState).length > 0;

  var cb = el._enterCb = once(function () {
    if (cb.cancelled) {
      enterCancelledHook && enterCancelledHook(el);
    } else {
      afterEnterHook && afterEnterHook(el);
    }
    el._enterCb = null;
  });

  // We need to wait until the native element has been inserted, but currently
  // there's no API to do that. So we have to wait "one frame" - not entirely
  // sure if this is guaranteed to be enough (e.g. on slow devices?)
  setTimeout(function () {
    var parent = el.parentNode;
    var pendingNode = parent && parent._pending && parent._pending[vnode.key];
    if (pendingNode &&
      pendingNode.context === vnode.context &&
      pendingNode.tag === vnode.tag &&
      pendingNode.elm._leaveCb
    ) {
      pendingNode.elm._leaveCb();
    }
    enterHook && enterHook(el, cb);

    if (needAnimation) {
      var animation = vnode.context.$requireWeexModule('animation');
      animation.transition(el.ref, {
        styles: endState,
        duration: transitionProperties.duration || 0,
        delay: transitionProperties.delay || 0,
        timingFunction: transitionProperties.timingFunction || 'linear'
      }, userWantsControl ? noop : cb);
    } else if (!userWantsControl) {
      cb();
    }
  }, 16);

  // start enter transition
  beforeEnterHook && beforeEnterHook(el);

  if (startState) {
    for (var key in startState) {
      el.setStyle(key, startState[key]);
    }
  }

  if (!needAnimation && !userWantsControl) {
    cb();
  }
}

function leave (vnode, rm) {
  var el = vnode.elm;

  // call enter callback now
  if (el._enterCb) {
    el._enterCb.cancelled = true;
    el._enterCb();
  }

  var data = resolveTransition(vnode.data.transition);
  if (!data) {
    return rm()
  }

  if (el._leaveCb) {
    return
  }

  var leaveClass = data.leaveClass;
  var leaveToClass = data.leaveToClass;
  var leaveActiveClass = data.leaveActiveClass;
  var beforeLeave = data.beforeLeave;
  var leave = data.leave;
  var afterLeave = data.afterLeave;
  var leaveCancelled = data.leaveCancelled;
  var delayLeave = data.delayLeave;

  var userWantsControl =
    leave &&
    // leave hook may be a bound method which exposes
    // the length of original fn as _length
    (leave._length || leave.length) > 1;

  var stylesheet = vnode.context.$options.style || {};
  var startState = stylesheet[leaveClass];
  var endState = stylesheet[leaveToClass] || stylesheet[leaveActiveClass];
  var transitionProperties = (stylesheet['@TRANSITION'] && stylesheet['@TRANSITION'][leaveActiveClass]) || {};

  var cb = el._leaveCb = once(function () {
    if (el.parentNode && el.parentNode._pending) {
      el.parentNode._pending[vnode.key] = null;
    }
    if (cb.cancelled) {
      leaveCancelled && leaveCancelled(el);
    } else {
      rm();
      afterLeave && afterLeave(el);
    }
    el._leaveCb = null;
  });

  if (delayLeave) {
    delayLeave(performLeave);
  } else {
    performLeave();
  }

  function performLeave () {
    var animation = vnode.context.$requireWeexModule('animation');
    // the delayed leave may have already been cancelled
    if (cb.cancelled) {
      return
    }
    // record leaving element
    if (!vnode.data.show) {
      (el.parentNode._pending || (el.parentNode._pending = {}))[vnode.key] = vnode;
    }
    beforeLeave && beforeLeave(el);

    if (startState) {
      animation.transition(el.ref, {
        styles: startState
      }, next);
    } else {
      next();
    }

    function next () {
      animation.transition(el.ref, {
        styles: endState,
        duration: transitionProperties.duration || 0,
        delay: transitionProperties.delay || 0,
        timingFunction: transitionProperties.timingFunction || 'linear'
      }, userWantsControl ? noop : cb);
    }

    leave && leave(el, cb);
    if (!endState && !userWantsControl) {
      cb();
    }
  }
}

// determine the target animation style for an entering transition.
function getEnterTargetState (el, stylesheet, startClass, endClass, activeClass, vm) {
  var targetState = {};
  var startState = stylesheet[startClass];
  var endState = stylesheet[endClass];
  var activeState = stylesheet[activeClass];
  // 1. fallback to element's default styling
  if (startState) {
    for (var key in startState) {
      targetState[key] = el.style[key];
      if (
        "development" !== 'production' &&
        targetState[key] == null &&
        (!activeState || activeState[key] == null) &&
        (!endState || endState[key] == null)
      ) {
        warn(
          "transition property \"" + key + "\" is declared in enter starting class (." + startClass + "), " +
          "but not declared anywhere in enter ending class (." + endClass + "), " +
          "enter active cass (." + activeClass + ") or the element's default styling. " +
          "Note in Weex, CSS properties need explicit values to be transitionable."
        );
      }
    }
  }
  // 2. if state is mixed in active state, extract them while excluding
  //    transition properties
  if (activeState) {
    for (var key$1 in activeState) {
      if (key$1.indexOf('transition') !== 0) {
        targetState[key$1] = activeState[key$1];
      }
    }
  }
  // 3. explicit endState has highest priority
  if (endState) {
    extend(targetState, endState);
  }
  return targetState
}

var platformModules = [
  attrs,
  klass,
  events,
  style,
  transition
];

/*  */

// the directive module should be applied last, after all
// built-in modules have been applied.
var modules = platformModules.concat(baseModules);

var patch = createPatchFunction({
  nodeOps: nodeOps,
  modules: modules,
  LONG_LIST_THRESHOLD: 10
});

var platformDirectives = {
};

function getVNodeType (vnode) {
  if (!vnode.tag) {
    return ''
  }
  return vnode.tag.replace(/vue\-component\-(\d+\-)?/, '')
}

function isSimpleSpan (vnode) {
  return vnode.children && vnode.children.length === 1 && !vnode.children[0].tag
}

var cssLengthRE = /^([+-]?[0-9]+(\.[0-9]+)?)(px|em|ex|%|in|cm|mm|pt|pc)$/i;
function trimCSSUnit (prop) {
  var res = String(prop).match(cssLengthRE);
  if (res) {
    return Number(res[1])
  }
  return prop
}

function parseStyle (vnode) {
  if (!vnode || !vnode.data) {
    return
  }

  var ref = vnode.data;
  var staticStyle = ref.staticStyle;
  var staticClass = ref.staticClass;
  if (vnode.data.style || vnode.data.class || staticStyle || staticClass) {
    var styles = Object.assign({}, staticStyle, vnode.data.style);

    var cssMap = vnode.context.$options.style || {};
    var classList = [].concat(staticClass, vnode.data.class);
    classList.forEach(function (name) {
      if (name && cssMap[name]) {
        Object.assign(styles, cssMap[name]);
      }
    });

    for (var key in styles) {
      styles[key] = trimCSSUnit(styles[key]);
    }
    return styles
  }
}

function convertVNodeChildren (children) {
  if (!children.length) {
    return
  }

  return children.map(function (vnode) {
    var type = getVNodeType(vnode);
    var props = { type: type };

    // convert raw text node
    if (!type) {
      props.type = 'span';
      props.attr = {
        value: (vnode.text || '').trim()
      };
    } else {
      props.style = parseStyle(vnode);
      if (vnode.data) {
        props.attr = vnode.data.attrs;
        if (vnode.data.on) {
          props.events = vnode.data.on;
        }
      }

      if (type === 'span' && isSimpleSpan(vnode)) {
        props.attr = props.attr || {};
        props.attr.value = vnode.children[0].text.trim();
        return props
      }
    }

    if (vnode.children && vnode.children.length) {
      props.children = convertVNodeChildren(vnode.children);
    }

    return props
  })
}

var Richtext = {
  name: 'richtext',
  // abstract: true,
  render: function render (h) {
    return h('weex:richtext', {
      on: this._events,
      attrs: {
        value: convertVNodeChildren(this.$options._renderChildren || [])
      }
    })
  }
};

/*  */

// Provides transition support for a single element/component.
// supports transition mode (out-in / in-out)

var transitionProps = {
  name: String,
  appear: Boolean,
  css: Boolean,
  mode: String,
  type: String,
  enterClass: String,
  leaveClass: String,
  enterToClass: String,
  leaveToClass: String,
  enterActiveClass: String,
  leaveActiveClass: String,
  appearClass: String,
  appearActiveClass: String,
  appearToClass: String,
  duration: [Number, String, Object]
};

// in case the child is also an abstract component, e.g. <keep-alive>
// we want to recursively retrieve the real component to be rendered
function getRealChild (vnode) {
  var compOptions = vnode && vnode.componentOptions;
  if (compOptions && compOptions.Ctor.options.abstract) {
    return getRealChild(getFirstComponentChild(compOptions.children))
  } else {
    return vnode
  }
}

function extractTransitionData (comp) {
  var data = {};
  var options = comp.$options;
  // props
  for (var key in options.propsData) {
    data[key] = comp[key];
  }
  // events.
  // extract listeners and pass them directly to the transition methods
  var listeners = options._parentListeners;
  for (var key$1 in listeners) {
    data[camelize(key$1)] = listeners[key$1];
  }
  return data
}

function placeholder (h, rawChild) {
  if (/\d-keep-alive$/.test(rawChild.tag)) {
    return h('keep-alive', {
      props: rawChild.componentOptions.propsData
    })
  }
}

function hasParentTransition (vnode) {
  while ((vnode = vnode.parent)) {
    if (vnode.data.transition) {
      return true
    }
  }
}

function isSameChild (child, oldChild) {
  return oldChild.key === child.key && oldChild.tag === child.tag
}

var Transition$1 = {
  name: 'transition',
  props: transitionProps,
  abstract: true,

  render: function render (h) {
    var this$1 = this;

    var children = this.$options._renderChildren;
    if (!children) {
      return
    }

    // filter out text nodes (possible whitespaces)
    children = children.filter(function (c) { return c.tag || isAsyncPlaceholder(c); });
    /* istanbul ignore if */
    if (!children.length) {
      return
    }

    // warn multiple elements
    if ("development" !== 'production' && children.length > 1) {
      warn(
        '<transition> can only be used on a single element. Use ' +
        '<transition-group> for lists.',
        this.$parent
      );
    }

    var mode = this.mode;

    // warn invalid mode
    if ("development" !== 'production' &&
      mode && mode !== 'in-out' && mode !== 'out-in'
    ) {
      warn(
        'invalid <transition> mode: ' + mode,
        this.$parent
      );
    }

    var rawChild = children[0];

    // if this is a component root node and the component's
    // parent container node also has transition, skip.
    if (hasParentTransition(this.$vnode)) {
      return rawChild
    }

    // apply transition data to child
    // use getRealChild() to ignore abstract components e.g. keep-alive
    var child = getRealChild(rawChild);
    /* istanbul ignore if */
    if (!child) {
      return rawChild
    }

    if (this._leaving) {
      return placeholder(h, rawChild)
    }

    // ensure a key that is unique to the vnode type and to this transition
    // component instance. This key will be used to remove pending leaving nodes
    // during entering.
    var id = "__transition-" + (this._uid) + "-";
    child.key = child.key == null
      ? child.isComment
        ? id + 'comment'
        : id + child.tag
      : isPrimitive(child.key)
        ? (String(child.key).indexOf(id) === 0 ? child.key : id + child.key)
        : child.key;

    var data = (child.data || (child.data = {})).transition = extractTransitionData(this);
    var oldRawChild = this._vnode;
    var oldChild = getRealChild(oldRawChild);

    // mark v-show
    // so that the transition module can hand over the control to the directive
    if (child.data.directives && child.data.directives.some(function (d) { return d.name === 'show'; })) {
      child.data.show = true;
    }

    if (
      oldChild &&
      oldChild.data &&
      !isSameChild(child, oldChild) &&
      !isAsyncPlaceholder(oldChild)
    ) {
      // replace old child transition data with fresh one
      // important for dynamic transitions!
      var oldData = oldChild && (oldChild.data.transition = extend({}, data));
      // handle transition mode
      if (mode === 'out-in') {
        // return placeholder node and queue update when leave finishes
        this._leaving = true;
        mergeVNodeHook(oldData, 'afterLeave', function () {
          this$1._leaving = false;
          this$1.$forceUpdate();
        });
        return placeholder(h, rawChild)
      } else if (mode === 'in-out') {
        if (isAsyncPlaceholder(child)) {
          return oldRawChild
        }
        var delayedLeave;
        var performLeave = function () { delayedLeave(); };
        mergeVNodeHook(data, 'afterEnter', performLeave);
        mergeVNodeHook(data, 'enterCancelled', performLeave);
        mergeVNodeHook(oldData, 'delayLeave', function (leave) { delayedLeave = leave; });
      }
    }

    return rawChild
  }
};

// reuse same transition component logic from web

var props = extend({
  tag: String,
  moveClass: String
}, transitionProps);

delete props.mode;

var TransitionGroup = {
  props: props,

  created: function created () {
    var dom = this.$requireWeexModule('dom');
    this.getPosition = function (el) { return new Promise(function (resolve, reject) {
      dom.getComponentRect(el.ref, function (res) {
        if (!res.result) {
          reject(new Error(("failed to get rect for element: " + (el.tag))));
        } else {
          resolve(res.size);
        }
      });
    }); };

    var animation = this.$requireWeexModule('animation');
    this.animate = function (el, options) { return new Promise(function (resolve) {
      animation.transition(el.ref, options, resolve);
    }); };
  },

  render: function render (h) {
    var tag = this.tag || this.$vnode.data.tag || 'span';
    var map = Object.create(null);
    var prevChildren = this.prevChildren = this.children;
    var rawChildren = this.$slots.default || [];
    var children = this.children = [];
    var transitionData = extractTransitionData(this);

    for (var i = 0; i < rawChildren.length; i++) {
      var c = rawChildren[i];
      if (c.tag) {
        if (c.key != null && String(c.key).indexOf('__vlist') !== 0) {
          children.push(c);
          map[c.key] = c
          ;(c.data || (c.data = {})).transition = transitionData;
        } else {
          var opts = c.componentOptions;
          var name = opts
            ? (opts.Ctor.options.name || opts.tag)
            : c.tag;
          warn(("<transition-group> children must be keyed: <" + name + ">"));
        }
      }
    }

    if (prevChildren) {
      var kept = [];
      var removed = [];
      prevChildren.forEach(function (c) {
        c.data.transition = transitionData;

        // TODO: record before patch positions

        if (map[c.key]) {
          kept.push(c);
        } else {
          removed.push(c);
        }
      });
      this.kept = h(tag, null, kept);
      this.removed = removed;
    }

    return h(tag, null, children)
  },

  beforeUpdate: function beforeUpdate () {
    // force removing pass
    this.__patch__(
      this._vnode,
      this.kept,
      false, // hydrating
      true // removeOnly (!important, avoids unnecessary moves)
    );
    this._vnode = this.kept;
  },

  updated: function updated () {
    var children = this.prevChildren;
    var moveClass = this.moveClass || ((this.name || 'v') + '-move');
    var moveData = children.length && this.getMoveData(children[0].context, moveClass);
    if (!moveData) {
      return
    }

    // TODO: finish implementing move animations once
    // we have access to sync getComponentRect()

    // children.forEach(callPendingCbs)

    // Promise.all(children.map(c => {
    //   const oldPos = c.data.pos
    //   const newPos = c.data.newPos
    //   const dx = oldPos.left - newPos.left
    //   const dy = oldPos.top - newPos.top
    //   if (dx || dy) {
    //     c.data.moved = true
    //     return this.animate(c.elm, {
    //       styles: {
    //         transform: `translate(${dx}px,${dy}px)`
    //       }
    //     })
    //   }
    // })).then(() => {
    //   children.forEach(c => {
    //     if (c.data.moved) {
    //       this.animate(c.elm, {
    //         styles: {
    //           transform: ''
    //         },
    //         duration: moveData.duration || 0,
    //         delay: moveData.delay || 0,
    //         timingFunction: moveData.timingFunction || 'linear'
    //       })
    //     }
    //   })
    // })
  },

  methods: {
    getMoveData: function getMoveData (context, moveClass) {
      var stylesheet = context.$options.style || {};
      return stylesheet['@TRANSITION'] && stylesheet['@TRANSITION'][moveClass]
    }
  }
};

// function callPendingCbs (c) {
//   /* istanbul ignore if */
//   if (c.elm._moveCb) {
//     c.elm._moveCb()
//   }
//   /* istanbul ignore if */
//   if (c.elm._enterCb) {
//     c.elm._enterCb()
//   }
// }

var platformComponents = {
  Richtext: Richtext,
  Transition: Transition$1,
  TransitionGroup: TransitionGroup
};

/* globals renderer */

var isReservedTag$1 = makeMap(
  'template,script,style,element,content,slot,link,meta,svg,view,' +
  'a,div,img,image,text,span,input,switch,textarea,spinner,select,' +
  'slider,slider-neighbor,indicator,canvas,' +
  'list,cell,header,loading,loading-indicator,refresh,scrollable,scroller,' +
  'video,web,embed,tabbar,tabheader,datepicker,timepicker,marquee,countdown',
  true
);

// Elements that you can, intentionally, leave open (and which close themselves)
// more flexible than web
var canBeLeftOpenTag = makeMap(
  'web,spinner,switch,video,textarea,canvas,' +
  'indicator,marquee,countdown',
  true
);

var isRuntimeComponent = makeMap(
  'richtext,trisition,trisition-group',
  true
);

var isUnaryTag = makeMap(
  'embed,img,image,input,link,meta',
  true
);

function mustUseProp () { /* console.log('mustUseProp') */ }

function isUnknownElement$1 () { /* console.log('isUnknownElement') */ }

function query (el, document) {
  // renderer is injected by weex factory wrapper
  var placeholder = new renderer.Comment('root');
  placeholder.hasAttribute = placeholder.removeAttribute = function () {}; // hack for patch
  document.documentElement.appendChild(placeholder);
  return placeholder
}

/*  */

// install platform specific utils
Vue$2.config.mustUseProp = mustUseProp;
Vue$2.config.isReservedTag = isReservedTag$1;
Vue$2.config.isRuntimeComponent = isRuntimeComponent;
Vue$2.config.isUnknownElement = isUnknownElement$1;

// install platform runtime directives and components
Vue$2.options.directives = platformDirectives;
Vue$2.options.components = platformComponents;

// install platform patch function
Vue$2.prototype.__patch__ = patch;

// wrap mount
Vue$2.prototype.$mount = function (
  el,
  hydrating
) {
  return mountComponent(
    this,
    el && query(el, this.$document),
    hydrating
  )
};

// this entry is built and wrapped with a factory function
// used to generate a fresh copy of Vue for every Weex instance.

exports.Vue = Vue$2;

};
});

var index = createCommonjsModule(function (module, exports) {
'use strict';

Object.defineProperty(exports, '__esModule', { value: true });

var latestNodeId = 1;

function TextNode (text) {
  this.instanceId = '';
  this.nodeId = latestNodeId++;
  this.parentNode = null;
  this.nodeType = 3;
  this.text = text;
}

// this will be preserved during build
var VueFactory = factory;

var instances = {};
var modules = {};
var components = {};

var renderer = {
  TextNode: TextNode,
  instances: instances,
  modules: modules,
  components: components
};

/**
 * Prepare framework config, basically about the virtual-DOM and JS bridge.
 * @param {object} cfg
 */
function init (cfg) {
  renderer.Document = cfg.Document;
  renderer.Element = cfg.Element;
  renderer.Comment = cfg.Comment;
  renderer.compileBundle = cfg.compileBundle;
}

/**
 * Reset framework config and clear all registrations.
 */
function reset () {
  clear(instances);
  clear(modules);
  clear(components);
  delete renderer.Document;
  delete renderer.Element;
  delete renderer.Comment;
  delete renderer.compileBundle;
}

/**
 * Delete all keys of an object.
 * @param {object} obj
 */
function clear (obj) {
  for (var key in obj) {
    delete obj[key];
  }
}

/**
 * Create an instance with id, code, config and external data.
 * @param {string} instanceId
 * @param {string} appCode
 * @param {object} config
 * @param {object} data
 * @param {object} env { info, config, services }
 */
function createInstance (
  instanceId,
  appCode,
  config,
  data,
  env
) {
  if ( appCode === void 0 ) { appCode = ''; }
  if ( config === void 0 ) { config = {}; }
  if ( env === void 0 ) { env = {}; }

  // Virtual-DOM object.
  var document = new renderer.Document(instanceId, config.bundleUrl);

  var instance = instances[instanceId] = {
    instanceId: instanceId, config: config, data: data,
    document: document
  };

  // Prepare native module getter and HTML5 Timer APIs.
  var moduleGetter = genModuleGetter(instanceId);
  var timerAPIs = getInstanceTimer(instanceId, moduleGetter);

  // Prepare `weex` instance variable.
  var weexInstanceVar = {
    config: config,
    document: document,
    supports: supports,
    requireModule: moduleGetter
  };
  Object.freeze(weexInstanceVar);

  // Each instance has a independent `Vue` module instance
  var Vue = instance.Vue = createVueModuleInstance(instanceId, moduleGetter);

  // The function which create a closure the JS Bundle will run in.
  // It will declare some instance variables like `Vue`, HTML5 Timer APIs etc.
  var instanceVars = Object.assign({
    Vue: Vue,
    weex: weexInstanceVar
  }, timerAPIs, env.services);

  appCode = "(function(global){ \n" + appCode + "\n })(Object.create(this))";

  if (!callFunctionNative(instanceVars, appCode)) {
    // If failed to compile functionBody on native side,
    // fallback to 'callFunction()'.
    callFunction(instanceVars, appCode);
  }

  // Send `createFinish` signal to native.
  instance.document.taskCenter.send('dom', { action: 'createFinish' }, []);

  return instance
}

/**
 * Destroy an instance with id. It will make sure all memory of
 * this instance released and no more leaks.
 * @param {string} instanceId
 */
function destroyInstance (instanceId) {
  var instance = instances[instanceId];
  if (instance && instance.app instanceof instance.Vue) {
    instance.document.destroy();
    instance.app.$destroy();
  }
  delete instances[instanceId];
}

/**
 * Refresh an instance with id and new top-level component data.
 * It will use `Vue.set` on all keys of the new data. So it's better
 * define all possible meaningful keys when instance created.
 * @param {string} instanceId
 * @param {object} data
 */
function refreshInstance (instanceId, data) {
  var instance = instances[instanceId];
  if (!instance || !(instance.app instanceof instance.Vue)) {
    return new Error(("refreshInstance: instance " + instanceId + " not found!"))
  }
  for (var key in data) {
    instance.Vue.set(instance.app, key, data[key]);
  }
  // Finally `refreshFinish` signal needed.
  instance.document.taskCenter.send('dom', { action: 'refreshFinish' }, []);
}

/**
 * Get the JSON object of the root element.
 * @param {string} instanceId
 */
function getRoot (instanceId) {
  var instance = instances[instanceId];
  if (!instance || !(instance.app instanceof instance.Vue)) {
    return new Error(("getRoot: instance " + instanceId + " not found!"))
  }
  return instance.app.$el.toJSON()
}

var jsHandlers = {
  fireEvent: function (id) {
    var arguments$1 = arguments;

    var args = [], len = arguments.length - 1;
    while ( len-- > 0 ) { args[ len ] = arguments$1[ len + 1 ]; }

    return fireEvent.apply(void 0, [ instances[id] ].concat( args ))
  },
  callback: function (id) {
    var arguments$1 = arguments;

    var args = [], len = arguments.length - 1;
    while ( len-- > 0 ) { args[ len ] = arguments$1[ len + 1 ]; }

    return callback.apply(void 0, [ instances[id] ].concat( args ))
  }
};

function fireEvent (instance, nodeId, type, e, domChanges, params) {
  var el = instance.document.getRef(nodeId);
  if (el) {
    return instance.document.fireEvent(el, type, e, domChanges, params)
  }
  return new Error(("invalid element reference \"" + nodeId + "\""))
}

function callback (instance, callbackId, data, ifKeepAlive) {
  var result = instance.document.taskCenter.callback(callbackId, data, ifKeepAlive);
  instance.document.taskCenter.send('dom', { action: 'updateFinish' }, []);
  return result
}

/**
 * Accept calls from native (event or callback).
 *
 * @param  {string} id
 * @param  {array} tasks list with `method` and `args`
 */
function receiveTasks (id, tasks) {
  var instance = instances[id];
  if (instance && Array.isArray(tasks)) {
    var results = [];
    tasks.forEach(function (task) {
      var handler = jsHandlers[task.method];
      var args = [].concat( task.args );
      /* istanbul ignore else */
      if (typeof handler === 'function') {
        args.unshift(id);
        results.push(handler.apply(void 0, args));
      }
    });
    return results
  }
  return new Error(("invalid instance id \"" + id + "\" or tasks"))
}

/**
 * Register native modules information.
 * @param {object} newModules
 */
function registerModules (newModules) {
  var loop = function ( name ) {
    if (!modules[name]) {
      modules[name] = {};
    }
    newModules[name].forEach(function (method) {
      if (typeof method === 'string') {
        modules[name][method] = true;
      } else {
        modules[name][method.name] = method.args;
      }
    });
  };

  for (var name in newModules) { loop( name ); }
}

/**
 * Check whether the module or the method has been registered.
 * @param {String} module name
 * @param {String} method name (optional)
 */
function isRegisteredModule (name, method) {
  if (typeof method === 'string') {
    return !!(modules[name] && modules[name][method])
  }
  return !!modules[name]
}

/**
 * Register native components information.
 * @param {array} newComponents
 */
function registerComponents (newComponents) {
  if (Array.isArray(newComponents)) {
    newComponents.forEach(function (component) {
      if (!component) {
        return
      }
      if (typeof component === 'string') {
        components[component] = true;
      } else if (typeof component === 'object' && typeof component.type === 'string') {
        components[component.type] = component;
      }
    });
  }
}

/**
 * Check whether the component has been registered.
 * @param {String} component name
 */
function isRegisteredComponent (name) {
  return !!components[name]
}

/**
 * Detects whether Weex supports specific features.
 * @param {String} condition
 */
function supports (condition) {
  if (typeof condition !== 'string') { return null }

  var res = condition.match(/^@(\w+)\/(\w+)(\.(\w+))?$/i);
  if (res) {
    var type = res[1];
    var name = res[2];
    var method = res[4];
    switch (type) {
      case 'module': return isRegisteredModule(name, method)
      case 'component': return isRegisteredComponent(name)
    }
  }

  return null
}

/**
 * Create a fresh instance of Vue for each Weex instance.
 */
function createVueModuleInstance (instanceId, moduleGetter) {
  var exports = {};
  VueFactory(exports, renderer);
  var Vue = exports.Vue;

  var instance = instances[instanceId];

  // patch reserved tag detection to account for dynamically registered
  // components
  var weexRegex = /^weex:/i;
  var isReservedTag = Vue.config.isReservedTag || (function () { return false; });
  var isRuntimeComponent = Vue.config.isRuntimeComponent || (function () { return false; });
  Vue.config.isReservedTag = function (name) {
    return (!isRuntimeComponent(name) && components[name]) ||
      isReservedTag(name) ||
      weexRegex.test(name)
  };
  Vue.config.parsePlatformTagName = function (name) { return name.replace(weexRegex, ''); };

  // expose weex-specific info
  Vue.prototype.$instanceId = instanceId;
  Vue.prototype.$document = instance.document;

  // expose weex native module getter on subVue prototype so that
  // vdom runtime modules can access native modules via vnode.context
  Vue.prototype.$requireWeexModule = moduleGetter;

  // Hack `Vue` behavior to handle instance information and data
  // before root component created.
  Vue.mixin({
    beforeCreate: function beforeCreate () {
      var options = this.$options;
      // root component (vm)
      if (options.el) {
        // set external data of instance
        var dataOption = options.data;
        var internalData = (typeof dataOption === 'function' ? dataOption() : dataOption) || {};
        options.data = Object.assign(internalData, instance.data);
        // record instance by id
        instance.app = this;
      }
    }
  });

  /**
   * @deprecated Just instance variable `weex.config`
   * Get instance config.
   * @return {object}
   */
  Vue.prototype.$getConfig = function () {
    if (instance.app instanceof Vue) {
      return instance.config
    }
  };

  return Vue
}

/**
 * Generate native module getter. Each native module has several
 * methods to call. And all the behaviors is instance-related. So
 * this getter will return a set of methods which additionally
 * send current instance id to native when called.
 * @param  {string}  instanceId
 * @return {function}
 */
function genModuleGetter (instanceId) {
  var instance = instances[instanceId];
  return function (name) {
    var nativeModule = modules[name] || [];
    var output = {};
    var loop = function ( methodName ) {
      Object.defineProperty(output, methodName, {
        enumerable: true,
        configurable: true,
        get: function proxyGetter () {
          return function () {
            var arguments$1 = arguments;

            var args = [], len = arguments.length;
            while ( len-- ) { args[ len ] = arguments$1[ len ]; }

            return instance.document.taskCenter.send('module', { module: name, method: methodName }, args)
          }
        },
        set: function proxySetter (val) {
          if (typeof val === 'function') {
            return instance.document.taskCenter.send('module', { module: name, method: methodName }, [val])
          }
        }
      });
    };

    for (var methodName in nativeModule) { loop( methodName ); }
    return output
  }
}

/**
 * Generate HTML5 Timer APIs. An important point is that the callback
 * will be converted into callback id when sent to native. So the
 * framework can make sure no side effect of the callback happened after
 * an instance destroyed.
 * @param  {[type]} instanceId   [description]
 * @param  {[type]} moduleGetter [description]
 * @return {[type]}              [description]
 */
function getInstanceTimer (instanceId, moduleGetter) {
  var instance = instances[instanceId];
  var timer = moduleGetter('timer');
  var timerAPIs = {
    setTimeout: function () {
      var arguments$1 = arguments;

      var args = [], len = arguments.length;
      while ( len-- ) { args[ len ] = arguments$1[ len ]; }

      var handler = function () {
        args[0].apply(args, args.slice(2));
      };

      timer.setTimeout(handler, args[1]);
      return instance.document.taskCenter.callbackManager.lastCallbackId.toString()
    },
    setInterval: function () {
      var arguments$1 = arguments;

      var args = [], len = arguments.length;
      while ( len-- ) { args[ len ] = arguments$1[ len ]; }

      var handler = function () {
        args[0].apply(args, args.slice(2));
      };

      timer.setInterval(handler, args[1]);
      return instance.document.taskCenter.callbackManager.lastCallbackId.toString()
    },
    clearTimeout: function (n) {
      timer.clearTimeout(n);
    },
    clearInterval: function (n) {
      timer.clearInterval(n);
    }
  };
  return timerAPIs
}

/**
 * Call a new function body with some global objects.
 * @param  {object} globalObjects
 * @param  {string} code
 * @return {any}
 */
function callFunction (globalObjects, body) {
  var globalKeys = [];
  var globalValues = [];
  for (var key in globalObjects) {
    globalKeys.push(key);
    globalValues.push(globalObjects[key]);
  }
  globalKeys.push(body);

  var result = new (Function.prototype.bind.apply( Function, [ null ].concat( globalKeys) ));
  return result.apply(void 0, globalValues)
}

/**
 * Call a new function generated on the V8 native side.
 *
 * This function helps speed up bundle compiling. Normally, the V8
 * engine needs to download, parse, and compile a bundle on every
 * visit. If 'compileBundle()' is available on native side,
 * the downloading, parsing, and compiling steps would be skipped.
 * @param  {object} globalObjects
 * @param  {string} body
 * @return {boolean}
 */
function callFunctionNative (globalObjects, body) {
  if (typeof renderer.compileBundle !== 'function') {
    return false
  }

  var fn = void 0;
  var isNativeCompileOk = false;
  var script = '(function (';
  var globalKeys = [];
  var globalValues = [];
  for (var key in globalObjects) {
    globalKeys.push(key);
    globalValues.push(globalObjects[key]);
  }
  for (var i = 0; i < globalKeys.length - 1; ++i) {
    script += globalKeys[i];
    script += ',';
  }
  script += globalKeys[globalKeys.length - 1];
  script += ') {';
  script += body;
  script += '} )';

  try {
    var weex = globalObjects.weex || {};
    var config = weex.config || {};
    fn = renderer.compileBundle(script,
      config.bundleUrl,
      config.bundleDigest,
      config.codeCachePath);
    if (fn && typeof fn === 'function') {
      fn.apply(void 0, globalValues);
      isNativeCompileOk = true;
    }
  } catch (e) {
    console.error(e);
  }

  return isNativeCompileOk
}

exports.init = init;
exports.reset = reset;
exports.createInstance = createInstance;
exports.destroyInstance = destroyInstance;
exports.refreshInstance = refreshInstance;
exports.getRoot = getRoot;
exports.receiveTasks = receiveTasks;
exports.registerModules = registerModules;
exports.isRegisteredModule = isRegisteredModule;
exports.registerComponents = registerComponents;
exports.isRegisteredComponent = isRegisteredComponent;
exports.supports = supports;
});

var index$1 = unwrapExports(index);
var supports$1 = index.supports;
var isRegisteredComponent$1 = index.isRegisteredComponent;
var registerComponents$1 = index.registerComponents;
var isRegisteredModule$1 = index.isRegisteredModule;
var registerModules$1 = index.registerModules;
var receiveTasks$1 = index.receiveTasks;
var getRoot$1 = index.getRoot;
var refreshInstance$1 = index.refreshInstance;
var destroyInstance$2 = index.destroyInstance;
var createInstance$2 = index.createInstance;
var reset = index.reset;
var init$4 = index.init;

var Vue = Object.freeze({
	default: index$1,
	__moduleExports: index,
	supports: supports$1,
	isRegisteredComponent: isRegisteredComponent$1,
	registerComponents: registerComponents$1,
	isRegisteredModule: isRegisteredModule$1,
	registerModules: registerModules$1,
	receiveTasks: receiveTasks$1,
	getRoot: getRoot$1,
	refreshInstance: refreshInstance$1,
	destroyInstance: destroyInstance$2,
	createInstance: createInstance$2,
	reset: reset,
	init: init$4
});

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
/**
 * @fileOverview The api for invoking with "$" prefix
 */

/**
 * @deprecated use $vm instead
 * find the vm by id
 * Note: there is only one id in whole component
 * @param  {string} id
 * @return {Vm}
 */
function $ (id) {
  console.warn('[JS Framework] Vm#$ is deprecated, please use Vm#$vm instead');
  var info = this._ids[id];
  if (info) {
    return info.vm
  }
}

/**
 * find the element by id
 * Note: there is only one id in whole component
 * @param  {string} id
 * @return {Element}
 */
function $el (id) {
  var info = this._ids[id];
  if (info) {
    return info.el
  }
}

/**
 * find the vm of the custom component by id
 * Note: there is only one id in whole component
 * @param  {string} id
 * @return {Vm}
 */
function $vm (id) {
  var info = this._ids[id];
  if (info) {
    return info.vm
  }
}

/**
 * Fire when differ rendering finished
 *
 * @param  {Function} fn
 */
function $renderThen (fn) {
  var app = this._app;
  var differ = app.differ;
  return differ.then(function () {
    fn();
  })
}

/**
 * scroll an element specified by id into view,
 * moreover specify a number of offset optionally
 * @param  {string} id
 * @param  {number} offset
 */
function $scrollTo (id, offset) {
  console.warn('[JS Framework] Vm#$scrollTo is deprecated, ' +
          'please use "require(\'@weex-module/dom\')' +
          '.scrollTo(el, options)" instead');
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
function $transition (id, options, callback) {
  var this$1 = this;

  var el = this.$el(id);
  if (el && options && options.styles) {
    var animation = this._app.requireModule('animation');
    animation.transition(el.ref, options, function () {
      var args = [], len = arguments.length;
      while ( len-- ) args[ len ] = arguments[ len ];

      this$1._setStyle(el, options.styles);
      callback && callback.apply(void 0, args);
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
function $getConfig (callback) {
  var config = this._app.options;
  if (typeof callback === 'function') {
    console.warn('[JS Framework] the callback of Vm#$getConfig(callback) is deprecated, ' +
      'this api now can directly RETURN config info.');
    callback(config);
  }
  return config
}

/**
 * @deprecated
 * request network via http protocol
 * @param  {object}   params
 * @param  {Function} callback
 */
function $sendHttp (params, callback) {
  console.warn('[JS Framework] Vm#$sendHttp is deprecated, ' +
          'please use "require(\'@weex-module/stream\')' +
          '.sendHttp(params, callback)" instead');
  var stream = this._app.requireModule('stream');
  stream.sendHttp(params, callback);
}

/**
 * @deprecated
 * open a url
 * @param  {string} url
 */
function $openURL (url) {
  console.warn('[JS Framework] Vm#$openURL is deprecated, ' +
          'please use "require(\'@weex-module/event\')' +
          '.openURL(url)" instead');
  var event = this._app.requireModule('event');
  event.openURL(url);
}

/**
 * @deprecated
 * set a title for page
 * @param  {string} title
 */
function $setTitle (title) {
  console.warn('[JS Framework] Vm#$setTitle is deprecated, ' +
          'please use "require(\'@weex-module/pageInfo\')' +
          '.setTitle(title)" instead');
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
function $call (moduleName, methodName) {
  var args = [], len = arguments.length - 2;
  while ( len-- > 0 ) args[ len ] = arguments[ len + 2 ];

  console.warn('[JS Framework] Vm#$call is deprecated, ' +
    'please use "require(\'@weex-module/moduleName\')" instead');
  var module = this._app.requireModule(moduleName);
  if (module && module[methodName]) {
    module[methodName].apply(module, args);
  }
}


var methods$1 = Object.freeze({
	$: $,
	$el: $el,
	$vm: $vm,
	$renderThen: $renderThen,
	$scrollTo: $scrollTo,
	$transition: $transition,
	$getConfig: $getConfig,
	$sendHttp: $sendHttp,
	$openURL: $openURL,
	$setTitle: $setTitle,
	$call: $call
});

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
/**
 * Mix properties into target object.
 *
 * @param {Object} to
 * @param {Object} from
 */

function extend (target) {
  var src = [], len = arguments.length - 1;
  while ( len-- > 0 ) src[ len ] = arguments[ len + 1 ];

  /* istanbul ignore else */
  if (typeof Object.assign === 'function') {
    Object.assign.apply(Object, [ target ].concat( src ));
  }
  else {
    var first = src.shift();
    for (var key in first) {
      target[key] = first[key];
    }
    if (src.length) {
      extend.apply(void 0, [ target ].concat( src ));
    }
  }
  return target
}

/**
 * Define a property.
 *
 * @param {Object} obj
 * @param {String} key
 * @param {*} val
 * @param {Boolean} [enumerable]
 */

function def$1 (obj, key, val, enumerable) {
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

function remove$1 (arr, item) {
  if (arr.length) {
    var index = arr.indexOf(item);
    if (index > -1) {
      return arr.splice(index, 1)
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
var hasOwnProperty$1 = Object.prototype.hasOwnProperty;
function hasOwn (obj, key) {
  return hasOwnProperty$1.call(obj, key)
}

/**
 * Simple bind, faster than native
 *
 * @param {Function} fn
 * @param {Object} ctx
 * @return {Function}
 */

function bind (fn, ctx) {
  return function (a) {
    var l = arguments.length;
    return l
      ? l > 1
        ? fn.apply(ctx, arguments)
        : fn.call(ctx, a)
      : fn.call(ctx)
  }
}

/**
 * Quick object check - this is primarily used to tell
 * Objects from primitive values when we know the value
 * is a JSON-compliant type.
 *
 * @param {*} obj
 * @return {Boolean}
 */

function isObject$4 (obj) {
  return obj !== null && typeof obj === 'object'
}

/**
 * Strict object type check. Only returns true
 * for plain JavaScript objects.
 *
 * @param {*} obj
 * @return {Boolean}
 */

var toString$2 = Object.prototype.toString;
var OBJECT_STRING = '[object Object]';
function isPlainObject (obj) {
  return toString$2.call(obj) === OBJECT_STRING
}

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
/**
 * Check if a string starts with $ or _
 *
 * @param {String} str
 * @return {Boolean}
 */

function isReserved (str) {
  var c = (str + '').charCodeAt(0);
  return c === 0x24 || c === 0x5F
}

// can we use __proto__?
var hasProto = '__proto__' in {};

var _Set;
/* istanbul ignore next */
if (typeof Set !== 'undefined' && Set.toString().match(/native code/)) {
  // use native Set when available.
  _Set = Set;
}
else {
  // a non-standard Set polyfill that only works with primitive keys.
  _Set = function () {
    this.set = Object.create(null);
  };
  _Set.prototype.has = function (key) {
    return this.set[key] !== undefined
  };
  _Set.prototype.add = function (key) {
    if (key == null || this.set[key]) {
      return
    }
    this.set[key] = 1;
  };
  _Set.prototype.clear = function () {
    this.set = Object.create(null);
  };
}

/**
 * Polyfill in iOS7 by native because the JavaScript polyfill has memory problem.
 * @return {object}
 */

function createNewSet () {
  /* istanbul ignore next */
  /* eslint-disable */
  if (typeof nativeSet === 'object') {
    return nativeSet.create()
  }
  /* eslint-enable */
  return new _Set()
}

/**
 * Create a cached version of a pure function.
 *
 * @param {Function} fn
 * @return {Function}
 */







function typof$1 (v) {
  var s = Object.prototype.toString.call(v);
  return s.substring(8, s.length - 1).toLowerCase()
}

// weex name rules

var WEEX_COMPONENT_REG = /^@weex-component\//;
var WEEX_MODULE_REG = /^@weex-module\//;
var NORMAL_MODULE_REG = /^\.{1,2}\//;
var JS_SURFIX_REG = /\.js$/;

var isWeexComponent = function (name) { return !!name.match(WEEX_COMPONENT_REG); };
var isWeexModule = function (name) { return !!name.match(WEEX_MODULE_REG); };
var isNormalModule = function (name) { return !!name.match(NORMAL_MODULE_REG); };
var isNpmModule = function (name) { return !isWeexComponent(name) && !isWeexModule(name) && !isNormalModule(name); };

function removeWeexPrefix (str) {
  var result = str.replace(WEEX_COMPONENT_REG, '').replace(WEEX_MODULE_REG, '');
  return result
}

function removeJSSurfix (str) {
  return str.replace(JS_SURFIX_REG, '')
}

/* eslint-disable */


var uid$2 = 0;

/**
 * A dep is an observable that can have multiple
 * directives subscribing to it.
 *
 * @constructor
 */

function Dep () {
  this.id = uid$2++;
  this.subs = [];
}

// the current target watcher being evaluated.
// this is globally unique because there could be only one
// watcher being evaluated at any time.
Dep.target = null;
var targetStack = [];

function pushTarget (_target) {
  if (Dep.target) { targetStack.push(Dep.target); }
  Dep.target = _target;
}

function popTarget () {
  Dep.target = targetStack.pop();
}

function resetTarget () {
  Dep.target = null;
  targetStack = [];
}

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
  remove$1(this.subs, sub);
};

/**
 * Add self as a dependency to the target watcher.
 */

Dep.prototype.depend = function () {
  if (Dep.target) {
    Dep.target.addDep(this);
  }
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

/* eslint-disable */


// import { pushWatcher } from './batcher'
var uid$1 = 0;

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

function Watcher (vm, expOrFn, cb, options) {
  // mix in options
  if (options) {
    extend(this, options);
  }
  var isFn = typeof expOrFn === 'function';
  this.vm = vm;
  vm._watchers.push(this);
  this.expression = expOrFn;
  this.cb = cb;
  this.id = ++uid$1; // uid for batching
  this.active = true;
  this.dirty = this.lazy; // for lazy watchers
  this.deps = [];
  this.newDeps = [];
  this.depIds = createNewSet(); // new Set()
  this.newDepIds = createNewSet(); // new Set()
  // parse expression for getter
  if (isFn) {
    this.getter = expOrFn;
  }
  this.value = this.lazy
    ? undefined
    : this.get();
  // state for avoiding false triggers for deep and Array
  // watchers during vm._digest()
  this.queued = this.shallow = false;
}

/**
 * Evaluate the getter, and re-collect dependencies.
 */

Watcher.prototype.get = function () {
  pushTarget(this);
  var value = this.getter.call(this.vm, this.vm);
  // "touch" every property so they are all tracked as
  // dependencies for deep watching
  if (this.deep) {
    traverse(value);
  }
  popTarget();
  this.cleanupDeps();
  return value
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

Watcher.prototype.cleanupDeps = function () {
  var this$1 = this;

  var i = this.deps.length;
  while (i--) {
    var dep = this$1.deps[i];
    if (!this$1.newDepIds.has(dep.id)) {
      dep.removeSub(this$1);
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
    if (
      value !== this.value ||
      // Deep watchers and watchers on Object/Arrays should fire even
      // when the value is the same, because the value may
      // have mutated; but only do so if this is a
      // non-shallow update (caused by a vm digest).
      ((isObject$4(value) || this.deep) && !this.shallow)
    ) {
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
  this.value = this.get();
  this.dirty = false;
};

/**
 * Depend on all deps collected by this watcher.
 */

Watcher.prototype.depend = function () {
  var this$1 = this;

  var i = this.deps.length;
  while (i--) {
    this$1.deps[i].depend();
  }
};

/**
 * Remove self from all dependencies' subcriber list.
 */

Watcher.prototype.teardown = function () {
  var this$1 = this;

  if (this.active) {
    // remove self from vm's watcher list
    // this is a somewhat expensive operation so we skip it
    // if the vm is being destroyed or is performing a v-for
    // re-render (the watcher list is then filtered by v-for).
    if (!this.vm._isBeingDestroyed && !this.vm._vForRemoving) {
      remove$1(this.vm._watchers, this);
    }
    var i = this.deps.length;
    while (i--) {
      this$1.deps[i].removeSub(this$1);
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

var seenObjects = createNewSet(); // new Set()
/* istanbul ignore next */
function traverse (val, seen) {
  var i, keys, isA, isO;
  if (!seen) {
    seen = seenObjects;
    seen.clear();
  }
  isA = Array.isArray(val);
  isO = isObject$4(val);
  if (isA || isO) {
    if (val.__ob__) {
      var depId = val.__ob__.dep.id;
      if (seen.has(depId)) {
        return
      } else {
        seen.add(depId);
      }
    }
    if (isA) {
      i = val.length;
      while (i--) { traverse(val[i], seen); }
    } else if (isO) {
      keys = Object.keys(val);
      i = keys.length;
      while (i--) { traverse(val[keys[i]], seen); }
    }
  }
}

/* eslint-disable */


var arrayProto = Array.prototype;
var arrayMethods = Object.create(arrayProto);[
  'push',
  'pop',
  'shift',
  'unshift',
  'splice',
  'sort',
  'reverse'
]
.forEach(function (method) {
  // cache original method
  var original = arrayProto[method];
  def$1(arrayMethods, method, function mutator () {
    var arguments$1 = arguments;

    // avoid leaking arguments:
    // http://jsperf.com/closure-with-arguments
    var i = arguments.length;
    var args = new Array(i);
    while (i--) {
      args[i] = arguments$1[i];
    }
    var result = original.apply(this, args);
    var ob = this.__ob__;
    var inserted;
    switch (method) {
      case 'push':
        inserted = args;
        break
      case 'unshift':
        inserted = args;
        break
      case 'splice':
        inserted = args.slice(2);
        break
    }
    if (inserted) { ob.observeArray(inserted); }
    // notify change
    ob.dep.notify();
    return result
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

def$1(
  arrayProto,
  '$set',
  function $set (index, val) {
    console.warn("[JS Framework] \"Array.prototype.$set\" is not a standard API,"
      + " it will be removed in the next version.");
    if (index >= this.length) {
      this.length = index + 1;
    }
    return this.splice(index, 1, val)[0]
  }
);

/**
 * Convenience method to remove the element at given index.
 *
 * @param {Number} index
 * @param {*} val
 */

def$1(
  arrayProto,
  '$remove',
  function $remove (index) {
    console.warn("[JS Framework] \"Array.prototype.$remove\" is not a standard API,"
      + " it will be removed in the next version.");
    /* istanbul ignore if */
    if (!this.length) { return }
    /* istanbul ignore else */
    if (typeof index !== 'number') {
      index = this.indexOf(index);
    }
    /* istanbul ignore else */
    if (index > -1) {
      this.splice(index, 1);
    }
  }
);

/* eslint-disable */


var arrayKeys = Object.getOwnPropertyNames(arrayMethods);

/**
 * Observer class that are attached to each observed
 * object. Once attached, the observer converts target
 * object's property keys into getter/setters that
 * collect dependencies and dispatches updates.
 *
 * @param {Array|Object} value
 * @constructor
 */

function Observer$1 (value) {
  this.value = value;
  this.dep = new Dep();
  def$1(value, '__ob__', this);
  if (Array.isArray(value)) {
    var augment = hasProto
      ? protoAugment
      : copyAugment;
    augment(value, arrayMethods, arrayKeys);
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

Observer$1.prototype.walk = function (obj) {
  var this$1 = this;

  for (var key in obj) {
    this$1.convert(key, obj[key]);
  }
};

/**
 * Observe a list of Array items.
 *
 * @param {Array} items
 */

Observer$1.prototype.observeArray = function (items) {
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

Observer$1.prototype.convert = function (key, val) {
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

Observer$1.prototype.addVm = function (vm) {
  (this.vms || (this.vms = [])).push(vm);
};

/**
 * Remove an owner vm. This is called when the object is
 * swapped out as an instance's $data object.
 *
 * @param {Vue} vm
 */

/* istanbul ignore next */
Observer$1.prototype.removeVm = function (vm) {
  remove$1(this.vms, vm);
};

// helpers

/**
 * Augment an target Object or Array by intercepting
 * the prototype chain using __proto__
 *
 * @param {Object|Array} target
 * @param {Object} src
 */

function protoAugment (target, src) {
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

/* istanbul ignore next */
function copyAugment (target, src, keys) {
  for (var i = 0, l = keys.length; i < l; i++) {
    var key = keys[i];
    def$1(target, key, src[key]);
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

function observe (value, vm) {
  if (!isObject$4(value)) {
    return
  }
  var ob;
  if (hasOwn(value, '__ob__') && value.__ob__ instanceof Observer$1) {
    ob = value.__ob__;
  } else if (
    (Array.isArray(value) || isPlainObject(value)) &&
    Object.isExtensible(value) &&
    !value._isVue
  ) {
    ob = new Observer$1(value);
  }
  if (ob && vm) {
    ob.addVm(vm);
  }
  return ob
}

/**
 * Define a reactive property on an Object.
 *
 * @param {Object} obj
 * @param {String} key
 * @param {*} val
 */

function defineReactive (obj, key, val) {
  var dep = new Dep();

  var property = Object.getOwnPropertyDescriptor(obj, key);
  if (property && property.configurable === false) {
    return
  }

  // cater for pre-defined getter/setters
  var getter = property && property.get;
  var setter = property && property.set;

  var childOb = observe(val);
  Object.defineProperty(obj, key, {
    enumerable: true,
    configurable: true,
    get: function reactiveGetter () {
      var value = getter ? getter.call(obj) : val;
      if (Dep.target) {
        dep.depend();
        if (childOb) {
          childOb.dep.depend();
        }
        if (Array.isArray(value)) {
          for (var e = (void 0), i = 0, l = value.length; i < l; i++) {
            e = value[i];
            e && e.__ob__ && e.__ob__.dep.depend();
          }
        }
      }
      return value
    },
    set: function reactiveSetter (newVal) {
      var value = getter ? getter.call(obj) : val;
      if (newVal === value) {
        return
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

/* istanbul ignore next */
function set$1 (obj, key, val) {
  if (Array.isArray(obj)) {
    return obj.splice(key, 1, val)
  }
  if (hasOwn(obj, key)) {
    obj[key] = val;
    return
  }
  if (obj._isVue) {
    set$1(obj._data, key, val);
    return
  }
  var ob = obj.__ob__;
  if (!ob) {
    obj[key] = val;
    return
  }
  ob.convert(key, val);
  ob.dep.notify();
  if (ob.vms) {
    var i = ob.vms.length;
    while (i--) {
      var vm = ob.vms[i];
      proxy(vm, key);
      // vm.$forceUpdate()
    }
  }
  return val
}

/**
 * Delete a property and trigger change if necessary.
 *
 * @param {Object} obj
 * @param {String} key
 */

/* istanbul ignore next */
function del (obj, key) {
  if (!hasOwn(obj, key)) {
    return
  }
  delete obj[key];
  var ob = obj.__ob__;

  if (!ob) {
    if (obj._isVue) {
      delete obj._data[key];
      // obj.$forceUpdate()
    }
    return
  }
  ob.dep.notify();
  if (ob.vms) {
    var i = ob.vms.length;
    while (i--) {
      var vm = ob.vms[i];
      unproxy(vm, key);
      // vm.$forceUpdate()
    }
  }
}

var KEY_WORDS = ['$index', '$value', '$event'];
function proxy (vm, key) {
  if (KEY_WORDS.indexOf(key) > -1 || !isReserved(key)) {
    Object.defineProperty(vm, key, {
      configurable: true,
      enumerable: true,
      get: function proxyGetter () {
        return vm._data[key]
      },
      set: function proxySetter (val) {
        vm._data[key] = val;
      }
    });
  }
}

/* istanbul ignore next */
function unproxy (vm, key) {
  if (!isReserved(key)) {
    delete vm[key];
  }
}

/* eslint-disable */


function initState (vm) {
  vm._watchers = [];
  initData(vm);
  initComputed(vm);
  initMethods(vm);
}

function initData (vm) {
  var data = vm._data;

  if (!isPlainObject(data)) {
    data = {};
  }
  // proxy data on instance
  var keys = Object.keys(data);
  var i = keys.length;
  while (i--) {
    proxy(vm, keys[i]);
  }
  // observe data
  observe(data, vm);
}

/* istanbul ignore next */
function noop () {
}

function initComputed (vm) {
  var computed = vm._computed;
  if (computed) {
    for (var key in computed) {
      var userDef = computed[key];
      var def$$1 = {
        enumerable: true,
        configurable: true
      };
      if (typeof userDef === 'function') {
        def$$1.get = makeComputedGetter(userDef, vm);
        def$$1.set = noop;
      } else {
        def$$1.get = userDef.get
          ? userDef.cache !== false
            ? makeComputedGetter(userDef.get, vm)
            : bind(userDef.get, vm)
          : noop;
        def$$1.set = userDef.set
          ? bind(userDef.set, vm)
          : noop;
      }
      Object.defineProperty(vm, key, def$$1);
    }
  }
}

function makeComputedGetter (getter, owner) {
  var watcher = new Watcher(owner, getter, null, {
    lazy: true
  });
  return function computedGetter () {
    if (watcher.dirty) {
      watcher.evaluate();
    }
    if (Dep.target) {
      watcher.depend();
    }
    return watcher.value
  }
}

function initMethods (vm) {
  var methods = vm._methods;
  if (methods) {
    for (var key in methods) {
      vm[key] = methods[key];
    }
  }
}

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
// @todo: It should be registered by native from `registerComponents()`.

var config$3 = {
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
/**
 * @fileOverview
 * Directive Parser
 */

var nativeComponentMap = config$3.nativeComponentMap;

var SETTERS = {
  attr: 'setAttr',
  style: 'setStyle',
  event: 'addEvent'
};

/**
 * apply the native component's options(specified by template.type)
 * to the template
 */
function applyNaitveComponentOptions (template) {
  var type = template.type;
  var options = nativeComponentMap[type];

  if (typeof options === 'object') {
    for (var key in options) {
      if (template[key] == null) {
        template[key] = options[key];
      }
      else if (typof$1(template[key]) === 'object' &&
        typof$1(options[key]) === 'object') {
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
function bindElement (vm, el, template) {
  setId$1(vm, el, template.id, vm);
  setAttr$1(vm, el, template.attr);
  setClass(vm, el, template.classList);
  setStyle$1(vm, el, template.style);
  bindEvents(vm, el, template.events);
}

/**
 * bind all props to sub vm and bind all style, events to the root element
 * of the sub vm if it doesn't have a replaced multi-node fragment
 */
function bindSubVm (vm, subVm, template, repeatItem) {
  subVm = subVm || {};
  template = template || {};

  var options = subVm._options || {};

  // bind props
  var props = options.props;

  if (Array.isArray(props)) {
    props = props.reduce(function (result, value) {
      result[value] = true;
      return result
    }, {});
  }

  mergeProps(repeatItem, props, vm, subVm);
  mergeProps(template.attr, props, vm, subVm);
}

/**
 * merge class and styles from vm to sub vm.
 */
function bindSubVmAfterInitialized (vm, subVm, template, target) {
  if ( target === void 0 ) target = {};

  mergeClassStyle(template.classList, vm, subVm);
  mergeStyle(template.style, vm, subVm);

  // bind subVm to the target element
  if (target.children) {
    target.children[target.children.length - 1]._vm = subVm;
  }
  else {
    target._vm = subVm;
  }
}

/**
 * Bind props from vm to sub vm and watch their updates.
 */
function mergeProps (target, props, vm, subVm) {
  if (!target) {
    return
  }
  var loop = function ( key ) {
    if (!props || props[key]) {
      var value = target[key];
      if (typeof value === 'function') {
        var returnValue = watch(vm, value, function (v) {
          subVm[key] = v;
        });
        subVm[key] = returnValue;
      }
      else {
        subVm[key] = value;
      }
    }
  };

  for (var key in target) loop( key );
}

/**
 * Bind style from vm to sub vm and watch their updates.
 */
function mergeStyle (target, vm, subVm) {
  var loop = function ( key ) {
    var value = target[key];
    if (typeof value === 'function') {
      var returnValue = watch(vm, value, function (v) {
        if (subVm._rootEl) {
          subVm._rootEl.setStyle(key, v);
        }
      });
      subVm._rootEl.setStyle(key, returnValue);
    }
    else {
      if (subVm._rootEl) {
        subVm._rootEl.setStyle(key, value);
      }
    }
  };

  for (var key in target) loop( key );
}

/**
 * Bind class & style from vm to sub vm and watch their updates.
 */
function mergeClassStyle (target, vm, subVm) {
  var css = vm._options && vm._options.style || {};

  /* istanbul ignore if */
  if (!subVm._rootEl) {
    return
  }

  var className = '@originalRootEl';
  css[className] = subVm._rootEl.classStyle;

  function addClassName (list, name) {
    if (typof$1(list) === 'array') {
      list.unshift(name);
    }
  }

  if (typeof target === 'function') {
    var value = watch(vm, target, function (v) {
      addClassName(v, className);
      setClassStyle$1(subVm._rootEl, css, v);
    });
    addClassName(value, className);
    setClassStyle$1(subVm._rootEl, css, value);
  }
  else if (target != null) {
    addClassName(target, className);
    setClassStyle$1(subVm._rootEl, css, target);
  }
}

/**
 * bind id to an element
 * each id is unique in a whole vm
 */
function setId$1 (vm, el, id, target) {
  var map = Object.create(null);

  Object.defineProperties(map, {
    vm: {
      value: target,
      writable: false,
      configurable: false
    },
    el: {
      get: function () { return el || target._rootEl; },
      configurable: false
    }
  });

  if (typeof id === 'function') {
    var handler = id;
    id = handler.call(vm);
    if (id || id === 0) {
      vm._ids[id] = map;
    }
    watch(vm, handler, function (newId) {
      if (newId) {
        vm._ids[newId] = map;
      }
    });
  }
  else if (id && typeof id === 'string') {
    vm._ids[id] = map;
  }
}

/**
 * bind attr to an element
 */
function setAttr$1 (vm, el, attr) {
  bindDir(vm, el, 'attr', attr);
}

function setClassStyle$1 (el, css, classList) {
  if (typeof classList === 'string') {
    classList = classList.split(/\s+/);
  }
  classList.forEach(function (name, i) {
    classList.splice.apply(classList, [ i, 1 ].concat( name.split(/\s+/) ));
  });
  var classStyle = {};
  var length = classList.length;

  var loop = function ( i ) {
    var style = css[classList[i]];
    if (style) {
      Object.keys(style).forEach(function (key) {
        classStyle[key] = style[key];
      });
    }
  };

  for (var i = 0; i < length; i++) loop( i );
  el.setClassStyle(classStyle);
}

/**
 * bind classnames to an element
 */
function setClass (vm, el, classList) {
  if (typeof classList !== 'function' && !Array.isArray(classList)) {
    return
  }
  if (Array.isArray(classList) && !classList.length) {
    el.setClassStyle({});
    return
  }

  var style = vm._options && vm._options.style || {};
  if (typeof classList === 'function') {
    var value = watch(vm, classList, function (v) {
      setClassStyle$1(el, style, v);
    });
    setClassStyle$1(el, style, value);
  }
  else {
    setClassStyle$1(el, style, classList);
  }
}

/**
 * bind style to an element
 */
function setStyle$1 (vm, el, style) {
  bindDir(vm, el, 'style', style);
}

/**
 * add an event type and handler to an element and generate a dom update
 */
function setEvent (vm, el, type, handler) {
  el.addEvent(type, bind(handler, vm));
}

/**
 * add all events of an element
 */
function bindEvents (vm, el, events) {
  if (!events) {
    return
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
        console.warn(("[JS Framework] The event handler \"" + handler + "\" is not defined."));
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
function bindDir (vm, el, name, data) {
  if (!data) {
    return
  }
  var keys = Object.keys(data);
  var i = keys.length;
  while (i--) {
    var key = keys[i];
    var value = data[key];
    if (typeof value === 'function') {
      bindKey(vm, el, name, key, value);
    }
    else {
      el[SETTERS[name]](key, value);
    }
  }
}

/**
 * bind data changes to a certain key to a name series in an element
 */
function bindKey (vm, el, name, key, calc) {
  var methodName = SETTERS[name];
  // watch the calc, and returns a value by calc.call()
  var value = watch(vm, calc, function (value) {
    function handler () {
      el[methodName](key, value);
    }
    var differ = vm && vm._app && vm._app.differ;
    if (differ) {
      differ.append('element', el.depth || 0, el.ref, handler);
    }
    else {
      handler();
    }
  });

  el[methodName](key, value);
}

/**
 * watch a calc function and callback if the calc value changes
 */
function watch (vm, calc, callback) {
  if (vm._static) {
    return calc.call(vm, vm)
  }
  var watcher = new Watcher(vm, calc, function (value, oldValue) {
    /* istanbul ignore if */
    if (typeof value !== 'object' && value === oldValue) {
      return
    }
    callback(value);
  });

  return watcher.value
}

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
function createBody$1 (vm, type) {
  var doc = vm._app.doc;
  return doc.createBody(type)
}

/**
 * Create an element by type
 * Using this._app.doc
 *
 * @param  {string} type
 */
function createElement$1 (vm, type) {
  var doc = vm._app.doc;
  return doc.createElement(type)
}

/**
 * Create and return a frag block for an element.
 * The frag block has a starter, ender and the element itself.
 *
 * @param  {object} element
 */
function createBlock (vm, element) {
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
    }
    else {
      element.element.insertBefore(start, element.end);
      element.element.insertBefore(end, element.end);
    }
    element = element.element;
  }
  else {
    element.appendChild(start);
    element.appendChild(end);
  }
  return { start: start, end: end, element: element, blockId: blockId }
}

var lastestBlockId = 1;

/**
 * Create and return a block starter.
 * Using this._app.doc
 */
function createBlockStart (vm) {
  var doc = vm._app.doc;
  var anchor = doc.createComment('start');
  return anchor
}

/**
 * Create and return a block ender.
 * Using this._app.doc
 */
function createBlockEnd (vm) {
  var doc = vm._app.doc;
  var anchor = doc.createComment('end');
  return anchor
}

/**
 * Attach target to a certain dest using appendChild by default.
 * If the dest is a frag block then insert before the ender.
 * If the target is a frag block then attach the starter and ender in order.
 *
 * @param  {object} target
 * @param  {object} dest
 */
function attachTarget (vm, target, dest) {
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
      return signal
    }
    else if (target.element) {
      dest.element.insertBefore(target.start, before);
      dest.element.insertBefore(target.end, before);
    }
    else {
      return dest.element.insertBefore(target, before)
    }
  }
  else {
    if (target.element) {
      dest.appendChild(target.start);
      dest.appendChild(target.end);
    }
    else {
      return dest.appendChild(target)
    }
  }
}

/**
 * Move target before a certain element. The target maybe block or element.
 *
 * @param  {object} target
 * @param  {object} before
 */
function moveTarget (vm, target, after) {
  if (target.element) {
    return moveBlock(target, after)
  }
  return moveElement$1(target, after)
}

/**
 * Move element before a certain element.
 *
 * @param  {object} element
 * @param  {object} before
 */
function moveElement$1 (element, after) {
  var parent = after.parentNode;
  if (parent) {
    return parent.insertAfter(element, after)
  }
}

/**
 * Move all elements of the block before a certain element.
 *
 * @param  {object} fragBlock
 * @param  {object} before
 */
function moveBlock (fragBlock, after) {
  var parent = after.parentNode;

  if (parent) {
    var el = fragBlock.start;
    var signal;
    var group = [el];

    while (el && el !== fragBlock.end) {
      el = el.nextSibling;
      group.push(el);
    }

    var temp = after;
    group.every(function (el) {
      signal = parent.insertAfter(el, temp);
      temp = el;
      return signal !== -1
    });

    return signal
  }
}

/**
 * Remove target from DOM tree.
 * If the target is a frag block then call _removeBlock
 *
 * @param  {object} target
 */
function removeTarget (vm, target, preserveBlock) {
  if ( preserveBlock === void 0 ) preserveBlock = false;

  if (target.element) {
    removeBlock(target, preserveBlock);
  }
  else {
    removeElement$1(target);
  }
  if (target._vm) {
    target._vm.$emit('hook:destroyed');
  }
}

/**
 * Remove a certain element.
 * Using this._app.doc
 *
 * @param  {object} target
 */
function removeElement$1 (target) {
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
function removeBlock (fragBlock, preserveBlock) {
  if ( preserveBlock === void 0 ) preserveBlock = false;

  var result = [];
  var el = fragBlock.start.nextSibling;

  while (el && el !== fragBlock.end) {
    result.push(el);
    el = el.nextSibling;
  }

  if (!preserveBlock) {
    removeElement$1(fragBlock.start);
  }
  result.forEach(function (el) {
    removeElement$1(el);
  });
  if (!preserveBlock) {
    removeElement$1(fragBlock.end);
  }
}

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
/**
 * @fileOverview
 * ViewModel template parser & data-binding process
 */

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
function build (vm) {
  var opt = vm._options || {};
  var template = opt.template || {};

  if (opt.replace) {
    if (template.children && template.children.length === 1) {
      compile(vm, template.children[0], vm._parentEl);
    }
    else {
      compile(vm, template.children, vm._parentEl);
    }
  }
  else {
    compile(vm, template, vm._parentEl);
  }

  console.debug(("[JS Framework] \"ready\" lifecycle in Vm(" + (vm._type) + ")"));
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
function compile (vm, target, dest, meta) {
  var app = vm._app || {};

  if (app.lastSignal === -1) {
    return
  }

  if (target.attr && target.attr.hasOwnProperty('static')) {
    vm._static = true;
  }

  if (targetIsFragment(target)) {
    compileFragment(vm, target, dest, meta);
    return
  }
  meta = meta || {};
  if (targetIsContent(target)) {
    console.debug('[JS Framework] compile "content" block by', target);
    vm._content = createBlock(vm, dest);
    return
  }

  if (targetNeedCheckRepeat(target, meta)) {
    console.debug('[JS Framework] compile "repeat" logic by', target);
    if (dest.type === 'document') {
      console.warn('[JS Framework] The root element does\'t support `repeat` directive!');
    }
    else {
      compileRepeat(vm, target, dest);
    }
    return
  }
  if (targetNeedCheckShown(target, meta)) {
    console.debug('[JS Framework] compile "if" logic by', target);
    if (dest.type === 'document') {
      console.warn('[JS Framework] The root element does\'t support `if` directive!');
    }
    else {
      compileShown(vm, target, dest, meta);
    }
    return
  }
  var typeGetter = meta.type || target.type;
  if (targetNeedCheckType(typeGetter, meta)) {
    compileType(vm, target, dest, typeGetter, meta);
    return
  }
  var type = typeGetter;
  var component = targetIsComposed(vm, target, type);
  if (component) {
    console.debug('[JS Framework] compile composed component by', target);
    compileCustomComponent(vm, component, target, dest, type, meta);
    return
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
function targetIsFragment (target) {
  return Array.isArray(target)
}

/**
 * Check if target type is content/slot.
 *
 * @param  {object}  target
 * @return {boolean}
 */
function targetIsContent (target) {
  return target.type === 'content' || target.type === 'slot'
}

/**
 * Check if target need to compile by a list.
 *
 * @param  {object}  target
 * @param  {object}  meta
 * @return {boolean}
 */
function targetNeedCheckRepeat (target, meta) {
  return !meta.hasOwnProperty('repeat') && target.repeat
}

/**
 * Check if target need to compile by a boolean value.
 *
 * @param  {object}  target
 * @param  {object}  meta
 * @return {boolean}
 */
function targetNeedCheckShown (target, meta) {
  return !meta.hasOwnProperty('shown') && target.shown
}

/**
 * Check if target need to compile by a dynamic type.
 *
 * @param  {string|function} typeGetter
 * @param  {object}          meta
 * @return {boolean}
 */
function targetNeedCheckType (typeGetter, meta) {
  return (typeof typeGetter === 'function') && !meta.hasOwnProperty('type')
}

/**
 * Check if this kind of component is composed.
 *
 * @param  {string}  type
 * @return {boolean}
 */
function targetIsComposed (vm, target, type) {
  var component;
  if (vm._app && vm._app.customComponentMap) {
    component = vm._app.customComponentMap[type];
  }
  if (vm._options && vm._options.components) {
    component = vm._options.components[type];
  }
  if (target.component) {
    component = component || {};
  }
  return component
}

/**
 * Compile a list of targets.
 *
 * @param {object} target
 * @param {object} dest
 * @param {object} meta
 */
function compileFragment (vm, target, dest, meta) {
  var fragBlock = createBlock(vm, dest);
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
function compileRepeat (vm, target, dest) {
  var repeat = target.repeat;
  var oldStyle = typeof repeat === 'function';
  var getter = repeat.getter || repeat.expression || repeat;
  if (typeof getter !== 'function') {
    getter = function () { return [] };
  }
  var key = repeat.key || '$index';
  var value = repeat.value || '$value';
  var trackBy = repeat.trackBy || target.trackBy ||
    (target.attr && target.attr.trackBy);

  var fragBlock = createBlock(vm, dest);
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
function compileShown (vm, target, dest, meta) {
  var newMeta = { shown: true };
  var fragBlock = createBlock(vm, dest);

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
function compileType (vm, target, dest, typeGetter, meta) {
  var type = typeGetter.call(vm);
  var newMeta = extend({ type: type }, meta);
  var fragBlock = createBlock(vm, dest);

  if (dest.element && dest.children) {
    dest.children.push(fragBlock);
  }

  watch(vm, typeGetter, function (value) {
    var newMeta = extend({ type: value }, meta);
    removeTarget(vm, fragBlock, true);
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
function compileCustomComponent (vm, component, target, dest, type, meta) {
  var Ctor = vm.constructor;
  var subVm = new Ctor(type, component, vm, dest, undefined, {
    'hook:init': function () {
      if (vm._static) {
        this._static = vm._static;
      }
      setId$1(vm, null, target.id, this);
      // bind template earlier because of lifecycle issues
      this._externalBinding = {
        parent: vm,
        template: target
      };
    },
    'hook:created': function () {
      bindSubVm(vm, this, target, meta.repeat);
    },
    'hook:ready': function () {
      if (this._content) {
        compileChildren(vm, target, this._content);
      }
    }
  });
  bindSubVmAfterInitialized(vm, subVm, target, dest);
}

/**
 * Generate element from template and attach to the dest if needed.
 * The time to attach depends on whether the mode status is node or tree.
 *
 * @param {object} template
 * @param {object} dest
 * @param {string} type
 */
function compileNativeComponent (vm, template, dest, type) {
  applyNaitveComponentOptions(template);

  var element;
  if (dest.ref === '_documentElement') {
    // if its parent is documentElement then it's a body
    console.debug(("[JS Framework] compile to create body for " + type));
    element = createBody$1(vm, type);
  }
  else {
    console.debug(("[JS Framework] compile to create element for " + type));
    element = createElement$1(vm, type);
  }

  if (!vm._rootEl) {
    vm._rootEl = element;
    // bind event earlier because of lifecycle issues
    var binding = vm._externalBinding || {};
    var target = binding.template;
    var parentVm = binding.parent;
    if (target && target.events && parentVm && element) {
      for (var type$1 in target.events) {
        var handler = parentVm[target.events[type$1]];
        if (handler) {
          element.addEvent(type$1, bind(handler, parentVm));
        }
      }
    }
  }

  bindElement(vm, element, template);

  if (template.attr && template.attr.append) { // backward, append prop in attr
    template.append = template.attr.append;
  }

  if (template.append) { // give the append attribute for ios adaptation
    element.attr = element.attr || {};
    element.attr.append = template.append;
  }

  var treeMode = template.append === 'tree';
  var app = vm._app || {};
  if (app.lastSignal !== -1 && !treeMode) {
    console.debug('[JS Framework] compile to append single node for', element);
    app.lastSignal = attachTarget(vm, element, dest);
  }
  if (app.lastSignal !== -1) {
    compileChildren(vm, template, element);
  }
  if (app.lastSignal !== -1 && treeMode) {
    console.debug('[JS Framework] compile to append whole tree for', element);
    app.lastSignal = attachTarget(vm, element, dest);
  }
}

/**
 * Set all children to a certain parent element.
 *
 * @param {object} template
 * @param {object} dest
 */
function compileChildren (vm, template, dest) {
  var app = vm._app || {};
  var children = template.children;
  if (children && children.length) {
    children.every(function (child) {
      compile(vm, child, dest);
      return app.lastSignal !== -1
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
function bindRepeat (vm, target, fragBlock, info) {
  var vms = fragBlock.vms;
  var children = fragBlock.children;
  var getter = info.getter;
  var trackBy = info.trackBy;
  var oldStyle = info.oldStyle;
  var keyName = info.key;
  var valueName = info.value;

  function compileItem (item, index, context) {
    var mergedData;
    if (oldStyle) {
      mergedData = item;
      if (isObject$4(item)) {
        mergedData[keyName] = index;
        if (!mergedData.hasOwnProperty('INDEX')) {
          Object.defineProperty(mergedData, 'INDEX', {
            value: function () {
              console.warn('[JS Framework] "INDEX" in repeat is deprecated, ' +
                'please use "$index" instead');
            }
          });
        }
      }
      else {
        console.warn('[JS Framework] Each list item must be an object in old-style repeat, '
          + 'please use `repeat={{v in list}}` instead.');
        mergedData = {};
        mergedData[keyName] = index;
        mergedData[valueName] = item;
      }
    }
    else {
      mergedData = {};
      mergedData[keyName] = index;
      mergedData[valueName] = item;
    }
    var newContext = mergeContext(context, mergedData);
    vms.push(newContext);
    compile(newContext, target, fragBlock, { repeat: item });
  }

  var list = watchBlock(vm, fragBlock, getter, 'repeat',
    function (data) {
      console.debug('[JS Framework] the "repeat" item has changed', data);
      if (!fragBlock || !data) {
        return
      }

      var oldChildren = children.slice();
      var oldVms = vms.slice();
      var oldData = fragBlock.data.slice();
      // 1. collect all new refs track by
      var trackMap = {};
      var reusedMap = {};
      data.forEach(function (item, index) {
        var key = trackBy ? item[trackBy] : (oldStyle ? item[keyName] : index);
        /* istanbul ignore if */
        if (key == null || key === '') {
          return
        }
        trackMap[key] = item;
      });

      // 2. remove unused element foreach old item
      var reusedList = [];
      oldData.forEach(function (item, index) {
        var key = trackBy ? item[trackBy] : (oldStyle ? item[keyName] : index);
        if (trackMap.hasOwnProperty(key)) {
          reusedMap[key] = {
            item: item, index: index, key: key,
            target: oldChildren[index],
            vm: oldVms[index]
          };
          reusedList.push(item);
        }
        else {
          removeTarget(vm, oldChildren[index]);
        }
      });

      // 3. create new element foreach new item
      children.length = 0;
      vms.length = 0;
      fragBlock.data = data.slice();
      fragBlock.updateMark = fragBlock.start;

      data.forEach(function (item, index) {
        var key = trackBy ? item[trackBy] : (oldStyle ? item[keyName] : index);
        var reused = reusedMap[key];
        if (reused) {
          if (reused.item === reusedList[0]) {
            reusedList.shift();
          }
          else {
            reusedList.$remove(reused.item);
            moveTarget(vm, reused.target, fragBlock.updateMark, true);
          }
          children.push(reused.target);
          vms.push(reused.vm);
          if (oldStyle) {
            reused.vm = item;
          }
          else {
            reused.vm[valueName] = item;
          }
          reused.vm[keyName] = index;
          fragBlock.updateMark = reused.target;
        }
        else {
          compileItem(item, index, vm);
        }
      });

      delete fragBlock.updateMark;
    }
  );

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
function bindShown (vm, target, fragBlock, meta) {
  var display = watchBlock(vm, fragBlock, target.shown, 'shown',
    function (display) {
      console.debug('[JS Framework] the "if" item was changed', display);

      if (!fragBlock || !!fragBlock.display === !!display) {
        return
      }
      fragBlock.display = !!display;
      if (display) {
        compile(vm, target, fragBlock, meta);
      }
      else {
        removeTarget(vm, fragBlock, true);
      }
    }
  );

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
function watchBlock (vm, fragBlock, calc, type, handler) {
  var differ = vm && vm._app && vm._app.differ;
  var config = {};
  var depth = (fragBlock.element.depth || 0) + 1;

  return watch(vm, calc, function (value) {
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
  })
}

/**
 * Clone a context and merge certain data.
 *
 * @param  {object} mergedData
 * @return {object}
 */
function mergeContext (context, mergedData) {
  var newContext = Object.create(context);
  newContext._data = mergedData;
  initData(newContext);
  initComputed(newContext);
  newContext._realParent = context;
  if (context._static) {
    newContext._static = context._static;
  }
  return newContext
}

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
function Evt (type, detail) {
  if (detail instanceof Evt) {
    return detail
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
    return shouldStop
  };
}

/**
 * Emit an event but not broadcast down or dispatch up.
 * @param  {string} type
 * @param  {any}    detail
 */
function $emit (type, detail) {
  var this$1 = this;

  var events = this._vmEvents;
  var handlerList = events[type];
  if (handlerList) {
    var evt = new Evt(type, detail);
    handlerList.forEach(function (handler) {
      handler.call(this$1, evt);
    });
  }
}

/**
 * Emit an event and dispatch it up.
 * @param  {string} type
 * @param  {any}    detail
 */
function $dispatch (type, detail) {
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
function $broadcast (type, detail) {
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
function $on (type, handler) {
  if (!type || typeof handler !== 'function') {
    return
  }
  var events = this._vmEvents;
  var handlerList = events[type] || [];
  handlerList.push(handler);
  events[type] = handlerList;

  // fixed old version lifecycle design
  /* istanbul ignore if */
  if (type === 'hook:ready' && this._ready) {
    this.$emit('hook:ready');
  }
}

/**
 * Remove event listener.
 * @param  {string}   type
 * @param  {function} handler
 */
function $off (type, handler) {
  if (!type) {
    return
  }
  var events = this._vmEvents;
  if (!handler) {
    delete events[type];
    return
  }
  var handlerList = events[type];
  if (!handlerList) {
    return
  }
  handlerList.$remove(handler);
}

var LIFE_CYCLE_TYPES = ['init', 'created', 'ready', 'destroyed'];

/**
 * Init events:
 * 1. listen `events` in component options & `externalEvents`.
 * 2. bind lifecycle hooks.
 * @param  {Vm}     vm
 * @param  {object} externalEvents
 */
function initEvents (vm, externalEvents) {
  var options = vm._options || {};
  var events = options.events || {};
  for (var type1 in events) {
    vm.$on(type1, events[type1]);
  }
  for (var type2 in externalEvents) {
    vm.$on(type2, externalEvents[type2]);
  }
  LIFE_CYCLE_TYPES.forEach(function (type) {
    vm.$on(("hook:" + type), options[type]);
  });
}

/**
 * Bind event related methods to ViewModel instance.
 * @param  {Vm} vm
 */
function mixinEvents (vm) {
  vm.$emit = $emit;
  vm.$dispatch = $dispatch;
  vm.$broadcast = $broadcast;
  vm.$on = $on;
  vm.$off = $off;
}

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
/**
 * @fileOverview
 * ViewModel Constructor & definition
 */

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
function Vm (
  type,
  options,
  parentVm,
  parentEl,
  mergedData,
  externalEvents
) {
  parentVm = parentVm || {};
  this._parent = parentVm._realParent ? parentVm._realParent : parentVm;
  this._app = parentVm._app || {};
  parentVm._childrenVms && parentVm._childrenVms.push(this);

  if (!options && this._app.customComponentMap) {
    options = this._app.customComponentMap[type];
  }
  options = options || {};

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
  initEvents(this, externalEvents);

  console.debug(("[JS Framework] \"init\" lifecycle in Vm(" + (this._type) + ")"));
  this.$emit('hook:init');
  this._inited = true;

  // proxy data and methods
  // observe data and add this to vms
  this._data = typeof data === 'function' ? data() : data;
  if (mergedData) {
    extend(this._data, mergedData);
  }
  initState(this);

  console.debug(("[JS Framework] \"created\" lifecycle in Vm(" + (this._type) + ")"));
  this.$emit('hook:created');
  this._created = true;

  // backward old ready entry
  if (options.methods && options.methods.ready) {
    console.warn('"exports.methods.ready" is deprecated, ' +
      'please use "exports.created" instead');
    options.methods.ready.call(this);
  }

  if (!this._app.doc) {
    return
  }

  // if no parentElement then specify the documentElement
  this._parentEl = parentEl || this._app.doc.documentElement;
  build(this);
}

mixinEvents(Vm.prototype);

/**
 * Watch an function and bind all the data appeared in it. When the related
 * data changes, the callback will be called with new value as 1st param.
 *
 * @param {Function} fn
 * @param {Function} callback
 */
Vm.prototype.$watch = function (fn, callback) {
  watch(this, fn, callback);
};

Vm.set = set$1;
Vm.delete = del;

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
var nativeModules = {};

// for testing

/**
 * for testing
 */


/**
 * for testing
 */


// for framework

/**
 * init modules for an app instance
 * the second param determines whether to replace an existed method
 */
function initModules (modules, ifReplace) {
  var loop = function ( moduleName ) {
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

  for (var moduleName in modules) loop( moduleName );
}

/**
 * init app methods
 */
function initMethods$1 (Vm, apis) {
  var p = Vm.prototype;

  for (var apiName in apis) {
    if (!p.hasOwnProperty(apiName)) {
      p[apiName] = apis[apiName];
    }
  }
}

/**
 * get a module of methods for an app instance
 */
function requireModule$1 (app, name) {
  var methods = nativeModules[name];
  var target = {};
  var loop = function ( methodName ) {
    Object.defineProperty(target, methodName, {
      configurable: true,
      enumerable: true,
      get: function moduleGetter () {
        return function () {
          var args = [], len = arguments.length;
          while ( len-- ) args[ len ] = arguments[ len ];

          return app.callTasks({
          module: name,
          method: methodName,
          args: args
        });
        }
      },
      set: function moduleSetter (value) {
        if (typeof value === 'function') {
          return app.callTasks({
            module: name,
            method: methodName,
            args: [value]
          })
        }
      }
    });
  };

  for (var methodName in methods) loop( methodName );
  return target
}

/**
 * get a custom component options
 */
function requireCustomComponent (app, name) {
  var customComponentMap = app.customComponentMap;
  return customComponentMap[name]
}

/**
 * register a custom component options
 */
function registerCustomComponent (app, name, def) {
  var customComponentMap = app.customComponentMap;

  if (customComponentMap[name]) {
    console.error(("[JS Framework] define a component(" + name + ") that already exists"));
    return
  }

  customComponentMap[name] = def;
}

var semver = createCommonjsModule(function (module, exports) {
exports = module.exports = SemVer;

// The debug function is excluded entirely from the minified version.
/* nomin */ var debug;
/* nomin */ if (typeof process === 'object' &&
    /* nomin */ process.env &&
    /* nomin */ false &&
    /* nomin */ /\bsemver\b/i.test(false))
  /* nomin */ { debug = function() {
    /* nomin */ var args = Array.prototype.slice.call(arguments, 0);
    /* nomin */ args.unshift('SEMVER');
    /* nomin */ console.log.apply(console, args);
    /* nomin */ }; }
/* nomin */ else
  /* nomin */ { debug = function() {}; }

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
src[MAINVERSION] = '(' + src[NUMERICIDENTIFIER] + ')\\.' +
                   '(' + src[NUMERICIDENTIFIER] + ')\\.' +
                   '(' + src[NUMERICIDENTIFIER] + ')';

var MAINVERSIONLOOSE = R++;
src[MAINVERSIONLOOSE] = '(' + src[NUMERICIDENTIFIERLOOSE] + ')\\.' +
                        '(' + src[NUMERICIDENTIFIERLOOSE] + ')\\.' +
                        '(' + src[NUMERICIDENTIFIERLOOSE] + ')';

// ## Pre-release Version Identifier
// A numeric identifier, or a non-numeric identifier.

var PRERELEASEIDENTIFIER = R++;
src[PRERELEASEIDENTIFIER] = '(?:' + src[NUMERICIDENTIFIER] +
                            '|' + src[NONNUMERICIDENTIFIER] + ')';

var PRERELEASEIDENTIFIERLOOSE = R++;
src[PRERELEASEIDENTIFIERLOOSE] = '(?:' + src[NUMERICIDENTIFIERLOOSE] +
                                 '|' + src[NONNUMERICIDENTIFIER] + ')';


// ## Pre-release Version
// Hyphen, followed by one or more dot-separated pre-release version
// identifiers.

var PRERELEASE = R++;
src[PRERELEASE] = '(?:-(' + src[PRERELEASEIDENTIFIER] +
                  '(?:\\.' + src[PRERELEASEIDENTIFIER] + ')*))';

var PRERELEASELOOSE = R++;
src[PRERELEASELOOSE] = '(?:-?(' + src[PRERELEASEIDENTIFIERLOOSE] +
                       '(?:\\.' + src[PRERELEASEIDENTIFIERLOOSE] + ')*))';

// ## Build Metadata Identifier
// Any combination of digits, letters, or hyphens.

var BUILDIDENTIFIER = R++;
src[BUILDIDENTIFIER] = '[0-9A-Za-z-]+';

// ## Build Metadata
// Plus sign, followed by one or more period-separated build metadata
// identifiers.

var BUILD = R++;
src[BUILD] = '(?:\\+(' + src[BUILDIDENTIFIER] +
             '(?:\\.' + src[BUILDIDENTIFIER] + ')*))';


// ## Full Version String
// A main version, followed optionally by a pre-release version and
// build metadata.

// Note that the only major, minor, patch, and pre-release sections of
// the version string are capturing groups.  The build metadata is not a
// capturing group, because it should not ever be used in version
// comparison.

var FULL = R++;
var FULLPLAIN = 'v?' + src[MAINVERSION] +
                src[PRERELEASE] + '?' +
                src[BUILD] + '?';

src[FULL] = '^' + FULLPLAIN + '$';

// like full, but allows v1.2.3 and =1.2.3, which people do sometimes.
// also, 1.0.0alpha1 (prerelease without the hyphen) which is pretty
// common in the npm registry.
var LOOSEPLAIN = '[v=\\s]*' + src[MAINVERSIONLOOSE] +
                 src[PRERELEASELOOSE] + '?' +
                 src[BUILD] + '?';

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
src[XRANGEPLAIN] = '[v=\\s]*(' + src[XRANGEIDENTIFIER] + ')' +
                   '(?:\\.(' + src[XRANGEIDENTIFIER] + ')' +
                   '(?:\\.(' + src[XRANGEIDENTIFIER] + ')' +
                   '(?:' + src[PRERELEASE] + ')?' +
                   src[BUILD] + '?' +
                   ')?)?';

var XRANGEPLAINLOOSE = R++;
src[XRANGEPLAINLOOSE] = '[v=\\s]*(' + src[XRANGEIDENTIFIERLOOSE] + ')' +
                        '(?:\\.(' + src[XRANGEIDENTIFIERLOOSE] + ')' +
                        '(?:\\.(' + src[XRANGEIDENTIFIERLOOSE] + ')' +
                        '(?:' + src[PRERELEASELOOSE] + ')?' +
                        src[BUILD] + '?' +
                        ')?)?';

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
src[COMPARATORTRIM] = '(\\s*)' + src[GTLT] +
                      '\\s*(' + LOOSEPLAIN + '|' + src[XRANGEPLAIN] + ')';

// this one has to use the /g flag
re[COMPARATORTRIM] = new RegExp(src[COMPARATORTRIM], 'g');
var comparatorTrimReplace = '$1$2$3';


// Something like `1.2.3 - 1.2.4`
// Note that these all use the loose form, because they'll be
// checked against either the strict or loose comparator form
// later.
var HYPHENRANGE = R++;
src[HYPHENRANGE] = '^\\s*(' + src[XRANGEPLAIN] + ')' +
                   '\\s+-\\s+' +
                   '(' + src[XRANGEPLAIN] + ')' +
                   '\\s*$';

var HYPHENRANGELOOSE = R++;
src[HYPHENRANGELOOSE] = '^\\s*(' + src[XRANGEPLAINLOOSE] + ')' +
                        '\\s+-\\s+' +
                        '(' + src[XRANGEPLAINLOOSE] + ')' +
                        '\\s*$';

// Star ranges basically just allow anything at all.
var STAR = R++;
src[STAR] = '(<|>)?=?\\s*\\*';

// Compile to actual regexp objects.
// All are flag-free, unless they were created above with a flag.
for (var i = 0; i < R; i++) {
  debug(i, src[i]);
  if (!re[i])
    { re[i] = new RegExp(src[i]); }
}

exports.parse = parse;
function parse(version, loose) {
  if (version instanceof SemVer)
    { return version; }

  if (typeof version !== 'string')
    { return null; }

  if (version.length > MAX_LENGTH)
    { return null; }

  var r = loose ? re[LOOSE] : re[FULL];
  if (!r.test(version))
    { return null; }

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
    if (version.loose === loose)
      { return version; }
    else
      { version = version.version; }
  } else if (typeof version !== 'string') {
    throw new TypeError('Invalid Version: ' + version);
  }

  if (version.length > MAX_LENGTH)
    { throw new TypeError('version is longer than ' + MAX_LENGTH + ' characters') }

  if (!(this instanceof SemVer))
    { return new SemVer(version, loose); }

  debug('SemVer', version, loose);
  this.loose = loose;
  var m = version.trim().match(loose ? re[LOOSE] : re[FULL]);

  if (!m)
    { throw new TypeError('Invalid Version: ' + version); }

  this.raw = version;

  // these are actually numbers
  this.major = +m[1];
  this.minor = +m[2];
  this.patch = +m[3];

  if (this.major > MAX_SAFE_INTEGER || this.major < 0)
    { throw new TypeError('Invalid major version') }

  if (this.minor > MAX_SAFE_INTEGER || this.minor < 0)
    { throw new TypeError('Invalid minor version') }

  if (this.patch > MAX_SAFE_INTEGER || this.patch < 0)
    { throw new TypeError('Invalid patch version') }

  // numberify any prerelease numeric ids
  if (!m[4])
    { this.prerelease = []; }
  else
    { this.prerelease = m[4].split('.').map(function(id) {
      if (/^[0-9]+$/.test(id)) {
        var num = +id;
        if (num >= 0 && num < MAX_SAFE_INTEGER)
          { return num; }
      }
      return id;
    }); }

  this.build = m[5] ? m[5].split('.') : [];
  this.format();
}

SemVer.prototype.format = function() {
  this.version = this.major + '.' + this.minor + '.' + this.patch;
  if (this.prerelease.length)
    { this.version += '-' + this.prerelease.join('.'); }
  return this.version;
};

SemVer.prototype.toString = function() {
  return this.version;
};

SemVer.prototype.compare = function(other) {
  debug('SemVer.compare', this.version, this.loose, other);
  if (!(other instanceof SemVer))
    { other = new SemVer(other, this.loose); }

  return this.compareMain(other) || this.comparePre(other);
};

SemVer.prototype.compareMain = function(other) {
  if (!(other instanceof SemVer))
    { other = new SemVer(other, this.loose); }

  return compareIdentifiers(this.major, other.major) ||
         compareIdentifiers(this.minor, other.minor) ||
         compareIdentifiers(this.patch, other.patch);
};

SemVer.prototype.comparePre = function(other) {
  var this$1 = this;

  if (!(other instanceof SemVer))
    { other = new SemVer(other, this.loose); }

  // NOT having a prerelease is > having one
  if (this.prerelease.length && !other.prerelease.length)
    { return -1; }
  else if (!this.prerelease.length && other.prerelease.length)
    { return 1; }
  else if (!this.prerelease.length && !other.prerelease.length)
    { return 0; }

  var i = 0;
  do {
    var a = this$1.prerelease[i];
    var b = other.prerelease[i];
    debug('prerelease compare', i, a, b);
    if (a === undefined && b === undefined)
      { return 0; }
    else if (b === undefined)
      { return 1; }
    else if (a === undefined)
      { return -1; }
    else if (a === b)
      { continue; }
    else
      { return compareIdentifiers(a, b); }
  } while (++i);
};

// preminor will bump the version up to the next minor release, and immediately
// down to pre-release. premajor and prepatch work the same way.
SemVer.prototype.inc = function(release, identifier) {
  var this$1 = this;

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
      if (this.prerelease.length === 0)
        { this.inc('patch', identifier); }
      this.inc('pre', identifier);
      break;

    case 'major':
      // If this is a pre-major version, bump up to the same major version.
      // Otherwise increment major.
      // 1.0.0-5 bumps to 1.0.0
      // 1.1.0 bumps to 2.0.0
      if (this.minor !== 0 || this.patch !== 0 || this.prerelease.length === 0)
        { this.major++; }
      this.minor = 0;
      this.patch = 0;
      this.prerelease = [];
      break;
    case 'minor':
      // If this is a pre-minor version, bump up to the same minor version.
      // Otherwise increment minor.
      // 1.2.0-5 bumps to 1.2.0
      // 1.2.1 bumps to 1.3.0
      if (this.patch !== 0 || this.prerelease.length === 0)
        { this.minor++; }
      this.patch = 0;
      this.prerelease = [];
      break;
    case 'patch':
      // If this is not a pre-release version, it will increment the patch.
      // If it is a pre-release it will bump up to the same patch version.
      // 1.2.0-5 patches to 1.2.0
      // 1.2.0 patches to 1.2.1
      if (this.prerelease.length === 0)
        { this.patch++; }
      this.prerelease = [];
      break;
    // This probably shouldn't be used publicly.
    // 1.0.0 "pre" would become 1.0.0-0 which is the wrong direction.
    case 'pre':
      if (this.prerelease.length === 0)
        { this.prerelease = [0]; }
      else {
        var i = this.prerelease.length;
        while (--i >= 0) {
          if (typeof this$1.prerelease[i] === 'number') {
            this$1.prerelease[i]++;
            i = -2;
          }
        }
        if (i === -1) // didn't increment anything
          { this.prerelease.push(0); }
      }
      if (identifier) {
        // 1.2.0-beta.1 bumps to 1.2.0-beta.2,
        // 1.2.0-beta.fooblz or 1.2.0-beta bumps to 1.2.0-beta.0
        if (this.prerelease[0] === identifier) {
          if (isNaN(this.prerelease[1]))
            { this.prerelease = [identifier, 0]; }
        } else
          { this.prerelease = [identifier, 0]; }
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
  if (typeof(loose) === 'string') {
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
            return 'pre'+key;
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

  return (anum && !bnum) ? -1 :
         (bnum && !anum) ? 1 :
         a < b ? -1 :
         a > b ? 1 :
         0;
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
  return new SemVer(a, loose).compare(new SemVer(b, loose));
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
  return list.sort(function(a, b) {
    return exports.compare(a, b, loose);
  });
}

exports.rsort = rsort;
function rsort(list, loose) {
  return list.sort(function(a, b) {
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
      if (typeof a === 'object') { a = a.version; }
      if (typeof b === 'object') { b = b.version; }
      ret = a === b;
      break;
    case '!==':
      if (typeof a === 'object') { a = a.version; }
      if (typeof b === 'object') { b = b.version; }
      ret = a !== b;
      break;
    case '': case '=': case '==': ret = eq(a, b, loose); break;
    case '!=': ret = neq(a, b, loose); break;
    case '>': ret = gt(a, b, loose); break;
    case '>=': ret = gte(a, b, loose); break;
    case '<': ret = lt(a, b, loose); break;
    case '<=': ret = lte(a, b, loose); break;
    default: throw new TypeError('Invalid operator: ' + op);
  }
  return ret;
}

exports.Comparator = Comparator;
function Comparator(comp, loose) {
  if (comp instanceof Comparator) {
    if (comp.loose === loose)
      { return comp; }
    else
      { comp = comp.value; }
  }

  if (!(this instanceof Comparator))
    { return new Comparator(comp, loose); }

  debug('comparator', comp, loose);
  this.loose = loose;
  this.parse(comp);

  if (this.semver === ANY)
    { this.value = ''; }
  else
    { this.value = this.operator + this.semver.version; }

  debug('comp', this);
}

var ANY = {};
Comparator.prototype.parse = function(comp) {
  var r = this.loose ? re[COMPARATORLOOSE] : re[COMPARATOR];
  var m = comp.match(r);

  if (!m)
    { throw new TypeError('Invalid comparator: ' + comp); }

  this.operator = m[1];
  if (this.operator === '=')
    { this.operator = ''; }

  // if it literally is just '>' or '' then allow anything.
  if (!m[2])
    { this.semver = ANY; }
  else
    { this.semver = new SemVer(m[2], this.loose); }
};

Comparator.prototype.toString = function() {
  return this.value;
};

Comparator.prototype.test = function(version) {
  debug('Comparator.test', version, this.loose);

  if (this.semver === ANY)
    { return true; }

  if (typeof version === 'string')
    { version = new SemVer(version, this.loose); }

  return cmp(version, this.operator, this.semver, this.loose);
};

Comparator.prototype.intersects = function(comp, loose) {
  if (!(comp instanceof Comparator)) {
    throw new TypeError('a Comparator is required');
  }

  var rangeTmp;

  if (this.operator === '') {
    rangeTmp = new Range(comp.value, loose);
    return satisfies(this.value, rangeTmp, loose);
  } else if (comp.operator === '') {
    rangeTmp = new Range(this.value, loose);
    return satisfies(comp.semver, rangeTmp, loose);
  }

  var sameDirectionIncreasing =
    (this.operator === '>=' || this.operator === '>') &&
    (comp.operator === '>=' || comp.operator === '>');
  var sameDirectionDecreasing =
    (this.operator === '<=' || this.operator === '<') &&
    (comp.operator === '<=' || comp.operator === '<');
  var sameSemVer = this.semver.version === comp.semver.version;
  var differentDirectionsInclusive =
    (this.operator === '>=' || this.operator === '<=') &&
    (comp.operator === '>=' || comp.operator === '<=');
  var oppositeDirectionsLessThan =
    cmp(this.semver, '<', comp.semver, loose) &&
    ((this.operator === '>=' || this.operator === '>') &&
    (comp.operator === '<=' || comp.operator === '<'));
  var oppositeDirectionsGreaterThan =
    cmp(this.semver, '>', comp.semver, loose) &&
    ((this.operator === '<=' || this.operator === '<') &&
    (comp.operator === '>=' || comp.operator === '>'));

  return sameDirectionIncreasing || sameDirectionDecreasing ||
    (sameSemVer && differentDirectionsInclusive) ||
    oppositeDirectionsLessThan || oppositeDirectionsGreaterThan;
};


exports.Range = Range;
function Range(range, loose) {
  if (range instanceof Range) {
    if (range.loose === loose) {
      return range;
    } else {
      return new Range(range.raw, loose);
    }
  }

  if (range instanceof Comparator) {
    return new Range(range.value, loose);
  }

  if (!(this instanceof Range))
    { return new Range(range, loose); }

  this.loose = loose;

  // First, split based on boolean or ||
  this.raw = range;
  this.set = range.split(/\s*\|\|\s*/).map(function(range) {
    return this.parseRange(range.trim());
  }, this).filter(function(c) {
    // throw out any that are not relevant for whatever reason
    return c.length;
  });

  if (!this.set.length) {
    throw new TypeError('Invalid SemVer Range: ' + range);
  }

  this.format();
}

Range.prototype.format = function() {
  this.range = this.set.map(function(comps) {
    return comps.join(' ').trim();
  }).join('||').trim();
  return this.range;
};

Range.prototype.toString = function() {
  return this.range;
};

Range.prototype.parseRange = function(range) {
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
  var set = range.split(' ').map(function(comp) {
    return parseComparator(comp, loose);
  }).join(' ').split(/\s+/);
  if (this.loose) {
    // in loose mode, throw out any that are not valid comparators
    set = set.filter(function(comp) {
      return !!comp.match(compRe);
    });
  }
  set = set.map(function(comp) {
    return new Comparator(comp, loose);
  });

  return set;
};

Range.prototype.intersects = function(range, loose) {
  if (!(range instanceof Range)) {
    throw new TypeError('a Range is required');
  }

  return this.set.some(function(thisComparators) {
    return thisComparators.every(function(thisComparator) {
      return range.set.some(function(rangeComparators) {
        return rangeComparators.every(function(rangeComparator) {
          return thisComparator.intersects(rangeComparator, loose);
        });
      });
    });
  });
};

// Mostly just for testing and legacy API reasons
exports.toComparators = toComparators;
function toComparators(range, loose) {
  return new Range(range, loose).set.map(function(comp) {
    return comp.map(function(c) {
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
  return comp.trim().split(/\s+/).map(function(comp) {
    return replaceTilde(comp, loose);
  }).join(' ');
}

function replaceTilde(comp, loose) {
  var r = loose ? re[TILDELOOSE] : re[TILDE];
  return comp.replace(r, function(_, M, m, p, pr) {
    debug('tilde', comp, _, M, m, p, pr);
    var ret;

    if (isX(M))
      { ret = ''; }
    else if (isX(m))
      { ret = '>=' + M + '.0.0 <' + (+M + 1) + '.0.0'; }
    else if (isX(p))
      // ~1.2 == >=1.2.0 <1.3.0
      { ret = '>=' + M + '.' + m + '.0 <' + M + '.' + (+m + 1) + '.0'; }
    else if (pr) {
      debug('replaceTilde pr', pr);
      if (pr.charAt(0) !== '-')
        { pr = '-' + pr; }
      ret = '>=' + M + '.' + m + '.' + p + pr +
            ' <' + M + '.' + (+m + 1) + '.0';
    } else
      // ~1.2.3 == >=1.2.3 <1.3.0
      { ret = '>=' + M + '.' + m + '.' + p +
            ' <' + M + '.' + (+m + 1) + '.0'; }

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
  return comp.trim().split(/\s+/).map(function(comp) {
    return replaceCaret(comp, loose);
  }).join(' ');
}

function replaceCaret(comp, loose) {
  debug('caret', comp, loose);
  var r = loose ? re[CARETLOOSE] : re[CARET];
  return comp.replace(r, function(_, M, m, p, pr) {
    debug('caret', comp, _, M, m, p, pr);
    var ret;

    if (isX(M))
      { ret = ''; }
    else if (isX(m))
      { ret = '>=' + M + '.0.0 <' + (+M + 1) + '.0.0'; }
    else if (isX(p)) {
      if (M === '0')
        { ret = '>=' + M + '.' + m + '.0 <' + M + '.' + (+m + 1) + '.0'; }
      else
        { ret = '>=' + M + '.' + m + '.0 <' + (+M + 1) + '.0.0'; }
    } else if (pr) {
      debug('replaceCaret pr', pr);
      if (pr.charAt(0) !== '-')
        { pr = '-' + pr; }
      if (M === '0') {
        if (m === '0')
          { ret = '>=' + M + '.' + m + '.' + p + pr +
                ' <' + M + '.' + m + '.' + (+p + 1); }
        else
          { ret = '>=' + M + '.' + m + '.' + p + pr +
                ' <' + M + '.' + (+m + 1) + '.0'; }
      } else
        { ret = '>=' + M + '.' + m + '.' + p + pr +
              ' <' + (+M + 1) + '.0.0'; }
    } else {
      debug('no pr');
      if (M === '0') {
        if (m === '0')
          { ret = '>=' + M + '.' + m + '.' + p +
                ' <' + M + '.' + m + '.' + (+p + 1); }
        else
          { ret = '>=' + M + '.' + m + '.' + p +
                ' <' + M + '.' + (+m + 1) + '.0'; }
      } else
        { ret = '>=' + M + '.' + m + '.' + p +
              ' <' + (+M + 1) + '.0.0'; }
    }

    debug('caret return', ret);
    return ret;
  });
}

function replaceXRanges(comp, loose) {
  debug('replaceXRanges', comp, loose);
  return comp.split(/\s+/).map(function(comp) {
    return replaceXRange(comp, loose);
  }).join(' ');
}

function replaceXRange(comp, loose) {
  comp = comp.trim();
  var r = loose ? re[XRANGELOOSE] : re[XRANGE];
  return comp.replace(r, function(ret, gtlt, M, m, p, pr) {
    debug('xRange', comp, ret, gtlt, M, m, p, pr);
    var xM = isX(M);
    var xm = xM || isX(m);
    var xp = xm || isX(p);
    var anyX = xp;

    if (gtlt === '=' && anyX)
      { gtlt = ''; }

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
      if (xm)
        { m = 0; }
      if (xp)
        { p = 0; }

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
        if (xm)
          { M = +M + 1; }
        else
          { m = +m + 1; }
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
function hyphenReplace($0,
                       from, fM, fm, fp, fpr, fb,
                       to, tM, tm, tp, tpr, tb) {

  if (isX(fM))
    { from = ''; }
  else if (isX(fm))
    { from = '>=' + fM + '.0.0'; }
  else if (isX(fp))
    { from = '>=' + fM + '.' + fm + '.0'; }
  else
    { from = '>=' + from; }

  if (isX(tM))
    { to = ''; }
  else if (isX(tm))
    { to = '<' + (+tM + 1) + '.0.0'; }
  else if (isX(tp))
    { to = '<' + tM + '.' + (+tm + 1) + '.0'; }
  else if (tpr)
    { to = '<=' + tM + '.' + tm + '.' + tp + '-' + tpr; }
  else
    { to = '<=' + to; }

  return (from + ' ' + to).trim();
}


// if ANY of the sets match ALL of its comparators, then pass
Range.prototype.test = function(version) {
  var this$1 = this;

  if (!version)
    { return false; }

  if (typeof version === 'string')
    { version = new SemVer(version, this.loose); }

  for (var i = 0; i < this.set.length; i++) {
    if (testSet(this$1.set[i], version))
      { return true; }
  }
  return false;
};

function testSet(set, version) {
  for (var i = 0; i < set.length; i++) {
    if (!set[i].test(version))
      { return false; }
  }

  if (version.prerelease.length) {
    // Find the set of versions that are allowed to have prereleases
    // For example, ^1.2.3-pr.1 desugars to >=1.2.3-pr.1 <2.0.0
    // That should allow `1.2.3-pr.2` to pass.
    // However, `1.2.4-alpha.notready` should NOT be allowed,
    // even though it's within the range set by the comparators.
    for (var i = 0; i < set.length; i++) {
      debug(set[i].semver);
      if (set[i].semver === ANY)
        { continue; }

      if (set[i].semver.prerelease.length > 0) {
        var allowed = set[i].semver;
        if (allowed.major === version.major &&
            allowed.minor === version.minor &&
            allowed.patch === version.patch)
          { return true; }
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
  var max = null;
  var maxSV = null;
  try {
    var rangeObj = new Range(range, loose);
  } catch (er) {
    return null;
  }
  versions.forEach(function (v) {
    if (rangeObj.test(v)) { // satisfies(v, range, loose)
      if (!max || maxSV.compare(v) === -1) { // compare(max, v, true)
        max = v;
        maxSV = new SemVer(max, loose);
      }
    }
  });
  return max;
}

exports.minSatisfying = minSatisfying;
function minSatisfying(versions, range, loose) {
  var min = null;
  var minSV = null;
  try {
    var rangeObj = new Range(range, loose);
  } catch (er) {
    return null;
  }
  versions.forEach(function (v) {
    if (rangeObj.test(v)) { // satisfies(v, range, loose)
      if (!min || minSV.compare(v) === 1) { // compare(min, v, true)
        min = v;
        minSV = new SemVer(min, loose);
      }
    }
  });
  return min;
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

    comparators.forEach(function(comparator) {
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
    if ((!low.operator || low.operator === comp) &&
        ltefn(version, low.semver)) {
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
  return (parsed && parsed.prerelease.length) ? parsed.prerelease : null;
}

exports.intersects = intersects;
function intersects(r1, r2, loose) {
  r1 = new Range(r1, loose);
  r2 = new Range(r2, loose);
  return r1.intersects(r2)
}
});

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
/**
 * Normalize a version string.
 * @param  {String} Version. ie: 1, 1.0, 1.0.0
 * @return {String} Version
 */
function normalizeVersion (v) {
  var isValid = semver.valid(v);
  if (isValid) {
    return v
  }

  v = typeof (v) === 'string' ? v : '';
  var split = v.split('.');
  var i = 0;
  var result = [];

  while (i < 3) {
    var s = typeof (split[i]) === 'string' && split[i] ? split[i] : '0';
    result.push(s);
    i++;
  }

  return result.join('.')
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
function getError (key, val, criteria) {
  var result = {
    isDowngrade: true,
    errorType: 1,
    code: 1000
  };
  var getMsg = function (key, val, criteria) {
    return 'Downgrade[' + key + '] :: deviceInfo '
      + val + ' matched criteria ' + criteria
  };
  var _key = key.toLowerCase();

  result.errorMessage = getMsg(key, val, criteria);

  if (_key.indexOf('osversion') >= 0) {
    result.code = 1001;
  }
  else if (_key.indexOf('appversion') >= 0) {
    result.code = 1002;
  }
  else if (_key.indexOf('weexversion') >= 0) {
    result.code = 1003;
  }
  else if (_key.indexOf('devicemodel') >= 0) {
    result.code = 1004;
  }

  return result
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
function check (config, deviceInfo) {
  deviceInfo = deviceInfo || global.WXEnvironment;
  deviceInfo = isPlainObject(deviceInfo) ? deviceInfo : {};

  var result = {
    isDowngrade: false // defautl is pass
  };

  if (typof$1(config) === 'function') {
    var customDowngrade = config.call(this, deviceInfo, {
      semver: semver,
      normalizeVersion: normalizeVersion
    });

    customDowngrade = !!customDowngrade;

    result = customDowngrade ? getError('custom', '', 'custom params') : result;
  }
  else {
    config = isPlainObject(config) ? config : {};

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

        if (semver.satisfies(d, c)) {
          result = getError(key, val, criteria);
          break
        }
      }
      else if (isDeviceModel) {
        var _criteria = typof$1(criteria) === 'array' ? criteria : [criteria];
        if (_criteria.indexOf(val) >= 0) {
          result = getError(key, val, criteria);
          break
        }
      }
    }
  }

  return result
}

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
function setViewport (app, configs) {
  if ( configs === void 0 ) configs = {};

  /* istanbul ignore if */
  {
    console.debug(("[JS Framework] Set viewport (width: " + (configs.width) + ") for app#" + (app.id) + "."));
    validateViewport(configs);
  }

  // Send viewport configs to native
  if (app && app.callTasks) {
    return app.callTasks([{
      module: 'meta',
      method: 'setViewport',
      args: [configs]
    }])
  }

  /* istanbul ignore next */
  else {
    console.warn("[JS Framework] Can't find \"callTasks\" method on current app.");
  }
}

/**
 * Validate the viewport config.
 * @param {Object} configs
 */
function validateViewport (configs) {
  if ( configs === void 0 ) configs = {};

  var width = configs.width;
  if (width) {
    if (typeof width !== 'number' && width !== 'device-width') {
      console.warn(("[JS Framework] Not support to use " + width + " as viewport width."));
      return false
    }
    return true
  }
  console.warn('[JS Framework] the viewport config should contain the "width" property.');
  return false
}

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
/**
 * bootstrap app from a certain custom component with config & data
 */
function bootstrap (app, name, config, data) {
  console.debug(("[JS Framework] bootstrap for " + name));

  // 1. validate custom component name first
  var cleanName;
  if (isWeexComponent(name)) {
    cleanName = removeWeexPrefix(name);
  }
  else if (isNpmModule(name)) {
    cleanName = removeJSSurfix(name);
    // check if define by old 'define' method
    /* istanbul ignore if */
    if (!requireCustomComponent(app, cleanName)) {
      return new Error(("It's not a component: " + name))
    }
  }
  else {
    return new Error(("Wrong component name: " + name))
  }

  // 2. validate configuration
  config = isPlainObject(config) ? config : {};
  // 2.1 transformer version check
  if (typeof config.transformerVersion === 'string' &&
    typeof global.transformerVersion === 'string' &&
    !semver.satisfies(config.transformerVersion,
      global.transformerVersion)) {
    return new Error("JS Bundle version: " + (config.transformerVersion) + " " +
      "not compatible with " + (global.transformerVersion))
  }
  // 2.2 downgrade version check
  var downgradeResult = check(config.downgrade);
  /* istanbul ignore if */
  if (downgradeResult.isDowngrade) {
    app.callTasks([{
      module: 'instanceWrap',
      method: 'error',
      args: [
        downgradeResult.errorType,
        downgradeResult.code,
        downgradeResult.errorMessage
      ]
    }]);
    return new Error(("Downgrade[" + (downgradeResult.code) + "]: " + (downgradeResult.errorMessage)))
  }

  // set viewport
  if (config.viewport) {
    setViewport(app, config.viewport);
  }

  // 3. create a new Vm with custom component name and data
  app.vm = new Vm(cleanName, null, { _app: app }, null, data);
}

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
/**
 * define(name, factory) for primary usage
 * or
 * define(name, deps, factory) for compatibility
 * Notice: DO NOT use function define() {},
 * it will cause error after builded by webpack
 */
var defineFn = function (app, name) {
  var args = [], len = arguments.length - 2;
  while ( len-- > 0 ) args[ len ] = arguments[ len + 2 ];

  console.debug(("[JS Framework] define a component " + name));

  // adapt args:
  // 1. name, deps[], factory()
  // 2. name, factory()
  // 3. name, definition{}
  var factory, definition;
  if (args.length > 1) {
    definition = args[1];
  }
  else {
    definition = args[0];
  }
  if (typeof definition === 'function') {
    factory = definition;
    definition = null;
  }

  // resolve definition from factory
  if (factory) {
    var r = function (name) {
      if (isWeexComponent(name)) {
        var cleanName = removeWeexPrefix(name);
        return requireCustomComponent(app, cleanName)
      }
      if (isWeexModule(name)) {
        var cleanName$1 = removeWeexPrefix(name);
        return app.requireModule(cleanName$1)
      }
      if (isNormalModule(name) || isNpmModule(name)) {
        var cleanName$2 = removeJSSurfix(name);
        return app.commonModules[cleanName$2]
      }
    };
    var m = { exports: {}};
    factory(r, m.exports, m);
    definition = m.exports;
  }

  // apply definition
  if (isWeexComponent(name)) {
    var cleanName = removeWeexPrefix(name);
    registerCustomComponent(app, cleanName, definition);
  }
  else if (isWeexModule(name)) {
    var cleanName$1 = removeWeexPrefix(name);
    var obj;
    initModules(( obj = {}, obj[cleanName$1] = definition, obj ));
  }
  else if (isNormalModule(name)) {
    var cleanName$2 = removeJSSurfix(name);
    app.commonModules[cleanName$2] = definition;
  }
  else if (isNpmModule(name)) {
    var cleanName$3 = removeJSSurfix(name);
    if (definition.template ||
        definition.style ||
        definition.methods) {
      // downgrade to old define method (define('componentName', factory))
      // the exports contain one key of template, style or methods
      // but it has risk!!!
      registerCustomComponent(app, cleanName$3, definition);
    }
    else {
      app.commonModules[cleanName$3] = definition;
    }
  }
};

/**
 * @deprecated
 */
function register$1 (app, type, options) {
  console.warn('[JS Framework] Register is deprecated, please install lastest transformer.');
  registerCustomComponent(app, type, options);
}

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
/**
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
/**
 * @fileOverview
 * instance controls from native
 *
 * - fire event
 * - callback
 * - refresh
 * - destroy
 *
 * corresponded with the API of instance manager (framework.js)
 */
/**
 * Refresh an app with data to its root component options.
 * @param  {object} app
 * @param  {any}    data
 */
function refresh (app, data) {
  console.debug("[JS Framework] Refresh with", data,
            ("in instance[" + (app.id) + "]"));
  var vm = app.vm;
  if (vm && data) {
    if (typeof vm.refreshData === 'function') {
      vm.refreshData(data);
    }
    else {
      extend(vm, data);
    }
    app.differ.flush();
    app.doc.taskCenter.send('dom', { action: 'refreshFinish' }, []);
    return
  }
  return new Error(("invalid data \"" + data + "\""))
}

/**
 * Destroy an app.
 * @param  {object} app
 */
function destroy$1 (app) {
  console.debug(("[JS Framework] Destory an instance(" + (app.id) + ")"));

  if (app.vm) {
    destroyVm(app.vm);
  }

  app.id = '';
  app.options = null;
  app.blocks = null;
  app.vm = null;
  app.doc.taskCenter.destroyCallback();
  app.doc.destroy();
  app.doc = null;
  app.customComponentMap = null;
  app.commonModules = null;
}

/**
 * Destroy an Vm.
 * @param {object} vm
 */
function destroyVm (vm) {
  delete vm._app;
  delete vm._computed;
  delete vm._css;
  delete vm._data;
  delete vm._ids;
  delete vm._methods;
  delete vm._options;
  delete vm._parent;
  delete vm._parentEl;
  delete vm._rootEl;

  // remove all watchers
  if (vm._watchers) {
    var watcherCount = vm._watchers.length;
    while (watcherCount--) {
      vm._watchers[watcherCount].teardown();
    }
    delete vm._watchers;
  }

  // destroy child vms recursively
  if (vm._childrenVms) {
    var vmCount = vm._childrenVms.length;
    while (vmCount--) {
      destroyVm(vm._childrenVms[vmCount]);
    }
    delete vm._childrenVms;
  }

  console.debug(("[JS Framework] \"destroyed\" lifecycle in Vm(" + (vm._type) + ")"));
  vm.$emit('hook:destroyed');

  delete vm._type;
  delete vm._vmEvents;
}

/**
 * Get a JSON object to describe the document body.
 * @param  {object} app
 * @return {object}
 */
function getRootElement (app) {
  var doc = app.doc || {};
  var body = doc.body || {};
  return body.toJSON ? body.toJSON() : {}
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
function fireEvent$1 (app, ref, type, e, domChanges) {
  console.debug(("[JS Framework] Fire a \"" + type + "\" event on an element(" + ref + ") in instance(" + (app.id) + ")"));
  if (Array.isArray(ref)) {
    ref.some(function (ref) {
      return fireEvent$1(app, ref, type, e) !== false
    });
    return
  }
  var el = app.doc.getRef(ref);
  if (el) {
    var result = app.doc.fireEvent(el, type, e, domChanges);
    app.differ.flush();
    app.doc.taskCenter.send('dom', { action: 'updateFinish' }, []);
    return result
  }
  return new Error(("invalid element reference \"" + ref + "\""))
}

/**
 * Make a callback for a certain app.
 * @param  {object}   app
 * @param  {number}   callbackId
 * @param  {any}      data
 * @param  {boolean}  ifKeepAlive
 */
function callback$1 (app, callbackId, data, ifKeepAlive) {
  console.debug(("[JS Framework] Invoke a callback(" + callbackId + ") with"), data,
            ("in instance(" + (app.id) + ")"));
  var result = app.doc.taskCenter.callback(callbackId, data, ifKeepAlive);
  updateActions(app);
  app.doc.taskCenter.send('dom', { action: 'updateFinish' }, []);
  return result
}

/**
 * Collect all virtual-DOM mutations together and send them to renderer.
 * @param  {object} app
 */
function updateActions (app) {
  app.differ.flush();
}

/**
 * Call all tasks from an app to renderer (native).
 * @param  {object} app
 * @param  {array}  tasks
 */
function callTasks (app, tasks) {
  var result;

  /* istanbul ignore next */
  if (typof$1(tasks) !== 'array') {
    tasks = [tasks];
  }

  tasks.forEach(function (task) {
    result = app.doc.taskCenter.send(
      'module',
      {
        module: task.module,
        method: task.method
      },
      task.args
    );
  });

  return result
}

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
/**
 * @fileOverview
 * instance controls from native
 *
 * - init bundle
 *
 * corresponded with the API of instance manager (framework.js)
 */

/**
 * Init an app by run code witgh data
 * @param  {object} app
 * @param  {string} code
 * @param  {object} data
 */
function init$5 (app, code, data, services) {
  console.debug('[JS Framework] Intialize an instance with:\n', data);
  var result;

  // prepare app env methods
  var bundleDefine = function () {
    var args = [], len = arguments.length;
    while ( len-- ) args[ len ] = arguments[ len ];

    return defineFn.apply(void 0, [ app ].concat( args ));
  };
  var bundleBootstrap = function (name, config, _data) {
    result = bootstrap(app, name, config, _data || data);
    updateActions(app);
    app.doc.listener.createFinish();
    console.debug(("[JS Framework] After intialized an instance(" + (app.id) + ")"));
  };
  var bundleVm = Vm;
  /* istanbul ignore next */
  var bundleRegister = function () {
    var args = [], len = arguments.length;
    while ( len-- ) args[ len ] = arguments[ len ];

    return register$1.apply(void 0, [ app ].concat( args ));
  };
  /* istanbul ignore next */
  var bundleRender = function (name, _data) {
    result = bootstrap(app, name, {}, _data);
  };
  /* istanbul ignore next */
  var bundleRequire = function (name) { return function (_data) {
    result = bootstrap(app, name, {}, _data);
  }; };
  var bundleDocument = app.doc;
  /* istanbul ignore next */
  var bundleRequireModule = function (name) { return app.requireModule(removeWeexPrefix(name)); };

  var weexGlobalObject = {
    config: app.options,
    define: bundleDefine,
    bootstrap: bundleBootstrap,
    requireModule: bundleRequireModule,
    document: bundleDocument,
    Vm: bundleVm
  };

  Object.freeze(weexGlobalObject);

  // prepare code
  var functionBody;
  /* istanbul ignore if */
  if (typeof code === 'function') {
    // `function () {...}` -> `{...}`
    // not very strict
    functionBody = code.toString().substr(12);
  }
  /* istanbul ignore next */
  else if (code) {
    functionBody = code.toString();
  }
  // wrap IFFE and use strict mode
  functionBody = "(function(global){\n\n\"use strict\";\n\n " + functionBody + " \n\n})(Object.create(this))";

  // run code and get result
  var WXEnvironment = global.WXEnvironment;
  var timerAPIs = {};

  /* istanbul ignore if */
  if (WXEnvironment && WXEnvironment.platform !== 'Web') {
    // timer APIs polyfill in native
    var timer = app.requireModule('timer');
    Object.assign(timerAPIs, {
      setTimeout: function () {
        var args = [], len = arguments.length;
        while ( len-- ) args[ len ] = arguments[ len ];

        var handler = function () {
          args[0].apply(args, args.slice(2));
        };
        timer.setTimeout(handler, args[1]);
        return app.doc.taskCenter.callbackManager.lastCallbackId.toString()
      },
      setInterval: function () {
        var args = [], len = arguments.length;
        while ( len-- ) args[ len ] = arguments[ len ];

        var handler = function () {
          args[0].apply(args, args.slice(2));
        };
        timer.setInterval(handler, args[1]);
        return app.doc.taskCenter.callbackManager.lastCallbackId.toString()
      },
      clearTimeout: function (n) {
        timer.clearTimeout(n);
      },
      clearInterval: function (n) {
        timer.clearInterval(n);
      }
    });
  }
  // run code and get result
  var globalObjects = Object.assign({
    define: bundleDefine,
    require: bundleRequire,
    bootstrap: bundleBootstrap,
    register: bundleRegister,
    render: bundleRender,
    __weex_define__: bundleDefine, // alias for define
    __weex_bootstrap__: bundleBootstrap, // alias for bootstrap
    __weex_document__: bundleDocument,
    __weex_require__: bundleRequireModule,
    __weex_viewmodel__: bundleVm,
    weex: weexGlobalObject
  }, timerAPIs, services);
  if (!callFunctionNative(globalObjects, functionBody)) {
    // If failed to compile functionBody on native side,
    // fallback to callFunction.
    callFunction(globalObjects, functionBody);
  }

  return result
}

/**
 * Call a new function body with some global objects.
 * @param  {object} globalObjects
 * @param  {string} code
 * @return {any}
 */
function callFunction (globalObjects, body) {
  var globalKeys = [];
  var globalValues = [];
  for (var key in globalObjects) {
    globalKeys.push(key);
    globalValues.push(globalObjects[key]);
  }
  globalKeys.push(body);

  var result = new (Function.prototype.bind.apply( Function, [ null ].concat( globalKeys) ));
  return result.apply(void 0, globalValues)
}

/**
 * Call a new function generated on the V8 native side.
 * @param  {object} globalObjects
 * @param  {string} body
 * @return {boolean} return true if no error occurred.
 */
function callFunctionNative (globalObjects, body) {
  if (typeof compileAndRunBundle !== 'function') {
    return false
  }

  var fn = void 0;
  var isNativeCompileOk = false;
  var script = '(function (';
  var globalKeys = [];
  var globalValues = [];
  for (var key in globalObjects) {
    globalKeys.push(key);
    globalValues.push(globalObjects[key]);
  }
  for (var i = 0; i < globalKeys.length - 1; ++i) {
    script += globalKeys[i];
    script += ',';
  }
  script += globalKeys[globalKeys.length - 1];
  script += ') {';
  script += body;
  script += '} )';

  try {
    var weex = globalObjects.weex || {};
    var config = weex.config || {};
    fn = compileAndRunBundle(script,
                             config.bundleUrl,
                             config.bundleDigest,
                             config.codeCachePath);
    if (fn && typeof fn === 'function') {
      fn.apply(void 0, globalValues);
      isNativeCompileOk = true;
    }
  }
  catch (e) {
    console.error(e);
  }

  return isNativeCompileOk
}

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
/**
 * @fileOverview
 * instance controls from native
 *
 * - init bundle
 * - fire event
 * - callback
 * - destroy
 *
 * corresponded with the API of instance manager (framework.js)
 */

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
var Differ = function Differ (id) {
  this.id = id;
  this.map = [];
  this.hooks = [];
};
Differ.prototype.isEmpty = function isEmpty () {
  return this.map.length === 0
};
Differ.prototype.append = function append (type, depth, ref, handler) {
    var this$1 = this;
    if ( depth === void 0 ) depth = 0;

  if (!this.hasTimer) {
    this.hasTimer = true;
    setTimeout(function () {
      this$1.hasTimer = false;
      this$1.flush(true);
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
  }
  else {
    group[type][ref] = handler;
  }
};
Differ.prototype.flush = function flush (isTimeout) {
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
};
Differ.prototype.then = function then (fn) {
  this.hooks.push(fn);
};

function callTypeMap (group, type) {
  var map = group[type];
  for (var ref in map) {
    map[ref]();
  }
}

function callTypeList (group, type) {
  var map = group[type];
  for (var ref in map) {
    var list = map[ref];
    list.forEach(function (handler) { handler(); });
  }
}

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
/**
 * @fileOverview
 * Weex App constructor & definition
 */

/**
 * App constructor for Weex framework.
 * @param {string} id
 * @param {object} options
 */
function App$1 (id, options) {
  this.id = id;
  this.options = options || {};
  this.vm = null;
  this.customComponentMap = {};
  this.commonModules = {};

  // document
  this.doc = new config$3.Document(
    id,
    this.options.bundleUrl,
    null,
    config$3.Listener
  );
  this.differ = new Differ(id);
}

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
/**
 * @fileOverview
 * Weex instance constructor & definition
 */

/**
 * @deprecated
 */
App$1.prototype.requireModule = function (name) {
  return requireModule$1(this, name)
};

/**
 * @deprecated
 */
App$1.prototype.updateActions = function () {
  return updateActions(this)
};

/**
 * @deprecated
 */
App$1.prototype.callTasks = function (tasks) {
  return callTasks(this, tasks)
};

/**
 * Prevent modification of App and App.prototype
 */
Object.freeze(App$1);
Object.freeze(App$1.prototype);

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
var instanceMap$2 = {};

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
/**
 * Create a Weex instance.
 *
 * @param  {string} id
 * @param  {string} code
 * @param  {object} options
 *         option `HAS_LOG` enable print log
 * @param  {object} data
 * @param  {object} info { created, ... services }
 */
function createInstance$3 (id, code, options, data, info) {
  var ref = info || {};
  var services = ref.services;
  resetTarget();
  var instance = instanceMap$2[id];
  /* istanbul ignore else */
  options = options || {};
  var result;
  /* istanbul ignore else */
  if (!instance) {
    instance = new App$1(id, options);
    instanceMap$2[id] = instance;
    result = init$5(instance, code, data, services);
  }
  else {
    result = new Error(("invalid instance id \"" + id + "\""));
  }
  return (result instanceof Error) ? result : instance
}

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
/**
 * Init config informations for Weex framework
 * @param  {object} cfg
 */
function init$6 (cfg) {
  config$3.Document = cfg.Document;
  config$3.Element = cfg.Element;
  config$3.Comment = cfg.Comment;
  config$3.sendTasks = cfg.sendTasks;
  config$3.Listener = cfg.Listener;
}

/**
 * Refresh a Weex instance with data.
 *
 * @param  {string} id
 * @param  {object} data
 */
function refreshInstance$2 (id, data) {
  var instance = instanceMap$2[id];
  var result;
  /* istanbul ignore else */
  if (instance) {
    result = refresh(instance, data);
  }
  else {
    result = new Error(("invalid instance id \"" + id + "\""));
  }
  return result
}

/**
 * Destroy a Weex instance.
 * @param  {string} id
 */
function destroyInstance$3 (id) {
  // Markup some global state in native side
  if (typeof markupState === 'function') {
    markupState();
  }

  resetTarget();
  var instance = instanceMap$2[id];
  /* istanbul ignore else */
  if (!instance) {
    return new Error(("invalid instance id \"" + id + "\""))
  }
  destroy$1(instance);
  delete instanceMap$2[id];
  // notifyContextDisposed is used to tell v8 to do a full GC,
  // but this would have a negative performance impact on weex,
  // because all the inline cache in v8 would get cleared
  // during a full GC.
  // To take care of both memory and performance, just tell v8
  // to do a full GC every eighteen times.
  var idNum = Math.round(id);
  var round = 18;
  if (idNum > 0) {
    var remainder = idNum % round;
    if (!remainder && typeof notifyTrimMemory === 'function') {
      notifyTrimMemory();
    }
  }
  return instanceMap$2
}

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
var nativeComponentMap$1 = config$3.nativeComponentMap;

/**
 * Register the name of each native component.
 * @param  {array} components array of name
 */
function registerComponents$2 (components) {
  if (Array.isArray(components)) {
    components.forEach(function register (name) {
      /* istanbul ignore if */
      if (!name) {
        return
      }
      if (typeof name === 'string') {
        nativeComponentMap$1[name] = true;
      }
      /* istanbul ignore else */
      else if (typeof name === 'object' && typeof name.type === 'string') {
        nativeComponentMap$1[name.type] = name;
      }
    });
  }
}

/**
 * Register the name and methods of each module.
 * @param  {object} modules a object of modules
 */
function registerModules$2 (modules) {
  /* istanbul ignore else */
  if (typeof modules === 'object') {
    initModules(modules);
  }
}

/**
 * Register the name and methods of each api.
 * @param  {object} apis a object of apis
 */
function registerMethods (methods) {
  /* istanbul ignore else */
  if (typeof methods === 'object') {
    initMethods$1(Vm, methods);
  }
}

// @todo: Hack for this framework only. Will be re-designed or removed later.
global.registerMethods = registerMethods;

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
var jsHandlers = {
  fireEvent: function (id) {
    var args = [], len = arguments.length - 1;
    while ( len-- > 0 ) args[ len ] = arguments[ len + 1 ];

    return fireEvent$1.apply(void 0, [ instanceMap$2[id] ].concat( args ))
  },
  callback: function (id) {
    var args = [], len = arguments.length - 1;
    while ( len-- > 0 ) args[ len ] = arguments[ len + 1 ];

    return callback$1.apply(void 0, [ instanceMap$2[id] ].concat( args ))
  }
};

/**
 * Accept calls from native (event or callback).
 *
 * @param  {string} id
 * @param  {array} tasks list with `method` and `args`
 */
function receiveTasks$2 (id, tasks) {
  var instance = instanceMap$2[id];
  if (instance && Array.isArray(tasks)) {
    var results = [];
    tasks.forEach(function (task) {
      var handler = jsHandlers[task.method];
      var args = [].concat( task.args );
      /* istanbul ignore else */
      if (typeof handler === 'function') {
        args.unshift(id);
        results.push(handler.apply(void 0, args));
      }
    });
    return results
  }
  return new Error(("invalid instance id \"" + id + "\" or tasks"))
}

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
/**
 * Get a whole element tree of an instance for debugging.
 * @param  {string} id
 * @return {object} a virtual dom tree
 */
function getRoot$2 (id) {
  var instance = instanceMap$2[id];
  var result;
  /* istanbul ignore else */
  if (instance) {
    result = getRootElement(instance);
  }
  else {
    result = new Error(("invalid instance id \"" + id + "\""));
  }
  return result
}

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
/**
 * @fileOverview Weex framework entry.
 */

// register special methods for Weex framework
registerMethods(methods$1);

/**
 * Prevent modification of Vm and Vm.prototype
 */
Object.freeze(Vm);




var Weex = Object.freeze({
	registerComponents: registerComponents$2,
	registerModules: registerModules$2,
	registerMethods: registerMethods,
	createInstance: createInstance$3,
	init: init$6,
	refreshInstance: refreshInstance$2,
	destroyInstance: destroyInstance$3,
	receiveTasks: receiveTasks$2,
	getRoot: getRoot$2
});

var framework_weex = createCommonjsModule(function (module) {
module.exports = /******/ (function(modules) { // webpackBootstrap
/******/ 	// The module cache
/******/ 	var installedModules = {};
/******/
/******/ 	// The require function
/******/ 	function __webpack_require__(moduleId) {
/******/
/******/ 		// Check if module is in cache
/******/ 		if(installedModules[moduleId]) {
/******/ 			return installedModules[moduleId].exports;
/******/ 		}
/******/ 		// Create a new module (and put it into the cache)
/******/ 		var module = installedModules[moduleId] = {
/******/ 			i: moduleId,
/******/ 			l: false,
/******/ 			exports: {}
/******/ 		};
/******/
/******/ 		// Execute the module function
/******/ 		modules[moduleId].call(module.exports, module, module.exports, __webpack_require__);
/******/
/******/ 		// Flag the module as loaded
/******/ 		module.l = true;
/******/
/******/ 		// Return the exports of the module
/******/ 		return module.exports;
/******/ 	}
/******/
/******/
/******/ 	// expose the modules object (__webpack_modules__)
/******/ 	__webpack_require__.m = modules;
/******/
/******/ 	// expose the module cache
/******/ 	__webpack_require__.c = installedModules;
/******/
/******/ 	// define getter function for harmony exports
/******/ 	__webpack_require__.d = function(exports, name, getter) {
/******/ 		if(!__webpack_require__.o(exports, name)) {
/******/ 			Object.defineProperty(exports, name, {
/******/ 				configurable: false,
/******/ 				enumerable: true,
/******/ 				get: getter
/******/ 			});
/******/ 		}
/******/ 	};
/******/
/******/ 	// getDefaultExport function for compatibility with non-harmony modules
/******/ 	__webpack_require__.n = function(module) {
/******/ 		var getter = module && module.__esModule ?
/******/ 			function getDefault() { return module['default']; } :
/******/ 			function getModuleExports() { return module; };
/******/ 		__webpack_require__.d(getter, 'a', getter);
/******/ 		return getter;
/******/ 	};
/******/
/******/ 	// Object.prototype.hasOwnProperty.call
/******/ 	__webpack_require__.o = function(object, property) { return Object.prototype.hasOwnProperty.call(object, property); };
/******/
/******/ 	// __webpack_public_path__
/******/ 	__webpack_require__.p = "";
/******/
/******/ 	// Load entry module and return exports
/******/ 	return __webpack_require__(__webpack_require__.s = 3);
/******/ })
/************************************************************************/
/******/ ([
/* 0 */
/*!***********************************************************************************************************************!*\
  !*** ./packages/weex-rax-framework/node_modules/.npminstall/event-target-shim/2.0.0/event-target-shim/lib/commons.js ***!
  \***********************************************************************************************************************/
/*! no static exports found */
/*! all exports used */
/***/ (function(module, exports, __webpack_require__) {

"use strict";
/**
 * @author Toru Nagashima
 * @copyright 2015 Toru Nagashima. All rights reserved.
 * See LICENSE file in root directory for full license.
 */


/**
 * Creates a unique key.
 *
 * @param {string} name - A name to create.
 * @returns {symbol|string} Generated unique key.
 * @private
 */
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
/*!****************************************************!*\
  !*** ./packages/weex-rax-framework/src/emitter.js ***!
  \****************************************************/
/*! no static exports found */
/*! all exports used */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


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
/*!****************************************************************************************************************************!*\
  !*** ./packages/weex-rax-framework/node_modules/.npminstall/event-target-shim/2.0.0/event-target-shim/lib/event-target.js ***!
  \****************************************************************************************************************************/
/*! no static exports found */
/*! all exports used */
/***/ (function(module, exports, __webpack_require__) {

"use strict";
/**
 * @author Toru Nagashima
 * @copyright 2015 Toru Nagashima. All rights reserved.
 * See LICENSE file in root directory for full license.
 */


//------------------------------------------------------------------------------
// Requirements
//------------------------------------------------------------------------------

/*globals window */

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
/*!**************************************************!*\
  !*** ./packages/weex-rax-framework/src/index.js ***!
  \**************************************************/
/*! no static exports found */
/*! all exports used */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


Object.defineProperty(exports, "__esModule", {
  value: true
});

var _slicedToArray = function () { function sliceIterator(arr, i) { var _arr = []; var _n = true; var _d = false; var _e = undefined; try { for (var _i = arr[Symbol.iterator](), _s; !(_n = (_s = _i.next()).done); _n = true) { _arr.push(_s.value); if (i && _arr.length === i) { break; } } } catch (err) { _d = true; _e = err; } finally { try { if (!_n && _i["return"]) { _i["return"](); } } finally { if (_d) { throw _e; } } } return _arr; } return function (arr, i) { if (Array.isArray(arr)) { return arr; } else if (Symbol.iterator in Object(arr)) { return sliceIterator(arr, i); } else { throw new TypeError("Invalid attempt to destructure non-iterable instance"); } }; }();

var _extends = Object.assign || function (target) {
var arguments$1 = arguments;
 for (var i = 1; i < arguments.length; i++) { var source = arguments$1[i]; for (var key in source) { if (Object.prototype.hasOwnProperty.call(source, key)) { target[key] = source[key]; } } } return target; };

var _typeof = typeof Symbol === "function" && typeof Symbol.iterator === "symbol" ? function (obj) { return typeof obj; } : function (obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; };

exports.getInstance = getInstance;
exports.init = init;
exports.registerComponents = registerComponents;
exports.registerMethods = registerMethods;
exports.registerModules = registerModules;
exports.createInstance = createInstance;
exports.refreshInstance = refreshInstance;
exports.destroyInstance = destroyInstance;
exports.getRoot = getRoot;
exports.receiveTasks = receiveTasks;

var _builtin = __webpack_require__(/*! ./builtin */ 4);

var _emitter = __webpack_require__(/*! ./emitter */ 1);

var _emitter2 = _interopRequireDefault(_emitter);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

var NativeComponents = {};
var NativeModules = {};

var Document = void 0;
var Element = void 0;
var Comment = void 0;

var MODULE_NAME_PREFIX = '@weex-module/';
var MODAL_MODULE = MODULE_NAME_PREFIX + 'modal';
var NAVIGATOR_MODULE = MODULE_NAME_PREFIX + 'navigator';
// Instance hub
var instances = {};
// Bundles hub
var bundles = {};
var noop = function noop() {};

function dispatchEventToInstance(event, targetOrigin) {
  var instance;
  for (var i in instances) {
    if (instances.hasOwnProperty(i)) {
      instance = instances[i];
      if (targetOrigin === '*' || targetOrigin === instance.origin) {
        event.target = instance.window;
        // FIXME: Need async?
        instance.window.dispatchEvent(event);
      }
    }
  }
}

function updateFinish(doc) {
  doc.taskCenter.send('dom', { action: 'updateFinish' }, []);
}

function getInstance(instanceId) {
  var instance = instances[instanceId];
  if (!instance) {
    throw new Error('Invalid instance id "' + instanceId + '"');
  }
  return instance;
}

function init(config) {
  Document = config.Document;
  Element = config.Element;
  Comment = config.Comment;
}

/**
 * register the name of each native component
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
        NativeComponents[name] = true;
      } else if ((typeof name === 'undefined' ? 'undefined' : _typeof(name)) === 'object' && typeof name.type === 'string') {
        NativeComponents[name.type] = name;
      }
    });
  }
}

/**
 * register the name and methods of each api
 * @param  {object} apis a object of apis
 */
function registerMethods(apis) {}
// Noop


/**
 * register the name and methods of each module
 * @param  {object} modules a object of modules
 */
function registerModules(newModules) {
  if ((typeof newModules === 'undefined' ? 'undefined' : _typeof(newModules)) === 'object') {
    for (var name in newModules) {
      if (Object.prototype.hasOwnProperty.call(newModules, name)) {
        NativeModules[name] = newModules[name];
      }
    }
  }
}

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

function genNativeModules(modules, document) {
  if ((typeof NativeModules === 'undefined' ? 'undefined' : _typeof(NativeModules)) === 'object') {
    var _loop = function _loop(name) {
      var moduleName = MODULE_NAME_PREFIX + name;
      modules[moduleName] = {
        module: { exports: {} },
        isInitialized: true
      };

      NativeModules[name].forEach(function (method) {
        if (typeof method === 'string') {
          method = {
            name: method
          };
        }
        var methodName = method.name;

        modules[moduleName].module.exports[methodName] = function () {
          var arguments$1 = arguments;

          for (var _len = arguments.length, args = Array(_len), _key = 0; _key < _len; _key++) {
            args[_key] = arguments$1[_key];
          }

          var options = {};
          var lastArg = args[args.length - 1];
          if (lastArg && (typeof lastArg === 'undefined' ? 'undefined' : _typeof(lastArg)) === 'object' && lastArg.__weex_options__) {
            options = lastArg.__weex_options__;
            // Remove the last in args
            args.pop();
          }
          // https://github.com/alibaba/weex/issues/1677
          return document.taskCenter.send('module', {
            module: name,
            method: methodName
          }, args, options);
        };
      });
    };

    for (var name in NativeModules) {
      _loop(name);
    }
  }

  return modules;
}

/**
 * create a Weex instance
 *
 * @param  {string} instanceId
 * @param  {string} __weex_code__
 * @param  {object} [__weex_options__] {bundleUrl, debug}
 */
function createInstance(instanceId, __weex_code__, __weex_options__, __weex_data__, __weex_config__) {
  var instance = instances[instanceId];
  if (instance == undefined) {
    // Mark start time
    var responseEnd = Date.now();
    var __weex_env__ = (typeof WXEnvironment === 'undefined' ? 'undefined' : _typeof(WXEnvironment)) === 'object' && WXEnvironment || {};
    // For better performance use built-in promise first
    var shared = __webpack_require__(/*! runtime-shared/dist/shared.function */ 6)();

    var _Promise = typeof _Promise === 'function' ? _Promise : shared.Promise;
    var _Symbol = typeof _Symbol === 'function' ? _Symbol : shared.Symbol;
    var _Set = typeof _Set === 'function' ? _Set : shared.Set;
    var _Map = typeof _Map === 'function' ? _Map : shared.Map;
    var _WeakMap = typeof _WeakMap === 'function' ? _WeakMap : shared.WeakMap;
    var _WeakSet = typeof _WeakSet === 'function' ? _WeakSet : shared.WeakSet;
    var URL = shared.URL,
        URLSearchParams = shared.URLSearchParams,
        FontFace = shared.FontFace,
        matchMedia = shared.matchMedia;

    var bundleUrl = __weex_options__.bundleUrl || 'about:blank';

    if (!__weex_options__.bundleUrl) {
      console.error('Error: Must have bundleUrl option when createInstance, downgrade to "about:blank".');
    }

    var document = new Document(instanceId, bundleUrl);
    var documentURL = new URL(bundleUrl);
    var modules = {};

    instance = instances[instanceId] = {
      document: document,
      instanceId: instanceId,
      bundleUrl: bundleUrl,
      bundleCode: __weex_code__,
      modules: modules,
      origin: documentURL.origin,
      uid: 0
    };

    // Generate native modules map at instance init
    genNativeModules(modules, document);
    var __weex_define__ = __webpack_require__(/*! ./define.weex */ 7)(modules);
    var __weex_require__ = __webpack_require__(/*! ./require.weex */ 8)(modules);
    var __weex_downgrade__ = __webpack_require__(/*! ./downgrade.weex */ 9)(__weex_require__);
    // Extend document
    __webpack_require__(/*! ./document.weex */ 11)(__weex_require__, document);

    var location = __webpack_require__(/*! ./location.weex */ 12)(__weex_require__, documentURL);

    var _require = __webpack_require__(/*! ./fetch.weex */ 13)(__weex_require__, _Promise),
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

    var window = {
      // ES
      Promise: _Promise,
      Symbol: _Symbol,
      Map: _Map,
      Set: _Set,
      WeakMap: _WeakMap,
      WeakSet: _WeakSet,
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
        dispatchEventToInstance(event, targetOrigin);
      },
      addEventListener: function addEventListener(type, listener) {
        windowEmitter.on(type, listener);
      },
      removeEventListener: function removeEventListener(type, listener) {
        windowEmitter.off(type, listener);
      },
      dispatchEvent: function dispatchEvent(e) {
        windowEmitter.emit(e.type, e);
      },
      // ModuleJS
      define: __weex_define__,
      require: __weex_require__,
      // Weex
      __weex_document__: document,
      __weex_define__: __weex_define__,
      __weex_require__: __weex_require__,
      __weex_downgrade__: __weex_downgrade__,
      __weex_env__: __weex_env__,
      __weex_code__: __weex_code__,
      __weex_options__: __weex_options__,
      __weex_data__: __weex_data__,
      __weex_config__: __weex_config__
    };

    instance.window = window.self = window.window = window;

    var builtinGlobals = {};
    var builtinModules = {};
    try {
      builtinGlobals = __weex_config__.services.builtinGlobals;
      // Modules should wrap as module factory format
      builtinModules = __weex_config__.services.builtinModules;
    } catch (e) {}

    Object.assign(window, builtinGlobals);

    var moduleFactories = _extends({}, _builtin.ModuleFactories, builtinModules);
    genBuiltinModules(modules, moduleFactories, window);
    // In weex iOS or Android
    if (__weex_env__.platform !== 'Web') {
      var timing = performance.timing;
      timing.domLoading = Date.now();

      // Use the cached init function, if existed in bundles
      var _init = bundles[__weex_code__] ? bundles[__weex_code__] : new Function('with(this){(function(){"use strict";\n' + __weex_code__ + '\n}).call(this)}');

      _init.call(
      // Context is window
      window);

      timing.domInteractive = timing.domComplete = timing.domInteractive = Date.now();

      // Cache the init function
      bundles[__weex_code__] = _init;
    } else {
      // In weex h5
      var _init2 = new Function('"use strict";\n' + __weex_code__);

      _init2.call(window);
    }
  } else {
    throw new Error('Instance id "' + instanceId + '" existed when create instance');
  }
}

/**
 * refresh a Weex instance
 *
 * @param  {string} instanceId
 * @param  {object} data
 */
function refreshInstance(instanceId, data) {
  var instance = getInstance(instanceId);
  var document = instance.document;
  document.documentElement.fireEvent('refresh', {
    timestamp: Date.now(),
    data: data
  });
  document.taskCenter.send('dom', { action: 'refreshFinish' }, []);
}

/**
 * destroy a Weex instance
 * @param  {string} instanceId
 */
function destroyInstance(instanceId) {
  var instance = getInstance(instanceId);
  var bundleCode = instance.bundleCode;
  instance.window.closed = true;

  var document = instance.document;
  document.documentElement.fireEvent('destory', {
    timestamp: Date.now()
  });

  if (document.destroy) {
    document.destroy();
  }

  if (document.taskCenter && document.taskCenter.destroyCallback) {
    document.taskCenter.destroyCallback();
  }

  delete instances[instanceId];
  delete bundles[bundleCode];
}

/**
 * get a whole element tree of an instance
 * for debugging
 * @param  {string} instanceId
 * @return {object} a virtual dom tree
 */
function getRoot(instanceId) {
  var instance = getInstance(instanceId);
  var document = instance.document;
  return document.toJSON ? document.toJSON() : {};
}

function fireEvent(doc, ref, type, e, domChanges) {
  if (Array.isArray(ref)) {
    ref.some(function (ref) {
      return fireEvent(doc, ref, type, e) !== false;
    });
    return;
  }

  var el = doc.getRef(ref);

  if (el) {
    var result = doc.fireEvent(el, type, e, domChanges);
    updateFinish(doc);
    return result;
  }

  return new Error('Invalid element reference "' + ref + '"');
}

/**
 * accept calls from native (event or callback)
 *
 * @param  {string} instanceId
 * @param  {array} tasks list with `method` and `args`
 */
function receiveTasks(instanceId, tasks) {
  var instance = getInstance(instanceId);
  if (Array.isArray(tasks)) {
    var document = instance.document;

    var results = [];
    tasks.forEach(function (task) {
      var result = void 0;
      if (task.method === 'fireEvent') {
        var _task$args = _slicedToArray(task.args, 4),
            nodeId = _task$args[0],
            type = _task$args[1],
            data = _task$args[2],
            domChanges = _task$args[3];

        result = fireEvent(document, nodeId, type, data, domChanges);
      } else if (task.method === 'callback') {
        var _task$args2 = _slicedToArray(task.args, 3),
            uid = _task$args2[0],
            _data = _task$args2[1],
            ifKeepAlive = _task$args2[2];

        result = document.taskCenter.callback(uid, _data, ifKeepAlive);
        updateFinish(document);
      }
      results.push(result);
    });
    return results;
  }
}

// FIXME: Hack for rollup build "import Rax from 'weex-rax-framework'", in rollup if `module.exports` has `__esModule` key must return by export default
exports.default = exports;

/***/ }),
/* 4 */
/*!****************************************************!*\
  !*** ./packages/weex-rax-framework/src/builtin.js ***!
  \****************************************************/
/*! no static exports found */
/*! all exports used */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


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
/*! no static exports found */
/*! all exports used */
/***/ (function(module, exports) {

module.exports = function(require, exports, module) {
  var __weex_document__ = this["__weex_document__"];
var document = this["document"];
  module.exports = /******/ (function(modules) { // webpackBootstrap
/******/ 	// The module cache
/******/ 	var installedModules = {};
/******/
/******/ 	// The require function
/******/ 	function __webpack_require__(moduleId) {
/******/
/******/ 		// Check if module is in cache
/******/ 		if(installedModules[moduleId]) {
/******/ 			return installedModules[moduleId].exports;
/******/ 		}
/******/ 		// Create a new module (and put it into the cache)
/******/ 		var module = installedModules[moduleId] = {
/******/ 			i: moduleId,
/******/ 			l: false,
/******/ 			exports: {}
/******/ 		};
/******/
/******/ 		// Execute the module function
/******/ 		modules[moduleId].call(module.exports, module, module.exports, __webpack_require__);
/******/
/******/ 		// Flag the module as loaded
/******/ 		module.l = true;
/******/
/******/ 		// Return the exports of the module
/******/ 		return module.exports;
/******/ 	}
/******/
/******/
/******/ 	// expose the modules object (__webpack_modules__)
/******/ 	__webpack_require__.m = modules;
/******/
/******/ 	// expose the module cache
/******/ 	__webpack_require__.c = installedModules;
/******/
/******/ 	// define getter function for harmony exports
/******/ 	__webpack_require__.d = function(exports, name, getter) {
/******/ 		if(!__webpack_require__.o(exports, name)) {
/******/ 			Object.defineProperty(exports, name, {
/******/ 				configurable: false,
/******/ 				enumerable: true,
/******/ 				get: getter
/******/ 			});
/******/ 		}
/******/ 	};
/******/
/******/ 	// getDefaultExport function for compatibility with non-harmony modules
/******/ 	__webpack_require__.n = function(module) {
/******/ 		var getter = module && module.__esModule ?
/******/ 			function getDefault() { return module['default']; } :
/******/ 			function getModuleExports() { return module; };
/******/ 		__webpack_require__.d(getter, 'a', getter);
/******/ 		return getter;
/******/ 	};
/******/
/******/ 	// Object.prototype.hasOwnProperty.call
/******/ 	__webpack_require__.o = function(object, property) { return Object.prototype.hasOwnProperty.call(object, property); };
/******/
/******/ 	// __webpack_public_path__
/******/ 	__webpack_require__.p = "";
/******/
/******/ 	// Load entry module and return exports
/******/ 	return __webpack_require__(__webpack_require__.s = 15);
/******/ })
/************************************************************************/
/******/ ([
/* 0 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


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

"use strict";


Object.defineProperty(exports, "__esModule", {
  value: true
});

var _host = __webpack_require__(0);

var _host2 = _interopRequireDefault(_host);

var _element = __webpack_require__(7);

var _unmountComponentAtNode = __webpack_require__(8);

var _unmountComponentAtNode2 = _interopRequireDefault(_unmountComponentAtNode);

var _instantiateComponent = __webpack_require__(2);

var _instantiateComponent2 = _interopRequireDefault(_instantiateComponent);

var _shouldUpdateComponent = __webpack_require__(4);

var _shouldUpdateComponent2 = _interopRequireDefault(_shouldUpdateComponent);

var _root = __webpack_require__(18);

var _root2 = _interopRequireDefault(_root);

var _universalEnv = __webpack_require__(3);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _toConsumableArray(arr) { if (Array.isArray(arr)) { for (var i = 0, arr2 = Array(arr.length); i < arr.length; i++) { arr2[i] = arr[i]; } return arr2; } else { return Array.from(arr); } }

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
  render: function render(element, container) {
    _host2.default.driver.beforeRender && _host2.default.driver.beforeRender();

    // Real native root node is body
    if (container == null) {
      container = _host2.default.driver.createBody();
    }

    var prevRootInstance = this.get(container);
    var hasPrevRootInstance = prevRootInstance && prevRootInstance.isRootComponent;

    if (hasPrevRootInstance) {
      var prevRenderedComponent = prevRootInstance.getRenderedComponent();
      var prevElement = prevRenderedComponent._currentElement;
      if ((0, _shouldUpdateComponent2.default)(prevElement, element)) {
        var prevUnmaskedContext = prevRenderedComponent._context;
        prevRenderedComponent.updateComponent(prevElement, element, prevUnmaskedContext, prevUnmaskedContext);

        return prevRootInstance;
      } else {
        _host2.default.hook.Reconciler.unmountComponent(prevRootInstance);
        (0, _unmountComponentAtNode2.default)(container);
      }
    }

    // Handle server rendered element
    if (_universalEnv.isWeb && container.childNodes) {
      // Clone childNodes, Because removeChild will causing change in childNodes length
      var childNodes = [].concat(_toConsumableArray(container.childNodes));

      for (var i = 0; i < childNodes.length; i++) {
        var rootChildNode = childNodes[i];
        if (rootChildNode.hasAttribute && rootChildNode.hasAttribute('data-rendered')) {
          _host2.default.driver.removeChild(rootChildNode, container);
        }
      }
    }

    var wrappedElement = (0, _element.createElement)(_root2.default, null, element);
    var renderedComponent = (0, _instantiateComponent2.default)(wrappedElement);
    var defaultContext = {};
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

"use strict";


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
    throw Error('Invalid element type ' + JSON.stringify(element));
  }

  instance._mountIndex = 0;

  return instance;
}

exports.default = instantiateComponent;
module.exports = exports['default'];

/***/ }),
/* 3 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


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

/***/ }),
/* 4 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


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
/* 5 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


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
/* 6 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


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
/* 7 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


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

var _flattenChildren = __webpack_require__(17);

var _flattenChildren2 = _interopRequireDefault(_flattenChildren);

var _universalEnv = __webpack_require__(3);

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

var Element = function Element(type, key, ref, props, owner) {
  props = filterProps(type, props);

  return {
    // Built-in properties that belong on the element
    type: type,
    key: key,
    ref: ref,
    props: props,
    // Record the component responsible for creating this element.
    _owner: owner
  };
};

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
  if (_universalEnv.isWeex && type === 'text') {
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

function createElement(type, config) {
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
      if (config.hasOwnProperty(propName) && !RESERVED_PROPS.hasOwnProperty(propName)) {
        props[propName] = config[propName];
      }
    }
  }

  for (var _len = arguments.length, children = Array(_len > 2 ? _len - 2 : 0), _key = 2; _key < _len; _key++) {
    children[_key - 2] = arguments$1[_key];
  }

  if (children.length) {
    props.children = (0, _flattenChildren2.default)(children);
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

  for (var _len2 = arguments.length, children = Array(_len2 > 2 ? _len2 - 2 : 0), _key2 = 2; _key2 < _len2; _key2++) {
    children[_key2 - 2] = arguments$1[_key2];
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
/* 8 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


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
/* 9 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


Object.defineProperty(exports, "__esModule", {
  value: true
});

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) { descriptor.writable = true; } Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) { defineProperties(Constructor.prototype, protoProps); } if (staticProps) { defineProperties(Constructor, staticProps); } return Constructor; }; }();

var _host = __webpack_require__(0);

var _host2 = _interopRequireDefault(_host);

var _ref = __webpack_require__(10);

var _ref2 = _interopRequireDefault(_ref);

var _instantiateComponent = __webpack_require__(2);

var _instantiateComponent2 = _interopRequireDefault(_instantiateComponent);

var _shouldUpdateComponent = __webpack_require__(4);

var _shouldUpdateComponent2 = _interopRequireDefault(_shouldUpdateComponent);

var _getElementKeyName = __webpack_require__(11);

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
          if (typeof prevProp === 'function') {
            _host2.default.driver.removeEventListener(this$1.getNativeNode(), propKey.slice(2).toLowerCase(), prevProp);
          }

          if (typeof nextProp === 'function') {
            _host2.default.driver.addEventListener(this$1.getNativeNode(), propKey.slice(2).toLowerCase(), nextProp);
          }
          // Update other property
        } else {
          var payload = {};
          payload[propKey] = nextProp;
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
/* 10 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


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
/* 11 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


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
/* 12 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


Object.defineProperty(exports, "__esModule", {
  value: true
});
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

// Default 1 rem to 1 px
var defaultRemUnit = 1;

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
  var remUnit = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : defaultRemUnit;

  return str.replace(REM_REG, function (rem) {
    return parseFloat(rem) * remUnit + 'px';
  });
}

function getRem() {
  return defaultRemUnit;
}

function setRem(rem) {
  defaultRemUnit = rem;
}

function isUnitNumber(val, prop) {
  return typeof val === 'number' && !UNITLESS_NUMBER_PROPS[prop];
}

function convertUnit(val, prop) {
  var remUnit = arguments.length > 2 && arguments[2] !== undefined ? arguments[2] : defaultRemUnit;

  if (prop && isUnitNumber(val, prop)) {
    return val * remUnit + 'px';
  } else if (isRem(val)) {
    return calcRem(val, remUnit);
  }

  return val;
}

/***/ }),
/* 13 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


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

/***/ }),
/* 14 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


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
    throw new Error('Appears to be neither Component nor DOMNode.');
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
    throw new Error('findDOMNode was called on an unmounted component.');
  }
}

exports.default = findDOMNode;
module.exports = exports['default'];

/***/ }),
/* 15 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.version = exports.setNativeProps = exports.findComponentInstance = exports.unmountComponentAtNode = exports.findDOMNode = exports.render = exports.PropTypes = exports.PureComponent = exports.Component = exports.createFactory = exports.isValidElement = exports.cloneElement = exports.createElement = undefined;

__webpack_require__(16);

var _element = __webpack_require__(7);

var _component = __webpack_require__(5);

var _component2 = _interopRequireDefault(_component);

var _purecomponent = __webpack_require__(19);

var _purecomponent2 = _interopRequireDefault(_purecomponent);

var _proptypes = __webpack_require__(20);

var _proptypes2 = _interopRequireDefault(_proptypes);

var _render2 = __webpack_require__(21);

var _render3 = _interopRequireDefault(_render2);

var _findDOMNode2 = __webpack_require__(14);

var _findDOMNode3 = _interopRequireDefault(_findDOMNode2);

var _unmountComponentAtNode2 = __webpack_require__(8);

var _unmountComponentAtNode3 = _interopRequireDefault(_unmountComponentAtNode2);

var _findComponentInstance2 = __webpack_require__(42);

var _findComponentInstance3 = _interopRequireDefault(_findComponentInstance2);

var _setNativeProps2 = __webpack_require__(43);

var _setNativeProps3 = _interopRequireDefault(_setNativeProps2);

var _version2 = __webpack_require__(44);

var _version3 = _interopRequireDefault(_version2);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

exports.createElement = _element.createElement;
exports.cloneElement = _element.cloneElement;
exports.isValidElement = _element.isValidElement;
exports.createFactory = _element.createFactory;
exports.Component = _component2.default;
exports.PureComponent = _purecomponent2.default;
exports.PropTypes = _proptypes2.default;
exports.render = _render3.default;
exports.findDOMNode = _findDOMNode3.default;
exports.unmountComponentAtNode = _unmountComponentAtNode3.default;
exports.findComponentInstance = _findComponentInstance3.default;
exports.setNativeProps = _setNativeProps3.default;
exports.version = _version3.default;

/***/ }),
/* 16 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


var _hook = __webpack_require__(6);

var _hook2 = _interopRequireDefault(_hook);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

/* global __REACT_DEVTOOLS_GLOBAL_HOOK__ */
if (typeof __REACT_DEVTOOLS_GLOBAL_HOOK__ !== 'undefined' && typeof __REACT_DEVTOOLS_GLOBAL_HOOK__.inject === 'function') {
  __REACT_DEVTOOLS_GLOBAL_HOOK__.inject(_hook2.default);
}

/***/ }),
/* 17 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


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
/* 18 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


Object.defineProperty(exports, "__esModule", {
  value: true
});

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) { descriptor.writable = true; } Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) { defineProperties(Constructor.prototype, protoProps); } if (staticProps) { defineProperties(Constructor, staticProps); } return Constructor; }; }();

var _component = __webpack_require__(5);

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
/* 19 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


Object.defineProperty(exports, "__esModule", {
  value: true
});

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) { descriptor.writable = true; } Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) { defineProperties(Constructor.prototype, protoProps); } if (staticProps) { defineProperties(Constructor, staticProps); } return Constructor; }; }();

var _component = __webpack_require__(5);

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
/* 20 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


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
/* 21 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


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

  // Init inject
  (0, _inject2.default)(options || {});

  var rootComponent = _instance2.default.render(element, container);
  var component = rootComponent.getPublicInstance();

  if (callback) {
    callback.call(component);
  }

  return component;
}

exports.default = render;
module.exports = exports['default'];

/***/ }),
/* 22 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.default = inject;

var _universalEnv = __webpack_require__(3);

var _host = __webpack_require__(0);

var _host2 = _interopRequireDefault(_host);

var _empty = __webpack_require__(23);

var _empty2 = _interopRequireDefault(_empty);

var _native = __webpack_require__(9);

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

var _hook = __webpack_require__(6);

var _hook2 = _interopRequireDefault(_hook);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function inject(_ref) {
  var driver = _ref.driver,
      hook = _ref.hook,
      measurer = _ref.measurer,
      deviceWidth = _ref.deviceWidth,
      viewportWidth = _ref.viewportWidth;

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

  if (deviceWidth && driver.setDeviceWidth) {
    driver.setDeviceWidth(deviceWidth);
  }

  if (viewportWidth && driver.setViewportWidth) {
    driver.setViewportWidth(viewportWidth);
  }
}
module.exports = exports['default'];

/***/ }),
/* 23 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


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

"use strict";


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

"use strict";


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

var _ref = __webpack_require__(10);

var _ref2 = _interopRequireDefault(_ref);

var _instantiateComponent = __webpack_require__(2);

var _instantiateComponent2 = _interopRequireDefault(_instantiateComponent);

var _shouldUpdateComponent = __webpack_require__(4);

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
  var _instance = instance;

  while (_instance) {
    if (typeof _instance.componentDidCatch === 'function') {
      boundary = _instance;
      break;
    } else if (_instance._internal && _instance._internal._parentInstance) {
      _instance = _instance._internal._parentInstance;
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

var measureLifeCycle = void 0;
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
      var _this = this;

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
        throw Error('Invalid component type ' + JSON.stringify(Component));
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

      performInSandbox(function () {
        if (instance.componentWillMount) {
          {
            instance.componentWillMount();
          }
        }
      }, instance, errorCallback);

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

      performInSandbox(function () {
        if (instance.componentDidMount) {
          {
            instance.componentDidMount();
          }
        }
      }, instance);

      _host2.default.hook.Reconciler.mountComponent(this);

      return instance;
    }
  }, {
    key: 'unmountComponent',
    value: function unmountComponent(notRemoveChild) {
      var instance = this._instance;

      performInSandbox(function () {
        if (instance.componentWillUnmount) {
          instance.componentWillUnmount();
        }
      }, instance);

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
      var _this2 = this;

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

"use strict";


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

"use strict";


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

    if (!internal._pendingState) {
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
    this.runUpdate(component);
  },

  runUpdate: function runUpdate(component) {
    var internal = component._internal;

    if (!internal || !internal._renderedComponent) {
      return;
    }

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

"use strict";


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

"use strict";


Object.defineProperty(exports, "__esModule", {
  value: true
});

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) { descriptor.writable = true; } Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) { defineProperties(Constructor.prototype, protoProps); } if (staticProps) { defineProperties(Constructor, staticProps); } return Constructor; }; }();

var _host = __webpack_require__(0);

var _host2 = _interopRequireDefault(_host);

var _native = __webpack_require__(9);

var _native2 = _interopRequireDefault(_native);

var _instance = __webpack_require__(1);

var _instance2 = _interopRequireDefault(_instance);

var _instantiateComponent = __webpack_require__(2);

var _instantiateComponent2 = _interopRequireDefault(_instantiateComponent);

var _getElementKeyName = __webpack_require__(11);

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

"use strict";


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

var _styleUnit = __webpack_require__(12);

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
    var body = document.createBody();
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
    var events = [];
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
  addEventListener: function addEventListener(node, eventName, eventHandler) {
    return node.addEvent(eventName, eventHandler);
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
/* 31 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


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

"use strict";


Object.defineProperty(exports, "__esModule", {
  value: true
});

var _parseProps = __webpack_require__(13);

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

"use strict";


Object.defineProperty(exports, "__esModule", {
  value: true
});

var _parseProps = __webpack_require__(13);

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

"use strict";


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

"use strict";


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

"use strict";


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

"use strict";


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

"use strict";


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

"use strict";


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

"use strict";


Object.defineProperty(exports, "__esModule", {
  value: true
});

var _styleUnit = __webpack_require__(12);

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

var Driver = {

  deviceWidth: typeof DEVICE_WIDTH !== 'undefined' && DEVICE_WIDTH || null,
  viewportWidth: typeof VIEWPORT_WIDTH !== 'undefined' && VIEWPORT_WIDTH || 750,

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
  addEventListener: function addEventListener(node, eventName, eventHandler) {
    return node.addEventListener(eventName, eventHandler);
  },
  removeEventListener: function removeEventListener(node, eventName, eventHandler) {
    return node.removeEventListener(eventName, eventHandler);
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
      node[propKey] = null;
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
      node[propKey] = propValue;
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

"use strict";


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

"use strict";


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
/* 43 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.default = setNativeProps;

var _host = __webpack_require__(0);

var _host2 = _interopRequireDefault(_host);

var _findDOMNode = __webpack_require__(14);

var _findDOMNode2 = _interopRequireDefault(_findDOMNode);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function setNativeProps(node, props) {
  node = (0, _findDOMNode2.default)(node);
  _host2.default.driver.setNativeProps(node, props);
}
module.exports = exports['default'];

/***/ }),
/* 44 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.default = '0.4.11';
module.exports = exports['default'];

/***/ })
/******/ ]);};

/***/ }),
/* 6 */
/*!*********************************************************!*\
  !*** ./packages/runtime-shared/dist/shared.function.js ***!
  \*********************************************************/
/*! no static exports found */
/*! all exports used */
/***/ (function(module, exports) {

module.exports = function() {
  return /******/ (function(modules) { // webpackBootstrap
/******/ 	// The module cache
/******/ 	var installedModules = {};
/******/
/******/ 	// The require function
/******/ 	function __webpack_require__(moduleId) {
/******/
/******/ 		// Check if module is in cache
/******/ 		if(installedModules[moduleId]) {
/******/ 			return installedModules[moduleId].exports;
/******/ 		}
/******/ 		// Create a new module (and put it into the cache)
/******/ 		var module = installedModules[moduleId] = {
/******/ 			i: moduleId,
/******/ 			l: false,
/******/ 			exports: {}
/******/ 		};
/******/
/******/ 		// Execute the module function
/******/ 		modules[moduleId].call(module.exports, module, module.exports, __webpack_require__);
/******/
/******/ 		// Flag the module as loaded
/******/ 		module.l = true;
/******/
/******/ 		// Return the exports of the module
/******/ 		return module.exports;
/******/ 	}
/******/
/******/
/******/ 	// expose the modules object (__webpack_modules__)
/******/ 	__webpack_require__.m = modules;
/******/
/******/ 	// expose the module cache
/******/ 	__webpack_require__.c = installedModules;
/******/
/******/ 	// define getter function for harmony exports
/******/ 	__webpack_require__.d = function(exports, name, getter) {
/******/ 		if(!__webpack_require__.o(exports, name)) {
/******/ 			Object.defineProperty(exports, name, {
/******/ 				configurable: false,
/******/ 				enumerable: true,
/******/ 				get: getter
/******/ 			});
/******/ 		}
/******/ 	};
/******/
/******/ 	// getDefaultExport function for compatibility with non-harmony modules
/******/ 	__webpack_require__.n = function(module) {
/******/ 		var getter = module && module.__esModule ?
/******/ 			function getDefault() { return module['default']; } :
/******/ 			function getModuleExports() { return module; };
/******/ 		__webpack_require__.d(getter, 'a', getter);
/******/ 		return getter;
/******/ 	};
/******/
/******/ 	// Object.prototype.hasOwnProperty.call
/******/ 	__webpack_require__.o = function(object, property) { return Object.prototype.hasOwnProperty.call(object, property); };
/******/
/******/ 	// __webpack_public_path__
/******/ 	__webpack_require__.p = "";
/******/
/******/ 	// Load entry module and return exports
/******/ 	return __webpack_require__(__webpack_require__.s = 2);
/******/ })
/************************************************************************/
/******/ ([
/* 0 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


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

"use strict";


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

"use strict";


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

"use strict";


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

"use strict";


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
  return Number.isNaN(key) ? NaNMarker : key;
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

  thisArg = thisArg || commonjsGlobal;
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

"use strict";


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
  return Number.isNaN(data) ? NaNMarker : data;
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

  thisArg = thisArg || commonjsGlobal;
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

"use strict";


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

"use strict";


/* eslint no-extend-native: "off" */

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

"use strict";


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

"use strict";


// https://github.com/Polymer/URL

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
function URL(url, base /* , encoding */) {
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

"use strict";


// https://github.com/ericf/css-mediaquery

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
/*!********************************************************!*\
  !*** ./packages/weex-rax-framework/src/define.weex.js ***!
  \********************************************************/
/*! no static exports found */
/*! all exports used */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


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
/*!*********************************************************!*\
  !*** ./packages/weex-rax-framework/src/require.weex.js ***!
  \*********************************************************/
/*! no static exports found */
/*! all exports used */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


module.exports = function (modules) {
  function require(name) {
    var mod = modules[name];

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
/*!***********************************************************!*\
  !*** ./packages/weex-rax-framework/src/downgrade.weex.js ***!
  \***********************************************************/
/*! no static exports found */
/*! all exports used */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


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
/*!***************************************************!*\
  !*** ./packages/weex-rax-framework/src/semver.js ***!
  \***************************************************/
/*! no static exports found */
/*! all exports used */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


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
/*!**********************************************************!*\
  !*** ./packages/weex-rax-framework/src/document.weex.js ***!
  \**********************************************************/
/*! no static exports found */
/*! all exports used */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


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
/*!**********************************************************!*\
  !*** ./packages/weex-rax-framework/src/location.weex.js ***!
  \**********************************************************/
/*! no static exports found */
/*! all exports used */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


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

  location.reload = function (forceReload) {
    var weexLocation = __weex_require__(LOCATION_MODULE);
    weexLocation.reload(forceReload);
  };

  return location;
};

/***/ }),
/* 13 */
/*!*******************************************************!*\
  !*** ./packages/weex-rax-framework/src/fetch.weex.js ***!
  \*******************************************************/
/*! no static exports found */
/*! all exports used */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


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

    for (var name in this.map) {
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

  function headers(xhr) {
    var head = new Headers();
    var pairs = xhr.getAllResponseHeaders().trim().split('\n');
    pairs.forEach(function (header) {
      var split = header.trim().split(':');
      var key = split.shift().trim();
      var value = split.join(':').trim();
      head.append(key, value);
    });
    return head;
  }

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
/*!****************************************************************!*\
  !*** ./packages/weex-rax-framework/src/xmlhttprequest.weex.js ***!
  \****************************************************************/
/*! no static exports found */
/*! all exports used */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


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
/*!***********************************************************************************************************************************!*\
  !*** ./packages/weex-rax-framework/node_modules/.npminstall/event-target-shim/2.0.0/event-target-shim/lib/custom-event-target.js ***!
  \***********************************************************************************************************************************/
/*! no static exports found */
/*! all exports used */
/***/ (function(module, exports, __webpack_require__) {

"use strict";
/**
 * @author Toru Nagashima
 * @copyright 2015 Toru Nagashima. All rights reserved.
 * See LICENSE file in root directory for full license.
 */


//-----------------------------------------------------------------------------
// Requirements
//-----------------------------------------------------------------------------

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
/*!*****************************************************************************************************************************!*\
  !*** ./packages/weex-rax-framework/node_modules/.npminstall/event-target-shim/2.0.0/event-target-shim/lib/event-wrapper.js ***!
  \*****************************************************************************************************************************/
/*! no static exports found */
/*! all exports used */
/***/ (function(module, exports, __webpack_require__) {

"use strict";
/**
 * @author Toru Nagashima
 * @copyright 2015 Toru Nagashima. All rights reserved.
 * See LICENSE file in root directory for full license.
 */


//-----------------------------------------------------------------------------
// Requirements
//-----------------------------------------------------------------------------

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
/*!***********************************************************!*\
  !*** ./packages/weex-rax-framework/src/websocket.weex.js ***!
  \***********************************************************/
/*! no static exports found */
/*! all exports used */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


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

var CLOSE_NORMAL = 1000;

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
/*!*******************************************************!*\
  !*** ./packages/weex-rax-framework/src/timer.weex.js ***!
  \*******************************************************/
/*! no static exports found */
/*! all exports used */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


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
/*!********************************************************!*\
  !*** ./packages/weex-rax-framework/src/base64.weex.js ***!
  \********************************************************/
/*! no static exports found */
/*! all exports used */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


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
    ~buffer && (bs = bc % 4 ? bs * 64 + buffer : buffer,
    // and if not first of each 4 characters,
    // convert the first 8 bits to one ascii character
    bc++ % 4) ? output += String.fromCharCode(255 & bs >> (-2 * bc & 6)) : 0) {
      // try to find character in table (0-63, not found => -1)
      buffer = chars.indexOf(buffer);
    }
    return output;
  };

  return base64;
};

/***/ }),
/* 20 */
/*!*************************************************************!*\
  !*** ./packages/weex-rax-framework/src/performance.weex.js ***!
  \*************************************************************/
/*! no static exports found */
/*! all exports used */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


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
/*!*******************************************************!*\
  !*** ./packages/weex-rax-framework/src/event.weex.js ***!
  \*******************************************************/
/*! no static exports found */
/*! all exports used */
/***/ (function(module, exports, __webpack_require__) {

"use strict";


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

var Rax = unwrapExports(framework_weex);

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
var frameworks = {
  Vanilla: Vanilla,
  Vue: Vue,
  Rax: Rax,
  Weex: Weex
};

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
setup(frameworks);

})));