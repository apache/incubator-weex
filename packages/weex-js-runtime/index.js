/* 'WEEX JS RUNTIME 0.20.6, Build 2017-08-01 19:09. */

;(this.getJSFMVersion = function(){return "0.20.6"});
var global = this, process = { env: {} };var setTimeout = global.setTimeout;

(function (global, factory) {
	typeof exports === 'object' && typeof module !== 'undefined' ? module.exports = factory() :
	typeof define === 'function' && define.amd ? define(factory) :
	(global.WeexRuntime = factory());
}(this, (function () { 'use strict';

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
      // 17. Repeat, while k < len… (also steps a - h)
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





function createCommonjsModule(fn, module) {
	return module = { exports: {} }, fn(module, module.exports), module.exports;
}

var _global = createCommonjsModule(function (module) {
// https://github.com/zloirock/core-js/issues/86#issuecomment-115759028
var global = module.exports = typeof window != 'undefined' && window.Math == Math
  ? window : typeof self != 'undefined' && self.Math == Math ? self : Function('return this')();
if(typeof __g == 'number'){ __g = global; } // eslint-disable-line no-undef
});

var _core = createCommonjsModule(function (module) {
var core = module.exports = {version: '2.4.0'};
if(typeof __e == 'number'){ __e = core; } // eslint-disable-line no-undef
});

var _isObject = function(it){
  return typeof it === 'object' ? it !== null : typeof it === 'function';
};

var isObject = _isObject;
var _anObject = function(it){
  if(!isObject(it)){ throw TypeError(it + ' is not an object!'); }
  return it;
};

var _fails = function(exec){
  try {
    return !!exec();
  } catch(e){
    return true;
  }
};

// Thank's IE8 for his funny defineProperty
var _descriptors = !_fails(function(){
  return Object.defineProperty({}, 'a', {get: function(){ return 7; }}).a != 7;
});

var isObject$1 = _isObject;
var document$1 = _global.document;
var is = isObject$1(document$1) && isObject$1(document$1.createElement);
var _domCreate = function(it){
  return is ? document$1.createElement(it) : {};
};

var _ie8DomDefine = !_descriptors && !_fails(function(){
  return Object.defineProperty(_domCreate('div'), 'a', {get: function(){ return 7; }}).a != 7;
});

// 7.1.1 ToPrimitive(input [, PreferredType])
var isObject$2 = _isObject;
// instead of the ES6 spec version, we didn't implement @@toPrimitive case
// and the second argument - flag - preferred type is a string
var _toPrimitive = function(it, S){
  if(!isObject$2(it)){ return it; }
  var fn, val;
  if(S && typeof (fn = it.toString) == 'function' && !isObject$2(val = fn.call(it))){ return val; }
  if(typeof (fn = it.valueOf) == 'function' && !isObject$2(val = fn.call(it))){ return val; }
  if(!S && typeof (fn = it.toString) == 'function' && !isObject$2(val = fn.call(it))){ return val; }
  throw TypeError("Can't convert object to primitive value");
};

var anObject       = _anObject;
var IE8_DOM_DEFINE = _ie8DomDefine;
var toPrimitive    = _toPrimitive;
var dP$1             = Object.defineProperty;

var f = _descriptors ? Object.defineProperty : function defineProperty(O, P, Attributes){
  anObject(O);
  P = toPrimitive(P, true);
  anObject(Attributes);
  if(IE8_DOM_DEFINE){ try {
    return dP$1(O, P, Attributes);
  } catch(e){ /* empty */ } }
  if('get' in Attributes || 'set' in Attributes){ throw TypeError('Accessors not supported!'); }
  if('value' in Attributes){ O[P] = Attributes.value; }
  return O;
};

var _objectDp = {
	f: f
};

var _propertyDesc = function(bitmap, value){
  return {
    enumerable  : !(bitmap & 1),
    configurable: !(bitmap & 2),
    writable    : !(bitmap & 4),
    value       : value
  };
};

var dP         = _objectDp;
var createDesc = _propertyDesc;
var _hide = _descriptors ? function(object, key, value){
  return dP.f(object, key, createDesc(1, value));
} : function(object, key, value){
  object[key] = value;
  return object;
};

var hasOwnProperty = {}.hasOwnProperty;
var _has = function(it, key){
  return hasOwnProperty.call(it, key);
};

var id = 0;
var px = Math.random();
var _uid = function(key){
  return 'Symbol('.concat(key === undefined ? '' : key, ')_', (++id + px).toString(36));
};

var _redefine = createCommonjsModule(function (module) {
var global    = _global
  , hide      = _hide
  , has       = _has
  , SRC       = _uid('src')
  , TO_STRING = 'toString'
  , $toString = Function[TO_STRING]
  , TPL       = ('' + $toString).split(TO_STRING);

_core.inspectSource = function(it){
  return $toString.call(it);
};

(module.exports = function(O, key, val, safe){
  var isFunction = typeof val == 'function';
  if(isFunction){ has(val, 'name') || hide(val, 'name', key); }
  if(O[key] === val){ return; }
  if(isFunction){ has(val, SRC) || hide(val, SRC, O[key] ? '' + O[key] : TPL.join(String(key))); }
  if(O === global){
    O[key] = val;
  } else {
    if(!safe){
      delete O[key];
      hide(O, key, val);
    } else {
      if(O[key]){ O[key] = val; }
      else { hide(O, key, val); }
    }
  }
// add fake Function#toString for correct work wrapped methods / constructors with methods like LoDash isNative
})(Function.prototype, TO_STRING, function toString(){
  return typeof this == 'function' && this[SRC] || $toString.call(this);
});
});

var _aFunction = function(it){
  if(typeof it != 'function'){ throw TypeError(it + ' is not a function!'); }
  return it;
};

// optional / simple context binding
var aFunction = _aFunction;
var _ctx = function(fn, that, length){
  aFunction(fn);
  if(that === undefined){ return fn; }
  switch(length){
    case 1: return function(a){
      return fn.call(that, a);
    };
    case 2: return function(a, b){
      return fn.call(that, a, b);
    };
    case 3: return function(a, b, c){
      return fn.call(that, a, b, c);
    };
  }
  return function(/* ...args */){
    return fn.apply(that, arguments);
  };
};

var global$1    = _global;
var core      = _core;
var hide      = _hide;
var redefine  = _redefine;
var ctx       = _ctx;
var PROTOTYPE = 'prototype';

var $export$1 = function(type, name, source){
  var IS_FORCED = type & $export$1.F
    , IS_GLOBAL = type & $export$1.G
    , IS_STATIC = type & $export$1.S
    , IS_PROTO  = type & $export$1.P
    , IS_BIND   = type & $export$1.B
    , target    = IS_GLOBAL ? global$1 : IS_STATIC ? global$1[name] || (global$1[name] = {}) : (global$1[name] || {})[PROTOTYPE]
    , exports   = IS_GLOBAL ? core : core[name] || (core[name] = {})
    , expProto  = exports[PROTOTYPE] || (exports[PROTOTYPE] = {})
    , key, own, out, exp;
  if(IS_GLOBAL){ source = name; }
  for(key in source){
    // contains in native
    own = !IS_FORCED && target && target[key] !== undefined;
    // export native or passed
    out = (own ? target : source)[key];
    // bind timers to global for call from export context
    exp = IS_BIND && own ? ctx(out, global$1) : IS_PROTO && typeof out == 'function' ? ctx(Function.call, out) : out;
    // extend global
    if(target){ redefine(target, key, out, type & $export$1.U); }
    // export
    if(exports[key] != out){ hide(exports, key, exp); }
    if(IS_PROTO && expProto[key] != out){ expProto[key] = out; }
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

var _cof = function(it){
  return toString$1.call(it).slice(8, -1);
};

// fallback for non-array-like ES3 and non-enumerable old V8 strings
var cof = _cof;
var _iobject = Object('z').propertyIsEnumerable(0) ? Object : function(it){
  return cof(it) == 'String' ? it.split('') : Object(it);
};

// 7.2.1 RequireObjectCoercible(argument)
var _defined = function(it){
  if(it == undefined){ throw TypeError("Can't call method on  " + it); }
  return it;
};

// to indexed object, toObject with fallback for non-array-like ES3 strings
var IObject$1 = _iobject;
var defined = _defined;
var _toIobject = function(it){
  return IObject$1(defined(it));
};

// 7.1.4 ToInteger
var ceil  = Math.ceil;
var floor = Math.floor;
var _toInteger = function(it){
  return isNaN(it = +it) ? 0 : (it > 0 ? floor : ceil)(it);
};

// 7.1.15 ToLength
var toInteger = _toInteger;
var min       = Math.min;
var _toLength = function(it){
  return it > 0 ? min(toInteger(it), 0x1fffffffffffff) : 0; // pow(2, 53) - 1 == 9007199254740991
};

var toInteger$1 = _toInteger;
var max       = Math.max;
var min$1       = Math.min;
var _toIndex = function(index, length){
  index = toInteger$1(index);
  return index < 0 ? max(index + length, 0) : min$1(index, length);
};

// false -> Array#indexOf
// true  -> Array#includes
var toIObject$1 = _toIobject;
var toLength  = _toLength;
var toIndex   = _toIndex;
var _arrayIncludes = function(IS_INCLUDES){
  return function($this, el, fromIndex){
    var O      = toIObject$1($this)
      , length = toLength(O.length)
      , index  = toIndex(fromIndex, length)
      , value;
    // Array#includes uses SameValueZero equality algorithm
    if(IS_INCLUDES && el != el){ while(length > index){
      value = O[index++];
      if(value != value){ return true; }
    // Array#toIndex ignores holes, Array#includes - not
    } } else { for(;length > index; index++){ if(IS_INCLUDES || index in O){
      if(O[index] === el){ return IS_INCLUDES || index || 0; }
    } } } return !IS_INCLUDES && -1;
  };
};

var global$2 = _global;
var SHARED = '__core-js_shared__';
var store  = global$2[SHARED] || (global$2[SHARED] = {});
var _shared = function(key){
  return store[key] || (store[key] = {});
};

var shared = _shared('keys');
var uid    = _uid;
var _sharedKey = function(key){
  return shared[key] || (shared[key] = uid(key));
};

var has          = _has;
var toIObject    = _toIobject;
var arrayIndexOf = _arrayIncludes(false);
var IE_PROTO     = _sharedKey('IE_PROTO');

var _objectKeysInternal = function(object, names){
  var O      = toIObject(object)
    , i      = 0
    , result = []
    , key;
  for(key in O){ if(key != IE_PROTO){ has(O, key) && result.push(key); } }
  // Don't enum bug & hidden keys
  while(names.length > i){ if(has(O, key = names[i++])){
    ~arrayIndexOf(result, key) || result.push(key);
  } }
  return result;
};

// IE 8- don't enum bug keys
var _enumBugKeys = (
  'constructor,hasOwnProperty,isPrototypeOf,propertyIsEnumerable,toLocaleString,toString,valueOf'
).split(',');

// 19.1.2.14 / 15.2.3.14 Object.keys(O)
var $keys       = _objectKeysInternal;
var enumBugKeys = _enumBugKeys;

var _objectKeys = Object.keys || function keys(O){
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
var _toObject = function(it){
  return Object(defined$1(it));
};

// 19.1.2.1 Object.assign(target, source, ...)
var getKeys  = _objectKeys;
var gOPS     = _objectGops;
var pIE      = _objectPie;
var toObject = _toObject;
var IObject  = _iobject;
var $assign  = Object.assign;

// should work with symbols and should have deterministic property order (V8 bug)
var _objectAssign = !$assign || _fails(function(){
  var A = {}
    , B = {}
    , S = Symbol()
    , K = 'abcdefghijklmnopqrst';
  A[S] = 7;
  K.split('').forEach(function(k){ B[k] = k; });
  return $assign({}, A)[S] != 7 || Object.keys($assign({}, B)).join('') != K;
}) ? function assign(target, source){
  var arguments$1 = arguments;
 // eslint-disable-line no-unused-vars
  var T     = toObject(target)
    , aLen  = arguments.length
    , index = 1
    , getSymbols = gOPS.f
    , isEnum     = pIE.f;
  while(aLen > index){
    var S      = IObject(arguments$1[index++])
      , keys   = getSymbols ? getKeys(S).concat(getSymbols(S)) : getKeys(S)
      , length = keys.length
      , j      = 0
      , key;
    while(length > j){ if(isEnum.call(S, key = keys[j++])){ T[key] = S[key]; } }
  } return T;
} : $assign;

// 19.1.3.1 Object.assign(target, source)
var $export = _export;

$export($export.S + $export.F, 'Object', {assign: _objectAssign});

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
var WXEnvironment = ref.WXEnvironment;

/* istanbul ignore next */
if (WXEnvironment && WXEnvironment.platform === 'iOS') {
  commonjsGlobal.Promise = undefined;
}

var _wks = createCommonjsModule(function (module) {
var store      = _shared('wks')
  , uid        = _uid
  , Symbol     = _global.Symbol
  , USE_SYMBOL = typeof Symbol == 'function';

var $exports = module.exports = function(name){
  return store[name] || (store[name] =
    USE_SYMBOL && Symbol[name] || (USE_SYMBOL ? Symbol : uid)('Symbol.' + name));
};

$exports.store = store;
});

// getting tag from 19.1.3.6 Object.prototype.toString()
var cof$1 = _cof;
var TAG = _wks('toStringTag');
var ARG = cof$1(function(){ return arguments; }()) == 'Arguments';

// fallback for IE11 Script Access Denied error
var tryGet = function(it, key){
  try {
    return it[key];
  } catch(e){ /* empty */ }
};

var _classof = function(it){
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
var test    = {};
test[_wks('toStringTag')] = 'z';
if(test + '' != '[object z]'){
  _redefine(Object.prototype, 'toString', function toString(){
    return '[object ' + classof(this) + ']';
  }, true);
}

var toInteger$2 = _toInteger;
var defined$2   = _defined;
// true  -> String#at
// false -> String#codePointAt
var _stringAt = function(TO_STRING){
  return function(that, pos){
    var s = String(defined$2(that))
      , i = toInteger$2(pos)
      , l = s.length
      , a, b;
    if(i < 0 || i >= l){ return TO_STRING ? '' : undefined; }
    a = s.charCodeAt(i);
    return a < 0xd800 || a > 0xdbff || i + 1 === l || (b = s.charCodeAt(i + 1)) < 0xdc00 || b > 0xdfff
      ? TO_STRING ? s.charAt(i) : a
      : TO_STRING ? s.slice(i, i + 2) : (a - 0xd800 << 10) + (b - 0xdc00) + 0x10000;
  };
};

var _library = false;

var _iterators = {};

var dP$2       = _objectDp;
var anObject$2 = _anObject;
var getKeys$1  = _objectKeys;

var _objectDps = _descriptors ? Object.defineProperties : function defineProperties(O, Properties){
  anObject$2(O);
  var keys   = getKeys$1(Properties)
    , length = keys.length
    , i = 0
    , P;
  while(length > i){ dP$2.f(O, P = keys[i++], Properties[P]); }
  return O;
};

var _html = _global.document && document.documentElement;

// 19.1.2.2 / 15.2.3.5 Object.create(O [, Properties])
var anObject$1    = _anObject;
var dPs         = _objectDps;
var enumBugKeys$1 = _enumBugKeys;
var IE_PROTO$1    = _sharedKey('IE_PROTO');
var Empty       = function(){ /* empty */ };
var PROTOTYPE$1   = 'prototype';

// Create object with fake `null` prototype: use iframe Object with cleared prototype
var createDict = function(){
  // Thrash, waste and sodomy: IE GC bug
  var iframe = _domCreate('iframe')
    , i      = enumBugKeys$1.length
    , lt     = '<'
    , gt     = '>'
    , iframeDocument;
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
  while(i--){ delete createDict[PROTOTYPE$1][enumBugKeys$1[i]]; }
  return createDict();
};

var _objectCreate = Object.create || function create(O, Properties){
  var result;
  if(O !== null){
    Empty[PROTOTYPE$1] = anObject$1(O);
    result = new Empty;
    Empty[PROTOTYPE$1] = null;
    // add "__proto__" for Object.getPrototypeOf polyfill
    result[IE_PROTO$1] = O;
  } else { result = createDict(); }
  return Properties === undefined ? result : dPs(result, Properties);
};

var def = _objectDp.f;
var has$2 = _has;
var TAG$1 = _wks('toStringTag');

var _setToStringTag = function(it, tag, stat){
  if(it && !has$2(it = stat ? it : it.prototype, TAG$1)){ def(it, TAG$1, {configurable: true, value: tag}); }
};

var create$1         = _objectCreate;
var descriptor     = _propertyDesc;
var setToStringTag$1 = _setToStringTag;
var IteratorPrototype = {};

// 25.1.2.1.1 %IteratorPrototype%[@@iterator]()
_hide(IteratorPrototype, _wks('iterator'), function(){ return this; });

var _iterCreate = function(Constructor, NAME, next){
  Constructor.prototype = create$1(IteratorPrototype, {next: descriptor(1, next)});
  setToStringTag$1(Constructor, NAME + ' Iterator');
};

// 19.1.2.9 / 15.2.3.2 Object.getPrototypeOf(O)
var has$3         = _has;
var toObject$1    = _toObject;
var IE_PROTO$2    = _sharedKey('IE_PROTO');
var ObjectProto = Object.prototype;

var _objectGpo = Object.getPrototypeOf || function(O){
  O = toObject$1(O);
  if(has$3(O, IE_PROTO$2)){ return O[IE_PROTO$2]; }
  if(typeof O.constructor == 'function' && O instanceof O.constructor){
    return O.constructor.prototype;
  } return O instanceof Object ? ObjectProto : null;
};

var LIBRARY        = _library;
var $export$2        = _export;
var redefine$1       = _redefine;
var hide$1           = _hide;
var has$1            = _has;
var Iterators      = _iterators;
var $iterCreate    = _iterCreate;
var setToStringTag = _setToStringTag;
var getPrototypeOf = _objectGpo;
var ITERATOR       = _wks('iterator');
var BUGGY          = !([].keys && 'next' in [].keys());
var FF_ITERATOR    = '@@iterator';
var KEYS           = 'keys';
var VALUES         = 'values';

var returnThis = function(){ return this; };

var _iterDefine = function(Base, NAME, Constructor, next, DEFAULT, IS_SET, FORCED){
  $iterCreate(Constructor, NAME, next);
  var getMethod = function(kind){
    if(!BUGGY && kind in proto){ return proto[kind]; }
    switch(kind){
      case KEYS: return function keys(){ return new Constructor(this, kind); };
      case VALUES: return function values(){ return new Constructor(this, kind); };
    } return function entries(){ return new Constructor(this, kind); };
  };
  var TAG        = NAME + ' Iterator'
    , DEF_VALUES = DEFAULT == VALUES
    , VALUES_BUG = false
    , proto      = Base.prototype
    , $native    = proto[ITERATOR] || proto[FF_ITERATOR] || DEFAULT && proto[DEFAULT]
    , $default   = $native || getMethod(DEFAULT)
    , $entries   = DEFAULT ? !DEF_VALUES ? $default : getMethod('entries') : undefined
    , $anyNative = NAME == 'Array' ? proto.entries || $native : $native
    , methods, key, IteratorPrototype;
  // Fix native
  if($anyNative){
    IteratorPrototype = getPrototypeOf($anyNative.call(new Base));
    if(IteratorPrototype !== Object.prototype){
      // Set @@toStringTag to native iterators
      setToStringTag(IteratorPrototype, TAG, true);
      // fix for some old engines
      if(!LIBRARY && !has$1(IteratorPrototype, ITERATOR)){ hide$1(IteratorPrototype, ITERATOR, returnThis); }
    }
  }
  // fix Array#{values, @@iterator}.name in V8 / FF
  if(DEF_VALUES && $native && $native.name !== VALUES){
    VALUES_BUG = true;
    $default = function values(){ return $native.call(this); };
  }
  // Define iterator
  if((!LIBRARY || FORCED) && (BUGGY || VALUES_BUG || !proto[ITERATOR])){
    hide$1(proto, ITERATOR, $default);
  }
  // Plug for library
  Iterators[NAME] = $default;
  Iterators[TAG]  = returnThis;
  if(DEFAULT){
    methods = {
      values:  DEF_VALUES ? $default : getMethod(VALUES),
      keys:    IS_SET     ? $default : getMethod(KEYS),
      entries: $entries
    };
    if(FORCED){ for(key in methods){
      if(!(key in proto)){ redefine$1(proto, key, methods[key]); }
    } } else { $export$2($export$2.P + $export$2.F * (BUGGY || VALUES_BUG), NAME, methods); }
  }
  return methods;
};

var $at  = _stringAt(true);

// 21.1.3.27 String.prototype[@@iterator]()
_iterDefine(String, 'String', function(iterated){
  this._t = String(iterated); // target
  this._i = 0;                // next index
// 21.1.5.2.1 %StringIteratorPrototype%.next()
}, function(){
  var O     = this._t
    , index = this._i
    , point;
  if(index >= O.length){ return {value: undefined, done: true}; }
  point = $at(O, index);
  this._i += point.length;
  return {value: point, done: false};
});

// 22.1.3.31 Array.prototype[@@unscopables]
var UNSCOPABLES = _wks('unscopables');
var ArrayProto  = Array.prototype;
if(ArrayProto[UNSCOPABLES] == undefined){ _hide(ArrayProto, UNSCOPABLES, {}); }
var _addToUnscopables = function(key){
  ArrayProto[UNSCOPABLES][key] = true;
};

var _iterStep = function(done, value){
  return {value: value, done: !!done};
};

var addToUnscopables = _addToUnscopables;
var step             = _iterStep;
var Iterators$2        = _iterators;
var toIObject$2        = _toIobject;

// 22.1.3.4 Array.prototype.entries()
// 22.1.3.13 Array.prototype.keys()
// 22.1.3.29 Array.prototype.values()
// 22.1.3.30 Array.prototype[@@iterator]()
var es6_array_iterator = _iterDefine(Array, 'Array', function(iterated, kind){
  this._t = toIObject$2(iterated); // target
  this._i = 0;                   // next index
  this._k = kind;                // kind
// 22.1.5.2.1 %ArrayIteratorPrototype%.next()
}, function(){
  var O     = this._t
    , kind  = this._k
    , index = this._i++;
  if(!O || index >= O.length){
    this._t = undefined;
    return step(1);
  }
  if(kind == 'keys'  ){ return step(0, index); }
  if(kind == 'values'){ return step(0, O[index]); }
  return step(0, [index, O[index]]);
}, 'values');

// argumentsList[@@iterator] is %ArrayProto_values% (9.4.4.6, 9.4.4.7)
Iterators$2.Arguments = Iterators$2.Array;

addToUnscopables('keys');
addToUnscopables('values');
addToUnscopables('entries');

var $iterators    = es6_array_iterator;
var redefine$2      = _redefine;
var global$3        = _global;
var hide$2          = _hide;
var Iterators$1     = _iterators;
var wks           = _wks;
var ITERATOR$1      = wks('iterator');
var TO_STRING_TAG = wks('toStringTag');
var ArrayValues   = Iterators$1.Array;

for(var collections = ['NodeList', 'DOMTokenList', 'MediaList', 'StyleSheetList', 'CSSRuleList'], i = 0; i < 5; i++){
  var NAME       = collections[i]
    , Collection = global$3[NAME]
    , proto      = Collection && Collection.prototype
    , key;
  if(proto){
    if(!proto[ITERATOR$1]){ hide$2(proto, ITERATOR$1, ArrayValues); }
    if(!proto[TO_STRING_TAG]){ hide$2(proto, TO_STRING_TAG, NAME); }
    Iterators$1[NAME] = ArrayValues;
    for(key in $iterators){ if(!proto[key]){ redefine$2(proto, key, $iterators[key], true); } }
  }
}

var _anInstance = function(it, Constructor, name, forbiddenField){
  if(!(it instanceof Constructor) || (forbiddenField !== undefined && forbiddenField in it)){
    throw TypeError(name + ': incorrect invocation!');
  } return it;
};

// call something on iterator step with safe closing on error
var anObject$3 = _anObject;
var _iterCall = function(iterator, fn, value, entries){
  try {
    return entries ? fn(anObject$3(value)[0], value[1]) : fn(value);
  // 7.4.6 IteratorClose(iterator, completion)
  } catch(e){
    var ret = iterator['return'];
    if(ret !== undefined){ anObject$3(ret.call(iterator)); }
    throw e;
  }
};

// check on default Array iterator
var Iterators$3  = _iterators;
var ITERATOR$2   = _wks('iterator');
var ArrayProto$1 = Array.prototype;

var _isArrayIter = function(it){
  return it !== undefined && (Iterators$3.Array === it || ArrayProto$1[ITERATOR$2] === it);
};

var classof$2   = _classof;
var ITERATOR$3  = _wks('iterator');
var Iterators$4 = _iterators;
var core_getIteratorMethod = _core.getIteratorMethod = function(it){
  if(it != undefined){ return it[ITERATOR$3]
    || it['@@iterator']
    || Iterators$4[classof$2(it)]; }
};

var _forOf = createCommonjsModule(function (module) {
var ctx         = _ctx
  , call        = _iterCall
  , isArrayIter = _isArrayIter
  , anObject    = _anObject
  , toLength    = _toLength
  , getIterFn   = core_getIteratorMethod
  , BREAK       = {}
  , RETURN      = {};
var exports = module.exports = function(iterable, entries, fn, that, ITERATOR){
  var iterFn = ITERATOR ? function(){ return iterable; } : getIterFn(iterable)
    , f      = ctx(fn, that, entries ? 2 : 1)
    , index  = 0
    , length, step, iterator, result;
  if(typeof iterFn != 'function'){ throw TypeError(iterable + ' is not iterable!'); }
  // fast case for arrays with default iterator
  if(isArrayIter(iterFn)){ for(length = toLength(iterable.length); length > index; index++){
    result = entries ? f(anObject(step = iterable[index])[0], step[1]) : f(iterable[index]);
    if(result === BREAK || result === RETURN){ return result; }
  } } else { for(iterator = iterFn.call(iterable); !(step = iterator.next()).done; ){
    result = call(iterator, f, step.value, entries);
    if(result === BREAK || result === RETURN){ return result; }
  } }
};
exports.BREAK  = BREAK;
exports.RETURN = RETURN;
});

// 7.3.20 SpeciesConstructor(O, defaultConstructor)
var anObject$4  = _anObject;
var aFunction$2 = _aFunction;
var SPECIES   = _wks('species');
var _speciesConstructor = function(O, D){
  var C = anObject$4(O).constructor, S;
  return C === undefined || (S = anObject$4(C)[SPECIES]) == undefined ? D : aFunction$2(S);
};

// fast apply, http://jsperf.lnkit.com/fast-apply/5
var _invoke = function(fn, args, that){
  var un = that === undefined;
  switch(args.length){
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
  } return              fn.apply(that, args);
};

var ctx$2                = _ctx;
var invoke             = _invoke;
var html               = _html;
var cel                = _domCreate;
var global$5             = _global;
var process$1            = global$5.process;
var setTask            = global$5.setImmediate;
var clearTask          = global$5.clearImmediate;
var MessageChannel     = global$5.MessageChannel;
var counter            = 0;
var queue              = {};
var ONREADYSTATECHANGE = 'onreadystatechange';
var defer;
var channel;
var port;
var run = function(){
  var id = +this;
  if(queue.hasOwnProperty(id)){
    var fn = queue[id];
    delete queue[id];
    fn();
  }
};
var listener = function(event){
  run.call(event.data);
};
// Node.js 0.9+ & IE10+ has setImmediate, otherwise:
if(!setTask || !clearTask){
  setTask = function setImmediate(fn){
    var arguments$1 = arguments;

    var args = [], i = 1;
    while(arguments.length > i){ args.push(arguments$1[i++]); }
    queue[++counter] = function(){
      invoke(typeof fn == 'function' ? fn : Function(fn), args);
    };
    defer(counter);
    return counter;
  };
  clearTask = function clearImmediate(id){
    delete queue[id];
  };
  // Node.js 0.8-
  if(_cof(process$1) == 'process'){
    defer = function(id){
      process$1.nextTick(ctx$2(run, id, 1));
    };
  // Browsers with MessageChannel, includes WebWorkers
  } else if(MessageChannel){
    channel = new MessageChannel;
    port    = channel.port2;
    channel.port1.onmessage = listener;
    defer = ctx$2(port.postMessage, port, 1);
  // Browsers with postMessage, skip WebWorkers
  // IE8 has postMessage, but it's sync & typeof its postMessage is 'object'
  } else if(global$5.addEventListener && typeof postMessage == 'function' && !global$5.importScripts){
    defer = function(id){
      global$5.postMessage(id + '', '*');
    };
    global$5.addEventListener('message', listener, false);
  // IE8-
  } else if(ONREADYSTATECHANGE in cel('script')){
    defer = function(id){
      html.appendChild(cel('script'))[ONREADYSTATECHANGE] = function(){
        html.removeChild(this);
        run.call(id);
      };
    };
  // Rest old browsers
  } else {
    defer = function(id){
      setTimeout(ctx$2(run, id, 1), 0);
    };
  }
}
var _task = {
  set:   setTask,
  clear: clearTask
};

var global$6    = _global;
var macrotask = _task.set;
var Observer  = global$6.MutationObserver || global$6.WebKitMutationObserver;
var process$2   = global$6.process;
var Promise$1   = global$6.Promise;
var isNode$1    = _cof(process$2) == 'process';

var _microtask = function(){
  var head, last, notify;

  var flush = function(){
    var parent, fn;
    if(isNode$1 && (parent = process$2.domain)){ parent.exit(); }
    while(head){
      fn   = head.fn;
      head = head.next;
      try {
        fn();
      } catch(e){
        if(head){ notify(); }
        else { last = undefined; }
        throw e;
      }
    } last = undefined;
    if(parent){ parent.enter(); }
  };

  // Node.js
  if(isNode$1){
    notify = function(){
      process$2.nextTick(flush);
    };
  // browsers with MutationObserver
  } else if(Observer){
    var toggle = true
      , node   = document.createTextNode('');
    new Observer(flush).observe(node, {characterData: true}); // eslint-disable-line no-new
    notify = function(){
      node.data = toggle = !toggle;
    };
  // environments with maybe non-completely correct, but existent Promise
  } else if(Promise$1 && Promise$1.resolve){
    var promise = Promise$1.resolve();
    notify = function(){
      promise.then(flush);
    };
  // for other environments - macrotask based on:
  // - setImmediate
  // - MessageChannel
  // - window.postMessag
  // - onreadystatechange
  // - setTimeout
  } else {
    notify = function(){
      // strange IE + webpack dev server bug - use .call(global)
      macrotask.call(global$6, flush);
    };
  }

  return function(fn){
    var task = {fn: fn, next: undefined};
    if(last){ last.next = task; }
    if(!head){
      head = task;
      notify();
    } last = task;
  };
};

var redefine$3 = _redefine;
var _redefineAll = function(target, src, safe){
  for(var key in src){ redefine$3(target, key, src[key], safe); }
  return target;
};

var global$7      = _global;
var dP$3          = _objectDp;
var DESCRIPTORS = _descriptors;
var SPECIES$1     = _wks('species');

var _setSpecies = function(KEY){
  var C = global$7[KEY];
  if(DESCRIPTORS && C && !C[SPECIES$1]){ dP$3.f(C, SPECIES$1, {
    configurable: true,
    get: function(){ return this; }
  }); }
};

var ITERATOR$4     = _wks('iterator');
var SAFE_CLOSING = false;

try {
  var riter = [7][ITERATOR$4]();
  riter['return'] = function(){ SAFE_CLOSING = true; };
  Array.from(riter, function(){ throw 2; });
} catch(e){ /* empty */ }

var _iterDetect = function(exec, skipClosing){
  if(!skipClosing && !SAFE_CLOSING){ return false; }
  var safe = false;
  try {
    var arr  = [7]
      , iter = arr[ITERATOR$4]();
    iter.next = function(){ return {done: safe = true}; };
    arr[ITERATOR$4] = function(){ return iter; };
    exec(arr);
  } catch(e){ /* empty */ }
  return safe;
};

var LIBRARY$1            = _library;
var global$4             = _global;
var ctx$1                = _ctx;
var classof$1            = _classof;
var $export$3            = _export;
var isObject$3           = _isObject;
var aFunction$1          = _aFunction;
var anInstance         = _anInstance;
var forOf              = _forOf;
var speciesConstructor = _speciesConstructor;
var task               = _task.set;
var microtask          = _microtask();
var PROMISE            = 'Promise';
var TypeError$1          = global$4.TypeError;
var process            = global$4.process;
var $Promise           = global$4[PROMISE];
var process            = global$4.process;
var isNode             = classof$1(process) == 'process';
var empty              = function(){ /* empty */ };
var Internal;
var GenericPromiseCapability;
var Wrapper;

var USE_NATIVE = !!function(){
  try {
    // correct subclassing with @@species support
    var promise     = $Promise.resolve(1)
      , FakePromise = (promise.constructor = {})[_wks('species')] = function(exec){ exec(empty, empty); };
    // unhandled rejections tracking support, NodeJS Promise without it fails @@species test
    return (isNode || typeof PromiseRejectionEvent == 'function') && promise.then(empty) instanceof FakePromise;
  } catch(e){ /* empty */ }
}();

// helpers
var sameConstructor = function(a, b){
  // with library wrapper special case
  return a === b || a === $Promise && b === Wrapper;
};
var isThenable = function(it){
  var then;
  return isObject$3(it) && typeof (then = it.then) == 'function' ? then : false;
};
var newPromiseCapability = function(C){
  return sameConstructor($Promise, C)
    ? new PromiseCapability(C)
    : new GenericPromiseCapability(C);
};
var PromiseCapability = GenericPromiseCapability = function(C){
  var resolve, reject;
  this.promise = new C(function($$resolve, $$reject){
    if(resolve !== undefined || reject !== undefined){ throw TypeError$1('Bad Promise constructor'); }
    resolve = $$resolve;
    reject  = $$reject;
  });
  this.resolve = aFunction$1(resolve);
  this.reject  = aFunction$1(reject);
};
var perform = function(exec){
  try {
    exec();
  } catch(e){
    return {error: e};
  }
};
var notify = function(promise, isReject){
  if(promise._n){ return; }
  promise._n = true;
  var chain = promise._c;
  microtask(function(){
    var value = promise._v
      , ok    = promise._s == 1
      , i     = 0;
    var run = function(reaction){
      var handler = ok ? reaction.ok : reaction.fail
        , resolve = reaction.resolve
        , reject  = reaction.reject
        , domain  = reaction.domain
        , result, then;
      try {
        if(handler){
          if(!ok){
            if(promise._h == 2){ onHandleUnhandled(promise); }
            promise._h = 1;
          }
          if(handler === true){ result = value; }
          else {
            if(domain){ domain.enter(); }
            result = handler(value);
            if(domain){ domain.exit(); }
          }
          if(result === reaction.promise){
            reject(TypeError$1('Promise-chain cycle'));
          } else if(then = isThenable(result)){
            then.call(result, resolve, reject);
          } else { resolve(result); }
        } else { reject(value); }
      } catch(e){
        reject(e);
      }
    };
    while(chain.length > i){ run(chain[i++]); } // variable length - can't use forEach
    promise._c = [];
    promise._n = false;
    if(isReject && !promise._h){ onUnhandled(promise); }
  });
};
var onUnhandled = function(promise){
  task.call(global$4, function(){
    var value = promise._v
      , abrupt, handler, console;
    if(isUnhandled(promise)){
      abrupt = perform(function(){
        if(isNode){
          process.emit('unhandledRejection', value, promise);
        } else if(handler = global$4.onunhandledrejection){
          handler({promise: promise, reason: value});
        } else if((console = global$4.console) && console.error){
          console.error('Unhandled promise rejection', value);
        }
      });
      // Browsers should not trigger `rejectionHandled` event if it was handled here, NodeJS - should
      promise._h = isNode || isUnhandled(promise) ? 2 : 1;
    } promise._a = undefined;
    if(abrupt){ throw abrupt.error; }
  });
};
var isUnhandled = function(promise){
  if(promise._h == 1){ return false; }
  var chain = promise._a || promise._c
    , i     = 0
    , reaction;
  while(chain.length > i){
    reaction = chain[i++];
    if(reaction.fail || !isUnhandled(reaction.promise)){ return false; }
  } return true;
};
var onHandleUnhandled = function(promise){
  task.call(global$4, function(){
    var handler;
    if(isNode){
      process.emit('rejectionHandled', promise);
    } else if(handler = global$4.onrejectionhandled){
      handler({promise: promise, reason: promise._v});
    }
  });
};
var $reject = function(value){
  var promise = this;
  if(promise._d){ return; }
  promise._d = true;
  promise = promise._w || promise; // unwrap
  promise._v = value;
  promise._s = 2;
  if(!promise._a){ promise._a = promise._c.slice(); }
  notify(promise, true);
};
var $resolve = function(value){
  var promise = this
    , then;
  if(promise._d){ return; }
  promise._d = true;
  promise = promise._w || promise; // unwrap
  try {
    if(promise === value){ throw TypeError$1("Promise can't be resolved itself"); }
    if(then = isThenable(value)){
      microtask(function(){
        var wrapper = {_w: promise, _d: false}; // wrap
        try {
          then.call(value, ctx$1($resolve, wrapper, 1), ctx$1($reject, wrapper, 1));
        } catch(e){
          $reject.call(wrapper, e);
        }
      });
    } else {
      promise._v = value;
      promise._s = 1;
      notify(promise, false);
    }
  } catch(e){
    $reject.call({_w: promise, _d: false}, e); // wrap
  }
};

// constructor polyfill
if(!USE_NATIVE){
  // 25.4.3.1 Promise(executor)
  $Promise = function Promise(executor){
    anInstance(this, $Promise, PROMISE, '_h');
    aFunction$1(executor);
    Internal.call(this);
    try {
      executor(ctx$1($resolve, this, 1), ctx$1($reject, this, 1));
    } catch(err){
      $reject.call(this, err);
    }
  };
  Internal = function Promise(executor){
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
    then: function then(onFulfilled, onRejected){
      var reaction    = newPromiseCapability(speciesConstructor(this, $Promise));
      reaction.ok     = typeof onFulfilled == 'function' ? onFulfilled : true;
      reaction.fail   = typeof onRejected == 'function' && onRejected;
      reaction.domain = isNode ? process.domain : undefined;
      this._c.push(reaction);
      if(this._a){ this._a.push(reaction); }
      if(this._s){ notify(this, false); }
      return reaction.promise;
    },
    // 25.4.5.1 Promise.prototype.catch(onRejected)
    'catch': function(onRejected){
      return this.then(undefined, onRejected);
    }
  });
  PromiseCapability = function(){
    var promise  = new Internal;
    this.promise = promise;
    this.resolve = ctx$1($resolve, promise, 1);
    this.reject  = ctx$1($reject, promise, 1);
  };
}

$export$3($export$3.G + $export$3.W + $export$3.F * !USE_NATIVE, {Promise: $Promise});
_setToStringTag($Promise, PROMISE);
_setSpecies(PROMISE);
Wrapper = _core[PROMISE];

// statics
$export$3($export$3.S + $export$3.F * !USE_NATIVE, PROMISE, {
  // 25.4.4.5 Promise.reject(r)
  reject: function reject(r){
    var capability = newPromiseCapability(this)
      , $$reject   = capability.reject;
    $$reject(r);
    return capability.promise;
  }
});
$export$3($export$3.S + $export$3.F * (LIBRARY$1 || !USE_NATIVE), PROMISE, {
  // 25.4.4.6 Promise.resolve(x)
  resolve: function resolve(x){
    // instanceof instead of internal slot check because we should fix it without replacement native Promise core
    if(x instanceof $Promise && sameConstructor(x.constructor, this)){ return x; }
    var capability = newPromiseCapability(this)
      , $$resolve  = capability.resolve;
    $$resolve(x);
    return capability.promise;
  }
});
$export$3($export$3.S + $export$3.F * !(USE_NATIVE && _iterDetect(function(iter){
  $Promise.all(iter)['catch'](empty);
})), PROMISE, {
  // 25.4.4.1 Promise.all(iterable)
  all: function all(iterable){
    var C          = this
      , capability = newPromiseCapability(C)
      , resolve    = capability.resolve
      , reject     = capability.reject;
    var abrupt = perform(function(){
      var values    = []
        , index     = 0
        , remaining = 1;
      forOf(iterable, false, function(promise){
        var $index        = index++
          , alreadyCalled = false;
        values.push(undefined);
        remaining++;
        C.resolve(promise).then(function(value){
          if(alreadyCalled){ return; }
          alreadyCalled  = true;
          values[$index] = value;
          --remaining || resolve(values);
        }, reject);
      });
      --remaining || resolve(values);
    });
    if(abrupt){ reject(abrupt.error); }
    return capability.promise;
  },
  // 25.4.4.4 Promise.race(iterable)
  race: function race(iterable){
    var C          = this
      , capability = newPromiseCapability(C)
      , reject     = capability.reject;
    var abrupt = perform(function(){
      forOf(iterable, false, function(promise){
        C.resolve(promise).then(capability.resolve, reject);
      });
    });
    if(abrupt){ reject(abrupt.error); }
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
  this.callbacks = [];
};
CallbackManager.prototype.add = function add (callback) {
  this.lastCallbackId++;
  this.callbacks[this.lastCallbackId] = callback;
  return this.lastCallbackId
};
CallbackManager.prototype.remove = function remove (callbackId) {
  var callback = this.callbacks[callbackId];
  this.callbacks[callbackId] = undefined;
  return callback
};
CallbackManager.prototype.consume = function consume (callbackId, data, ifKeepAlive) {
  var callback = this.callbacks[callbackId];
  if (typeof ifKeepAlive === 'undefined' || ifKeepAlive === false) {
    this.callbacks[callbackId] = undefined;
  }
  if (typeof callback === 'function') {
    return callback(data)
  }
  return new Error(("invalid callback id \"" + callbackId + "\""))
};
CallbackManager.prototype.close = function close () {
  this.callbacks = this.callbacks.map(function (cb) { return undefined; });
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
 * Get a unique id.
 */
var nextNodeRef = 1;
function uniqueId () {
  return (nextNodeRef++).toString()
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

/**
 * @fileOverview
 * Virtual-DOM Node. It's the supper class of Element and Comment.
 */

function Node () {
  this.nodeId = uniqueId();
  this.ref = this.nodeId;
  this.children = [];
  this.pureChildren = [];
  this.parentNode = null;
  this.nextSibling = null;
  this.previousSibling = null;
}

/**
 * Destroy current node, and remove itself form nodeMap.
 */
Node.prototype.destroy = function () {
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
var Element$1;

function setElement (El) {
  Element$1 = El;
}

/**
 * A map which stores all type of elements.
 * @type {Object}
 */
var elementTypes = {};

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
  var XElement = function (props) {
    Element$1.call(this, type, props, true);
  };

  // Init prototype.
  XElement.prototype = Object.create(Element$1.prototype);
  Object.defineProperty(XElement.prototype, 'constructor', {
    configurable: false,
    enumerable: false,
    writable: false,
    value: Element$1
  });

  // Add methods to prototype.
  methods.forEach(function (methodName) {
    XElement.prototype[methodName] = function () {
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
  elementTypes[type] = XElement;
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

/**
 * @fileOverview
 * Virtual-DOM Element.
 */

var DEFAULT_TAG_NAME = 'div';
var BUBBLE_EVENTS = ['click', 'longpress', 'touchstart', 'touchmove', 'touchend', 'panstart', 'panmove', 'panend', 'horizontalpan', 'verticalpan', 'swipe'];

function Element (type, props, isExtended) {
  if ( type === void 0 ) type = DEFAULT_TAG_NAME;

  var XElement = elementTypes[type];
  if (XElement && !isExtended) {
    return new XElement(props)
  }
  props = props || {};
  this.nodeType = 1;
  this.nodeId = uniqueId();
  this.ref = this.nodeId;
  this.type = type;
  this.attr = props.attr || {};
  this.style = props.style || {};
  this.classStyle = props.classStyle || {};
  this.event = {};
  this.children = [];
  this.pureChildren = [];
}

Element.prototype = Object.create(Node.prototype);
Element.prototype.constructor = Element;

function registerNode (docId, node) {
  var doc = getDoc(docId);
  doc.nodeMap[node.nodeId] = node;
}

setElement(Element);

Object.assign(Element.prototype, {
  /**
   * Append a child node.
   * @param {object} node
   * @return {undefined | number} the signal sent by native
   */
  appendChild: function appendChild (node) {
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
  },

  /**
   * Insert a node before specified node.
   * @param {object} node
   * @param {object} before
   * @return {undefined | number} the signal sent by native
   */
  insertBefore: function insertBefore (node, before) {
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
  },

  /**
   * Insert a node after specified node.
   * @param {object} node
   * @param {object} after
   * @return {undefined | number} the signal sent by native
   */
  insertAfter: function insertAfter (node, after) {
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
  },

  /**
   * Remove a child node, and decide whether it should be destroyed.
   * @param {object} node
   * @param {boolean} preserved
   */
  removeChild: function removeChild (node, preserved) {
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
  },

  /**
   * Clear all child nodes.
   */
  clear: function clear () {
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
  },

  /**
   * Set an attribute, and decide whether the task should be send to native.
   * @param {string} key
   * @param {string | number} value
   * @param {boolean} silent
   */
  setAttr: function setAttr (key, value, silent) {
    if (this.attr[key] === value && silent !== false) {
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
  },

  /**
   * Set a style property, and decide whether the task should be send to native.
   * @param {string} key
   * @param {string | number} value
   * @param {boolean} silent
   */
  setStyle: function setStyle (key, value, silent) {
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
  },

  /**
   * Set style properties from class.
   * @param {object} classStyle
   */
  setClassStyle: function setClassStyle (classStyle) {
    var this$1 = this;

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
  },

  /**
   * Add an event handler.
   * @param {string} event type
   * @param {function} event handler
   */
  addEvent: function addEvent (type, handler) {
    if (!this.event[type]) {
      this.event[type] = handler;
      var taskCenter = getTaskCenter(this.docId);
      if (taskCenter) {
        taskCenter.send(
          'dom',
          { action: 'addEvent' },
          [this.ref, type]
        );
      }
    }
  },

  /**
   * Remove an event handler.
   * @param {string} event type
   */
  removeEvent: function removeEvent (type) {
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
  },

  /**
   * Fire an event manually.
   * @param {string} type type
   * @param {function} e handler
   * @param {boolean} isBubble whether or not event bubble
   * @return {} anything returned by handler function
   */
  fireEvent: function fireEvent (type, e, isBubble) {
    var isStopPropagation = false;
    var handler = this.event[type];
    if (handler && e) {
      e.stopPropagation = function () {
        isStopPropagation = true;
      };
      handler.call(this, e);
    }

    if (isStopPropagation) {
      return
    }

    if (isBubble && BUBBLE_EVENTS.includes(type) && this.parentNode && this.parentNode.fireEvent) {
      e.currentTarget = this.parentNode;
      this.parentNode.fireEvent(type, e, isBubble);
    }
  },

  /**
   * Get all styles of current element.
   * @return {object} style
   */
  toStyle: function toStyle () {
    return Object.assign({}, this.classStyle, this.style)
  },

  /**
   * Convert current element to JSON like object.
   * @return {object} element
   */
  toJSON: function toJSON () {
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
      result.children = this.pureChildren.map(function (child) { return child.toJSON(); });
    }
    return result
  },

  /**
   * Convert to HTML element tag string.
   * @return {stirng} html
   */
  toString: function toString () {
    return '<' + this.type +
    ' attr=' + JSON.stringify(this.attr) +
    ' style=' + JSON.stringify(this.toStyle()) + '>' +
    this.pureChildren.map(function (child) { return child.toString(); }).join('') +
    '</' + this.type + '>'
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
var fallback = function () {};

// The API of TaskCenter would be re-design.
var TaskCenter = function TaskCenter (id, sendTasks) {
  Object.defineProperty(this, 'instanceId', {
    enumerable: true,
    value: id
  });
  Object.defineProperty(this, 'callbackManager', {
    enumerable: true,
    value: new CallbackManager()
  });
  fallback = sendTasks || function () {};
};

TaskCenter.prototype.callback = function callback (callbackId, data, ifKeepAlive) {
  return this.callbackManager.consume(callbackId, data, ifKeepAlive)
};

TaskCenter.prototype.destroyCallback = function destroyCallback () {
  return this.callbackManager.close()
};

TaskCenter.prototype.typof = function typof (v) {
  var s = Object.prototype.toString.call(v);
  return s.substring(8, s.length - 1).toLowerCase()
};

/**
 * Normalize a value. Specially, if the value is a function, then generate a function id
 * and save it to `CallbackManager`, at last return the function id.
 * @param{any}      v
 * @param{object}   app
 * @return {primitive}
 */
TaskCenter.prototype.normalize = function normalize (v) {
  var type = this.typof(v);

  switch (type) {
    case 'undefined':
    case 'null':
      return ''
    case 'regexp':
      return v.toString()
    case 'date':
      return v.toISOString()
    case 'number':
    case 'string':
    case 'boolean':
    case 'array':
    case 'object':
      if (v instanceof Element) {
        return v.ref
      }
      return v
    case 'function':
      return this.callbackManager.add(v).toString()
    /* istanbul ignore next */
    default:
      return JSON.stringify(v)
  }
};

TaskCenter.prototype.send = function send (type, options, args) {
    var this$1 = this;

  var action = options.action;
    var component = options.component;
    var ref = options.ref;
    var module = options.module;
    var method = options.method;

  args = args.map(function (arg) { return this$1.normalize(arg); });

  switch (type) {
    case 'dom':
      return this[action](this.instanceId, args)
    case 'component':
      return this.componentHandler(this.instanceId, ref, method, args, { component: component })
    default:
      return this.moduleHandler(this.instanceId, module, method, args, {})
  }
};

TaskCenter.prototype.callDOM = function callDOM (action, args) {
  return this[action](this.instanceId, args)
};

TaskCenter.prototype.callComponent = function callComponent (ref, method, args) {
  return this.componentHandler(this.instanceId, ref, method, args, {})
};

TaskCenter.prototype.callModule = function callModule (module, method, args) {
  return this.moduleHandler(this.instanceId, module, method, args, {})
};

function init$1 () {
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
function register (name, options) {
  if (has$4(name)) {
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
function unregister (name) {
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
function has$4 (name) {
  return indexOf(name) >= 0
}

/**
 * Find the index of a JavaScript service by name
 * @param  {string} name
 * @return {number}
 */
function indexOf (name) {
  return services.map(function (service) { return service.name; }).indexOf(name)
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
var frameworks;
var runtimeConfig;

var versionRegExp = /^\s*\/\/ *(\{[^}]*\}) *\r?\n/;

/**
 * Detect a JS Bundle code and make sure which framework it's based to. Each JS
 * Bundle should make sure that it starts with a line of JSON comment and is
 * more that one line.
 * @param  {string} code
 * @return {object}
 */
function checkVersion (code) {
  var info;
  var result = versionRegExp.exec(code);
  if (result) {
    try {
      info = JSON.parse(result[1]);
    }
    catch (e) {}
  }
  return info
}

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

var instanceMap = {};

/**
 * Check which framework a certain JS Bundle code based to. And create instance
 * by this framework.
 * @param {string} id
 * @param {string} code
 * @param {object} config
 * @param {object} data
 */
function createInstance (id, code, config, data) {
  var info = instanceMap[id];

  if (!info) {
    // Init instance info.
    info = checkVersion(code) || {};
    if (!frameworks[info.framework]) {
      info.framework = 'Weex';
    }

    // Init instance config.
    config = JSON.parse(JSON.stringify(config || {}));
    config.bundleVersion = info.version;
    config.env = JSON.parse(JSON.stringify(global.WXEnvironment || {}));
    console.debug(("[JS Framework] create an " + (info.framework) + "@" + (config.bundleVersion) + " instance from " + (config.bundleVersion)));

    var env = {
      info: info,
      config: config,
      created: Date.now(),
      framework: info.framework
    };
    env.services = createServices(id, env, runtimeConfig);
    instanceMap[id] = env;

    return frameworks[info.framework].createInstance(id, code, config, data, env)
  }
  return new Error(("invalid instance id \"" + id + "\""))
}

var methods = {
  createInstance: createInstance,
  registerService: register,
  unregisterService: unregister
};

/**
 * Register methods which init each frameworks.
 * @param {string} methodName
 */
function genInit (methodName) {
  methods[methodName] = function () {
    var args = [], len = arguments.length;
    while ( len-- ) args[ len ] = arguments[ len ];

    if (methodName === 'registerComponents') {
      checkComponentMethods(args[0]);
    }
    for (var name in frameworks) {
      var framework = frameworks[name];
      if (framework && framework[methodName]) {
        framework[methodName].apply(framework, args);
      }
    }
  };
}

function checkComponentMethods (components) {
  if (Array.isArray(components)) {
    components.forEach(function (name) {
      if (name && name.type && name.methods) {
        registerElement(name.type, name.methods);
      }
    });
  }
}

/**
 * Register methods which will be called for each instance.
 * @param {string} methodName
 */
function genInstance (methodName) {
  methods[methodName] = function () {
    var args = [], len = arguments.length;
    while ( len-- ) args[ len ] = arguments[ len ];

    var id = args[0];
    var info = instanceMap[id];
    if (info && frameworks[info.framework]) {
      var result = (ref = frameworks[info.framework])[methodName].apply(ref, args);

      // Lifecycle methods
      if (methodName === 'refreshInstance') {
        services.forEach(function (service) {
          var refresh = service.options.refresh;
          if (refresh) {
            refresh(id, { info: info, runtime: runtimeConfig });
          }
        });
      }
      else if (methodName === 'destroyInstance') {
        services.forEach(function (service) {
          var destroy = service.options.destroy;
          if (destroy) {
            destroy(id, { info: info, runtime: runtimeConfig });
          }
        });
        delete instanceMap[id];
      }

      return result
    }
    return new Error(("invalid instance id \"" + id + "\""))
    var ref;
  };
}

/**
 * Adapt some legacy method(s) which will be called for each instance. These
 * methods should be deprecated and removed later.
 * @param {string} methodName
 * @param {string} nativeMethodName
 */
function adaptInstance (methodName, nativeMethodName) {
  methods[nativeMethodName] = function () {
    var args = [], len = arguments.length;
    while ( len-- ) args[ len ] = arguments[ len ];

    var id = args[0];
    var info = instanceMap[id];
    if (info && frameworks[info.framework]) {
      return (ref = frameworks[info.framework])[methodName].apply(ref, args)
    }
    return new Error(("invalid instance id \"" + id + "\""))
    var ref;
  };
}

function init$$1 (config) {
  runtimeConfig = config || {};
  frameworks = runtimeConfig.frameworks || {};
  init$1();

  // Init each framework by `init` method and `config` which contains three
  // virtual-DOM Class: `Document`, `Element` & `Comment`, and a JS bridge method:
  // `sendTasks(...args)`.
  for (var name in frameworks) {
    var framework = frameworks[name];
    framework.init(config);
  }

  // @todo: The method `registerMethods` will be re-designed or removed later.
   ['registerComponents', 'registerModules', 'registerMethods'].forEach(genInit)

  ; ['destroyInstance', 'refreshInstance', 'receiveTasks', 'getRoot'].forEach(genInstance);

  adaptInstance('receiveTasks', 'callJS');

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

/**
 * @fileOverview
 * Virtual-DOM Comment.
 */

function Comment (value) {
  this.nodeType = 8;
  this.nodeId = uniqueId();
  this.ref = this.nodeId;
  this.type = 'comment';
  this.value = value;
  this.children = [];
  this.pureChildren = [];
}

Comment.prototype = Object.create(Node.prototype);
Comment.prototype.constructor = Comment;

/**
 * Convert to HTML comment string.
 * @return {stirng} html
 */
Comment.prototype.toString = function () {
  return '<!-- ' + this.value + ' -->'
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
 * Listen virtual-dom operations and create corresponding actions.
 */

function Listener (id, handler) {
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
}

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

Object.assign(Listener.prototype, {
  /**
   * Send the "createFinish" signal.
   * @param {function} callback
   * @return {undefined | number} the signal sent by native
   */
  createFinish: function createFinish (callback) {
    var handler = this.handler;
    return handler([createAction('createFinish')], callback)
  },

  /**
   * Send the "updateFinish" signal.
   * @param {function} callback
   * @return {undefined | number} the signal sent by native
   */
  updateFinish: function updateFinish (callback) {
    var handler = this.handler;
    return handler([createAction('updateFinish')], callback)
  },

  /**
   * Send the "refreshFinish" signal.
   * @param {function} callback
   * @return {undefined | number} the signal sent by native
   */
  refreshFinish: function refreshFinish (callback) {
    var handler = this.handler;
    return handler([createAction('refreshFinish')], callback)
  },

  /**
   * Send the "createBody" signal.
   * @param {object} element
   * @return {undefined | number} the signal sent by native
   */
  createBody: function createBody (element) {
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
  },

  /**
   * Send the "addElement" signal.
   * @param {object} element
   * @param {stirng} reference id
   * @param {number} index
   * @return {undefined | number} the signal sent by native
   */
  addElement: function addElement (element, ref, index) {
    if (!(index >= 0)) {
      index = -1;
    }
    return this.addActions(createAction('addElement', [ref, element.toJSON(), index]))
  },

  /**
   * Send the "removeElement" signal.
   * @param {stirng} reference id
   * @return {undefined | number} the signal sent by native
   */
  removeElement: function removeElement (ref) {
    if (Array.isArray(ref)) {
      var actions = ref.map(function (r) { return createAction('removeElement', [r]); });
      return this.addActions(actions)
    }
    return this.addActions(createAction('removeElement', [ref]))
  },

  /**
   * Send the "moveElement" signal.
   * @param {stirng} target reference id
   * @param {stirng} parent reference id
   * @param {number} index
   * @return {undefined | number} the signal sent by native
   */
  moveElement: function moveElement (targetRef, parentRef, index) {
    return this.addActions(createAction('moveElement', [targetRef, parentRef, index]))
  },

  /**
   * Send the "updateAttrs" signal.
   * @param {stirng} reference id
   * @param {stirng} key
   * @param {stirng} value
   * @return {undefined | number} the signal sent by native
   */
  setAttr: function setAttr (ref, key, value) {
    var result = {};
    result[key] = value;
    return this.addActions(createAction('updateAttrs', [ref, result]))
  },

  /**
   * Send the "updateStyle" signal, update a sole style.
   * @param {stirng} reference id
   * @param {stirng} key
   * @param {stirng} value
   * @return {undefined | number} the signal sent by native
   */
  setStyle: function setStyle (ref, key, value) {
    var result = {};
    result[key] = value;
    return this.addActions(createAction('updateStyle', [ref, result]))
  },

  /**
   * Send the "updateStyle" signal.
   * @param {stirng} reference id
   * @param {object} style
   * @return {undefined | number} the signal sent by native
   */
  setStyles: function setStyles (ref, style) {
    return this.addActions(createAction('updateStyle', [ref, style]))
  },

  /**
   * Send the "addEvent" signal.
   * @param {stirng} reference id
   * @param {string} event type
   * @return {undefined | number} the signal sent by native
   */
  addEvent: function addEvent (ref, type) {
    return this.addActions(createAction('addEvent', [ref, type]))
  },

  /**
   * Send the "removeEvent" signal.
   * @param {stirng} reference id
   * @param {string} event type
   * @return {undefined | number} the signal sent by native
   */
  removeEvent: function removeEvent (ref, type) {
    return this.addActions(createAction('removeEvent', [ref, type]))
  },

  /**
   * Default handler.
   * @param {object | array} actions
   * @param {function} callback
   * @return {} anything returned by callback function
   */
  handler: function handler (actions, cb) {
    return cb && cb()
  },

  /**
   * Add actions into updates.
   * @param {object | array} actions
   * @return {undefined | number} the signal sent by native
   */
  addActions: function addActions (actions) {
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
 * @fileOverview
 * Virtual-DOM Document.
 */

function Document (id, url, handler) {
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
}

// default task handler
Document.handler = null;

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

Object.assign(Document.prototype, {
  /**
   * Get the node from nodeMap.
   * @param {string} reference id
   * @return {object} node
   */
  getRef: function getRef (ref) {
    return this.nodeMap[ref]
  },

  /**
   * Turn on batched updates.
   */
  open: function open () {
    this.listener.batched = false;
  },

  /**
   * Turn off batched updates.
   */
  close: function close () {
    this.listener.batched = true;
  },

  /**
   * Create the document element.
   * @return {object} documentElement
   */
  createDocumentElement: function createDocumentElement () {
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
  },

  /**
   * Create the body element.
   * @param {string} type
   * @param {objct} props
   * @return {object} body element
   */
  createBody: function createBody (type, props) {
    if (!this.body) {
      var el = new Element(type, props);
      setBody(this, el);
    }

    return this.body
  },

  /**
   * Create an element.
   * @param {string} tagName
   * @param {objct} props
   * @return {object} element
   */
  createElement: function createElement (tagName, props) {
    return new Element(tagName, props)
  },

  /**
   * Create an comment.
   * @param {string} text
   * @return {object} comment
   */
  createComment: function createComment (text) {
    return new Comment(text)
  },

  /**
   * Fire an event on specified element manually.
   * @param {object} element
   * @param {string} event type
   * @param {object} event object
   * @param {object} dom changes
   * @return {} anything returned by handler function
   */
  fireEvent: function fireEvent (el, type, e, domChanges) {
    if (!el) {
      return
    }
    e = e || {};
    e.type = type;
    e.target = el;
    e.currentTarget = el;
    e.timestamp = Date.now();
    if (domChanges) {
      updateElement(el, domChanges);
    }
    var isBubble = this.getRef('_root').attr['bubble'] === 'true';
    return el.fireEvent(type, e, isBubble)
  },

  /**
   * Destroy current document, and remove itself form docMap.
   */
  destroy: function destroy () {
    delete this.listener;
    delete this.nodeMap;
    removeDoc(this.id);
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

    return global.callNative.apply(global, args)
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

  Object.freeze(config.Element);
  Object.freeze(config.Comment);
  Object.freeze(config.Listener);
  Object.freeze(config.Document.prototype);
  Object.freeze(config.Element.prototype);
  Object.freeze(config.Comment.prototype);
  Object.freeze(config.Listener.prototype);
}

var index = {
  setNativeConsole: setNativeConsole,
  resetNativeConsole: resetNativeConsole,
  setNativeTimer: setNativeTimer,
  resetNativeTimer: resetNativeTimer,
  service: { register: register, unregister: unregister, has: has$4 },
  freezePrototype: freezePrototype$$1,
  init: init$$1,
  config: config
};

return index;

})));
//# sourceMappingURL=data:application/json;charset=utf-8;base64,eyJ2ZXJzaW9uIjozLCJmaWxlIjpudWxsLCJzb3VyY2VzIjpbIi4uLy4uL2h0bWw1L3NoYXJlZC9hcnJheUZyb20uanMiLCIuLi8uLi9ub2RlX21vZHVsZXMvY29yZS1qcy9tb2R1bGVzL19nbG9iYWwuanMiLCIuLi8uLi9ub2RlX21vZHVsZXMvY29yZS1qcy9tb2R1bGVzL19jb3JlLmpzIiwiLi4vLi4vbm9kZV9tb2R1bGVzL2NvcmUtanMvbW9kdWxlcy9faXMtb2JqZWN0LmpzIiwiLi4vLi4vbm9kZV9tb2R1bGVzL2NvcmUtanMvbW9kdWxlcy9fYW4tb2JqZWN0LmpzIiwiLi4vLi4vbm9kZV9tb2R1bGVzL2NvcmUtanMvbW9kdWxlcy9fZmFpbHMuanMiLCIuLi8uLi9ub2RlX21vZHVsZXMvY29yZS1qcy9tb2R1bGVzL19kZXNjcmlwdG9ycy5qcyIsIi4uLy4uL25vZGVfbW9kdWxlcy9jb3JlLWpzL21vZHVsZXMvX2RvbS1jcmVhdGUuanMiLCIuLi8uLi9ub2RlX21vZHVsZXMvY29yZS1qcy9tb2R1bGVzL19pZTgtZG9tLWRlZmluZS5qcyIsIi4uLy4uL25vZGVfbW9kdWxlcy9jb3JlLWpzL21vZHVsZXMvX3RvLXByaW1pdGl2ZS5qcyIsIi4uLy4uL25vZGVfbW9kdWxlcy9jb3JlLWpzL21vZHVsZXMvX29iamVjdC1kcC5qcyIsIi4uLy4uL25vZGVfbW9kdWxlcy9jb3JlLWpzL21vZHVsZXMvX3Byb3BlcnR5LWRlc2MuanMiLCIuLi8uLi9ub2RlX21vZHVsZXMvY29yZS1qcy9tb2R1bGVzL19oaWRlLmpzIiwiLi4vLi4vbm9kZV9tb2R1bGVzL2NvcmUtanMvbW9kdWxlcy9faGFzLmpzIiwiLi4vLi4vbm9kZV9tb2R1bGVzL2NvcmUtanMvbW9kdWxlcy9fdWlkLmpzIiwiLi4vLi4vbm9kZV9tb2R1bGVzL2NvcmUtanMvbW9kdWxlcy9fcmVkZWZpbmUuanMiLCIuLi8uLi9ub2RlX21vZHVsZXMvY29yZS1qcy9tb2R1bGVzL19hLWZ1bmN0aW9uLmpzIiwiLi4vLi4vbm9kZV9tb2R1bGVzL2NvcmUtanMvbW9kdWxlcy9fY3R4LmpzIiwiLi4vLi4vbm9kZV9tb2R1bGVzL2NvcmUtanMvbW9kdWxlcy9fZXhwb3J0LmpzIiwiLi4vLi4vbm9kZV9tb2R1bGVzL2NvcmUtanMvbW9kdWxlcy9fY29mLmpzIiwiLi4vLi4vbm9kZV9tb2R1bGVzL2NvcmUtanMvbW9kdWxlcy9faW9iamVjdC5qcyIsIi4uLy4uL25vZGVfbW9kdWxlcy9jb3JlLWpzL21vZHVsZXMvX2RlZmluZWQuanMiLCIuLi8uLi9ub2RlX21vZHVsZXMvY29yZS1qcy9tb2R1bGVzL190by1pb2JqZWN0LmpzIiwiLi4vLi4vbm9kZV9tb2R1bGVzL2NvcmUtanMvbW9kdWxlcy9fdG8taW50ZWdlci5qcyIsIi4uLy4uL25vZGVfbW9kdWxlcy9jb3JlLWpzL21vZHVsZXMvX3RvLWxlbmd0aC5qcyIsIi4uLy4uL25vZGVfbW9kdWxlcy9jb3JlLWpzL21vZHVsZXMvX3RvLWluZGV4LmpzIiwiLi4vLi4vbm9kZV9tb2R1bGVzL2NvcmUtanMvbW9kdWxlcy9fYXJyYXktaW5jbHVkZXMuanMiLCIuLi8uLi9ub2RlX21vZHVsZXMvY29yZS1qcy9tb2R1bGVzL19zaGFyZWQuanMiLCIuLi8uLi9ub2RlX21vZHVsZXMvY29yZS1qcy9tb2R1bGVzL19zaGFyZWQta2V5LmpzIiwiLi4vLi4vbm9kZV9tb2R1bGVzL2NvcmUtanMvbW9kdWxlcy9fb2JqZWN0LWtleXMtaW50ZXJuYWwuanMiLCIuLi8uLi9ub2RlX21vZHVsZXMvY29yZS1qcy9tb2R1bGVzL19lbnVtLWJ1Zy1rZXlzLmpzIiwiLi4vLi4vbm9kZV9tb2R1bGVzL2NvcmUtanMvbW9kdWxlcy9fb2JqZWN0LWtleXMuanMiLCIuLi8uLi9ub2RlX21vZHVsZXMvY29yZS1qcy9tb2R1bGVzL19vYmplY3QtZ29wcy5qcyIsIi4uLy4uL25vZGVfbW9kdWxlcy9jb3JlLWpzL21vZHVsZXMvX29iamVjdC1waWUuanMiLCIuLi8uLi9ub2RlX21vZHVsZXMvY29yZS1qcy9tb2R1bGVzL190by1vYmplY3QuanMiLCIuLi8uLi9ub2RlX21vZHVsZXMvY29yZS1qcy9tb2R1bGVzL19vYmplY3QtYXNzaWduLmpzIiwiLi4vLi4vbm9kZV9tb2R1bGVzL2NvcmUtanMvbW9kdWxlcy9lczYub2JqZWN0LmFzc2lnbi5qcyIsIi4uLy4uL2h0bWw1L3NoYXJlZC9vYmplY3RBc3NpZ24uanMiLCIuLi8uLi9odG1sNS9zaGFyZWQvb2JqZWN0U2V0UHJvdG90eXBlT2YuanMiLCIuLi8uLi9odG1sNS9zaGFyZWQvcHJvbWlzZS5qcyIsIi4uLy4uL25vZGVfbW9kdWxlcy9jb3JlLWpzL21vZHVsZXMvX3drcy5qcyIsIi4uLy4uL25vZGVfbW9kdWxlcy9jb3JlLWpzL21vZHVsZXMvX2NsYXNzb2YuanMiLCIuLi8uLi9ub2RlX21vZHVsZXMvY29yZS1qcy9tb2R1bGVzL2VzNi5vYmplY3QudG8tc3RyaW5nLmpzIiwiLi4vLi4vbm9kZV9tb2R1bGVzL2NvcmUtanMvbW9kdWxlcy9fc3RyaW5nLWF0LmpzIiwiLi4vLi4vbm9kZV9tb2R1bGVzL2NvcmUtanMvbW9kdWxlcy9fbGlicmFyeS5qcyIsIi4uLy4uL25vZGVfbW9kdWxlcy9jb3JlLWpzL21vZHVsZXMvX2l0ZXJhdG9ycy5qcyIsIi4uLy4uL25vZGVfbW9kdWxlcy9jb3JlLWpzL21vZHVsZXMvX29iamVjdC1kcHMuanMiLCIuLi8uLi9ub2RlX21vZHVsZXMvY29yZS1qcy9tb2R1bGVzL19odG1sLmpzIiwiLi4vLi4vbm9kZV9tb2R1bGVzL2NvcmUtanMvbW9kdWxlcy9fb2JqZWN0LWNyZWF0ZS5qcyIsIi4uLy4uL25vZGVfbW9kdWxlcy9jb3JlLWpzL21vZHVsZXMvX3NldC10by1zdHJpbmctdGFnLmpzIiwiLi4vLi4vbm9kZV9tb2R1bGVzL2NvcmUtanMvbW9kdWxlcy9faXRlci1jcmVhdGUuanMiLCIuLi8uLi9ub2RlX21vZHVsZXMvY29yZS1qcy9tb2R1bGVzL19vYmplY3QtZ3BvLmpzIiwiLi4vLi4vbm9kZV9tb2R1bGVzL2NvcmUtanMvbW9kdWxlcy9faXRlci1kZWZpbmUuanMiLCIuLi8uLi9ub2RlX21vZHVsZXMvY29yZS1qcy9tb2R1bGVzL2VzNi5zdHJpbmcuaXRlcmF0b3IuanMiLCIuLi8uLi9ub2RlX21vZHVsZXMvY29yZS1qcy9tb2R1bGVzL19hZGQtdG8tdW5zY29wYWJsZXMuanMiLCIuLi8uLi9ub2RlX21vZHVsZXMvY29yZS1qcy9tb2R1bGVzL19pdGVyLXN0ZXAuanMiLCIuLi8uLi9ub2RlX21vZHVsZXMvY29yZS1qcy9tb2R1bGVzL2VzNi5hcnJheS5pdGVyYXRvci5qcyIsIi4uLy4uL25vZGVfbW9kdWxlcy9jb3JlLWpzL21vZHVsZXMvd2ViLmRvbS5pdGVyYWJsZS5qcyIsIi4uLy4uL25vZGVfbW9kdWxlcy9jb3JlLWpzL21vZHVsZXMvX2FuLWluc3RhbmNlLmpzIiwiLi4vLi4vbm9kZV9tb2R1bGVzL2NvcmUtanMvbW9kdWxlcy9faXRlci1jYWxsLmpzIiwiLi4vLi4vbm9kZV9tb2R1bGVzL2NvcmUtanMvbW9kdWxlcy9faXMtYXJyYXktaXRlci5qcyIsIi4uLy4uL25vZGVfbW9kdWxlcy9jb3JlLWpzL21vZHVsZXMvY29yZS5nZXQtaXRlcmF0b3ItbWV0aG9kLmpzIiwiLi4vLi4vbm9kZV9tb2R1bGVzL2NvcmUtanMvbW9kdWxlcy9fZm9yLW9mLmpzIiwiLi4vLi4vbm9kZV9tb2R1bGVzL2NvcmUtanMvbW9kdWxlcy9fc3BlY2llcy1jb25zdHJ1Y3Rvci5qcyIsIi4uLy4uL25vZGVfbW9kdWxlcy9jb3JlLWpzL21vZHVsZXMvX2ludm9rZS5qcyIsIi4uLy4uL25vZGVfbW9kdWxlcy9jb3JlLWpzL21vZHVsZXMvX3Rhc2suanMiLCIuLi8uLi9ub2RlX21vZHVsZXMvY29yZS1qcy9tb2R1bGVzL19taWNyb3Rhc2suanMiLCIuLi8uLi9ub2RlX21vZHVsZXMvY29yZS1qcy9tb2R1bGVzL19yZWRlZmluZS1hbGwuanMiLCIuLi8uLi9ub2RlX21vZHVsZXMvY29yZS1qcy9tb2R1bGVzL19zZXQtc3BlY2llcy5qcyIsIi4uLy4uL25vZGVfbW9kdWxlcy9jb3JlLWpzL21vZHVsZXMvX2l0ZXItZGV0ZWN0LmpzIiwiLi4vLi4vbm9kZV9tb2R1bGVzL2NvcmUtanMvbW9kdWxlcy9lczYucHJvbWlzZS5qcyIsIi4uLy4uL2h0bWw1L3NoYXJlZC9jb25zb2xlLmpzIiwiLi4vLi4vaHRtbDUvc2hhcmVkL3NldFRpbWVvdXQuanMiLCIuLi8uLi9odG1sNS9zaGFyZWQvZnJlZXplLmpzIiwiLi4vLi4vaHRtbDUvc2hhcmVkL2luZGV4LmpzIiwiLi4vLi4vaHRtbDUvcnVudGltZS9jYWxsYmFjay1tYW5hZ2VyLmpzIiwiLi4vLi4vaHRtbDUvcnVudGltZS92ZG9tL29wZXJhdGlvbi5qcyIsIi4uLy4uL2h0bWw1L3J1bnRpbWUvdmRvbS9ub2RlLmpzIiwiLi4vLi4vaHRtbDUvcnVudGltZS92ZG9tL2VsZW1lbnQtdHlwZXMuanMiLCIuLi8uLi9odG1sNS9ydW50aW1lL3Zkb20vZWxlbWVudC5qcyIsIi4uLy4uL2h0bWw1L3J1bnRpbWUvdGFzay1jZW50ZXIuanMiLCIuLi8uLi9odG1sNS9ydW50aW1lL3NlcnZpY2UuanMiLCIuLi8uLi9odG1sNS9ydW50aW1lL2luaXQuanMiLCIuLi8uLi9odG1sNS9ydW50aW1lL3Zkb20vY29tbWVudC5qcyIsIi4uLy4uL2h0bWw1L3J1bnRpbWUvbGlzdGVuZXIuanMiLCIuLi8uLi9odG1sNS9ydW50aW1lL2hhbmRsZXIuanMiLCIuLi8uLi9odG1sNS9ydW50aW1lL3Zkb20vZG9jdW1lbnQuanMiLCIuLi8uLi9odG1sNS9ydW50aW1lL3Zkb20vaW5kZXguanMiLCIuLi8uLi9odG1sNS9ydW50aW1lL2NvbmZpZy5qcyIsIi4uLy4uL2h0bWw1L3J1bnRpbWUvaW5kZXguanMiXSwic291cmNlc0NvbnRlbnQiOlsiLypcbiAqIExpY2Vuc2VkIHRvIHRoZSBBcGFjaGUgU29mdHdhcmUgRm91bmRhdGlvbiAoQVNGKSB1bmRlciBvbmVcbiAqIG9yIG1vcmUgY29udHJpYnV0b3IgbGljZW5zZSBhZ3JlZW1lbnRzLiAgU2VlIHRoZSBOT1RJQ0UgZmlsZVxuICogZGlzdHJpYnV0ZWQgd2l0aCB0aGlzIHdvcmsgZm9yIGFkZGl0aW9uYWwgaW5mb3JtYXRpb25cbiAqIHJlZ2FyZGluZyBjb3B5cmlnaHQgb3duZXJzaGlwLiAgVGhlIEFTRiBsaWNlbnNlcyB0aGlzIGZpbGVcbiAqIHRvIHlvdSB1bmRlciB0aGUgQXBhY2hlIExpY2Vuc2UsIFZlcnNpb24gMi4wICh0aGVcbiAqIFwiTGljZW5zZVwiKTsgeW91IG1heSBub3QgdXNlIHRoaXMgZmlsZSBleGNlcHQgaW4gY29tcGxpYW5jZVxuICogd2l0aCB0aGUgTGljZW5zZS4gIFlvdSBtYXkgb2J0YWluIGEgY29weSBvZiB0aGUgTGljZW5zZSBhdFxuICpcbiAqICAgaHR0cDovL3d3dy5hcGFjaGUub3JnL2xpY2Vuc2VzL0xJQ0VOU0UtMi4wXG4gKlxuICogVW5sZXNzIHJlcXVpcmVkIGJ5IGFwcGxpY2FibGUgbGF3IG9yIGFncmVlZCB0byBpbiB3cml0aW5nLFxuICogc29mdHdhcmUgZGlzdHJpYnV0ZWQgdW5kZXIgdGhlIExpY2Vuc2UgaXMgZGlzdHJpYnV0ZWQgb24gYW5cbiAqIFwiQVMgSVNcIiBCQVNJUywgV0lUSE9VVCBXQVJSQU5USUVTIE9SIENPTkRJVElPTlMgT0YgQU5ZXG4gKiBLSU5ELCBlaXRoZXIgZXhwcmVzcyBvciBpbXBsaWVkLiAgU2VlIHRoZSBMaWNlbnNlIGZvciB0aGVcbiAqIHNwZWNpZmljIGxhbmd1YWdlIGdvdmVybmluZyBwZXJtaXNzaW9ucyBhbmQgbGltaXRhdGlvbnNcbiAqIHVuZGVyIHRoZSBMaWNlbnNlLlxuICovXG5cbi8qIGVzbGludC1kaXNhYmxlICovXG5cbi8vIFByb2R1Y3Rpb24gc3RlcHMgb2YgRUNNQS0yNjIsIEVkaXRpb24gNiwgMjIuMS4yLjFcbi8vIFJlZmVyZW5jZTogaHR0cHM6Ly9wZW9wbGUubW96aWxsYS5vcmcvfmpvcmVuZG9yZmYvZXM2LWRyYWZ0Lmh0bWwjc2VjLWFycmF5LmZyb21cblxuLyogaXN0YW5idWwgaWdub3JlIGlmICovXG5pZiAoIUFycmF5LmZyb20pIHtcbiAgQXJyYXkuZnJvbSA9IChmdW5jdGlvbigpIHtcbiAgICB2YXIgdG9TdHIgPSBPYmplY3QucHJvdG90eXBlLnRvU3RyaW5nO1xuICAgIHZhciBpc0NhbGxhYmxlID0gZnVuY3Rpb24oZm4pIHtcbiAgICAgIHJldHVybiB0eXBlb2YgZm4gPT09ICdmdW5jdGlvbicgfHwgdG9TdHIuY2FsbChmbikgPT09ICdbb2JqZWN0IEZ1bmN0aW9uXSc7XG4gICAgfTtcbiAgICB2YXIgdG9JbnRlZ2VyID0gZnVuY3Rpb24odmFsdWUpIHtcbiAgICAgIHZhciBudW1iZXIgPSBOdW1iZXIodmFsdWUpO1xuICAgICAgaWYgKGlzTmFOKG51bWJlcikpIHtcbiAgICAgICAgcmV0dXJuIDA7XG4gICAgICB9XG4gICAgICBpZiAobnVtYmVyID09PSAwIHx8ICFpc0Zpbml0ZShudW1iZXIpKSB7XG4gICAgICAgIHJldHVybiBudW1iZXI7XG4gICAgICB9XG4gICAgICByZXR1cm4gKG51bWJlciA+IDAgPyAxIDogLTEpICogTWF0aC5mbG9vcihNYXRoLmFicyhudW1iZXIpKTtcbiAgICB9O1xuICAgIHZhciBtYXhTYWZlSW50ZWdlciA9IE1hdGgucG93KDIsIDUzKSAtIDE7XG4gICAgdmFyIHRvTGVuZ3RoID0gZnVuY3Rpb24odmFsdWUpIHtcbiAgICAgIHZhciBsZW4gPSB0b0ludGVnZXIodmFsdWUpO1xuICAgICAgcmV0dXJuIE1hdGgubWluKE1hdGgubWF4KGxlbiwgMCksIG1heFNhZmVJbnRlZ2VyKTtcbiAgICB9O1xuXG4gICAgLy8gVGhlIGxlbmd0aCBwcm9wZXJ0eSBvZiB0aGUgZnJvbSBtZXRob2QgaXMgMS5cbiAgICByZXR1cm4gZnVuY3Rpb24gZnJvbShhcnJheUxpa2UvKiwgbWFwRm4sIHRoaXNBcmcgKi8pIHtcbiAgICAgIC8vIDEuIExldCBDIGJlIHRoZSB0aGlzIHZhbHVlLlxuICAgICAgdmFyIEMgPSB0aGlzO1xuXG4gICAgICAvLyAyLiBMZXQgaXRlbXMgYmUgVG9PYmplY3QoYXJyYXlMaWtlKS5cbiAgICAgIHZhciBpdGVtcyA9IE9iamVjdChhcnJheUxpa2UpO1xuXG4gICAgICAvLyAzLiBSZXR1cm5JZkFicnVwdChpdGVtcykuXG4gICAgICBpZiAoYXJyYXlMaWtlID09IG51bGwpIHtcbiAgICAgICAgdGhyb3cgbmV3IFR5cGVFcnJvcignQXJyYXkuZnJvbSByZXF1aXJlcyBhbiBhcnJheS1saWtlIG9iamVjdCAtIG5vdCBudWxsIG9yIHVuZGVmaW5lZCcpO1xuICAgICAgfVxuXG4gICAgICAvLyA0LiBJZiBtYXBmbiBpcyB1bmRlZmluZWQsIHRoZW4gbGV0IG1hcHBpbmcgYmUgZmFsc2UuXG4gICAgICB2YXIgbWFwRm4gPSBhcmd1bWVudHMubGVuZ3RoID4gMSA/IGFyZ3VtZW50c1sxXSA6IHZvaWQgdW5kZWZpbmVkO1xuICAgICAgdmFyIFQ7XG4gICAgICBpZiAodHlwZW9mIG1hcEZuICE9PSAndW5kZWZpbmVkJykge1xuICAgICAgICAvLyA1LiBlbHNlXG4gICAgICAgIC8vIDUuIGEgSWYgSXNDYWxsYWJsZShtYXBmbikgaXMgZmFsc2UsIHRocm93IGEgVHlwZUVycm9yIGV4Y2VwdGlvbi5cbiAgICAgICAgaWYgKCFpc0NhbGxhYmxlKG1hcEZuKSkge1xuICAgICAgICAgIHRocm93IG5ldyBUeXBlRXJyb3IoJ0FycmF5LmZyb206IHdoZW4gcHJvdmlkZWQsIHRoZSBzZWNvbmQgYXJndW1lbnQgbXVzdCBiZSBhIGZ1bmN0aW9uJyk7XG4gICAgICAgIH1cblxuICAgICAgICAvLyA1LiBiLiBJZiB0aGlzQXJnIHdhcyBzdXBwbGllZCwgbGV0IFQgYmUgdGhpc0FyZzsgZWxzZSBsZXQgVCBiZSB1bmRlZmluZWQuXG4gICAgICAgIGlmIChhcmd1bWVudHMubGVuZ3RoID4gMikge1xuICAgICAgICAgIFQgPSBhcmd1bWVudHNbMl07XG4gICAgICAgIH1cbiAgICAgIH1cblxuICAgICAgLy8gMTAuIExldCBsZW5WYWx1ZSBiZSBHZXQoaXRlbXMsIFwibGVuZ3RoXCIpLlxuICAgICAgLy8gMTEuIExldCBsZW4gYmUgVG9MZW5ndGgobGVuVmFsdWUpLlxuICAgICAgdmFyIGxlbiA9IHRvTGVuZ3RoKGl0ZW1zLmxlbmd0aCk7XG5cbiAgICAgIC8vIDEzLiBJZiBJc0NvbnN0cnVjdG9yKEMpIGlzIHRydWUsIHRoZW5cbiAgICAgIC8vIDEzLiBhLiBMZXQgQSBiZSB0aGUgcmVzdWx0IG9mIGNhbGxpbmcgdGhlIFtbQ29uc3RydWN0XV0gaW50ZXJuYWwgbWV0aG9kIG9mIEMgd2l0aCBhbiBhcmd1bWVudCBsaXN0IGNvbnRhaW5pbmcgdGhlIHNpbmdsZSBpdGVtIGxlbi5cbiAgICAgIC8vIDE0LiBhLiBFbHNlLCBMZXQgQSBiZSBBcnJheUNyZWF0ZShsZW4pLlxuICAgICAgdmFyIEEgPSBpc0NhbGxhYmxlKEMpID8gT2JqZWN0KG5ldyBDKGxlbikpIDogbmV3IEFycmF5KGxlbik7XG5cbiAgICAgIC8vIDE2LiBMZXQgayBiZSAwLlxuICAgICAgdmFyIGsgPSAwO1xuICAgICAgLy8gMTcuIFJlcGVhdCwgd2hpbGUgayA8IGxlbuKApiAoYWxzbyBzdGVwcyBhIC0gaClcbiAgICAgIHZhciBrVmFsdWU7XG4gICAgICB3aGlsZSAoayA8IGxlbikge1xuICAgICAgICBrVmFsdWUgPSBpdGVtc1trXTtcbiAgICAgICAgaWYgKG1hcEZuKSB7XG4gICAgICAgICAgQVtrXSA9IHR5cGVvZiBUID09PSAndW5kZWZpbmVkJyA/IG1hcEZuKGtWYWx1ZSwgaykgOiBtYXBGbi5jYWxsKFQsIGtWYWx1ZSwgayk7XG4gICAgICAgIH0gZWxzZSB7XG4gICAgICAgICAgQVtrXSA9IGtWYWx1ZTtcbiAgICAgICAgfVxuICAgICAgICBrICs9IDE7XG4gICAgICB9XG4gICAgICAvLyAxOC4gTGV0IHB1dFN0YXR1cyBiZSBQdXQoQSwgXCJsZW5ndGhcIiwgbGVuLCB0cnVlKS5cbiAgICAgIEEubGVuZ3RoID0gbGVuO1xuICAgICAgLy8gMjAuIFJldHVybiBBLlxuICAgICAgcmV0dXJuIEE7XG4gICAgfTtcbiAgfSgpKTtcbn1cbiIsIi8vIGh0dHBzOi8vZ2l0aHViLmNvbS96bG9pcm9jay9jb3JlLWpzL2lzc3Vlcy84NiNpc3N1ZWNvbW1lbnQtMTE1NzU5MDI4XG52YXIgZ2xvYmFsID0gbW9kdWxlLmV4cG9ydHMgPSB0eXBlb2Ygd2luZG93ICE9ICd1bmRlZmluZWQnICYmIHdpbmRvdy5NYXRoID09IE1hdGhcbiAgPyB3aW5kb3cgOiB0eXBlb2Ygc2VsZiAhPSAndW5kZWZpbmVkJyAmJiBzZWxmLk1hdGggPT0gTWF0aCA/IHNlbGYgOiBGdW5jdGlvbigncmV0dXJuIHRoaXMnKSgpO1xuaWYodHlwZW9mIF9fZyA9PSAnbnVtYmVyJylfX2cgPSBnbG9iYWw7IC8vIGVzbGludC1kaXNhYmxlLWxpbmUgbm8tdW5kZWYiLCJ2YXIgY29yZSA9IG1vZHVsZS5leHBvcnRzID0ge3ZlcnNpb246ICcyLjQuMCd9O1xuaWYodHlwZW9mIF9fZSA9PSAnbnVtYmVyJylfX2UgPSBjb3JlOyAvLyBlc2xpbnQtZGlzYWJsZS1saW5lIG5vLXVuZGVmIiwibW9kdWxlLmV4cG9ydHMgPSBmdW5jdGlvbihpdCl7XG4gIHJldHVybiB0eXBlb2YgaXQgPT09ICdvYmplY3QnID8gaXQgIT09IG51bGwgOiB0eXBlb2YgaXQgPT09ICdmdW5jdGlvbic7XG59OyIsInZhciBpc09iamVjdCA9IHJlcXVpcmUoJy4vX2lzLW9iamVjdCcpO1xubW9kdWxlLmV4cG9ydHMgPSBmdW5jdGlvbihpdCl7XG4gIGlmKCFpc09iamVjdChpdCkpdGhyb3cgVHlwZUVycm9yKGl0ICsgJyBpcyBub3QgYW4gb2JqZWN0IScpO1xuICByZXR1cm4gaXQ7XG59OyIsIm1vZHVsZS5leHBvcnRzID0gZnVuY3Rpb24oZXhlYyl7XG4gIHRyeSB7XG4gICAgcmV0dXJuICEhZXhlYygpO1xuICB9IGNhdGNoKGUpe1xuICAgIHJldHVybiB0cnVlO1xuICB9XG59OyIsIi8vIFRoYW5rJ3MgSUU4IGZvciBoaXMgZnVubnkgZGVmaW5lUHJvcGVydHlcbm1vZHVsZS5leHBvcnRzID0gIXJlcXVpcmUoJy4vX2ZhaWxzJykoZnVuY3Rpb24oKXtcbiAgcmV0dXJuIE9iamVjdC5kZWZpbmVQcm9wZXJ0eSh7fSwgJ2EnLCB7Z2V0OiBmdW5jdGlvbigpeyByZXR1cm4gNzsgfX0pLmEgIT0gNztcbn0pOyIsInZhciBpc09iamVjdCA9IHJlcXVpcmUoJy4vX2lzLW9iamVjdCcpXG4gICwgZG9jdW1lbnQgPSByZXF1aXJlKCcuL19nbG9iYWwnKS5kb2N1bWVudFxuICAvLyBpbiBvbGQgSUUgdHlwZW9mIGRvY3VtZW50LmNyZWF0ZUVsZW1lbnQgaXMgJ29iamVjdCdcbiAgLCBpcyA9IGlzT2JqZWN0KGRvY3VtZW50KSAmJiBpc09iamVjdChkb2N1bWVudC5jcmVhdGVFbGVtZW50KTtcbm1vZHVsZS5leHBvcnRzID0gZnVuY3Rpb24oaXQpe1xuICByZXR1cm4gaXMgPyBkb2N1bWVudC5jcmVhdGVFbGVtZW50KGl0KSA6IHt9O1xufTsiLCJtb2R1bGUuZXhwb3J0cyA9ICFyZXF1aXJlKCcuL19kZXNjcmlwdG9ycycpICYmICFyZXF1aXJlKCcuL19mYWlscycpKGZ1bmN0aW9uKCl7XG4gIHJldHVybiBPYmplY3QuZGVmaW5lUHJvcGVydHkocmVxdWlyZSgnLi9fZG9tLWNyZWF0ZScpKCdkaXYnKSwgJ2EnLCB7Z2V0OiBmdW5jdGlvbigpeyByZXR1cm4gNzsgfX0pLmEgIT0gNztcbn0pOyIsIi8vIDcuMS4xIFRvUHJpbWl0aXZlKGlucHV0IFssIFByZWZlcnJlZFR5cGVdKVxudmFyIGlzT2JqZWN0ID0gcmVxdWlyZSgnLi9faXMtb2JqZWN0Jyk7XG4vLyBpbnN0ZWFkIG9mIHRoZSBFUzYgc3BlYyB2ZXJzaW9uLCB3ZSBkaWRuJ3QgaW1wbGVtZW50IEBAdG9QcmltaXRpdmUgY2FzZVxuLy8gYW5kIHRoZSBzZWNvbmQgYXJndW1lbnQgLSBmbGFnIC0gcHJlZmVycmVkIHR5cGUgaXMgYSBzdHJpbmdcbm1vZHVsZS5leHBvcnRzID0gZnVuY3Rpb24oaXQsIFMpe1xuICBpZighaXNPYmplY3QoaXQpKXJldHVybiBpdDtcbiAgdmFyIGZuLCB2YWw7XG4gIGlmKFMgJiYgdHlwZW9mIChmbiA9IGl0LnRvU3RyaW5nKSA9PSAnZnVuY3Rpb24nICYmICFpc09iamVjdCh2YWwgPSBmbi5jYWxsKGl0KSkpcmV0dXJuIHZhbDtcbiAgaWYodHlwZW9mIChmbiA9IGl0LnZhbHVlT2YpID09ICdmdW5jdGlvbicgJiYgIWlzT2JqZWN0KHZhbCA9IGZuLmNhbGwoaXQpKSlyZXR1cm4gdmFsO1xuICBpZighUyAmJiB0eXBlb2YgKGZuID0gaXQudG9TdHJpbmcpID09ICdmdW5jdGlvbicgJiYgIWlzT2JqZWN0KHZhbCA9IGZuLmNhbGwoaXQpKSlyZXR1cm4gdmFsO1xuICB0aHJvdyBUeXBlRXJyb3IoXCJDYW4ndCBjb252ZXJ0IG9iamVjdCB0byBwcmltaXRpdmUgdmFsdWVcIik7XG59OyIsInZhciBhbk9iamVjdCAgICAgICA9IHJlcXVpcmUoJy4vX2FuLW9iamVjdCcpXG4gICwgSUU4X0RPTV9ERUZJTkUgPSByZXF1aXJlKCcuL19pZTgtZG9tLWRlZmluZScpXG4gICwgdG9QcmltaXRpdmUgICAgPSByZXF1aXJlKCcuL190by1wcmltaXRpdmUnKVxuICAsIGRQICAgICAgICAgICAgID0gT2JqZWN0LmRlZmluZVByb3BlcnR5O1xuXG5leHBvcnRzLmYgPSByZXF1aXJlKCcuL19kZXNjcmlwdG9ycycpID8gT2JqZWN0LmRlZmluZVByb3BlcnR5IDogZnVuY3Rpb24gZGVmaW5lUHJvcGVydHkoTywgUCwgQXR0cmlidXRlcyl7XG4gIGFuT2JqZWN0KE8pO1xuICBQID0gdG9QcmltaXRpdmUoUCwgdHJ1ZSk7XG4gIGFuT2JqZWN0KEF0dHJpYnV0ZXMpO1xuICBpZihJRThfRE9NX0RFRklORSl0cnkge1xuICAgIHJldHVybiBkUChPLCBQLCBBdHRyaWJ1dGVzKTtcbiAgfSBjYXRjaChlKXsgLyogZW1wdHkgKi8gfVxuICBpZignZ2V0JyBpbiBBdHRyaWJ1dGVzIHx8ICdzZXQnIGluIEF0dHJpYnV0ZXMpdGhyb3cgVHlwZUVycm9yKCdBY2Nlc3NvcnMgbm90IHN1cHBvcnRlZCEnKTtcbiAgaWYoJ3ZhbHVlJyBpbiBBdHRyaWJ1dGVzKU9bUF0gPSBBdHRyaWJ1dGVzLnZhbHVlO1xuICByZXR1cm4gTztcbn07IiwibW9kdWxlLmV4cG9ydHMgPSBmdW5jdGlvbihiaXRtYXAsIHZhbHVlKXtcbiAgcmV0dXJuIHtcbiAgICBlbnVtZXJhYmxlICA6ICEoYml0bWFwICYgMSksXG4gICAgY29uZmlndXJhYmxlOiAhKGJpdG1hcCAmIDIpLFxuICAgIHdyaXRhYmxlICAgIDogIShiaXRtYXAgJiA0KSxcbiAgICB2YWx1ZSAgICAgICA6IHZhbHVlXG4gIH07XG59OyIsInZhciBkUCAgICAgICAgID0gcmVxdWlyZSgnLi9fb2JqZWN0LWRwJylcbiAgLCBjcmVhdGVEZXNjID0gcmVxdWlyZSgnLi9fcHJvcGVydHktZGVzYycpO1xubW9kdWxlLmV4cG9ydHMgPSByZXF1aXJlKCcuL19kZXNjcmlwdG9ycycpID8gZnVuY3Rpb24ob2JqZWN0LCBrZXksIHZhbHVlKXtcbiAgcmV0dXJuIGRQLmYob2JqZWN0LCBrZXksIGNyZWF0ZURlc2MoMSwgdmFsdWUpKTtcbn0gOiBmdW5jdGlvbihvYmplY3QsIGtleSwgdmFsdWUpe1xuICBvYmplY3Rba2V5XSA9IHZhbHVlO1xuICByZXR1cm4gb2JqZWN0O1xufTsiLCJ2YXIgaGFzT3duUHJvcGVydHkgPSB7fS5oYXNPd25Qcm9wZXJ0eTtcbm1vZHVsZS5leHBvcnRzID0gZnVuY3Rpb24oaXQsIGtleSl7XG4gIHJldHVybiBoYXNPd25Qcm9wZXJ0eS5jYWxsKGl0LCBrZXkpO1xufTsiLCJ2YXIgaWQgPSAwXG4gICwgcHggPSBNYXRoLnJhbmRvbSgpO1xubW9kdWxlLmV4cG9ydHMgPSBmdW5jdGlvbihrZXkpe1xuICByZXR1cm4gJ1N5bWJvbCgnLmNvbmNhdChrZXkgPT09IHVuZGVmaW5lZCA/ICcnIDoga2V5LCAnKV8nLCAoKytpZCArIHB4KS50b1N0cmluZygzNikpO1xufTsiLCJ2YXIgZ2xvYmFsICAgID0gcmVxdWlyZSgnLi9fZ2xvYmFsJylcbiAgLCBoaWRlICAgICAgPSByZXF1aXJlKCcuL19oaWRlJylcbiAgLCBoYXMgICAgICAgPSByZXF1aXJlKCcuL19oYXMnKVxuICAsIFNSQyAgICAgICA9IHJlcXVpcmUoJy4vX3VpZCcpKCdzcmMnKVxuICAsIFRPX1NUUklORyA9ICd0b1N0cmluZydcbiAgLCAkdG9TdHJpbmcgPSBGdW5jdGlvbltUT19TVFJJTkddXG4gICwgVFBMICAgICAgID0gKCcnICsgJHRvU3RyaW5nKS5zcGxpdChUT19TVFJJTkcpO1xuXG5yZXF1aXJlKCcuL19jb3JlJykuaW5zcGVjdFNvdXJjZSA9IGZ1bmN0aW9uKGl0KXtcbiAgcmV0dXJuICR0b1N0cmluZy5jYWxsKGl0KTtcbn07XG5cbihtb2R1bGUuZXhwb3J0cyA9IGZ1bmN0aW9uKE8sIGtleSwgdmFsLCBzYWZlKXtcbiAgdmFyIGlzRnVuY3Rpb24gPSB0eXBlb2YgdmFsID09ICdmdW5jdGlvbic7XG4gIGlmKGlzRnVuY3Rpb24paGFzKHZhbCwgJ25hbWUnKSB8fCBoaWRlKHZhbCwgJ25hbWUnLCBrZXkpO1xuICBpZihPW2tleV0gPT09IHZhbClyZXR1cm47XG4gIGlmKGlzRnVuY3Rpb24paGFzKHZhbCwgU1JDKSB8fCBoaWRlKHZhbCwgU1JDLCBPW2tleV0gPyAnJyArIE9ba2V5XSA6IFRQTC5qb2luKFN0cmluZyhrZXkpKSk7XG4gIGlmKE8gPT09IGdsb2JhbCl7XG4gICAgT1trZXldID0gdmFsO1xuICB9IGVsc2Uge1xuICAgIGlmKCFzYWZlKXtcbiAgICAgIGRlbGV0ZSBPW2tleV07XG4gICAgICBoaWRlKE8sIGtleSwgdmFsKTtcbiAgICB9IGVsc2Uge1xuICAgICAgaWYoT1trZXldKU9ba2V5XSA9IHZhbDtcbiAgICAgIGVsc2UgaGlkZShPLCBrZXksIHZhbCk7XG4gICAgfVxuICB9XG4vLyBhZGQgZmFrZSBGdW5jdGlvbiN0b1N0cmluZyBmb3IgY29ycmVjdCB3b3JrIHdyYXBwZWQgbWV0aG9kcyAvIGNvbnN0cnVjdG9ycyB3aXRoIG1ldGhvZHMgbGlrZSBMb0Rhc2ggaXNOYXRpdmVcbn0pKEZ1bmN0aW9uLnByb3RvdHlwZSwgVE9fU1RSSU5HLCBmdW5jdGlvbiB0b1N0cmluZygpe1xuICByZXR1cm4gdHlwZW9mIHRoaXMgPT0gJ2Z1bmN0aW9uJyAmJiB0aGlzW1NSQ10gfHwgJHRvU3RyaW5nLmNhbGwodGhpcyk7XG59KTsiLCJtb2R1bGUuZXhwb3J0cyA9IGZ1bmN0aW9uKGl0KXtcbiAgaWYodHlwZW9mIGl0ICE9ICdmdW5jdGlvbicpdGhyb3cgVHlwZUVycm9yKGl0ICsgJyBpcyBub3QgYSBmdW5jdGlvbiEnKTtcbiAgcmV0dXJuIGl0O1xufTsiLCIvLyBvcHRpb25hbCAvIHNpbXBsZSBjb250ZXh0IGJpbmRpbmdcbnZhciBhRnVuY3Rpb24gPSByZXF1aXJlKCcuL19hLWZ1bmN0aW9uJyk7XG5tb2R1bGUuZXhwb3J0cyA9IGZ1bmN0aW9uKGZuLCB0aGF0LCBsZW5ndGgpe1xuICBhRnVuY3Rpb24oZm4pO1xuICBpZih0aGF0ID09PSB1bmRlZmluZWQpcmV0dXJuIGZuO1xuICBzd2l0Y2gobGVuZ3RoKXtcbiAgICBjYXNlIDE6IHJldHVybiBmdW5jdGlvbihhKXtcbiAgICAgIHJldHVybiBmbi5jYWxsKHRoYXQsIGEpO1xuICAgIH07XG4gICAgY2FzZSAyOiByZXR1cm4gZnVuY3Rpb24oYSwgYil7XG4gICAgICByZXR1cm4gZm4uY2FsbCh0aGF0LCBhLCBiKTtcbiAgICB9O1xuICAgIGNhc2UgMzogcmV0dXJuIGZ1bmN0aW9uKGEsIGIsIGMpe1xuICAgICAgcmV0dXJuIGZuLmNhbGwodGhhdCwgYSwgYiwgYyk7XG4gICAgfTtcbiAgfVxuICByZXR1cm4gZnVuY3Rpb24oLyogLi4uYXJncyAqLyl7XG4gICAgcmV0dXJuIGZuLmFwcGx5KHRoYXQsIGFyZ3VtZW50cyk7XG4gIH07XG59OyIsInZhciBnbG9iYWwgICAgPSByZXF1aXJlKCcuL19nbG9iYWwnKVxuICAsIGNvcmUgICAgICA9IHJlcXVpcmUoJy4vX2NvcmUnKVxuICAsIGhpZGUgICAgICA9IHJlcXVpcmUoJy4vX2hpZGUnKVxuICAsIHJlZGVmaW5lICA9IHJlcXVpcmUoJy4vX3JlZGVmaW5lJylcbiAgLCBjdHggICAgICAgPSByZXF1aXJlKCcuL19jdHgnKVxuICAsIFBST1RPVFlQRSA9ICdwcm90b3R5cGUnO1xuXG52YXIgJGV4cG9ydCA9IGZ1bmN0aW9uKHR5cGUsIG5hbWUsIHNvdXJjZSl7XG4gIHZhciBJU19GT1JDRUQgPSB0eXBlICYgJGV4cG9ydC5GXG4gICAgLCBJU19HTE9CQUwgPSB0eXBlICYgJGV4cG9ydC5HXG4gICAgLCBJU19TVEFUSUMgPSB0eXBlICYgJGV4cG9ydC5TXG4gICAgLCBJU19QUk9UTyAgPSB0eXBlICYgJGV4cG9ydC5QXG4gICAgLCBJU19CSU5EICAgPSB0eXBlICYgJGV4cG9ydC5CXG4gICAgLCB0YXJnZXQgICAgPSBJU19HTE9CQUwgPyBnbG9iYWwgOiBJU19TVEFUSUMgPyBnbG9iYWxbbmFtZV0gfHwgKGdsb2JhbFtuYW1lXSA9IHt9KSA6IChnbG9iYWxbbmFtZV0gfHwge30pW1BST1RPVFlQRV1cbiAgICAsIGV4cG9ydHMgICA9IElTX0dMT0JBTCA/IGNvcmUgOiBjb3JlW25hbWVdIHx8IChjb3JlW25hbWVdID0ge30pXG4gICAgLCBleHBQcm90byAgPSBleHBvcnRzW1BST1RPVFlQRV0gfHwgKGV4cG9ydHNbUFJPVE9UWVBFXSA9IHt9KVxuICAgICwga2V5LCBvd24sIG91dCwgZXhwO1xuICBpZihJU19HTE9CQUwpc291cmNlID0gbmFtZTtcbiAgZm9yKGtleSBpbiBzb3VyY2Upe1xuICAgIC8vIGNvbnRhaW5zIGluIG5hdGl2ZVxuICAgIG93biA9ICFJU19GT1JDRUQgJiYgdGFyZ2V0ICYmIHRhcmdldFtrZXldICE9PSB1bmRlZmluZWQ7XG4gICAgLy8gZXhwb3J0IG5hdGl2ZSBvciBwYXNzZWRcbiAgICBvdXQgPSAob3duID8gdGFyZ2V0IDogc291cmNlKVtrZXldO1xuICAgIC8vIGJpbmQgdGltZXJzIHRvIGdsb2JhbCBmb3IgY2FsbCBmcm9tIGV4cG9ydCBjb250ZXh0XG4gICAgZXhwID0gSVNfQklORCAmJiBvd24gPyBjdHgob3V0LCBnbG9iYWwpIDogSVNfUFJPVE8gJiYgdHlwZW9mIG91dCA9PSAnZnVuY3Rpb24nID8gY3R4KEZ1bmN0aW9uLmNhbGwsIG91dCkgOiBvdXQ7XG4gICAgLy8gZXh0ZW5kIGdsb2JhbFxuICAgIGlmKHRhcmdldClyZWRlZmluZSh0YXJnZXQsIGtleSwgb3V0LCB0eXBlICYgJGV4cG9ydC5VKTtcbiAgICAvLyBleHBvcnRcbiAgICBpZihleHBvcnRzW2tleV0gIT0gb3V0KWhpZGUoZXhwb3J0cywga2V5LCBleHApO1xuICAgIGlmKElTX1BST1RPICYmIGV4cFByb3RvW2tleV0gIT0gb3V0KWV4cFByb3RvW2tleV0gPSBvdXQ7XG4gIH1cbn07XG5nbG9iYWwuY29yZSA9IGNvcmU7XG4vLyB0eXBlIGJpdG1hcFxuJGV4cG9ydC5GID0gMTsgICAvLyBmb3JjZWRcbiRleHBvcnQuRyA9IDI7ICAgLy8gZ2xvYmFsXG4kZXhwb3J0LlMgPSA0OyAgIC8vIHN0YXRpY1xuJGV4cG9ydC5QID0gODsgICAvLyBwcm90b1xuJGV4cG9ydC5CID0gMTY7ICAvLyBiaW5kXG4kZXhwb3J0LlcgPSAzMjsgIC8vIHdyYXBcbiRleHBvcnQuVSA9IDY0OyAgLy8gc2FmZVxuJGV4cG9ydC5SID0gMTI4OyAvLyByZWFsIHByb3RvIG1ldGhvZCBmb3IgYGxpYnJhcnlgIFxubW9kdWxlLmV4cG9ydHMgPSAkZXhwb3J0OyIsInZhciB0b1N0cmluZyA9IHt9LnRvU3RyaW5nO1xuXG5tb2R1bGUuZXhwb3J0cyA9IGZ1bmN0aW9uKGl0KXtcbiAgcmV0dXJuIHRvU3RyaW5nLmNhbGwoaXQpLnNsaWNlKDgsIC0xKTtcbn07IiwiLy8gZmFsbGJhY2sgZm9yIG5vbi1hcnJheS1saWtlIEVTMyBhbmQgbm9uLWVudW1lcmFibGUgb2xkIFY4IHN0cmluZ3NcbnZhciBjb2YgPSByZXF1aXJlKCcuL19jb2YnKTtcbm1vZHVsZS5leHBvcnRzID0gT2JqZWN0KCd6JykucHJvcGVydHlJc0VudW1lcmFibGUoMCkgPyBPYmplY3QgOiBmdW5jdGlvbihpdCl7XG4gIHJldHVybiBjb2YoaXQpID09ICdTdHJpbmcnID8gaXQuc3BsaXQoJycpIDogT2JqZWN0KGl0KTtcbn07IiwiLy8gNy4yLjEgUmVxdWlyZU9iamVjdENvZXJjaWJsZShhcmd1bWVudClcbm1vZHVsZS5leHBvcnRzID0gZnVuY3Rpb24oaXQpe1xuICBpZihpdCA9PSB1bmRlZmluZWQpdGhyb3cgVHlwZUVycm9yKFwiQ2FuJ3QgY2FsbCBtZXRob2Qgb24gIFwiICsgaXQpO1xuICByZXR1cm4gaXQ7XG59OyIsIi8vIHRvIGluZGV4ZWQgb2JqZWN0LCB0b09iamVjdCB3aXRoIGZhbGxiYWNrIGZvciBub24tYXJyYXktbGlrZSBFUzMgc3RyaW5nc1xudmFyIElPYmplY3QgPSByZXF1aXJlKCcuL19pb2JqZWN0JylcbiAgLCBkZWZpbmVkID0gcmVxdWlyZSgnLi9fZGVmaW5lZCcpO1xubW9kdWxlLmV4cG9ydHMgPSBmdW5jdGlvbihpdCl7XG4gIHJldHVybiBJT2JqZWN0KGRlZmluZWQoaXQpKTtcbn07IiwiLy8gNy4xLjQgVG9JbnRlZ2VyXG52YXIgY2VpbCAgPSBNYXRoLmNlaWxcbiAgLCBmbG9vciA9IE1hdGguZmxvb3I7XG5tb2R1bGUuZXhwb3J0cyA9IGZ1bmN0aW9uKGl0KXtcbiAgcmV0dXJuIGlzTmFOKGl0ID0gK2l0KSA/IDAgOiAoaXQgPiAwID8gZmxvb3IgOiBjZWlsKShpdCk7XG59OyIsIi8vIDcuMS4xNSBUb0xlbmd0aFxudmFyIHRvSW50ZWdlciA9IHJlcXVpcmUoJy4vX3RvLWludGVnZXInKVxuICAsIG1pbiAgICAgICA9IE1hdGgubWluO1xubW9kdWxlLmV4cG9ydHMgPSBmdW5jdGlvbihpdCl7XG4gIHJldHVybiBpdCA+IDAgPyBtaW4odG9JbnRlZ2VyKGl0KSwgMHgxZmZmZmZmZmZmZmZmZikgOiAwOyAvLyBwb3coMiwgNTMpIC0gMSA9PSA5MDA3MTk5MjU0NzQwOTkxXG59OyIsInZhciB0b0ludGVnZXIgPSByZXF1aXJlKCcuL190by1pbnRlZ2VyJylcbiAgLCBtYXggICAgICAgPSBNYXRoLm1heFxuICAsIG1pbiAgICAgICA9IE1hdGgubWluO1xubW9kdWxlLmV4cG9ydHMgPSBmdW5jdGlvbihpbmRleCwgbGVuZ3RoKXtcbiAgaW5kZXggPSB0b0ludGVnZXIoaW5kZXgpO1xuICByZXR1cm4gaW5kZXggPCAwID8gbWF4KGluZGV4ICsgbGVuZ3RoLCAwKSA6IG1pbihpbmRleCwgbGVuZ3RoKTtcbn07IiwiLy8gZmFsc2UgLT4gQXJyYXkjaW5kZXhPZlxuLy8gdHJ1ZSAgLT4gQXJyYXkjaW5jbHVkZXNcbnZhciB0b0lPYmplY3QgPSByZXF1aXJlKCcuL190by1pb2JqZWN0JylcbiAgLCB0b0xlbmd0aCAgPSByZXF1aXJlKCcuL190by1sZW5ndGgnKVxuICAsIHRvSW5kZXggICA9IHJlcXVpcmUoJy4vX3RvLWluZGV4Jyk7XG5tb2R1bGUuZXhwb3J0cyA9IGZ1bmN0aW9uKElTX0lOQ0xVREVTKXtcbiAgcmV0dXJuIGZ1bmN0aW9uKCR0aGlzLCBlbCwgZnJvbUluZGV4KXtcbiAgICB2YXIgTyAgICAgID0gdG9JT2JqZWN0KCR0aGlzKVxuICAgICAgLCBsZW5ndGggPSB0b0xlbmd0aChPLmxlbmd0aClcbiAgICAgICwgaW5kZXggID0gdG9JbmRleChmcm9tSW5kZXgsIGxlbmd0aClcbiAgICAgICwgdmFsdWU7XG4gICAgLy8gQXJyYXkjaW5jbHVkZXMgdXNlcyBTYW1lVmFsdWVaZXJvIGVxdWFsaXR5IGFsZ29yaXRobVxuICAgIGlmKElTX0lOQ0xVREVTICYmIGVsICE9IGVsKXdoaWxlKGxlbmd0aCA+IGluZGV4KXtcbiAgICAgIHZhbHVlID0gT1tpbmRleCsrXTtcbiAgICAgIGlmKHZhbHVlICE9IHZhbHVlKXJldHVybiB0cnVlO1xuICAgIC8vIEFycmF5I3RvSW5kZXggaWdub3JlcyBob2xlcywgQXJyYXkjaW5jbHVkZXMgLSBub3RcbiAgICB9IGVsc2UgZm9yKDtsZW5ndGggPiBpbmRleDsgaW5kZXgrKylpZihJU19JTkNMVURFUyB8fCBpbmRleCBpbiBPKXtcbiAgICAgIGlmKE9baW5kZXhdID09PSBlbClyZXR1cm4gSVNfSU5DTFVERVMgfHwgaW5kZXggfHwgMDtcbiAgICB9IHJldHVybiAhSVNfSU5DTFVERVMgJiYgLTE7XG4gIH07XG59OyIsInZhciBnbG9iYWwgPSByZXF1aXJlKCcuL19nbG9iYWwnKVxuICAsIFNIQVJFRCA9ICdfX2NvcmUtanNfc2hhcmVkX18nXG4gICwgc3RvcmUgID0gZ2xvYmFsW1NIQVJFRF0gfHwgKGdsb2JhbFtTSEFSRURdID0ge30pO1xubW9kdWxlLmV4cG9ydHMgPSBmdW5jdGlvbihrZXkpe1xuICByZXR1cm4gc3RvcmVba2V5XSB8fCAoc3RvcmVba2V5XSA9IHt9KTtcbn07IiwidmFyIHNoYXJlZCA9IHJlcXVpcmUoJy4vX3NoYXJlZCcpKCdrZXlzJylcbiAgLCB1aWQgICAgPSByZXF1aXJlKCcuL191aWQnKTtcbm1vZHVsZS5leHBvcnRzID0gZnVuY3Rpb24oa2V5KXtcbiAgcmV0dXJuIHNoYXJlZFtrZXldIHx8IChzaGFyZWRba2V5XSA9IHVpZChrZXkpKTtcbn07IiwidmFyIGhhcyAgICAgICAgICA9IHJlcXVpcmUoJy4vX2hhcycpXG4gICwgdG9JT2JqZWN0ICAgID0gcmVxdWlyZSgnLi9fdG8taW9iamVjdCcpXG4gICwgYXJyYXlJbmRleE9mID0gcmVxdWlyZSgnLi9fYXJyYXktaW5jbHVkZXMnKShmYWxzZSlcbiAgLCBJRV9QUk9UTyAgICAgPSByZXF1aXJlKCcuL19zaGFyZWQta2V5JykoJ0lFX1BST1RPJyk7XG5cbm1vZHVsZS5leHBvcnRzID0gZnVuY3Rpb24ob2JqZWN0LCBuYW1lcyl7XG4gIHZhciBPICAgICAgPSB0b0lPYmplY3Qob2JqZWN0KVxuICAgICwgaSAgICAgID0gMFxuICAgICwgcmVzdWx0ID0gW11cbiAgICAsIGtleTtcbiAgZm9yKGtleSBpbiBPKWlmKGtleSAhPSBJRV9QUk9UTyloYXMoTywga2V5KSAmJiByZXN1bHQucHVzaChrZXkpO1xuICAvLyBEb24ndCBlbnVtIGJ1ZyAmIGhpZGRlbiBrZXlzXG4gIHdoaWxlKG5hbWVzLmxlbmd0aCA+IGkpaWYoaGFzKE8sIGtleSA9IG5hbWVzW2krK10pKXtcbiAgICB+YXJyYXlJbmRleE9mKHJlc3VsdCwga2V5KSB8fCByZXN1bHQucHVzaChrZXkpO1xuICB9XG4gIHJldHVybiByZXN1bHQ7XG59OyIsIi8vIElFIDgtIGRvbid0IGVudW0gYnVnIGtleXNcbm1vZHVsZS5leHBvcnRzID0gKFxuICAnY29uc3RydWN0b3IsaGFzT3duUHJvcGVydHksaXNQcm90b3R5cGVPZixwcm9wZXJ0eUlzRW51bWVyYWJsZSx0b0xvY2FsZVN0cmluZyx0b1N0cmluZyx2YWx1ZU9mJ1xuKS5zcGxpdCgnLCcpOyIsIi8vIDE5LjEuMi4xNCAvIDE1LjIuMy4xNCBPYmplY3Qua2V5cyhPKVxudmFyICRrZXlzICAgICAgID0gcmVxdWlyZSgnLi9fb2JqZWN0LWtleXMtaW50ZXJuYWwnKVxuICAsIGVudW1CdWdLZXlzID0gcmVxdWlyZSgnLi9fZW51bS1idWcta2V5cycpO1xuXG5tb2R1bGUuZXhwb3J0cyA9IE9iamVjdC5rZXlzIHx8IGZ1bmN0aW9uIGtleXMoTyl7XG4gIHJldHVybiAka2V5cyhPLCBlbnVtQnVnS2V5cyk7XG59OyIsImV4cG9ydHMuZiA9IE9iamVjdC5nZXRPd25Qcm9wZXJ0eVN5bWJvbHM7IiwiZXhwb3J0cy5mID0ge30ucHJvcGVydHlJc0VudW1lcmFibGU7IiwiLy8gNy4xLjEzIFRvT2JqZWN0KGFyZ3VtZW50KVxudmFyIGRlZmluZWQgPSByZXF1aXJlKCcuL19kZWZpbmVkJyk7XG5tb2R1bGUuZXhwb3J0cyA9IGZ1bmN0aW9uKGl0KXtcbiAgcmV0dXJuIE9iamVjdChkZWZpbmVkKGl0KSk7XG59OyIsIid1c2Ugc3RyaWN0Jztcbi8vIDE5LjEuMi4xIE9iamVjdC5hc3NpZ24odGFyZ2V0LCBzb3VyY2UsIC4uLilcbnZhciBnZXRLZXlzICA9IHJlcXVpcmUoJy4vX29iamVjdC1rZXlzJylcbiAgLCBnT1BTICAgICA9IHJlcXVpcmUoJy4vX29iamVjdC1nb3BzJylcbiAgLCBwSUUgICAgICA9IHJlcXVpcmUoJy4vX29iamVjdC1waWUnKVxuICAsIHRvT2JqZWN0ID0gcmVxdWlyZSgnLi9fdG8tb2JqZWN0JylcbiAgLCBJT2JqZWN0ICA9IHJlcXVpcmUoJy4vX2lvYmplY3QnKVxuICAsICRhc3NpZ24gID0gT2JqZWN0LmFzc2lnbjtcblxuLy8gc2hvdWxkIHdvcmsgd2l0aCBzeW1ib2xzIGFuZCBzaG91bGQgaGF2ZSBkZXRlcm1pbmlzdGljIHByb3BlcnR5IG9yZGVyIChWOCBidWcpXG5tb2R1bGUuZXhwb3J0cyA9ICEkYXNzaWduIHx8IHJlcXVpcmUoJy4vX2ZhaWxzJykoZnVuY3Rpb24oKXtcbiAgdmFyIEEgPSB7fVxuICAgICwgQiA9IHt9XG4gICAgLCBTID0gU3ltYm9sKClcbiAgICAsIEsgPSAnYWJjZGVmZ2hpamtsbW5vcHFyc3QnO1xuICBBW1NdID0gNztcbiAgSy5zcGxpdCgnJykuZm9yRWFjaChmdW5jdGlvbihrKXsgQltrXSA9IGs7IH0pO1xuICByZXR1cm4gJGFzc2lnbih7fSwgQSlbU10gIT0gNyB8fCBPYmplY3Qua2V5cygkYXNzaWduKHt9LCBCKSkuam9pbignJykgIT0gSztcbn0pID8gZnVuY3Rpb24gYXNzaWduKHRhcmdldCwgc291cmNlKXsgLy8gZXNsaW50LWRpc2FibGUtbGluZSBuby11bnVzZWQtdmFyc1xuICB2YXIgVCAgICAgPSB0b09iamVjdCh0YXJnZXQpXG4gICAgLCBhTGVuICA9IGFyZ3VtZW50cy5sZW5ndGhcbiAgICAsIGluZGV4ID0gMVxuICAgICwgZ2V0U3ltYm9scyA9IGdPUFMuZlxuICAgICwgaXNFbnVtICAgICA9IHBJRS5mO1xuICB3aGlsZShhTGVuID4gaW5kZXgpe1xuICAgIHZhciBTICAgICAgPSBJT2JqZWN0KGFyZ3VtZW50c1tpbmRleCsrXSlcbiAgICAgICwga2V5cyAgID0gZ2V0U3ltYm9scyA/IGdldEtleXMoUykuY29uY2F0KGdldFN5bWJvbHMoUykpIDogZ2V0S2V5cyhTKVxuICAgICAgLCBsZW5ndGggPSBrZXlzLmxlbmd0aFxuICAgICAgLCBqICAgICAgPSAwXG4gICAgICAsIGtleTtcbiAgICB3aGlsZShsZW5ndGggPiBqKWlmKGlzRW51bS5jYWxsKFMsIGtleSA9IGtleXNbaisrXSkpVFtrZXldID0gU1trZXldO1xuICB9IHJldHVybiBUO1xufSA6ICRhc3NpZ247IiwiLy8gMTkuMS4zLjEgT2JqZWN0LmFzc2lnbih0YXJnZXQsIHNvdXJjZSlcbnZhciAkZXhwb3J0ID0gcmVxdWlyZSgnLi9fZXhwb3J0Jyk7XG5cbiRleHBvcnQoJGV4cG9ydC5TICsgJGV4cG9ydC5GLCAnT2JqZWN0Jywge2Fzc2lnbjogcmVxdWlyZSgnLi9fb2JqZWN0LWFzc2lnbicpfSk7IiwiLypcbiAqIExpY2Vuc2VkIHRvIHRoZSBBcGFjaGUgU29mdHdhcmUgRm91bmRhdGlvbiAoQVNGKSB1bmRlciBvbmVcbiAqIG9yIG1vcmUgY29udHJpYnV0b3IgbGljZW5zZSBhZ3JlZW1lbnRzLiAgU2VlIHRoZSBOT1RJQ0UgZmlsZVxuICogZGlzdHJpYnV0ZWQgd2l0aCB0aGlzIHdvcmsgZm9yIGFkZGl0aW9uYWwgaW5mb3JtYXRpb25cbiAqIHJlZ2FyZGluZyBjb3B5cmlnaHQgb3duZXJzaGlwLiAgVGhlIEFTRiBsaWNlbnNlcyB0aGlzIGZpbGVcbiAqIHRvIHlvdSB1bmRlciB0aGUgQXBhY2hlIExpY2Vuc2UsIFZlcnNpb24gMi4wICh0aGVcbiAqIFwiTGljZW5zZVwiKTsgeW91IG1heSBub3QgdXNlIHRoaXMgZmlsZSBleGNlcHQgaW4gY29tcGxpYW5jZVxuICogd2l0aCB0aGUgTGljZW5zZS4gIFlvdSBtYXkgb2J0YWluIGEgY29weSBvZiB0aGUgTGljZW5zZSBhdFxuICpcbiAqICAgaHR0cDovL3d3dy5hcGFjaGUub3JnL2xpY2Vuc2VzL0xJQ0VOU0UtMi4wXG4gKlxuICogVW5sZXNzIHJlcXVpcmVkIGJ5IGFwcGxpY2FibGUgbGF3IG9yIGFncmVlZCB0byBpbiB3cml0aW5nLFxuICogc29mdHdhcmUgZGlzdHJpYnV0ZWQgdW5kZXIgdGhlIExpY2Vuc2UgaXMgZGlzdHJpYnV0ZWQgb24gYW5cbiAqIFwiQVMgSVNcIiBCQVNJUywgV0lUSE9VVCBXQVJSQU5USUVTIE9SIENPTkRJVElPTlMgT0YgQU5ZXG4gKiBLSU5ELCBlaXRoZXIgZXhwcmVzcyBvciBpbXBsaWVkLiAgU2VlIHRoZSBMaWNlbnNlIGZvciB0aGVcbiAqIHNwZWNpZmljIGxhbmd1YWdlIGdvdmVybmluZyBwZXJtaXNzaW9ucyBhbmQgbGltaXRhdGlvbnNcbiAqIHVuZGVyIHRoZSBMaWNlbnNlLlxuICovXG5pbXBvcnQgJ2NvcmUtanMvZm4vb2JqZWN0L2Fzc2lnbidcbiIsIi8qXG4gKiBMaWNlbnNlZCB0byB0aGUgQXBhY2hlIFNvZnR3YXJlIEZvdW5kYXRpb24gKEFTRikgdW5kZXIgb25lXG4gKiBvciBtb3JlIGNvbnRyaWJ1dG9yIGxpY2Vuc2UgYWdyZWVtZW50cy4gIFNlZSB0aGUgTk9USUNFIGZpbGVcbiAqIGRpc3RyaWJ1dGVkIHdpdGggdGhpcyB3b3JrIGZvciBhZGRpdGlvbmFsIGluZm9ybWF0aW9uXG4gKiByZWdhcmRpbmcgY29weXJpZ2h0IG93bmVyc2hpcC4gIFRoZSBBU0YgbGljZW5zZXMgdGhpcyBmaWxlXG4gKiB0byB5b3UgdW5kZXIgdGhlIEFwYWNoZSBMaWNlbnNlLCBWZXJzaW9uIDIuMCAodGhlXG4gKiBcIkxpY2Vuc2VcIik7IHlvdSBtYXkgbm90IHVzZSB0aGlzIGZpbGUgZXhjZXB0IGluIGNvbXBsaWFuY2VcbiAqIHdpdGggdGhlIExpY2Vuc2UuICBZb3UgbWF5IG9idGFpbiBhIGNvcHkgb2YgdGhlIExpY2Vuc2UgYXRcbiAqXG4gKiAgIGh0dHA6Ly93d3cuYXBhY2hlLm9yZy9saWNlbnNlcy9MSUNFTlNFLTIuMFxuICpcbiAqIFVubGVzcyByZXF1aXJlZCBieSBhcHBsaWNhYmxlIGxhdyBvciBhZ3JlZWQgdG8gaW4gd3JpdGluZyxcbiAqIHNvZnR3YXJlIGRpc3RyaWJ1dGVkIHVuZGVyIHRoZSBMaWNlbnNlIGlzIGRpc3RyaWJ1dGVkIG9uIGFuXG4gKiBcIkFTIElTXCIgQkFTSVMsIFdJVEhPVVQgV0FSUkFOVElFUyBPUiBDT05ESVRJT05TIE9GIEFOWVxuICogS0lORCwgZWl0aGVyIGV4cHJlc3Mgb3IgaW1wbGllZC4gIFNlZSB0aGUgTGljZW5zZSBmb3IgdGhlXG4gKiBzcGVjaWZpYyBsYW5ndWFnZSBnb3Zlcm5pbmcgcGVybWlzc2lvbnMgYW5kIGxpbWl0YXRpb25zXG4gKiB1bmRlciB0aGUgTGljZW5zZS5cbiAqL1xuXG4vKiBlc2xpbnQtZGlzYWJsZSAqL1xuXG4vLyBodHRwczovL2dpc3QuZ2l0aHViLmNvbS9XZWJSZWZsZWN0aW9uLzU1OTM1NTRcblxuLyogaXN0YW5idWwgaWdub3JlIGlmICovXG5pZiAoIU9iamVjdC5zZXRQcm90b3R5cGVPZikge1xuICBPYmplY3Quc2V0UHJvdG90eXBlT2YgPSAoZnVuY3Rpb24oT2JqZWN0LCBtYWdpYykge1xuICAgIHZhciBzZXQ7XG4gICAgZnVuY3Rpb24gc2V0UHJvdG90eXBlT2YoTywgcHJvdG8pIHtcbiAgICAgIHNldC5jYWxsKE8sIHByb3RvKTtcbiAgICAgIHJldHVybiBPO1xuICAgIH1cbiAgICB0cnkge1xuICAgICAgLy8gdGhpcyB3b3JrcyBhbHJlYWR5IGluIEZpcmVmb3ggYW5kIFNhZmFyaVxuICAgICAgc2V0ID0gT2JqZWN0LmdldE93blByb3BlcnR5RGVzY3JpcHRvcihPYmplY3QucHJvdG90eXBlLCBtYWdpYykuc2V0O1xuICAgICAgc2V0LmNhbGwoe30sIG51bGwpO1xuICAgIH0gY2F0Y2ggKGUpIHtcbiAgICAgIGlmIChcbiAgICAgICAgLy8gSUUgPCAxMSBjYW5ub3QgYmUgc2hpbW1lZFxuICAgICAgICBPYmplY3QucHJvdG90eXBlICE9PSB7fVttYWdpY10gfHxcbiAgICAgICAgLy8gbmVpdGhlciBjYW4gYW55IGJyb3dzZXIgdGhhdCBhY3R1YWxseVxuICAgICAgICAvLyBpbXBsZW1lbnRlZCBfX3Byb3RvX18gY29ycmVjdGx5XG4gICAgICAgIC8vIChhbGwgYnV0IG9sZCBWOCB3aWxsIHJldHVybiBoZXJlKVxuICAgICAgICB7X19wcm90b19fOiBudWxsfS5fX3Byb3RvX18gPT09IHZvaWQgMFxuICAgICAgICAvLyB0aGlzIGNhc2UgbWVhbnMgbnVsbCBvYmplY3RzIGNhbm5vdCBiZSBwYXNzZWRcbiAgICAgICAgLy8gdGhyb3VnaCBzZXRQcm90b3R5cGVPZiBpbiBhIHJlbGlhYmxlIHdheVxuICAgICAgICAvLyB3aGljaCBtZWFucyBoZXJlIGEgKipTaGFtKiogaXMgbmVlZGVkIGluc3RlYWRcbiAgICAgICkge1xuICAgICAgICByZXR1cm47XG4gICAgICB9XG4gICAgICAvLyBub2RlanMgMC44IGFuZCAwLjEwIGFyZSAoYnVnZ3kgYW5kLi4pIGZpbmUgaGVyZVxuICAgICAgLy8gcHJvYmFibHkgQ2hyb21lIG9yIHNvbWUgb2xkIE1vYmlsZSBzdG9jayBicm93c2VyXG4gICAgICBzZXQgPSBmdW5jdGlvbihwcm90bykge1xuICAgICAgICB0aGlzW21hZ2ljXSA9IHByb3RvO1xuICAgICAgfTtcbiAgICAgIC8vIHBsZWFzZSBub3RlIHRoYXQgdGhpcyB3aWxsICoqbm90Kiogd29ya1xuICAgICAgLy8gaW4gdGhvc2UgYnJvd3NlcnMgdGhhdCBkbyBub3QgaW5oZXJpdFxuICAgICAgLy8gX19wcm90b19fIGJ5IG1pc3Rha2UgZnJvbSBPYmplY3QucHJvdG90eXBlXG4gICAgICAvLyBpbiB0aGVzZSBjYXNlcyB3ZSBzaG91bGQgcHJvYmFibHkgdGhyb3cgYW4gZXJyb3JcbiAgICAgIC8vIG9yIGF0IGxlYXN0IGJlIGluZm9ybWVkIGFib3V0IHRoZSBpc3N1ZVxuICAgICAgc2V0UHJvdG90eXBlT2YucG9seWZpbGwgPSBzZXRQcm90b3R5cGVPZihcbiAgICAgICAgc2V0UHJvdG90eXBlT2Yoe30sIG51bGwpLFxuICAgICAgICBPYmplY3QucHJvdG90eXBlXG4gICAgICApIGluc3RhbmNlb2YgT2JqZWN0O1xuICAgICAgLy8gc2V0UHJvdG90eXBlT2YucG9seWZpbGwgPT09IHRydWUgbWVhbnMgaXQgd29ya3MgYXMgbWVhbnRcbiAgICAgIC8vIHNldFByb3RvdHlwZU9mLnBvbHlmaWxsID09PSBmYWxzZSBtZWFucyBpdCdzIG5vdCAxMDAlIHJlbGlhYmxlXG4gICAgICAvLyBzZXRQcm90b3R5cGVPZi5wb2x5ZmlsbCA9PT0gdW5kZWZpbmVkXG4gICAgICAvLyBvclxuICAgICAgLy8gc2V0UHJvdG90eXBlT2YucG9seWZpbGwgPT0gIG51bGwgbWVhbnMgaXQncyBub3QgYSBwb2x5ZmlsbFxuICAgICAgLy8gd2hpY2ggbWVhbnMgaXQgd29ya3MgYXMgZXhwZWN0ZWRcbiAgICAgIC8vIHdlIGNhbiBldmVuIGRlbGV0ZSBPYmplY3QucHJvdG90eXBlLl9fcHJvdG9fXztcbiAgICB9XG4gICAgcmV0dXJuIHNldFByb3RvdHlwZU9mO1xuICB9KE9iamVjdCwgJ19fcHJvdG9fXycpKTtcbn1cbiIsIi8qXG4gKiBMaWNlbnNlZCB0byB0aGUgQXBhY2hlIFNvZnR3YXJlIEZvdW5kYXRpb24gKEFTRikgdW5kZXIgb25lXG4gKiBvciBtb3JlIGNvbnRyaWJ1dG9yIGxpY2Vuc2UgYWdyZWVtZW50cy4gIFNlZSB0aGUgTk9USUNFIGZpbGVcbiAqIGRpc3RyaWJ1dGVkIHdpdGggdGhpcyB3b3JrIGZvciBhZGRpdGlvbmFsIGluZm9ybWF0aW9uXG4gKiByZWdhcmRpbmcgY29weXJpZ2h0IG93bmVyc2hpcC4gIFRoZSBBU0YgbGljZW5zZXMgdGhpcyBmaWxlXG4gKiB0byB5b3UgdW5kZXIgdGhlIEFwYWNoZSBMaWNlbnNlLCBWZXJzaW9uIDIuMCAodGhlXG4gKiBcIkxpY2Vuc2VcIik7IHlvdSBtYXkgbm90IHVzZSB0aGlzIGZpbGUgZXhjZXB0IGluIGNvbXBsaWFuY2VcbiAqIHdpdGggdGhlIExpY2Vuc2UuICBZb3UgbWF5IG9idGFpbiBhIGNvcHkgb2YgdGhlIExpY2Vuc2UgYXRcbiAqXG4gKiAgIGh0dHA6Ly93d3cuYXBhY2hlLm9yZy9saWNlbnNlcy9MSUNFTlNFLTIuMFxuICpcbiAqIFVubGVzcyByZXF1aXJlZCBieSBhcHBsaWNhYmxlIGxhdyBvciBhZ3JlZWQgdG8gaW4gd3JpdGluZyxcbiAqIHNvZnR3YXJlIGRpc3RyaWJ1dGVkIHVuZGVyIHRoZSBMaWNlbnNlIGlzIGRpc3RyaWJ1dGVkIG9uIGFuXG4gKiBcIkFTIElTXCIgQkFTSVMsIFdJVEhPVVQgV0FSUkFOVElFUyBPUiBDT05ESVRJT05TIE9GIEFOWVxuICogS0lORCwgZWl0aGVyIGV4cHJlc3Mgb3IgaW1wbGllZC4gIFNlZSB0aGUgTGljZW5zZSBmb3IgdGhlXG4gKiBzcGVjaWZpYyBsYW5ndWFnZSBnb3Zlcm5pbmcgcGVybWlzc2lvbnMgYW5kIGxpbWl0YXRpb25zXG4gKiB1bmRlciB0aGUgTGljZW5zZS5cbiAqL1xuXG4vLyBmaXggUHJvbWlzZSBQcm9ibGVtIG9uIEpTQ29udGV4dCBvZiBpT1M3fjhcbi8vIEBzZWUgaHR0cHM6Ly9idWdzLndlYmtpdC5vcmcvc2hvd19idWcuY2dpP2lkPTEzNTg2NlxuXG5jb25zdCB7IFdYRW52aXJvbm1lbnQgfSA9IGdsb2JhbFxuXG4vKiBpc3RhbmJ1bCBpZ25vcmUgbmV4dCAqL1xuaWYgKFdYRW52aXJvbm1lbnQgJiYgV1hFbnZpcm9ubWVudC5wbGF0Zm9ybSA9PT0gJ2lPUycpIHtcbiAgZ2xvYmFsLlByb21pc2UgPSB1bmRlZmluZWRcbn1cbiIsInZhciBzdG9yZSAgICAgID0gcmVxdWlyZSgnLi9fc2hhcmVkJykoJ3drcycpXG4gICwgdWlkICAgICAgICA9IHJlcXVpcmUoJy4vX3VpZCcpXG4gICwgU3ltYm9sICAgICA9IHJlcXVpcmUoJy4vX2dsb2JhbCcpLlN5bWJvbFxuICAsIFVTRV9TWU1CT0wgPSB0eXBlb2YgU3ltYm9sID09ICdmdW5jdGlvbic7XG5cbnZhciAkZXhwb3J0cyA9IG1vZHVsZS5leHBvcnRzID0gZnVuY3Rpb24obmFtZSl7XG4gIHJldHVybiBzdG9yZVtuYW1lXSB8fCAoc3RvcmVbbmFtZV0gPVxuICAgIFVTRV9TWU1CT0wgJiYgU3ltYm9sW25hbWVdIHx8IChVU0VfU1lNQk9MID8gU3ltYm9sIDogdWlkKSgnU3ltYm9sLicgKyBuYW1lKSk7XG59O1xuXG4kZXhwb3J0cy5zdG9yZSA9IHN0b3JlOyIsIi8vIGdldHRpbmcgdGFnIGZyb20gMTkuMS4zLjYgT2JqZWN0LnByb3RvdHlwZS50b1N0cmluZygpXG52YXIgY29mID0gcmVxdWlyZSgnLi9fY29mJylcbiAgLCBUQUcgPSByZXF1aXJlKCcuL193a3MnKSgndG9TdHJpbmdUYWcnKVxuICAvLyBFUzMgd3JvbmcgaGVyZVxuICAsIEFSRyA9IGNvZihmdW5jdGlvbigpeyByZXR1cm4gYXJndW1lbnRzOyB9KCkpID09ICdBcmd1bWVudHMnO1xuXG4vLyBmYWxsYmFjayBmb3IgSUUxMSBTY3JpcHQgQWNjZXNzIERlbmllZCBlcnJvclxudmFyIHRyeUdldCA9IGZ1bmN0aW9uKGl0LCBrZXkpe1xuICB0cnkge1xuICAgIHJldHVybiBpdFtrZXldO1xuICB9IGNhdGNoKGUpeyAvKiBlbXB0eSAqLyB9XG59O1xuXG5tb2R1bGUuZXhwb3J0cyA9IGZ1bmN0aW9uKGl0KXtcbiAgdmFyIE8sIFQsIEI7XG4gIHJldHVybiBpdCA9PT0gdW5kZWZpbmVkID8gJ1VuZGVmaW5lZCcgOiBpdCA9PT0gbnVsbCA/ICdOdWxsJ1xuICAgIC8vIEBAdG9TdHJpbmdUYWcgY2FzZVxuICAgIDogdHlwZW9mIChUID0gdHJ5R2V0KE8gPSBPYmplY3QoaXQpLCBUQUcpKSA9PSAnc3RyaW5nJyA/IFRcbiAgICAvLyBidWlsdGluVGFnIGNhc2VcbiAgICA6IEFSRyA/IGNvZihPKVxuICAgIC8vIEVTMyBhcmd1bWVudHMgZmFsbGJhY2tcbiAgICA6IChCID0gY29mKE8pKSA9PSAnT2JqZWN0JyAmJiB0eXBlb2YgTy5jYWxsZWUgPT0gJ2Z1bmN0aW9uJyA/ICdBcmd1bWVudHMnIDogQjtcbn07IiwiJ3VzZSBzdHJpY3QnO1xuLy8gMTkuMS4zLjYgT2JqZWN0LnByb3RvdHlwZS50b1N0cmluZygpXG52YXIgY2xhc3NvZiA9IHJlcXVpcmUoJy4vX2NsYXNzb2YnKVxuICAsIHRlc3QgICAgPSB7fTtcbnRlc3RbcmVxdWlyZSgnLi9fd2tzJykoJ3RvU3RyaW5nVGFnJyldID0gJ3onO1xuaWYodGVzdCArICcnICE9ICdbb2JqZWN0IHpdJyl7XG4gIHJlcXVpcmUoJy4vX3JlZGVmaW5lJykoT2JqZWN0LnByb3RvdHlwZSwgJ3RvU3RyaW5nJywgZnVuY3Rpb24gdG9TdHJpbmcoKXtcbiAgICByZXR1cm4gJ1tvYmplY3QgJyArIGNsYXNzb2YodGhpcykgKyAnXSc7XG4gIH0sIHRydWUpO1xufSIsInZhciB0b0ludGVnZXIgPSByZXF1aXJlKCcuL190by1pbnRlZ2VyJylcbiAgLCBkZWZpbmVkICAgPSByZXF1aXJlKCcuL19kZWZpbmVkJyk7XG4vLyB0cnVlICAtPiBTdHJpbmcjYXRcbi8vIGZhbHNlIC0+IFN0cmluZyNjb2RlUG9pbnRBdFxubW9kdWxlLmV4cG9ydHMgPSBmdW5jdGlvbihUT19TVFJJTkcpe1xuICByZXR1cm4gZnVuY3Rpb24odGhhdCwgcG9zKXtcbiAgICB2YXIgcyA9IFN0cmluZyhkZWZpbmVkKHRoYXQpKVxuICAgICAgLCBpID0gdG9JbnRlZ2VyKHBvcylcbiAgICAgICwgbCA9IHMubGVuZ3RoXG4gICAgICAsIGEsIGI7XG4gICAgaWYoaSA8IDAgfHwgaSA+PSBsKXJldHVybiBUT19TVFJJTkcgPyAnJyA6IHVuZGVmaW5lZDtcbiAgICBhID0gcy5jaGFyQ29kZUF0KGkpO1xuICAgIHJldHVybiBhIDwgMHhkODAwIHx8IGEgPiAweGRiZmYgfHwgaSArIDEgPT09IGwgfHwgKGIgPSBzLmNoYXJDb2RlQXQoaSArIDEpKSA8IDB4ZGMwMCB8fCBiID4gMHhkZmZmXG4gICAgICA/IFRPX1NUUklORyA/IHMuY2hhckF0KGkpIDogYVxuICAgICAgOiBUT19TVFJJTkcgPyBzLnNsaWNlKGksIGkgKyAyKSA6IChhIC0gMHhkODAwIDw8IDEwKSArIChiIC0gMHhkYzAwKSArIDB4MTAwMDA7XG4gIH07XG59OyIsIm1vZHVsZS5leHBvcnRzID0gZmFsc2U7IiwibW9kdWxlLmV4cG9ydHMgPSB7fTsiLCJ2YXIgZFAgICAgICAgPSByZXF1aXJlKCcuL19vYmplY3QtZHAnKVxuICAsIGFuT2JqZWN0ID0gcmVxdWlyZSgnLi9fYW4tb2JqZWN0JylcbiAgLCBnZXRLZXlzICA9IHJlcXVpcmUoJy4vX29iamVjdC1rZXlzJyk7XG5cbm1vZHVsZS5leHBvcnRzID0gcmVxdWlyZSgnLi9fZGVzY3JpcHRvcnMnKSA/IE9iamVjdC5kZWZpbmVQcm9wZXJ0aWVzIDogZnVuY3Rpb24gZGVmaW5lUHJvcGVydGllcyhPLCBQcm9wZXJ0aWVzKXtcbiAgYW5PYmplY3QoTyk7XG4gIHZhciBrZXlzICAgPSBnZXRLZXlzKFByb3BlcnRpZXMpXG4gICAgLCBsZW5ndGggPSBrZXlzLmxlbmd0aFxuICAgICwgaSA9IDBcbiAgICAsIFA7XG4gIHdoaWxlKGxlbmd0aCA+IGkpZFAuZihPLCBQID0ga2V5c1tpKytdLCBQcm9wZXJ0aWVzW1BdKTtcbiAgcmV0dXJuIE87XG59OyIsIm1vZHVsZS5leHBvcnRzID0gcmVxdWlyZSgnLi9fZ2xvYmFsJykuZG9jdW1lbnQgJiYgZG9jdW1lbnQuZG9jdW1lbnRFbGVtZW50OyIsIi8vIDE5LjEuMi4yIC8gMTUuMi4zLjUgT2JqZWN0LmNyZWF0ZShPIFssIFByb3BlcnRpZXNdKVxudmFyIGFuT2JqZWN0ICAgID0gcmVxdWlyZSgnLi9fYW4tb2JqZWN0JylcbiAgLCBkUHMgICAgICAgICA9IHJlcXVpcmUoJy4vX29iamVjdC1kcHMnKVxuICAsIGVudW1CdWdLZXlzID0gcmVxdWlyZSgnLi9fZW51bS1idWcta2V5cycpXG4gICwgSUVfUFJPVE8gICAgPSByZXF1aXJlKCcuL19zaGFyZWQta2V5JykoJ0lFX1BST1RPJylcbiAgLCBFbXB0eSAgICAgICA9IGZ1bmN0aW9uKCl7IC8qIGVtcHR5ICovIH1cbiAgLCBQUk9UT1RZUEUgICA9ICdwcm90b3R5cGUnO1xuXG4vLyBDcmVhdGUgb2JqZWN0IHdpdGggZmFrZSBgbnVsbGAgcHJvdG90eXBlOiB1c2UgaWZyYW1lIE9iamVjdCB3aXRoIGNsZWFyZWQgcHJvdG90eXBlXG52YXIgY3JlYXRlRGljdCA9IGZ1bmN0aW9uKCl7XG4gIC8vIFRocmFzaCwgd2FzdGUgYW5kIHNvZG9teTogSUUgR0MgYnVnXG4gIHZhciBpZnJhbWUgPSByZXF1aXJlKCcuL19kb20tY3JlYXRlJykoJ2lmcmFtZScpXG4gICAgLCBpICAgICAgPSBlbnVtQnVnS2V5cy5sZW5ndGhcbiAgICAsIGx0ICAgICA9ICc8J1xuICAgICwgZ3QgICAgID0gJz4nXG4gICAgLCBpZnJhbWVEb2N1bWVudDtcbiAgaWZyYW1lLnN0eWxlLmRpc3BsYXkgPSAnbm9uZSc7XG4gIHJlcXVpcmUoJy4vX2h0bWwnKS5hcHBlbmRDaGlsZChpZnJhbWUpO1xuICBpZnJhbWUuc3JjID0gJ2phdmFzY3JpcHQ6JzsgLy8gZXNsaW50LWRpc2FibGUtbGluZSBuby1zY3JpcHQtdXJsXG4gIC8vIGNyZWF0ZURpY3QgPSBpZnJhbWUuY29udGVudFdpbmRvdy5PYmplY3Q7XG4gIC8vIGh0bWwucmVtb3ZlQ2hpbGQoaWZyYW1lKTtcbiAgaWZyYW1lRG9jdW1lbnQgPSBpZnJhbWUuY29udGVudFdpbmRvdy5kb2N1bWVudDtcbiAgaWZyYW1lRG9jdW1lbnQub3BlbigpO1xuICBpZnJhbWVEb2N1bWVudC53cml0ZShsdCArICdzY3JpcHQnICsgZ3QgKyAnZG9jdW1lbnQuRj1PYmplY3QnICsgbHQgKyAnL3NjcmlwdCcgKyBndCk7XG4gIGlmcmFtZURvY3VtZW50LmNsb3NlKCk7XG4gIGNyZWF0ZURpY3QgPSBpZnJhbWVEb2N1bWVudC5GO1xuICB3aGlsZShpLS0pZGVsZXRlIGNyZWF0ZURpY3RbUFJPVE9UWVBFXVtlbnVtQnVnS2V5c1tpXV07XG4gIHJldHVybiBjcmVhdGVEaWN0KCk7XG59O1xuXG5tb2R1bGUuZXhwb3J0cyA9IE9iamVjdC5jcmVhdGUgfHwgZnVuY3Rpb24gY3JlYXRlKE8sIFByb3BlcnRpZXMpe1xuICB2YXIgcmVzdWx0O1xuICBpZihPICE9PSBudWxsKXtcbiAgICBFbXB0eVtQUk9UT1RZUEVdID0gYW5PYmplY3QoTyk7XG4gICAgcmVzdWx0ID0gbmV3IEVtcHR5O1xuICAgIEVtcHR5W1BST1RPVFlQRV0gPSBudWxsO1xuICAgIC8vIGFkZCBcIl9fcHJvdG9fX1wiIGZvciBPYmplY3QuZ2V0UHJvdG90eXBlT2YgcG9seWZpbGxcbiAgICByZXN1bHRbSUVfUFJPVE9dID0gTztcbiAgfSBlbHNlIHJlc3VsdCA9IGNyZWF0ZURpY3QoKTtcbiAgcmV0dXJuIFByb3BlcnRpZXMgPT09IHVuZGVmaW5lZCA/IHJlc3VsdCA6IGRQcyhyZXN1bHQsIFByb3BlcnRpZXMpO1xufTtcbiIsInZhciBkZWYgPSByZXF1aXJlKCcuL19vYmplY3QtZHAnKS5mXG4gICwgaGFzID0gcmVxdWlyZSgnLi9faGFzJylcbiAgLCBUQUcgPSByZXF1aXJlKCcuL193a3MnKSgndG9TdHJpbmdUYWcnKTtcblxubW9kdWxlLmV4cG9ydHMgPSBmdW5jdGlvbihpdCwgdGFnLCBzdGF0KXtcbiAgaWYoaXQgJiYgIWhhcyhpdCA9IHN0YXQgPyBpdCA6IGl0LnByb3RvdHlwZSwgVEFHKSlkZWYoaXQsIFRBRywge2NvbmZpZ3VyYWJsZTogdHJ1ZSwgdmFsdWU6IHRhZ30pO1xufTsiLCIndXNlIHN0cmljdCc7XG52YXIgY3JlYXRlICAgICAgICAgPSByZXF1aXJlKCcuL19vYmplY3QtY3JlYXRlJylcbiAgLCBkZXNjcmlwdG9yICAgICA9IHJlcXVpcmUoJy4vX3Byb3BlcnR5LWRlc2MnKVxuICAsIHNldFRvU3RyaW5nVGFnID0gcmVxdWlyZSgnLi9fc2V0LXRvLXN0cmluZy10YWcnKVxuICAsIEl0ZXJhdG9yUHJvdG90eXBlID0ge307XG5cbi8vIDI1LjEuMi4xLjEgJUl0ZXJhdG9yUHJvdG90eXBlJVtAQGl0ZXJhdG9yXSgpXG5yZXF1aXJlKCcuL19oaWRlJykoSXRlcmF0b3JQcm90b3R5cGUsIHJlcXVpcmUoJy4vX3drcycpKCdpdGVyYXRvcicpLCBmdW5jdGlvbigpeyByZXR1cm4gdGhpczsgfSk7XG5cbm1vZHVsZS5leHBvcnRzID0gZnVuY3Rpb24oQ29uc3RydWN0b3IsIE5BTUUsIG5leHQpe1xuICBDb25zdHJ1Y3Rvci5wcm90b3R5cGUgPSBjcmVhdGUoSXRlcmF0b3JQcm90b3R5cGUsIHtuZXh0OiBkZXNjcmlwdG9yKDEsIG5leHQpfSk7XG4gIHNldFRvU3RyaW5nVGFnKENvbnN0cnVjdG9yLCBOQU1FICsgJyBJdGVyYXRvcicpO1xufTsiLCIvLyAxOS4xLjIuOSAvIDE1LjIuMy4yIE9iamVjdC5nZXRQcm90b3R5cGVPZihPKVxudmFyIGhhcyAgICAgICAgID0gcmVxdWlyZSgnLi9faGFzJylcbiAgLCB0b09iamVjdCAgICA9IHJlcXVpcmUoJy4vX3RvLW9iamVjdCcpXG4gICwgSUVfUFJPVE8gICAgPSByZXF1aXJlKCcuL19zaGFyZWQta2V5JykoJ0lFX1BST1RPJylcbiAgLCBPYmplY3RQcm90byA9IE9iamVjdC5wcm90b3R5cGU7XG5cbm1vZHVsZS5leHBvcnRzID0gT2JqZWN0LmdldFByb3RvdHlwZU9mIHx8IGZ1bmN0aW9uKE8pe1xuICBPID0gdG9PYmplY3QoTyk7XG4gIGlmKGhhcyhPLCBJRV9QUk9UTykpcmV0dXJuIE9bSUVfUFJPVE9dO1xuICBpZih0eXBlb2YgTy5jb25zdHJ1Y3RvciA9PSAnZnVuY3Rpb24nICYmIE8gaW5zdGFuY2VvZiBPLmNvbnN0cnVjdG9yKXtcbiAgICByZXR1cm4gTy5jb25zdHJ1Y3Rvci5wcm90b3R5cGU7XG4gIH0gcmV0dXJuIE8gaW5zdGFuY2VvZiBPYmplY3QgPyBPYmplY3RQcm90byA6IG51bGw7XG59OyIsIid1c2Ugc3RyaWN0JztcbnZhciBMSUJSQVJZICAgICAgICA9IHJlcXVpcmUoJy4vX2xpYnJhcnknKVxuICAsICRleHBvcnQgICAgICAgID0gcmVxdWlyZSgnLi9fZXhwb3J0JylcbiAgLCByZWRlZmluZSAgICAgICA9IHJlcXVpcmUoJy4vX3JlZGVmaW5lJylcbiAgLCBoaWRlICAgICAgICAgICA9IHJlcXVpcmUoJy4vX2hpZGUnKVxuICAsIGhhcyAgICAgICAgICAgID0gcmVxdWlyZSgnLi9faGFzJylcbiAgLCBJdGVyYXRvcnMgICAgICA9IHJlcXVpcmUoJy4vX2l0ZXJhdG9ycycpXG4gICwgJGl0ZXJDcmVhdGUgICAgPSByZXF1aXJlKCcuL19pdGVyLWNyZWF0ZScpXG4gICwgc2V0VG9TdHJpbmdUYWcgPSByZXF1aXJlKCcuL19zZXQtdG8tc3RyaW5nLXRhZycpXG4gICwgZ2V0UHJvdG90eXBlT2YgPSByZXF1aXJlKCcuL19vYmplY3QtZ3BvJylcbiAgLCBJVEVSQVRPUiAgICAgICA9IHJlcXVpcmUoJy4vX3drcycpKCdpdGVyYXRvcicpXG4gICwgQlVHR1kgICAgICAgICAgPSAhKFtdLmtleXMgJiYgJ25leHQnIGluIFtdLmtleXMoKSkgLy8gU2FmYXJpIGhhcyBidWdneSBpdGVyYXRvcnMgdy9vIGBuZXh0YFxuICAsIEZGX0lURVJBVE9SICAgID0gJ0BAaXRlcmF0b3InXG4gICwgS0VZUyAgICAgICAgICAgPSAna2V5cydcbiAgLCBWQUxVRVMgICAgICAgICA9ICd2YWx1ZXMnO1xuXG52YXIgcmV0dXJuVGhpcyA9IGZ1bmN0aW9uKCl7IHJldHVybiB0aGlzOyB9O1xuXG5tb2R1bGUuZXhwb3J0cyA9IGZ1bmN0aW9uKEJhc2UsIE5BTUUsIENvbnN0cnVjdG9yLCBuZXh0LCBERUZBVUxULCBJU19TRVQsIEZPUkNFRCl7XG4gICRpdGVyQ3JlYXRlKENvbnN0cnVjdG9yLCBOQU1FLCBuZXh0KTtcbiAgdmFyIGdldE1ldGhvZCA9IGZ1bmN0aW9uKGtpbmQpe1xuICAgIGlmKCFCVUdHWSAmJiBraW5kIGluIHByb3RvKXJldHVybiBwcm90b1traW5kXTtcbiAgICBzd2l0Y2goa2luZCl7XG4gICAgICBjYXNlIEtFWVM6IHJldHVybiBmdW5jdGlvbiBrZXlzKCl7IHJldHVybiBuZXcgQ29uc3RydWN0b3IodGhpcywga2luZCk7IH07XG4gICAgICBjYXNlIFZBTFVFUzogcmV0dXJuIGZ1bmN0aW9uIHZhbHVlcygpeyByZXR1cm4gbmV3IENvbnN0cnVjdG9yKHRoaXMsIGtpbmQpOyB9O1xuICAgIH0gcmV0dXJuIGZ1bmN0aW9uIGVudHJpZXMoKXsgcmV0dXJuIG5ldyBDb25zdHJ1Y3Rvcih0aGlzLCBraW5kKTsgfTtcbiAgfTtcbiAgdmFyIFRBRyAgICAgICAgPSBOQU1FICsgJyBJdGVyYXRvcidcbiAgICAsIERFRl9WQUxVRVMgPSBERUZBVUxUID09IFZBTFVFU1xuICAgICwgVkFMVUVTX0JVRyA9IGZhbHNlXG4gICAgLCBwcm90byAgICAgID0gQmFzZS5wcm90b3R5cGVcbiAgICAsICRuYXRpdmUgICAgPSBwcm90b1tJVEVSQVRPUl0gfHwgcHJvdG9bRkZfSVRFUkFUT1JdIHx8IERFRkFVTFQgJiYgcHJvdG9bREVGQVVMVF1cbiAgICAsICRkZWZhdWx0ICAgPSAkbmF0aXZlIHx8IGdldE1ldGhvZChERUZBVUxUKVxuICAgICwgJGVudHJpZXMgICA9IERFRkFVTFQgPyAhREVGX1ZBTFVFUyA/ICRkZWZhdWx0IDogZ2V0TWV0aG9kKCdlbnRyaWVzJykgOiB1bmRlZmluZWRcbiAgICAsICRhbnlOYXRpdmUgPSBOQU1FID09ICdBcnJheScgPyBwcm90by5lbnRyaWVzIHx8ICRuYXRpdmUgOiAkbmF0aXZlXG4gICAgLCBtZXRob2RzLCBrZXksIEl0ZXJhdG9yUHJvdG90eXBlO1xuICAvLyBGaXggbmF0aXZlXG4gIGlmKCRhbnlOYXRpdmUpe1xuICAgIEl0ZXJhdG9yUHJvdG90eXBlID0gZ2V0UHJvdG90eXBlT2YoJGFueU5hdGl2ZS5jYWxsKG5ldyBCYXNlKSk7XG4gICAgaWYoSXRlcmF0b3JQcm90b3R5cGUgIT09IE9iamVjdC5wcm90b3R5cGUpe1xuICAgICAgLy8gU2V0IEBAdG9TdHJpbmdUYWcgdG8gbmF0aXZlIGl0ZXJhdG9yc1xuICAgICAgc2V0VG9TdHJpbmdUYWcoSXRlcmF0b3JQcm90b3R5cGUsIFRBRywgdHJ1ZSk7XG4gICAgICAvLyBmaXggZm9yIHNvbWUgb2xkIGVuZ2luZXNcbiAgICAgIGlmKCFMSUJSQVJZICYmICFoYXMoSXRlcmF0b3JQcm90b3R5cGUsIElURVJBVE9SKSloaWRlKEl0ZXJhdG9yUHJvdG90eXBlLCBJVEVSQVRPUiwgcmV0dXJuVGhpcyk7XG4gICAgfVxuICB9XG4gIC8vIGZpeCBBcnJheSN7dmFsdWVzLCBAQGl0ZXJhdG9yfS5uYW1lIGluIFY4IC8gRkZcbiAgaWYoREVGX1ZBTFVFUyAmJiAkbmF0aXZlICYmICRuYXRpdmUubmFtZSAhPT0gVkFMVUVTKXtcbiAgICBWQUxVRVNfQlVHID0gdHJ1ZTtcbiAgICAkZGVmYXVsdCA9IGZ1bmN0aW9uIHZhbHVlcygpeyByZXR1cm4gJG5hdGl2ZS5jYWxsKHRoaXMpOyB9O1xuICB9XG4gIC8vIERlZmluZSBpdGVyYXRvclxuICBpZigoIUxJQlJBUlkgfHwgRk9SQ0VEKSAmJiAoQlVHR1kgfHwgVkFMVUVTX0JVRyB8fCAhcHJvdG9bSVRFUkFUT1JdKSl7XG4gICAgaGlkZShwcm90bywgSVRFUkFUT1IsICRkZWZhdWx0KTtcbiAgfVxuICAvLyBQbHVnIGZvciBsaWJyYXJ5XG4gIEl0ZXJhdG9yc1tOQU1FXSA9ICRkZWZhdWx0O1xuICBJdGVyYXRvcnNbVEFHXSAgPSByZXR1cm5UaGlzO1xuICBpZihERUZBVUxUKXtcbiAgICBtZXRob2RzID0ge1xuICAgICAgdmFsdWVzOiAgREVGX1ZBTFVFUyA/ICRkZWZhdWx0IDogZ2V0TWV0aG9kKFZBTFVFUyksXG4gICAgICBrZXlzOiAgICBJU19TRVQgICAgID8gJGRlZmF1bHQgOiBnZXRNZXRob2QoS0VZUyksXG4gICAgICBlbnRyaWVzOiAkZW50cmllc1xuICAgIH07XG4gICAgaWYoRk9SQ0VEKWZvcihrZXkgaW4gbWV0aG9kcyl7XG4gICAgICBpZighKGtleSBpbiBwcm90bykpcmVkZWZpbmUocHJvdG8sIGtleSwgbWV0aG9kc1trZXldKTtcbiAgICB9IGVsc2UgJGV4cG9ydCgkZXhwb3J0LlAgKyAkZXhwb3J0LkYgKiAoQlVHR1kgfHwgVkFMVUVTX0JVRyksIE5BTUUsIG1ldGhvZHMpO1xuICB9XG4gIHJldHVybiBtZXRob2RzO1xufTsiLCIndXNlIHN0cmljdCc7XG52YXIgJGF0ICA9IHJlcXVpcmUoJy4vX3N0cmluZy1hdCcpKHRydWUpO1xuXG4vLyAyMS4xLjMuMjcgU3RyaW5nLnByb3RvdHlwZVtAQGl0ZXJhdG9yXSgpXG5yZXF1aXJlKCcuL19pdGVyLWRlZmluZScpKFN0cmluZywgJ1N0cmluZycsIGZ1bmN0aW9uKGl0ZXJhdGVkKXtcbiAgdGhpcy5fdCA9IFN0cmluZyhpdGVyYXRlZCk7IC8vIHRhcmdldFxuICB0aGlzLl9pID0gMDsgICAgICAgICAgICAgICAgLy8gbmV4dCBpbmRleFxuLy8gMjEuMS41LjIuMSAlU3RyaW5nSXRlcmF0b3JQcm90b3R5cGUlLm5leHQoKVxufSwgZnVuY3Rpb24oKXtcbiAgdmFyIE8gICAgID0gdGhpcy5fdFxuICAgICwgaW5kZXggPSB0aGlzLl9pXG4gICAgLCBwb2ludDtcbiAgaWYoaW5kZXggPj0gTy5sZW5ndGgpcmV0dXJuIHt2YWx1ZTogdW5kZWZpbmVkLCBkb25lOiB0cnVlfTtcbiAgcG9pbnQgPSAkYXQoTywgaW5kZXgpO1xuICB0aGlzLl9pICs9IHBvaW50Lmxlbmd0aDtcbiAgcmV0dXJuIHt2YWx1ZTogcG9pbnQsIGRvbmU6IGZhbHNlfTtcbn0pOyIsIi8vIDIyLjEuMy4zMSBBcnJheS5wcm90b3R5cGVbQEB1bnNjb3BhYmxlc11cbnZhciBVTlNDT1BBQkxFUyA9IHJlcXVpcmUoJy4vX3drcycpKCd1bnNjb3BhYmxlcycpXG4gICwgQXJyYXlQcm90byAgPSBBcnJheS5wcm90b3R5cGU7XG5pZihBcnJheVByb3RvW1VOU0NPUEFCTEVTXSA9PSB1bmRlZmluZWQpcmVxdWlyZSgnLi9faGlkZScpKEFycmF5UHJvdG8sIFVOU0NPUEFCTEVTLCB7fSk7XG5tb2R1bGUuZXhwb3J0cyA9IGZ1bmN0aW9uKGtleSl7XG4gIEFycmF5UHJvdG9bVU5TQ09QQUJMRVNdW2tleV0gPSB0cnVlO1xufTsiLCJtb2R1bGUuZXhwb3J0cyA9IGZ1bmN0aW9uKGRvbmUsIHZhbHVlKXtcbiAgcmV0dXJuIHt2YWx1ZTogdmFsdWUsIGRvbmU6ICEhZG9uZX07XG59OyIsIid1c2Ugc3RyaWN0JztcbnZhciBhZGRUb1Vuc2NvcGFibGVzID0gcmVxdWlyZSgnLi9fYWRkLXRvLXVuc2NvcGFibGVzJylcbiAgLCBzdGVwICAgICAgICAgICAgID0gcmVxdWlyZSgnLi9faXRlci1zdGVwJylcbiAgLCBJdGVyYXRvcnMgICAgICAgID0gcmVxdWlyZSgnLi9faXRlcmF0b3JzJylcbiAgLCB0b0lPYmplY3QgICAgICAgID0gcmVxdWlyZSgnLi9fdG8taW9iamVjdCcpO1xuXG4vLyAyMi4xLjMuNCBBcnJheS5wcm90b3R5cGUuZW50cmllcygpXG4vLyAyMi4xLjMuMTMgQXJyYXkucHJvdG90eXBlLmtleXMoKVxuLy8gMjIuMS4zLjI5IEFycmF5LnByb3RvdHlwZS52YWx1ZXMoKVxuLy8gMjIuMS4zLjMwIEFycmF5LnByb3RvdHlwZVtAQGl0ZXJhdG9yXSgpXG5tb2R1bGUuZXhwb3J0cyA9IHJlcXVpcmUoJy4vX2l0ZXItZGVmaW5lJykoQXJyYXksICdBcnJheScsIGZ1bmN0aW9uKGl0ZXJhdGVkLCBraW5kKXtcbiAgdGhpcy5fdCA9IHRvSU9iamVjdChpdGVyYXRlZCk7IC8vIHRhcmdldFxuICB0aGlzLl9pID0gMDsgICAgICAgICAgICAgICAgICAgLy8gbmV4dCBpbmRleFxuICB0aGlzLl9rID0ga2luZDsgICAgICAgICAgICAgICAgLy8ga2luZFxuLy8gMjIuMS41LjIuMSAlQXJyYXlJdGVyYXRvclByb3RvdHlwZSUubmV4dCgpXG59LCBmdW5jdGlvbigpe1xuICB2YXIgTyAgICAgPSB0aGlzLl90XG4gICAgLCBraW5kICA9IHRoaXMuX2tcbiAgICAsIGluZGV4ID0gdGhpcy5faSsrO1xuICBpZighTyB8fCBpbmRleCA+PSBPLmxlbmd0aCl7XG4gICAgdGhpcy5fdCA9IHVuZGVmaW5lZDtcbiAgICByZXR1cm4gc3RlcCgxKTtcbiAgfVxuICBpZihraW5kID09ICdrZXlzJyAgKXJldHVybiBzdGVwKDAsIGluZGV4KTtcbiAgaWYoa2luZCA9PSAndmFsdWVzJylyZXR1cm4gc3RlcCgwLCBPW2luZGV4XSk7XG4gIHJldHVybiBzdGVwKDAsIFtpbmRleCwgT1tpbmRleF1dKTtcbn0sICd2YWx1ZXMnKTtcblxuLy8gYXJndW1lbnRzTGlzdFtAQGl0ZXJhdG9yXSBpcyAlQXJyYXlQcm90b192YWx1ZXMlICg5LjQuNC42LCA5LjQuNC43KVxuSXRlcmF0b3JzLkFyZ3VtZW50cyA9IEl0ZXJhdG9ycy5BcnJheTtcblxuYWRkVG9VbnNjb3BhYmxlcygna2V5cycpO1xuYWRkVG9VbnNjb3BhYmxlcygndmFsdWVzJyk7XG5hZGRUb1Vuc2NvcGFibGVzKCdlbnRyaWVzJyk7IiwidmFyICRpdGVyYXRvcnMgICAgPSByZXF1aXJlKCcuL2VzNi5hcnJheS5pdGVyYXRvcicpXG4gICwgcmVkZWZpbmUgICAgICA9IHJlcXVpcmUoJy4vX3JlZGVmaW5lJylcbiAgLCBnbG9iYWwgICAgICAgID0gcmVxdWlyZSgnLi9fZ2xvYmFsJylcbiAgLCBoaWRlICAgICAgICAgID0gcmVxdWlyZSgnLi9faGlkZScpXG4gICwgSXRlcmF0b3JzICAgICA9IHJlcXVpcmUoJy4vX2l0ZXJhdG9ycycpXG4gICwgd2tzICAgICAgICAgICA9IHJlcXVpcmUoJy4vX3drcycpXG4gICwgSVRFUkFUT1IgICAgICA9IHdrcygnaXRlcmF0b3InKVxuICAsIFRPX1NUUklOR19UQUcgPSB3a3MoJ3RvU3RyaW5nVGFnJylcbiAgLCBBcnJheVZhbHVlcyAgID0gSXRlcmF0b3JzLkFycmF5O1xuXG5mb3IodmFyIGNvbGxlY3Rpb25zID0gWydOb2RlTGlzdCcsICdET01Ub2tlbkxpc3QnLCAnTWVkaWFMaXN0JywgJ1N0eWxlU2hlZXRMaXN0JywgJ0NTU1J1bGVMaXN0J10sIGkgPSAwOyBpIDwgNTsgaSsrKXtcbiAgdmFyIE5BTUUgICAgICAgPSBjb2xsZWN0aW9uc1tpXVxuICAgICwgQ29sbGVjdGlvbiA9IGdsb2JhbFtOQU1FXVxuICAgICwgcHJvdG8gICAgICA9IENvbGxlY3Rpb24gJiYgQ29sbGVjdGlvbi5wcm90b3R5cGVcbiAgICAsIGtleTtcbiAgaWYocHJvdG8pe1xuICAgIGlmKCFwcm90b1tJVEVSQVRPUl0paGlkZShwcm90bywgSVRFUkFUT1IsIEFycmF5VmFsdWVzKTtcbiAgICBpZighcHJvdG9bVE9fU1RSSU5HX1RBR10paGlkZShwcm90bywgVE9fU1RSSU5HX1RBRywgTkFNRSk7XG4gICAgSXRlcmF0b3JzW05BTUVdID0gQXJyYXlWYWx1ZXM7XG4gICAgZm9yKGtleSBpbiAkaXRlcmF0b3JzKWlmKCFwcm90b1trZXldKXJlZGVmaW5lKHByb3RvLCBrZXksICRpdGVyYXRvcnNba2V5XSwgdHJ1ZSk7XG4gIH1cbn0iLCJtb2R1bGUuZXhwb3J0cyA9IGZ1bmN0aW9uKGl0LCBDb25zdHJ1Y3RvciwgbmFtZSwgZm9yYmlkZGVuRmllbGQpe1xuICBpZighKGl0IGluc3RhbmNlb2YgQ29uc3RydWN0b3IpIHx8IChmb3JiaWRkZW5GaWVsZCAhPT0gdW5kZWZpbmVkICYmIGZvcmJpZGRlbkZpZWxkIGluIGl0KSl7XG4gICAgdGhyb3cgVHlwZUVycm9yKG5hbWUgKyAnOiBpbmNvcnJlY3QgaW52b2NhdGlvbiEnKTtcbiAgfSByZXR1cm4gaXQ7XG59OyIsIi8vIGNhbGwgc29tZXRoaW5nIG9uIGl0ZXJhdG9yIHN0ZXAgd2l0aCBzYWZlIGNsb3Npbmcgb24gZXJyb3JcbnZhciBhbk9iamVjdCA9IHJlcXVpcmUoJy4vX2FuLW9iamVjdCcpO1xubW9kdWxlLmV4cG9ydHMgPSBmdW5jdGlvbihpdGVyYXRvciwgZm4sIHZhbHVlLCBlbnRyaWVzKXtcbiAgdHJ5IHtcbiAgICByZXR1cm4gZW50cmllcyA/IGZuKGFuT2JqZWN0KHZhbHVlKVswXSwgdmFsdWVbMV0pIDogZm4odmFsdWUpO1xuICAvLyA3LjQuNiBJdGVyYXRvckNsb3NlKGl0ZXJhdG9yLCBjb21wbGV0aW9uKVxuICB9IGNhdGNoKGUpe1xuICAgIHZhciByZXQgPSBpdGVyYXRvclsncmV0dXJuJ107XG4gICAgaWYocmV0ICE9PSB1bmRlZmluZWQpYW5PYmplY3QocmV0LmNhbGwoaXRlcmF0b3IpKTtcbiAgICB0aHJvdyBlO1xuICB9XG59OyIsIi8vIGNoZWNrIG9uIGRlZmF1bHQgQXJyYXkgaXRlcmF0b3JcbnZhciBJdGVyYXRvcnMgID0gcmVxdWlyZSgnLi9faXRlcmF0b3JzJylcbiAgLCBJVEVSQVRPUiAgID0gcmVxdWlyZSgnLi9fd2tzJykoJ2l0ZXJhdG9yJylcbiAgLCBBcnJheVByb3RvID0gQXJyYXkucHJvdG90eXBlO1xuXG5tb2R1bGUuZXhwb3J0cyA9IGZ1bmN0aW9uKGl0KXtcbiAgcmV0dXJuIGl0ICE9PSB1bmRlZmluZWQgJiYgKEl0ZXJhdG9ycy5BcnJheSA9PT0gaXQgfHwgQXJyYXlQcm90b1tJVEVSQVRPUl0gPT09IGl0KTtcbn07IiwidmFyIGNsYXNzb2YgICA9IHJlcXVpcmUoJy4vX2NsYXNzb2YnKVxuICAsIElURVJBVE9SICA9IHJlcXVpcmUoJy4vX3drcycpKCdpdGVyYXRvcicpXG4gICwgSXRlcmF0b3JzID0gcmVxdWlyZSgnLi9faXRlcmF0b3JzJyk7XG5tb2R1bGUuZXhwb3J0cyA9IHJlcXVpcmUoJy4vX2NvcmUnKS5nZXRJdGVyYXRvck1ldGhvZCA9IGZ1bmN0aW9uKGl0KXtcbiAgaWYoaXQgIT0gdW5kZWZpbmVkKXJldHVybiBpdFtJVEVSQVRPUl1cbiAgICB8fCBpdFsnQEBpdGVyYXRvciddXG4gICAgfHwgSXRlcmF0b3JzW2NsYXNzb2YoaXQpXTtcbn07IiwidmFyIGN0eCAgICAgICAgID0gcmVxdWlyZSgnLi9fY3R4JylcbiAgLCBjYWxsICAgICAgICA9IHJlcXVpcmUoJy4vX2l0ZXItY2FsbCcpXG4gICwgaXNBcnJheUl0ZXIgPSByZXF1aXJlKCcuL19pcy1hcnJheS1pdGVyJylcbiAgLCBhbk9iamVjdCAgICA9IHJlcXVpcmUoJy4vX2FuLW9iamVjdCcpXG4gICwgdG9MZW5ndGggICAgPSByZXF1aXJlKCcuL190by1sZW5ndGgnKVxuICAsIGdldEl0ZXJGbiAgID0gcmVxdWlyZSgnLi9jb3JlLmdldC1pdGVyYXRvci1tZXRob2QnKVxuICAsIEJSRUFLICAgICAgID0ge31cbiAgLCBSRVRVUk4gICAgICA9IHt9O1xudmFyIGV4cG9ydHMgPSBtb2R1bGUuZXhwb3J0cyA9IGZ1bmN0aW9uKGl0ZXJhYmxlLCBlbnRyaWVzLCBmbiwgdGhhdCwgSVRFUkFUT1Ipe1xuICB2YXIgaXRlckZuID0gSVRFUkFUT1IgPyBmdW5jdGlvbigpeyByZXR1cm4gaXRlcmFibGU7IH0gOiBnZXRJdGVyRm4oaXRlcmFibGUpXG4gICAgLCBmICAgICAgPSBjdHgoZm4sIHRoYXQsIGVudHJpZXMgPyAyIDogMSlcbiAgICAsIGluZGV4ICA9IDBcbiAgICAsIGxlbmd0aCwgc3RlcCwgaXRlcmF0b3IsIHJlc3VsdDtcbiAgaWYodHlwZW9mIGl0ZXJGbiAhPSAnZnVuY3Rpb24nKXRocm93IFR5cGVFcnJvcihpdGVyYWJsZSArICcgaXMgbm90IGl0ZXJhYmxlIScpO1xuICAvLyBmYXN0IGNhc2UgZm9yIGFycmF5cyB3aXRoIGRlZmF1bHQgaXRlcmF0b3JcbiAgaWYoaXNBcnJheUl0ZXIoaXRlckZuKSlmb3IobGVuZ3RoID0gdG9MZW5ndGgoaXRlcmFibGUubGVuZ3RoKTsgbGVuZ3RoID4gaW5kZXg7IGluZGV4Kyspe1xuICAgIHJlc3VsdCA9IGVudHJpZXMgPyBmKGFuT2JqZWN0KHN0ZXAgPSBpdGVyYWJsZVtpbmRleF0pWzBdLCBzdGVwWzFdKSA6IGYoaXRlcmFibGVbaW5kZXhdKTtcbiAgICBpZihyZXN1bHQgPT09IEJSRUFLIHx8IHJlc3VsdCA9PT0gUkVUVVJOKXJldHVybiByZXN1bHQ7XG4gIH0gZWxzZSBmb3IoaXRlcmF0b3IgPSBpdGVyRm4uY2FsbChpdGVyYWJsZSk7ICEoc3RlcCA9IGl0ZXJhdG9yLm5leHQoKSkuZG9uZTsgKXtcbiAgICByZXN1bHQgPSBjYWxsKGl0ZXJhdG9yLCBmLCBzdGVwLnZhbHVlLCBlbnRyaWVzKTtcbiAgICBpZihyZXN1bHQgPT09IEJSRUFLIHx8IHJlc3VsdCA9PT0gUkVUVVJOKXJldHVybiByZXN1bHQ7XG4gIH1cbn07XG5leHBvcnRzLkJSRUFLICA9IEJSRUFLO1xuZXhwb3J0cy5SRVRVUk4gPSBSRVRVUk47IiwiLy8gNy4zLjIwIFNwZWNpZXNDb25zdHJ1Y3RvcihPLCBkZWZhdWx0Q29uc3RydWN0b3IpXG52YXIgYW5PYmplY3QgID0gcmVxdWlyZSgnLi9fYW4tb2JqZWN0JylcbiAgLCBhRnVuY3Rpb24gPSByZXF1aXJlKCcuL19hLWZ1bmN0aW9uJylcbiAgLCBTUEVDSUVTICAgPSByZXF1aXJlKCcuL193a3MnKSgnc3BlY2llcycpO1xubW9kdWxlLmV4cG9ydHMgPSBmdW5jdGlvbihPLCBEKXtcbiAgdmFyIEMgPSBhbk9iamVjdChPKS5jb25zdHJ1Y3RvciwgUztcbiAgcmV0dXJuIEMgPT09IHVuZGVmaW5lZCB8fCAoUyA9IGFuT2JqZWN0KEMpW1NQRUNJRVNdKSA9PSB1bmRlZmluZWQgPyBEIDogYUZ1bmN0aW9uKFMpO1xufTsiLCIvLyBmYXN0IGFwcGx5LCBodHRwOi8vanNwZXJmLmxua2l0LmNvbS9mYXN0LWFwcGx5LzVcbm1vZHVsZS5leHBvcnRzID0gZnVuY3Rpb24oZm4sIGFyZ3MsIHRoYXQpe1xuICB2YXIgdW4gPSB0aGF0ID09PSB1bmRlZmluZWQ7XG4gIHN3aXRjaChhcmdzLmxlbmd0aCl7XG4gICAgY2FzZSAwOiByZXR1cm4gdW4gPyBmbigpXG4gICAgICAgICAgICAgICAgICAgICAgOiBmbi5jYWxsKHRoYXQpO1xuICAgIGNhc2UgMTogcmV0dXJuIHVuID8gZm4oYXJnc1swXSlcbiAgICAgICAgICAgICAgICAgICAgICA6IGZuLmNhbGwodGhhdCwgYXJnc1swXSk7XG4gICAgY2FzZSAyOiByZXR1cm4gdW4gPyBmbihhcmdzWzBdLCBhcmdzWzFdKVxuICAgICAgICAgICAgICAgICAgICAgIDogZm4uY2FsbCh0aGF0LCBhcmdzWzBdLCBhcmdzWzFdKTtcbiAgICBjYXNlIDM6IHJldHVybiB1biA/IGZuKGFyZ3NbMF0sIGFyZ3NbMV0sIGFyZ3NbMl0pXG4gICAgICAgICAgICAgICAgICAgICAgOiBmbi5jYWxsKHRoYXQsIGFyZ3NbMF0sIGFyZ3NbMV0sIGFyZ3NbMl0pO1xuICAgIGNhc2UgNDogcmV0dXJuIHVuID8gZm4oYXJnc1swXSwgYXJnc1sxXSwgYXJnc1syXSwgYXJnc1szXSlcbiAgICAgICAgICAgICAgICAgICAgICA6IGZuLmNhbGwodGhhdCwgYXJnc1swXSwgYXJnc1sxXSwgYXJnc1syXSwgYXJnc1szXSk7XG4gIH0gcmV0dXJuICAgICAgICAgICAgICBmbi5hcHBseSh0aGF0LCBhcmdzKTtcbn07IiwidmFyIGN0eCAgICAgICAgICAgICAgICA9IHJlcXVpcmUoJy4vX2N0eCcpXG4gICwgaW52b2tlICAgICAgICAgICAgID0gcmVxdWlyZSgnLi9faW52b2tlJylcbiAgLCBodG1sICAgICAgICAgICAgICAgPSByZXF1aXJlKCcuL19odG1sJylcbiAgLCBjZWwgICAgICAgICAgICAgICAgPSByZXF1aXJlKCcuL19kb20tY3JlYXRlJylcbiAgLCBnbG9iYWwgICAgICAgICAgICAgPSByZXF1aXJlKCcuL19nbG9iYWwnKVxuICAsIHByb2Nlc3MgICAgICAgICAgICA9IGdsb2JhbC5wcm9jZXNzXG4gICwgc2V0VGFzayAgICAgICAgICAgID0gZ2xvYmFsLnNldEltbWVkaWF0ZVxuICAsIGNsZWFyVGFzayAgICAgICAgICA9IGdsb2JhbC5jbGVhckltbWVkaWF0ZVxuICAsIE1lc3NhZ2VDaGFubmVsICAgICA9IGdsb2JhbC5NZXNzYWdlQ2hhbm5lbFxuICAsIGNvdW50ZXIgICAgICAgICAgICA9IDBcbiAgLCBxdWV1ZSAgICAgICAgICAgICAgPSB7fVxuICAsIE9OUkVBRFlTVEFURUNIQU5HRSA9ICdvbnJlYWR5c3RhdGVjaGFuZ2UnXG4gICwgZGVmZXIsIGNoYW5uZWwsIHBvcnQ7XG52YXIgcnVuID0gZnVuY3Rpb24oKXtcbiAgdmFyIGlkID0gK3RoaXM7XG4gIGlmKHF1ZXVlLmhhc093blByb3BlcnR5KGlkKSl7XG4gICAgdmFyIGZuID0gcXVldWVbaWRdO1xuICAgIGRlbGV0ZSBxdWV1ZVtpZF07XG4gICAgZm4oKTtcbiAgfVxufTtcbnZhciBsaXN0ZW5lciA9IGZ1bmN0aW9uKGV2ZW50KXtcbiAgcnVuLmNhbGwoZXZlbnQuZGF0YSk7XG59O1xuLy8gTm9kZS5qcyAwLjkrICYgSUUxMCsgaGFzIHNldEltbWVkaWF0ZSwgb3RoZXJ3aXNlOlxuaWYoIXNldFRhc2sgfHwgIWNsZWFyVGFzayl7XG4gIHNldFRhc2sgPSBmdW5jdGlvbiBzZXRJbW1lZGlhdGUoZm4pe1xuICAgIHZhciBhcmdzID0gW10sIGkgPSAxO1xuICAgIHdoaWxlKGFyZ3VtZW50cy5sZW5ndGggPiBpKWFyZ3MucHVzaChhcmd1bWVudHNbaSsrXSk7XG4gICAgcXVldWVbKytjb3VudGVyXSA9IGZ1bmN0aW9uKCl7XG4gICAgICBpbnZva2UodHlwZW9mIGZuID09ICdmdW5jdGlvbicgPyBmbiA6IEZ1bmN0aW9uKGZuKSwgYXJncyk7XG4gICAgfTtcbiAgICBkZWZlcihjb3VudGVyKTtcbiAgICByZXR1cm4gY291bnRlcjtcbiAgfTtcbiAgY2xlYXJUYXNrID0gZnVuY3Rpb24gY2xlYXJJbW1lZGlhdGUoaWQpe1xuICAgIGRlbGV0ZSBxdWV1ZVtpZF07XG4gIH07XG4gIC8vIE5vZGUuanMgMC44LVxuICBpZihyZXF1aXJlKCcuL19jb2YnKShwcm9jZXNzKSA9PSAncHJvY2Vzcycpe1xuICAgIGRlZmVyID0gZnVuY3Rpb24oaWQpe1xuICAgICAgcHJvY2Vzcy5uZXh0VGljayhjdHgocnVuLCBpZCwgMSkpO1xuICAgIH07XG4gIC8vIEJyb3dzZXJzIHdpdGggTWVzc2FnZUNoYW5uZWwsIGluY2x1ZGVzIFdlYldvcmtlcnNcbiAgfSBlbHNlIGlmKE1lc3NhZ2VDaGFubmVsKXtcbiAgICBjaGFubmVsID0gbmV3IE1lc3NhZ2VDaGFubmVsO1xuICAgIHBvcnQgICAgPSBjaGFubmVsLnBvcnQyO1xuICAgIGNoYW5uZWwucG9ydDEub25tZXNzYWdlID0gbGlzdGVuZXI7XG4gICAgZGVmZXIgPSBjdHgocG9ydC5wb3N0TWVzc2FnZSwgcG9ydCwgMSk7XG4gIC8vIEJyb3dzZXJzIHdpdGggcG9zdE1lc3NhZ2UsIHNraXAgV2ViV29ya2Vyc1xuICAvLyBJRTggaGFzIHBvc3RNZXNzYWdlLCBidXQgaXQncyBzeW5jICYgdHlwZW9mIGl0cyBwb3N0TWVzc2FnZSBpcyAnb2JqZWN0J1xuICB9IGVsc2UgaWYoZ2xvYmFsLmFkZEV2ZW50TGlzdGVuZXIgJiYgdHlwZW9mIHBvc3RNZXNzYWdlID09ICdmdW5jdGlvbicgJiYgIWdsb2JhbC5pbXBvcnRTY3JpcHRzKXtcbiAgICBkZWZlciA9IGZ1bmN0aW9uKGlkKXtcbiAgICAgIGdsb2JhbC5wb3N0TWVzc2FnZShpZCArICcnLCAnKicpO1xuICAgIH07XG4gICAgZ2xvYmFsLmFkZEV2ZW50TGlzdGVuZXIoJ21lc3NhZ2UnLCBsaXN0ZW5lciwgZmFsc2UpO1xuICAvLyBJRTgtXG4gIH0gZWxzZSBpZihPTlJFQURZU1RBVEVDSEFOR0UgaW4gY2VsKCdzY3JpcHQnKSl7XG4gICAgZGVmZXIgPSBmdW5jdGlvbihpZCl7XG4gICAgICBodG1sLmFwcGVuZENoaWxkKGNlbCgnc2NyaXB0JykpW09OUkVBRFlTVEFURUNIQU5HRV0gPSBmdW5jdGlvbigpe1xuICAgICAgICBodG1sLnJlbW92ZUNoaWxkKHRoaXMpO1xuICAgICAgICBydW4uY2FsbChpZCk7XG4gICAgICB9O1xuICAgIH07XG4gIC8vIFJlc3Qgb2xkIGJyb3dzZXJzXG4gIH0gZWxzZSB7XG4gICAgZGVmZXIgPSBmdW5jdGlvbihpZCl7XG4gICAgICBzZXRUaW1lb3V0KGN0eChydW4sIGlkLCAxKSwgMCk7XG4gICAgfTtcbiAgfVxufVxubW9kdWxlLmV4cG9ydHMgPSB7XG4gIHNldDogICBzZXRUYXNrLFxuICBjbGVhcjogY2xlYXJUYXNrXG59OyIsInZhciBnbG9iYWwgICAgPSByZXF1aXJlKCcuL19nbG9iYWwnKVxuICAsIG1hY3JvdGFzayA9IHJlcXVpcmUoJy4vX3Rhc2snKS5zZXRcbiAgLCBPYnNlcnZlciAgPSBnbG9iYWwuTXV0YXRpb25PYnNlcnZlciB8fCBnbG9iYWwuV2ViS2l0TXV0YXRpb25PYnNlcnZlclxuICAsIHByb2Nlc3MgICA9IGdsb2JhbC5wcm9jZXNzXG4gICwgUHJvbWlzZSAgID0gZ2xvYmFsLlByb21pc2VcbiAgLCBpc05vZGUgICAgPSByZXF1aXJlKCcuL19jb2YnKShwcm9jZXNzKSA9PSAncHJvY2Vzcyc7XG5cbm1vZHVsZS5leHBvcnRzID0gZnVuY3Rpb24oKXtcbiAgdmFyIGhlYWQsIGxhc3QsIG5vdGlmeTtcblxuICB2YXIgZmx1c2ggPSBmdW5jdGlvbigpe1xuICAgIHZhciBwYXJlbnQsIGZuO1xuICAgIGlmKGlzTm9kZSAmJiAocGFyZW50ID0gcHJvY2Vzcy5kb21haW4pKXBhcmVudC5leGl0KCk7XG4gICAgd2hpbGUoaGVhZCl7XG4gICAgICBmbiAgID0gaGVhZC5mbjtcbiAgICAgIGhlYWQgPSBoZWFkLm5leHQ7XG4gICAgICB0cnkge1xuICAgICAgICBmbigpO1xuICAgICAgfSBjYXRjaChlKXtcbiAgICAgICAgaWYoaGVhZClub3RpZnkoKTtcbiAgICAgICAgZWxzZSBsYXN0ID0gdW5kZWZpbmVkO1xuICAgICAgICB0aHJvdyBlO1xuICAgICAgfVxuICAgIH0gbGFzdCA9IHVuZGVmaW5lZDtcbiAgICBpZihwYXJlbnQpcGFyZW50LmVudGVyKCk7XG4gIH07XG5cbiAgLy8gTm9kZS5qc1xuICBpZihpc05vZGUpe1xuICAgIG5vdGlmeSA9IGZ1bmN0aW9uKCl7XG4gICAgICBwcm9jZXNzLm5leHRUaWNrKGZsdXNoKTtcbiAgICB9O1xuICAvLyBicm93c2VycyB3aXRoIE11dGF0aW9uT2JzZXJ2ZXJcbiAgfSBlbHNlIGlmKE9ic2VydmVyKXtcbiAgICB2YXIgdG9nZ2xlID0gdHJ1ZVxuICAgICAgLCBub2RlICAgPSBkb2N1bWVudC5jcmVhdGVUZXh0Tm9kZSgnJyk7XG4gICAgbmV3IE9ic2VydmVyKGZsdXNoKS5vYnNlcnZlKG5vZGUsIHtjaGFyYWN0ZXJEYXRhOiB0cnVlfSk7IC8vIGVzbGludC1kaXNhYmxlLWxpbmUgbm8tbmV3XG4gICAgbm90aWZ5ID0gZnVuY3Rpb24oKXtcbiAgICAgIG5vZGUuZGF0YSA9IHRvZ2dsZSA9ICF0b2dnbGU7XG4gICAgfTtcbiAgLy8gZW52aXJvbm1lbnRzIHdpdGggbWF5YmUgbm9uLWNvbXBsZXRlbHkgY29ycmVjdCwgYnV0IGV4aXN0ZW50IFByb21pc2VcbiAgfSBlbHNlIGlmKFByb21pc2UgJiYgUHJvbWlzZS5yZXNvbHZlKXtcbiAgICB2YXIgcHJvbWlzZSA9IFByb21pc2UucmVzb2x2ZSgpO1xuICAgIG5vdGlmeSA9IGZ1bmN0aW9uKCl7XG4gICAgICBwcm9taXNlLnRoZW4oZmx1c2gpO1xuICAgIH07XG4gIC8vIGZvciBvdGhlciBlbnZpcm9ubWVudHMgLSBtYWNyb3Rhc2sgYmFzZWQgb246XG4gIC8vIC0gc2V0SW1tZWRpYXRlXG4gIC8vIC0gTWVzc2FnZUNoYW5uZWxcbiAgLy8gLSB3aW5kb3cucG9zdE1lc3NhZ1xuICAvLyAtIG9ucmVhZHlzdGF0ZWNoYW5nZVxuICAvLyAtIHNldFRpbWVvdXRcbiAgfSBlbHNlIHtcbiAgICBub3RpZnkgPSBmdW5jdGlvbigpe1xuICAgICAgLy8gc3RyYW5nZSBJRSArIHdlYnBhY2sgZGV2IHNlcnZlciBidWcgLSB1c2UgLmNhbGwoZ2xvYmFsKVxuICAgICAgbWFjcm90YXNrLmNhbGwoZ2xvYmFsLCBmbHVzaCk7XG4gICAgfTtcbiAgfVxuXG4gIHJldHVybiBmdW5jdGlvbihmbil7XG4gICAgdmFyIHRhc2sgPSB7Zm46IGZuLCBuZXh0OiB1bmRlZmluZWR9O1xuICAgIGlmKGxhc3QpbGFzdC5uZXh0ID0gdGFzaztcbiAgICBpZighaGVhZCl7XG4gICAgICBoZWFkID0gdGFzaztcbiAgICAgIG5vdGlmeSgpO1xuICAgIH0gbGFzdCA9IHRhc2s7XG4gIH07XG59OyIsInZhciByZWRlZmluZSA9IHJlcXVpcmUoJy4vX3JlZGVmaW5lJyk7XG5tb2R1bGUuZXhwb3J0cyA9IGZ1bmN0aW9uKHRhcmdldCwgc3JjLCBzYWZlKXtcbiAgZm9yKHZhciBrZXkgaW4gc3JjKXJlZGVmaW5lKHRhcmdldCwga2V5LCBzcmNba2V5XSwgc2FmZSk7XG4gIHJldHVybiB0YXJnZXQ7XG59OyIsIid1c2Ugc3RyaWN0JztcbnZhciBnbG9iYWwgICAgICA9IHJlcXVpcmUoJy4vX2dsb2JhbCcpXG4gICwgZFAgICAgICAgICAgPSByZXF1aXJlKCcuL19vYmplY3QtZHAnKVxuICAsIERFU0NSSVBUT1JTID0gcmVxdWlyZSgnLi9fZGVzY3JpcHRvcnMnKVxuICAsIFNQRUNJRVMgICAgID0gcmVxdWlyZSgnLi9fd2tzJykoJ3NwZWNpZXMnKTtcblxubW9kdWxlLmV4cG9ydHMgPSBmdW5jdGlvbihLRVkpe1xuICB2YXIgQyA9IGdsb2JhbFtLRVldO1xuICBpZihERVNDUklQVE9SUyAmJiBDICYmICFDW1NQRUNJRVNdKWRQLmYoQywgU1BFQ0lFUywge1xuICAgIGNvbmZpZ3VyYWJsZTogdHJ1ZSxcbiAgICBnZXQ6IGZ1bmN0aW9uKCl7IHJldHVybiB0aGlzOyB9XG4gIH0pO1xufTsiLCJ2YXIgSVRFUkFUT1IgICAgID0gcmVxdWlyZSgnLi9fd2tzJykoJ2l0ZXJhdG9yJylcbiAgLCBTQUZFX0NMT1NJTkcgPSBmYWxzZTtcblxudHJ5IHtcbiAgdmFyIHJpdGVyID0gWzddW0lURVJBVE9SXSgpO1xuICByaXRlclsncmV0dXJuJ10gPSBmdW5jdGlvbigpeyBTQUZFX0NMT1NJTkcgPSB0cnVlOyB9O1xuICBBcnJheS5mcm9tKHJpdGVyLCBmdW5jdGlvbigpeyB0aHJvdyAyOyB9KTtcbn0gY2F0Y2goZSl7IC8qIGVtcHR5ICovIH1cblxubW9kdWxlLmV4cG9ydHMgPSBmdW5jdGlvbihleGVjLCBza2lwQ2xvc2luZyl7XG4gIGlmKCFza2lwQ2xvc2luZyAmJiAhU0FGRV9DTE9TSU5HKXJldHVybiBmYWxzZTtcbiAgdmFyIHNhZmUgPSBmYWxzZTtcbiAgdHJ5IHtcbiAgICB2YXIgYXJyICA9IFs3XVxuICAgICAgLCBpdGVyID0gYXJyW0lURVJBVE9SXSgpO1xuICAgIGl0ZXIubmV4dCA9IGZ1bmN0aW9uKCl7IHJldHVybiB7ZG9uZTogc2FmZSA9IHRydWV9OyB9O1xuICAgIGFycltJVEVSQVRPUl0gPSBmdW5jdGlvbigpeyByZXR1cm4gaXRlcjsgfTtcbiAgICBleGVjKGFycik7XG4gIH0gY2F0Y2goZSl7IC8qIGVtcHR5ICovIH1cbiAgcmV0dXJuIHNhZmU7XG59OyIsIid1c2Ugc3RyaWN0JztcbnZhciBMSUJSQVJZICAgICAgICAgICAgPSByZXF1aXJlKCcuL19saWJyYXJ5JylcbiAgLCBnbG9iYWwgICAgICAgICAgICAgPSByZXF1aXJlKCcuL19nbG9iYWwnKVxuICAsIGN0eCAgICAgICAgICAgICAgICA9IHJlcXVpcmUoJy4vX2N0eCcpXG4gICwgY2xhc3NvZiAgICAgICAgICAgID0gcmVxdWlyZSgnLi9fY2xhc3NvZicpXG4gICwgJGV4cG9ydCAgICAgICAgICAgID0gcmVxdWlyZSgnLi9fZXhwb3J0JylcbiAgLCBpc09iamVjdCAgICAgICAgICAgPSByZXF1aXJlKCcuL19pcy1vYmplY3QnKVxuICAsIGFGdW5jdGlvbiAgICAgICAgICA9IHJlcXVpcmUoJy4vX2EtZnVuY3Rpb24nKVxuICAsIGFuSW5zdGFuY2UgICAgICAgICA9IHJlcXVpcmUoJy4vX2FuLWluc3RhbmNlJylcbiAgLCBmb3JPZiAgICAgICAgICAgICAgPSByZXF1aXJlKCcuL19mb3Itb2YnKVxuICAsIHNwZWNpZXNDb25zdHJ1Y3RvciA9IHJlcXVpcmUoJy4vX3NwZWNpZXMtY29uc3RydWN0b3InKVxuICAsIHRhc2sgICAgICAgICAgICAgICA9IHJlcXVpcmUoJy4vX3Rhc2snKS5zZXRcbiAgLCBtaWNyb3Rhc2sgICAgICAgICAgPSByZXF1aXJlKCcuL19taWNyb3Rhc2snKSgpXG4gICwgUFJPTUlTRSAgICAgICAgICAgID0gJ1Byb21pc2UnXG4gICwgVHlwZUVycm9yICAgICAgICAgID0gZ2xvYmFsLlR5cGVFcnJvclxuICAsIHByb2Nlc3MgICAgICAgICAgICA9IGdsb2JhbC5wcm9jZXNzXG4gICwgJFByb21pc2UgICAgICAgICAgID0gZ2xvYmFsW1BST01JU0VdXG4gICwgcHJvY2VzcyAgICAgICAgICAgID0gZ2xvYmFsLnByb2Nlc3NcbiAgLCBpc05vZGUgICAgICAgICAgICAgPSBjbGFzc29mKHByb2Nlc3MpID09ICdwcm9jZXNzJ1xuICAsIGVtcHR5ICAgICAgICAgICAgICA9IGZ1bmN0aW9uKCl7IC8qIGVtcHR5ICovIH1cbiAgLCBJbnRlcm5hbCwgR2VuZXJpY1Byb21pc2VDYXBhYmlsaXR5LCBXcmFwcGVyO1xuXG52YXIgVVNFX05BVElWRSA9ICEhZnVuY3Rpb24oKXtcbiAgdHJ5IHtcbiAgICAvLyBjb3JyZWN0IHN1YmNsYXNzaW5nIHdpdGggQEBzcGVjaWVzIHN1cHBvcnRcbiAgICB2YXIgcHJvbWlzZSAgICAgPSAkUHJvbWlzZS5yZXNvbHZlKDEpXG4gICAgICAsIEZha2VQcm9taXNlID0gKHByb21pc2UuY29uc3RydWN0b3IgPSB7fSlbcmVxdWlyZSgnLi9fd2tzJykoJ3NwZWNpZXMnKV0gPSBmdW5jdGlvbihleGVjKXsgZXhlYyhlbXB0eSwgZW1wdHkpOyB9O1xuICAgIC8vIHVuaGFuZGxlZCByZWplY3Rpb25zIHRyYWNraW5nIHN1cHBvcnQsIE5vZGVKUyBQcm9taXNlIHdpdGhvdXQgaXQgZmFpbHMgQEBzcGVjaWVzIHRlc3RcbiAgICByZXR1cm4gKGlzTm9kZSB8fCB0eXBlb2YgUHJvbWlzZVJlamVjdGlvbkV2ZW50ID09ICdmdW5jdGlvbicpICYmIHByb21pc2UudGhlbihlbXB0eSkgaW5zdGFuY2VvZiBGYWtlUHJvbWlzZTtcbiAgfSBjYXRjaChlKXsgLyogZW1wdHkgKi8gfVxufSgpO1xuXG4vLyBoZWxwZXJzXG52YXIgc2FtZUNvbnN0cnVjdG9yID0gZnVuY3Rpb24oYSwgYil7XG4gIC8vIHdpdGggbGlicmFyeSB3cmFwcGVyIHNwZWNpYWwgY2FzZVxuICByZXR1cm4gYSA9PT0gYiB8fCBhID09PSAkUHJvbWlzZSAmJiBiID09PSBXcmFwcGVyO1xufTtcbnZhciBpc1RoZW5hYmxlID0gZnVuY3Rpb24oaXQpe1xuICB2YXIgdGhlbjtcbiAgcmV0dXJuIGlzT2JqZWN0KGl0KSAmJiB0eXBlb2YgKHRoZW4gPSBpdC50aGVuKSA9PSAnZnVuY3Rpb24nID8gdGhlbiA6IGZhbHNlO1xufTtcbnZhciBuZXdQcm9taXNlQ2FwYWJpbGl0eSA9IGZ1bmN0aW9uKEMpe1xuICByZXR1cm4gc2FtZUNvbnN0cnVjdG9yKCRQcm9taXNlLCBDKVxuICAgID8gbmV3IFByb21pc2VDYXBhYmlsaXR5KEMpXG4gICAgOiBuZXcgR2VuZXJpY1Byb21pc2VDYXBhYmlsaXR5KEMpO1xufTtcbnZhciBQcm9taXNlQ2FwYWJpbGl0eSA9IEdlbmVyaWNQcm9taXNlQ2FwYWJpbGl0eSA9IGZ1bmN0aW9uKEMpe1xuICB2YXIgcmVzb2x2ZSwgcmVqZWN0O1xuICB0aGlzLnByb21pc2UgPSBuZXcgQyhmdW5jdGlvbigkJHJlc29sdmUsICQkcmVqZWN0KXtcbiAgICBpZihyZXNvbHZlICE9PSB1bmRlZmluZWQgfHwgcmVqZWN0ICE9PSB1bmRlZmluZWQpdGhyb3cgVHlwZUVycm9yKCdCYWQgUHJvbWlzZSBjb25zdHJ1Y3RvcicpO1xuICAgIHJlc29sdmUgPSAkJHJlc29sdmU7XG4gICAgcmVqZWN0ICA9ICQkcmVqZWN0O1xuICB9KTtcbiAgdGhpcy5yZXNvbHZlID0gYUZ1bmN0aW9uKHJlc29sdmUpO1xuICB0aGlzLnJlamVjdCAgPSBhRnVuY3Rpb24ocmVqZWN0KTtcbn07XG52YXIgcGVyZm9ybSA9IGZ1bmN0aW9uKGV4ZWMpe1xuICB0cnkge1xuICAgIGV4ZWMoKTtcbiAgfSBjYXRjaChlKXtcbiAgICByZXR1cm4ge2Vycm9yOiBlfTtcbiAgfVxufTtcbnZhciBub3RpZnkgPSBmdW5jdGlvbihwcm9taXNlLCBpc1JlamVjdCl7XG4gIGlmKHByb21pc2UuX24pcmV0dXJuO1xuICBwcm9taXNlLl9uID0gdHJ1ZTtcbiAgdmFyIGNoYWluID0gcHJvbWlzZS5fYztcbiAgbWljcm90YXNrKGZ1bmN0aW9uKCl7XG4gICAgdmFyIHZhbHVlID0gcHJvbWlzZS5fdlxuICAgICAgLCBvayAgICA9IHByb21pc2UuX3MgPT0gMVxuICAgICAgLCBpICAgICA9IDA7XG4gICAgdmFyIHJ1biA9IGZ1bmN0aW9uKHJlYWN0aW9uKXtcbiAgICAgIHZhciBoYW5kbGVyID0gb2sgPyByZWFjdGlvbi5vayA6IHJlYWN0aW9uLmZhaWxcbiAgICAgICAgLCByZXNvbHZlID0gcmVhY3Rpb24ucmVzb2x2ZVxuICAgICAgICAsIHJlamVjdCAgPSByZWFjdGlvbi5yZWplY3RcbiAgICAgICAgLCBkb21haW4gID0gcmVhY3Rpb24uZG9tYWluXG4gICAgICAgICwgcmVzdWx0LCB0aGVuO1xuICAgICAgdHJ5IHtcbiAgICAgICAgaWYoaGFuZGxlcil7XG4gICAgICAgICAgaWYoIW9rKXtcbiAgICAgICAgICAgIGlmKHByb21pc2UuX2ggPT0gMilvbkhhbmRsZVVuaGFuZGxlZChwcm9taXNlKTtcbiAgICAgICAgICAgIHByb21pc2UuX2ggPSAxO1xuICAgICAgICAgIH1cbiAgICAgICAgICBpZihoYW5kbGVyID09PSB0cnVlKXJlc3VsdCA9IHZhbHVlO1xuICAgICAgICAgIGVsc2Uge1xuICAgICAgICAgICAgaWYoZG9tYWluKWRvbWFpbi5lbnRlcigpO1xuICAgICAgICAgICAgcmVzdWx0ID0gaGFuZGxlcih2YWx1ZSk7XG4gICAgICAgICAgICBpZihkb21haW4pZG9tYWluLmV4aXQoKTtcbiAgICAgICAgICB9XG4gICAgICAgICAgaWYocmVzdWx0ID09PSByZWFjdGlvbi5wcm9taXNlKXtcbiAgICAgICAgICAgIHJlamVjdChUeXBlRXJyb3IoJ1Byb21pc2UtY2hhaW4gY3ljbGUnKSk7XG4gICAgICAgICAgfSBlbHNlIGlmKHRoZW4gPSBpc1RoZW5hYmxlKHJlc3VsdCkpe1xuICAgICAgICAgICAgdGhlbi5jYWxsKHJlc3VsdCwgcmVzb2x2ZSwgcmVqZWN0KTtcbiAgICAgICAgICB9IGVsc2UgcmVzb2x2ZShyZXN1bHQpO1xuICAgICAgICB9IGVsc2UgcmVqZWN0KHZhbHVlKTtcbiAgICAgIH0gY2F0Y2goZSl7XG4gICAgICAgIHJlamVjdChlKTtcbiAgICAgIH1cbiAgICB9O1xuICAgIHdoaWxlKGNoYWluLmxlbmd0aCA+IGkpcnVuKGNoYWluW2krK10pOyAvLyB2YXJpYWJsZSBsZW5ndGggLSBjYW4ndCB1c2UgZm9yRWFjaFxuICAgIHByb21pc2UuX2MgPSBbXTtcbiAgICBwcm9taXNlLl9uID0gZmFsc2U7XG4gICAgaWYoaXNSZWplY3QgJiYgIXByb21pc2UuX2gpb25VbmhhbmRsZWQocHJvbWlzZSk7XG4gIH0pO1xufTtcbnZhciBvblVuaGFuZGxlZCA9IGZ1bmN0aW9uKHByb21pc2Upe1xuICB0YXNrLmNhbGwoZ2xvYmFsLCBmdW5jdGlvbigpe1xuICAgIHZhciB2YWx1ZSA9IHByb21pc2UuX3ZcbiAgICAgICwgYWJydXB0LCBoYW5kbGVyLCBjb25zb2xlO1xuICAgIGlmKGlzVW5oYW5kbGVkKHByb21pc2UpKXtcbiAgICAgIGFicnVwdCA9IHBlcmZvcm0oZnVuY3Rpb24oKXtcbiAgICAgICAgaWYoaXNOb2RlKXtcbiAgICAgICAgICBwcm9jZXNzLmVtaXQoJ3VuaGFuZGxlZFJlamVjdGlvbicsIHZhbHVlLCBwcm9taXNlKTtcbiAgICAgICAgfSBlbHNlIGlmKGhhbmRsZXIgPSBnbG9iYWwub251bmhhbmRsZWRyZWplY3Rpb24pe1xuICAgICAgICAgIGhhbmRsZXIoe3Byb21pc2U6IHByb21pc2UsIHJlYXNvbjogdmFsdWV9KTtcbiAgICAgICAgfSBlbHNlIGlmKChjb25zb2xlID0gZ2xvYmFsLmNvbnNvbGUpICYmIGNvbnNvbGUuZXJyb3Ipe1xuICAgICAgICAgIGNvbnNvbGUuZXJyb3IoJ1VuaGFuZGxlZCBwcm9taXNlIHJlamVjdGlvbicsIHZhbHVlKTtcbiAgICAgICAgfVxuICAgICAgfSk7XG4gICAgICAvLyBCcm93c2VycyBzaG91bGQgbm90IHRyaWdnZXIgYHJlamVjdGlvbkhhbmRsZWRgIGV2ZW50IGlmIGl0IHdhcyBoYW5kbGVkIGhlcmUsIE5vZGVKUyAtIHNob3VsZFxuICAgICAgcHJvbWlzZS5faCA9IGlzTm9kZSB8fCBpc1VuaGFuZGxlZChwcm9taXNlKSA/IDIgOiAxO1xuICAgIH0gcHJvbWlzZS5fYSA9IHVuZGVmaW5lZDtcbiAgICBpZihhYnJ1cHQpdGhyb3cgYWJydXB0LmVycm9yO1xuICB9KTtcbn07XG52YXIgaXNVbmhhbmRsZWQgPSBmdW5jdGlvbihwcm9taXNlKXtcbiAgaWYocHJvbWlzZS5faCA9PSAxKXJldHVybiBmYWxzZTtcbiAgdmFyIGNoYWluID0gcHJvbWlzZS5fYSB8fCBwcm9taXNlLl9jXG4gICAgLCBpICAgICA9IDBcbiAgICAsIHJlYWN0aW9uO1xuICB3aGlsZShjaGFpbi5sZW5ndGggPiBpKXtcbiAgICByZWFjdGlvbiA9IGNoYWluW2krK107XG4gICAgaWYocmVhY3Rpb24uZmFpbCB8fCAhaXNVbmhhbmRsZWQocmVhY3Rpb24ucHJvbWlzZSkpcmV0dXJuIGZhbHNlO1xuICB9IHJldHVybiB0cnVlO1xufTtcbnZhciBvbkhhbmRsZVVuaGFuZGxlZCA9IGZ1bmN0aW9uKHByb21pc2Upe1xuICB0YXNrLmNhbGwoZ2xvYmFsLCBmdW5jdGlvbigpe1xuICAgIHZhciBoYW5kbGVyO1xuICAgIGlmKGlzTm9kZSl7XG4gICAgICBwcm9jZXNzLmVtaXQoJ3JlamVjdGlvbkhhbmRsZWQnLCBwcm9taXNlKTtcbiAgICB9IGVsc2UgaWYoaGFuZGxlciA9IGdsb2JhbC5vbnJlamVjdGlvbmhhbmRsZWQpe1xuICAgICAgaGFuZGxlcih7cHJvbWlzZTogcHJvbWlzZSwgcmVhc29uOiBwcm9taXNlLl92fSk7XG4gICAgfVxuICB9KTtcbn07XG52YXIgJHJlamVjdCA9IGZ1bmN0aW9uKHZhbHVlKXtcbiAgdmFyIHByb21pc2UgPSB0aGlzO1xuICBpZihwcm9taXNlLl9kKXJldHVybjtcbiAgcHJvbWlzZS5fZCA9IHRydWU7XG4gIHByb21pc2UgPSBwcm9taXNlLl93IHx8IHByb21pc2U7IC8vIHVud3JhcFxuICBwcm9taXNlLl92ID0gdmFsdWU7XG4gIHByb21pc2UuX3MgPSAyO1xuICBpZighcHJvbWlzZS5fYSlwcm9taXNlLl9hID0gcHJvbWlzZS5fYy5zbGljZSgpO1xuICBub3RpZnkocHJvbWlzZSwgdHJ1ZSk7XG59O1xudmFyICRyZXNvbHZlID0gZnVuY3Rpb24odmFsdWUpe1xuICB2YXIgcHJvbWlzZSA9IHRoaXNcbiAgICAsIHRoZW47XG4gIGlmKHByb21pc2UuX2QpcmV0dXJuO1xuICBwcm9taXNlLl9kID0gdHJ1ZTtcbiAgcHJvbWlzZSA9IHByb21pc2UuX3cgfHwgcHJvbWlzZTsgLy8gdW53cmFwXG4gIHRyeSB7XG4gICAgaWYocHJvbWlzZSA9PT0gdmFsdWUpdGhyb3cgVHlwZUVycm9yKFwiUHJvbWlzZSBjYW4ndCBiZSByZXNvbHZlZCBpdHNlbGZcIik7XG4gICAgaWYodGhlbiA9IGlzVGhlbmFibGUodmFsdWUpKXtcbiAgICAgIG1pY3JvdGFzayhmdW5jdGlvbigpe1xuICAgICAgICB2YXIgd3JhcHBlciA9IHtfdzogcHJvbWlzZSwgX2Q6IGZhbHNlfTsgLy8gd3JhcFxuICAgICAgICB0cnkge1xuICAgICAgICAgIHRoZW4uY2FsbCh2YWx1ZSwgY3R4KCRyZXNvbHZlLCB3cmFwcGVyLCAxKSwgY3R4KCRyZWplY3QsIHdyYXBwZXIsIDEpKTtcbiAgICAgICAgfSBjYXRjaChlKXtcbiAgICAgICAgICAkcmVqZWN0LmNhbGwod3JhcHBlciwgZSk7XG4gICAgICAgIH1cbiAgICAgIH0pO1xuICAgIH0gZWxzZSB7XG4gICAgICBwcm9taXNlLl92ID0gdmFsdWU7XG4gICAgICBwcm9taXNlLl9zID0gMTtcbiAgICAgIG5vdGlmeShwcm9taXNlLCBmYWxzZSk7XG4gICAgfVxuICB9IGNhdGNoKGUpe1xuICAgICRyZWplY3QuY2FsbCh7X3c6IHByb21pc2UsIF9kOiBmYWxzZX0sIGUpOyAvLyB3cmFwXG4gIH1cbn07XG5cbi8vIGNvbnN0cnVjdG9yIHBvbHlmaWxsXG5pZighVVNFX05BVElWRSl7XG4gIC8vIDI1LjQuMy4xIFByb21pc2UoZXhlY3V0b3IpXG4gICRQcm9taXNlID0gZnVuY3Rpb24gUHJvbWlzZShleGVjdXRvcil7XG4gICAgYW5JbnN0YW5jZSh0aGlzLCAkUHJvbWlzZSwgUFJPTUlTRSwgJ19oJyk7XG4gICAgYUZ1bmN0aW9uKGV4ZWN1dG9yKTtcbiAgICBJbnRlcm5hbC5jYWxsKHRoaXMpO1xuICAgIHRyeSB7XG4gICAgICBleGVjdXRvcihjdHgoJHJlc29sdmUsIHRoaXMsIDEpLCBjdHgoJHJlamVjdCwgdGhpcywgMSkpO1xuICAgIH0gY2F0Y2goZXJyKXtcbiAgICAgICRyZWplY3QuY2FsbCh0aGlzLCBlcnIpO1xuICAgIH1cbiAgfTtcbiAgSW50ZXJuYWwgPSBmdW5jdGlvbiBQcm9taXNlKGV4ZWN1dG9yKXtcbiAgICB0aGlzLl9jID0gW107ICAgICAgICAgICAgIC8vIDwtIGF3YWl0aW5nIHJlYWN0aW9uc1xuICAgIHRoaXMuX2EgPSB1bmRlZmluZWQ7ICAgICAgLy8gPC0gY2hlY2tlZCBpbiBpc1VuaGFuZGxlZCByZWFjdGlvbnNcbiAgICB0aGlzLl9zID0gMDsgICAgICAgICAgICAgIC8vIDwtIHN0YXRlXG4gICAgdGhpcy5fZCA9IGZhbHNlOyAgICAgICAgICAvLyA8LSBkb25lXG4gICAgdGhpcy5fdiA9IHVuZGVmaW5lZDsgICAgICAvLyA8LSB2YWx1ZVxuICAgIHRoaXMuX2ggPSAwOyAgICAgICAgICAgICAgLy8gPC0gcmVqZWN0aW9uIHN0YXRlLCAwIC0gZGVmYXVsdCwgMSAtIGhhbmRsZWQsIDIgLSB1bmhhbmRsZWRcbiAgICB0aGlzLl9uID0gZmFsc2U7ICAgICAgICAgIC8vIDwtIG5vdGlmeVxuICB9O1xuICBJbnRlcm5hbC5wcm90b3R5cGUgPSByZXF1aXJlKCcuL19yZWRlZmluZS1hbGwnKSgkUHJvbWlzZS5wcm90b3R5cGUsIHtcbiAgICAvLyAyNS40LjUuMyBQcm9taXNlLnByb3RvdHlwZS50aGVuKG9uRnVsZmlsbGVkLCBvblJlamVjdGVkKVxuICAgIHRoZW46IGZ1bmN0aW9uIHRoZW4ob25GdWxmaWxsZWQsIG9uUmVqZWN0ZWQpe1xuICAgICAgdmFyIHJlYWN0aW9uICAgID0gbmV3UHJvbWlzZUNhcGFiaWxpdHkoc3BlY2llc0NvbnN0cnVjdG9yKHRoaXMsICRQcm9taXNlKSk7XG4gICAgICByZWFjdGlvbi5vayAgICAgPSB0eXBlb2Ygb25GdWxmaWxsZWQgPT0gJ2Z1bmN0aW9uJyA/IG9uRnVsZmlsbGVkIDogdHJ1ZTtcbiAgICAgIHJlYWN0aW9uLmZhaWwgICA9IHR5cGVvZiBvblJlamVjdGVkID09ICdmdW5jdGlvbicgJiYgb25SZWplY3RlZDtcbiAgICAgIHJlYWN0aW9uLmRvbWFpbiA9IGlzTm9kZSA/IHByb2Nlc3MuZG9tYWluIDogdW5kZWZpbmVkO1xuICAgICAgdGhpcy5fYy5wdXNoKHJlYWN0aW9uKTtcbiAgICAgIGlmKHRoaXMuX2EpdGhpcy5fYS5wdXNoKHJlYWN0aW9uKTtcbiAgICAgIGlmKHRoaXMuX3Mpbm90aWZ5KHRoaXMsIGZhbHNlKTtcbiAgICAgIHJldHVybiByZWFjdGlvbi5wcm9taXNlO1xuICAgIH0sXG4gICAgLy8gMjUuNC41LjEgUHJvbWlzZS5wcm90b3R5cGUuY2F0Y2gob25SZWplY3RlZClcbiAgICAnY2F0Y2gnOiBmdW5jdGlvbihvblJlamVjdGVkKXtcbiAgICAgIHJldHVybiB0aGlzLnRoZW4odW5kZWZpbmVkLCBvblJlamVjdGVkKTtcbiAgICB9XG4gIH0pO1xuICBQcm9taXNlQ2FwYWJpbGl0eSA9IGZ1bmN0aW9uKCl7XG4gICAgdmFyIHByb21pc2UgID0gbmV3IEludGVybmFsO1xuICAgIHRoaXMucHJvbWlzZSA9IHByb21pc2U7XG4gICAgdGhpcy5yZXNvbHZlID0gY3R4KCRyZXNvbHZlLCBwcm9taXNlLCAxKTtcbiAgICB0aGlzLnJlamVjdCAgPSBjdHgoJHJlamVjdCwgcHJvbWlzZSwgMSk7XG4gIH07XG59XG5cbiRleHBvcnQoJGV4cG9ydC5HICsgJGV4cG9ydC5XICsgJGV4cG9ydC5GICogIVVTRV9OQVRJVkUsIHtQcm9taXNlOiAkUHJvbWlzZX0pO1xucmVxdWlyZSgnLi9fc2V0LXRvLXN0cmluZy10YWcnKSgkUHJvbWlzZSwgUFJPTUlTRSk7XG5yZXF1aXJlKCcuL19zZXQtc3BlY2llcycpKFBST01JU0UpO1xuV3JhcHBlciA9IHJlcXVpcmUoJy4vX2NvcmUnKVtQUk9NSVNFXTtcblxuLy8gc3RhdGljc1xuJGV4cG9ydCgkZXhwb3J0LlMgKyAkZXhwb3J0LkYgKiAhVVNFX05BVElWRSwgUFJPTUlTRSwge1xuICAvLyAyNS40LjQuNSBQcm9taXNlLnJlamVjdChyKVxuICByZWplY3Q6IGZ1bmN0aW9uIHJlamVjdChyKXtcbiAgICB2YXIgY2FwYWJpbGl0eSA9IG5ld1Byb21pc2VDYXBhYmlsaXR5KHRoaXMpXG4gICAgICAsICQkcmVqZWN0ICAgPSBjYXBhYmlsaXR5LnJlamVjdDtcbiAgICAkJHJlamVjdChyKTtcbiAgICByZXR1cm4gY2FwYWJpbGl0eS5wcm9taXNlO1xuICB9XG59KTtcbiRleHBvcnQoJGV4cG9ydC5TICsgJGV4cG9ydC5GICogKExJQlJBUlkgfHwgIVVTRV9OQVRJVkUpLCBQUk9NSVNFLCB7XG4gIC8vIDI1LjQuNC42IFByb21pc2UucmVzb2x2ZSh4KVxuICByZXNvbHZlOiBmdW5jdGlvbiByZXNvbHZlKHgpe1xuICAgIC8vIGluc3RhbmNlb2YgaW5zdGVhZCBvZiBpbnRlcm5hbCBzbG90IGNoZWNrIGJlY2F1c2Ugd2Ugc2hvdWxkIGZpeCBpdCB3aXRob3V0IHJlcGxhY2VtZW50IG5hdGl2ZSBQcm9taXNlIGNvcmVcbiAgICBpZih4IGluc3RhbmNlb2YgJFByb21pc2UgJiYgc2FtZUNvbnN0cnVjdG9yKHguY29uc3RydWN0b3IsIHRoaXMpKXJldHVybiB4O1xuICAgIHZhciBjYXBhYmlsaXR5ID0gbmV3UHJvbWlzZUNhcGFiaWxpdHkodGhpcylcbiAgICAgICwgJCRyZXNvbHZlICA9IGNhcGFiaWxpdHkucmVzb2x2ZTtcbiAgICAkJHJlc29sdmUoeCk7XG4gICAgcmV0dXJuIGNhcGFiaWxpdHkucHJvbWlzZTtcbiAgfVxufSk7XG4kZXhwb3J0KCRleHBvcnQuUyArICRleHBvcnQuRiAqICEoVVNFX05BVElWRSAmJiByZXF1aXJlKCcuL19pdGVyLWRldGVjdCcpKGZ1bmN0aW9uKGl0ZXIpe1xuICAkUHJvbWlzZS5hbGwoaXRlcilbJ2NhdGNoJ10oZW1wdHkpO1xufSkpLCBQUk9NSVNFLCB7XG4gIC8vIDI1LjQuNC4xIFByb21pc2UuYWxsKGl0ZXJhYmxlKVxuICBhbGw6IGZ1bmN0aW9uIGFsbChpdGVyYWJsZSl7XG4gICAgdmFyIEMgICAgICAgICAgPSB0aGlzXG4gICAgICAsIGNhcGFiaWxpdHkgPSBuZXdQcm9taXNlQ2FwYWJpbGl0eShDKVxuICAgICAgLCByZXNvbHZlICAgID0gY2FwYWJpbGl0eS5yZXNvbHZlXG4gICAgICAsIHJlamVjdCAgICAgPSBjYXBhYmlsaXR5LnJlamVjdDtcbiAgICB2YXIgYWJydXB0ID0gcGVyZm9ybShmdW5jdGlvbigpe1xuICAgICAgdmFyIHZhbHVlcyAgICA9IFtdXG4gICAgICAgICwgaW5kZXggICAgID0gMFxuICAgICAgICAsIHJlbWFpbmluZyA9IDE7XG4gICAgICBmb3JPZihpdGVyYWJsZSwgZmFsc2UsIGZ1bmN0aW9uKHByb21pc2Upe1xuICAgICAgICB2YXIgJGluZGV4ICAgICAgICA9IGluZGV4KytcbiAgICAgICAgICAsIGFscmVhZHlDYWxsZWQgPSBmYWxzZTtcbiAgICAgICAgdmFsdWVzLnB1c2godW5kZWZpbmVkKTtcbiAgICAgICAgcmVtYWluaW5nKys7XG4gICAgICAgIEMucmVzb2x2ZShwcm9taXNlKS50aGVuKGZ1bmN0aW9uKHZhbHVlKXtcbiAgICAgICAgICBpZihhbHJlYWR5Q2FsbGVkKXJldHVybjtcbiAgICAgICAgICBhbHJlYWR5Q2FsbGVkICA9IHRydWU7XG4gICAgICAgICAgdmFsdWVzWyRpbmRleF0gPSB2YWx1ZTtcbiAgICAgICAgICAtLXJlbWFpbmluZyB8fCByZXNvbHZlKHZhbHVlcyk7XG4gICAgICAgIH0sIHJlamVjdCk7XG4gICAgICB9KTtcbiAgICAgIC0tcmVtYWluaW5nIHx8IHJlc29sdmUodmFsdWVzKTtcbiAgICB9KTtcbiAgICBpZihhYnJ1cHQpcmVqZWN0KGFicnVwdC5lcnJvcik7XG4gICAgcmV0dXJuIGNhcGFiaWxpdHkucHJvbWlzZTtcbiAgfSxcbiAgLy8gMjUuNC40LjQgUHJvbWlzZS5yYWNlKGl0ZXJhYmxlKVxuICByYWNlOiBmdW5jdGlvbiByYWNlKGl0ZXJhYmxlKXtcbiAgICB2YXIgQyAgICAgICAgICA9IHRoaXNcbiAgICAgICwgY2FwYWJpbGl0eSA9IG5ld1Byb21pc2VDYXBhYmlsaXR5KEMpXG4gICAgICAsIHJlamVjdCAgICAgPSBjYXBhYmlsaXR5LnJlamVjdDtcbiAgICB2YXIgYWJydXB0ID0gcGVyZm9ybShmdW5jdGlvbigpe1xuICAgICAgZm9yT2YoaXRlcmFibGUsIGZhbHNlLCBmdW5jdGlvbihwcm9taXNlKXtcbiAgICAgICAgQy5yZXNvbHZlKHByb21pc2UpLnRoZW4oY2FwYWJpbGl0eS5yZXNvbHZlLCByZWplY3QpO1xuICAgICAgfSk7XG4gICAgfSk7XG4gICAgaWYoYWJydXB0KXJlamVjdChhYnJ1cHQuZXJyb3IpO1xuICAgIHJldHVybiBjYXBhYmlsaXR5LnByb21pc2U7XG4gIH1cbn0pOyIsIi8qXG4gKiBMaWNlbnNlZCB0byB0aGUgQXBhY2hlIFNvZnR3YXJlIEZvdW5kYXRpb24gKEFTRikgdW5kZXIgb25lXG4gKiBvciBtb3JlIGNvbnRyaWJ1dG9yIGxpY2Vuc2UgYWdyZWVtZW50cy4gIFNlZSB0aGUgTk9USUNFIGZpbGVcbiAqIGRpc3RyaWJ1dGVkIHdpdGggdGhpcyB3b3JrIGZvciBhZGRpdGlvbmFsIGluZm9ybWF0aW9uXG4gKiByZWdhcmRpbmcgY29weXJpZ2h0IG93bmVyc2hpcC4gIFRoZSBBU0YgbGljZW5zZXMgdGhpcyBmaWxlXG4gKiB0byB5b3UgdW5kZXIgdGhlIEFwYWNoZSBMaWNlbnNlLCBWZXJzaW9uIDIuMCAodGhlXG4gKiBcIkxpY2Vuc2VcIik7IHlvdSBtYXkgbm90IHVzZSB0aGlzIGZpbGUgZXhjZXB0IGluIGNvbXBsaWFuY2VcbiAqIHdpdGggdGhlIExpY2Vuc2UuICBZb3UgbWF5IG9idGFpbiBhIGNvcHkgb2YgdGhlIExpY2Vuc2UgYXRcbiAqXG4gKiAgIGh0dHA6Ly93d3cuYXBhY2hlLm9yZy9saWNlbnNlcy9MSUNFTlNFLTIuMFxuICpcbiAqIFVubGVzcyByZXF1aXJlZCBieSBhcHBsaWNhYmxlIGxhdyBvciBhZ3JlZWQgdG8gaW4gd3JpdGluZyxcbiAqIHNvZnR3YXJlIGRpc3RyaWJ1dGVkIHVuZGVyIHRoZSBMaWNlbnNlIGlzIGRpc3RyaWJ1dGVkIG9uIGFuXG4gKiBcIkFTIElTXCIgQkFTSVMsIFdJVEhPVVQgV0FSUkFOVElFUyBPUiBDT05ESVRJT05TIE9GIEFOWVxuICogS0lORCwgZWl0aGVyIGV4cHJlc3Mgb3IgaW1wbGllZC4gIFNlZSB0aGUgTGljZW5zZSBmb3IgdGhlXG4gKiBzcGVjaWZpYyBsYW5ndWFnZSBnb3Zlcm5pbmcgcGVybWlzc2lvbnMgYW5kIGxpbWl0YXRpb25zXG4gKiB1bmRlciB0aGUgTGljZW5zZS5cbiAqL1xuXG4vKipcbiAqIEBmaWxlT3ZlcnZpZXdcbiAqIFRoaXMgZmlsZSB3aWxsIGhhY2sgYGNvbnNvbGVgIG1ldGhvZHMgYnkgYFdYRW52aXJvbm1lbnQubG9nTGV2ZWxgLlxuICogU28gd2UgY2FuIGNvbnRyb2wgaG93IG1hbnkgYW5kIHdoaWNoIG1lc3NhZ2VzIHdpbGwgYmUgc2VudCBieSBjaGFuZ2UgdGhlIGxvZyBsZXZlbC5cbiAqIEFkZGl0aW9uYWxseSBpbiBuYXRpdmUgcGxhdGZvcm0gdGhlIG1lc3NhZ2UgY29udGVudCBtdXN0IGJlIHByaW1pdGl2ZSB2YWx1ZXMgYW5kXG4gKiB1c2luZyBgbmF0aXZlTG9nKC4uLmFyZ3MsIGxvZ0xldmVsTWFyaylgIHNvIHdlIGNyZWF0ZSBhIG5ldyBgY29uc29sZWAgb2JqZWN0IGluXG4gKiBnbG9iYWwgYWRkIGEgZm9ybWF0IHByb2Nlc3MgZm9yIGl0cyBtZXRob2RzLlxuICovXG5cbmNvbnN0IExFVkVMUyA9IFsnb2ZmJywgJ2Vycm9yJywgJ3dhcm4nLCAnaW5mbycsICdsb2cnLCAnZGVidWcnXVxubGV0IGxldmVsTWFwID0ge31cblxuY29uc3Qgb3JpZ2luYWxDb25zb2xlID0gZ2xvYmFsLmNvbnNvbGVcblxuLyoqXG4gKiBIYWNrIGNvbnNvbGUgZm9yIG5hdGl2ZSBlbnZpcm9ubWVudC5cbiAqL1xuZXhwb3J0IGZ1bmN0aW9uIHNldE5hdGl2ZUNvbnNvbGUgKCkge1xuICBnZW5lcmF0ZUxldmVsTWFwKClcblxuICAvKiBpc3RhbmJ1bCBpZ25vcmUgbmV4dCAqL1xuICAvLyBtb2NrIGNvbnNvbGUgaW4gbmF0aXZlIGVudmlyb25tZW50XG4gIGlmIChnbG9iYWwuV1hFbnZpcm9ubWVudCAmJiBnbG9iYWwuV1hFbnZpcm9ubWVudC5wbGF0Zm9ybSAhPT0gJ1dlYicpIHtcbiAgICBnbG9iYWwuY29uc29sZSA9IHtcbiAgICAgIGRlYnVnOiAoLi4uYXJncykgPT4ge1xuICAgICAgICBpZiAoY2hlY2tMZXZlbCgnZGVidWcnKSkgeyBnbG9iYWwubmF0aXZlTG9nKC4uLmZvcm1hdChhcmdzKSwgJ19fREVCVUcnKSB9XG4gICAgICB9LFxuICAgICAgbG9nOiAoLi4uYXJncykgPT4ge1xuICAgICAgICBpZiAoY2hlY2tMZXZlbCgnbG9nJykpIHsgZ2xvYmFsLm5hdGl2ZUxvZyguLi5mb3JtYXQoYXJncyksICdfX0xPRycpIH1cbiAgICAgIH0sXG4gICAgICBpbmZvOiAoLi4uYXJncykgPT4ge1xuICAgICAgICBpZiAoY2hlY2tMZXZlbCgnaW5mbycpKSB7IGdsb2JhbC5uYXRpdmVMb2coLi4uZm9ybWF0KGFyZ3MpLCAnX19JTkZPJykgfVxuICAgICAgfSxcbiAgICAgIHdhcm46ICguLi5hcmdzKSA9PiB7XG4gICAgICAgIGlmIChjaGVja0xldmVsKCd3YXJuJykpIHsgZ2xvYmFsLm5hdGl2ZUxvZyguLi5mb3JtYXQoYXJncyksICdfX1dBUk4nKSB9XG4gICAgICB9LFxuICAgICAgZXJyb3I6ICguLi5hcmdzKSA9PiB7XG4gICAgICAgIGlmIChjaGVja0xldmVsKCdlcnJvcicpKSB7IGdsb2JhbC5uYXRpdmVMb2coLi4uZm9ybWF0KGFyZ3MpLCAnX19FUlJPUicpIH1cbiAgICAgIH1cbiAgICB9XG4gIH1cblxuICAvLyBXZWIgb3IgTm9kZVxuICBlbHNlIHtcbiAgICBjb25zdCB7IGRlYnVnLCBsb2csIGluZm8sIHdhcm4sIGVycm9yIH0gPSBjb25zb2xlXG4gICAgY29uc29sZS5fX29yaV9fID0geyBkZWJ1ZywgbG9nLCBpbmZvLCB3YXJuLCBlcnJvciB9XG4gICAgY29uc29sZS5kZWJ1ZyA9ICguLi5hcmdzKSA9PiB7XG4gICAgICBpZiAoY2hlY2tMZXZlbCgnZGVidWcnKSkgeyBjb25zb2xlLl9fb3JpX18uZGVidWcuYXBwbHkoY29uc29sZSwgYXJncykgfVxuICAgIH1cbiAgICBjb25zb2xlLmxvZyA9ICguLi5hcmdzKSA9PiB7XG4gICAgICBpZiAoY2hlY2tMZXZlbCgnbG9nJykpIHsgY29uc29sZS5fX29yaV9fLmxvZy5hcHBseShjb25zb2xlLCBhcmdzKSB9XG4gICAgfVxuICAgIGNvbnNvbGUuaW5mbyA9ICguLi5hcmdzKSA9PiB7XG4gICAgICBpZiAoY2hlY2tMZXZlbCgnaW5mbycpKSB7IGNvbnNvbGUuX19vcmlfXy5pbmZvLmFwcGx5KGNvbnNvbGUsIGFyZ3MpIH1cbiAgICB9XG4gICAgY29uc29sZS53YXJuID0gKC4uLmFyZ3MpID0+IHtcbiAgICAgIGlmIChjaGVja0xldmVsKCd3YXJuJykpIHsgY29uc29sZS5fX29yaV9fLndhcm4uYXBwbHkoY29uc29sZSwgYXJncykgfVxuICAgIH1cbiAgICBjb25zb2xlLmVycm9yID0gKC4uLmFyZ3MpID0+IHtcbiAgICAgIGlmIChjaGVja0xldmVsKCdlcnJvcicpKSB7IGNvbnNvbGUuX19vcmlfXy5lcnJvci5hcHBseShjb25zb2xlLCBhcmdzKSB9XG4gICAgfVxuICB9XG59XG5cbi8qKlxuICogUmVzZXQgaGFja2VkIGNvbnNvbGUgdG8gb3JpZ2luYWwuXG4gKi9cbi8qIGlzdGFuYnVsIGlnbm9yZSBuZXh0ICovXG5leHBvcnQgZnVuY3Rpb24gcmVzZXROYXRpdmVDb25zb2xlICgpIHtcbiAgbGV2ZWxNYXAgPSB7fVxuICBnbG9iYWwuY29uc29sZSA9IG9yaWdpbmFsQ29uc29sZVxufVxuXG4vKipcbiAqIEdlbmVyYXRlIG1hcCBmb3Igd2hpY2ggdHlwZXMgb2YgbWVzc2FnZSB3aWxsIGJlIHNlbnQgaW4gYSBjZXJ0YWluIG1lc3NhZ2UgbGV2ZWxcbiAqIGFzIHRoZSBvcmRlciBvZiBMRVZFTFMuXG4gKi9cbmZ1bmN0aW9uIGdlbmVyYXRlTGV2ZWxNYXAgKCkge1xuICBMRVZFTFMuZm9yRWFjaChsZXZlbCA9PiB7XG4gICAgY29uc3QgbGV2ZWxJbmRleCA9IExFVkVMUy5pbmRleE9mKGxldmVsKVxuICAgIGxldmVsTWFwW2xldmVsXSA9IHt9XG4gICAgTEVWRUxTLmZvckVhY2godHlwZSA9PiB7XG4gICAgICBjb25zdCB0eXBlSW5kZXggPSBMRVZFTFMuaW5kZXhPZih0eXBlKVxuICAgICAgaWYgKHR5cGVJbmRleCA8PSBsZXZlbEluZGV4KSB7XG4gICAgICAgIGxldmVsTWFwW2xldmVsXVt0eXBlXSA9IHRydWVcbiAgICAgIH1cbiAgICB9KVxuICB9KVxufVxuXG4vKipcbiAqIENoZWNrIGlmIGEgY2VydGFpbiB0eXBlIG9mIG1lc3NhZ2Ugd2lsbCBiZSBzZW50IGluIGN1cnJlbnQgbG9nIGxldmVsIG9mIGVudi5cbiAqIEBwYXJhbSAge3N0cmluZ30gdHlwZVxuICogQHJldHVybiB7Ym9vbGVhbn1cbiAqL1xuZnVuY3Rpb24gY2hlY2tMZXZlbCAodHlwZSkge1xuICBjb25zdCBsb2dMZXZlbCA9IChnbG9iYWwuV1hFbnZpcm9ubWVudCAmJiBnbG9iYWwuV1hFbnZpcm9ubWVudC5sb2dMZXZlbCkgfHwgJ2xvZydcbiAgcmV0dXJuIGxldmVsTWFwW2xvZ0xldmVsXSAmJiBsZXZlbE1hcFtsb2dMZXZlbF1bdHlwZV1cbn1cblxuLyoqXG4gKiBDb252ZXJ0IGFsbCBsb2cgYXJndW1lbnRzIGludG8gcHJpbWl0aXZlIHZhbHVlcy5cbiAqIEBwYXJhbSAge2FycmF5fSBhcmdzXG4gKiBAcmV0dXJuIHthcnJheX1cbiAqL1xuLyogaXN0YW5idWwgaWdub3JlIG5leHQgKi9cbmZ1bmN0aW9uIGZvcm1hdCAoYXJncykge1xuICByZXR1cm4gYXJncy5tYXAoKHYpID0+IHtcbiAgICBjb25zdCB0eXBlID0gT2JqZWN0LnByb3RvdHlwZS50b1N0cmluZy5jYWxsKHYpXG4gICAgaWYgKHR5cGUudG9Mb3dlckNhc2UoKSA9PT0gJ1tvYmplY3Qgb2JqZWN0XScpIHtcbiAgICAgIHYgPSBKU09OLnN0cmluZ2lmeSh2KVxuICAgIH1cbiAgICBlbHNlIHtcbiAgICAgIHYgPSBTdHJpbmcodilcbiAgICB9XG4gICAgcmV0dXJuIHZcbiAgfSlcbn1cbiIsIi8qXG4gKiBMaWNlbnNlZCB0byB0aGUgQXBhY2hlIFNvZnR3YXJlIEZvdW5kYXRpb24gKEFTRikgdW5kZXIgb25lXG4gKiBvciBtb3JlIGNvbnRyaWJ1dG9yIGxpY2Vuc2UgYWdyZWVtZW50cy4gIFNlZSB0aGUgTk9USUNFIGZpbGVcbiAqIGRpc3RyaWJ1dGVkIHdpdGggdGhpcyB3b3JrIGZvciBhZGRpdGlvbmFsIGluZm9ybWF0aW9uXG4gKiByZWdhcmRpbmcgY29weXJpZ2h0IG93bmVyc2hpcC4gIFRoZSBBU0YgbGljZW5zZXMgdGhpcyBmaWxlXG4gKiB0byB5b3UgdW5kZXIgdGhlIEFwYWNoZSBMaWNlbnNlLCBWZXJzaW9uIDIuMCAodGhlXG4gKiBcIkxpY2Vuc2VcIik7IHlvdSBtYXkgbm90IHVzZSB0aGlzIGZpbGUgZXhjZXB0IGluIGNvbXBsaWFuY2VcbiAqIHdpdGggdGhlIExpY2Vuc2UuICBZb3UgbWF5IG9idGFpbiBhIGNvcHkgb2YgdGhlIExpY2Vuc2UgYXRcbiAqXG4gKiAgIGh0dHA6Ly93d3cuYXBhY2hlLm9yZy9saWNlbnNlcy9MSUNFTlNFLTIuMFxuICpcbiAqIFVubGVzcyByZXF1aXJlZCBieSBhcHBsaWNhYmxlIGxhdyBvciBhZ3JlZWQgdG8gaW4gd3JpdGluZyxcbiAqIHNvZnR3YXJlIGRpc3RyaWJ1dGVkIHVuZGVyIHRoZSBMaWNlbnNlIGlzIGRpc3RyaWJ1dGVkIG9uIGFuXG4gKiBcIkFTIElTXCIgQkFTSVMsIFdJVEhPVVQgV0FSUkFOVElFUyBPUiBDT05ESVRJT05TIE9GIEFOWVxuICogS0lORCwgZWl0aGVyIGV4cHJlc3Mgb3IgaW1wbGllZC4gIFNlZSB0aGUgTGljZW5zZSBmb3IgdGhlXG4gKiBzcGVjaWZpYyBsYW5ndWFnZSBnb3Zlcm5pbmcgcGVybWlzc2lvbnMgYW5kIGxpbWl0YXRpb25zXG4gKiB1bmRlciB0aGUgTGljZW5zZS5cbiAqL1xuXG4vKipcbiAqIEBmaWxlT3ZlcnZpZXdcbiAqIFBvbHlmaWxsIGBzZXRUaW1lb3V0YCBvbiBBbmRyb2lkIFY4IHVzaW5nIG5hdGl2ZSBtZXRob2RcbiAqIGBzZXRUaW1lb3V0TmF0aXZlKGNhbGxiYWNrSWQsIHRpbWUpYCBhbmQgSlMgbWV0aG9kXG4gKiBgc2V0VGltZW91dENhbGxiYWNrKGNhbGxiYWNrSWQpYC5cbiAqIFRoaXMgcG9seWZpbGwgaXMgb25seSB1c2VkIGluIHZpcnR1YWwtRE9NIGRpZmYgJiBmbHVzaCBhZ29yaXRobS4gTm90XG4gKiBhY2Nlc3NlZCBieSBKUyBCdW5kbGUgY29kZSAoVGhlIHRpbWVyIEFQSXMgcG9seWZpbGwgZm9yIEpTIEJ1bmRsZSBpcyBpblxuICogYGh0bWw1L2RlZmF1bHQvYXBwL2N0cmwuanNgKS5cbiAqL1xuXG5jb25zdCBvcmlnaW5hbFNldFRpbWVvdXQgPSBnbG9iYWwuc2V0VGltZW91dFxuY29uc3Qgc2V0VGltZW91dE5hdGl2ZSA9IGdsb2JhbC5zZXRUaW1lb3V0TmF0aXZlXG5cbi8qKlxuICogU2V0IHVwIG5hdGl2ZSB0aW1lclxuICovXG4vKiBpc3RhbmJ1bCBpZ25vcmUgbmV4dCAqL1xuZXhwb3J0IGZ1bmN0aW9uIHNldE5hdGl2ZVRpbWVyICgpIHtcbiAgaWYgKHR5cGVvZiBzZXRUaW1lb3V0ID09PSAndW5kZWZpbmVkJyAmJlxuICB0eXBlb2Ygc2V0VGltZW91dE5hdGl2ZSA9PT0gJ2Z1bmN0aW9uJykge1xuICAgIGNvbnN0IHRpbWVvdXRNYXAgPSB7fVxuICAgIGxldCB0aW1lb3V0SWQgPSAwXG5cbiAgICBnbG9iYWwuc2V0VGltZW91dCA9IChjYiwgdGltZSkgPT4ge1xuICAgICAgdGltZW91dE1hcFsrK3RpbWVvdXRJZF0gPSBjYlxuICAgICAgc2V0VGltZW91dE5hdGl2ZSh0aW1lb3V0SWQudG9TdHJpbmcoKSwgdGltZSlcbiAgICB9XG5cbiAgICBnbG9iYWwuc2V0VGltZW91dENhbGxiYWNrID0gKGlkKSA9PiB7XG4gICAgICBpZiAodHlwZW9mIHRpbWVvdXRNYXBbaWRdID09PSAnZnVuY3Rpb24nKSB7XG4gICAgICAgIHRpbWVvdXRNYXBbaWRdKClcbiAgICAgICAgZGVsZXRlIHRpbWVvdXRNYXBbaWRdXG4gICAgICB9XG4gICAgfVxuICB9XG59XG5cbi8qIGlzdGFuYnVsIGlnbm9yZSBuZXh0ICovXG5leHBvcnQgZnVuY3Rpb24gcmVzZXROYXRpdmVUaW1lciAoKSB7XG4gIGdsb2JhbC5zZXRUaW1lb3V0ID0gb3JpZ2luYWxTZXRUaW1lb3V0XG4gIGdsb2JhbC5zZXRUaW1lb3V0Q2FsbGJhY2sgPSBudWxsXG59XG5cbnNldE5hdGl2ZVRpbWVyKClcbiIsIi8qXG4gKiBMaWNlbnNlZCB0byB0aGUgQXBhY2hlIFNvZnR3YXJlIEZvdW5kYXRpb24gKEFTRikgdW5kZXIgb25lXG4gKiBvciBtb3JlIGNvbnRyaWJ1dG9yIGxpY2Vuc2UgYWdyZWVtZW50cy4gIFNlZSB0aGUgTk9USUNFIGZpbGVcbiAqIGRpc3RyaWJ1dGVkIHdpdGggdGhpcyB3b3JrIGZvciBhZGRpdGlvbmFsIGluZm9ybWF0aW9uXG4gKiByZWdhcmRpbmcgY29weXJpZ2h0IG93bmVyc2hpcC4gIFRoZSBBU0YgbGljZW5zZXMgdGhpcyBmaWxlXG4gKiB0byB5b3UgdW5kZXIgdGhlIEFwYWNoZSBMaWNlbnNlLCBWZXJzaW9uIDIuMCAodGhlXG4gKiBcIkxpY2Vuc2VcIik7IHlvdSBtYXkgbm90IHVzZSB0aGlzIGZpbGUgZXhjZXB0IGluIGNvbXBsaWFuY2VcbiAqIHdpdGggdGhlIExpY2Vuc2UuICBZb3UgbWF5IG9idGFpbiBhIGNvcHkgb2YgdGhlIExpY2Vuc2UgYXRcbiAqXG4gKiAgIGh0dHA6Ly93d3cuYXBhY2hlLm9yZy9saWNlbnNlcy9MSUNFTlNFLTIuMFxuICpcbiAqIFVubGVzcyByZXF1aXJlZCBieSBhcHBsaWNhYmxlIGxhdyBvciBhZ3JlZWQgdG8gaW4gd3JpdGluZyxcbiAqIHNvZnR3YXJlIGRpc3RyaWJ1dGVkIHVuZGVyIHRoZSBMaWNlbnNlIGlzIGRpc3RyaWJ1dGVkIG9uIGFuXG4gKiBcIkFTIElTXCIgQkFTSVMsIFdJVEhPVVQgV0FSUkFOVElFUyBPUiBDT05ESVRJT05TIE9GIEFOWVxuICogS0lORCwgZWl0aGVyIGV4cHJlc3Mgb3IgaW1wbGllZC4gIFNlZSB0aGUgTGljZW5zZSBmb3IgdGhlXG4gKiBzcGVjaWZpYyBsYW5ndWFnZSBnb3Zlcm5pbmcgcGVybWlzc2lvbnMgYW5kIGxpbWl0YXRpb25zXG4gKiB1bmRlciB0aGUgTGljZW5zZS5cbiAqL1xuLyoqXG4gKiBGcmVlemUgdGhlIHByb3RvdHlwZSBvZiBqYXZhc2NyaXB0IGJ1aWxkLWluIG9iamVjdHMuXG4gKi9cbi8qIGlzdGFuYnVsIGlnbm9yZSBuZXh0ICovXG5leHBvcnQgZnVuY3Rpb24gZnJlZXplUHJvdG90eXBlICgpIHtcbiAgT2JqZWN0LmZyZWV6ZShPYmplY3QpXG4gIE9iamVjdC5mcmVlemUoQXJyYXkpXG5cbiAgLy8gT2JqZWN0LmZyZWV6ZShPYmplY3QucHJvdG90eXBlKVxuICBmcmVlemVPYmplY3RQcm90bygpXG4gIE9iamVjdC5mcmVlemUoQXJyYXkucHJvdG90eXBlKVxuICBPYmplY3QuZnJlZXplKFN0cmluZy5wcm90b3R5cGUpXG4gIE9iamVjdC5mcmVlemUoTnVtYmVyLnByb3RvdHlwZSlcbiAgT2JqZWN0LmZyZWV6ZShCb29sZWFuLnByb3RvdHlwZSlcblxuICAvLyBPYmplY3QuZnJlZXplKEVycm9yLnByb3RvdHlwZSlcbiAgZnJlZXplRXJyb3JQcm90bygpXG4gIE9iamVjdC5mcmVlemUoRGF0ZS5wcm90b3R5cGUpXG4gIE9iamVjdC5mcmVlemUoUmVnRXhwLnByb3RvdHlwZSlcbn1cblxuZnVuY3Rpb24gZnJlZXplT2JqZWN0UHJvdG8gKCkge1xuICBjb25zdCBwcm90byA9IE9iamVjdC5wcm90b3R5cGVcbiAgY29uc3QgcHJvdG9OYW1lID0gJ09iamVjdC5wcm90b3R5cGUnXG4gIGZyZWV6ZVByb3RvUHJvcGVydHkocHJvdG8sICdfX2RlZmluZUdldHRlcl9fJywgcHJvdG9OYW1lKVxuICBmcmVlemVQcm90b1Byb3BlcnR5KHByb3RvLCAnX19kZWZpbmVTZXR0ZXJfXycsIHByb3RvTmFtZSlcbiAgZnJlZXplUHJvdG9Qcm9wZXJ0eShwcm90bywgJ19fbG9va3VwR2V0dGVyX18nLCBwcm90b05hbWUpXG4gIGZyZWV6ZVByb3RvUHJvcGVydHkocHJvdG8sICdfX2xvb2t1cFNldHRlcl9fJywgcHJvdG9OYW1lKVxuICBmcmVlemVQcm90b1Byb3BlcnR5KHByb3RvLCAnY29uc3RydWN0b3InLCBwcm90b05hbWUpXG4gIGZyZWV6ZVByb3RvUHJvcGVydHkocHJvdG8sICdoYXNPd25Qcm9wZXJ0eScsIHByb3RvTmFtZSlcbiAgZnJlZXplUHJvdG9Qcm9wZXJ0eShwcm90bywgJ2lzUHJvdG90eXBlT2YnLCBwcm90b05hbWUpXG4gIGZyZWV6ZVByb3RvUHJvcGVydHkocHJvdG8sICdwcm9wZXJ0eUlzRW51bWVyYWJsZScsIHByb3RvTmFtZSlcbiAgZnJlZXplUHJvdG9Qcm9wZXJ0eShwcm90bywgJ3RvTG9jYWxlU3RyaW5nJywgcHJvdG9OYW1lKVxuICBmcmVlemVQcm90b1Byb3BlcnR5KHByb3RvLCAndG9TdHJpbmcnLCBwcm90b05hbWUpXG4gIGZyZWV6ZVByb3RvUHJvcGVydHkocHJvdG8sICd2YWx1ZU9mJywgcHJvdG9OYW1lKVxuICBPYmplY3Quc2VhbChwcm90bylcbn1cblxuZnVuY3Rpb24gZnJlZXplRXJyb3JQcm90byAoKSB7XG4gIGNvbnN0IHByb3RvID0gRXJyb3IucHJvdG90eXBlXG4gIGNvbnN0IHByb3RvTmFtZSA9ICdFcnJvci5wcm90b3R5cGUnXG4gIGZyZWV6ZVByb3RvUHJvcGVydHkocHJvdG8sICduYW1lJywgcHJvdG9OYW1lKVxuICBmcmVlemVQcm90b1Byb3BlcnR5KHByb3RvLCAnbWVzc2FnZScsIHByb3RvTmFtZSlcbiAgZnJlZXplUHJvdG9Qcm9wZXJ0eShwcm90bywgJ3RvU3RyaW5nJywgcHJvdG9OYW1lKVxuICBmcmVlemVQcm90b1Byb3BlcnR5KHByb3RvLCAnY29uc3RydWN0b3InLCBwcm90b05hbWUpXG4gIE9iamVjdC5zZWFsKHByb3RvKVxufVxuXG5mdW5jdGlvbiBmcmVlemVQcm90b1Byb3BlcnR5IChwcm90bywgcHJvcGVydHlOYW1lLCBwcm90b05hbWUpIHtcbiAgaWYgKCFwcm90by5oYXNPd25Qcm9wZXJ0eShwcm9wZXJ0eU5hbWUpKSB7XG4gICAgcmV0dXJuXG4gIH1cblxuICBjb25zdCBvcmlnaW4gPSBwcm90b1twcm9wZXJ0eU5hbWVdXG4gIE9iamVjdC5kZWZpbmVQcm9wZXJ0eShwcm90bywgcHJvcGVydHlOYW1lLCB7XG4gICAgZ2V0OiBmdW5jdGlvbiAoKSB7XG4gICAgICByZXR1cm4gb3JpZ2luXG4gICAgfSxcbiAgICBzZXQ6IGZ1bmN0aW9uICh2YWx1ZSkge1xuICAgICAgaWYgKHRoaXMgPT09IHByb3RvKSB7XG4gICAgICAgIHRocm93IEVycm9yKGBDYW5ub3QgYXNzaWduIHRvIHJlYWQgb25seSBwcm9wZXJ0eSAke3Byb3BlcnR5TmFtZX0gb2YgJHtwcm90b05hbWV9YClcbiAgICAgIH1cblxuICAgICAgT2JqZWN0LmRlZmluZVByb3BlcnR5KHRoaXMsIHByb3BlcnR5TmFtZSwge1xuICAgICAgICB2YWx1ZTogdmFsdWUsXG4gICAgICAgIHdyaXRhYmxlOiB0cnVlXG4gICAgICB9KVxuXG4gICAgICByZXR1cm4gdmFsdWVcbiAgICB9XG4gIH0pXG59XG4iLCIvKlxuICogTGljZW5zZWQgdG8gdGhlIEFwYWNoZSBTb2Z0d2FyZSBGb3VuZGF0aW9uIChBU0YpIHVuZGVyIG9uZVxuICogb3IgbW9yZSBjb250cmlidXRvciBsaWNlbnNlIGFncmVlbWVudHMuICBTZWUgdGhlIE5PVElDRSBmaWxlXG4gKiBkaXN0cmlidXRlZCB3aXRoIHRoaXMgd29yayBmb3IgYWRkaXRpb25hbCBpbmZvcm1hdGlvblxuICogcmVnYXJkaW5nIGNvcHlyaWdodCBvd25lcnNoaXAuICBUaGUgQVNGIGxpY2Vuc2VzIHRoaXMgZmlsZVxuICogdG8geW91IHVuZGVyIHRoZSBBcGFjaGUgTGljZW5zZSwgVmVyc2lvbiAyLjAgKHRoZVxuICogXCJMaWNlbnNlXCIpOyB5b3UgbWF5IG5vdCB1c2UgdGhpcyBmaWxlIGV4Y2VwdCBpbiBjb21wbGlhbmNlXG4gKiB3aXRoIHRoZSBMaWNlbnNlLiAgWW91IG1heSBvYnRhaW4gYSBjb3B5IG9mIHRoZSBMaWNlbnNlIGF0XG4gKlxuICogICBodHRwOi8vd3d3LmFwYWNoZS5vcmcvbGljZW5zZXMvTElDRU5TRS0yLjBcbiAqXG4gKiBVbmxlc3MgcmVxdWlyZWQgYnkgYXBwbGljYWJsZSBsYXcgb3IgYWdyZWVkIHRvIGluIHdyaXRpbmcsXG4gKiBzb2Z0d2FyZSBkaXN0cmlidXRlZCB1bmRlciB0aGUgTGljZW5zZSBpcyBkaXN0cmlidXRlZCBvbiBhblxuICogXCJBUyBJU1wiIEJBU0lTLCBXSVRIT1VUIFdBUlJBTlRJRVMgT1IgQ09ORElUSU9OUyBPRiBBTllcbiAqIEtJTkQsIGVpdGhlciBleHByZXNzIG9yIGltcGxpZWQuICBTZWUgdGhlIExpY2Vuc2UgZm9yIHRoZVxuICogc3BlY2lmaWMgbGFuZ3VhZ2UgZ292ZXJuaW5nIHBlcm1pc3Npb25zIGFuZCBsaW1pdGF0aW9uc1xuICogdW5kZXIgdGhlIExpY2Vuc2UuXG4gKi9cbmltcG9ydCAnLi9hcnJheUZyb20nXG5pbXBvcnQgJy4vb2JqZWN0QXNzaWduJ1xuaW1wb3J0ICcuL29iamVjdFNldFByb3RvdHlwZU9mJ1xuXG4vLyBpbXBvcnQgcHJvbWlzZSBoYWNrIGFuZCBwb2x5ZmlsbHNcbmltcG9ydCAnLi9wcm9taXNlJ1xuaW1wb3J0ICdjb3JlLWpzL21vZHVsZXMvZXM2Lm9iamVjdC50by1zdHJpbmcnXG5pbXBvcnQgJ2NvcmUtanMvbW9kdWxlcy9lczYuc3RyaW5nLml0ZXJhdG9yJ1xuaW1wb3J0ICdjb3JlLWpzL21vZHVsZXMvd2ViLmRvbS5pdGVyYWJsZSdcbmltcG9ydCAnY29yZS1qcy9tb2R1bGVzL2VzNi5wcm9taXNlJ1xuXG5leHBvcnQgKiBmcm9tICcuL2NvbnNvbGUnXG5leHBvcnQgKiBmcm9tICcuL3NldFRpbWVvdXQnXG5leHBvcnQgKiBmcm9tICcuL2ZyZWV6ZSdcbiIsIi8qXG4gKiBMaWNlbnNlZCB0byB0aGUgQXBhY2hlIFNvZnR3YXJlIEZvdW5kYXRpb24gKEFTRikgdW5kZXIgb25lXG4gKiBvciBtb3JlIGNvbnRyaWJ1dG9yIGxpY2Vuc2UgYWdyZWVtZW50cy4gIFNlZSB0aGUgTk9USUNFIGZpbGVcbiAqIGRpc3RyaWJ1dGVkIHdpdGggdGhpcyB3b3JrIGZvciBhZGRpdGlvbmFsIGluZm9ybWF0aW9uXG4gKiByZWdhcmRpbmcgY29weXJpZ2h0IG93bmVyc2hpcC4gIFRoZSBBU0YgbGljZW5zZXMgdGhpcyBmaWxlXG4gKiB0byB5b3UgdW5kZXIgdGhlIEFwYWNoZSBMaWNlbnNlLCBWZXJzaW9uIDIuMCAodGhlXG4gKiBcIkxpY2Vuc2VcIik7IHlvdSBtYXkgbm90IHVzZSB0aGlzIGZpbGUgZXhjZXB0IGluIGNvbXBsaWFuY2VcbiAqIHdpdGggdGhlIExpY2Vuc2UuICBZb3UgbWF5IG9idGFpbiBhIGNvcHkgb2YgdGhlIExpY2Vuc2UgYXRcbiAqXG4gKiAgIGh0dHA6Ly93d3cuYXBhY2hlLm9yZy9saWNlbnNlcy9MSUNFTlNFLTIuMFxuICpcbiAqIFVubGVzcyByZXF1aXJlZCBieSBhcHBsaWNhYmxlIGxhdyBvciBhZ3JlZWQgdG8gaW4gd3JpdGluZyxcbiAqIHNvZnR3YXJlIGRpc3RyaWJ1dGVkIHVuZGVyIHRoZSBMaWNlbnNlIGlzIGRpc3RyaWJ1dGVkIG9uIGFuXG4gKiBcIkFTIElTXCIgQkFTSVMsIFdJVEhPVVQgV0FSUkFOVElFUyBPUiBDT05ESVRJT05TIE9GIEFOWVxuICogS0lORCwgZWl0aGVyIGV4cHJlc3Mgb3IgaW1wbGllZC4gIFNlZSB0aGUgTGljZW5zZSBmb3IgdGhlXG4gKiBzcGVjaWZpYyBsYW5ndWFnZSBnb3Zlcm5pbmcgcGVybWlzc2lvbnMgYW5kIGxpbWl0YXRpb25zXG4gKiB1bmRlciB0aGUgTGljZW5zZS5cbiAqL1xuXG4vKipcbiAqIEZvciBnZW5lcmFsIGNhbGxiYWNrIG1hbmFnZW1lbnQgb2YgYSBjZXJ0YWluIFdlZXggaW5zdGFuY2UuXG4gKiBCZWNhdXNlIGZ1bmN0aW9uIGNhbiBub3QgcGFzc2VkIGludG8gbmF0aXZlLCBzbyB3ZSBjcmVhdGUgY2FsbGJhY2tcbiAqIGNhbGxiYWNrIGlkIGZvciBlYWNoIGZ1bmN0aW9uIGFuZCBwYXNzIHRoZSBjYWxsYmFjayBpZCBpbnRvIG5hdGl2ZVxuICogaW4gZmFjdC4gQW5kIHdoZW4gYSBjYWxsYmFjayBjYWxsZWQgZnJvbSBuYXRpdmUsIHdlIGNhbiBmaW5kIHRoZSByZWFsXG4gKiBjYWxsYmFjayB0aHJvdWdoIHRoZSBjYWxsYmFjayBpZCB3ZSBoYXZlIHBhc3NlZCBiZWZvcmUuXG4gKi9cbmV4cG9ydCBkZWZhdWx0IGNsYXNzIENhbGxiYWNrTWFuYWdlciB7XG4gIGNvbnN0cnVjdG9yIChpbnN0YW5jZUlkKSB7XG4gICAgdGhpcy5pbnN0YW5jZUlkID0gaW5zdGFuY2VJZFxuICAgIHRoaXMubGFzdENhbGxiYWNrSWQgPSAwXG4gICAgdGhpcy5jYWxsYmFja3MgPSBbXVxuICB9XG4gIGFkZCAoY2FsbGJhY2spIHtcbiAgICB0aGlzLmxhc3RDYWxsYmFja0lkKytcbiAgICB0aGlzLmNhbGxiYWNrc1t0aGlzLmxhc3RDYWxsYmFja0lkXSA9IGNhbGxiYWNrXG4gICAgcmV0dXJuIHRoaXMubGFzdENhbGxiYWNrSWRcbiAgfVxuICByZW1vdmUgKGNhbGxiYWNrSWQpIHtcbiAgICBjb25zdCBjYWxsYmFjayA9IHRoaXMuY2FsbGJhY2tzW2NhbGxiYWNrSWRdXG4gICAgdGhpcy5jYWxsYmFja3NbY2FsbGJhY2tJZF0gPSB1bmRlZmluZWRcbiAgICByZXR1cm4gY2FsbGJhY2tcbiAgfVxuICBjb25zdW1lIChjYWxsYmFja0lkLCBkYXRhLCBpZktlZXBBbGl2ZSkge1xuICAgIGNvbnN0IGNhbGxiYWNrID0gdGhpcy5jYWxsYmFja3NbY2FsbGJhY2tJZF1cbiAgICBpZiAodHlwZW9mIGlmS2VlcEFsaXZlID09PSAndW5kZWZpbmVkJyB8fCBpZktlZXBBbGl2ZSA9PT0gZmFsc2UpIHtcbiAgICAgIHRoaXMuY2FsbGJhY2tzW2NhbGxiYWNrSWRdID0gdW5kZWZpbmVkXG4gICAgfVxuICAgIGlmICh0eXBlb2YgY2FsbGJhY2sgPT09ICdmdW5jdGlvbicpIHtcbiAgICAgIHJldHVybiBjYWxsYmFjayhkYXRhKVxuICAgIH1cbiAgICByZXR1cm4gbmV3IEVycm9yKGBpbnZhbGlkIGNhbGxiYWNrIGlkIFwiJHtjYWxsYmFja0lkfVwiYClcbiAgfVxuICBjbG9zZSAoKSB7XG4gICAgdGhpcy5jYWxsYmFja3MgPSB0aGlzLmNhbGxiYWNrcy5tYXAoY2IgPT4gdW5kZWZpbmVkKVxuICB9XG59XG4iLCIvKlxuICogTGljZW5zZWQgdG8gdGhlIEFwYWNoZSBTb2Z0d2FyZSBGb3VuZGF0aW9uIChBU0YpIHVuZGVyIG9uZVxuICogb3IgbW9yZSBjb250cmlidXRvciBsaWNlbnNlIGFncmVlbWVudHMuICBTZWUgdGhlIE5PVElDRSBmaWxlXG4gKiBkaXN0cmlidXRlZCB3aXRoIHRoaXMgd29yayBmb3IgYWRkaXRpb25hbCBpbmZvcm1hdGlvblxuICogcmVnYXJkaW5nIGNvcHlyaWdodCBvd25lcnNoaXAuICBUaGUgQVNGIGxpY2Vuc2VzIHRoaXMgZmlsZVxuICogdG8geW91IHVuZGVyIHRoZSBBcGFjaGUgTGljZW5zZSwgVmVyc2lvbiAyLjAgKHRoZVxuICogXCJMaWNlbnNlXCIpOyB5b3UgbWF5IG5vdCB1c2UgdGhpcyBmaWxlIGV4Y2VwdCBpbiBjb21wbGlhbmNlXG4gKiB3aXRoIHRoZSBMaWNlbnNlLiAgWW91IG1heSBvYnRhaW4gYSBjb3B5IG9mIHRoZSBMaWNlbnNlIGF0XG4gKlxuICogICBodHRwOi8vd3d3LmFwYWNoZS5vcmcvbGljZW5zZXMvTElDRU5TRS0yLjBcbiAqXG4gKiBVbmxlc3MgcmVxdWlyZWQgYnkgYXBwbGljYWJsZSBsYXcgb3IgYWdyZWVkIHRvIGluIHdyaXRpbmcsXG4gKiBzb2Z0d2FyZSBkaXN0cmlidXRlZCB1bmRlciB0aGUgTGljZW5zZSBpcyBkaXN0cmlidXRlZCBvbiBhblxuICogXCJBUyBJU1wiIEJBU0lTLCBXSVRIT1VUIFdBUlJBTlRJRVMgT1IgQ09ORElUSU9OUyBPRiBBTllcbiAqIEtJTkQsIGVpdGhlciBleHByZXNzIG9yIGltcGxpZWQuICBTZWUgdGhlIExpY2Vuc2UgZm9yIHRoZVxuICogc3BlY2lmaWMgbGFuZ3VhZ2UgZ292ZXJuaW5nIHBlcm1pc3Npb25zIGFuZCBsaW1pdGF0aW9uc1xuICogdW5kZXIgdGhlIExpY2Vuc2UuXG4gKi9cblxuY29uc3QgZG9jTWFwID0ge31cblxuLyoqXG4gKiBBZGQgYSBkb2N1bWVudCBvYmplY3QgaW50byBkb2NNYXAuXG4gKiBAcGFyYW0ge3N0cmluZ30gaWRcbiAqIEBwYXJhbSB7b2JqZWN0fSBkb2N1bWVudFxuICovXG5leHBvcnQgZnVuY3Rpb24gYWRkRG9jIChpZCwgZG9jKSB7XG4gIGlmIChpZCkge1xuICAgIGRvY01hcFtpZF0gPSBkb2NcbiAgfVxufVxuXG4vKipcbiAqIEdldCB0aGUgZG9jdW1lbnQgb2JqZWN0IGJ5IGlkLlxuICogQHBhcmFtIHtzdHJpbmd9IGlkXG4gKi9cbmV4cG9ydCBmdW5jdGlvbiBnZXREb2MgKGlkKSB7XG4gIHJldHVybiBkb2NNYXBbaWRdXG59XG5cbi8qKlxuICogUmVtb3ZlIHRoZSBkb2N1bWVudCBmcm9tIGRvY01hcCBieSBpZC5cbiAqIEBwYXJhbSB7c3RyaW5nfSBpZFxuICovXG5leHBvcnQgZnVuY3Rpb24gcmVtb3ZlRG9jIChpZCkge1xuICBkZWxldGUgZG9jTWFwW2lkXVxufVxuXG4vKipcbiAqIEBkZXByZWNhdGVkXG4gKiBHZXQgbGlzdGVuZXIgYnkgZG9jdW1lbnQgaWQuXG4gKiBAcGFyYW0ge3N0cmluZ30gaWRcbiAqIEByZXR1cm4ge29iamVjdH0gbGlzdGVuZXJcbiAqL1xuZXhwb3J0IGZ1bmN0aW9uIGdldExpc3RlbmVyIChpZCkge1xuICBjb25zdCBkb2MgPSBkb2NNYXBbaWRdXG4gIGlmIChkb2MgJiYgZG9jLmxpc3RlbmVyKSB7XG4gICAgcmV0dXJuIGRvYy5saXN0ZW5lclxuICB9XG4gIHJldHVybiBudWxsXG59XG5cbi8qKlxuICogR2V0IFRhc2tDZW50ZXIgaW5zdGFuY2UgYnkgaWQuXG4gKiBAcGFyYW0ge3N0cmluZ30gaWRcbiAqIEByZXR1cm4ge29iamVjdH0gVGFza0NlbnRlclxuICovXG5leHBvcnQgZnVuY3Rpb24gZ2V0VGFza0NlbnRlciAoaWQpIHtcbiAgY29uc3QgZG9jID0gZG9jTWFwW2lkXVxuICBpZiAoZG9jICYmIGRvYy50YXNrQ2VudGVyKSB7XG4gICAgcmV0dXJuIGRvYy50YXNrQ2VudGVyXG4gIH1cbiAgcmV0dXJuIG51bGxcbn1cblxuLyoqXG4gKiBHZXQgYSB1bmlxdWUgaWQuXG4gKi9cbmxldCBuZXh0Tm9kZVJlZiA9IDFcbmV4cG9ydCBmdW5jdGlvbiB1bmlxdWVJZCAoKSB7XG4gIHJldHVybiAobmV4dE5vZGVSZWYrKykudG9TdHJpbmcoKVxufVxuXG4vKipcbiAqIEFwcGVuZCBib2R5IG5vZGUgdG8gZG9jdW1lbnRFbGVtZW50LlxuICogQHBhcmFtIHtvYmplY3R9IGRvY3VtZW50XG4gKiBAcGFyYW0ge29iamVjdH0gbm9kZVxuICogQHBhcmFtIHtvYmplY3R9IGJlZm9yZVxuICovXG5leHBvcnQgZnVuY3Rpb24gYXBwZW5kQm9keSAoZG9jLCBub2RlLCBiZWZvcmUpIHtcbiAgY29uc3QgeyBkb2N1bWVudEVsZW1lbnQgfSA9IGRvY1xuXG4gIGlmIChkb2N1bWVudEVsZW1lbnQucHVyZUNoaWxkcmVuLmxlbmd0aCA+IDAgfHwgbm9kZS5wYXJlbnROb2RlKSB7XG4gICAgcmV0dXJuXG4gIH1cbiAgY29uc3QgY2hpbGRyZW4gPSBkb2N1bWVudEVsZW1lbnQuY2hpbGRyZW5cbiAgY29uc3QgYmVmb3JlSW5kZXggPSBjaGlsZHJlbi5pbmRleE9mKGJlZm9yZSlcbiAgaWYgKGJlZm9yZUluZGV4IDwgMCkge1xuICAgIGNoaWxkcmVuLnB1c2gobm9kZSlcbiAgfVxuICBlbHNlIHtcbiAgICBjaGlsZHJlbi5zcGxpY2UoYmVmb3JlSW5kZXgsIDAsIG5vZGUpXG4gIH1cblxuICBpZiAobm9kZS5ub2RlVHlwZSA9PT0gMSkge1xuICAgIGlmIChub2RlLnJvbGUgPT09ICdib2R5Jykge1xuICAgICAgbm9kZS5kb2NJZCA9IGRvYy5pZFxuICAgICAgbm9kZS5vd25lckRvY3VtZW50ID0gZG9jXG4gICAgICBub2RlLnBhcmVudE5vZGUgPSBkb2N1bWVudEVsZW1lbnRcbiAgICAgIGxpbmtQYXJlbnQobm9kZSwgZG9jdW1lbnRFbGVtZW50KVxuICAgIH1cbiAgICBlbHNlIHtcbiAgICAgIG5vZGUuY2hpbGRyZW4uZm9yRWFjaChjaGlsZCA9PiB7XG4gICAgICAgIGNoaWxkLnBhcmVudE5vZGUgPSBub2RlXG4gICAgICB9KVxuICAgICAgc2V0Qm9keShkb2MsIG5vZGUpXG4gICAgICBub2RlLmRvY0lkID0gZG9jLmlkXG4gICAgICBub2RlLm93bmVyRG9jdW1lbnQgPSBkb2NcbiAgICAgIGxpbmtQYXJlbnQobm9kZSwgZG9jdW1lbnRFbGVtZW50KVxuICAgICAgZGVsZXRlIGRvYy5ub2RlTWFwW25vZGUubm9kZUlkXVxuICAgIH1cbiAgICBkb2N1bWVudEVsZW1lbnQucHVyZUNoaWxkcmVuLnB1c2gobm9kZSlcbiAgICBzZW5kQm9keShkb2MsIG5vZGUpXG4gIH1cbiAgZWxzZSB7XG4gICAgbm9kZS5wYXJlbnROb2RlID0gZG9jdW1lbnRFbGVtZW50XG4gICAgZG9jLm5vZGVNYXBbbm9kZS5yZWZdID0gbm9kZVxuICB9XG59XG5cbmZ1bmN0aW9uIHNlbmRCb2R5IChkb2MsIG5vZGUpIHtcbiAgY29uc3QgYm9keSA9IG5vZGUudG9KU09OKClcbiAgY29uc3QgY2hpbGRyZW4gPSBib2R5LmNoaWxkcmVuXG4gIGRlbGV0ZSBib2R5LmNoaWxkcmVuXG4gIGxldCByZXN1bHQgPSBkb2MudGFza0NlbnRlci5zZW5kKCdkb20nLCB7IGFjdGlvbjogJ2NyZWF0ZUJvZHknIH0sIFtib2R5XSlcbiAgaWYgKGNoaWxkcmVuKSB7XG4gICAgY2hpbGRyZW4uZm9yRWFjaChjaGlsZCA9PiB7XG4gICAgICByZXN1bHQgPSBkb2MudGFza0NlbnRlci5zZW5kKCdkb20nLCB7IGFjdGlvbjogJ2FkZEVsZW1lbnQnIH0sIFtib2R5LnJlZiwgY2hpbGQsIC0xXSlcbiAgICB9KVxuICB9XG4gIHJldHVybiByZXN1bHRcbn1cblxuLyoqXG4gKiBTZXQgdXAgYm9keSBub2RlLlxuICogQHBhcmFtIHtvYmplY3R9IGRvY3VtZW50XG4gKiBAcGFyYW0ge29iamVjdH0gZWxlbWVudFxuICovXG5leHBvcnQgZnVuY3Rpb24gc2V0Qm9keSAoZG9jLCBlbCkge1xuICBlbC5yb2xlID0gJ2JvZHknXG4gIGVsLmRlcHRoID0gMVxuICBkZWxldGUgZG9jLm5vZGVNYXBbZWwubm9kZUlkXVxuICBlbC5yZWYgPSAnX3Jvb3QnXG4gIGRvYy5ub2RlTWFwLl9yb290ID0gZWxcbiAgZG9jLmJvZHkgPSBlbFxufVxuXG4vKipcbiAqIEVzdGFibGlzaCB0aGUgY29ubmVjdGlvbiBiZXR3ZWVuIHBhcmVudCBhbmQgY2hpbGQgbm9kZS5cbiAqIEBwYXJhbSB7b2JqZWN0fSBjaGlsZCBub2RlXG4gKiBAcGFyYW0ge29iamVjdH0gcGFyZW50IG5vZGVcbiAqL1xuZXhwb3J0IGZ1bmN0aW9uIGxpbmtQYXJlbnQgKG5vZGUsIHBhcmVudCkge1xuICBub2RlLnBhcmVudE5vZGUgPSBwYXJlbnRcbiAgaWYgKHBhcmVudC5kb2NJZCkge1xuICAgIG5vZGUuZG9jSWQgPSBwYXJlbnQuZG9jSWRcbiAgICBub2RlLm93bmVyRG9jdW1lbnQgPSBwYXJlbnQub3duZXJEb2N1bWVudFxuICAgIG5vZGUub3duZXJEb2N1bWVudC5ub2RlTWFwW25vZGUubm9kZUlkXSA9IG5vZGVcbiAgICBub2RlLmRlcHRoID0gcGFyZW50LmRlcHRoICsgMVxuICB9XG4gIG5vZGUuY2hpbGRyZW4uZm9yRWFjaChjaGlsZCA9PiB7XG4gICAgbGlua1BhcmVudChjaGlsZCwgbm9kZSlcbiAgfSlcbn1cblxuLyoqXG4gKiBHZXQgdGhlIG5leHQgc2libGluZyBlbGVtZW50LlxuICogQHBhcmFtIHtvYmplY3R9IG5vZGVcbiAqL1xuZXhwb3J0IGZ1bmN0aW9uIG5leHRFbGVtZW50IChub2RlKSB7XG4gIHdoaWxlIChub2RlKSB7XG4gICAgaWYgKG5vZGUubm9kZVR5cGUgPT09IDEpIHtcbiAgICAgIHJldHVybiBub2RlXG4gICAgfVxuICAgIG5vZGUgPSBub2RlLm5leHRTaWJsaW5nXG4gIH1cbn1cblxuLyoqXG4gKiBHZXQgdGhlIHByZXZpb3VzIHNpYmxpbmcgZWxlbWVudC5cbiAqIEBwYXJhbSB7b2JqZWN0fSBub2RlXG4gKi9cbmV4cG9ydCBmdW5jdGlvbiBwcmV2aW91c0VsZW1lbnQgKG5vZGUpIHtcbiAgd2hpbGUgKG5vZGUpIHtcbiAgICBpZiAobm9kZS5ub2RlVHlwZSA9PT0gMSkge1xuICAgICAgcmV0dXJuIG5vZGVcbiAgICB9XG4gICAgbm9kZSA9IG5vZGUucHJldmlvdXNTaWJsaW5nXG4gIH1cbn1cblxuLyoqXG4gKiBJbnNlcnQgYSBub2RlIGludG8gbGlzdCBhdCB0aGUgc3BlY2lmaWVkIGluZGV4LlxuICogQHBhcmFtIHtvYmplY3R9IHRhcmdldCBub2RlXG4gKiBAcGFyYW0ge2FycmF5fSBsaXN0XG4gKiBAcGFyYW0ge251bWJlcn0gbmV3SW5kZXhcbiAqIEBwYXJhbSB7Ym9vbGVhbn0gY2hhbmdlU2libGluZ1xuICogQHJldHVybiB7bnVtYmVyfSBuZXdJbmRleFxuICovXG5leHBvcnQgZnVuY3Rpb24gaW5zZXJ0SW5kZXggKHRhcmdldCwgbGlzdCwgbmV3SW5kZXgsIGNoYW5nZVNpYmxpbmcpIHtcbiAgLyogaXN0YW5idWwgaWdub3JlIG5leHQgKi9cbiAgaWYgKG5ld0luZGV4IDwgMCkge1xuICAgIG5ld0luZGV4ID0gMFxuICB9XG4gIGNvbnN0IGJlZm9yZSA9IGxpc3RbbmV3SW5kZXggLSAxXVxuICBjb25zdCBhZnRlciA9IGxpc3RbbmV3SW5kZXhdXG4gIGxpc3Quc3BsaWNlKG5ld0luZGV4LCAwLCB0YXJnZXQpXG4gIGlmIChjaGFuZ2VTaWJsaW5nKSB7XG4gICAgYmVmb3JlICYmIChiZWZvcmUubmV4dFNpYmxpbmcgPSB0YXJnZXQpXG4gICAgdGFyZ2V0LnByZXZpb3VzU2libGluZyA9IGJlZm9yZVxuICAgIHRhcmdldC5uZXh0U2libGluZyA9IGFmdGVyXG4gICAgYWZ0ZXIgJiYgKGFmdGVyLnByZXZpb3VzU2libGluZyA9IHRhcmdldClcbiAgfVxuICByZXR1cm4gbmV3SW5kZXhcbn1cblxuLyoqXG4gKiBNb3ZlIHRoZSBub2RlIHRvIGEgbmV3IGluZGV4IGluIGxpc3QuXG4gKiBAcGFyYW0ge29iamVjdH0gdGFyZ2V0IG5vZGVcbiAqIEBwYXJhbSB7YXJyYXl9IGxpc3RcbiAqIEBwYXJhbSB7bnVtYmVyfSBuZXdJbmRleFxuICogQHBhcmFtIHtib29sZWFufSBjaGFuZ2VTaWJsaW5nXG4gKiBAcmV0dXJuIHtudW1iZXJ9IG5ld0luZGV4XG4gKi9cbmV4cG9ydCBmdW5jdGlvbiBtb3ZlSW5kZXggKHRhcmdldCwgbGlzdCwgbmV3SW5kZXgsIGNoYW5nZVNpYmxpbmcpIHtcbiAgY29uc3QgaW5kZXggPSBsaXN0LmluZGV4T2YodGFyZ2V0KVxuICAvKiBpc3RhbmJ1bCBpZ25vcmUgbmV4dCAqL1xuICBpZiAoaW5kZXggPCAwKSB7XG4gICAgcmV0dXJuIC0xXG4gIH1cbiAgaWYgKGNoYW5nZVNpYmxpbmcpIHtcbiAgICBjb25zdCBiZWZvcmUgPSBsaXN0W2luZGV4IC0gMV1cbiAgICBjb25zdCBhZnRlciA9IGxpc3RbaW5kZXggKyAxXVxuICAgIGJlZm9yZSAmJiAoYmVmb3JlLm5leHRTaWJsaW5nID0gYWZ0ZXIpXG4gICAgYWZ0ZXIgJiYgKGFmdGVyLnByZXZpb3VzU2libGluZyA9IGJlZm9yZSlcbiAgfVxuICBsaXN0LnNwbGljZShpbmRleCwgMSlcbiAgbGV0IG5ld0luZGV4QWZ0ZXIgPSBuZXdJbmRleFxuICBpZiAoaW5kZXggPD0gbmV3SW5kZXgpIHtcbiAgICBuZXdJbmRleEFmdGVyID0gbmV3SW5kZXggLSAxXG4gIH1cbiAgY29uc3QgYmVmb3JlTmV3ID0gbGlzdFtuZXdJbmRleEFmdGVyIC0gMV1cbiAgY29uc3QgYWZ0ZXJOZXcgPSBsaXN0W25ld0luZGV4QWZ0ZXJdXG4gIGxpc3Quc3BsaWNlKG5ld0luZGV4QWZ0ZXIsIDAsIHRhcmdldClcbiAgaWYgKGNoYW5nZVNpYmxpbmcpIHtcbiAgICBiZWZvcmVOZXcgJiYgKGJlZm9yZU5ldy5uZXh0U2libGluZyA9IHRhcmdldClcbiAgICB0YXJnZXQucHJldmlvdXNTaWJsaW5nID0gYmVmb3JlTmV3XG4gICAgdGFyZ2V0Lm5leHRTaWJsaW5nID0gYWZ0ZXJOZXdcbiAgICBhZnRlck5ldyAmJiAoYWZ0ZXJOZXcucHJldmlvdXNTaWJsaW5nID0gdGFyZ2V0KVxuICB9XG4gIGlmIChpbmRleCA9PT0gbmV3SW5kZXhBZnRlcikge1xuICAgIHJldHVybiAtMVxuICB9XG4gIHJldHVybiBuZXdJbmRleFxufVxuXG4vKipcbiAqIFJlbW92ZSB0aGUgbm9kZSBmcm9tIGxpc3QuXG4gKiBAcGFyYW0ge29iamVjdH0gdGFyZ2V0IG5vZGVcbiAqIEBwYXJhbSB7YXJyYXl9IGxpc3RcbiAqIEBwYXJhbSB7Ym9vbGVhbn0gY2hhbmdlU2libGluZ1xuICovXG5leHBvcnQgZnVuY3Rpb24gcmVtb3ZlSW5kZXggKHRhcmdldCwgbGlzdCwgY2hhbmdlU2libGluZykge1xuICBjb25zdCBpbmRleCA9IGxpc3QuaW5kZXhPZih0YXJnZXQpXG4gIC8qIGlzdGFuYnVsIGlnbm9yZSBuZXh0ICovXG4gIGlmIChpbmRleCA8IDApIHtcbiAgICByZXR1cm5cbiAgfVxuICBpZiAoY2hhbmdlU2libGluZykge1xuICAgIGNvbnN0IGJlZm9yZSA9IGxpc3RbaW5kZXggLSAxXVxuICAgIGNvbnN0IGFmdGVyID0gbGlzdFtpbmRleCArIDFdXG4gICAgYmVmb3JlICYmIChiZWZvcmUubmV4dFNpYmxpbmcgPSBhZnRlcilcbiAgICBhZnRlciAmJiAoYWZ0ZXIucHJldmlvdXNTaWJsaW5nID0gYmVmb3JlKVxuICB9XG4gIGxpc3Quc3BsaWNlKGluZGV4LCAxKVxufVxuIiwiLypcbiAqIExpY2Vuc2VkIHRvIHRoZSBBcGFjaGUgU29mdHdhcmUgRm91bmRhdGlvbiAoQVNGKSB1bmRlciBvbmVcbiAqIG9yIG1vcmUgY29udHJpYnV0b3IgbGljZW5zZSBhZ3JlZW1lbnRzLiAgU2VlIHRoZSBOT1RJQ0UgZmlsZVxuICogZGlzdHJpYnV0ZWQgd2l0aCB0aGlzIHdvcmsgZm9yIGFkZGl0aW9uYWwgaW5mb3JtYXRpb25cbiAqIHJlZ2FyZGluZyBjb3B5cmlnaHQgb3duZXJzaGlwLiAgVGhlIEFTRiBsaWNlbnNlcyB0aGlzIGZpbGVcbiAqIHRvIHlvdSB1bmRlciB0aGUgQXBhY2hlIExpY2Vuc2UsIFZlcnNpb24gMi4wICh0aGVcbiAqIFwiTGljZW5zZVwiKTsgeW91IG1heSBub3QgdXNlIHRoaXMgZmlsZSBleGNlcHQgaW4gY29tcGxpYW5jZVxuICogd2l0aCB0aGUgTGljZW5zZS4gIFlvdSBtYXkgb2J0YWluIGEgY29weSBvZiB0aGUgTGljZW5zZSBhdFxuICpcbiAqICAgaHR0cDovL3d3dy5hcGFjaGUub3JnL2xpY2Vuc2VzL0xJQ0VOU0UtMi4wXG4gKlxuICogVW5sZXNzIHJlcXVpcmVkIGJ5IGFwcGxpY2FibGUgbGF3IG9yIGFncmVlZCB0byBpbiB3cml0aW5nLFxuICogc29mdHdhcmUgZGlzdHJpYnV0ZWQgdW5kZXIgdGhlIExpY2Vuc2UgaXMgZGlzdHJpYnV0ZWQgb24gYW5cbiAqIFwiQVMgSVNcIiBCQVNJUywgV0lUSE9VVCBXQVJSQU5USUVTIE9SIENPTkRJVElPTlMgT0YgQU5ZXG4gKiBLSU5ELCBlaXRoZXIgZXhwcmVzcyBvciBpbXBsaWVkLiAgU2VlIHRoZSBMaWNlbnNlIGZvciB0aGVcbiAqIHNwZWNpZmljIGxhbmd1YWdlIGdvdmVybmluZyBwZXJtaXNzaW9ucyBhbmQgbGltaXRhdGlvbnNcbiAqIHVuZGVyIHRoZSBMaWNlbnNlLlxuICovXG5cbi8qKlxuICogQGZpbGVPdmVydmlld1xuICogVmlydHVhbC1ET00gTm9kZS4gSXQncyB0aGUgc3VwcGVyIGNsYXNzIG9mIEVsZW1lbnQgYW5kIENvbW1lbnQuXG4gKi9cblxuaW1wb3J0IHsgZ2V0RG9jLCB1bmlxdWVJZCB9IGZyb20gJy4vb3BlcmF0aW9uJ1xuXG5leHBvcnQgZGVmYXVsdCBmdW5jdGlvbiBOb2RlICgpIHtcbiAgdGhpcy5ub2RlSWQgPSB1bmlxdWVJZCgpXG4gIHRoaXMucmVmID0gdGhpcy5ub2RlSWRcbiAgdGhpcy5jaGlsZHJlbiA9IFtdXG4gIHRoaXMucHVyZUNoaWxkcmVuID0gW11cbiAgdGhpcy5wYXJlbnROb2RlID0gbnVsbFxuICB0aGlzLm5leHRTaWJsaW5nID0gbnVsbFxuICB0aGlzLnByZXZpb3VzU2libGluZyA9IG51bGxcbn1cblxuLyoqXG4gKiBEZXN0cm95IGN1cnJlbnQgbm9kZSwgYW5kIHJlbW92ZSBpdHNlbGYgZm9ybSBub2RlTWFwLlxuICovXG5Ob2RlLnByb3RvdHlwZS5kZXN0cm95ID0gZnVuY3Rpb24gKCkge1xuICBjb25zdCBkb2MgPSBnZXREb2ModGhpcy5kb2NJZClcbiAgaWYgKGRvYykge1xuICAgIGRlbGV0ZSB0aGlzLmRvY0lkXG4gICAgZGVsZXRlIGRvYy5ub2RlTWFwW3RoaXMubm9kZUlkXVxuICB9XG4gIHRoaXMuY2hpbGRyZW4uZm9yRWFjaChjaGlsZCA9PiB7XG4gICAgY2hpbGQuZGVzdHJveSgpXG4gIH0pXG59XG4iLCIvKlxuICogTGljZW5zZWQgdG8gdGhlIEFwYWNoZSBTb2Z0d2FyZSBGb3VuZGF0aW9uIChBU0YpIHVuZGVyIG9uZVxuICogb3IgbW9yZSBjb250cmlidXRvciBsaWNlbnNlIGFncmVlbWVudHMuICBTZWUgdGhlIE5PVElDRSBmaWxlXG4gKiBkaXN0cmlidXRlZCB3aXRoIHRoaXMgd29yayBmb3IgYWRkaXRpb25hbCBpbmZvcm1hdGlvblxuICogcmVnYXJkaW5nIGNvcHlyaWdodCBvd25lcnNoaXAuICBUaGUgQVNGIGxpY2Vuc2VzIHRoaXMgZmlsZVxuICogdG8geW91IHVuZGVyIHRoZSBBcGFjaGUgTGljZW5zZSwgVmVyc2lvbiAyLjAgKHRoZVxuICogXCJMaWNlbnNlXCIpOyB5b3UgbWF5IG5vdCB1c2UgdGhpcyBmaWxlIGV4Y2VwdCBpbiBjb21wbGlhbmNlXG4gKiB3aXRoIHRoZSBMaWNlbnNlLiAgWW91IG1heSBvYnRhaW4gYSBjb3B5IG9mIHRoZSBMaWNlbnNlIGF0XG4gKlxuICogICBodHRwOi8vd3d3LmFwYWNoZS5vcmcvbGljZW5zZXMvTElDRU5TRS0yLjBcbiAqXG4gKiBVbmxlc3MgcmVxdWlyZWQgYnkgYXBwbGljYWJsZSBsYXcgb3IgYWdyZWVkIHRvIGluIHdyaXRpbmcsXG4gKiBzb2Z0d2FyZSBkaXN0cmlidXRlZCB1bmRlciB0aGUgTGljZW5zZSBpcyBkaXN0cmlidXRlZCBvbiBhblxuICogXCJBUyBJU1wiIEJBU0lTLCBXSVRIT1VUIFdBUlJBTlRJRVMgT1IgQ09ORElUSU9OUyBPRiBBTllcbiAqIEtJTkQsIGVpdGhlciBleHByZXNzIG9yIGltcGxpZWQuICBTZWUgdGhlIExpY2Vuc2UgZm9yIHRoZVxuICogc3BlY2lmaWMgbGFuZ3VhZ2UgZ292ZXJuaW5nIHBlcm1pc3Npb25zIGFuZCBsaW1pdGF0aW9uc1xuICogdW5kZXIgdGhlIExpY2Vuc2UuXG4gKi9cbmltcG9ydCB7IGdldFRhc2tDZW50ZXIgfSBmcm9tICcuL29wZXJhdGlvbidcblxubGV0IEVsZW1lbnRcblxuZXhwb3J0IGZ1bmN0aW9uIHNldEVsZW1lbnQgKEVsKSB7XG4gIEVsZW1lbnQgPSBFbFxufVxuXG4vKipcbiAqIEEgbWFwIHdoaWNoIHN0b3JlcyBhbGwgdHlwZSBvZiBlbGVtZW50cy5cbiAqIEB0eXBlIHtPYmplY3R9XG4gKi9cbmV4cG9ydCBjb25zdCBlbGVtZW50VHlwZXMgPSB7fVxuXG4vKipcbiAqIFJlZ2lzdGVyIGFuIGV4dGVuZGVkIGVsZW1lbnQgdHlwZSB3aXRoIGNvbXBvbmVudCBtZXRob2RzLlxuICogQHBhcmFtICB7c3RyaW5nfSB0eXBlICAgIGNvbXBvbmVudCB0eXBlXG4gKiBAcGFyYW0gIHthcnJheX0gIG1ldGhvZHMgYSBsaXN0IG9mIG1ldGhvZCBuYW1lc1xuICovXG5leHBvcnQgZnVuY3Rpb24gcmVnaXN0ZXJFbGVtZW50ICh0eXBlLCBtZXRob2RzKSB7XG4gIC8vIFNraXAgd2hlbiBubyBzcGVjaWFsIGNvbXBvbmVudCBtZXRob2RzLlxuICBpZiAoIW1ldGhvZHMgfHwgIW1ldGhvZHMubGVuZ3RoKSB7XG4gICAgcmV0dXJuXG4gIH1cblxuICAvLyBJbml0IGNvbnN0cnVjdG9yLlxuICBjb25zdCBYRWxlbWVudCA9IGZ1bmN0aW9uIChwcm9wcykge1xuICAgIEVsZW1lbnQuY2FsbCh0aGlzLCB0eXBlLCBwcm9wcywgdHJ1ZSlcbiAgfVxuXG4gIC8vIEluaXQgcHJvdG90eXBlLlxuICBYRWxlbWVudC5wcm90b3R5cGUgPSBPYmplY3QuY3JlYXRlKEVsZW1lbnQucHJvdG90eXBlKVxuICBPYmplY3QuZGVmaW5lUHJvcGVydHkoWEVsZW1lbnQucHJvdG90eXBlLCAnY29uc3RydWN0b3InLCB7XG4gICAgY29uZmlndXJhYmxlOiBmYWxzZSxcbiAgICBlbnVtZXJhYmxlOiBmYWxzZSxcbiAgICB3cml0YWJsZTogZmFsc2UsXG4gICAgdmFsdWU6IEVsZW1lbnRcbiAgfSlcblxuICAvLyBBZGQgbWV0aG9kcyB0byBwcm90b3R5cGUuXG4gIG1ldGhvZHMuZm9yRWFjaChtZXRob2ROYW1lID0+IHtcbiAgICBYRWxlbWVudC5wcm90b3R5cGVbbWV0aG9kTmFtZV0gPSBmdW5jdGlvbiAoLi4uYXJncykge1xuICAgICAgY29uc3QgdGFza0NlbnRlciA9IGdldFRhc2tDZW50ZXIodGhpcy5kb2NJZClcbiAgICAgIGlmICh0YXNrQ2VudGVyKSB7XG4gICAgICAgIHJldHVybiB0YXNrQ2VudGVyLnNlbmQoJ2NvbXBvbmVudCcsIHtcbiAgICAgICAgICByZWY6IHRoaXMucmVmLFxuICAgICAgICAgIGNvbXBvbmVudDogdHlwZSxcbiAgICAgICAgICBtZXRob2Q6IG1ldGhvZE5hbWVcbiAgICAgICAgfSwgYXJncylcbiAgICAgIH1cbiAgICB9XG4gIH0pXG5cbiAgLy8gQWRkIHRvIGVsZW1lbnQgdHlwZSBtYXAuXG4gIGVsZW1lbnRUeXBlc1t0eXBlXSA9IFhFbGVtZW50XG59XG5cbi8qKlxuICogQ2xlYXIgYWxsIGVsZW1lbnQgdHlwZXMuIE9ubHkgZm9yIHRlc3RpbmcuXG4gKi9cbmV4cG9ydCBmdW5jdGlvbiBjbGVhckVsZW1lbnRUeXBlcyAoKSB7XG4gIGZvciAoY29uc3QgdHlwZSBpbiBlbGVtZW50VHlwZXMpIHtcbiAgICBkZWxldGUgZWxlbWVudFR5cGVzW3R5cGVdXG4gIH1cbn1cbiIsIi8qXG4gKiBMaWNlbnNlZCB0byB0aGUgQXBhY2hlIFNvZnR3YXJlIEZvdW5kYXRpb24gKEFTRikgdW5kZXIgb25lXG4gKiBvciBtb3JlIGNvbnRyaWJ1dG9yIGxpY2Vuc2UgYWdyZWVtZW50cy4gIFNlZSB0aGUgTk9USUNFIGZpbGVcbiAqIGRpc3RyaWJ1dGVkIHdpdGggdGhpcyB3b3JrIGZvciBhZGRpdGlvbmFsIGluZm9ybWF0aW9uXG4gKiByZWdhcmRpbmcgY29weXJpZ2h0IG93bmVyc2hpcC4gIFRoZSBBU0YgbGljZW5zZXMgdGhpcyBmaWxlXG4gKiB0byB5b3UgdW5kZXIgdGhlIEFwYWNoZSBMaWNlbnNlLCBWZXJzaW9uIDIuMCAodGhlXG4gKiBcIkxpY2Vuc2VcIik7IHlvdSBtYXkgbm90IHVzZSB0aGlzIGZpbGUgZXhjZXB0IGluIGNvbXBsaWFuY2VcbiAqIHdpdGggdGhlIExpY2Vuc2UuICBZb3UgbWF5IG9idGFpbiBhIGNvcHkgb2YgdGhlIExpY2Vuc2UgYXRcbiAqXG4gKiAgIGh0dHA6Ly93d3cuYXBhY2hlLm9yZy9saWNlbnNlcy9MSUNFTlNFLTIuMFxuICpcbiAqIFVubGVzcyByZXF1aXJlZCBieSBhcHBsaWNhYmxlIGxhdyBvciBhZ3JlZWQgdG8gaW4gd3JpdGluZyxcbiAqIHNvZnR3YXJlIGRpc3RyaWJ1dGVkIHVuZGVyIHRoZSBMaWNlbnNlIGlzIGRpc3RyaWJ1dGVkIG9uIGFuXG4gKiBcIkFTIElTXCIgQkFTSVMsIFdJVEhPVVQgV0FSUkFOVElFUyBPUiBDT05ESVRJT05TIE9GIEFOWVxuICogS0lORCwgZWl0aGVyIGV4cHJlc3Mgb3IgaW1wbGllZC4gIFNlZSB0aGUgTGljZW5zZSBmb3IgdGhlXG4gKiBzcGVjaWZpYyBsYW5ndWFnZSBnb3Zlcm5pbmcgcGVybWlzc2lvbnMgYW5kIGxpbWl0YXRpb25zXG4gKiB1bmRlciB0aGUgTGljZW5zZS5cbiAqL1xuXG4vKipcbiAqIEBmaWxlT3ZlcnZpZXdcbiAqIFZpcnR1YWwtRE9NIEVsZW1lbnQuXG4gKi9cblxuaW1wb3J0ICcuLi8uLi9zaGFyZWQvb2JqZWN0QXNzaWduJ1xuaW1wb3J0IE5vZGUgZnJvbSAnLi9ub2RlJ1xuaW1wb3J0IHtcbiAgZ2V0RG9jLFxuICBnZXRUYXNrQ2VudGVyLFxuICB1bmlxdWVJZCxcbiAgbGlua1BhcmVudCxcbiAgbmV4dEVsZW1lbnQsXG4gIHByZXZpb3VzRWxlbWVudCxcbiAgaW5zZXJ0SW5kZXgsXG4gIG1vdmVJbmRleCxcbiAgcmVtb3ZlSW5kZXhcbn0gZnJvbSAnLi9vcGVyYXRpb24nXG5pbXBvcnQge1xuICBlbGVtZW50VHlwZXMsXG4gIHNldEVsZW1lbnRcbn0gZnJvbSAnLi9lbGVtZW50LXR5cGVzJ1xuXG5jb25zdCBERUZBVUxUX1RBR19OQU1FID0gJ2RpdidcbmNvbnN0IEJVQkJMRV9FVkVOVFMgPSBbJ2NsaWNrJywgJ2xvbmdwcmVzcycsICd0b3VjaHN0YXJ0JywgJ3RvdWNobW92ZScsICd0b3VjaGVuZCcsICdwYW5zdGFydCcsICdwYW5tb3ZlJywgJ3BhbmVuZCcsICdob3Jpem9udGFscGFuJywgJ3ZlcnRpY2FscGFuJywgJ3N3aXBlJ11cblxuZXhwb3J0IGRlZmF1bHQgZnVuY3Rpb24gRWxlbWVudCAodHlwZSA9IERFRkFVTFRfVEFHX05BTUUsIHByb3BzLCBpc0V4dGVuZGVkKSB7XG4gIGNvbnN0IFhFbGVtZW50ID0gZWxlbWVudFR5cGVzW3R5cGVdXG4gIGlmIChYRWxlbWVudCAmJiAhaXNFeHRlbmRlZCkge1xuICAgIHJldHVybiBuZXcgWEVsZW1lbnQocHJvcHMpXG4gIH1cbiAgcHJvcHMgPSBwcm9wcyB8fCB7fVxuICB0aGlzLm5vZGVUeXBlID0gMVxuICB0aGlzLm5vZGVJZCA9IHVuaXF1ZUlkKClcbiAgdGhpcy5yZWYgPSB0aGlzLm5vZGVJZFxuICB0aGlzLnR5cGUgPSB0eXBlXG4gIHRoaXMuYXR0ciA9IHByb3BzLmF0dHIgfHwge31cbiAgdGhpcy5zdHlsZSA9IHByb3BzLnN0eWxlIHx8IHt9XG4gIHRoaXMuY2xhc3NTdHlsZSA9IHByb3BzLmNsYXNzU3R5bGUgfHwge31cbiAgdGhpcy5ldmVudCA9IHt9XG4gIHRoaXMuY2hpbGRyZW4gPSBbXVxuICB0aGlzLnB1cmVDaGlsZHJlbiA9IFtdXG59XG5cbkVsZW1lbnQucHJvdG90eXBlID0gT2JqZWN0LmNyZWF0ZShOb2RlLnByb3RvdHlwZSlcbkVsZW1lbnQucHJvdG90eXBlLmNvbnN0cnVjdG9yID0gRWxlbWVudFxuXG5mdW5jdGlvbiByZWdpc3Rlck5vZGUgKGRvY0lkLCBub2RlKSB7XG4gIGNvbnN0IGRvYyA9IGdldERvYyhkb2NJZClcbiAgZG9jLm5vZGVNYXBbbm9kZS5ub2RlSWRdID0gbm9kZVxufVxuXG5zZXRFbGVtZW50KEVsZW1lbnQpXG5cbk9iamVjdC5hc3NpZ24oRWxlbWVudC5wcm90b3R5cGUsIHtcbiAgLyoqXG4gICAqIEFwcGVuZCBhIGNoaWxkIG5vZGUuXG4gICAqIEBwYXJhbSB7b2JqZWN0fSBub2RlXG4gICAqIEByZXR1cm4ge3VuZGVmaW5lZCB8IG51bWJlcn0gdGhlIHNpZ25hbCBzZW50IGJ5IG5hdGl2ZVxuICAgKi9cbiAgYXBwZW5kQ2hpbGQgKG5vZGUpIHtcbiAgICBpZiAobm9kZS5wYXJlbnROb2RlICYmIG5vZGUucGFyZW50Tm9kZSAhPT0gdGhpcykge1xuICAgICAgcmV0dXJuXG4gICAgfVxuICAgIC8qIGlzdGFuYnVsIGlnbm9yZSBlbHNlICovXG4gICAgaWYgKCFub2RlLnBhcmVudE5vZGUpIHtcbiAgICAgIGxpbmtQYXJlbnQobm9kZSwgdGhpcylcbiAgICAgIGluc2VydEluZGV4KG5vZGUsIHRoaXMuY2hpbGRyZW4sIHRoaXMuY2hpbGRyZW4ubGVuZ3RoLCB0cnVlKVxuICAgICAgaWYgKHRoaXMuZG9jSWQpIHtcbiAgICAgICAgcmVnaXN0ZXJOb2RlKHRoaXMuZG9jSWQsIG5vZGUpXG4gICAgICB9XG4gICAgICBpZiAobm9kZS5ub2RlVHlwZSA9PT0gMSkge1xuICAgICAgICBpbnNlcnRJbmRleChub2RlLCB0aGlzLnB1cmVDaGlsZHJlbiwgdGhpcy5wdXJlQ2hpbGRyZW4ubGVuZ3RoKVxuICAgICAgICBjb25zdCB0YXNrQ2VudGVyID0gZ2V0VGFza0NlbnRlcih0aGlzLmRvY0lkKVxuICAgICAgICBpZiAodGFza0NlbnRlcikge1xuICAgICAgICAgIHJldHVybiB0YXNrQ2VudGVyLnNlbmQoXG4gICAgICAgICAgICAnZG9tJyxcbiAgICAgICAgICAgIHsgYWN0aW9uOiAnYWRkRWxlbWVudCcgfSxcbiAgICAgICAgICAgIFt0aGlzLnJlZiwgbm9kZS50b0pTT04oKSwgLTFdXG4gICAgICAgICAgKVxuICAgICAgICB9XG4gICAgICB9XG4gICAgfVxuICAgIGVsc2Uge1xuICAgICAgbW92ZUluZGV4KG5vZGUsIHRoaXMuY2hpbGRyZW4sIHRoaXMuY2hpbGRyZW4ubGVuZ3RoLCB0cnVlKVxuICAgICAgaWYgKG5vZGUubm9kZVR5cGUgPT09IDEpIHtcbiAgICAgICAgY29uc3QgaW5kZXggPSBtb3ZlSW5kZXgobm9kZSwgdGhpcy5wdXJlQ2hpbGRyZW4sIHRoaXMucHVyZUNoaWxkcmVuLmxlbmd0aClcbiAgICAgICAgY29uc3QgdGFza0NlbnRlciA9IGdldFRhc2tDZW50ZXIodGhpcy5kb2NJZClcbiAgICAgICAgaWYgKHRhc2tDZW50ZXIgJiYgaW5kZXggPj0gMCkge1xuICAgICAgICAgIHJldHVybiB0YXNrQ2VudGVyLnNlbmQoXG4gICAgICAgICAgICAnZG9tJyxcbiAgICAgICAgICAgIHsgYWN0aW9uOiAnbW92ZUVsZW1lbnQnIH0sXG4gICAgICAgICAgICBbbm9kZS5yZWYsIHRoaXMucmVmLCBpbmRleF1cbiAgICAgICAgICApXG4gICAgICAgIH1cbiAgICAgIH1cbiAgICB9XG4gIH0sXG5cbiAgLyoqXG4gICAqIEluc2VydCBhIG5vZGUgYmVmb3JlIHNwZWNpZmllZCBub2RlLlxuICAgKiBAcGFyYW0ge29iamVjdH0gbm9kZVxuICAgKiBAcGFyYW0ge29iamVjdH0gYmVmb3JlXG4gICAqIEByZXR1cm4ge3VuZGVmaW5lZCB8IG51bWJlcn0gdGhlIHNpZ25hbCBzZW50IGJ5IG5hdGl2ZVxuICAgKi9cbiAgaW5zZXJ0QmVmb3JlIChub2RlLCBiZWZvcmUpIHtcbiAgICBpZiAobm9kZS5wYXJlbnROb2RlICYmIG5vZGUucGFyZW50Tm9kZSAhPT0gdGhpcykge1xuICAgICAgcmV0dXJuXG4gICAgfVxuICAgIGlmIChub2RlID09PSBiZWZvcmUgfHwgKG5vZGUubmV4dFNpYmxpbmcgJiYgbm9kZS5uZXh0U2libGluZyA9PT0gYmVmb3JlKSkge1xuICAgICAgcmV0dXJuXG4gICAgfVxuICAgIGlmICghbm9kZS5wYXJlbnROb2RlKSB7XG4gICAgICBsaW5rUGFyZW50KG5vZGUsIHRoaXMpXG4gICAgICBpbnNlcnRJbmRleChub2RlLCB0aGlzLmNoaWxkcmVuLCB0aGlzLmNoaWxkcmVuLmluZGV4T2YoYmVmb3JlKSwgdHJ1ZSlcbiAgICAgIGlmICh0aGlzLmRvY0lkKSB7XG4gICAgICAgIHJlZ2lzdGVyTm9kZSh0aGlzLmRvY0lkLCBub2RlKVxuICAgICAgfVxuICAgICAgaWYgKG5vZGUubm9kZVR5cGUgPT09IDEpIHtcbiAgICAgICAgY29uc3QgcHVyZUJlZm9yZSA9IG5leHRFbGVtZW50KGJlZm9yZSlcbiAgICAgICAgY29uc3QgaW5kZXggPSBpbnNlcnRJbmRleChcbiAgICAgICAgICBub2RlLFxuICAgICAgICAgIHRoaXMucHVyZUNoaWxkcmVuLFxuICAgICAgICAgIHB1cmVCZWZvcmVcbiAgICAgICAgICA/IHRoaXMucHVyZUNoaWxkcmVuLmluZGV4T2YocHVyZUJlZm9yZSlcbiAgICAgICAgICA6IHRoaXMucHVyZUNoaWxkcmVuLmxlbmd0aFxuICAgICAgICApXG4gICAgICAgIGNvbnN0IHRhc2tDZW50ZXIgPSBnZXRUYXNrQ2VudGVyKHRoaXMuZG9jSWQpXG4gICAgICAgIGlmICh0YXNrQ2VudGVyKSB7XG4gICAgICAgICAgcmV0dXJuIHRhc2tDZW50ZXIuc2VuZChcbiAgICAgICAgICAgICdkb20nLFxuICAgICAgICAgICAgeyBhY3Rpb246ICdhZGRFbGVtZW50JyB9LFxuICAgICAgICAgICAgW3RoaXMucmVmLCBub2RlLnRvSlNPTigpLCBpbmRleF1cbiAgICAgICAgICApXG4gICAgICAgIH1cbiAgICAgIH1cbiAgICB9XG4gICAgZWxzZSB7XG4gICAgICBtb3ZlSW5kZXgobm9kZSwgdGhpcy5jaGlsZHJlbiwgdGhpcy5jaGlsZHJlbi5pbmRleE9mKGJlZm9yZSksIHRydWUpXG4gICAgICBpZiAobm9kZS5ub2RlVHlwZSA9PT0gMSkge1xuICAgICAgICBjb25zdCBwdXJlQmVmb3JlID0gbmV4dEVsZW1lbnQoYmVmb3JlKVxuICAgICAgICAvKiBpc3RhbmJ1bCBpZ25vcmUgbmV4dCAqL1xuICAgICAgICBjb25zdCBpbmRleCA9IG1vdmVJbmRleChcbiAgICAgICAgICBub2RlLFxuICAgICAgICAgIHRoaXMucHVyZUNoaWxkcmVuLFxuICAgICAgICAgIHB1cmVCZWZvcmVcbiAgICAgICAgICA/IHRoaXMucHVyZUNoaWxkcmVuLmluZGV4T2YocHVyZUJlZm9yZSlcbiAgICAgICAgICA6IHRoaXMucHVyZUNoaWxkcmVuLmxlbmd0aFxuICAgICAgICApXG4gICAgICAgIGNvbnN0IHRhc2tDZW50ZXIgPSBnZXRUYXNrQ2VudGVyKHRoaXMuZG9jSWQpXG4gICAgICAgIGlmICh0YXNrQ2VudGVyICYmIGluZGV4ID49IDApIHtcbiAgICAgICAgICByZXR1cm4gdGFza0NlbnRlci5zZW5kKFxuICAgICAgICAgICAgJ2RvbScsXG4gICAgICAgICAgICB7IGFjdGlvbjogJ21vdmVFbGVtZW50JyB9LFxuICAgICAgICAgICAgW25vZGUucmVmLCB0aGlzLnJlZiwgaW5kZXhdXG4gICAgICAgICAgKVxuICAgICAgICB9XG4gICAgICB9XG4gICAgfVxuICB9LFxuXG4gIC8qKlxuICAgKiBJbnNlcnQgYSBub2RlIGFmdGVyIHNwZWNpZmllZCBub2RlLlxuICAgKiBAcGFyYW0ge29iamVjdH0gbm9kZVxuICAgKiBAcGFyYW0ge29iamVjdH0gYWZ0ZXJcbiAgICogQHJldHVybiB7dW5kZWZpbmVkIHwgbnVtYmVyfSB0aGUgc2lnbmFsIHNlbnQgYnkgbmF0aXZlXG4gICAqL1xuICBpbnNlcnRBZnRlciAobm9kZSwgYWZ0ZXIpIHtcbiAgICBpZiAobm9kZS5wYXJlbnROb2RlICYmIG5vZGUucGFyZW50Tm9kZSAhPT0gdGhpcykge1xuICAgICAgcmV0dXJuXG4gICAgfVxuICAgIGlmIChub2RlID09PSBhZnRlciB8fCAobm9kZS5wcmV2aW91c1NpYmxpbmcgJiYgbm9kZS5wcmV2aW91c1NpYmxpbmcgPT09IGFmdGVyKSkge1xuICAgICAgcmV0dXJuXG4gICAgfVxuICAgIGlmICghbm9kZS5wYXJlbnROb2RlKSB7XG4gICAgICBsaW5rUGFyZW50KG5vZGUsIHRoaXMpXG4gICAgICBpbnNlcnRJbmRleChub2RlLCB0aGlzLmNoaWxkcmVuLCB0aGlzLmNoaWxkcmVuLmluZGV4T2YoYWZ0ZXIpICsgMSwgdHJ1ZSlcbiAgICAgIC8qIGlzdGFuYnVsIGlnbm9yZSBlbHNlICovXG4gICAgICBpZiAodGhpcy5kb2NJZCkge1xuICAgICAgICByZWdpc3Rlck5vZGUodGhpcy5kb2NJZCwgbm9kZSlcbiAgICAgIH1cbiAgICAgIGlmIChub2RlLm5vZGVUeXBlID09PSAxKSB7XG4gICAgICAgIGNvbnN0IGluZGV4ID0gaW5zZXJ0SW5kZXgoXG4gICAgICAgICAgbm9kZSxcbiAgICAgICAgICB0aGlzLnB1cmVDaGlsZHJlbixcbiAgICAgICAgICB0aGlzLnB1cmVDaGlsZHJlbi5pbmRleE9mKHByZXZpb3VzRWxlbWVudChhZnRlcikpICsgMVxuICAgICAgICApXG4gICAgICAgIGNvbnN0IHRhc2tDZW50ZXIgPSBnZXRUYXNrQ2VudGVyKHRoaXMuZG9jSWQpXG4gICAgICAgIC8qIGlzdGFuYnVsIGlnbm9yZSBlbHNlICovXG4gICAgICAgIGlmICh0YXNrQ2VudGVyKSB7XG4gICAgICAgICAgcmV0dXJuIHRhc2tDZW50ZXIuc2VuZChcbiAgICAgICAgICAgICdkb20nLFxuICAgICAgICAgICAgeyBhY3Rpb246ICdhZGRFbGVtZW50JyB9LFxuICAgICAgICAgICAgW3RoaXMucmVmLCBub2RlLnRvSlNPTigpLCBpbmRleF1cbiAgICAgICAgICApXG4gICAgICAgIH1cbiAgICAgIH1cbiAgICB9XG4gICAgZWxzZSB7XG4gICAgICBtb3ZlSW5kZXgobm9kZSwgdGhpcy5jaGlsZHJlbiwgdGhpcy5jaGlsZHJlbi5pbmRleE9mKGFmdGVyKSArIDEsIHRydWUpXG4gICAgICBpZiAobm9kZS5ub2RlVHlwZSA9PT0gMSkge1xuICAgICAgICBjb25zdCBpbmRleCA9IG1vdmVJbmRleChcbiAgICAgICAgICBub2RlLFxuICAgICAgICAgIHRoaXMucHVyZUNoaWxkcmVuLFxuICAgICAgICAgIHRoaXMucHVyZUNoaWxkcmVuLmluZGV4T2YocHJldmlvdXNFbGVtZW50KGFmdGVyKSkgKyAxXG4gICAgICAgIClcbiAgICAgICAgY29uc3QgdGFza0NlbnRlciA9IGdldFRhc2tDZW50ZXIodGhpcy5kb2NJZClcbiAgICAgICAgaWYgKHRhc2tDZW50ZXIgJiYgaW5kZXggPj0gMCkge1xuICAgICAgICAgIHJldHVybiB0YXNrQ2VudGVyLnNlbmQoXG4gICAgICAgICAgICAnZG9tJyxcbiAgICAgICAgICAgIHsgYWN0aW9uOiAnbW92ZUVsZW1lbnQnIH0sXG4gICAgICAgICAgICBbbm9kZS5yZWYsIHRoaXMucmVmLCBpbmRleF1cbiAgICAgICAgICApXG4gICAgICAgIH1cbiAgICAgIH1cbiAgICB9XG4gIH0sXG5cbiAgLyoqXG4gICAqIFJlbW92ZSBhIGNoaWxkIG5vZGUsIGFuZCBkZWNpZGUgd2hldGhlciBpdCBzaG91bGQgYmUgZGVzdHJveWVkLlxuICAgKiBAcGFyYW0ge29iamVjdH0gbm9kZVxuICAgKiBAcGFyYW0ge2Jvb2xlYW59IHByZXNlcnZlZFxuICAgKi9cbiAgcmVtb3ZlQ2hpbGQgKG5vZGUsIHByZXNlcnZlZCkge1xuICAgIGlmIChub2RlLnBhcmVudE5vZGUpIHtcbiAgICAgIHJlbW92ZUluZGV4KG5vZGUsIHRoaXMuY2hpbGRyZW4sIHRydWUpXG4gICAgICBpZiAobm9kZS5ub2RlVHlwZSA9PT0gMSkge1xuICAgICAgICByZW1vdmVJbmRleChub2RlLCB0aGlzLnB1cmVDaGlsZHJlbilcbiAgICAgICAgY29uc3QgdGFza0NlbnRlciA9IGdldFRhc2tDZW50ZXIodGhpcy5kb2NJZClcbiAgICAgICAgaWYgKHRhc2tDZW50ZXIpIHtcbiAgICAgICAgICB0YXNrQ2VudGVyLnNlbmQoXG4gICAgICAgICAgICAnZG9tJyxcbiAgICAgICAgICAgIHsgYWN0aW9uOiAncmVtb3ZlRWxlbWVudCcgfSxcbiAgICAgICAgICAgIFtub2RlLnJlZl1cbiAgICAgICAgICApXG4gICAgICAgIH1cbiAgICAgIH1cbiAgICB9XG4gICAgaWYgKCFwcmVzZXJ2ZWQpIHtcbiAgICAgIG5vZGUuZGVzdHJveSgpXG4gICAgfVxuICB9LFxuXG4gIC8qKlxuICAgKiBDbGVhciBhbGwgY2hpbGQgbm9kZXMuXG4gICAqL1xuICBjbGVhciAoKSB7XG4gICAgY29uc3QgdGFza0NlbnRlciA9IGdldFRhc2tDZW50ZXIodGhpcy5kb2NJZClcbiAgICAvKiBpc3RhbmJ1bCBpZ25vcmUgZWxzZSAqL1xuICAgIGlmICh0YXNrQ2VudGVyKSB7XG4gICAgICB0aGlzLnB1cmVDaGlsZHJlbi5mb3JFYWNoKG5vZGUgPT4ge1xuICAgICAgICB0YXNrQ2VudGVyLnNlbmQoXG4gICAgICAgICAgJ2RvbScsXG4gICAgICAgICAgeyBhY3Rpb246ICdyZW1vdmVFbGVtZW50JyB9LFxuICAgICAgICAgIFtub2RlLnJlZl1cbiAgICAgICAgKVxuICAgICAgfSlcbiAgICB9XG4gICAgdGhpcy5jaGlsZHJlbi5mb3JFYWNoKG5vZGUgPT4ge1xuICAgICAgbm9kZS5kZXN0cm95KClcbiAgICB9KVxuICAgIHRoaXMuY2hpbGRyZW4ubGVuZ3RoID0gMFxuICAgIHRoaXMucHVyZUNoaWxkcmVuLmxlbmd0aCA9IDBcbiAgfSxcblxuICAvKipcbiAgICogU2V0IGFuIGF0dHJpYnV0ZSwgYW5kIGRlY2lkZSB3aGV0aGVyIHRoZSB0YXNrIHNob3VsZCBiZSBzZW5kIHRvIG5hdGl2ZS5cbiAgICogQHBhcmFtIHtzdHJpbmd9IGtleVxuICAgKiBAcGFyYW0ge3N0cmluZyB8IG51bWJlcn0gdmFsdWVcbiAgICogQHBhcmFtIHtib29sZWFufSBzaWxlbnRcbiAgICovXG4gIHNldEF0dHIgKGtleSwgdmFsdWUsIHNpbGVudCkge1xuICAgIGlmICh0aGlzLmF0dHJba2V5XSA9PT0gdmFsdWUgJiYgc2lsZW50ICE9PSBmYWxzZSkge1xuICAgICAgcmV0dXJuXG4gICAgfVxuICAgIHRoaXMuYXR0cltrZXldID0gdmFsdWVcbiAgICBjb25zdCB0YXNrQ2VudGVyID0gZ2V0VGFza0NlbnRlcih0aGlzLmRvY0lkKVxuICAgIGlmICghc2lsZW50ICYmIHRhc2tDZW50ZXIpIHtcbiAgICAgIGNvbnN0IHJlc3VsdCA9IHt9XG4gICAgICByZXN1bHRba2V5XSA9IHZhbHVlXG4gICAgICB0YXNrQ2VudGVyLnNlbmQoXG4gICAgICAgICdkb20nLFxuICAgICAgICB7IGFjdGlvbjogJ3VwZGF0ZUF0dHJzJyB9LFxuICAgICAgICBbdGhpcy5yZWYsIHJlc3VsdF1cbiAgICAgIClcbiAgICB9XG4gIH0sXG5cbiAgLyoqXG4gICAqIFNldCBhIHN0eWxlIHByb3BlcnR5LCBhbmQgZGVjaWRlIHdoZXRoZXIgdGhlIHRhc2sgc2hvdWxkIGJlIHNlbmQgdG8gbmF0aXZlLlxuICAgKiBAcGFyYW0ge3N0cmluZ30ga2V5XG4gICAqIEBwYXJhbSB7c3RyaW5nIHwgbnVtYmVyfSB2YWx1ZVxuICAgKiBAcGFyYW0ge2Jvb2xlYW59IHNpbGVudFxuICAgKi9cbiAgc2V0U3R5bGUgKGtleSwgdmFsdWUsIHNpbGVudCkge1xuICAgIGlmICh0aGlzLnN0eWxlW2tleV0gPT09IHZhbHVlICYmIHNpbGVudCAhPT0gZmFsc2UpIHtcbiAgICAgIHJldHVyblxuICAgIH1cbiAgICB0aGlzLnN0eWxlW2tleV0gPSB2YWx1ZVxuICAgIGNvbnN0IHRhc2tDZW50ZXIgPSBnZXRUYXNrQ2VudGVyKHRoaXMuZG9jSWQpXG4gICAgaWYgKCFzaWxlbnQgJiYgdGFza0NlbnRlcikge1xuICAgICAgY29uc3QgcmVzdWx0ID0ge31cbiAgICAgIHJlc3VsdFtrZXldID0gdmFsdWVcbiAgICAgIHRhc2tDZW50ZXIuc2VuZChcbiAgICAgICAgJ2RvbScsXG4gICAgICAgIHsgYWN0aW9uOiAndXBkYXRlU3R5bGUnIH0sXG4gICAgICAgIFt0aGlzLnJlZiwgcmVzdWx0XVxuICAgICAgKVxuICAgIH1cbiAgfSxcblxuICAvKipcbiAgICogU2V0IHN0eWxlIHByb3BlcnRpZXMgZnJvbSBjbGFzcy5cbiAgICogQHBhcmFtIHtvYmplY3R9IGNsYXNzU3R5bGVcbiAgICovXG4gIHNldENsYXNzU3R5bGUgKGNsYXNzU3R5bGUpIHtcbiAgICAvLyByZXNldCBwcmV2aW91cyBjbGFzcyBzdHlsZSB0byBlbXB0eSBzdHJpbmdcbiAgICBmb3IgKGNvbnN0IGtleSBpbiB0aGlzLmNsYXNzU3R5bGUpIHtcbiAgICAgIHRoaXMuY2xhc3NTdHlsZVtrZXldID0gJydcbiAgICB9XG5cbiAgICBPYmplY3QuYXNzaWduKHRoaXMuY2xhc3NTdHlsZSwgY2xhc3NTdHlsZSlcbiAgICBjb25zdCB0YXNrQ2VudGVyID0gZ2V0VGFza0NlbnRlcih0aGlzLmRvY0lkKVxuICAgIGlmICh0YXNrQ2VudGVyKSB7XG4gICAgICB0YXNrQ2VudGVyLnNlbmQoXG4gICAgICAgICdkb20nLFxuICAgICAgICB7IGFjdGlvbjogJ3VwZGF0ZVN0eWxlJyB9LFxuICAgICAgICBbdGhpcy5yZWYsIHRoaXMudG9TdHlsZSgpXVxuICAgICAgKVxuICAgIH1cbiAgfSxcblxuICAvKipcbiAgICogQWRkIGFuIGV2ZW50IGhhbmRsZXIuXG4gICAqIEBwYXJhbSB7c3RyaW5nfSBldmVudCB0eXBlXG4gICAqIEBwYXJhbSB7ZnVuY3Rpb259IGV2ZW50IGhhbmRsZXJcbiAgICovXG4gIGFkZEV2ZW50ICh0eXBlLCBoYW5kbGVyKSB7XG4gICAgaWYgKCF0aGlzLmV2ZW50W3R5cGVdKSB7XG4gICAgICB0aGlzLmV2ZW50W3R5cGVdID0gaGFuZGxlclxuICAgICAgY29uc3QgdGFza0NlbnRlciA9IGdldFRhc2tDZW50ZXIodGhpcy5kb2NJZClcbiAgICAgIGlmICh0YXNrQ2VudGVyKSB7XG4gICAgICAgIHRhc2tDZW50ZXIuc2VuZChcbiAgICAgICAgICAnZG9tJyxcbiAgICAgICAgICB7IGFjdGlvbjogJ2FkZEV2ZW50JyB9LFxuICAgICAgICAgIFt0aGlzLnJlZiwgdHlwZV1cbiAgICAgICAgKVxuICAgICAgfVxuICAgIH1cbiAgfSxcblxuICAvKipcbiAgICogUmVtb3ZlIGFuIGV2ZW50IGhhbmRsZXIuXG4gICAqIEBwYXJhbSB7c3RyaW5nfSBldmVudCB0eXBlXG4gICAqL1xuICByZW1vdmVFdmVudCAodHlwZSkge1xuICAgIGlmICh0aGlzLmV2ZW50W3R5cGVdKSB7XG4gICAgICBkZWxldGUgdGhpcy5ldmVudFt0eXBlXVxuICAgICAgY29uc3QgdGFza0NlbnRlciA9IGdldFRhc2tDZW50ZXIodGhpcy5kb2NJZClcbiAgICAgIGlmICh0YXNrQ2VudGVyKSB7XG4gICAgICAgIHRhc2tDZW50ZXIuc2VuZChcbiAgICAgICAgICAnZG9tJyxcbiAgICAgICAgICB7IGFjdGlvbjogJ3JlbW92ZUV2ZW50JyB9LFxuICAgICAgICAgIFt0aGlzLnJlZiwgdHlwZV1cbiAgICAgICAgKVxuICAgICAgfVxuICAgIH1cbiAgfSxcblxuICAvKipcbiAgICogRmlyZSBhbiBldmVudCBtYW51YWxseS5cbiAgICogQHBhcmFtIHtzdHJpbmd9IHR5cGUgdHlwZVxuICAgKiBAcGFyYW0ge2Z1bmN0aW9ufSBlIGhhbmRsZXJcbiAgICogQHBhcmFtIHtib29sZWFufSBpc0J1YmJsZSB3aGV0aGVyIG9yIG5vdCBldmVudCBidWJibGVcbiAgICogQHJldHVybiB7fSBhbnl0aGluZyByZXR1cm5lZCBieSBoYW5kbGVyIGZ1bmN0aW9uXG4gICAqL1xuICBmaXJlRXZlbnQgKHR5cGUsIGUsIGlzQnViYmxlKSB7XG4gICAgbGV0IGlzU3RvcFByb3BhZ2F0aW9uID0gZmFsc2VcbiAgICBjb25zdCBoYW5kbGVyID0gdGhpcy5ldmVudFt0eXBlXVxuICAgIGlmIChoYW5kbGVyICYmIGUpIHtcbiAgICAgIGUuc3RvcFByb3BhZ2F0aW9uID0gKCkgPT4ge1xuICAgICAgICBpc1N0b3BQcm9wYWdhdGlvbiA9IHRydWVcbiAgICAgIH1cbiAgICAgIGhhbmRsZXIuY2FsbCh0aGlzLCBlKVxuICAgIH1cblxuICAgIGlmIChpc1N0b3BQcm9wYWdhdGlvbikge1xuICAgICAgcmV0dXJuXG4gICAgfVxuXG4gICAgaWYgKGlzQnViYmxlICYmIEJVQkJMRV9FVkVOVFMuaW5jbHVkZXModHlwZSkgJiYgdGhpcy5wYXJlbnROb2RlICYmIHRoaXMucGFyZW50Tm9kZS5maXJlRXZlbnQpIHtcbiAgICAgIGUuY3VycmVudFRhcmdldCA9IHRoaXMucGFyZW50Tm9kZVxuICAgICAgdGhpcy5wYXJlbnROb2RlLmZpcmVFdmVudCh0eXBlLCBlLCBpc0J1YmJsZSlcbiAgICB9XG4gIH0sXG5cbiAgLyoqXG4gICAqIEdldCBhbGwgc3R5bGVzIG9mIGN1cnJlbnQgZWxlbWVudC5cbiAgICogQHJldHVybiB7b2JqZWN0fSBzdHlsZVxuICAgKi9cbiAgdG9TdHlsZSAoKSB7XG4gICAgcmV0dXJuIE9iamVjdC5hc3NpZ24oe30sIHRoaXMuY2xhc3NTdHlsZSwgdGhpcy5zdHlsZSlcbiAgfSxcblxuICAvKipcbiAgICogQ29udmVydCBjdXJyZW50IGVsZW1lbnQgdG8gSlNPTiBsaWtlIG9iamVjdC5cbiAgICogQHJldHVybiB7b2JqZWN0fSBlbGVtZW50XG4gICAqL1xuICB0b0pTT04gKCkge1xuICAgIGNvbnN0IHJlc3VsdCA9IHtcbiAgICAgIHJlZjogdGhpcy5yZWYudG9TdHJpbmcoKSxcbiAgICAgIHR5cGU6IHRoaXMudHlwZSxcbiAgICAgIGF0dHI6IHRoaXMuYXR0cixcbiAgICAgIHN0eWxlOiB0aGlzLnRvU3R5bGUoKVxuICAgIH1cbiAgICBjb25zdCBldmVudCA9IE9iamVjdC5rZXlzKHRoaXMuZXZlbnQpXG4gICAgaWYgKGV2ZW50Lmxlbmd0aCkge1xuICAgICAgcmVzdWx0LmV2ZW50ID0gZXZlbnRcbiAgICB9XG4gICAgaWYgKHRoaXMucHVyZUNoaWxkcmVuLmxlbmd0aCkge1xuICAgICAgcmVzdWx0LmNoaWxkcmVuID0gdGhpcy5wdXJlQ2hpbGRyZW4ubWFwKChjaGlsZCkgPT4gY2hpbGQudG9KU09OKCkpXG4gICAgfVxuICAgIHJldHVybiByZXN1bHRcbiAgfSxcblxuICAvKipcbiAgICogQ29udmVydCB0byBIVE1MIGVsZW1lbnQgdGFnIHN0cmluZy5cbiAgICogQHJldHVybiB7c3Rpcm5nfSBodG1sXG4gICAqL1xuICB0b1N0cmluZyAoKSB7XG4gICAgcmV0dXJuICc8JyArIHRoaXMudHlwZSArXG4gICAgJyBhdHRyPScgKyBKU09OLnN0cmluZ2lmeSh0aGlzLmF0dHIpICtcbiAgICAnIHN0eWxlPScgKyBKU09OLnN0cmluZ2lmeSh0aGlzLnRvU3R5bGUoKSkgKyAnPicgK1xuICAgIHRoaXMucHVyZUNoaWxkcmVuLm1hcCgoY2hpbGQpID0+IGNoaWxkLnRvU3RyaW5nKCkpLmpvaW4oJycpICtcbiAgICAnPC8nICsgdGhpcy50eXBlICsgJz4nXG4gIH1cbn0pXG4iLCIvKlxuICogTGljZW5zZWQgdG8gdGhlIEFwYWNoZSBTb2Z0d2FyZSBGb3VuZGF0aW9uIChBU0YpIHVuZGVyIG9uZVxuICogb3IgbW9yZSBjb250cmlidXRvciBsaWNlbnNlIGFncmVlbWVudHMuICBTZWUgdGhlIE5PVElDRSBmaWxlXG4gKiBkaXN0cmlidXRlZCB3aXRoIHRoaXMgd29yayBmb3IgYWRkaXRpb25hbCBpbmZvcm1hdGlvblxuICogcmVnYXJkaW5nIGNvcHlyaWdodCBvd25lcnNoaXAuICBUaGUgQVNGIGxpY2Vuc2VzIHRoaXMgZmlsZVxuICogdG8geW91IHVuZGVyIHRoZSBBcGFjaGUgTGljZW5zZSwgVmVyc2lvbiAyLjAgKHRoZVxuICogXCJMaWNlbnNlXCIpOyB5b3UgbWF5IG5vdCB1c2UgdGhpcyBmaWxlIGV4Y2VwdCBpbiBjb21wbGlhbmNlXG4gKiB3aXRoIHRoZSBMaWNlbnNlLiAgWW91IG1heSBvYnRhaW4gYSBjb3B5IG9mIHRoZSBMaWNlbnNlIGF0XG4gKlxuICogICBodHRwOi8vd3d3LmFwYWNoZS5vcmcvbGljZW5zZXMvTElDRU5TRS0yLjBcbiAqXG4gKiBVbmxlc3MgcmVxdWlyZWQgYnkgYXBwbGljYWJsZSBsYXcgb3IgYWdyZWVkIHRvIGluIHdyaXRpbmcsXG4gKiBzb2Z0d2FyZSBkaXN0cmlidXRlZCB1bmRlciB0aGUgTGljZW5zZSBpcyBkaXN0cmlidXRlZCBvbiBhblxuICogXCJBUyBJU1wiIEJBU0lTLCBXSVRIT1VUIFdBUlJBTlRJRVMgT1IgQ09ORElUSU9OUyBPRiBBTllcbiAqIEtJTkQsIGVpdGhlciBleHByZXNzIG9yIGltcGxpZWQuICBTZWUgdGhlIExpY2Vuc2UgZm9yIHRoZVxuICogc3BlY2lmaWMgbGFuZ3VhZ2UgZ292ZXJuaW5nIHBlcm1pc3Npb25zIGFuZCBsaW1pdGF0aW9uc1xuICogdW5kZXIgdGhlIExpY2Vuc2UuXG4gKi9cbmltcG9ydCBDYWxsYmFja01hbmFnZXIgZnJvbSAnLi9jYWxsYmFjay1tYW5hZ2VyJ1xuaW1wb3J0IEVsZW1lbnQgZnJvbSAnLi92ZG9tL2VsZW1lbnQnXG5cbmxldCBmYWxsYmFjayA9IGZ1bmN0aW9uICgpIHt9XG5cbi8vIFRoZSBBUEkgb2YgVGFza0NlbnRlciB3b3VsZCBiZSByZS1kZXNpZ24uXG5leHBvcnQgY2xhc3MgVGFza0NlbnRlciB7XG4gIGNvbnN0cnVjdG9yIChpZCwgc2VuZFRhc2tzKSB7XG4gICAgT2JqZWN0LmRlZmluZVByb3BlcnR5KHRoaXMsICdpbnN0YW5jZUlkJywge1xuICAgICAgZW51bWVyYWJsZTogdHJ1ZSxcbiAgICAgIHZhbHVlOiBpZFxuICAgIH0pXG4gICAgT2JqZWN0LmRlZmluZVByb3BlcnR5KHRoaXMsICdjYWxsYmFja01hbmFnZXInLCB7XG4gICAgICBlbnVtZXJhYmxlOiB0cnVlLFxuICAgICAgdmFsdWU6IG5ldyBDYWxsYmFja01hbmFnZXIoKVxuICAgIH0pXG4gICAgZmFsbGJhY2sgPSBzZW5kVGFza3MgfHwgZnVuY3Rpb24gKCkge31cbiAgfVxuXG4gIGNhbGxiYWNrIChjYWxsYmFja0lkLCBkYXRhLCBpZktlZXBBbGl2ZSkge1xuICAgIHJldHVybiB0aGlzLmNhbGxiYWNrTWFuYWdlci5jb25zdW1lKGNhbGxiYWNrSWQsIGRhdGEsIGlmS2VlcEFsaXZlKVxuICB9XG5cbiAgZGVzdHJveUNhbGxiYWNrICgpIHtcbiAgICByZXR1cm4gdGhpcy5jYWxsYmFja01hbmFnZXIuY2xvc2UoKVxuICB9XG5cbiAgdHlwb2YgKHYpIHtcbiAgICBjb25zdCBzID0gT2JqZWN0LnByb3RvdHlwZS50b1N0cmluZy5jYWxsKHYpXG4gICAgcmV0dXJuIHMuc3Vic3RyaW5nKDgsIHMubGVuZ3RoIC0gMSkudG9Mb3dlckNhc2UoKVxuICB9XG5cbiAgLyoqXG4gICAqIE5vcm1hbGl6ZSBhIHZhbHVlLiBTcGVjaWFsbHksIGlmIHRoZSB2YWx1ZSBpcyBhIGZ1bmN0aW9uLCB0aGVuIGdlbmVyYXRlIGEgZnVuY3Rpb24gaWRcbiAgICogYW5kIHNhdmUgaXQgdG8gYENhbGxiYWNrTWFuYWdlcmAsIGF0IGxhc3QgcmV0dXJuIHRoZSBmdW5jdGlvbiBpZC5cbiAgICogQHBhcmFtICB7YW55fSAgICAgICAgdlxuICAgKiBAcGFyYW0gIHtvYmplY3R9ICAgICBhcHBcbiAgICogQHJldHVybiB7cHJpbWl0aXZlfVxuICAgKi9cbiAgbm9ybWFsaXplICh2KSB7XG4gICAgY29uc3QgdHlwZSA9IHRoaXMudHlwb2YodilcblxuICAgIHN3aXRjaCAodHlwZSkge1xuICAgICAgY2FzZSAndW5kZWZpbmVkJzpcbiAgICAgIGNhc2UgJ251bGwnOlxuICAgICAgICByZXR1cm4gJydcbiAgICAgIGNhc2UgJ3JlZ2V4cCc6XG4gICAgICAgIHJldHVybiB2LnRvU3RyaW5nKClcbiAgICAgIGNhc2UgJ2RhdGUnOlxuICAgICAgICByZXR1cm4gdi50b0lTT1N0cmluZygpXG4gICAgICBjYXNlICdudW1iZXInOlxuICAgICAgY2FzZSAnc3RyaW5nJzpcbiAgICAgIGNhc2UgJ2Jvb2xlYW4nOlxuICAgICAgY2FzZSAnYXJyYXknOlxuICAgICAgY2FzZSAnb2JqZWN0JzpcbiAgICAgICAgaWYgKHYgaW5zdGFuY2VvZiBFbGVtZW50KSB7XG4gICAgICAgICAgcmV0dXJuIHYucmVmXG4gICAgICAgIH1cbiAgICAgICAgcmV0dXJuIHZcbiAgICAgIGNhc2UgJ2Z1bmN0aW9uJzpcbiAgICAgICAgcmV0dXJuIHRoaXMuY2FsbGJhY2tNYW5hZ2VyLmFkZCh2KS50b1N0cmluZygpXG4gICAgICAvKiBpc3RhbmJ1bCBpZ25vcmUgbmV4dCAqL1xuICAgICAgZGVmYXVsdDpcbiAgICAgICAgcmV0dXJuIEpTT04uc3RyaW5naWZ5KHYpXG4gICAgfVxuICB9XG5cbiAgc2VuZCAodHlwZSwgb3B0aW9ucywgYXJncykge1xuICAgIGNvbnN0IHsgYWN0aW9uLCBjb21wb25lbnQsIHJlZiwgbW9kdWxlLCBtZXRob2QgfSA9IG9wdGlvbnNcblxuICAgIGFyZ3MgPSBhcmdzLm1hcChhcmcgPT4gdGhpcy5ub3JtYWxpemUoYXJnKSlcblxuICAgIHN3aXRjaCAodHlwZSkge1xuICAgICAgY2FzZSAnZG9tJzpcbiAgICAgICAgcmV0dXJuIHRoaXNbYWN0aW9uXSh0aGlzLmluc3RhbmNlSWQsIGFyZ3MpXG4gICAgICBjYXNlICdjb21wb25lbnQnOlxuICAgICAgICByZXR1cm4gdGhpcy5jb21wb25lbnRIYW5kbGVyKHRoaXMuaW5zdGFuY2VJZCwgcmVmLCBtZXRob2QsIGFyZ3MsIHsgY29tcG9uZW50IH0pXG4gICAgICBkZWZhdWx0OlxuICAgICAgICByZXR1cm4gdGhpcy5tb2R1bGVIYW5kbGVyKHRoaXMuaW5zdGFuY2VJZCwgbW9kdWxlLCBtZXRob2QsIGFyZ3MsIHt9KVxuICAgIH1cbiAgfVxuXG4gIGNhbGxET00gKGFjdGlvbiwgYXJncykge1xuICAgIHJldHVybiB0aGlzW2FjdGlvbl0odGhpcy5pbnN0YW5jZUlkLCBhcmdzKVxuICB9XG5cbiAgY2FsbENvbXBvbmVudCAocmVmLCBtZXRob2QsIGFyZ3MpIHtcbiAgICByZXR1cm4gdGhpcy5jb21wb25lbnRIYW5kbGVyKHRoaXMuaW5zdGFuY2VJZCwgcmVmLCBtZXRob2QsIGFyZ3MsIHt9KVxuICB9XG5cbiAgY2FsbE1vZHVsZSAobW9kdWxlLCBtZXRob2QsIGFyZ3MpIHtcbiAgICByZXR1cm4gdGhpcy5tb2R1bGVIYW5kbGVyKHRoaXMuaW5zdGFuY2VJZCwgbW9kdWxlLCBtZXRob2QsIGFyZ3MsIHt9KVxuICB9XG59XG5cbmV4cG9ydCBmdW5jdGlvbiBpbml0ICgpIHtcbiAgY29uc3QgRE9NX01FVEhPRFMgPSB7XG4gICAgY3JlYXRlRmluaXNoOiBnbG9iYWwuY2FsbENyZWF0ZUZpbmlzaCxcbiAgICB1cGRhdGVGaW5pc2g6IGdsb2JhbC5jYWxsVXBkYXRlRmluaXNoLFxuICAgIHJlZnJlc2hGaW5pc2g6IGdsb2JhbC5jYWxsUmVmcmVzaEZpbmlzaCxcblxuICAgIGNyZWF0ZUJvZHk6IGdsb2JhbC5jYWxsQ3JlYXRlQm9keSxcblxuICAgIGFkZEVsZW1lbnQ6IGdsb2JhbC5jYWxsQWRkRWxlbWVudCxcbiAgICByZW1vdmVFbGVtZW50OiBnbG9iYWwuY2FsbFJlbW92ZUVsZW1lbnQsXG4gICAgbW92ZUVsZW1lbnQ6IGdsb2JhbC5jYWxsTW92ZUVsZW1lbnQsXG4gICAgdXBkYXRlQXR0cnM6IGdsb2JhbC5jYWxsVXBkYXRlQXR0cnMsXG4gICAgdXBkYXRlU3R5bGU6IGdsb2JhbC5jYWxsVXBkYXRlU3R5bGUsXG5cbiAgICBhZGRFdmVudDogZ2xvYmFsLmNhbGxBZGRFdmVudCxcbiAgICByZW1vdmVFdmVudDogZ2xvYmFsLmNhbGxSZW1vdmVFdmVudFxuICB9XG4gIGNvbnN0IHByb3RvID0gVGFza0NlbnRlci5wcm90b3R5cGVcblxuICBmb3IgKGNvbnN0IG5hbWUgaW4gRE9NX01FVEhPRFMpIHtcbiAgICBjb25zdCBtZXRob2QgPSBET01fTUVUSE9EU1tuYW1lXVxuICAgIHByb3RvW25hbWVdID0gbWV0aG9kID9cbiAgICAgIChpZCwgYXJncykgPT4gbWV0aG9kKGlkLCAuLi5hcmdzKSA6XG4gICAgICAoaWQsIGFyZ3MpID0+IGZhbGxiYWNrKGlkLCBbeyBtb2R1bGU6ICdkb20nLCBtZXRob2Q6IG5hbWUsIGFyZ3MgfV0sICctMScpXG4gIH1cblxuICBwcm90by5jb21wb25lbnRIYW5kbGVyID0gZ2xvYmFsLmNhbGxOYXRpdmVDb21wb25lbnQgfHxcbiAgICAoKGlkLCByZWYsIG1ldGhvZCwgYXJncywgb3B0aW9ucykgPT5cbiAgICAgIGZhbGxiYWNrKGlkLCBbeyBjb21wb25lbnQ6IG9wdGlvbnMuY29tcG9uZW50LCByZWYsIG1ldGhvZCwgYXJncyB9XSkpXG5cbiAgcHJvdG8ubW9kdWxlSGFuZGxlciA9IGdsb2JhbC5jYWxsTmF0aXZlTW9kdWxlIHx8XG4gICAgKChpZCwgbW9kdWxlLCBtZXRob2QsIGFyZ3MpID0+XG4gICAgICBmYWxsYmFjayhpZCwgW3sgbW9kdWxlLCBtZXRob2QsIGFyZ3MgfV0pKVxufVxuIiwiLypcbiAqIExpY2Vuc2VkIHRvIHRoZSBBcGFjaGUgU29mdHdhcmUgRm91bmRhdGlvbiAoQVNGKSB1bmRlciBvbmVcbiAqIG9yIG1vcmUgY29udHJpYnV0b3IgbGljZW5zZSBhZ3JlZW1lbnRzLiAgU2VlIHRoZSBOT1RJQ0UgZmlsZVxuICogZGlzdHJpYnV0ZWQgd2l0aCB0aGlzIHdvcmsgZm9yIGFkZGl0aW9uYWwgaW5mb3JtYXRpb25cbiAqIHJlZ2FyZGluZyBjb3B5cmlnaHQgb3duZXJzaGlwLiAgVGhlIEFTRiBsaWNlbnNlcyB0aGlzIGZpbGVcbiAqIHRvIHlvdSB1bmRlciB0aGUgQXBhY2hlIExpY2Vuc2UsIFZlcnNpb24gMi4wICh0aGVcbiAqIFwiTGljZW5zZVwiKTsgeW91IG1heSBub3QgdXNlIHRoaXMgZmlsZSBleGNlcHQgaW4gY29tcGxpYW5jZVxuICogd2l0aCB0aGUgTGljZW5zZS4gIFlvdSBtYXkgb2J0YWluIGEgY29weSBvZiB0aGUgTGljZW5zZSBhdFxuICpcbiAqICAgaHR0cDovL3d3dy5hcGFjaGUub3JnL2xpY2Vuc2VzL0xJQ0VOU0UtMi4wXG4gKlxuICogVW5sZXNzIHJlcXVpcmVkIGJ5IGFwcGxpY2FibGUgbGF3IG9yIGFncmVlZCB0byBpbiB3cml0aW5nLFxuICogc29mdHdhcmUgZGlzdHJpYnV0ZWQgdW5kZXIgdGhlIExpY2Vuc2UgaXMgZGlzdHJpYnV0ZWQgb24gYW5cbiAqIFwiQVMgSVNcIiBCQVNJUywgV0lUSE9VVCBXQVJSQU5USUVTIE9SIENPTkRJVElPTlMgT0YgQU5ZXG4gKiBLSU5ELCBlaXRoZXIgZXhwcmVzcyBvciBpbXBsaWVkLiAgU2VlIHRoZSBMaWNlbnNlIGZvciB0aGVcbiAqIHNwZWNpZmljIGxhbmd1YWdlIGdvdmVybmluZyBwZXJtaXNzaW9ucyBhbmQgbGltaXRhdGlvbnNcbiAqIHVuZGVyIHRoZSBMaWNlbnNlLlxuICovXG5cbi8vIEpTIFNlcnZpY2VzXG5cbmV4cG9ydCBjb25zdCBzZXJ2aWNlcyA9IFtdXG5cbi8qKlxuICogUmVnaXN0ZXIgYSBKYXZhU2NyaXB0IHNlcnZpY2UuXG4gKiBBIEphdmFTY3JpcHQgc2VydmljZSBvcHRpb25zIGNvdWxkIGhhdmUgYSBzZXQgb2YgbGlmZWN5Y2xlIG1ldGhvZHNcbiAqIGZvciBlYWNoIFdlZXggaW5zdGFuY2UuIEZvciBleGFtcGxlOiBjcmVhdGUsIHJlZnJlc2gsIGRlc3Ryb3kuXG4gKiBGb3IgdGhlIEpTIGZyYW1ld29yayBtYWludGFpbmVyIGlmIHlvdSB3YW50IHRvIHN1cHBseSBzb21lIGZlYXR1cmVzXG4gKiB3aGljaCBuZWVkIHRvIHdvcmsgd2VsbCBpbiBkaWZmZXJlbnQgV2VleCBpbnN0YW5jZXMsIGV2ZW4gaW4gZGlmZmVyZW50XG4gKiBmcmFtZXdvcmtzIHNlcGFyYXRlbHkuIFlvdSBjYW4gbWFrZSBhIEphdmFTY3JpcHQgc2VydmljZSB0byBpbml0XG4gKiBpdHMgdmFyaWFibGVzIG9yIGNsYXNzZXMgZm9yIGVhY2ggV2VleCBpbnN0YW5jZSB3aGVuIGl0J3MgY3JlYXRlZFxuICogYW5kIHJlY3ljbGUgdGhlbSB3aGVuIGl0J3MgZGVzdHJveWVkLlxuICogQHBhcmFtIHtvYmplY3R9IG9wdGlvbnMgQ291bGQgaGF2ZSB7IGNyZWF0ZSwgcmVmcmVzaCwgZGVzdHJveSB9XG4gKiAgICAgICAgICAgICAgICAgICAgICAgICBsaWZlY3ljbGUgbWV0aG9kcy4gSW4gY3JlYXRlIG1ldGhvZCBpdCBzaG91bGRcbiAqICAgICAgICAgICAgICAgICAgICAgICAgIHJldHVybiBhbiBvYmplY3Qgb2Ygd2hhdCB2YXJpYWJsZXMgb3IgY2xhc3Nlc1xuICogICAgICAgICAgICAgICAgICAgICAgICAgd291bGQgYmUgaW5qZWN0ZWQgaW50byB0aGUgV2VleCBpbnN0YW5jZS5cbiAqL1xuZXhwb3J0IGZ1bmN0aW9uIHJlZ2lzdGVyIChuYW1lLCBvcHRpb25zKSB7XG4gIGlmIChoYXMobmFtZSkpIHtcbiAgICBjb25zb2xlLndhcm4oYFNlcnZpY2UgXCIke25hbWV9XCIgaGFzIGJlZW4gcmVnaXN0ZXJlZCBhbHJlYWR5IWApXG4gIH1cbiAgZWxzZSB7XG4gICAgb3B0aW9ucyA9IE9iamVjdC5hc3NpZ24oe30sIG9wdGlvbnMpXG4gICAgc2VydmljZXMucHVzaCh7IG5hbWUsIG9wdGlvbnMgfSlcbiAgfVxufVxuXG4vKipcbiAqIFVucmVnaXN0ZXIgYSBKYXZhU2NyaXB0IHNlcnZpY2UgYnkgbmFtZVxuICogQHBhcmFtIHtzdHJpbmd9IG5hbWVcbiAqL1xuZXhwb3J0IGZ1bmN0aW9uIHVucmVnaXN0ZXIgKG5hbWUpIHtcbiAgc2VydmljZXMuc29tZSgoc2VydmljZSwgaW5kZXgpID0+IHtcbiAgICBpZiAoc2VydmljZS5uYW1lID09PSBuYW1lKSB7XG4gICAgICBzZXJ2aWNlcy5zcGxpY2UoaW5kZXgsIDEpXG4gICAgICByZXR1cm4gdHJ1ZVxuICAgIH1cbiAgfSlcbn1cblxuLyoqXG4gKiBDaGVjayBpZiBhIEphdmFTY3JpcHQgc2VydmljZSB3aXRoIGEgY2VydGFpbiBuYW1lIGV4aXN0ZWQuXG4gKiBAcGFyYW0gIHtzdHJpbmd9ICBuYW1lXG4gKiBAcmV0dXJuIHtCb29sZWFufVxuICovXG5leHBvcnQgZnVuY3Rpb24gaGFzIChuYW1lKSB7XG4gIHJldHVybiBpbmRleE9mKG5hbWUpID49IDBcbn1cblxuLyoqXG4gKiBGaW5kIHRoZSBpbmRleCBvZiBhIEphdmFTY3JpcHQgc2VydmljZSBieSBuYW1lXG4gKiBAcGFyYW0gIHtzdHJpbmd9IG5hbWVcbiAqIEByZXR1cm4ge251bWJlcn1cbiAqL1xuZnVuY3Rpb24gaW5kZXhPZiAobmFtZSkge1xuICByZXR1cm4gc2VydmljZXMubWFwKHNlcnZpY2UgPT4gc2VydmljZS5uYW1lKS5pbmRleE9mKG5hbWUpXG59XG4iLCIvKlxuICogTGljZW5zZWQgdG8gdGhlIEFwYWNoZSBTb2Z0d2FyZSBGb3VuZGF0aW9uIChBU0YpIHVuZGVyIG9uZVxuICogb3IgbW9yZSBjb250cmlidXRvciBsaWNlbnNlIGFncmVlbWVudHMuICBTZWUgdGhlIE5PVElDRSBmaWxlXG4gKiBkaXN0cmlidXRlZCB3aXRoIHRoaXMgd29yayBmb3IgYWRkaXRpb25hbCBpbmZvcm1hdGlvblxuICogcmVnYXJkaW5nIGNvcHlyaWdodCBvd25lcnNoaXAuICBUaGUgQVNGIGxpY2Vuc2VzIHRoaXMgZmlsZVxuICogdG8geW91IHVuZGVyIHRoZSBBcGFjaGUgTGljZW5zZSwgVmVyc2lvbiAyLjAgKHRoZVxuICogXCJMaWNlbnNlXCIpOyB5b3UgbWF5IG5vdCB1c2UgdGhpcyBmaWxlIGV4Y2VwdCBpbiBjb21wbGlhbmNlXG4gKiB3aXRoIHRoZSBMaWNlbnNlLiAgWW91IG1heSBvYnRhaW4gYSBjb3B5IG9mIHRoZSBMaWNlbnNlIGF0XG4gKlxuICogICBodHRwOi8vd3d3LmFwYWNoZS5vcmcvbGljZW5zZXMvTElDRU5TRS0yLjBcbiAqXG4gKiBVbmxlc3MgcmVxdWlyZWQgYnkgYXBwbGljYWJsZSBsYXcgb3IgYWdyZWVkIHRvIGluIHdyaXRpbmcsXG4gKiBzb2Z0d2FyZSBkaXN0cmlidXRlZCB1bmRlciB0aGUgTGljZW5zZSBpcyBkaXN0cmlidXRlZCBvbiBhblxuICogXCJBUyBJU1wiIEJBU0lTLCBXSVRIT1VUIFdBUlJBTlRJRVMgT1IgQ09ORElUSU9OUyBPRiBBTllcbiAqIEtJTkQsIGVpdGhlciBleHByZXNzIG9yIGltcGxpZWQuICBTZWUgdGhlIExpY2Vuc2UgZm9yIHRoZVxuICogc3BlY2lmaWMgbGFuZ3VhZ2UgZ292ZXJuaW5nIHBlcm1pc3Npb25zIGFuZCBsaW1pdGF0aW9uc1xuICogdW5kZXIgdGhlIExpY2Vuc2UuXG4gKi9cbmltcG9ydCB7IGluaXQgYXMgaW5pdFRhc2tIYW5kbGVyIH0gZnJvbSAnLi90YXNrLWNlbnRlcidcbmltcG9ydCB7IHJlZ2lzdGVyRWxlbWVudCB9IGZyb20gJy4vdmRvbS9lbGVtZW50LXR5cGVzJ1xuaW1wb3J0IHsgc2VydmljZXMsIHJlZ2lzdGVyLCB1bnJlZ2lzdGVyIH0gZnJvbSAnLi9zZXJ2aWNlJ1xuXG5sZXQgZnJhbWV3b3Jrc1xubGV0IHJ1bnRpbWVDb25maWdcblxuY29uc3QgdmVyc2lvblJlZ0V4cCA9IC9eXFxzKlxcL1xcLyAqKFxce1tefV0qXFx9KSAqXFxyP1xcbi9cblxuLyoqXG4gKiBEZXRlY3QgYSBKUyBCdW5kbGUgY29kZSBhbmQgbWFrZSBzdXJlIHdoaWNoIGZyYW1ld29yayBpdCdzIGJhc2VkIHRvLiBFYWNoIEpTXG4gKiBCdW5kbGUgc2hvdWxkIG1ha2Ugc3VyZSB0aGF0IGl0IHN0YXJ0cyB3aXRoIGEgbGluZSBvZiBKU09OIGNvbW1lbnQgYW5kIGlzXG4gKiBtb3JlIHRoYXQgb25lIGxpbmUuXG4gKiBAcGFyYW0gIHtzdHJpbmd9IGNvZGVcbiAqIEByZXR1cm4ge29iamVjdH1cbiAqL1xuZnVuY3Rpb24gY2hlY2tWZXJzaW9uIChjb2RlKSB7XG4gIGxldCBpbmZvXG4gIGNvbnN0IHJlc3VsdCA9IHZlcnNpb25SZWdFeHAuZXhlYyhjb2RlKVxuICBpZiAocmVzdWx0KSB7XG4gICAgdHJ5IHtcbiAgICAgIGluZm8gPSBKU09OLnBhcnNlKHJlc3VsdFsxXSlcbiAgICB9XG4gICAgY2F0Y2ggKGUpIHt9XG4gIH1cbiAgcmV0dXJuIGluZm9cbn1cblxuZnVuY3Rpb24gY3JlYXRlU2VydmljZXMgKGlkLCBlbnYsIGNvbmZpZykge1xuICAvLyBJbml0IEphdmFTY3JpcHQgc2VydmljZXMgZm9yIHRoaXMgaW5zdGFuY2UuXG4gIGNvbnN0IHNlcnZpY2VNYXAgPSBPYmplY3QuY3JlYXRlKG51bGwpXG4gIHNlcnZpY2VNYXAuc2VydmljZSA9IE9iamVjdC5jcmVhdGUobnVsbClcbiAgc2VydmljZXMuZm9yRWFjaCgoeyBuYW1lLCBvcHRpb25zIH0pID0+IHtcbiAgICBpZiAocHJvY2Vzcy5lbnYuTk9ERV9FTlYgPT09ICdkZXZlbG9wbWVudCcpIHtcbiAgICAgIGNvbnNvbGUuZGVidWcoYFtKUyBSdW50aW1lXSBjcmVhdGUgc2VydmljZSAke25hbWV9LmApXG4gICAgfVxuICAgIGNvbnN0IGNyZWF0ZSA9IG9wdGlvbnMuY3JlYXRlXG4gICAgaWYgKGNyZWF0ZSkge1xuICAgICAgY29uc3QgcmVzdWx0ID0gY3JlYXRlKGlkLCBlbnYsIGNvbmZpZylcbiAgICAgIE9iamVjdC5hc3NpZ24oc2VydmljZU1hcC5zZXJ2aWNlLCByZXN1bHQpXG4gICAgICBPYmplY3QuYXNzaWduKHNlcnZpY2VNYXAsIHJlc3VsdC5pbnN0YW5jZSlcbiAgICB9XG4gIH0pXG4gIGRlbGV0ZSBzZXJ2aWNlTWFwLnNlcnZpY2UuaW5zdGFuY2VcbiAgT2JqZWN0LmZyZWV6ZShzZXJ2aWNlTWFwLnNlcnZpY2UpXG4gIHJldHVybiBzZXJ2aWNlTWFwXG59XG5cbmNvbnN0IGluc3RhbmNlTWFwID0ge31cblxuLyoqXG4gKiBDaGVjayB3aGljaCBmcmFtZXdvcmsgYSBjZXJ0YWluIEpTIEJ1bmRsZSBjb2RlIGJhc2VkIHRvLiBBbmQgY3JlYXRlIGluc3RhbmNlXG4gKiBieSB0aGlzIGZyYW1ld29yay5cbiAqIEBwYXJhbSB7c3RyaW5nfSBpZFxuICogQHBhcmFtIHtzdHJpbmd9IGNvZGVcbiAqIEBwYXJhbSB7b2JqZWN0fSBjb25maWdcbiAqIEBwYXJhbSB7b2JqZWN0fSBkYXRhXG4gKi9cbmZ1bmN0aW9uIGNyZWF0ZUluc3RhbmNlIChpZCwgY29kZSwgY29uZmlnLCBkYXRhKSB7XG4gIGxldCBpbmZvID0gaW5zdGFuY2VNYXBbaWRdXG5cbiAgaWYgKCFpbmZvKSB7XG4gICAgLy8gSW5pdCBpbnN0YW5jZSBpbmZvLlxuICAgIGluZm8gPSBjaGVja1ZlcnNpb24oY29kZSkgfHwge31cbiAgICBpZiAoIWZyYW1ld29ya3NbaW5mby5mcmFtZXdvcmtdKSB7XG4gICAgICBpbmZvLmZyYW1ld29yayA9ICdXZWV4J1xuICAgIH1cblxuICAgIC8vIEluaXQgaW5zdGFuY2UgY29uZmlnLlxuICAgIGNvbmZpZyA9IEpTT04ucGFyc2UoSlNPTi5zdHJpbmdpZnkoY29uZmlnIHx8IHt9KSlcbiAgICBjb25maWcuYnVuZGxlVmVyc2lvbiA9IGluZm8udmVyc2lvblxuICAgIGNvbmZpZy5lbnYgPSBKU09OLnBhcnNlKEpTT04uc3RyaW5naWZ5KGdsb2JhbC5XWEVudmlyb25tZW50IHx8IHt9KSlcbiAgICBjb25zb2xlLmRlYnVnKGBbSlMgRnJhbWV3b3JrXSBjcmVhdGUgYW4gJHtpbmZvLmZyYW1ld29ya31AJHtjb25maWcuYnVuZGxlVmVyc2lvbn0gaW5zdGFuY2UgZnJvbSAke2NvbmZpZy5idW5kbGVWZXJzaW9ufWApXG5cbiAgICBjb25zdCBlbnYgPSB7XG4gICAgICBpbmZvLFxuICAgICAgY29uZmlnLFxuICAgICAgY3JlYXRlZDogRGF0ZS5ub3coKSxcbiAgICAgIGZyYW1ld29yazogaW5mby5mcmFtZXdvcmtcbiAgICB9XG4gICAgZW52LnNlcnZpY2VzID0gY3JlYXRlU2VydmljZXMoaWQsIGVudiwgcnVudGltZUNvbmZpZylcbiAgICBpbnN0YW5jZU1hcFtpZF0gPSBlbnZcblxuICAgIHJldHVybiBmcmFtZXdvcmtzW2luZm8uZnJhbWV3b3JrXS5jcmVhdGVJbnN0YW5jZShpZCwgY29kZSwgY29uZmlnLCBkYXRhLCBlbnYpXG4gIH1cbiAgcmV0dXJuIG5ldyBFcnJvcihgaW52YWxpZCBpbnN0YW5jZSBpZCBcIiR7aWR9XCJgKVxufVxuXG5jb25zdCBtZXRob2RzID0ge1xuICBjcmVhdGVJbnN0YW5jZSxcbiAgcmVnaXN0ZXJTZXJ2aWNlOiByZWdpc3RlcixcbiAgdW5yZWdpc3RlclNlcnZpY2U6IHVucmVnaXN0ZXJcbn1cblxuLyoqXG4gKiBSZWdpc3RlciBtZXRob2RzIHdoaWNoIGluaXQgZWFjaCBmcmFtZXdvcmtzLlxuICogQHBhcmFtIHtzdHJpbmd9IG1ldGhvZE5hbWVcbiAqL1xuZnVuY3Rpb24gZ2VuSW5pdCAobWV0aG9kTmFtZSkge1xuICBtZXRob2RzW21ldGhvZE5hbWVdID0gZnVuY3Rpb24gKC4uLmFyZ3MpIHtcbiAgICBpZiAobWV0aG9kTmFtZSA9PT0gJ3JlZ2lzdGVyQ29tcG9uZW50cycpIHtcbiAgICAgIGNoZWNrQ29tcG9uZW50TWV0aG9kcyhhcmdzWzBdKVxuICAgIH1cbiAgICBmb3IgKGNvbnN0IG5hbWUgaW4gZnJhbWV3b3Jrcykge1xuICAgICAgY29uc3QgZnJhbWV3b3JrID0gZnJhbWV3b3Jrc1tuYW1lXVxuICAgICAgaWYgKGZyYW1ld29yayAmJiBmcmFtZXdvcmtbbWV0aG9kTmFtZV0pIHtcbiAgICAgICAgZnJhbWV3b3JrW21ldGhvZE5hbWVdKC4uLmFyZ3MpXG4gICAgICB9XG4gICAgfVxuICB9XG59XG5cbmZ1bmN0aW9uIGNoZWNrQ29tcG9uZW50TWV0aG9kcyAoY29tcG9uZW50cykge1xuICBpZiAoQXJyYXkuaXNBcnJheShjb21wb25lbnRzKSkge1xuICAgIGNvbXBvbmVudHMuZm9yRWFjaCgobmFtZSkgPT4ge1xuICAgICAgaWYgKG5hbWUgJiYgbmFtZS50eXBlICYmIG5hbWUubWV0aG9kcykge1xuICAgICAgICByZWdpc3RlckVsZW1lbnQobmFtZS50eXBlLCBuYW1lLm1ldGhvZHMpXG4gICAgICB9XG4gICAgfSlcbiAgfVxufVxuXG4vKipcbiAqIFJlZ2lzdGVyIG1ldGhvZHMgd2hpY2ggd2lsbCBiZSBjYWxsZWQgZm9yIGVhY2ggaW5zdGFuY2UuXG4gKiBAcGFyYW0ge3N0cmluZ30gbWV0aG9kTmFtZVxuICovXG5mdW5jdGlvbiBnZW5JbnN0YW5jZSAobWV0aG9kTmFtZSkge1xuICBtZXRob2RzW21ldGhvZE5hbWVdID0gZnVuY3Rpb24gKC4uLmFyZ3MpIHtcbiAgICBjb25zdCBpZCA9IGFyZ3NbMF1cbiAgICBjb25zdCBpbmZvID0gaW5zdGFuY2VNYXBbaWRdXG4gICAgaWYgKGluZm8gJiYgZnJhbWV3b3Jrc1tpbmZvLmZyYW1ld29ya10pIHtcbiAgICAgIGNvbnN0IHJlc3VsdCA9IGZyYW1ld29ya3NbaW5mby5mcmFtZXdvcmtdW21ldGhvZE5hbWVdKC4uLmFyZ3MpXG5cbiAgICAgIC8vIExpZmVjeWNsZSBtZXRob2RzXG4gICAgICBpZiAobWV0aG9kTmFtZSA9PT0gJ3JlZnJlc2hJbnN0YW5jZScpIHtcbiAgICAgICAgc2VydmljZXMuZm9yRWFjaChzZXJ2aWNlID0+IHtcbiAgICAgICAgICBjb25zdCByZWZyZXNoID0gc2VydmljZS5vcHRpb25zLnJlZnJlc2hcbiAgICAgICAgICBpZiAocmVmcmVzaCkge1xuICAgICAgICAgICAgcmVmcmVzaChpZCwgeyBpbmZvLCBydW50aW1lOiBydW50aW1lQ29uZmlnIH0pXG4gICAgICAgICAgfVxuICAgICAgICB9KVxuICAgICAgfVxuICAgICAgZWxzZSBpZiAobWV0aG9kTmFtZSA9PT0gJ2Rlc3Ryb3lJbnN0YW5jZScpIHtcbiAgICAgICAgc2VydmljZXMuZm9yRWFjaChzZXJ2aWNlID0+IHtcbiAgICAgICAgICBjb25zdCBkZXN0cm95ID0gc2VydmljZS5vcHRpb25zLmRlc3Ryb3lcbiAgICAgICAgICBpZiAoZGVzdHJveSkge1xuICAgICAgICAgICAgZGVzdHJveShpZCwgeyBpbmZvLCBydW50aW1lOiBydW50aW1lQ29uZmlnIH0pXG4gICAgICAgICAgfVxuICAgICAgICB9KVxuICAgICAgICBkZWxldGUgaW5zdGFuY2VNYXBbaWRdXG4gICAgICB9XG5cbiAgICAgIHJldHVybiByZXN1bHRcbiAgICB9XG4gICAgcmV0dXJuIG5ldyBFcnJvcihgaW52YWxpZCBpbnN0YW5jZSBpZCBcIiR7aWR9XCJgKVxuICB9XG59XG5cbi8qKlxuICogQWRhcHQgc29tZSBsZWdhY3kgbWV0aG9kKHMpIHdoaWNoIHdpbGwgYmUgY2FsbGVkIGZvciBlYWNoIGluc3RhbmNlLiBUaGVzZVxuICogbWV0aG9kcyBzaG91bGQgYmUgZGVwcmVjYXRlZCBhbmQgcmVtb3ZlZCBsYXRlci5cbiAqIEBwYXJhbSB7c3RyaW5nfSBtZXRob2ROYW1lXG4gKiBAcGFyYW0ge3N0cmluZ30gbmF0aXZlTWV0aG9kTmFtZVxuICovXG5mdW5jdGlvbiBhZGFwdEluc3RhbmNlIChtZXRob2ROYW1lLCBuYXRpdmVNZXRob2ROYW1lKSB7XG4gIG1ldGhvZHNbbmF0aXZlTWV0aG9kTmFtZV0gPSBmdW5jdGlvbiAoLi4uYXJncykge1xuICAgIGNvbnN0IGlkID0gYXJnc1swXVxuICAgIGNvbnN0IGluZm8gPSBpbnN0YW5jZU1hcFtpZF1cbiAgICBpZiAoaW5mbyAmJiBmcmFtZXdvcmtzW2luZm8uZnJhbWV3b3JrXSkge1xuICAgICAgcmV0dXJuIGZyYW1ld29ya3NbaW5mby5mcmFtZXdvcmtdW21ldGhvZE5hbWVdKC4uLmFyZ3MpXG4gICAgfVxuICAgIHJldHVybiBuZXcgRXJyb3IoYGludmFsaWQgaW5zdGFuY2UgaWQgXCIke2lkfVwiYClcbiAgfVxufVxuXG5leHBvcnQgZGVmYXVsdCBmdW5jdGlvbiBpbml0IChjb25maWcpIHtcbiAgcnVudGltZUNvbmZpZyA9IGNvbmZpZyB8fCB7fVxuICBmcmFtZXdvcmtzID0gcnVudGltZUNvbmZpZy5mcmFtZXdvcmtzIHx8IHt9XG4gIGluaXRUYXNrSGFuZGxlcigpXG5cbiAgLy8gSW5pdCBlYWNoIGZyYW1ld29yayBieSBgaW5pdGAgbWV0aG9kIGFuZCBgY29uZmlnYCB3aGljaCBjb250YWlucyB0aHJlZVxuICAvLyB2aXJ0dWFsLURPTSBDbGFzczogYERvY3VtZW50YCwgYEVsZW1lbnRgICYgYENvbW1lbnRgLCBhbmQgYSBKUyBicmlkZ2UgbWV0aG9kOlxuICAvLyBgc2VuZFRhc2tzKC4uLmFyZ3MpYC5cbiAgZm9yIChjb25zdCBuYW1lIGluIGZyYW1ld29ya3MpIHtcbiAgICBjb25zdCBmcmFtZXdvcmsgPSBmcmFtZXdvcmtzW25hbWVdXG4gICAgZnJhbWV3b3JrLmluaXQoY29uZmlnKVxuICB9XG5cbiAgLy8gQHRvZG86IFRoZSBtZXRob2QgYHJlZ2lzdGVyTWV0aG9kc2Agd2lsbCBiZSByZS1kZXNpZ25lZCBvciByZW1vdmVkIGxhdGVyLlxuICA7IFsncmVnaXN0ZXJDb21wb25lbnRzJywgJ3JlZ2lzdGVyTW9kdWxlcycsICdyZWdpc3Rlck1ldGhvZHMnXS5mb3JFYWNoKGdlbkluaXQpXG5cbiAgOyBbJ2Rlc3Ryb3lJbnN0YW5jZScsICdyZWZyZXNoSW5zdGFuY2UnLCAncmVjZWl2ZVRhc2tzJywgJ2dldFJvb3QnXS5mb3JFYWNoKGdlbkluc3RhbmNlKVxuXG4gIGFkYXB0SW5zdGFuY2UoJ3JlY2VpdmVUYXNrcycsICdjYWxsSlMnKVxuXG4gIHJldHVybiBtZXRob2RzXG59XG4iLCIvKlxuICogTGljZW5zZWQgdG8gdGhlIEFwYWNoZSBTb2Z0d2FyZSBGb3VuZGF0aW9uIChBU0YpIHVuZGVyIG9uZVxuICogb3IgbW9yZSBjb250cmlidXRvciBsaWNlbnNlIGFncmVlbWVudHMuICBTZWUgdGhlIE5PVElDRSBmaWxlXG4gKiBkaXN0cmlidXRlZCB3aXRoIHRoaXMgd29yayBmb3IgYWRkaXRpb25hbCBpbmZvcm1hdGlvblxuICogcmVnYXJkaW5nIGNvcHlyaWdodCBvd25lcnNoaXAuICBUaGUgQVNGIGxpY2Vuc2VzIHRoaXMgZmlsZVxuICogdG8geW91IHVuZGVyIHRoZSBBcGFjaGUgTGljZW5zZSwgVmVyc2lvbiAyLjAgKHRoZVxuICogXCJMaWNlbnNlXCIpOyB5b3UgbWF5IG5vdCB1c2UgdGhpcyBmaWxlIGV4Y2VwdCBpbiBjb21wbGlhbmNlXG4gKiB3aXRoIHRoZSBMaWNlbnNlLiAgWW91IG1heSBvYnRhaW4gYSBjb3B5IG9mIHRoZSBMaWNlbnNlIGF0XG4gKlxuICogICBodHRwOi8vd3d3LmFwYWNoZS5vcmcvbGljZW5zZXMvTElDRU5TRS0yLjBcbiAqXG4gKiBVbmxlc3MgcmVxdWlyZWQgYnkgYXBwbGljYWJsZSBsYXcgb3IgYWdyZWVkIHRvIGluIHdyaXRpbmcsXG4gKiBzb2Z0d2FyZSBkaXN0cmlidXRlZCB1bmRlciB0aGUgTGljZW5zZSBpcyBkaXN0cmlidXRlZCBvbiBhblxuICogXCJBUyBJU1wiIEJBU0lTLCBXSVRIT1VUIFdBUlJBTlRJRVMgT1IgQ09ORElUSU9OUyBPRiBBTllcbiAqIEtJTkQsIGVpdGhlciBleHByZXNzIG9yIGltcGxpZWQuICBTZWUgdGhlIExpY2Vuc2UgZm9yIHRoZVxuICogc3BlY2lmaWMgbGFuZ3VhZ2UgZ292ZXJuaW5nIHBlcm1pc3Npb25zIGFuZCBsaW1pdGF0aW9uc1xuICogdW5kZXIgdGhlIExpY2Vuc2UuXG4gKi9cblxuLyoqXG4gKiBAZmlsZU92ZXJ2aWV3XG4gKiBWaXJ0dWFsLURPTSBDb21tZW50LlxuICovXG5cbmltcG9ydCBOb2RlIGZyb20gJy4vbm9kZSdcbmltcG9ydCB7IHVuaXF1ZUlkIH0gZnJvbSAnLi9vcGVyYXRpb24nXG5cbmV4cG9ydCBkZWZhdWx0IGZ1bmN0aW9uIENvbW1lbnQgKHZhbHVlKSB7XG4gIHRoaXMubm9kZVR5cGUgPSA4XG4gIHRoaXMubm9kZUlkID0gdW5pcXVlSWQoKVxuICB0aGlzLnJlZiA9IHRoaXMubm9kZUlkXG4gIHRoaXMudHlwZSA9ICdjb21tZW50J1xuICB0aGlzLnZhbHVlID0gdmFsdWVcbiAgdGhpcy5jaGlsZHJlbiA9IFtdXG4gIHRoaXMucHVyZUNoaWxkcmVuID0gW11cbn1cblxuQ29tbWVudC5wcm90b3R5cGUgPSBPYmplY3QuY3JlYXRlKE5vZGUucHJvdG90eXBlKVxuQ29tbWVudC5wcm90b3R5cGUuY29uc3RydWN0b3IgPSBDb21tZW50XG5cbi8qKlxuICogQ29udmVydCB0byBIVE1MIGNvbW1lbnQgc3RyaW5nLlxuICogQHJldHVybiB7c3Rpcm5nfSBodG1sXG4gKi9cbkNvbW1lbnQucHJvdG90eXBlLnRvU3RyaW5nID0gZnVuY3Rpb24gKCkge1xuICByZXR1cm4gJzwhLS0gJyArIHRoaXMudmFsdWUgKyAnIC0tPidcbn1cbiIsIi8qXG4gKiBMaWNlbnNlZCB0byB0aGUgQXBhY2hlIFNvZnR3YXJlIEZvdW5kYXRpb24gKEFTRikgdW5kZXIgb25lXG4gKiBvciBtb3JlIGNvbnRyaWJ1dG9yIGxpY2Vuc2UgYWdyZWVtZW50cy4gIFNlZSB0aGUgTk9USUNFIGZpbGVcbiAqIGRpc3RyaWJ1dGVkIHdpdGggdGhpcyB3b3JrIGZvciBhZGRpdGlvbmFsIGluZm9ybWF0aW9uXG4gKiByZWdhcmRpbmcgY29weXJpZ2h0IG93bmVyc2hpcC4gIFRoZSBBU0YgbGljZW5zZXMgdGhpcyBmaWxlXG4gKiB0byB5b3UgdW5kZXIgdGhlIEFwYWNoZSBMaWNlbnNlLCBWZXJzaW9uIDIuMCAodGhlXG4gKiBcIkxpY2Vuc2VcIik7IHlvdSBtYXkgbm90IHVzZSB0aGlzIGZpbGUgZXhjZXB0IGluIGNvbXBsaWFuY2VcbiAqIHdpdGggdGhlIExpY2Vuc2UuICBZb3UgbWF5IG9idGFpbiBhIGNvcHkgb2YgdGhlIExpY2Vuc2UgYXRcbiAqXG4gKiAgIGh0dHA6Ly93d3cuYXBhY2hlLm9yZy9saWNlbnNlcy9MSUNFTlNFLTIuMFxuICpcbiAqIFVubGVzcyByZXF1aXJlZCBieSBhcHBsaWNhYmxlIGxhdyBvciBhZ3JlZWQgdG8gaW4gd3JpdGluZyxcbiAqIHNvZnR3YXJlIGRpc3RyaWJ1dGVkIHVuZGVyIHRoZSBMaWNlbnNlIGlzIGRpc3RyaWJ1dGVkIG9uIGFuXG4gKiBcIkFTIElTXCIgQkFTSVMsIFdJVEhPVVQgV0FSUkFOVElFUyBPUiBDT05ESVRJT05TIE9GIEFOWVxuICogS0lORCwgZWl0aGVyIGV4cHJlc3Mgb3IgaW1wbGllZC4gIFNlZSB0aGUgTGljZW5zZSBmb3IgdGhlXG4gKiBzcGVjaWZpYyBsYW5ndWFnZSBnb3Zlcm5pbmcgcGVybWlzc2lvbnMgYW5kIGxpbWl0YXRpb25zXG4gKiB1bmRlciB0aGUgTGljZW5zZS5cbiAqL1xuXG4vKipcbiAqIEBmaWxlT3ZlcnZpZXdcbiAqIExpc3RlbiB2aXJ0dWFsLWRvbSBvcGVyYXRpb25zIGFuZCBjcmVhdGUgY29ycmVzcG9uZGluZyBhY3Rpb25zLlxuICovXG5cbmltcG9ydCAnLi4vc2hhcmVkL29iamVjdEFzc2lnbidcblxuZXhwb3J0IGRlZmF1bHQgZnVuY3Rpb24gTGlzdGVuZXIgKGlkLCBoYW5kbGVyKSB7XG4gIHRoaXMuaWQgPSBpZFxuICB0aGlzLmJhdGNoZWQgPSBmYWxzZVxuICB0aGlzLnVwZGF0ZXMgPSBbXVxuICBpZiAodHlwZW9mIGhhbmRsZXIgPT09ICdmdW5jdGlvbicpIHtcbiAgICBPYmplY3QuZGVmaW5lUHJvcGVydHkodGhpcywgJ2hhbmRsZXInLCB7XG4gICAgICBjb25maWd1cmFibGU6IHRydWUsXG4gICAgICBlbnVtZXJhYmxlOiB0cnVlLFxuICAgICAgd3JpdGFibGU6IHRydWUsXG4gICAgICB2YWx1ZTogaGFuZGxlclxuICAgIH0pXG4gIH1cbiAgZWxzZSB7XG4gICAgY29uc29sZS5lcnJvcignW0pTIFJ1bnRpbWVdIGludmFsaWQgcGFyYW1ldGVyLCBoYW5kbGVyIG11c3QgYmUgYSBmdW5jdGlvbicpXG4gIH1cbn1cblxuLyoqXG4gKiBDcmVhdGUgdGhlIGFjdGlvbiBvYmplY3QuXG4gKiBAcGFyYW0ge3N0cmluZ30gbmFtZVxuICogQHBhcmFtIHthcnJheX0gYXJndW1lbnRzXG4gKiBAcmV0dXJuIHtvYmplY3R9IGFjdGlvblxuICovXG5leHBvcnQgZnVuY3Rpb24gY3JlYXRlQWN0aW9uIChuYW1lLCBhcmdzID0gW10pIHtcbiAgcmV0dXJuIHsgbW9kdWxlOiAnZG9tJywgbWV0aG9kOiBuYW1lLCBhcmdzOiBhcmdzIH1cbn1cblxuT2JqZWN0LmFzc2lnbihMaXN0ZW5lci5wcm90b3R5cGUsIHtcbiAgLyoqXG4gICAqIFNlbmQgdGhlIFwiY3JlYXRlRmluaXNoXCIgc2lnbmFsLlxuICAgKiBAcGFyYW0ge2Z1bmN0aW9ufSBjYWxsYmFja1xuICAgKiBAcmV0dXJuIHt1bmRlZmluZWQgfCBudW1iZXJ9IHRoZSBzaWduYWwgc2VudCBieSBuYXRpdmVcbiAgICovXG4gIGNyZWF0ZUZpbmlzaCAoY2FsbGJhY2spIHtcbiAgICBjb25zdCBoYW5kbGVyID0gdGhpcy5oYW5kbGVyXG4gICAgcmV0dXJuIGhhbmRsZXIoW2NyZWF0ZUFjdGlvbignY3JlYXRlRmluaXNoJyldLCBjYWxsYmFjaylcbiAgfSxcblxuICAvKipcbiAgICogU2VuZCB0aGUgXCJ1cGRhdGVGaW5pc2hcIiBzaWduYWwuXG4gICAqIEBwYXJhbSB7ZnVuY3Rpb259IGNhbGxiYWNrXG4gICAqIEByZXR1cm4ge3VuZGVmaW5lZCB8IG51bWJlcn0gdGhlIHNpZ25hbCBzZW50IGJ5IG5hdGl2ZVxuICAgKi9cbiAgdXBkYXRlRmluaXNoIChjYWxsYmFjaykge1xuICAgIGNvbnN0IGhhbmRsZXIgPSB0aGlzLmhhbmRsZXJcbiAgICByZXR1cm4gaGFuZGxlcihbY3JlYXRlQWN0aW9uKCd1cGRhdGVGaW5pc2gnKV0sIGNhbGxiYWNrKVxuICB9LFxuXG4gIC8qKlxuICAgKiBTZW5kIHRoZSBcInJlZnJlc2hGaW5pc2hcIiBzaWduYWwuXG4gICAqIEBwYXJhbSB7ZnVuY3Rpb259IGNhbGxiYWNrXG4gICAqIEByZXR1cm4ge3VuZGVmaW5lZCB8IG51bWJlcn0gdGhlIHNpZ25hbCBzZW50IGJ5IG5hdGl2ZVxuICAgKi9cbiAgcmVmcmVzaEZpbmlzaCAoY2FsbGJhY2spIHtcbiAgICBjb25zdCBoYW5kbGVyID0gdGhpcy5oYW5kbGVyXG4gICAgcmV0dXJuIGhhbmRsZXIoW2NyZWF0ZUFjdGlvbigncmVmcmVzaEZpbmlzaCcpXSwgY2FsbGJhY2spXG4gIH0sXG5cbiAgLyoqXG4gICAqIFNlbmQgdGhlIFwiY3JlYXRlQm9keVwiIHNpZ25hbC5cbiAgICogQHBhcmFtIHtvYmplY3R9IGVsZW1lbnRcbiAgICogQHJldHVybiB7dW5kZWZpbmVkIHwgbnVtYmVyfSB0aGUgc2lnbmFsIHNlbnQgYnkgbmF0aXZlXG4gICAqL1xuICBjcmVhdGVCb2R5IChlbGVtZW50KSB7XG4gICAgY29uc3QgYm9keSA9IGVsZW1lbnQudG9KU09OKClcbiAgICBjb25zdCBjaGlsZHJlbiA9IGJvZHkuY2hpbGRyZW5cbiAgICBkZWxldGUgYm9keS5jaGlsZHJlblxuICAgIGNvbnN0IGFjdGlvbnMgPSBbY3JlYXRlQWN0aW9uKCdjcmVhdGVCb2R5JywgW2JvZHldKV1cbiAgICBpZiAoY2hpbGRyZW4pIHtcbiAgICAgIGFjdGlvbnMucHVzaC5hcHBseShhY3Rpb25zLCBjaGlsZHJlbi5tYXAoY2hpbGQgPT4ge1xuICAgICAgICByZXR1cm4gY3JlYXRlQWN0aW9uKCdhZGRFbGVtZW50JywgW2JvZHkucmVmLCBjaGlsZCwgLTFdKVxuICAgICAgfSkpXG4gICAgfVxuICAgIHJldHVybiB0aGlzLmFkZEFjdGlvbnMoYWN0aW9ucylcbiAgfSxcblxuICAvKipcbiAgICogU2VuZCB0aGUgXCJhZGRFbGVtZW50XCIgc2lnbmFsLlxuICAgKiBAcGFyYW0ge29iamVjdH0gZWxlbWVudFxuICAgKiBAcGFyYW0ge3N0aXJuZ30gcmVmZXJlbmNlIGlkXG4gICAqIEBwYXJhbSB7bnVtYmVyfSBpbmRleFxuICAgKiBAcmV0dXJuIHt1bmRlZmluZWQgfCBudW1iZXJ9IHRoZSBzaWduYWwgc2VudCBieSBuYXRpdmVcbiAgICovXG4gIGFkZEVsZW1lbnQgKGVsZW1lbnQsIHJlZiwgaW5kZXgpIHtcbiAgICBpZiAoIShpbmRleCA+PSAwKSkge1xuICAgICAgaW5kZXggPSAtMVxuICAgIH1cbiAgICByZXR1cm4gdGhpcy5hZGRBY3Rpb25zKGNyZWF0ZUFjdGlvbignYWRkRWxlbWVudCcsIFtyZWYsIGVsZW1lbnQudG9KU09OKCksIGluZGV4XSkpXG4gIH0sXG5cbiAgLyoqXG4gICAqIFNlbmQgdGhlIFwicmVtb3ZlRWxlbWVudFwiIHNpZ25hbC5cbiAgICogQHBhcmFtIHtzdGlybmd9IHJlZmVyZW5jZSBpZFxuICAgKiBAcmV0dXJuIHt1bmRlZmluZWQgfCBudW1iZXJ9IHRoZSBzaWduYWwgc2VudCBieSBuYXRpdmVcbiAgICovXG4gIHJlbW92ZUVsZW1lbnQgKHJlZikge1xuICAgIGlmIChBcnJheS5pc0FycmF5KHJlZikpIHtcbiAgICAgIGNvbnN0IGFjdGlvbnMgPSByZWYubWFwKChyKSA9PiBjcmVhdGVBY3Rpb24oJ3JlbW92ZUVsZW1lbnQnLCBbcl0pKVxuICAgICAgcmV0dXJuIHRoaXMuYWRkQWN0aW9ucyhhY3Rpb25zKVxuICAgIH1cbiAgICByZXR1cm4gdGhpcy5hZGRBY3Rpb25zKGNyZWF0ZUFjdGlvbigncmVtb3ZlRWxlbWVudCcsIFtyZWZdKSlcbiAgfSxcblxuICAvKipcbiAgICogU2VuZCB0aGUgXCJtb3ZlRWxlbWVudFwiIHNpZ25hbC5cbiAgICogQHBhcmFtIHtzdGlybmd9IHRhcmdldCByZWZlcmVuY2UgaWRcbiAgICogQHBhcmFtIHtzdGlybmd9IHBhcmVudCByZWZlcmVuY2UgaWRcbiAgICogQHBhcmFtIHtudW1iZXJ9IGluZGV4XG4gICAqIEByZXR1cm4ge3VuZGVmaW5lZCB8IG51bWJlcn0gdGhlIHNpZ25hbCBzZW50IGJ5IG5hdGl2ZVxuICAgKi9cbiAgbW92ZUVsZW1lbnQgKHRhcmdldFJlZiwgcGFyZW50UmVmLCBpbmRleCkge1xuICAgIHJldHVybiB0aGlzLmFkZEFjdGlvbnMoY3JlYXRlQWN0aW9uKCdtb3ZlRWxlbWVudCcsIFt0YXJnZXRSZWYsIHBhcmVudFJlZiwgaW5kZXhdKSlcbiAgfSxcblxuICAvKipcbiAgICogU2VuZCB0aGUgXCJ1cGRhdGVBdHRyc1wiIHNpZ25hbC5cbiAgICogQHBhcmFtIHtzdGlybmd9IHJlZmVyZW5jZSBpZFxuICAgKiBAcGFyYW0ge3N0aXJuZ30ga2V5XG4gICAqIEBwYXJhbSB7c3Rpcm5nfSB2YWx1ZVxuICAgKiBAcmV0dXJuIHt1bmRlZmluZWQgfCBudW1iZXJ9IHRoZSBzaWduYWwgc2VudCBieSBuYXRpdmVcbiAgICovXG4gIHNldEF0dHIgKHJlZiwga2V5LCB2YWx1ZSkge1xuICAgIGNvbnN0IHJlc3VsdCA9IHt9XG4gICAgcmVzdWx0W2tleV0gPSB2YWx1ZVxuICAgIHJldHVybiB0aGlzLmFkZEFjdGlvbnMoY3JlYXRlQWN0aW9uKCd1cGRhdGVBdHRycycsIFtyZWYsIHJlc3VsdF0pKVxuICB9LFxuXG4gIC8qKlxuICAgKiBTZW5kIHRoZSBcInVwZGF0ZVN0eWxlXCIgc2lnbmFsLCB1cGRhdGUgYSBzb2xlIHN0eWxlLlxuICAgKiBAcGFyYW0ge3N0aXJuZ30gcmVmZXJlbmNlIGlkXG4gICAqIEBwYXJhbSB7c3Rpcm5nfSBrZXlcbiAgICogQHBhcmFtIHtzdGlybmd9IHZhbHVlXG4gICAqIEByZXR1cm4ge3VuZGVmaW5lZCB8IG51bWJlcn0gdGhlIHNpZ25hbCBzZW50IGJ5IG5hdGl2ZVxuICAgKi9cbiAgc2V0U3R5bGUgKHJlZiwga2V5LCB2YWx1ZSkge1xuICAgIGNvbnN0IHJlc3VsdCA9IHt9XG4gICAgcmVzdWx0W2tleV0gPSB2YWx1ZVxuICAgIHJldHVybiB0aGlzLmFkZEFjdGlvbnMoY3JlYXRlQWN0aW9uKCd1cGRhdGVTdHlsZScsIFtyZWYsIHJlc3VsdF0pKVxuICB9LFxuXG4gIC8qKlxuICAgKiBTZW5kIHRoZSBcInVwZGF0ZVN0eWxlXCIgc2lnbmFsLlxuICAgKiBAcGFyYW0ge3N0aXJuZ30gcmVmZXJlbmNlIGlkXG4gICAqIEBwYXJhbSB7b2JqZWN0fSBzdHlsZVxuICAgKiBAcmV0dXJuIHt1bmRlZmluZWQgfCBudW1iZXJ9IHRoZSBzaWduYWwgc2VudCBieSBuYXRpdmVcbiAgICovXG4gIHNldFN0eWxlcyAocmVmLCBzdHlsZSkge1xuICAgIHJldHVybiB0aGlzLmFkZEFjdGlvbnMoY3JlYXRlQWN0aW9uKCd1cGRhdGVTdHlsZScsIFtyZWYsIHN0eWxlXSkpXG4gIH0sXG5cbiAgLyoqXG4gICAqIFNlbmQgdGhlIFwiYWRkRXZlbnRcIiBzaWduYWwuXG4gICAqIEBwYXJhbSB7c3Rpcm5nfSByZWZlcmVuY2UgaWRcbiAgICogQHBhcmFtIHtzdHJpbmd9IGV2ZW50IHR5cGVcbiAgICogQHJldHVybiB7dW5kZWZpbmVkIHwgbnVtYmVyfSB0aGUgc2lnbmFsIHNlbnQgYnkgbmF0aXZlXG4gICAqL1xuICBhZGRFdmVudCAocmVmLCB0eXBlKSB7XG4gICAgcmV0dXJuIHRoaXMuYWRkQWN0aW9ucyhjcmVhdGVBY3Rpb24oJ2FkZEV2ZW50JywgW3JlZiwgdHlwZV0pKVxuICB9LFxuXG4gIC8qKlxuICAgKiBTZW5kIHRoZSBcInJlbW92ZUV2ZW50XCIgc2lnbmFsLlxuICAgKiBAcGFyYW0ge3N0aXJuZ30gcmVmZXJlbmNlIGlkXG4gICAqIEBwYXJhbSB7c3RyaW5nfSBldmVudCB0eXBlXG4gICAqIEByZXR1cm4ge3VuZGVmaW5lZCB8IG51bWJlcn0gdGhlIHNpZ25hbCBzZW50IGJ5IG5hdGl2ZVxuICAgKi9cbiAgcmVtb3ZlRXZlbnQgKHJlZiwgdHlwZSkge1xuICAgIHJldHVybiB0aGlzLmFkZEFjdGlvbnMoY3JlYXRlQWN0aW9uKCdyZW1vdmVFdmVudCcsIFtyZWYsIHR5cGVdKSlcbiAgfSxcblxuICAvKipcbiAgICogRGVmYXVsdCBoYW5kbGVyLlxuICAgKiBAcGFyYW0ge29iamVjdCB8IGFycmF5fSBhY3Rpb25zXG4gICAqIEBwYXJhbSB7ZnVuY3Rpb259IGNhbGxiYWNrXG4gICAqIEByZXR1cm4ge30gYW55dGhpbmcgcmV0dXJuZWQgYnkgY2FsbGJhY2sgZnVuY3Rpb25cbiAgICovXG4gIGhhbmRsZXIgKGFjdGlvbnMsIGNiKSB7XG4gICAgcmV0dXJuIGNiICYmIGNiKClcbiAgfSxcblxuICAvKipcbiAgICogQWRkIGFjdGlvbnMgaW50byB1cGRhdGVzLlxuICAgKiBAcGFyYW0ge29iamVjdCB8IGFycmF5fSBhY3Rpb25zXG4gICAqIEByZXR1cm4ge3VuZGVmaW5lZCB8IG51bWJlcn0gdGhlIHNpZ25hbCBzZW50IGJ5IG5hdGl2ZVxuICAgKi9cbiAgYWRkQWN0aW9ucyAoYWN0aW9ucykge1xuICAgIGNvbnN0IHVwZGF0ZXMgPSB0aGlzLnVwZGF0ZXNcbiAgICBjb25zdCBoYW5kbGVyID0gdGhpcy5oYW5kbGVyXG5cbiAgICBpZiAoIUFycmF5LmlzQXJyYXkoYWN0aW9ucykpIHtcbiAgICAgIGFjdGlvbnMgPSBbYWN0aW9uc11cbiAgICB9XG5cbiAgICBpZiAodGhpcy5iYXRjaGVkKSB7XG4gICAgICB1cGRhdGVzLnB1c2guYXBwbHkodXBkYXRlcywgYWN0aW9ucylcbiAgICB9XG4gICAgZWxzZSB7XG4gICAgICByZXR1cm4gaGFuZGxlcihhY3Rpb25zKVxuICAgIH1cbiAgfVxufSlcbiIsIi8qXG4gKiBMaWNlbnNlZCB0byB0aGUgQXBhY2hlIFNvZnR3YXJlIEZvdW5kYXRpb24gKEFTRikgdW5kZXIgb25lXG4gKiBvciBtb3JlIGNvbnRyaWJ1dG9yIGxpY2Vuc2UgYWdyZWVtZW50cy4gIFNlZSB0aGUgTk9USUNFIGZpbGVcbiAqIGRpc3RyaWJ1dGVkIHdpdGggdGhpcyB3b3JrIGZvciBhZGRpdGlvbmFsIGluZm9ybWF0aW9uXG4gKiByZWdhcmRpbmcgY29weXJpZ2h0IG93bmVyc2hpcC4gIFRoZSBBU0YgbGljZW5zZXMgdGhpcyBmaWxlXG4gKiB0byB5b3UgdW5kZXIgdGhlIEFwYWNoZSBMaWNlbnNlLCBWZXJzaW9uIDIuMCAodGhlXG4gKiBcIkxpY2Vuc2VcIik7IHlvdSBtYXkgbm90IHVzZSB0aGlzIGZpbGUgZXhjZXB0IGluIGNvbXBsaWFuY2VcbiAqIHdpdGggdGhlIExpY2Vuc2UuICBZb3UgbWF5IG9idGFpbiBhIGNvcHkgb2YgdGhlIExpY2Vuc2UgYXRcbiAqXG4gKiAgIGh0dHA6Ly93d3cuYXBhY2hlLm9yZy9saWNlbnNlcy9MSUNFTlNFLTIuMFxuICpcbiAqIFVubGVzcyByZXF1aXJlZCBieSBhcHBsaWNhYmxlIGxhdyBvciBhZ3JlZWQgdG8gaW4gd3JpdGluZyxcbiAqIHNvZnR3YXJlIGRpc3RyaWJ1dGVkIHVuZGVyIHRoZSBMaWNlbnNlIGlzIGRpc3RyaWJ1dGVkIG9uIGFuXG4gKiBcIkFTIElTXCIgQkFTSVMsIFdJVEhPVVQgV0FSUkFOVElFUyBPUiBDT05ESVRJT05TIE9GIEFOWVxuICogS0lORCwgZWl0aGVyIGV4cHJlc3Mgb3IgaW1wbGllZC4gIFNlZSB0aGUgTGljZW5zZSBmb3IgdGhlXG4gKiBzcGVjaWZpYyBsYW5ndWFnZSBnb3Zlcm5pbmcgcGVybWlzc2lvbnMgYW5kIGxpbWl0YXRpb25zXG4gKiB1bmRlciB0aGUgTGljZW5zZS5cbiAqL1xuXG4vKipcbiAqIEBmaWxlT3ZlcnZpZXdcbiAqIFRhc2sgaGFuZGxlciBmb3IgY29tbXVuaWNhdGlvbiBiZXR3ZWVuIGphdmFzY3JpcHQgYW5kIG5hdGl2ZS5cbiAqL1xuXG5jb25zdCBoYW5kbGVyTWFwID0ge1xuICBjcmVhdGVCb2R5OiAnY2FsbENyZWF0ZUJvZHknLFxuICBhZGRFbGVtZW50OiAnY2FsbEFkZEVsZW1lbnQnLFxuICByZW1vdmVFbGVtZW50OiAnY2FsbFJlbW92ZUVsZW1lbnQnLFxuICBtb3ZlRWxlbWVudDogJ2NhbGxNb3ZlRWxlbWVudCcsXG4gIHVwZGF0ZUF0dHJzOiAnY2FsbFVwZGF0ZUF0dHJzJyxcbiAgdXBkYXRlU3R5bGU6ICdjYWxsVXBkYXRlU3R5bGUnLFxuICBhZGRFdmVudDogJ2NhbGxBZGRFdmVudCcsXG4gIHJlbW92ZUV2ZW50OiAnY2FsbFJlbW92ZUV2ZW50J1xufVxuXG4vKipcbiAqIENyZWF0ZSBhIHRhc2sgaGFuZGxlci5cbiAqIEBwYXJhbSB7c3RyaW5nfSBpZFxuICogQHBhcmFtIHtmdW5jdGlvbn0gaGFuZGxlclxuICogQHJldHVybiB7ZnVuY3Rpb259IHRhc2tIYW5kbGVyXG4gKi9cbmV4cG9ydCBmdW5jdGlvbiBjcmVhdGVIYW5kbGVyIChpZCwgaGFuZGxlcikge1xuICBjb25zdCBkZWZhdWx0SGFuZGxlciA9IGhhbmRsZXIgfHwgZ2xvYmFsLmNhbGxOYXRpdmVcblxuICAvKiBpc3RhbmJ1bCBpZ25vcmUgaWYgKi9cbiAgaWYgKHR5cGVvZiBkZWZhdWx0SGFuZGxlciAhPT0gJ2Z1bmN0aW9uJykge1xuICAgIGNvbnNvbGUuZXJyb3IoJ1tKUyBSdW50aW1lXSBubyBkZWZhdWx0IGhhbmRsZXInKVxuICB9XG5cbiAgcmV0dXJuIGZ1bmN0aW9uIHRhc2tIYW5kbGVyICh0YXNrcykge1xuICAgIC8qIGlzdGFuYnVsIGlnbm9yZSBpZiAqL1xuICAgIGlmICghQXJyYXkuaXNBcnJheSh0YXNrcykpIHtcbiAgICAgIHRhc2tzID0gW3Rhc2tzXVxuICAgIH1cbiAgICBmb3IgKGxldCBpID0gMDsgaSA8IHRhc2tzLmxlbmd0aDsgaSsrKSB7XG4gICAgICBjb25zdCByZXR1cm5WYWx1ZSA9IGRpc3BhdGNoVGFzayhpZCwgdGFza3NbaV0sIGRlZmF1bHRIYW5kbGVyKVxuICAgICAgaWYgKHJldHVyblZhbHVlID09PSAtMSkge1xuICAgICAgICByZXR1cm4gcmV0dXJuVmFsdWVcbiAgICAgIH1cbiAgICB9XG4gIH1cbn1cblxuLyoqXG4gKiBDaGVjayBpZiB0aGVyZSBpcyBhIGNvcnJlc3BvbmRpbmcgYXZhaWxhYmxlIGhhbmRsZXIgaW4gdGhlIGVudmlyb25tZW50LlxuICogQHBhcmFtIHtzdHJpbmd9IG1vZHVsZVxuICogQHBhcmFtIHtzdHJpbmd9IG1ldGhvZFxuICogQHJldHVybiB7Ym9vbGVhbn1cbiAqL1xuZnVuY3Rpb24gaGFzQXZhaWxhYmxlSGFuZGxlciAobW9kdWxlLCBtZXRob2QpIHtcbiAgcmV0dXJuIG1vZHVsZSA9PT0gJ2RvbSdcbiAgICAmJiBoYW5kbGVyTWFwW21ldGhvZF1cbiAgICAmJiB0eXBlb2YgZ2xvYmFsW2hhbmRsZXJNYXBbbWV0aG9kXV0gPT09ICdmdW5jdGlvbidcbn1cblxuLyoqXG4gKiBEaXNwYXRjaCB0aGUgdGFzayB0byB0aGUgc3BlY2lmaWVkIGhhbmRsZXIuXG4gKiBAcGFyYW0ge3N0cmluZ30gaWRcbiAqIEBwYXJhbSB7b2JqZWN0fSB0YXNrXG4gKiBAcGFyYW0ge2Z1bmN0aW9ufSBkZWZhdWx0SGFuZGxlclxuICogQHJldHVybiB7bnVtYmVyfSBzaWduYWwgcmV0dXJuZWQgZnJvbSBuYXRpdmVcbiAqL1xuZnVuY3Rpb24gZGlzcGF0Y2hUYXNrIChpZCwgdGFzaywgZGVmYXVsdEhhbmRsZXIpIHtcbiAgY29uc3QgeyBtb2R1bGUsIG1ldGhvZCwgYXJncyB9ID0gdGFza1xuXG4gIGlmIChoYXNBdmFpbGFibGVIYW5kbGVyKG1vZHVsZSwgbWV0aG9kKSkge1xuICAgIHJldHVybiBnbG9iYWxbaGFuZGxlck1hcFttZXRob2RdXShpZCwgLi4uYXJncywgJy0xJylcbiAgfVxuXG4gIHJldHVybiBkZWZhdWx0SGFuZGxlcihpZCwgW3Rhc2tdLCAnLTEnKVxufVxuIiwiLypcbiAqIExpY2Vuc2VkIHRvIHRoZSBBcGFjaGUgU29mdHdhcmUgRm91bmRhdGlvbiAoQVNGKSB1bmRlciBvbmVcbiAqIG9yIG1vcmUgY29udHJpYnV0b3IgbGljZW5zZSBhZ3JlZW1lbnRzLiAgU2VlIHRoZSBOT1RJQ0UgZmlsZVxuICogZGlzdHJpYnV0ZWQgd2l0aCB0aGlzIHdvcmsgZm9yIGFkZGl0aW9uYWwgaW5mb3JtYXRpb25cbiAqIHJlZ2FyZGluZyBjb3B5cmlnaHQgb3duZXJzaGlwLiAgVGhlIEFTRiBsaWNlbnNlcyB0aGlzIGZpbGVcbiAqIHRvIHlvdSB1bmRlciB0aGUgQXBhY2hlIExpY2Vuc2UsIFZlcnNpb24gMi4wICh0aGVcbiAqIFwiTGljZW5zZVwiKTsgeW91IG1heSBub3QgdXNlIHRoaXMgZmlsZSBleGNlcHQgaW4gY29tcGxpYW5jZVxuICogd2l0aCB0aGUgTGljZW5zZS4gIFlvdSBtYXkgb2J0YWluIGEgY29weSBvZiB0aGUgTGljZW5zZSBhdFxuICpcbiAqICAgaHR0cDovL3d3dy5hcGFjaGUub3JnL2xpY2Vuc2VzL0xJQ0VOU0UtMi4wXG4gKlxuICogVW5sZXNzIHJlcXVpcmVkIGJ5IGFwcGxpY2FibGUgbGF3IG9yIGFncmVlZCB0byBpbiB3cml0aW5nLFxuICogc29mdHdhcmUgZGlzdHJpYnV0ZWQgdW5kZXIgdGhlIExpY2Vuc2UgaXMgZGlzdHJpYnV0ZWQgb24gYW5cbiAqIFwiQVMgSVNcIiBCQVNJUywgV0lUSE9VVCBXQVJSQU5USUVTIE9SIENPTkRJVElPTlMgT0YgQU5ZXG4gKiBLSU5ELCBlaXRoZXIgZXhwcmVzcyBvciBpbXBsaWVkLiAgU2VlIHRoZSBMaWNlbnNlIGZvciB0aGVcbiAqIHNwZWNpZmljIGxhbmd1YWdlIGdvdmVybmluZyBwZXJtaXNzaW9ucyBhbmQgbGltaXRhdGlvbnNcbiAqIHVuZGVyIHRoZSBMaWNlbnNlLlxuICovXG5cbi8qKlxuICogQGZpbGVPdmVydmlld1xuICogVmlydHVhbC1ET00gRG9jdW1lbnQuXG4gKi9cblxuaW1wb3J0ICcuLi8uLi9zaGFyZWQvb2JqZWN0QXNzaWduJ1xuaW1wb3J0IENvbW1lbnQgZnJvbSAnLi9jb21tZW50J1xuaW1wb3J0IEVsZW1lbnQgZnJvbSAnLi9lbGVtZW50J1xuaW1wb3J0IExpc3RlbmVyIGZyb20gJy4uL2xpc3RlbmVyJ1xuaW1wb3J0IHsgVGFza0NlbnRlciB9IGZyb20gJy4uL3Rhc2stY2VudGVyJ1xuaW1wb3J0IHsgY3JlYXRlSGFuZGxlciB9IGZyb20gJy4uL2hhbmRsZXInXG5pbXBvcnQgeyBhZGREb2MsIHJlbW92ZURvYywgYXBwZW5kQm9keSwgc2V0Qm9keSB9IGZyb20gJy4vb3BlcmF0aW9uJ1xuXG5leHBvcnQgZGVmYXVsdCBmdW5jdGlvbiBEb2N1bWVudCAoaWQsIHVybCwgaGFuZGxlcikge1xuICBpZCA9IGlkID8gaWQudG9TdHJpbmcoKSA6ICcnXG4gIHRoaXMuaWQgPSBpZFxuICB0aGlzLlVSTCA9IHVybFxuXG4gIGFkZERvYyhpZCwgdGhpcylcbiAgdGhpcy5ub2RlTWFwID0ge31cbiAgY29uc3QgTCA9IERvY3VtZW50Lkxpc3RlbmVyIHx8IExpc3RlbmVyXG4gIHRoaXMubGlzdGVuZXIgPSBuZXcgTChpZCwgaGFuZGxlciB8fCBjcmVhdGVIYW5kbGVyKGlkLCBEb2N1bWVudC5oYW5kbGVyKSkgLy8gZGVwcmVjYXRlZFxuICB0aGlzLnRhc2tDZW50ZXIgPSBuZXcgVGFza0NlbnRlcihpZCwgaGFuZGxlciA/IChpZCwgLi4uYXJncykgPT4gaGFuZGxlciguLi5hcmdzKSA6IERvY3VtZW50LmhhbmRsZXIpXG4gIHRoaXMuY3JlYXRlRG9jdW1lbnRFbGVtZW50KClcbn1cblxuLy8gZGVmYXVsdCB0YXNrIGhhbmRsZXJcbkRvY3VtZW50LmhhbmRsZXIgPSBudWxsXG5cbi8qKlxuICogVXBkYXRlIGFsbCBjaGFuZ2VzIGZvciBhbiBlbGVtZW50LlxuICogQHBhcmFtIHtvYmplY3R9IGVsZW1lbnRcbiAqIEBwYXJhbSB7b2JqZWN0fSBjaGFuZ2VzXG4gKi9cbmZ1bmN0aW9uIHVwZGF0ZUVsZW1lbnQgKGVsLCBjaGFuZ2VzKSB7XG4gIGNvbnN0IGF0dHJzID0gY2hhbmdlcy5hdHRycyB8fCB7fVxuICBmb3IgKGNvbnN0IG5hbWUgaW4gYXR0cnMpIHtcbiAgICBlbC5zZXRBdHRyKG5hbWUsIGF0dHJzW25hbWVdLCB0cnVlKVxuICB9XG4gIGNvbnN0IHN0eWxlID0gY2hhbmdlcy5zdHlsZSB8fCB7fVxuICBmb3IgKGNvbnN0IG5hbWUgaW4gc3R5bGUpIHtcbiAgICBlbC5zZXRTdHlsZShuYW1lLCBzdHlsZVtuYW1lXSwgdHJ1ZSlcbiAgfVxufVxuXG5PYmplY3QuYXNzaWduKERvY3VtZW50LnByb3RvdHlwZSwge1xuICAvKipcbiAgICogR2V0IHRoZSBub2RlIGZyb20gbm9kZU1hcC5cbiAgICogQHBhcmFtIHtzdHJpbmd9IHJlZmVyZW5jZSBpZFxuICAgKiBAcmV0dXJuIHtvYmplY3R9IG5vZGVcbiAgICovXG4gIGdldFJlZiAocmVmKSB7XG4gICAgcmV0dXJuIHRoaXMubm9kZU1hcFtyZWZdXG4gIH0sXG5cbiAgLyoqXG4gICAqIFR1cm4gb24gYmF0Y2hlZCB1cGRhdGVzLlxuICAgKi9cbiAgb3BlbiAoKSB7XG4gICAgdGhpcy5saXN0ZW5lci5iYXRjaGVkID0gZmFsc2VcbiAgfSxcblxuICAvKipcbiAgICogVHVybiBvZmYgYmF0Y2hlZCB1cGRhdGVzLlxuICAgKi9cbiAgY2xvc2UgKCkge1xuICAgIHRoaXMubGlzdGVuZXIuYmF0Y2hlZCA9IHRydWVcbiAgfSxcblxuICAvKipcbiAgICogQ3JlYXRlIHRoZSBkb2N1bWVudCBlbGVtZW50LlxuICAgKiBAcmV0dXJuIHtvYmplY3R9IGRvY3VtZW50RWxlbWVudFxuICAgKi9cbiAgY3JlYXRlRG9jdW1lbnRFbGVtZW50ICgpIHtcbiAgICBpZiAoIXRoaXMuZG9jdW1lbnRFbGVtZW50KSB7XG4gICAgICBjb25zdCBlbCA9IG5ldyBFbGVtZW50KCdkb2N1bWVudCcpXG4gICAgICBlbC5kb2NJZCA9IHRoaXMuaWRcbiAgICAgIGVsLm93bmVyRG9jdW1lbnQgPSB0aGlzXG4gICAgICBlbC5yb2xlID0gJ2RvY3VtZW50RWxlbWVudCdcbiAgICAgIGVsLmRlcHRoID0gMFxuICAgICAgZWwucmVmID0gJ19kb2N1bWVudEVsZW1lbnQnXG4gICAgICB0aGlzLm5vZGVNYXAuX2RvY3VtZW50RWxlbWVudCA9IGVsXG4gICAgICB0aGlzLmRvY3VtZW50RWxlbWVudCA9IGVsXG5cbiAgICAgIE9iamVjdC5kZWZpbmVQcm9wZXJ0eShlbCwgJ2FwcGVuZENoaWxkJywge1xuICAgICAgICBjb25maWd1cmFibGU6IHRydWUsXG4gICAgICAgIGVudW1lcmFibGU6IHRydWUsXG4gICAgICAgIHdyaXRhYmxlOiB0cnVlLFxuICAgICAgICB2YWx1ZTogKG5vZGUpID0+IHtcbiAgICAgICAgICBhcHBlbmRCb2R5KHRoaXMsIG5vZGUpXG4gICAgICAgIH1cbiAgICAgIH0pXG5cbiAgICAgIE9iamVjdC5kZWZpbmVQcm9wZXJ0eShlbCwgJ2luc2VydEJlZm9yZScsIHtcbiAgICAgICAgY29uZmlndXJhYmxlOiB0cnVlLFxuICAgICAgICBlbnVtZXJhYmxlOiB0cnVlLFxuICAgICAgICB3cml0YWJsZTogdHJ1ZSxcbiAgICAgICAgdmFsdWU6IChub2RlLCBiZWZvcmUpID0+IHtcbiAgICAgICAgICBhcHBlbmRCb2R5KHRoaXMsIG5vZGUsIGJlZm9yZSlcbiAgICAgICAgfVxuICAgICAgfSlcbiAgICB9XG5cbiAgICByZXR1cm4gdGhpcy5kb2N1bWVudEVsZW1lbnRcbiAgfSxcblxuICAvKipcbiAgICogQ3JlYXRlIHRoZSBib2R5IGVsZW1lbnQuXG4gICAqIEBwYXJhbSB7c3RyaW5nfSB0eXBlXG4gICAqIEBwYXJhbSB7b2JqY3R9IHByb3BzXG4gICAqIEByZXR1cm4ge29iamVjdH0gYm9keSBlbGVtZW50XG4gICAqL1xuICBjcmVhdGVCb2R5ICh0eXBlLCBwcm9wcykge1xuICAgIGlmICghdGhpcy5ib2R5KSB7XG4gICAgICBjb25zdCBlbCA9IG5ldyBFbGVtZW50KHR5cGUsIHByb3BzKVxuICAgICAgc2V0Qm9keSh0aGlzLCBlbClcbiAgICB9XG5cbiAgICByZXR1cm4gdGhpcy5ib2R5XG4gIH0sXG5cbiAgLyoqXG4gICAqIENyZWF0ZSBhbiBlbGVtZW50LlxuICAgKiBAcGFyYW0ge3N0cmluZ30gdGFnTmFtZVxuICAgKiBAcGFyYW0ge29iamN0fSBwcm9wc1xuICAgKiBAcmV0dXJuIHtvYmplY3R9IGVsZW1lbnRcbiAgICovXG4gIGNyZWF0ZUVsZW1lbnQgKHRhZ05hbWUsIHByb3BzKSB7XG4gICAgcmV0dXJuIG5ldyBFbGVtZW50KHRhZ05hbWUsIHByb3BzKVxuICB9LFxuXG4gIC8qKlxuICAgKiBDcmVhdGUgYW4gY29tbWVudC5cbiAgICogQHBhcmFtIHtzdHJpbmd9IHRleHRcbiAgICogQHJldHVybiB7b2JqZWN0fSBjb21tZW50XG4gICAqL1xuICBjcmVhdGVDb21tZW50ICh0ZXh0KSB7XG4gICAgcmV0dXJuIG5ldyBDb21tZW50KHRleHQpXG4gIH0sXG5cbiAgLyoqXG4gICAqIEZpcmUgYW4gZXZlbnQgb24gc3BlY2lmaWVkIGVsZW1lbnQgbWFudWFsbHkuXG4gICAqIEBwYXJhbSB7b2JqZWN0fSBlbGVtZW50XG4gICAqIEBwYXJhbSB7c3RyaW5nfSBldmVudCB0eXBlXG4gICAqIEBwYXJhbSB7b2JqZWN0fSBldmVudCBvYmplY3RcbiAgICogQHBhcmFtIHtvYmplY3R9IGRvbSBjaGFuZ2VzXG4gICAqIEByZXR1cm4ge30gYW55dGhpbmcgcmV0dXJuZWQgYnkgaGFuZGxlciBmdW5jdGlvblxuICAgKi9cbiAgZmlyZUV2ZW50IChlbCwgdHlwZSwgZSwgZG9tQ2hhbmdlcykge1xuICAgIGlmICghZWwpIHtcbiAgICAgIHJldHVyblxuICAgIH1cbiAgICBlID0gZSB8fCB7fVxuICAgIGUudHlwZSA9IHR5cGVcbiAgICBlLnRhcmdldCA9IGVsXG4gICAgZS5jdXJyZW50VGFyZ2V0ID0gZWxcbiAgICBlLnRpbWVzdGFtcCA9IERhdGUubm93KClcbiAgICBpZiAoZG9tQ2hhbmdlcykge1xuICAgICAgdXBkYXRlRWxlbWVudChlbCwgZG9tQ2hhbmdlcylcbiAgICB9XG4gICAgY29uc3QgaXNCdWJibGUgPSB0aGlzLmdldFJlZignX3Jvb3QnKS5hdHRyWydidWJibGUnXSA9PT0gJ3RydWUnXG4gICAgcmV0dXJuIGVsLmZpcmVFdmVudCh0eXBlLCBlLCBpc0J1YmJsZSlcbiAgfSxcblxuICAvKipcbiAgICogRGVzdHJveSBjdXJyZW50IGRvY3VtZW50LCBhbmQgcmVtb3ZlIGl0c2VsZiBmb3JtIGRvY01hcC5cbiAgICovXG4gIGRlc3Ryb3kgKCkge1xuICAgIGRlbGV0ZSB0aGlzLmxpc3RlbmVyXG4gICAgZGVsZXRlIHRoaXMubm9kZU1hcFxuICAgIHJlbW92ZURvYyh0aGlzLmlkKVxuICB9XG59KVxuIiwiLypcbiAqIExpY2Vuc2VkIHRvIHRoZSBBcGFjaGUgU29mdHdhcmUgRm91bmRhdGlvbiAoQVNGKSB1bmRlciBvbmVcbiAqIG9yIG1vcmUgY29udHJpYnV0b3IgbGljZW5zZSBhZ3JlZW1lbnRzLiAgU2VlIHRoZSBOT1RJQ0UgZmlsZVxuICogZGlzdHJpYnV0ZWQgd2l0aCB0aGlzIHdvcmsgZm9yIGFkZGl0aW9uYWwgaW5mb3JtYXRpb25cbiAqIHJlZ2FyZGluZyBjb3B5cmlnaHQgb3duZXJzaGlwLiAgVGhlIEFTRiBsaWNlbnNlcyB0aGlzIGZpbGVcbiAqIHRvIHlvdSB1bmRlciB0aGUgQXBhY2hlIExpY2Vuc2UsIFZlcnNpb24gMi4wICh0aGVcbiAqIFwiTGljZW5zZVwiKTsgeW91IG1heSBub3QgdXNlIHRoaXMgZmlsZSBleGNlcHQgaW4gY29tcGxpYW5jZVxuICogd2l0aCB0aGUgTGljZW5zZS4gIFlvdSBtYXkgb2J0YWluIGEgY29weSBvZiB0aGUgTGljZW5zZSBhdFxuICpcbiAqICAgaHR0cDovL3d3dy5hcGFjaGUub3JnL2xpY2Vuc2VzL0xJQ0VOU0UtMi4wXG4gKlxuICogVW5sZXNzIHJlcXVpcmVkIGJ5IGFwcGxpY2FibGUgbGF3IG9yIGFncmVlZCB0byBpbiB3cml0aW5nLFxuICogc29mdHdhcmUgZGlzdHJpYnV0ZWQgdW5kZXIgdGhlIExpY2Vuc2UgaXMgZGlzdHJpYnV0ZWQgb24gYW5cbiAqIFwiQVMgSVNcIiBCQVNJUywgV0lUSE9VVCBXQVJSQU5USUVTIE9SIENPTkRJVElPTlMgT0YgQU5ZXG4gKiBLSU5ELCBlaXRoZXIgZXhwcmVzcyBvciBpbXBsaWVkLiAgU2VlIHRoZSBMaWNlbnNlIGZvciB0aGVcbiAqIHNwZWNpZmljIGxhbmd1YWdlIGdvdmVybmluZyBwZXJtaXNzaW9ucyBhbmQgbGltaXRhdGlvbnNcbiAqIHVuZGVyIHRoZSBMaWNlbnNlLlxuICovXG5pbXBvcnQgTm9kZSBmcm9tICcuL25vZGUnXG5pbXBvcnQgRWxlbWVudCBmcm9tICcuL2VsZW1lbnQnXG5pbXBvcnQgQ29tbWVudCBmcm9tICcuL2NvbW1lbnQnXG5pbXBvcnQgRG9jdW1lbnQgZnJvbSAnLi9kb2N1bWVudCdcblxuZXhwb3J0IHtcbiAgZWxlbWVudFR5cGVzLFxuICByZWdpc3RlckVsZW1lbnQsXG4gIGNsZWFyRWxlbWVudFR5cGVzXG59IGZyb20gJy4vZWxlbWVudC10eXBlcydcblxuZXhwb3J0IHtcbiAgRG9jdW1lbnQsXG4gIE5vZGUsXG4gIEVsZW1lbnQsXG4gIENvbW1lbnRcbn1cbiIsIi8qXG4gKiBMaWNlbnNlZCB0byB0aGUgQXBhY2hlIFNvZnR3YXJlIEZvdW5kYXRpb24gKEFTRikgdW5kZXIgb25lXG4gKiBvciBtb3JlIGNvbnRyaWJ1dG9yIGxpY2Vuc2UgYWdyZWVtZW50cy4gIFNlZSB0aGUgTk9USUNFIGZpbGVcbiAqIGRpc3RyaWJ1dGVkIHdpdGggdGhpcyB3b3JrIGZvciBhZGRpdGlvbmFsIGluZm9ybWF0aW9uXG4gKiByZWdhcmRpbmcgY29weXJpZ2h0IG93bmVyc2hpcC4gIFRoZSBBU0YgbGljZW5zZXMgdGhpcyBmaWxlXG4gKiB0byB5b3UgdW5kZXIgdGhlIEFwYWNoZSBMaWNlbnNlLCBWZXJzaW9uIDIuMCAodGhlXG4gKiBcIkxpY2Vuc2VcIik7IHlvdSBtYXkgbm90IHVzZSB0aGlzIGZpbGUgZXhjZXB0IGluIGNvbXBsaWFuY2VcbiAqIHdpdGggdGhlIExpY2Vuc2UuICBZb3UgbWF5IG9idGFpbiBhIGNvcHkgb2YgdGhlIExpY2Vuc2UgYXRcbiAqXG4gKiAgIGh0dHA6Ly93d3cuYXBhY2hlLm9yZy9saWNlbnNlcy9MSUNFTlNFLTIuMFxuICpcbiAqIFVubGVzcyByZXF1aXJlZCBieSBhcHBsaWNhYmxlIGxhdyBvciBhZ3JlZWQgdG8gaW4gd3JpdGluZyxcbiAqIHNvZnR3YXJlIGRpc3RyaWJ1dGVkIHVuZGVyIHRoZSBMaWNlbnNlIGlzIGRpc3RyaWJ1dGVkIG9uIGFuXG4gKiBcIkFTIElTXCIgQkFTSVMsIFdJVEhPVVQgV0FSUkFOVElFUyBPUiBDT05ESVRJT05TIE9GIEFOWVxuICogS0lORCwgZWl0aGVyIGV4cHJlc3Mgb3IgaW1wbGllZC4gIFNlZSB0aGUgTGljZW5zZSBmb3IgdGhlXG4gKiBzcGVjaWZpYyBsYW5ndWFnZSBnb3Zlcm5pbmcgcGVybWlzc2lvbnMgYW5kIGxpbWl0YXRpb25zXG4gKiB1bmRlciB0aGUgTGljZW5zZS5cbiAqL1xuaW1wb3J0IHsgRG9jdW1lbnQsIEVsZW1lbnQsIENvbW1lbnQgfSBmcm9tICcuL3Zkb20nXG5pbXBvcnQgTGlzdGVuZXIgZnJvbSAnLi9saXN0ZW5lcidcbmltcG9ydCB7IFRhc2tDZW50ZXIgfSBmcm9tICcuL3Rhc2stY2VudGVyJ1xuXG5jb25zdCBjb25maWcgPSB7XG4gIERvY3VtZW50LCBFbGVtZW50LCBDb21tZW50LCBMaXN0ZW5lcixcbiAgVGFza0NlbnRlcixcbiAgc2VuZFRhc2tzICguLi5hcmdzKSB7XG4gICAgcmV0dXJuIGdsb2JhbC5jYWxsTmF0aXZlKC4uLmFyZ3MpXG4gIH1cbn1cblxuRG9jdW1lbnQuaGFuZGxlciA9IGNvbmZpZy5zZW5kVGFza3NcblxuZXhwb3J0IGRlZmF1bHQgY29uZmlnXG4iLCIvKlxuICogTGljZW5zZWQgdG8gdGhlIEFwYWNoZSBTb2Z0d2FyZSBGb3VuZGF0aW9uIChBU0YpIHVuZGVyIG9uZVxuICogb3IgbW9yZSBjb250cmlidXRvciBsaWNlbnNlIGFncmVlbWVudHMuICBTZWUgdGhlIE5PVElDRSBmaWxlXG4gKiBkaXN0cmlidXRlZCB3aXRoIHRoaXMgd29yayBmb3IgYWRkaXRpb25hbCBpbmZvcm1hdGlvblxuICogcmVnYXJkaW5nIGNvcHlyaWdodCBvd25lcnNoaXAuICBUaGUgQVNGIGxpY2Vuc2VzIHRoaXMgZmlsZVxuICogdG8geW91IHVuZGVyIHRoZSBBcGFjaGUgTGljZW5zZSwgVmVyc2lvbiAyLjAgKHRoZVxuICogXCJMaWNlbnNlXCIpOyB5b3UgbWF5IG5vdCB1c2UgdGhpcyBmaWxlIGV4Y2VwdCBpbiBjb21wbGlhbmNlXG4gKiB3aXRoIHRoZSBMaWNlbnNlLiAgWW91IG1heSBvYnRhaW4gYSBjb3B5IG9mIHRoZSBMaWNlbnNlIGF0XG4gKlxuICogICBodHRwOi8vd3d3LmFwYWNoZS5vcmcvbGljZW5zZXMvTElDRU5TRS0yLjBcbiAqXG4gKiBVbmxlc3MgcmVxdWlyZWQgYnkgYXBwbGljYWJsZSBsYXcgb3IgYWdyZWVkIHRvIGluIHdyaXRpbmcsXG4gKiBzb2Z0d2FyZSBkaXN0cmlidXRlZCB1bmRlciB0aGUgTGljZW5zZSBpcyBkaXN0cmlidXRlZCBvbiBhblxuICogXCJBUyBJU1wiIEJBU0lTLCBXSVRIT1VUIFdBUlJBTlRJRVMgT1IgQ09ORElUSU9OUyBPRiBBTllcbiAqIEtJTkQsIGVpdGhlciBleHByZXNzIG9yIGltcGxpZWQuICBTZWUgdGhlIExpY2Vuc2UgZm9yIHRoZVxuICogc3BlY2lmaWMgbGFuZ3VhZ2UgZ292ZXJuaW5nIHBlcm1pc3Npb25zIGFuZCBsaW1pdGF0aW9uc1xuICogdW5kZXIgdGhlIExpY2Vuc2UuXG4gKi9cblxuLyoqXG4gKiBAZmlsZU92ZXJ2aWV3XG4gKiBSZWdpc3RlciBmcmFtZXdvcmsocykgaW4gSlMgcnVudGltZS4gV2VleCBzdXBwbHkgdHdvIGxheWVycyBmb3IgM3JkLXBhcnR5XG4gKiBmcmFtZXdvcmsocyk6IG9uZSBpcyB0aGUgaW5zdGFuY2UgbWFuYWdlbWVudCBsYXllciwgYW5vdGhlciBpcyB0aGVcbiAqIHZpcnR1YWwtRE9NIGxheWVyLlxuICovXG5cbmltcG9ydCAqIGFzIHNoYXJlZCBmcm9tICcuLi9zaGFyZWQnXG5cbmltcG9ydCBpbml0IGZyb20gJy4vaW5pdCdcbmltcG9ydCBjb25maWcgZnJvbSAnLi9jb25maWcnXG5cbmltcG9ydCB7XG4gIHJlZ2lzdGVyLFxuICB1bnJlZ2lzdGVyLFxuICBoYXNcbn0gZnJvbSAnLi9zZXJ2aWNlJ1xuXG4vKiBpc3RhbmJ1bCBpZ25vcmUgbmV4dCAqL1xuZnVuY3Rpb24gZnJlZXplUHJvdG90eXBlICgpIHtcbiAgc2hhcmVkLmZyZWV6ZVByb3RvdHlwZSgpXG5cbiAgT2JqZWN0LmZyZWV6ZShjb25maWcuRWxlbWVudClcbiAgT2JqZWN0LmZyZWV6ZShjb25maWcuQ29tbWVudClcbiAgT2JqZWN0LmZyZWV6ZShjb25maWcuTGlzdGVuZXIpXG4gIE9iamVjdC5mcmVlemUoY29uZmlnLkRvY3VtZW50LnByb3RvdHlwZSlcbiAgT2JqZWN0LmZyZWV6ZShjb25maWcuRWxlbWVudC5wcm90b3R5cGUpXG4gIE9iamVjdC5mcmVlemUoY29uZmlnLkNvbW1lbnQucHJvdG90eXBlKVxuICBPYmplY3QuZnJlZXplKGNvbmZpZy5MaXN0ZW5lci5wcm90b3R5cGUpXG59XG5cbmV4cG9ydCBkZWZhdWx0IHtcbiAgc2V0TmF0aXZlQ29uc29sZTogc2hhcmVkLnNldE5hdGl2ZUNvbnNvbGUsXG4gIHJlc2V0TmF0aXZlQ29uc29sZTogc2hhcmVkLnJlc2V0TmF0aXZlQ29uc29sZSxcbiAgc2V0TmF0aXZlVGltZXI6IHNoYXJlZC5zZXROYXRpdmVUaW1lcixcbiAgcmVzZXROYXRpdmVUaW1lcjogc2hhcmVkLnJlc2V0TmF0aXZlVGltZXIsXG4gIHNlcnZpY2U6IHsgcmVnaXN0ZXIsIHVucmVnaXN0ZXIsIGhhcyB9LFxuICBmcmVlemVQcm90b3R5cGUsXG4gIGluaXQsXG4gIGNvbmZpZ1xufVxuIl0sIm5hbWVzIjpbInJlcXVpcmUkJDAiLCJpc09iamVjdCIsInJlcXVpcmUkJDEiLCJkb2N1bWVudCIsInJlcXVpcmUkJDIiLCJyZXF1aXJlJCQzIiwiZFAiLCJyZXF1aXJlJCQ0IiwiZ2xvYmFsIiwiJGV4cG9ydCIsInRvU3RyaW5nIiwiSU9iamVjdCIsInRvSW50ZWdlciIsIm1pbiIsInRvSU9iamVjdCIsImRlZmluZWQiLCJyZXF1aXJlJCQ1IiwiYXJndW1lbnRzIiwiY29mIiwiYW5PYmplY3QiLCJnZXRLZXlzIiwiZW51bUJ1Z0tleXMiLCJJRV9QUk9UTyIsIlBST1RPVFlQRSIsImhhcyIsIlRBRyIsImNyZWF0ZSIsInNldFRvU3RyaW5nVGFnIiwidG9PYmplY3QiLCJyZXF1aXJlJCQ5IiwicmVxdWlyZSQkOCIsInJlZGVmaW5lIiwicmVxdWlyZSQkNyIsImhpZGUiLCJyZXF1aXJlJCQ2IiwiSXRlcmF0b3JzIiwiSVRFUkFUT1IiLCJBcnJheVByb3RvIiwiY2xhc3NvZiIsImFGdW5jdGlvbiIsImN0eCIsInByb2Nlc3MiLCJQcm9taXNlIiwiaXNOb2RlIiwiU1BFQ0lFUyIsIkxJQlJBUlkiLCJyZXF1aXJlJCQxNyIsInJlcXVpcmUkJDE2IiwicmVxdWlyZSQkMTUiLCJyZXF1aXJlJCQxNCIsInJlcXVpcmUkJDEzIiwicmVxdWlyZSQkMTIiLCJyZXF1aXJlJCQxMSIsInJlcXVpcmUkJDEwIiwiVHlwZUVycm9yIiwiY29uc3QiLCJsZXQiLCJmcmVlemVQcm90b3R5cGUiLCJFbGVtZW50IiwidGFza0NlbnRlciIsInB1cmVCZWZvcmUiLCJpbmRleCIsInRoaXMiLCJpbml0IiwiaW5pdFRhc2tIYW5kbGVyIiwibmFtZSIsInNoYXJlZC5mcmVlemVQcm90b3R5cGUiLCJzaGFyZWQuc2V0TmF0aXZlQ29uc29sZSIsInNoYXJlZC5yZXNldE5hdGl2ZUNvbnNvbGUiLCJzaGFyZWQuc2V0TmF0aXZlVGltZXIiLCJzaGFyZWQucmVzZXROYXRpdmVUaW1lciJdLCJtYXBwaW5ncyI6Ijs7Ozs7Ozs7Ozs7QUFBQTs7Ozs7Ozs7Ozs7Ozs7Ozs7Ozs7Ozs7OztBQXlCQSxJQUFJLENBQUMsS0FBSyxDQUFDLElBQUksRUFBRTtFQUNmLEtBQUssQ0FBQyxJQUFJLEdBQUcsQ0FBQyxXQUFXO0lBQ3ZCLElBQUksS0FBSyxHQUFHLE1BQU0sQ0FBQyxTQUFTLENBQUMsUUFBUSxDQUFDO0lBQ3RDLElBQUksVUFBVSxHQUFHLFNBQVMsRUFBRSxFQUFFO01BQzVCLE9BQU8sT0FBTyxFQUFFLEtBQUssVUFBVSxJQUFJLEtBQUssQ0FBQyxJQUFJLENBQUMsRUFBRSxDQUFDLEtBQUssbUJBQW1CLENBQUM7S0FDM0UsQ0FBQztJQUNGLElBQUksU0FBUyxHQUFHLFNBQVMsS0FBSyxFQUFFO01BQzlCLElBQUksTUFBTSxHQUFHLE1BQU0sQ0FBQyxLQUFLLENBQUMsQ0FBQztNQUMzQixJQUFJLEtBQUssQ0FBQyxNQUFNLENBQUMsRUFBRTtRQUNqQixPQUFPLENBQUMsQ0FBQztPQUNWO01BQ0QsSUFBSSxNQUFNLEtBQUssQ0FBQyxJQUFJLENBQUMsUUFBUSxDQUFDLE1BQU0sQ0FBQyxFQUFFO1FBQ3JDLE9BQU8sTUFBTSxDQUFDO09BQ2Y7TUFDRCxPQUFPLENBQUMsTUFBTSxHQUFHLENBQUMsR0FBRyxDQUFDLEdBQUcsQ0FBQyxDQUFDLENBQUMsR0FBRyxJQUFJLENBQUMsS0FBSyxDQUFDLElBQUksQ0FBQyxHQUFHLENBQUMsTUFBTSxDQUFDLENBQUMsQ0FBQztLQUM3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LENBQUM7RUFDNUUsSUFBSSxNQUFNLEdBQUcsUUFBUSxHQUFHLFVBQVUsRUFBRSxPQUFPLFFBQVEsQ0FBQyxFQUFFLEdBQUcsU0FBUyxDQUFDLFFBQVEsQ0FBQztNQUN4RSxDQUFDLFFBQVEsR0FBRyxDQUFDLEVBQUUsRUFBRSxJQUFJLEVBQUUsT0FBTyxHQUFHLENBQUMsR0FBRyxDQUFDLENBQUM7TUFDdkMsS0FBSyxJQUFJLENBQUM7TUFDVixNQUFNLEVBQUUsSUFBSSxFQUFFLFFBQVEsRUFBRSxNQUFNLENBQUM7RUFDbkMsR0FBRyxPQUFPLE1BQU0sSUFBSSxVQUFVLENBQUMsRUFBQSxNQUFNLFNBQVMsQ0FBQyxRQUFRLEdBQUcsbUJBQW1CLENBQUMsQ0FBQyxFQUFBOztFQUUvRSxHQUFHLFdBQVcsQ0FBQyxNQUFNLENBQUMsQ0FBQyxFQUFBLElBQUksTUFBTSxHQUFHLFFBQVEsQ0FBQyxRQUFRLENBQUMsTUFBTSxDQUFDLEVBQUUsTUFBTSxHQUFHLEtBQUssRUFBRSxLQUFLLEVBQUUsQ0FBQztJQUNyRixNQUFNLEdBQUcsT0FBTyxHQUFHLENBQUMsQ0FBQyxRQUFRLENBQUMsSUFBSSxHQUFHLFFBQVEsQ0FBQyxLQUFLLENBQUMsQ0FBQyxDQUFDLENBQUMsQ0FBQyxFQUFFLElBQUksQ0FBQyxDQUFDLENBQUMsQ0FBQyxHQUFHLENBQUMsQ0FBQyxRQUFRLENBQUMsS0FBSyxDQUFDLENBQUMsQ0FBQztJQUN4RixHQUFHLE1BQU0sS0FBSyxLQUFLLElBQUksTUFBTSxLQUFLLE1BQU0sQ0FBQyxFQUFBLE9BQU8sTUFBTSxDQUFDLEVBQUE7R0FDeEQsRUFBQSxNQUFNLEVBQUEsSUFBSSxRQUFRLEdBQUcsTUFBTSxDQUFDLElBQUksQ0FBQyxRQUFRLENBQUMsRUFBRSxDQUFDLENBQUMsSUFBSSxHQUFHLFFBQVEsQ0FBQyxJQUFJLEVBQUUsRUFBRSxJQUFJLEdBQUc7SUFDNUUsTUFBTSxHQUFHLElBQUksQ0FBQyxRQUFRLEVBQUUsQ0FBQyxFQUFFLElBQUksQ0FBQyxLQUFLLEVBQUUsT0FBTyxDQUFDLENB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
