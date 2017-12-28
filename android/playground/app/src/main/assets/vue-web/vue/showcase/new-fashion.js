// NOTE: for vue2.0 and platform:web only.

/******/ (function(modules) { // webpackBootstrap
/******/ 	// The module cache
/******/ 	var installedModules = {};

/******/ 	// The require function
/******/ 	function __webpack_require__(moduleId) {

/******/ 		// Check if module is in cache
/******/ 		if(installedModules[moduleId])
/******/ 			return installedModules[moduleId].exports;

/******/ 		// Create a new module (and put it into the cache)
/******/ 		var module = installedModules[moduleId] = {
/******/ 			exports: {},
/******/ 			id: moduleId,
/******/ 			loaded: false
/******/ 		};

/******/ 		// Execute the module function
/******/ 		modules[moduleId].call(module.exports, module, module.exports, __webpack_require__);

/******/ 		// Flag the module as loaded
/******/ 		module.loaded = true;

/******/ 		// Return the exports of the module
/******/ 		return module.exports;
/******/ 	}


/******/ 	// expose the modules object (__webpack_modules__)
/******/ 	__webpack_require__.m = modules;

/******/ 	// expose the module cache
/******/ 	__webpack_require__.c = installedModules;

/******/ 	// __webpack_public_path__
/******/ 	__webpack_require__.p = "";

/******/ 	// Load entry module and return exports
/******/ 	return __webpack_require__(0);
/******/ })
/************************************************************************/
/******/ ([
/* 0 */
/***/ (function(module, exports, __webpack_require__) {

	var App = __webpack_require__(227)
	App.el = '#root'
	new Vue(App)


/***/ }),
/* 1 */,
/* 2 */,
/* 3 */,
/* 4 */
/***/ (function(module, exports) {

	/*
		MIT License http://www.opensource.org/licenses/mit-license.php
		Author Tobias Koppers @sokra
	*/
	// css base code, injected by the css-loader
	module.exports = function() {
		var list = [];

		// return the list of modules as css string
		list.toString = function toString() {
			var result = [];
			for(var i = 0; i < this.length; i++) {
				var item = this[i];
				if(item[2]) {
					result.push("@media " + item[2] + "{" + item[1] + "}");
				} else {
					result.push(item[1]);
				}
			}
			return result.join("");
		};

		// import a list of modules into the list
		list.i = function(modules, mediaQuery) {
			if(typeof modules === "string")
				modules = [[null, modules, ""]];
			var alreadyImportedModules = {};
			for(var i = 0; i < this.length; i++) {
				var id = this[i][0];
				if(typeof id === "number")
					alreadyImportedModules[id] = true;
			}
			for(i = 0; i < modules.length; i++) {
				var item = modules[i];
				// skip already imported module
				// this implementation is not 100% perfect for weird media query combinations
				//  when a module is imported multiple times with different media queries.
				//  I hope this will never occur (Hey this way we have smaller bundles)
				if(typeof item[0] !== "number" || !alreadyImportedModules[item[0]]) {
					if(mediaQuery && !item[2]) {
						item[2] = mediaQuery;
					} else if(mediaQuery) {
						item[2] = "(" + item[2] + ") and (" + mediaQuery + ")";
					}
					list.push(item);
				}
			}
		};
		return list;
	};


/***/ }),
/* 5 */
/***/ (function(module, exports, __webpack_require__) {

	/*
	  MIT License http://www.opensource.org/licenses/mit-license.php
	  Author Tobias Koppers @sokra
	  Modified by Evan You @yyx990803
	*/

	var hasDocument = typeof document !== 'undefined'

	if (false) {
	  if (!hasDocument) {
	    throw new Error(
	    'vue-style-loader cannot be used in a non-browser environment. ' +
	    "Use { target: 'node' } in your Webpack config to indicate a server-rendering environment."
	  ) }
	}

	var listToStyles = __webpack_require__(6)

	/*
	type StyleObject = {
	  id: number;
	  parts: Array<StyleObjectPart>
	}

	type StyleObjectPart = {
	  css: string;
	  media: string;
	  sourceMap: ?string
	}
	*/

	var stylesInDom = {/*
	  [id: number]: {
	    id: number,
	    refs: number,
	    parts: Array<(obj?: StyleObjectPart) => void>
	  }
	*/}

	var head = hasDocument && (document.head || document.getElementsByTagName('head')[0])
	var singletonElement = null
	var singletonCounter = 0
	var isProduction = false
	var noop = function () {}

	// Force single-tag solution on IE6-9, which has a hard limit on the # of <style>
	// tags it will allow on a page
	var isOldIE = typeof navigator !== 'undefined' && /msie [6-9]\b/.test(navigator.userAgent.toLowerCase())

	module.exports = function (parentId, list, _isProduction) {
	  isProduction = _isProduction

	  var styles = listToStyles(parentId, list)
	  addStylesToDom(styles)

	  return function update (newList) {
	    var mayRemove = []
	    for (var i = 0; i < styles.length; i++) {
	      var item = styles[i]
	      var domStyle = stylesInDom[item.id]
	      domStyle.refs--
	      mayRemove.push(domStyle)
	    }
	    if (newList) {
	      styles = listToStyles(parentId, newList)
	      addStylesToDom(styles)
	    } else {
	      styles = []
	    }
	    for (var i = 0; i < mayRemove.length; i++) {
	      var domStyle = mayRemove[i]
	      if (domStyle.refs === 0) {
	        for (var j = 0; j < domStyle.parts.length; j++) {
	          domStyle.parts[j]()
	        }
	        delete stylesInDom[domStyle.id]
	      }
	    }
	  }
	}

	function addStylesToDom (styles /* Array<StyleObject> */) {
	  for (var i = 0; i < styles.length; i++) {
	    var item = styles[i]
	    var domStyle = stylesInDom[item.id]
	    if (domStyle) {
	      domStyle.refs++
	      for (var j = 0; j < domStyle.parts.length; j++) {
	        domStyle.parts[j](item.parts[j])
	      }
	      for (; j < item.parts.length; j++) {
	        domStyle.parts.push(addStyle(item.parts[j]))
	      }
	      if (domStyle.parts.length > item.parts.length) {
	        domStyle.parts.length = item.parts.length
	      }
	    } else {
	      var parts = []
	      for (var j = 0; j < item.parts.length; j++) {
	        parts.push(addStyle(item.parts[j]))
	      }
	      stylesInDom[item.id] = { id: item.id, refs: 1, parts: parts }
	    }
	  }
	}

	function createStyleElement () {
	  var styleElement = document.createElement('style')
	  styleElement.type = 'text/css'
	  head.appendChild(styleElement)
	  return styleElement
	}

	function addStyle (obj /* StyleObjectPart */) {
	  var update, remove
	  var styleElement = document.querySelector('style[data-vue-ssr-id~="' + obj.id + '"]')

	  if (styleElement) {
	    if (isProduction) {
	      // has SSR styles and in production mode.
	      // simply do nothing.
	      return noop
	    } else {
	      // has SSR styles but in dev mode.
	      // for some reason Chrome can't handle source map in server-rendered
	      // style tags - source maps in <style> only works if the style tag is
	      // created and inserted dynamically. So we remove the server rendered
	      // styles and inject new ones.
	      styleElement.parentNode.removeChild(styleElement)
	    }
	  }

	  if (isOldIE) {
	    // use singleton mode for IE9.
	    var styleIndex = singletonCounter++
	    styleElement = singletonElement || (singletonElement = createStyleElement())
	    update = applyToSingletonTag.bind(null, styleElement, styleIndex, false)
	    remove = applyToSingletonTag.bind(null, styleElement, styleIndex, true)
	  } else {
	    // use multi-style-tag mode in all other cases
	    styleElement = createStyleElement()
	    update = applyToTag.bind(null, styleElement)
	    remove = function () {
	      styleElement.parentNode.removeChild(styleElement)
	    }
	  }

	  update(obj)

	  return function updateStyle (newObj /* StyleObjectPart */) {
	    if (newObj) {
	      if (newObj.css === obj.css &&
	          newObj.media === obj.media &&
	          newObj.sourceMap === obj.sourceMap) {
	        return
	      }
	      update(obj = newObj)
	    } else {
	      remove()
	    }
	  }
	}

	var replaceText = (function () {
	  var textStore = []

	  return function (index, replacement) {
	    textStore[index] = replacement
	    return textStore.filter(Boolean).join('\n')
	  }
	})()

	function applyToSingletonTag (styleElement, index, remove, obj) {
	  var css = remove ? '' : obj.css

	  if (styleElement.styleSheet) {
	    styleElement.styleSheet.cssText = replaceText(index, css)
	  } else {
	    var cssNode = document.createTextNode(css)
	    var childNodes = styleElement.childNodes
	    if (childNodes[index]) styleElement.removeChild(childNodes[index])
	    if (childNodes.length) {
	      styleElement.insertBefore(cssNode, childNodes[index])
	    } else {
	      styleElement.appendChild(cssNode)
	    }
	  }
	}

	function applyToTag (styleElement, obj) {
	  var css = obj.css
	  var media = obj.media
	  var sourceMap = obj.sourceMap

	  if (media) {
	    styleElement.setAttribute('media', media)
	  }

	  if (sourceMap) {
	    // https://developer.chrome.com/devtools/docs/javascript-debugging
	    // this makes source maps inside style tags work properly in Chrome
	    css += '\n/*# sourceURL=' + sourceMap.sources[0] + ' */'
	    // http://stackoverflow.com/a/26603875
	    css += '\n/*# sourceMappingURL=data:application/json;base64,' + btoa(unescape(encodeURIComponent(JSON.stringify(sourceMap)))) + ' */'
	  }

	  if (styleElement.styleSheet) {
	    styleElement.styleSheet.cssText = css
	  } else {
	    while (styleElement.firstChild) {
	      styleElement.removeChild(styleElement.firstChild)
	    }
	    styleElement.appendChild(document.createTextNode(css))
	  }
	}


/***/ }),
/* 6 */
/***/ (function(module, exports) {

	/**
	 * Translates the list format produced by css-loader into something
	 * easier to manipulate.
	 */
	module.exports = function listToStyles (parentId, list) {
	  var styles = []
	  var newStyles = {}
	  for (var i = 0; i < list.length; i++) {
	    var item = list[i]
	    var id = item[0]
	    var css = item[1]
	    var media = item[2]
	    var sourceMap = item[3]
	    var part = {
	      id: parentId + ':' + i,
	      css: css,
	      media: media,
	      sourceMap: sourceMap
	    }
	    if (!newStyles[id]) {
	      styles.push(newStyles[id] = { id: id, parts: [part] })
	    } else {
	      newStyles[id].parts.push(part)
	    }
	  }
	  return styles
	}


/***/ }),
/* 7 */
/***/ (function(module, exports) {

	/* globals __VUE_SSR_CONTEXT__ */

	// this module is a runtime utility for cleaner component module output and will
	// be included in the final webpack user bundle

	module.exports = function normalizeComponent (
	  rawScriptExports,
	  compiledTemplate,
	  injectStyles,
	  scopeId,
	  moduleIdentifier /* server only */
	) {
	  var esModule
	  var scriptExports = rawScriptExports = rawScriptExports || {}

	  // ES6 modules interop
	  var type = typeof rawScriptExports.default
	  if (type === 'object' || type === 'function') {
	    esModule = rawScriptExports
	    scriptExports = rawScriptExports.default
	  }

	  // Vue.extend constructor export interop
	  var options = typeof scriptExports === 'function'
	    ? scriptExports.options
	    : scriptExports

	  // render functions
	  if (compiledTemplate) {
	    options.render = compiledTemplate.render
	    options.staticRenderFns = compiledTemplate.staticRenderFns
	  }

	  // scopedId
	  if (scopeId) {
	    options._scopeId = scopeId
	  }

	  var hook
	  if (moduleIdentifier) { // server build
	    hook = function (context) {
	      // 2.3 injection
	      context =
	        context || // cached call
	        (this.$vnode && this.$vnode.ssrContext) || // stateful
	        (this.parent && this.parent.$vnode && this.parent.$vnode.ssrContext) // functional
	      // 2.2 with runInNewContext: true
	      if (!context && typeof __VUE_SSR_CONTEXT__ !== 'undefined') {
	        context = __VUE_SSR_CONTEXT__
	      }
	      // inject component styles
	      if (injectStyles) {
	        injectStyles.call(this, context)
	      }
	      // register component module identifier for async chunk inferrence
	      if (context && context._registeredComponents) {
	        context._registeredComponents.add(moduleIdentifier)
	      }
	    }
	    // used by ssr in case component is cached and beforeCreate
	    // never gets called
	    options._ssrRegister = hook
	  } else if (injectStyles) {
	    hook = injectStyles
	  }

	  if (hook) {
	    var functional = options.functional
	    var existing = functional
	      ? options.render
	      : options.beforeCreate
	    if (!functional) {
	      // inject component registration as beforeCreate hook
	      options.beforeCreate = existing
	        ? [].concat(existing, hook)
	        : [hook]
	    } else {
	      // register for functioal component in vue file
	      options.render = function renderWithStyleInjection (h, context) {
	        hook.call(context)
	        return existing(h, context)
	      }
	    }
	  }

	  return {
	    esModule: esModule,
	    exports: scriptExports,
	    options: options
	  }
	}


/***/ }),
/* 8 */,
/* 9 */,
/* 10 */,
/* 11 */,
/* 12 */,
/* 13 */,
/* 14 */,
/* 15 */,
/* 16 */,
/* 17 */,
/* 18 */,
/* 19 */,
/* 20 */,
/* 21 */,
/* 22 */,
/* 23 */,
/* 24 */,
/* 25 */,
/* 26 */,
/* 27 */,
/* 28 */,
/* 29 */,
/* 30 */,
/* 31 */,
/* 32 */,
/* 33 */,
/* 34 */,
/* 35 */,
/* 36 */,
/* 37 */,
/* 38 */,
/* 39 */,
/* 40 */,
/* 41 */,
/* 42 */,
/* 43 */,
/* 44 */,
/* 45 */,
/* 46 */,
/* 47 */,
/* 48 */,
/* 49 */,
/* 50 */,
/* 51 */,
/* 52 */,
/* 53 */,
/* 54 */,
/* 55 */,
/* 56 */,
/* 57 */,
/* 58 */,
/* 59 */,
/* 60 */,
/* 61 */,
/* 62 */,
/* 63 */,
/* 64 */,
/* 65 */,
/* 66 */,
/* 67 */,
/* 68 */,
/* 69 */,
/* 70 */,
/* 71 */,
/* 72 */,
/* 73 */,
/* 74 */,
/* 75 */,
/* 76 */,
/* 77 */,
/* 78 */,
/* 79 */,
/* 80 */,
/* 81 */,
/* 82 */,
/* 83 */,
/* 84 */,
/* 85 */,
/* 86 */,
/* 87 */,
/* 88 */,
/* 89 */,
/* 90 */,
/* 91 */,
/* 92 */,
/* 93 */,
/* 94 */,
/* 95 */,
/* 96 */,
/* 97 */,
/* 98 */,
/* 99 */,
/* 100 */,
/* 101 */,
/* 102 */,
/* 103 */,
/* 104 */,
/* 105 */,
/* 106 */,
/* 107 */,
/* 108 */,
/* 109 */,
/* 110 */,
/* 111 */,
/* 112 */,
/* 113 */,
/* 114 */,
/* 115 */,
/* 116 */,
/* 117 */,
/* 118 */,
/* 119 */,
/* 120 */,
/* 121 */,
/* 122 */,
/* 123 */,
/* 124 */,
/* 125 */,
/* 126 */,
/* 127 */,
/* 128 */,
/* 129 */,
/* 130 */,
/* 131 */,
/* 132 */,
/* 133 */,
/* 134 */,
/* 135 */,
/* 136 */,
/* 137 */,
/* 138 */,
/* 139 */,
/* 140 */,
/* 141 */,
/* 142 */,
/* 143 */,
/* 144 */,
/* 145 */,
/* 146 */,
/* 147 */,
/* 148 */,
/* 149 */,
/* 150 */,
/* 151 */,
/* 152 */,
/* 153 */,
/* 154 */,
/* 155 */,
/* 156 */,
/* 157 */,
/* 158 */,
/* 159 */,
/* 160 */,
/* 161 */,
/* 162 */,
/* 163 */,
/* 164 */,
/* 165 */,
/* 166 */,
/* 167 */,
/* 168 */,
/* 169 */,
/* 170 */,
/* 171 */,
/* 172 */,
/* 173 */,
/* 174 */,
/* 175 */,
/* 176 */,
/* 177 */,
/* 178 */,
/* 179 */,
/* 180 */,
/* 181 */,
/* 182 */,
/* 183 */,
/* 184 */,
/* 185 */,
/* 186 */,
/* 187 */,
/* 188 */,
/* 189 */,
/* 190 */,
/* 191 */,
/* 192 */,
/* 193 */,
/* 194 */,
/* 195 */,
/* 196 */,
/* 197 */,
/* 198 */,
/* 199 */,
/* 200 */,
/* 201 */,
/* 202 */,
/* 203 */,
/* 204 */,
/* 205 */,
/* 206 */,
/* 207 */,
/* 208 */,
/* 209 */,
/* 210 */,
/* 211 */,
/* 212 */,
/* 213 */,
/* 214 */,
/* 215 */,
/* 216 */,
/* 217 */,
/* 218 */,
/* 219 */,
/* 220 */,
/* 221 */,
/* 222 */,
/* 223 */,
/* 224 */,
/* 225 */,
/* 226 */,
/* 227 */
/***/ (function(module, exports, __webpack_require__) {

	var disposed = false
	function injectStyle (ssrContext) {
	  if (disposed) return
	  __webpack_require__(228)
	}
	var Component = __webpack_require__(7)(
	  /* script */
	  __webpack_require__(230),
	  /* template */
	  __webpack_require__(280),
	  /* styles */
	  injectStyle,
	  /* scopeId */
	  "data-v-1ef3bcb8",
	  /* moduleIdentifier (server only) */
	  null
	)
	Component.options.__file = "/Users/Hanks/Codes/work/incubator-weex/examples/vue/showcase/new-fashion.vue"
	if (Component.esModule && Object.keys(Component.esModule).some(function (key) {return key !== "default" && key.substr(0, 2) !== "__"})) {console.error("named exports are not supported in *.vue files.")}
	if (Component.options.functional) {console.error("[vue-loader] new-fashion.vue: functional components are not supported with templates, they should use render functions.")}

	/* hot reload */
	if (false) {(function () {
	  var hotAPI = require("vue-hot-reload-api")
	  hotAPI.install(require("vue"), false)
	  if (!hotAPI.compatible) return
	  module.hot.accept()
	  if (!module.hot.data) {
	    hotAPI.createRecord("data-v-1ef3bcb8", Component.options)
	  } else {
	    hotAPI.reload("data-v-1ef3bcb8", Component.options)
	  }
	  module.hot.dispose(function (data) {
	    disposed = true
	  })
	})()}

	module.exports = Component.exports


/***/ }),
/* 228 */
/***/ (function(module, exports, __webpack_require__) {

	// style-loader: Adds some css to the DOM by adding a <style> tag

	// load the styles
	var content = __webpack_require__(229);
	if(typeof content === 'string') content = [[module.id, content, '']];
	if(content.locals) module.exports = content.locals;
	// add the styles to the DOM
	var update = __webpack_require__(5)("a5b17b34", content, false);
	// Hot Module Replacement
	if(false) {
	 // When the styles change, update the <style> tags
	 if(!content.locals) {
	   module.hot.accept("!!../../../node_modules/css-loader/index.js!../../../node_modules/vue-loader/lib/style-compiler/index.js?{\"vue\":true,\"id\":\"data-v-1ef3bcb8\",\"scoped\":true,\"hasInlineConfig\":false}!../../../node_modules/vue-loader/lib/selector.js?type=styles&index=0!./new-fashion.vue", function() {
	     var newContent = require("!!../../../node_modules/css-loader/index.js!../../../node_modules/vue-loader/lib/style-compiler/index.js?{\"vue\":true,\"id\":\"data-v-1ef3bcb8\",\"scoped\":true,\"hasInlineConfig\":false}!../../../node_modules/vue-loader/lib/selector.js?type=styles&index=0!./new-fashion.vue");
	     if(typeof newContent === 'string') newContent = [[module.id, newContent, '']];
	     update(newContent);
	   });
	 }
	 // When the module is disposed, remove the <style> tags
	 module.hot.dispose(function() { update(); });
	}

/***/ }),
/* 229 */
/***/ (function(module, exports, __webpack_require__) {

	exports = module.exports = __webpack_require__(4)();
	// imports


	// module
	exports.push([module.id, "\n.content[data-v-1ef3bcb8] {\n  color: #353535;\n  background-color: #666;\n  position: absolute;\n  top: 0;\n  left: 0;\n  right: 0;\n  bottom: 0;\n}\n.refresh[data-v-1ef3bcb8] {\n  align-items: center;\n  justify-content: center;\n}\n.loading[data-v-1ef3bcb8] {\n  align-items: center;\n  justify-content: center;\n}\n", ""]);

	// exports


/***/ }),
/* 230 */
/***/ (function(module, exports, __webpack_require__) {

	'use strict';

	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//

	var modal = weex.requireModule('modal');
	var timer = weex.requireModule('timer');

	module.exports = {
	  components: {
	    headlines: __webpack_require__(231),
	    category: __webpack_require__(245),
	    coupon: __webpack_require__(250),
	    goods: __webpack_require__(255),
	    resource: __webpack_require__(260),
	    scene: __webpack_require__(265),
	    match: __webpack_require__(270),
	    brand: __webpack_require__(275)
	  },
	  data: {
	    navBarHeight: 88,
	    cnt: 0,
	    show0: true,
	    show1: false,
	    show2: false,
	    displayRefresh: 'show',
	    displayLoading: 'show',
	    showData: {}
	  },
	  created: function created() {
	    this.showData = {
	      "header": {
	        "topBanner": "https://gw.alicdn.com/tps/TB1J48kMXXXXXXtapXXXXXXXXXX-750-782.jpg",
	        "ruleLink": "https://chaoshi.m.tmall.com/",
	        "isShowShare": true,
	        "shareInfo": {
	          "shareTitle": "我是分享标题",
	          "shareIntro": " 我是分享内容测试",
	          "allLink": "https://www.tmall.com",
	          "mobileImage": "https://gw.alicdn.com/tps/TB1J48kMXXXXXXtapXXXXXXXXXX-750-782.jpg",
	          "pcImage": "https://img.alicdn.com/tps/i4/TB1Dyw1LVXXXXXEapXXSutbFXXX.jpg",
	          "shareScene": "我是测试业务",
	          "bizId": ""
	        },
	        "announceHdBanner": "https://gw.alicdn.com/tps/i4/TB1fzclLVXXXXaVaXXXwu0bFXXX.png",
	        "bannerItems": [{
	          "img": "https://img.alicdn.com/tps/TB1U8LHLVXXXXa3XpXXXXXXXXXX-233-172.jpg",
	          "url": "https://chaoshi.m.tmall.com/"
	        }, {
	          "img": "https://img.alicdn.com/tps/TB1U8LHLVXXXXa3XpXXXXXXXXXX-233-172.jpg",
	          "url": "https://chaoshi.m.tmall.com/"
	        }, {
	          "img": "https://img.alicdn.com/tps/TB1U8LHLVXXXXa3XpXXXXXXXXXX-233-172.jpg",
	          "url": "https://chaoshi.m.tmall.com/"
	        }]
	      },
	      "fashion": {
	        "isHide": false,
	        "floorTitle": "https://gw.alicdn.com/tps/TB1._64LVXXXXatXVXXXXXXXXXX-750-100.jpg",
	        "bigBannerImg": "https://gw.alicdn.com/tps/TB1tO.aLVXXXXcvXpXXXXXXXXXX-750-262.jpg",
	        "bigBannerUrl": "https://chaoshi.m.tmall.com/",
	        "smallBannerItems": [{
	          "img": "https://img.alicdn.com/tps/TB1Ff_0LVXXXXXNaXXXXXXXXXXX-375-262.jpg",
	          "url": "https://chaoshi.m.tmall.com/"
	        }, {
	          "img": "https://img.alicdn.com/tps/TB1Ff_0LVXXXXXNaXXXXXXXXXXX-375-262.jpg",
	          "url": "https://chaoshi.m.tmall.com/"
	        }],
	        "isHideStarBanner": false,
	        "firstStarBannerItems": [{
	          "img": "https://gw.alicdn.com/tps/TB1xlIVLVXXXXckXVXXXXXXXXXX-250-220.jpg",
	          "url": "https://chaoshi.m.tmall.com/"
	        }, {
	          "img": "https://gw.alicdn.com/tps/TB1xlIVLVXXXXckXVXXXXXXXXXX-250-220.jpg",
	          "url": "https://chaoshi.m.tmall.com/"
	        }],
	        "secondStarBannerItems": [{
	          "img": "https://gw.alicdn.com/tps/TB1xlIVLVXXXXckXVXXXXXXXXXX-250-220.jpg",
	          "url": "https://chaoshi.m.tmall.com/"
	        }, {
	          "img": "https://gw.alicdn.com/tps/TB1xlIVLVXXXXckXVXXXXXXXXXX-250-220.jpg",
	          "url": "https://chaoshi.m.tmall.com/"
	        }],
	        "thirdStarBannerItems": [{
	          "img": "https://gw.alicdn.com/tps/TB1xlIVLVXXXXckXVXXXXXXXXXX-250-220.jpg",
	          "url": "https://chaoshi.m.tmall.com/"
	        }, {
	          "img": "https://gw.alicdn.com/tps/TB1xlIVLVXXXXckXVXXXXXXXXXX-250-220.jpg",
	          "url": "https://chaoshi.m.tmall.com/"
	        }]
	      },
	      "goods": {
	        "isHide": false,
	        "floorTitle": "https://gw.alicdn.com/tps/TB1._64LVXXXXatXVXXXXXXXXXX-750-100.jpg",
	        "bannerItems": [{
	          "img1": "https://gw.alicdn.com/tps/TB1TUzGLVXXXXc1XFXXXXXXXXXX-250-296.jpg",
	          "url1": "https://chaoshi.m.tmall.com/",
	          "img2": "https://gw.alicdn.com/tps/TB1TUzGLVXXXXc1XFXXXXXXXXXX-250-296.jpg",
	          "url2": "https://chaoshi.m.tmall.com/",
	          "img3": "https://gw.alicdn.com/tps/TB12MPFLVXXXXXnXVXXXXXXXXXX-250-592.jpg",
	          "url3": "https://chaoshi.m.tmall.com/",
	          "img4": "https://gw.alicdn.com/tps/TB1TUzGLVXXXXc1XFXXXXXXXXXX-250-296.jpg",
	          "url4": "https://chaoshi.m.tmall.com/",
	          "img5": "https://gw.alicdn.com/tps/TB1TUzGLVXXXXc1XFXXXXXXXXXX-250-296.jpg",
	          "url5": "https://chaoshi.m.tmall.com/"
	        }, {
	          "img1": "https://gw.alicdn.com/tps/TB1TUzGLVXXXXc1XFXXXXXXXXXX-250-296.jpg",
	          "url1": "https://chaoshi.m.tmall.com/",
	          "img2": "https://gw.alicdn.com/tps/TB1TUzGLVXXXXc1XFXXXXXXXXXX-250-296.jpg",
	          "url2": "https://chaoshi.m.tmall.com/",
	          "img3": "https://gw.alicdn.com/tps/TB12MPFLVXXXXXnXVXXXXXXXXXX-250-592.jpg",
	          "url3": "https://chaoshi.m.tmall.com/",
	          "img4": "https://gw.alicdn.com/tps/TB1TUzGLVXXXXc1XFXXXXXXXXXX-250-296.jpg",
	          "url4": "https://chaoshi.m.tmall.com/",
	          "img5": "https://gw.alicdn.com/tps/TB1TUzGLVXXXXc1XFXXXXXXXXXX-250-296.jpg",
	          "url5": "https://chaoshi.m.tmall.com/"
	        }]
	      },
	      "coupon": {
	        "floorTitle": "https://gw.alicdn.com/tps/TB1._64LVXXXXatXVXXXXXXXXXX-750-100.jpg"
	      },
	      "scene": {
	        "floorTitle": "https://gw.alicdn.com/tps/TB1._64LVXXXXatXVXXXXXXXXXX-750-100.jpg",
	        "_backup": {
	          "id": "03653",
	          "data": [{
	            "id": "1395",
	            "sceneUrl": "https://pages.tmall.com/wow/act/15617/scene?acm=03653.1003.1.710531&aldid=TPMkJDIw&wh_id=1395&scm=1003.1.03653.MAIN_1395_710531&pos=1",
	            "_s_": "d",
	            "itemImg": "//img.alicdn.com/tps/i2/TB1n6bVLVXXXXcCXVXXTCU0QpXX-300-300.jpg",
	            "sceneCat": "50008881,50008883,50008882,50008884,50012774,50012775,50012776,50012781,50008886,50012766,50012771,50012772,50010850,50000671,162201,50022566,50007068,50026651,50000852,162104,162116,162205,1623,123216004,162404,162401,50011412,50011411,16240,50012773,50006846,50010394,50008888,50008890,50008889,50012784,50012785,50012786,50012787,50008885,50012777,50012778,124730001,50001748,50011130,124730001,50001748,50011130,50011129,50010160,50011161,50011159,50010159,50010158,50011127,3035,50010167,50000557,50011123,50010402,50000436,50011153,50011740,50006843,50006842,124942005,50010368,50011892,50011894,50023751,121366037,121458036,121392038,121420035,121364028,121468026,121400029,121418030,121478031,121480030,121366035,121424025,121426030,121484030,121412031,125054002,125074007,125074008,125104003,125080003,125024006,125080004,121400030,121424024,121482025,121418031,121402028,121388035",
	            "finalScore": "77.26011111111112",
	            "_pos_": 1,
	            "entityType": "MAIN",
	            "benefitImg": "//img.alicdn.com/tps/i3/TB1Onr9LVXXXXbiXpXXB8gAQpXX-300-100.png"
	          }, {
	            "id": "1394",
	            "sceneUrl": "https://pages.tmall.com/wow/act/15617/scene?acm=03653.1003.1.710531&aldid=TPMkJDIw&wh_id=1394&scm=1003.1.03653.MAIN_1394_710531&pos=2",
	            "_s_": "d",
	            "itemImg": "//img.alicdn.com/tps/i1/TB13mj1LVXXXXa.XFXXTCU0QpXX-300-300.jpg",
	            "sceneCat": "50010850,50000671,162201,50022566,50007068,50026651,50000852,162104,162116,162205,1623,123216004,162404,162401,50011412,50011411,162402,50008903,50011413,162403,50000697,1629,50011277,121412004,50008897,50008901,50008899,50013194,50008904,50008905,50008898,162103,50008900,50013196,121434004,50005065,162702,162701,162703,1636,124730001,50001748,50011130,124730001,50001748,50011130,50011129,50010160,50011161,50011159,50010159,50010158,50011127,3035,50010167,50000557,50011123,50010402,50000436,50011153,50011740,50006843,50006842,124986002,50010368, 50011892,50011894,121480030,121366035,121424025,121426030,121484030,121412031",
	            "finalScore": "77.06103999999999",
	            "_pos_": 2,
	            "entityType": "MAIN",
	            "benefitImg": "//img.alicdn.com/tps/i2/TB17kvVLVXXXXaTXVXXB8gAQpXX-300-100.png"
	          }, {
	            "id": "1396",
	            "sceneUrl": "https://pages.tmall.com/wow/act/15617/scene?acm=03653.1003.1.710531&aldid=TPMkJDIw&wh_id=1396&scm=1003.1.03653.MAIN_1396_710531&pos=3",
	            "_s_": "d",
	            "itemImg": "//img.alicdn.com/tps/i2/TB1ZOAXLVXXXXa0XXXXTCU0QpXX-300-300.jpg",
	            "sceneCat": "50010368,50011892,50011894,50013794,50010792,50010790,50010790,121434025,121396024,50010793,50010789,50010793,121386035,121392037,121458034,121392036,121482027,121392015,121368014,121418013,121392016,50011982,122690003,302910,50012010,122654005,50012019,50012825,50012027,50012042,50011744,50012906,50012907,50008881,50008883,50008882?,50012784?,50022889,50022728,50013228,50013896,50014775,50019578,50014822,50026394,50019694,50019279,50019272,50014079,50014078,50014076,50014077,50017238,121464011,121468014,50019130,50019127,50019126,50019128,50019125,50015984,50014803,50014802,50015980,50014788,50014787,50019526,50014791,50019520,50013932,50014677,50015978,50014786,50014785,50522001,50014493,50023100,121418021,121388027,121424021,121418020,121368020,50012036,50012044,50012043,50010368, 50011892, 50019274,50012044,50012048,50012043,50013228,50013238,50011726,50019279,50015984,50014803,50014802,50019526,50014791,50019520,50013932,50014677,50019526,50013314,50023341,50016752,50016749,50016747,50016744,50016741,50016740,50016739,50016738,50016737,50016769,50016763,50016750,50007003,302910,50012010,50012032,50012033,50011745,50011746,50011894,121458034,121756003,121426033,50011982,50011979,121390006,124730001,50001748,50011130,124730001,50001748,50011130,50011129,50010160,50011161,50011159,50010159,50010158,50011127,3035,50010167,50000557,50011123,50010402,50000436,50011153,50010850,50000671,162201,50022566,50007068,50026651,50000852,162104,162116,162205,1623,123216004,162404,162401,50011412,50011411,162402,50008903,50011413,162403,50000697,1629,50011277,121412004,50008897,50008901,50008899,50013194,50008904,50008905,50008898,162103,50008900,50013196,121434004,50005065,162702,162701,162703,1636,1512,123534001,14,50008090,50012341,50132002,50012342,50012343,50012345,50012346,50013189,50010527,121394006,50013618,50010537,50012424,121452038,121484044,50010540,50010548,50024769,121450007,50013697,50010218,50005960,50005962,50012820,50012406,50023620,50023605,50008829,50024064,50023440,50008687,124392001,50023771,50006804,50012410,124482010,121398023,50023789,50011975",
	            "finalScore": "76.76379487179486",
	            "_pos_": 3,
	            "entityType": "MAIN",
	            "benefitImg": "//img.alicdn.com/tps/i3/TB1Usn.LVXXXXaoXpXXB8gAQpXX-300-100.png"
	          }, {
	            "id": "1619",
	            "sceneUrl": "https://pages.tmall.com/wow/act/15617/scene?acm=03653.1003.1.710531&aldid=TPMkJDIw&wh_id=1619&scm=1003.1.03653.SUB_1619_710531&pos=4",
	            "_s_": "d",
	            "itemImg": "//img.alicdn.com/tps/i2/TB1MpoBLVXXXXbxXpXXKofZZpXX-182-206.jpg",
	            "sceneCat": "124730001,50001748,50011130,124730001,50001748,50011130,50011129,50010160,50011161,50011159,50010159,50010158,50011127,3035,50010167,50000557,50011123,50010402,50000436,50011153,122654005,50011740,50009032,124688012，213202,121396029,121476023,121410029,213203,50023326,213205,50022686,50023293,50466023,50024999,50016885，50011894",
	            "finalScore": "75.88480000000001",
	            "_pos_": 4,
	            "entityType": "SUB",
	            "benefitImg": "//img.alicdn.com/tps/i4/TB1j0AxLVXXXXaRXFXX3bpXTXXX-170-100.png"
	          }, {
	            "id": "1612",
	            "sceneUrl": "https://pages.tmall.com/wow/act/15617/scene?acm=03653.1003.1.710531&aldid=TPMkJDIw&wh_id=1612&scm=1003.1.03653.SUB_1612_710531&pos=5",
	            "_s_": "d",
	            "itemImg": "//img.alicdn.com/tps/i3/TB1NNwyLVXXXXaEXVXXKofZZpXX-182-206.jpg",
	            "sceneCat": "50010850,50000671,162104,162116,123216004",
	            "finalScore": "75.23733333333334",
	            "_pos_": 5,
	            "entityType": "SUB",
	            "benefitImg": "//img.alicdn.com/tps/i1/TB1PsoBLVXXXXbQXpXX3bpXTXXX-170-100.png"
	          }, {
	            "id": "1610",
	            "sceneUrl": "https://pages.tmall.com/wow/act/15617/scene?acm=03653.1003.1.710531&aldid=TPMkJDIw&wh_id=1610&scm=1003.1.03653.SUB_1610_710531&pos=6",
	            "_s_": "d",
	            "itemImg": "//img.alicdn.com/tps/i4/TB1EkczLVXXXXcVXpXXKofZZpXX-182-206.jpg",
	            "sceneCat": "121484044,121452038,50012342,50012027,50012032,50006843,50010850,50000671,162104,1623,50011277,162116,123216004,50008898,121366037,121458036,121392038,121420035,121364028,121468026,121472031,121476036,121470042,121418032,121410036,121472032,121420032,121470040,121384032,121460028,121448028,121400030,121424024,121482025,121418031,121402028,121388035,121460029,121400028,121400029,121418030,121476034,121478031,121448029,121480030,121366035,121424025,121426030,121484030,121412031,121384033,121424023,121402027,121408039,121420033,121452028,121466040,121416029,121482028,121420034,121396046,121422052,121370037,121424031,21364043,121472040,121412045,121424032,125054002,125074007,125074008,125104003,125080003,125024006,125080004",
	            "finalScore": "75.00533333333334",
	            "_pos_": 6,
	            "entityType": "SUB",
	            "benefitImg": "//img.alicdn.com/tps/i2/TB1RzwFLVXXXXXsXpXX3bpXTXXX-170-100.png"
	          }, {
	            "id": "1620",
	            "sceneUrl": "https://pages.tmall.com/wow/act/15617/scene?acm=03653.1003.1.710531&aldid=TPMkJDIw&wh_id=1620&scm=1003.1.03653.SUB_1620_710531&pos=7",
	            "_s_": "d",
	            "itemImg": "//img.alicdn.com/tps/i1/TB1IJsILVXXXXbGXXXXKofZZpXX-182-206.jpg",
	            "sceneCat": "50010789,50010790,50010792,50010793,50010794,50010796,50010797,50010798,50010801,50010803,50010805,50010807,50010808,50010813,50010814,50010815,50013794,121382014,121386023,121388007,121388008,121398004,121364007,121398006,121410007,121426007,121452004,50012010,50012027,50012032，124688012, 124986002, 124942005，50010850,50000671,162104,162201,50022566,162116,1623,50011277,50008897,162401,162402",
	            "finalScore": "74.72522222222223",
	            "_pos_": 7,
	            "entityType": "SUB",
	            "benefitImg": "//img.alicdn.com/tps/i1/TB16A.yLVXXXXX8XFXX3bpXTXXX-170-100.png"
	          }, {
	            "id": "1608",
	            "sceneUrl": "https://pages.tmall.com/wow/act/15617/scene?acm=03653.1003.1.710531&aldid=TPMkJDIw&wh_id=1608&scm=1003.1.03653.SUB_1608_710531&pos=8",
	            "_s_": "d",
	            "itemImg": "//img.alicdn.com/tps/i3/TB1xA3jLVXXXXXqapXXKofZZpXX-182-206.jpg",
	            "sceneCat": "50012010,50012027,50012032,50007003,50008881,50008883,1623,162116,50010850,123216004",
	            "finalScore": "74.626",
	            "_pos_": 8,
	            "entityType": "SUB",
	            "benefitImg": "//img.alicdn.com/tps/i2/TB1WvoALVXXXXcdXpXX3bpXTXXX-170-100.png"
	          }, {
	            "id": "1615",
	            "sceneUrl": "https://pages.tmall.com/wow/act/15617/scene?acm=03653.1003.1.710531&aldid=TPMkJDIw&wh_id=1615&scm=1003.1.03653.SUB_1615_710531&pos=9",
	            "_s_": "d",
	            "itemImg": "//img.alicdn.com/tps/i1/TB1PVctLVXXXXchXFXXKofZZpXX-182-206.jpg",
	            "sceneCat": "50011978,50011979,50011980,50011993,50011996,50011997,121390006,121408011,121474010,50010789,50010790,50010792,50010793,50010803,50013794,121398004,121410007，213205,121470030,50024980,121408030",
	            "finalScore": "74.02799999999999",
	            "_pos_": 9,
	            "entityType": "SUB",
	            "benefitImg": "//img.alicdn.com/tps/i4/TB1yKwBLVXXXXa4XpXX3bpXTXXX-170-100.png"
	          }, {
	            "id": "1618",
	            "sceneUrl": "https://pages.tmall.com/wow/act/15617/scene?acm=03653.1003.1.710531&aldid=TPMkJDIw&wh_id=1618&scm=1003.1.03653.SUB_1618_710531&pos=10",
	            "_s_": "d",
	            "itemImg": "//img.alicdn.com/tps/i1/TB1vn3sLVXXXXX5XVXXKofZZpXX-182-206.jpg",
	            "sceneCat": "50008886,50012766,50012771,50012772,50012773,50008090,111219,50012100,50012082,50012478,50012475,213202,50018977,121396029,121476023,50018994,121410029,213203,50023326,50003820,213205,50022686,50012481,50013976?,50001871,122854005,121458010,121452006,121416008,50008779,50002777,121394007,121386009,50010825,50013810,121434010,50001865,50008565,50008246,50017192,121404045,121368009,121462014,50019372,50008248,121418011,50008249,50017193,50008252,50008251,50000512,121364047",
	            "finalScore": "73.94212121212121",
	            "_pos_": 10,
	            "entityType": "SUB",
	            "benefitImg": "//img.alicdn.com/tps/i4/TB1kSkpLVXXXXbTXVXX3bpXTXXX-170-100.png"
	          }, {
	            "id": "1617",
	            "sceneUrl": "https://pages.tmall.com/wow/act/15617/scene?acm=03653.1003.1.710531&aldid=TPMkJDIw&wh_id=1617&scm=1003.1.03653.SUB_1617_710531&pos=11",
	            "_s_": "d",
	            "itemImg": "//img.alicdn.com/tps/i1/TB1ELwrLVXXXXaNXVXXKofZZpXX-182-206.jpg",
	            "sceneCat": "50013189,50010527,121394006,50013618,50010537,50012424,121452038,121484044,50010540,50010548,50024769,50012341,50132002,50012342,50012343,50012345,50012346,50012341,50132002,50012342,50012343,50012345,50012346",
	            "finalScore": "73.78883333333334",
	            "_pos_": 11,
	            "entityType": "SUB",
	            "benefitImg": "//img.alicdn.com/tps/i3/TB1CtsuLVXXXXcJXFXX3bpXTXXX-170-100.png"
	          }],
	          "exposureParam": "//ac.mmstat.com/1.gif?apply=vote&abbucket=_AB-M1001_B5&com=02&acm=03653.1003.1.710531&cod=03653&cache=jWl8FJsb&aldid=TPMkJDIw&logtype=4&abtest=_AB-LR1001-PR1001&scm=1003.1.03653.710531&ip=42.120.74.104",
	          "serverTime": 1458296110334
	        }
	      },
	      "resource": {
	        "isHide": false,
	        "bigBannerImg": "https://gw.alicdn.com/tps/TB1tO.aLVXXXXcvXpXXXXXXXXXX-750-262.jpg",
	        "bigBannerUrl": "https://chaoshi.m.tmall.com/",
	        "smallBannerItems": [{
	          "img": "https://img.alicdn.com/tps/TB1Ff_0LVXXXXXNaXXXXXXXXXXX-375-262.jpg",
	          "url": "https://chaoshi.m.tmall.com/"
	        }, {
	          "img": "https://img.alicdn.com/tps/TB1Ff_0LVXXXXXNaXXXXXXXXXXX-375-262.jpg",
	          "url": "https://chaoshi.m.tmall.com/"
	        }]
	      },
	      "category": {
	        "floorTitle": "https://gw.alicdn.com/tps/TB1._64LVXXXXatXVXXXXXXXXXX-750-100.jpg",
	        "_backup": {
	          "main": [],
	          "sub": [{
	            "finalScore": 0,
	            "industryUrl": "//pages.tmall.com/wow/act/15617/dqc?abbucket=_AB-M1011_B5&acm=03654.1003.1.735362&aldid=e5zPEI6R&abtest=_AB-LR1011-PR1011&scm=1003.1.03654.OTHER_15_735362&pos=1",
	            "entityType": "OTHER",
	            "colour": "b4ebff",
	            "industryTitle": "数码",
	            "_pos_": 1,
	            "industryBenefit": "疯抢大牌新品",
	            "industryId": "15",
	            "industryImg": "//img.alicdn.com/tps/i4/TB1_xG.LVXXXXbWaXXXrdvGIFXX-248-155.jpg"
	          }, {
	            "finalScore": 0,
	            "industryUrl": "//pages.tmall.com/wow/act/15617/xihu?abbucket=_AB-M1011_B5&acm=03654.1003.1.735362&aldid=e5zPEI6R&abtest=_AB-LR1011-PR1011&scm=1003.1.03654.OTHER_13_735362&pos=2",
	            "entityType": "OTHER",
	            "colour": "cdffa4",
	            "industryTitle": "洗护",
	            "_pos_": 2,
	            "industryBenefit": "爆款1分钱抢",
	            "industryId": "13",
	            "industryImg": "//img.alicdn.com/tps/i3/TB1Ft_CLVXXXXX2XXXXrdvGIFXX-248-155.jpg"
	          }, {
	            "finalScore": 0,
	            "industryUrl": "//pages.tmall.com/wow/act/15617/mobilephone?abbucket=_AB-M1011_B5&acm=03654.1003.1.735362&aldid=e5zPEI6R&abtest=_AB-LR1011-PR1011&scm=1003.1.03654.OTHER_14_735362&pos=3",
	            "entityType": "OTHER",
	            "colour": "c5ffdf",
	            "industryTitle": "手机",
	            "_pos_": 3,
	            "industryBenefit": "1元抢优惠券",
	            "industryId": "14",
	            "industryImg": "//img.alicdn.com/tps/i3/TB1mJjuLVXXXXc.XXXXrdvGIFXX-248-155.jpg"
	          }, {
	            "finalScore": 0,
	            "industryUrl": "//pages.tmall.com/wow/act/15617/xiangbaohuanxin?abbucket=_AB-M1011_B5&acm=03654.1003.1.735362&aldid=e5zPEI6R&abtest=_AB-LR1011-PR1011&scm=1003.1.03654.OTHER_11_735362&pos=4",
	            "entityType": "OTHER",
	            "colour": "ffccdc",
	            "industryTitle": "箱包",
	            "_pos_": 4,
	            "industryBenefit": "极致焕新尖货",
	            "industryId": "11",
	            "industryImg": "//img.alicdn.com/tps/i2/TB1_5P9KVXXXXayaXXXrdvGIFXX-248-155.jpg"
	          }, {
	            "finalScore": 0,
	            "industryUrl": "//pages.tmall.com/wow/act/15617/newfood?abbucket=_AB-M1011_B5&acm=03654.1003.1.735362&aldid=e5zPEI6R&abtest=_AB-LR1011-PR1011&scm=1003.1.03654.OTHER_12_735362&pos=5",
	            "entityType": "OTHER",
	            "colour": "fff5a3",
	            "industryTitle": "食品",
	            "_pos_": 5,
	            "industryBenefit": "新品1分试吃",
	            "industryId": "12",
	            "industryImg": "//img.alicdn.com/tps/i2/TB1HfLzLVXXXXb_XXXXrdvGIFXX-248-155.jpg"
	          }, {
	            "finalScore": 0,
	            "industryUrl": "//pages.tmall.com/wow/act/15617/zbps?abbucket=_AB-M1011_B5&acm=03654.1003.1.735362&aldid=e5zPEI6R&abtest=_AB-LR1011-PR1011&scm=1003.1.03654.OTHER_3_735362&pos=6",
	            "entityType": "OTHER",
	            "colour": "bbffe5",
	            "industryTitle": "珠宝配饰",
	            "_pos_": 6,
	            "industryBenefit": "大牌1折秒杀",
	            "industryId": "3",
	            "industryImg": "//img.alicdn.com/tps/i1/TB1s5y.LVXXXXb.aXXXrdvGIFXX-248-155.jpg"
	          }, {
	            "finalScore": 0,
	            "industryUrl": "//pages.tmall.com/wow/act/15617/getbeauty?abbucket=_AB-M1011_B5&acm=03654.1003.1.735362&aldid=e5zPEI6R&abtest=_AB-LR1011-PR1011&scm=1003.1.03654.OTHER_2_735362&pos=7",
	            "entityType": "OTHER",
	            "colour": "ffd1b6",
	            "industryTitle": "美妆",
	            "_pos_": 7,
	            "industryBenefit": "即刻预约美丽",
	            "industryId": "2",
	            "industryImg": "//img.alicdn.com/tps/i1/TB1lNvoLVXXXXaoXFXXrdvGIFXX-248-155.jpg"
	          }, {
	            "finalScore": 0,
	            "industryUrl": "//pages.tmall.com/wow/act/15617/guojixfs?abbucket=_AB-M1011_B5&acm=03654.1003.1.735362&aldid=e5zPEI6R&abtest=_AB-LR1011-PR1011&scm=1003.1.03654.OTHER_1_735362&pos=8",
	            "entityType": "OTHER",
	            "colour": "b9eaf2",
	            "industryTitle": "天猫国际",
	            "_pos_": 8,
	            "industryBenefit": "唤醒全球潮流",
	            "industryId": "1",
	            "industryImg": "//img.alicdn.com/tps/i4/TB1JHbkLVXXXXblXFXXrdvGIFXX-248-155.jpg"
	          }, {
	            "finalScore": 0,
	            "industryUrl": "//pages.tmall.com/wow/act/15617/newnv?abbucket=_AB-M1011_B5&acm=03654.1003.1.735362&aldid=e5zPEI6R&abtest=_AB-LR1011-PR1011&scm=1003.1.03654.OTHER_10_735362&pos=9",
	            "entityType": "OTHER",
	            "colour": "fff8ee",
	            "industryTitle": "鞋履",
	            "_pos_": 9,
	            "industryBenefit": "潮品精选鞋履",
	            "industryId": "10",
	            "industryImg": "//img.alicdn.com/tps/i2/TB1RovwLVXXXXXnXpXXrdvGIFXX-248-155.jpg"
	          }, {
	            "finalScore": 0,
	            "industryUrl": "//pages.tmall.com/wow/act/15617/nvzhuang?abbucket=_AB-M1011_B5&acm=03654.1003.1.735362&aldid=e5zPEI6R&abtest=_AB-LR1011-PR1011&scm=1003.1.03654.OTHER_7_735362&pos=10",
	            "entityType": "OTHER",
	            "colour": "d5cbe8",
	            "industryTitle": "女装",
	            "_pos_": 10,
	            "industryBenefit": "折后用券抢新",
	            "industryId": "7",
	            "industryImg": "//img.alicdn.com/tps/i1/TB1BOncLVXXXXXSaXXXrdvGIFXX-248-155.jpg"
	          }, {
	            "finalScore": 0,
	            "industryUrl": "//pages.tmall.com/wow/act/15617/man?abbucket=_AB-M1011_B5&acm=03654.1003.1.735362&aldid=e5zPEI6R&abtest=_AB-LR1011-PR1011&scm=1003.1.03654.OTHER_6_735362&pos=11",
	            "entityType": "OTHER",
	            "colour": "cbe9a9",
	            "industryTitle": "男装",
	            "_pos_": 11,
	            "industryBenefit": "新品低至9.9元",
	            "industryId": "6",
	            "industryImg": "//img.alicdn.com/tps/i4/TB1t9TmLVXXXXbkXFXXrdvGIFXX-248-155.jpg"
	          }, {
	            "finalScore": 0,
	            "industryUrl": "//pages.tmall.com/wow/act/15617/myxfs?abbucket=_AB-M1011_B5&acm=03654.1003.1.735362&aldid=e5zPEI6R&abtest=_AB-LR1011-PR1011&scm=1003.1.03654.OTHER_5_735362&pos=12",
	            "entityType": "OTHER",
	            "colour": "f8e1ff",
	            "industryTitle": "母婴童装",
	            "_pos_": 12,
	            "industryBenefit": "跨店满299减40",
	            "industryId": "5",
	            "industryImg": "//img.alicdn.com/tps/i3/TB1kpziLVXXXXabXVXXrdvGIFXX-248-155.jpg"
	          }, {
	            "finalScore": 0,
	            "industryUrl": "//pages.tmall.com/wow/act/15617/jfjs?abbucket=_AB-M1011_B5&acm=03654.1003.1.735362&aldid=e5zPEI6R&abtest=_AB-LR1011-PR1011&scm=1003.1.03654.OTHER_4_735362&pos=13",
	            "entityType": "OTHER",
	            "colour": "bbffe5",
	            "industryTitle": "家纺家饰",
	            "_pos_": 13,
	            "industryBenefit": "全场低至9.9元",
	            "industryId": "4",
	            "industryImg": "//img.alicdn.com/tps/i1/TB1lUjzLVXXXXbAXXXXrdvGIFXX-248-155.jpg"
	          }, {
	            "finalScore": 0,
	            "industryUrl": "//pages.tmall.com/wow/act/15617/neiyi328?abbucket=_AB-M1011_B5&acm=03654.1003.1.735362&aldid=e5zPEI6R&abtest=_AB-LR1011-PR1011&scm=1003.1.03654.OTHER_9_735362&pos=14",
	            "entityType": "OTHER",
	            "colour": "ffc7c9",
	            "industryTitle": "内衣",
	            "_pos_": 14,
	            "industryBenefit": "新品3折起",
	            "industryId": "9",
	            "industryImg": "//img.alicdn.com/tps/i1/TB1S7PuLVXXXXcVXXXXrdvGIFXX-248-155.jpg"
	          }, {
	            "finalScore": 0,
	            "industryUrl": "//pages.tmall.com/wow/act/15617/ydhw?abbucket=_AB-M1011_B5&acm=03654.1003.1.735362&aldid=e5zPEI6R&abtest=_AB-LR1011-PR1011&scm=1003.1.03654.OTHER_8_735362&pos=15",
	            "entityType": "OTHER",
	            "colour": "cbdbfe",
	            "industryTitle": "运动户外",
	            "_pos_": 15,
	            "industryBenefit": "大牌新品抢券",
	            "industryId": "8",
	            "industryImg": "//img.alicdn.com/tps/i2/TB1mQDALVXXXXbdXXXXrdvGIFXX-248-155.jpg"
	          }],
	          "isFormal": false,
	          "exposureParam": "//ac.mmstat.com/1.gif?apply=vote&abbucket=_AB-M1011_B5&com=02&acm=03654.1003.1.735362&cod=03654&cache=CG5ZB02B&aldid=e5zPEI6R&logtype=4&abtest=_AB-LR1011-PR1011&scm=1003.1.03654.735362&ip=42.120.74.159",
	          "id": "03654"
	        }
	      },
	      "brand": {
	        "floorTitle": "https://gw.alicdn.com/tps/TB1._64LVXXXXatXVXXXXXXXXXX-750-100.jpg",
	        "bannerItems": [{
	          "leftImg": "https://gw.alicdn.com/tps/TB1F2fJLVXXXXccXFXXXXXXXXXX-750-230.jpg",
	          "leftUrl": "https://chaoshi.m.tmall.com/",
	          "rightImg": "https://gw.alicdn.com/tps/TB1F2fJLVXXXXccXFXXXXXXXXXX-750-230.jpg",
	          "rightUrl": "https://chaoshi.m.tmall.com/"
	        }, {
	          "leftImg": "https://gw.alicdn.com/tps/TB1F2fJLVXXXXccXFXXXXXXXXXX-750-230.jpg",
	          "leftUrl": "https://chaoshi.m.tmall.com/",
	          "rightImg": "https://gw.alicdn.com/tps/TB1F2fJLVXXXXccXFXXXXXXXXXX-750-230.jpg",
	          "rightUrl": "https://chaoshi.m.tmall.com/"
	        }, {
	          "leftImg": "https://gw.alicdn.com/tps/TB1F2fJLVXXXXccXFXXXXXXXXXX-750-230.jpg",
	          "leftUrl": "https://chaoshi.m.tmall.com/",
	          "rightImg": "https://gw.alicdn.com/tps/TB1F2fJLVXXXXccXFXXXXXXXXXX-750-230.jpg",
	          "rightUrl": "https://chaoshi.m.tmall.com/"
	        }, {
	          "leftImg": "https://gw.alicdn.com/tps/TB1F2fJLVXXXXccXFXXXXXXXXXX-750-230.jpg",
	          "leftUrl": "https://chaoshi.m.tmall.com/",
	          "rightImg": "https://gw.alicdn.com/tps/TB1F2fJLVXXXXccXFXXXXXXXXXX-750-230.jpg",
	          "rightUrl": "https://chaoshi.m.tmall.com/"
	        }, {
	          "leftImg": "https://gw.alicdn.com/tps/TB1F2fJLVXXXXccXFXXXXXXXXXX-750-230.jpg",
	          "leftUrl": "https://chaoshi.m.tmall.com/",
	          "rightImg": "https://gw.alicdn.com/tps/TB1F2fJLVXXXXccXFXXXXXXXXXX-750-230.jpg",
	          "rightUrl": "https://chaoshi.m.tmall.com/"
	        }, {
	          "leftImg": "https://gw.alicdn.com/tps/TB1F2fJLVXXXXccXFXXXXXXXXXX-750-230.jpg",
	          "leftUrl": "https://chaoshi.m.tmall.com/",
	          "rightImg": "https://gw.alicdn.com/tps/TB1F2fJLVXXXXccXFXXXXXXXXXX-750-230.jpg",
	          "rightUrl": "https://chaoshi.m.tmall.com/"
	        }, {
	          "leftImg": "https://gw.alicdn.com/tps/TB1F2fJLVXXXXccXFXXXXXXXXXX-750-230.jpg",
	          "leftUrl": "https://chaoshi.m.tmall.com/",
	          "rightImg": "https://gw.alicdn.com/tps/TB1F2fJLVXXXXccXFXXXXXXXXXX-750-230.jpg",
	          "rightUrl": "https://chaoshi.m.tmall.com/"
	        }, {
	          "leftImg": "https://gw.alicdn.com/tps/TB1F2fJLVXXXXccXFXXXXXXXXXX-750-230.jpg",
	          "leftUrl": "https://chaoshi.m.tmall.com/",
	          "rightImg": "https://gw.alicdn.com/tps/TB1F2fJLVXXXXccXFXXXXXXXXXX-750-230.jpg",
	          "rightUrl": "https://chaoshi.m.tmall.com/"
	        }, {
	          "leftImg": "https://gw.alicdn.com/tps/TB1F2fJLVXXXXccXFXXXXXXXXXX-750-230.jpg",
	          "leftUrl": "https://chaoshi.m.tmall.com/",
	          "rightImg": "https://gw.alicdn.com/tps/TB1F2fJLVXXXXccXFXXXXXXXXXX-750-230.jpg",
	          "rightUrl": "https://chaoshi.m.tmall.com/"
	        }]
	      },
	      "match": {
	        "floorTitle": "https://gw.alicdn.com/tps/TB1._64LVXXXXatXVXXXXXXXXXX-750-100.jpg",
	        "isShowTmSpecBanner": true,
	        "tmSpecBanner": {
	          "img": "https://gw.alicdn.com/tps/TB1F2fJLVXXXXccXFXXXXXXXXXX-750-230.jpg",
	          "url": "https://chaoshi.m.tmall.com/"
	        },
	        "bannerItems": [{
	          "leftImg": "https://gw.alicdn.com/tps/TB1OUnILVXXXXcoXFXXXXXXXXXX-374-416.jpg",
	          "leftUrl": "https://chaoshi.m.tmall.com/",
	          "rightImg": "https://gw.alicdn.com/tps/TB1OUnILVXXXXcoXFXXXXXXXXXX-374-416.jpg",
	          "rightUrl": "https://chaoshi.m.tmall.com/"
	        }, {
	          "leftImg": "https://gw.alicdn.com/tps/TB1OUnILVXXXXcoXFXXXXXXXXXX-374-416.jpg",
	          "leftUrl": "https://chaoshi.m.tmall.com/",
	          "rightImg": "https://gw.alicdn.com/tps/TB1OUnILVXXXXcoXFXXXXXXXXXX-374-416.jpg",
	          "rightUrl": "https://chaoshi.m.tmall.com/"
	        }]
	      }
	    };
	    this.simLoadingData(this.cnt++);
	  },
	  methods: {
	    handleRefresh: function handleRefresh(e) {
	      var vm = this;
	      vm.displayRefresh = 'show';
	      if (timer) {
	        timer.setTimeout(function () {
	          vm.displayRefresh = 'hide';
	        }, 2000);
	      } else {
	        modal.alert({
	          message: 'refreshing.'
	        }, function () {
	          vm.displayRefresh = 'hide';
	        });
	      }
	    },
	    handleLoading: function handleLoading(e) {
	      var vm = this;
	      vm.displayLoading = 'show';
	      modal.toast({
	        message: 'loading.'
	      });
	      if (timer) {
	        timer.setTimeout(function () {
	          vm.simLoadingData(vm.cnt++);
	          vm.displayLoading = 'hide';
	        }, 2000);
	      } else {
	        vm.simLoadingData(vm.cnt++);
	        vm.displayLoading = 'hide';
	      }
	    },
	    simLoadingData: function simLoadingData(num) {
	      if (num >= 3) {
	        modal.toast({
	          message: '已到底部'
	        });
	        return;
	      }
	      this['show' + num] = true;
	    }
	  }
	};

/***/ }),
/* 231 */
/***/ (function(module, exports, __webpack_require__) {

	var disposed = false
	function injectStyle (ssrContext) {
	  if (disposed) return
	  __webpack_require__(232)
	}
	var Component = __webpack_require__(7)(
	  /* script */
	  __webpack_require__(234),
	  /* template */
	  __webpack_require__(244),
	  /* styles */
	  injectStyle,
	  /* scopeId */
	  "data-v-f46caf5e",
	  /* moduleIdentifier (server only) */
	  null
	)
	Component.options.__file = "/Users/Hanks/Codes/work/incubator-weex/examples/vue/showcase/include/headlines.vue"
	if (Component.esModule && Object.keys(Component.esModule).some(function (key) {return key !== "default" && key.substr(0, 2) !== "__"})) {console.error("named exports are not supported in *.vue files.")}
	if (Component.options.functional) {console.error("[vue-loader] headlines.vue: functional components are not supported with templates, they should use render functions.")}

	/* hot reload */
	if (false) {(function () {
	  var hotAPI = require("vue-hot-reload-api")
	  hotAPI.install(require("vue"), false)
	  if (!hotAPI.compatible) return
	  module.hot.accept()
	  if (!module.hot.data) {
	    hotAPI.createRecord("data-v-f46caf5e", Component.options)
	  } else {
	    hotAPI.reload("data-v-f46caf5e", Component.options)
	  }
	  module.hot.dispose(function (data) {
	    disposed = true
	  })
	})()}

	module.exports = Component.exports


/***/ }),
/* 232 */
/***/ (function(module, exports, __webpack_require__) {

	// style-loader: Adds some css to the DOM by adding a <style> tag

	// load the styles
	var content = __webpack_require__(233);
	if(typeof content === 'string') content = [[module.id, content, '']];
	if(content.locals) module.exports = content.locals;
	// add the styles to the DOM
	var update = __webpack_require__(5)("b0580bee", content, false);
	// Hot Module Replacement
	if(false) {
	 // When the styles change, update the <style> tags
	 if(!content.locals) {
	   module.hot.accept("!!../../../../node_modules/css-loader/index.js!../../../../node_modules/vue-loader/lib/style-compiler/index.js?{\"vue\":true,\"id\":\"data-v-f46caf5e\",\"scoped\":true,\"hasInlineConfig\":false}!../../../../node_modules/vue-loader/lib/selector.js?type=styles&index=0!./headlines.vue", function() {
	     var newContent = require("!!../../../../node_modules/css-loader/index.js!../../../../node_modules/vue-loader/lib/style-compiler/index.js?{\"vue\":true,\"id\":\"data-v-f46caf5e\",\"scoped\":true,\"hasInlineConfig\":false}!../../../../node_modules/vue-loader/lib/selector.js?type=styles&index=0!./headlines.vue");
	     if(typeof newContent === 'string') newContent = [[module.id, newContent, '']];
	     update(newContent);
	   });
	 }
	 // When the module is disposed, remove the <style> tags
	 module.hot.dispose(function() { update(); });
	}

/***/ }),
/* 233 */
/***/ (function(module, exports, __webpack_require__) {

	exports = module.exports = __webpack_require__(4)();
	// imports


	// module
	exports.push([module.id, "\n.banner[data-v-f46caf5e] {\n  width: 750;\n  height: 782;\n}\n.share-container[data-v-f46caf5e] {\n  position: absolute;\n  right: 100;\n  top: 15;\n  z-index: 100;\n  font-size: 20;\n  color: #ffffff;\n  background-color: #000000;\n  border-radius: 17;\n  width: 110;\n  height: 35;\n  justify-content: center;\n  align-items: center;\n}\n.rule-container[data-v-f46caf5e] {\n  position: absolute;\n  right: 22;\n  top: 15;\n  z-index: 100;\n  font-size: 20;\n  color: #ffffff;\n  background-color: #000000;\n  border-radius: 17;\n  width: 70;\n  height: 35;\n  justify-content: center;\n}\n.announce[data-v-f46caf5e] {\n  background-color: #f5f3f4;\n  width: 716;\n  height: 286;\n  position: absolute;\n  bottom: 17;\n  left: 17;\n  border-radius: 5;\n}\n.announce-hd[data-v-f46caf5e] {\n  width: 700;\n  height: 90;\n  margin-top: 8;\n  margin-bottom: 8;\n  margin-left: 5;\n  margin-right: 5;\n}\n", ""]);

	// exports


/***/ }),
/* 234 */
/***/ (function(module, exports, __webpack_require__) {

	'use strict';

	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//

	module.exports = {
	  components: {
	    link: __webpack_require__(235),
	    banners: __webpack_require__(238)
	  },
	  props: {
	    ds: {
	      default: function _default() {
	        return {};
	      }
	    }
	  },
	  data: function data() {
	    return {
	      NUMBER_233: 233,
	      NUMBER_172: 172,
	      NUMBER_3: 3
	    };
	  }
	};

/***/ }),
/* 235 */
/***/ (function(module, exports, __webpack_require__) {

	var disposed = false
	var Component = __webpack_require__(7)(
	  /* script */
	  __webpack_require__(236),
	  /* template */
	  __webpack_require__(237),
	  /* styles */
	  null,
	  /* scopeId */
	  null,
	  /* moduleIdentifier (server only) */
	  null
	)
	Component.options.__file = "/Users/Hanks/Codes/work/incubator-weex/examples/vue/showcase/include/link.vue"
	if (Component.esModule && Object.keys(Component.esModule).some(function (key) {return key !== "default" && key.substr(0, 2) !== "__"})) {console.error("named exports are not supported in *.vue files.")}
	if (Component.options.functional) {console.error("[vue-loader] link.vue: functional components are not supported with templates, they should use render functions.")}

	/* hot reload */
	if (false) {(function () {
	  var hotAPI = require("vue-hot-reload-api")
	  hotAPI.install(require("vue"), false)
	  if (!hotAPI.compatible) return
	  module.hot.accept()
	  if (!module.hot.data) {
	    hotAPI.createRecord("data-v-83558590", Component.options)
	  } else {
	    hotAPI.reload("data-v-83558590", Component.options)
	  }
	  module.hot.dispose(function (data) {
	    disposed = true
	  })
	})()}

	module.exports = Component.exports


/***/ }),
/* 236 */
/***/ (function(module, exports) {

	'use strict';

	//
	//
	//

	var modal = weex.requireModule('modal');
	module.exports = {
	  props: ['text', 'href'],
	  methods: {
	    clickHandler: function clickHandler() {
	      modal.toast({
	        message: 'click',
	        duration: 1
	      });
	    }
	  }
	};

/***/ }),
/* 237 */
/***/ (function(module, exports, __webpack_require__) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('text', {
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    on: {
	      "click": _vm.clickHandler
	    }
	  }, [_vm._v(_vm._s(_vm.text))])
	},staticRenderFns: []}
	module.exports.render._withStripped = true
	if (false) {
	  module.hot.accept()
	  if (module.hot.data) {
	     require("vue-hot-reload-api").rerender("data-v-83558590", module.exports)
	  }
	}

/***/ }),
/* 238 */
/***/ (function(module, exports, __webpack_require__) {

	var disposed = false
	var Component = __webpack_require__(7)(
	  /* script */
	  __webpack_require__(239),
	  /* template */
	  __webpack_require__(243),
	  /* styles */
	  null,
	  /* scopeId */
	  null,
	  /* moduleIdentifier (server only) */
	  null
	)
	Component.options.__file = "/Users/Hanks/Codes/work/incubator-weex/examples/vue/showcase/include/banners.vue"
	if (Component.esModule && Object.keys(Component.esModule).some(function (key) {return key !== "default" && key.substr(0, 2) !== "__"})) {console.error("named exports are not supported in *.vue files.")}
	if (Component.options.functional) {console.error("[vue-loader] banners.vue: functional components are not supported with templates, they should use render functions.")}

	/* hot reload */
	if (false) {(function () {
	  var hotAPI = require("vue-hot-reload-api")
	  hotAPI.install(require("vue"), false)
	  if (!hotAPI.compatible) return
	  module.hot.accept()
	  if (!module.hot.data) {
	    hotAPI.createRecord("data-v-5c380c79", Component.options)
	  } else {
	    hotAPI.reload("data-v-5c380c79", Component.options)
	  }
	  module.hot.dispose(function (data) {
	    disposed = true
	  })
	})()}

	module.exports = Component.exports


/***/ }),
/* 239 */
/***/ (function(module, exports, __webpack_require__) {

	'use strict';

	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//

	module.exports = {
	  components: {
	    banner: __webpack_require__(240)
	  },
	  props: ['ds', 'width', 'height', 'space', 'direction']
	};

/***/ }),
/* 240 */
/***/ (function(module, exports, __webpack_require__) {

	var disposed = false
	var Component = __webpack_require__(7)(
	  /* script */
	  __webpack_require__(241),
	  /* template */
	  __webpack_require__(242),
	  /* styles */
	  null,
	  /* scopeId */
	  null,
	  /* moduleIdentifier (server only) */
	  null
	)
	Component.options.__file = "/Users/Hanks/Codes/work/incubator-weex/examples/vue/showcase/include/banner.vue"
	if (Component.esModule && Object.keys(Component.esModule).some(function (key) {return key !== "default" && key.substr(0, 2) !== "__"})) {console.error("named exports are not supported in *.vue files.")}
	if (Component.options.functional) {console.error("[vue-loader] banner.vue: functional components are not supported with templates, they should use render functions.")}

	/* hot reload */
	if (false) {(function () {
	  var hotAPI = require("vue-hot-reload-api")
	  hotAPI.install(require("vue"), false)
	  if (!hotAPI.compatible) return
	  module.hot.accept()
	  if (!module.hot.data) {
	    hotAPI.createRecord("data-v-4d2dd1ea", Component.options)
	  } else {
	    hotAPI.reload("data-v-4d2dd1ea", Component.options)
	  }
	  module.hot.dispose(function (data) {
	    disposed = true
	  })
	})()}

	module.exports = Component.exports


/***/ }),
/* 241 */
/***/ (function(module, exports) {

	'use strict';

	//
	//
	//
	//

	var modal = weex.requireModule('modal');
	module.exports = {
	  props: ['width', 'height', 'src', 'href'],
	  data: function data() {
	    return {
	      quality: 'normal'
	    };
	  },
	  methods: {
	    clickHandler: function clickHandler() {
	      modal.toast({
	        message: 'click',
	        duration: 1
	      });
	    }
	  }
	};

/***/ }),
/* 242 */
/***/ (function(module, exports, __webpack_require__) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('image', {
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle({
	      width: _vm.width,
	      height: _vm.height
	    })),
	    attrs: {
	      "src": _vm.src,
	      "image-quality": _vm.quality
	    },
	    on: {
	      "click": _vm.clickHandler
	    }
	  })
	},staticRenderFns: []}
	module.exports.render._withStripped = true
	if (false) {
	  module.hot.accept()
	  if (module.hot.data) {
	     require("vue-hot-reload-api").rerender("data-v-4d2dd1ea", module.exports)
	  }
	}

/***/ }),
/* 243 */
/***/ (function(module, exports, __webpack_require__) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', [(_vm.direction === 'row') ? _c('div', {
	    staticStyle: _vm.$processStyle({
	      "flex-direction": "row"
	    }),
	    style: (_vm.$processStyle(undefined))
	  }, _vm._l((_vm.ds), function(item, i) {
	    return _c('div', {
	      key: i,
	      staticStyle: _vm.$processStyle(undefined),
	      style: (_vm.$processStyle({
	        width: _vm.width,
	        height: _vm.height,
	        marginLeft: (i % _vm.ds.length ? _vm.space : 0)
	      }))
	    }, [_c('banner', {
	      staticStyle: _vm.$processStyle(undefined),
	      style: (_vm.$processStyle(undefined)),
	      attrs: {
	        "width": _vm.width,
	        "height": _vm.height,
	        "src": item.img,
	        "href": item.url
	      }
	    })], 1)
	  })) : _vm._e(), _vm._v(" "), (_vm.direction === 'column') ? _c('div', _vm._l((_vm.ds), function(item, i) {
	    return _c('div', {
	      key: i,
	      staticStyle: _vm.$processStyle(undefined),
	      style: (_vm.$processStyle({
	        width: _vm.width,
	        height: _vm.height,
	        marginTop: (i % _vm.ds.length ? _vm.space : 0)
	      }))
	    }, [_c('banner', {
	      staticStyle: _vm.$processStyle(undefined),
	      style: (_vm.$processStyle(undefined)),
	      attrs: {
	        "width": _vm.width,
	        "height": _vm.height,
	        "src": item.img,
	        "href": item.url
	      }
	    })], 1)
	  })) : _vm._e()])
	},staticRenderFns: []}
	module.exports.render._withStripped = true
	if (false) {
	  module.hot.accept()
	  if (module.hot.data) {
	     require("vue-hot-reload-api").rerender("data-v-5c380c79", module.exports)
	  }
	}

/***/ }),
/* 244 */
/***/ (function(module, exports, __webpack_require__) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', {
	    staticClass: "container",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined))
	  }, [_c('image', {
	    staticClass: "banner",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "src": _vm.ds.topBanner
	    }
	  }), _vm._v(" "), _c('div', {
	    staticClass: "rule-container",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined))
	  }, [_c('link', {
	    staticStyle: _vm.$processStyle({
	      "font-size": "22px",
	      "color": "#ffffff",
	      "text-align": "center"
	    }),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "text": "帮助",
	      "href": _vm.ds.ruleLink
	    }
	  })]), _vm._v(" "), _c('div', {
	    staticClass: "announce",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined))
	  }, [_c('image', {
	    staticClass: "announce-hd",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "src": _vm.ds.announceHdBanner
	    }
	  }), _vm._v(" "), _c('banners', {
	    staticStyle: _vm.$processStyle({
	      "margin-left": "6",
	      "margin-right": "6"
	    }),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "ds": _vm.ds.bannerItems,
	      "direction": "row",
	      "width": _vm.NUMBER_233,
	      "height": _vm.NUMBER_172,
	      "space": _vm.NUMBER_3
	    }
	  })], 1)])
	},staticRenderFns: []}
	module.exports.render._withStripped = true
	if (false) {
	  module.hot.accept()
	  if (module.hot.data) {
	     require("vue-hot-reload-api").rerender("data-v-f46caf5e", module.exports)
	  }
	}

/***/ }),
/* 245 */
/***/ (function(module, exports, __webpack_require__) {

	var disposed = false
	function injectStyle (ssrContext) {
	  if (disposed) return
	  __webpack_require__(246)
	}
	var Component = __webpack_require__(7)(
	  /* script */
	  __webpack_require__(248),
	  /* template */
	  __webpack_require__(249),
	  /* styles */
	  injectStyle,
	  /* scopeId */
	  "data-v-2db23588",
	  /* moduleIdentifier (server only) */
	  null
	)
	Component.options.__file = "/Users/Hanks/Codes/work/incubator-weex/examples/vue/showcase/include/category.vue"
	if (Component.esModule && Object.keys(Component.esModule).some(function (key) {return key !== "default" && key.substr(0, 2) !== "__"})) {console.error("named exports are not supported in *.vue files.")}
	if (Component.options.functional) {console.error("[vue-loader] category.vue: functional components are not supported with templates, they should use render functions.")}

	/* hot reload */
	if (false) {(function () {
	  var hotAPI = require("vue-hot-reload-api")
	  hotAPI.install(require("vue"), false)
	  if (!hotAPI.compatible) return
	  module.hot.accept()
	  if (!module.hot.data) {
	    hotAPI.createRecord("data-v-2db23588", Component.options)
	  } else {
	    hotAPI.reload("data-v-2db23588", Component.options)
	  }
	  module.hot.dispose(function (data) {
	    disposed = true
	  })
	})()}

	module.exports = Component.exports


/***/ }),
/* 246 */
/***/ (function(module, exports, __webpack_require__) {

	// style-loader: Adds some css to the DOM by adding a <style> tag

	// load the styles
	var content = __webpack_require__(247);
	if(typeof content === 'string') content = [[module.id, content, '']];
	if(content.locals) module.exports = content.locals;
	// add the styles to the DOM
	var update = __webpack_require__(5)("1289de1a", content, false);
	// Hot Module Replacement
	if(false) {
	 // When the styles change, update the <style> tags
	 if(!content.locals) {
	   module.hot.accept("!!../../../../node_modules/css-loader/index.js!../../../../node_modules/vue-loader/lib/style-compiler/index.js?{\"vue\":true,\"id\":\"data-v-2db23588\",\"scoped\":true,\"hasInlineConfig\":false}!../../../../node_modules/vue-loader/lib/selector.js?type=styles&index=0!./category.vue", function() {
	     var newContent = require("!!../../../../node_modules/css-loader/index.js!../../../../node_modules/vue-loader/lib/style-compiler/index.js?{\"vue\":true,\"id\":\"data-v-2db23588\",\"scoped\":true,\"hasInlineConfig\":false}!../../../../node_modules/vue-loader/lib/selector.js?type=styles&index=0!./category.vue");
	     if(typeof newContent === 'string') newContent = [[module.id, newContent, '']];
	     update(newContent);
	   });
	 }
	 // When the module is disposed, remove the <style> tags
	 module.hot.dispose(function() { update(); });
	}

/***/ }),
/* 247 */
/***/ (function(module, exports, __webpack_require__) {

	exports = module.exports = __webpack_require__(4)();
	// imports


	// module
	exports.push([module.id, "\n.title[data-v-2db23588] {\n  width: 750;\n  height: 100;\n}\n.shop-list[data-v-2db23588] {\n  flex-direction: row;\n}\n.shop-img-container[data-v-2db23588] {\n  margin-right: 2;\n}\n.sub-item[data-v-2db23588] {\n  position: absolute;\n  width: 248;\n  height: 241;\n  align-items: center;\n  background-color:#ffffff;\n}\n.sub-item-bg[data-v-2db23588] {\n  width: 248;\n  height: 86;\n}\n.sub-item-title[data-v-2db23588] {\n  position: absolute;\n  left: 0;\n  top: 0;\n  width: 248;\n  text-align: center;\n}\n.industry-title[data-v-2db23588] {\n  font-size: 28;\n  color: #484848;\n  text-align: center;\n  margin-top: 10;\n  height:34;\n}\n.industry-benefit[data-v-2db23588] {\n  font-size: 24;\n  color: #999999;\n  text-align: center;\n}\n", ""]);

	// exports


/***/ }),
/* 248 */
/***/ (function(module, exports, __webpack_require__) {

	"use strict";

	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//

	var demoContent = {
	  "main": [{
	    "shopList": [{
	      "id": "619123122",
	      "picUrl": "//img.alicdn.com/tps/i1/TB1Cl5MLVXXXXXTaXXXSutbFXXX.jpg",
	      "finalScore": "0.884765209440857",
	      "shopUrl": "http://liangpinpuzi.tmall.com/campaign-3735-7.htm",
	      "entityType": "SHOP",
	      "brandId": "7724367"
	    }, {
	      "id": "880734502",
	      "picUrl": "//img.alicdn.com/tps/i1/TB1HN2XLVXXXXbkXFXXSutbFXXX.jpg",
	      "finalScore": "0.8822246866512714",
	      "shopUrl": "http://sanzhisongshu.tmall.com/campaign-3735-7.htm",
	      "entityType": "SHOP",
	      "brandId": "147280915"
	    }, {
	      "id": "392147177",
	      "picUrl": "//img.alicdn.com/tps/i1/TB1yq6dLVXXXXbXXpXXSutbFXXX.jpg",
	      "finalScore": "0.8805854724243631",
	      "shopUrl": "http://zhouheiya.tmall.com/campaign-3735-6.htm",
	      "entityType": "SHOP",
	      "brandId": "111496"
	    }, {
	      "id": "2081058060",
	      "picUrl": "//img.alicdn.com/tps/i1/TB1w4jxLVXXXXaxXXXXtKXbFXXX.gif",
	      "finalScore": "0.8640439551049249",
	      "shopUrl": "http://rongxintang.tmall.com/campaign-3735-6.htm",
	      "entityType": "SHOP",
	      "brandId": "8709890"
	    }],
	    "finalScore": "4.657333333333334",
	    "industryUrl": null,
	    "entityType": "OTHER",
	    "colour": "#fff5a3",
	    "industryTitle": null,
	    "bannerUrl": "//pages.tmall.com/wow/act/15617/newfood?abbucket=_AB-M1011_B14&acm=03654.1003.1.735362&aldid=KXPpnvEH&abtest=_AB-LR1011-PR1011&scm=1003.1.03654.OTHER_12_735362&pos=1",
	    "leftBannerImg": "http://img.alicdn.com/tps/i1/TB1LHh4MXXXXXbsXpXXeWjm2pXX-750-240.jpg",
	    "_pos_": "1",
	    "industryBenefit": null,
	    "industryId": "12",
	    "industryImg": null,
	    "rightBannerImg": "//img.alicdn.com/tps/i4/TB1wshUMXXXXXXaXVXXeWjm2pXX-750-240.jpg"
	  }, {
	    "shopList": [{
	      "id": "520557274",
	      "picUrl": "//img.alicdn.com/tps/i1/TB1aMjXLVXXXXbUaXXXSutbFXXX.jpg",
	      "finalScore": "0.8986382989845325",
	      "shopUrl": "http://newbalance.tmall.com/campaign-3735-20.htm",
	      "entityType": "SHOP",
	      "brandId": "20584"
	    }, {
	      "id": "746866993",
	      "picUrl": "//img.alicdn.com/tps/i1/TB1pXG9LVXXXXaLXFXXSutbFXXX.jpg",
	      "finalScore": "0.8917855242977327",
	      "shopUrl": "http://vans.tmall.com/campaign-3735-19.htm",
	      "entityType": "SHOP",
	      "brandId": "29529"
	    }, {
	      "id": "1612713147",
	      "picUrl": "//img.alicdn.com/tps/i1/TB1jHfXLVXXXXXPXFXXSutbFXXX.jpg",
	      "finalScore": "0.8895943606442631",
	      "shopUrl": "http://dcshoecousa.tmall.com/campaign-3735-8.htm",
	      "entityType": "SHOP",
	      "brandId": "3851662"
	    }, {
	      "id": "2379251418",
	      "picUrl": "//img.alicdn.com/tps/i1/TB1sbLlLVXXXXbeXXXXSutbFXXX.jpg",
	      "finalScore": "0.8881539668874383",
	      "shopUrl": "http://baijinydhw.tmall.com/campaign-3735-2.htm",
	      "entityType": "SHOP",
	      "brandId": "20579"
	    }],
	    "finalScore": "4.545333333333334",
	    "industryUrl": null,
	    "entityType": "OTHER",
	    "colour": "#cbdbfe",
	    "industryTitle": null,
	    "bannerUrl": "//pages.tmall.com/wow/act/15617/ydhw?abbucket=_AB-M1011_B14&acm=03654.1003.1.735362&aldid=KXPpnvEH&abtest=_AB-LR1011-PR1011&scm=1003.1.03654.OTHER_8_735362&pos=2",
	    "leftBannerImg": "http://img.alicdn.com/tps/i3/TB1h4kzLVXXXXawXpXXeWjm2pXX-750-240.jpg",
	    "_pos_": "2",
	    "industryBenefit": null,
	    "industryId": "8",
	    "industryImg": null,
	    "rightBannerImg": "//img.alicdn.com/tps/i4/TB19VMjLVXXXXaeaXXXeWjm2pXX-750-240.jpg"
	  }, {
	    "shopList": [{
	      "id": "811383091",
	      "picUrl": "//img.alicdn.com/tps/i1/TB1q49ZLVXXXXaJXVXXSutbFXXX.jpg",
	      "finalScore": "0.8723682716749482",
	      "shopUrl": "http://tongrentangbj.tmall.com/campaign-3735-9.htm",
	      "entityType": "SHOP",
	      "brandId": "44652"
	    }, {
	      "id": "838914626",
	      "picUrl": "//img.alicdn.com/tps/i1/TB1GQYbLVXXXXaXXVXXSutbFXXX.jpg",
	      "finalScore": "0.5027131908086584",
	      "shopUrl": "http://jouo.tmall.com/campaign-3735-3.htm",
	      "entityType": "SHOP",
	      "brandId": "14170081"
	    }, {
	      "id": "1077716829",
	      "picUrl": "//img.alicdn.com/tps/i1/TB1rdW2LVXXXXagapXXSutbFXXX.jpg",
	      "finalScore": "0.5015387557024262",
	      "shopUrl": "http://wetcode.tmall.com/campaign-3735-14.htm",
	      "entityType": "SHOP",
	      "brandId": "3675642"
	    }, {
	      "id": "1664976033",
	      "picUrl": "//img.alicdn.com/tps/i1/TB1Q.rpLVXXXXXiXpXXSutbFXXX.jpg",
	      "finalScore": "0.5014897109669442",
	      "shopUrl": "http://hanhoohzp.tmall.com/campaign-3735-17.htm",
	      "entityType": "SHOP",
	      "brandId": "78888695"
	    }],
	    "finalScore": "4.526666666666666",
	    "industryUrl": null,
	    "entityType": "OTHER",
	    "colour": "#ffd1b6",
	    "industryTitle": null,
	    "bannerUrl": "//pages.tmall.com/wow/act/15617/getbeauty?abbucket=_AB-M1011_B14&acm=03654.1003.1.735362&aldid=KXPpnvEH&abtest=_AB-LR1011-PR1011&scm=1003.1.03654.OTHER_2_735362&pos=3",
	    "leftBannerImg": "http://img.alicdn.com/tps/i4/TB1Djt9MXXXXXanXXXXeWjm2pXX-750-240.jpg",
	    "_pos_": "3",
	    "industryBenefit": null,
	    "industryId": "2",
	    "industryImg": null,
	    "rightBannerImg": "//img.alicdn.com/tps/i3/TB1_gVQMXXXXXX9XVXXeWjm2pXX-750-240.jpg"
	  }, {
	    "shopList": [{
	      "id": "2153169655",
	      "picUrl": "//img.alicdn.com/tps/i1/TB1nlK7LVXXXXaRXFXXSutbFXXX.jpg",
	      "finalScore": "0.8820210808835905",
	      "shopUrl": "http://ctkicks.tmall.com/campaign-3735-15.htm",
	      "entityType": "SHOP",
	      "brandId": "20584"
	    }, {
	      "id": "2784101115",
	      "picUrl": "//img.alicdn.com/tps/i1/TB1RrS.LVXXXXcrXpXXSutbFXXX.jpg",
	      "finalScore": "0.5930383227012586",
	      "shopUrl": "http://tomtailor.tmall.com/campaign-3735-0.htm",
	      "entityType": "SHOP",
	      "brandId": "213474060"
	    }, {
	      "id": "2183813726",
	      "picUrl": "//img.alicdn.com/tps/i1/TB1kmS_LVXXXXaBaXXXSutbFXXX.jpg",
	      "finalScore": "0.5882558957771681",
	      "shopUrl": "http://huaiyuan.tmall.com/campaign-3735-5.htm",
	      "entityType": "SHOP",
	      "brandId": "29465"
	    }, {
	      "id": "1785908005",
	      "picUrl": "//img.alicdn.com/tps/i1/TB1dB6zLVXXXXcJXXXXSutbFXXX.jpg",
	      "finalScore": "0.5694211862124341",
	      "shopUrl": "http://axonus.tmall.com/campaign-3735-1.htm",
	      "entityType": "SHOP",
	      "brandId": "115035841"
	    }],
	    "finalScore": "4.413955555555555",
	    "industryUrl": null,
	    "entityType": "OTHER",
	    "colour": "#b9eaf2",
	    "industryTitle": null,
	    "bannerUrl": "//pages.tmall.com/wow/act/15617/guojixfs?abbucket=_AB-M1011_B14&acm=03654.1003.1.735362&aldid=KXPpnvEH&abtest=_AB-LR1011-PR1011&scm=1003.1.03654.OTHER_1_735362&pos=4",
	    "leftBannerImg": "http://img.alicdn.com/tps/i3/TB1_T7ALVXXXXXOXXXXeWjm2pXX-750-240.jpg",
	    "_pos_": "4",
	    "industryBenefit": null,
	    "industryId": "1",
	    "industryImg": null,
	    "rightBannerImg": "//img.alicdn.com/tps/i2/TB1s7koLVXXXXafXFXXeWjm2pXX-750-240.jpg"
	  }, {
	    "shopList": [{
	      "id": "2453054335",
	      "picUrl": "//img.alicdn.com/tps/i1/TB1RX_vLVXXXXchXpXXSutbFXXX.jpg",
	      "finalScore": "0.8463447899590267",
	      "shopUrl": "http://nanshizixing.tmall.com/campaign-3735-2.htm",
	      "entityType": "SHOP",
	      "brandId": "844502560"
	    }, {
	      "id": "1041773234",
	      "picUrl": "//img.alicdn.com/tps/i1/TB1Q7jpLVXXXXX8XpXXSutbFXXX.jpg",
	      "finalScore": "0.560952734503075",
	      "shopUrl": "http://dapu.tmall.com/campaign-3735-8.htm",
	      "entityType": "SHOP",
	      "brandId": "14493763"
	    }, {
	      "id": "300031438",
	      "picUrl": "//img.alicdn.com/tps/i1/TB13VC4LVXXXXarXVXXSutbFXXX.jpg",
	      "finalScore": "0.5450674634839442",
	      "shopUrl": "http://kangerxin.tmall.com/campaign-3735-10.htm",
	      "entityType": "SHOP",
	      "brandId": "3781905"
	    }, {
	      "id": "92042735",
	      "picUrl": "//img.alicdn.com/tps/i1/TB1PxYcLVXXXXasaXXXSutbFXXX.jpg",
	      "finalScore": "0.5448023228053981",
	      "shopUrl": "http://shuixing.tmall.com/campaign-3735-10.htm",
	      "entityType": "SHOP",
	      "brandId": "3685660"
	    }],
	    "finalScore": "4.13",
	    "industryUrl": null,
	    "entityType": "OTHER",
	    "colour": "#bbffe5",
	    "industryTitle": null,
	    "bannerUrl": "//pages.tmall.com/wow/act/15617/jfjs?abbucket=_AB-M1011_B14&acm=03654.1003.1.735362&aldid=KXPpnvEH&abtest=_AB-LR1011-PR1011&scm=1003.1.03654.OTHER_4_735362&pos=5",
	    "leftBannerImg": "http://img.alicdn.com/tps/i3/TB1jeoCLVXXXXcoXXXXeWjm2pXX-750-240.jpg",
	    "_pos_": "5",
	    "industryBenefit": null,
	    "industryId": "4",
	    "industryImg": null,
	    "rightBannerImg": "//img.alicdn.com/tps/i4/TB1C5.fLVXXXXb3aXXXeWjm2pXX-750-240.jpg"
	  }, {
	    "shopList": [{
	      "id": "822428555",
	      "picUrl": "//img.alicdn.com/tps/i1/TB1g46aLVXXXXcUXVXXSutbFXXX.jpg",
	      "finalScore": "0.9183458776971917",
	      "shopUrl": "http://playboyyd.tmall.com/campaign-3735-11.htm",
	      "entityType": "SHOP",
	      "brandId": "29510"
	    }, {
	      "id": "196993935",
	      "picUrl": "//img.alicdn.com/tps/i1/TB1pHG8LVXXXXbraXXXSutbFXXX.jpg",
	      "finalScore": "0.9105525246667775",
	      "shopUrl": "https://uniqlo.tmall.com/campaign-3735-56.htm",
	      "entityType": "SHOP",
	      "brandId": "29527"
	    }, {
	      "id": "505753958",
	      "picUrl": "//img.alicdn.com/tps/i1/TB1WKYyLVXXXXXHXXXXSutbFXXX.jpg",
	      "finalScore": "0.90750966370726",
	      "shopUrl": "http://bonas.tmall.com/campaign-3735-11.htm",
	      "entityType": "SHOP",
	      "brandId": "3486580"
	    }, {
	      "id": "2113823580",
	      "picUrl": "//img.alicdn.com/tps/i1/TB1itHkLVXXXXX9XFXXSutbFXXX.jpg",
	      "finalScore": "0.5900977955635289",
	      "shopUrl": "http://kafanya.tmall.com/campaign-3735-7.htm",
	      "entityType": "SHOP",
	      "brandId": "110684218"
	    }],
	    "finalScore": "4.088",
	    "industryUrl": null,
	    "entityType": "OTHER",
	    "colour": "#ffc7c9",
	    "industryTitle": null,
	    "bannerUrl": "//pages.tmall.com/wow/act/15617/neiyi328?abbucket=_AB-M1011_B14&acm=03654.1003.1.735362&aldid=KXPpnvEH&abtest=_AB-LR1011-PR1011&scm=1003.1.03654.OTHER_9_735362&pos=6",
	    "leftBannerImg": "http://img.alicdn.com/tps/i1/TB1tl3tLVXXXXbbXFXXeWjm2pXX-750-240.jpg",
	    "_pos_": "6",
	    "industryBenefit": null,
	    "industryId": "9",
	    "industryImg": null,
	    "rightBannerImg": "//img.alicdn.com/tps/i2/TB1AkIyLVXXXXbjXpXXeWjm2pXX-750-240.jpg"
	  }],
	  "sub": [{
	    "shopList": null,
	    "finalScore": "4.033333333333333",
	    "industryUrl": "//pages.tmall.com/wow/act/15617/zbps?abbucket=_AB-M1011_B14&acm=03654.1003.1.735362&aldid=KXPpnvEH&abtest=_AB-LR1011-PR1011&scm=1003.1.03654.OTHER_3_735362&pos=7",
	    "entityType": "OTHER",
	    "colour": "#bbffe5",
	    "industryTitle": "珠宝配饰",
	    "bannerUrl": null,
	    "leftBannerImg": null,
	    "_pos_": "7",
	    "industryBenefit": "大牌1折秒杀",
	    "industryId": "3",
	    "industryImg": "//img.alicdn.com/tps/i1/TB1s5y.LVXXXXb.aXXXrdvGIFXX-248-155.jpg",
	    "rightBannerImg": null
	  }, {
	    "shopList": null,
	    "finalScore": "3.8773333333333335",
	    "industryUrl": "//pages.tmall.com/wow/act/15617/dqc?abbucket=_AB-M1011_B14&acm=03654.1003.1.735362&aldid=KXPpnvEH&abtest=_AB-LR1011-PR1011&scm=1003.1.03654.OTHER_15_735362&pos=8",
	    "entityType": "OTHER",
	    "colour": "#b4ebff",
	    "industryTitle": "数码",
	    "bannerUrl": null,
	    "leftBannerImg": null,
	    "_pos_": "8",
	    "industryBenefit": "疯抢大牌新品",
	    "industryId": "15",
	    "industryImg": "//img.alicdn.com/tps/i4/TB1_xG.LVXXXXbWaXXXrdvGIFXX-248-155.jpg",
	    "rightBannerImg": null
	  }, {
	    "shopList": null,
	    "finalScore": "3.8359999999999994",
	    "industryUrl": "//pages.tmall.com/wow/act/15617/man?abbucket=_AB-M1011_B14&acm=03654.1003.1.735362&aldid=KXPpnvEH&abtest=_AB-LR1011-PR1011&scm=1003.1.03654.OTHER_6_735362&pos=9",
	    "entityType": "OTHER",
	    "colour": "#cbe9a9",
	    "industryTitle": "男装",
	    "bannerUrl": null,
	    "leftBannerImg": null,
	    "_pos_": "9",
	    "industryBenefit": "新品低至9.9元",
	    "industryId": "6",
	    "industryImg": "//img.alicdn.com/tps/i1/TB1mO2qLVXXXXXiXFXXrdvGIFXX-248-155.jpg",
	    "rightBannerImg": null
	  }, {
	    "shopList": null,
	    "finalScore": "3.243333333333333",
	    "industryUrl": "//pages.tmall.com/wow/act/15617/xihu?abbucket=_AB-M1011_B14&acm=03654.1003.1.735362&aldid=KXPpnvEH&abtest=_AB-LR1011-PR1011&scm=1003.1.03654.OTHER_13_735362&pos=10",
	    "entityType": "OTHER",
	    "colour": "#cdffa4",
	    "industryTitle": "洗护",
	    "bannerUrl": null,
	    "leftBannerImg": null,
	    "_pos_": "10",
	    "industryBenefit": "爆款1分钱抢",
	    "industryId": "13",
	    "industryImg": "//img.alicdn.com/tps/i3/TB1Ft_CLVXXXXX2XXXXrdvGIFXX-248-155.jpg",
	    "rightBannerImg": null
	  }, {
	    "shopList": null,
	    "finalScore": "3.1553333333333335",
	    "industryUrl": "//pages.tmall.com/wow/act/15617/myxfs?abbucket=_AB-M1011_B14&acm=03654.1003.1.735362&aldid=KXPpnvEH&abtest=_AB-LR1011-PR1011&scm=1003.1.03654.OTHER_5_735362&pos=11",
	    "entityType": "OTHER",
	    "colour": "#f8e1ff",
	    "industryTitle": "母婴童装",
	    "bannerUrl": null,
	    "leftBannerImg": null,
	    "_pos_": "11",
	    "industryBenefit": "跨店满299减40",
	    "industryId": "5",
	    "industryImg": "//img.alicdn.com/tps/i4/TB1c.HvLVXXXXXPXpXXrdvGIFXX-248-155.jpg",
	    "rightBannerImg": null
	  }, {
	    "shopList": null,
	    "finalScore": "2.6739999999999995",
	    "industryUrl": "//pages.tmall.com/wow/act/15617/nvzhuang?abbucket=_AB-M1011_B14&acm=03654.1003.1.735362&aldid=KXPpnvEH&abtest=_AB-LR1011-PR1011&scm=1003.1.03654.OTHER_7_735362&pos=12",
	    "entityType": "OTHER",
	    "colour": "#d5cbe8",
	    "industryTitle": "女装",
	    "bannerUrl": null,
	    "leftBannerImg": null,
	    "_pos_": "12",
	    "industryBenefit": "折后用券抢新",
	    "industryId": "7",
	    "industryImg": "//img.alicdn.com/tps/i3/TB1JdfsLVXXXXbRXpXXrdvGIFXX-248-155.jpg",
	    "rightBannerImg": null
	  }, {
	    "shopList": null,
	    "finalScore": "0.0",
	    "industryUrl": "//pages.tmall.com/wow/act/15617/mobilephone?abbucket=_AB-M1011_B14&acm=03654.1003.1.735362&aldid=KXPpnvEH&abtest=_AB-LR1011-PR1011&scm=1003.1.03654.OTHER_14_735362&pos=13",
	    "entityType": "OTHER",
	    "colour": "#c5ffdf",
	    "industryTitle": "手机",
	    "bannerUrl": null,
	    "leftBannerImg": null,
	    "_pos_": "13",
	    "industryBenefit": "1元抢优惠券",
	    "industryId": "14",
	    "industryImg": "//img.alicdn.com/tps/i4/TB1M3zhLVXXXXX4XVXXrdvGIFXX-248-155.jpg",
	    "rightBannerImg": null
	  }, {
	    "shopList": null,
	    "finalScore": "0.0",
	    "industryUrl": "//pages.tmall.com/wow/act/15617/xiangbaohuanxin?abbucket=_AB-M1011_B14&acm=03654.1003.1.735362&aldid=KXPpnvEH&abtest=_AB-LR1011-PR1011&scm=1003.1.03654.OTHER_11_735362&pos=14",
	    "entityType": "OTHER",
	    "colour": "#ffccdc",
	    "industryTitle": "箱包",
	    "bannerUrl": null,
	    "leftBannerImg": null,
	    "_pos_": "14",
	    "industryBenefit": "极致焕新尖货",
	    "industryId": "11",
	    "industryImg": "//img.alicdn.com/tps/i3/TB1gE6kLVXXXXcrXFXXrdvGIFXX-248-155.jpg",
	    "rightBannerImg": null
	  }, {
	    "shopList": null,
	    "finalScore": "0.0",
	    "industryUrl": "//pages.tmall.com/wow/act/15617/newnv?abbucket=_AB-M1011_B14&acm=03654.1003.1.735362&aldid=KXPpnvEH&abtest=_AB-LR1011-PR1011&scm=1003.1.03654.OTHER_10_735362&pos=15",
	    "entityType": "OTHER",
	    "colour": "#fff8ee",
	    "industryTitle": "鞋履",
	    "bannerUrl": null,
	    "leftBannerImg": null,
	    "_pos_": "15",
	    "industryBenefit": "潮品精选鞋履",
	    "industryId": "10",
	    "industryImg": "//img.alicdn.com/tps/i3/TB1.qzpLVXXXXX.XFXXrdvGIFXX-248-155.jpg",
	    "rightBannerImg": null
	  }],
	  "isFormal": "false",
	  "exposureParam": "//ac.mmstat.com/1.gif?apply=vote&abbucket=_AB-M1011_B14&com=02&acm=03654.1003.1.735362&cod=03654&cache=U5revsTn&aldid=KXPpnvEH&logtype=4&abtest=_AB-LR1011-PR1011&scm=1003.1.03654.735362&ip=",
	  "id": "03654"
	};

	module.exports = {
	  components: {
	    banner: __webpack_require__(240)
	  },
	  props: {
	    ds: {
	      default: function _default() {
	        return {};
	      }
	    }
	  },
	  data: function data() {
	    return {
	      NUMBER_248: 248,
	      NUMBER_155: 155,
	      NUMBER_373: 373,
	      NUMBER_237: 237,
	      NUMBER_186: 186,
	      NUMBER_208: 208,
	      NUMBER_750: 750,
	      subItemBg: '//gw.alicdn.com/tps/TB1QzUfLVXXXXaOXVXXXXXXXXXX-248-86.jpg',
	      items: [],
	      subItems: []
	    };
	  },
	  created: function created() {
	    this.initData(demoContent);
	  },
	  methods: {
	    getSubItemTop: function getSubItemTop(i) {
	      return (i - i % 3) / 3 * (241 + 3);
	    },
	    getSubItemLeft: function getSubItemLeft(i) {
	      return i % 3 * (248 + 3);
	    },
	    initData: function initData(content) {
	      this.items = content.main;
	      this.subItems = content.sub;
	    }
	  }
	};

/***/ }),
/* 249 */
/***/ (function(module, exports, __webpack_require__) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', [(_vm.ds.floorTitle) ? _c('image', {
	    staticClass: "title",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "src": _vm.ds.floorTitle
	    }
	  }) : _vm._e(), _vm._v(" "), _c('div', {
	    staticClass: "item-container",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined))
	  }, _vm._l((_vm.items), function(item, i) {
	    return _c('div', {
	      key: i,
	      staticStyle: _vm.$processStyle(undefined),
	      style: (_vm.$processStyle({
	        width: 750,
	        paddingBottom: 14,
	        backgroundColor: item.colour
	      }))
	    }, [_c('div', [_c('banner', {
	      staticStyle: _vm.$processStyle(undefined),
	      style: (_vm.$processStyle(undefined)),
	      attrs: {
	        "width": _vm.NUMBER_750,
	        "height": _vm.NUMBER_237,
	        "src": i % 2 === 0 ? item.leftBannerImg : item.rightBannerImg,
	        "href": item.bannerUrl
	      }
	    })], 1), _vm._v(" "), _c('div', {
	      staticClass: "shop-list",
	      staticStyle: _vm.$processStyle(undefined),
	      style: (_vm.$processStyle(undefined))
	    }, _vm._l((item.shopList), function(i, shop) {
	      return _c('div', {
	        key: i,
	        staticClass: "shop-img-container",
	        staticStyle: _vm.$processStyle(undefined),
	        style: (_vm.$processStyle(undefined))
	      }, [_c('banner', {
	        staticStyle: _vm.$processStyle(undefined),
	        style: (_vm.$processStyle(undefined)),
	        attrs: {
	          "width": _vm.NUMBER_186,
	          "height": _vm.NUMBER_208,
	          "src": shop.picUrl,
	          "href": shop.shopUrl
	        }
	      })], 1)
	    }))])
	  })), _vm._v(" "), _c('div', {
	    staticClass: "sub-item-container",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle({
	      height: Math.round(_vm.subItems.length / 3) * (241 + 3)
	    }))
	  }, _vm._l((_vm.subItems), function(item, i) {
	    return _c('div', {
	      key: i,
	      staticClass: "sub-item",
	      staticStyle: _vm.$processStyle(undefined),
	      style: (_vm.$processStyle({
	        top: _vm.getSubItemTop(i),
	        left: _vm.getSubItemLeft(i)
	      }))
	    }, [_c('image', {
	      staticClass: "sub-item-bg",
	      staticStyle: _vm.$processStyle(undefined),
	      style: (_vm.$processStyle(undefined)),
	      attrs: {
	        "src": _vm.subItemBg
	      }
	    }), _vm._v(" "), _c('div', {
	      staticClass: "sub-item-img",
	      staticStyle: _vm.$processStyle(undefined),
	      style: (_vm.$processStyle(undefined))
	    }, [_c('banner', {
	      staticStyle: _vm.$processStyle(undefined),
	      style: (_vm.$processStyle(undefined)),
	      attrs: {
	        "width": _vm.NUMBER_248,
	        "height": _vm.NUMBER_155,
	        "src": item.industryImg,
	        "href": item.industryUrl
	      }
	    })], 1), _vm._v(" "), _c('div', {
	      staticClass: "sub-item-title",
	      staticStyle: _vm.$processStyle(undefined),
	      style: (_vm.$processStyle(undefined))
	    }, [_c('text', {
	      staticClass: "industry-title",
	      staticStyle: _vm.$processStyle(undefined),
	      style: (_vm.$processStyle(undefined)),
	      attrs: {
	        "href": item.industryUrl
	      }
	    }, [_vm._v(_vm._s(item.industryTitle))]), _vm._v(" "), _c('text', {
	      staticClass: "industry-benefit",
	      staticStyle: _vm.$processStyle(undefined),
	      style: (_vm.$processStyle(undefined)),
	      attrs: {
	        "href": item.industryUrl
	      }
	    }, [_vm._v(_vm._s(item.industryBenefit))])])])
	  }))])
	},staticRenderFns: []}
	module.exports.render._withStripped = true
	if (false) {
	  module.hot.accept()
	  if (module.hot.data) {
	     require("vue-hot-reload-api").rerender("data-v-2db23588", module.exports)
	  }
	}

/***/ }),
/* 250 */
/***/ (function(module, exports, __webpack_require__) {

	var disposed = false
	function injectStyle (ssrContext) {
	  if (disposed) return
	  __webpack_require__(251)
	}
	var Component = __webpack_require__(7)(
	  /* script */
	  __webpack_require__(253),
	  /* template */
	  __webpack_require__(254),
	  /* styles */
	  injectStyle,
	  /* scopeId */
	  "data-v-a1fbc438",
	  /* moduleIdentifier (server only) */
	  null
	)
	Component.options.__file = "/Users/Hanks/Codes/work/incubator-weex/examples/vue/showcase/include/coupon.vue"
	if (Component.esModule && Object.keys(Component.esModule).some(function (key) {return key !== "default" && key.substr(0, 2) !== "__"})) {console.error("named exports are not supported in *.vue files.")}
	if (Component.options.functional) {console.error("[vue-loader] coupon.vue: functional components are not supported with templates, they should use render functions.")}

	/* hot reload */
	if (false) {(function () {
	  var hotAPI = require("vue-hot-reload-api")
	  hotAPI.install(require("vue"), false)
	  if (!hotAPI.compatible) return
	  module.hot.accept()
	  if (!module.hot.data) {
	    hotAPI.createRecord("data-v-a1fbc438", Component.options)
	  } else {
	    hotAPI.reload("data-v-a1fbc438", Component.options)
	  }
	  module.hot.dispose(function (data) {
	    disposed = true
	  })
	})()}

	module.exports = Component.exports


/***/ }),
/* 251 */
/***/ (function(module, exports, __webpack_require__) {

	// style-loader: Adds some css to the DOM by adding a <style> tag

	// load the styles
	var content = __webpack_require__(252);
	if(typeof content === 'string') content = [[module.id, content, '']];
	if(content.locals) module.exports = content.locals;
	// add the styles to the DOM
	var update = __webpack_require__(5)("0d823394", content, false);
	// Hot Module Replacement
	if(false) {
	 // When the styles change, update the <style> tags
	 if(!content.locals) {
	   module.hot.accept("!!../../../../node_modules/css-loader/index.js!../../../../node_modules/vue-loader/lib/style-compiler/index.js?{\"vue\":true,\"id\":\"data-v-a1fbc438\",\"scoped\":true,\"hasInlineConfig\":false}!../../../../node_modules/vue-loader/lib/selector.js?type=styles&index=0!./coupon.vue", function() {
	     var newContent = require("!!../../../../node_modules/css-loader/index.js!../../../../node_modules/vue-loader/lib/style-compiler/index.js?{\"vue\":true,\"id\":\"data-v-a1fbc438\",\"scoped\":true,\"hasInlineConfig\":false}!../../../../node_modules/vue-loader/lib/selector.js?type=styles&index=0!./coupon.vue");
	     if(typeof newContent === 'string') newContent = [[module.id, newContent, '']];
	     update(newContent);
	   });
	 }
	 // When the module is disposed, remove the <style> tags
	 module.hot.dispose(function() { update(); });
	}

/***/ }),
/* 252 */
/***/ (function(module, exports, __webpack_require__) {

	exports = module.exports = __webpack_require__(4)();
	// imports


	// module
	exports.push([module.id, "\n.title[data-v-a1fbc438]{\n  width:750;\n  height:100;\n}\n.wrapper[data-v-a1fbc438] {\n  color: #FF4550;\n  width: 750;\n  height: 230;\n}\n.my-coupon[data-v-a1fbc438] {\n  width: 180;\n  height: 39;\n  position: absolute;\n  top: 15;\n  right: 149;\n}\n.guides[data-v-a1fbc438] {\n  height: 39;\n  width: 110;\n  position: absolute;\n  top: 15;\n  right: 32;\n}\n.lottery[data-v-a1fbc438] {\n  width: 348;\n  height: 76;\n  position: absolute;\n  top: 132;\n  right: 70;\n}\n.info[data-v-a1fbc438] {\n  font-size: 18;\n  position: absolute;\n  top: 84;\n  right: 70;\n}\n.img[data-v-a1fbc438] {\n  width: 750;\n  height: 230;\n}\n", ""]);

	// exports


/***/ }),
/* 253 */
/***/ (function(module, exports) {

	'use strict';

	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//

	var modal = weex.requireModule('modal');
	module.exports = {
	  props: {
	    ds: {
	      default: function _default() {
	        return {};
	      }
	    }
	  },
	  data: function data() {
	    return {
	      currentStatus: '//img.alicdn.com/tps/i4/TB1cYBKMXXXXXayaXXXkxHk2pXX-750-228.jpg_q75.jpg'
	    };
	  },
	  methods: {
	    handleClick: function handleClick(e) {
	      modal.toast({
	        message: 'click',
	        duration: 1
	      });
	    }
	  }
	};

/***/ }),
/* 254 */
/***/ (function(module, exports, __webpack_require__) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', [(_vm.ds.floorTitle) ? _c('image', {
	    staticClass: "title",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "src": _vm.ds.floorTitle
	    }
	  }) : _vm._e(), _vm._v(" "), _c('div', {
	    staticClass: "wrapper",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined))
	  }, [_c('image', {
	    staticClass: "img",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "src": _vm.currentStatus
	    }
	  }), _vm._v(" "), _c('div', {
	    staticClass: "my-coupon",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    on: {
	      "click": _vm.handleClick
	    }
	  }), _vm._v(" "), _c('div', {
	    staticClass: "guides",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    on: {
	      "click": _vm.handleClick
	    }
	  }), _vm._v(" "), _c('div', {
	    staticClass: "lottery",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    on: {
	      "click": _vm.handleClick
	    }
	  })])])
	},staticRenderFns: []}
	module.exports.render._withStripped = true
	if (false) {
	  module.hot.accept()
	  if (module.hot.data) {
	     require("vue-hot-reload-api").rerender("data-v-a1fbc438", module.exports)
	  }
	}

/***/ }),
/* 255 */
/***/ (function(module, exports, __webpack_require__) {

	var disposed = false
	function injectStyle (ssrContext) {
	  if (disposed) return
	  __webpack_require__(256)
	}
	var Component = __webpack_require__(7)(
	  /* script */
	  __webpack_require__(258),
	  /* template */
	  __webpack_require__(259),
	  /* styles */
	  injectStyle,
	  /* scopeId */
	  "data-v-7a3987f0",
	  /* moduleIdentifier (server only) */
	  null
	)
	Component.options.__file = "/Users/Hanks/Codes/work/incubator-weex/examples/vue/showcase/include/goods.vue"
	if (Component.esModule && Object.keys(Component.esModule).some(function (key) {return key !== "default" && key.substr(0, 2) !== "__"})) {console.error("named exports are not supported in *.vue files.")}
	if (Component.options.functional) {console.error("[vue-loader] goods.vue: functional components are not supported with templates, they should use render functions.")}

	/* hot reload */
	if (false) {(function () {
	  var hotAPI = require("vue-hot-reload-api")
	  hotAPI.install(require("vue"), false)
	  if (!hotAPI.compatible) return
	  module.hot.accept()
	  if (!module.hot.data) {
	    hotAPI.createRecord("data-v-7a3987f0", Component.options)
	  } else {
	    hotAPI.reload("data-v-7a3987f0", Component.options)
	  }
	  module.hot.dispose(function (data) {
	    disposed = true
	  })
	})()}

	module.exports = Component.exports


/***/ }),
/* 256 */
/***/ (function(module, exports, __webpack_require__) {

	// style-loader: Adds some css to the DOM by adding a <style> tag

	// load the styles
	var content = __webpack_require__(257);
	if(typeof content === 'string') content = [[module.id, content, '']];
	if(content.locals) module.exports = content.locals;
	// add the styles to the DOM
	var update = __webpack_require__(5)("0dca157b", content, false);
	// Hot Module Replacement
	if(false) {
	 // When the styles change, update the <style> tags
	 if(!content.locals) {
	   module.hot.accept("!!../../../../node_modules/css-loader/index.js!../../../../node_modules/vue-loader/lib/style-compiler/index.js?{\"vue\":true,\"id\":\"data-v-7a3987f0\",\"scoped\":true,\"hasInlineConfig\":false}!../../../../node_modules/vue-loader/lib/selector.js?type=styles&index=0!./goods.vue", function() {
	     var newContent = require("!!../../../../node_modules/css-loader/index.js!../../../../node_modules/vue-loader/lib/style-compiler/index.js?{\"vue\":true,\"id\":\"data-v-7a3987f0\",\"scoped\":true,\"hasInlineConfig\":false}!../../../../node_modules/vue-loader/lib/selector.js?type=styles&index=0!./goods.vue");
	     if(typeof newContent === 'string') newContent = [[module.id, newContent, '']];
	     update(newContent);
	   });
	 }
	 // When the module is disposed, remove the <style> tags
	 module.hot.dispose(function() { update(); });
	}

/***/ }),
/* 257 */
/***/ (function(module, exports, __webpack_require__) {

	exports = module.exports = __webpack_require__(4)();
	// imports


	// module
	exports.push([module.id, "\n.title[data-v-7a3987f0] {\n  width: 750;\n  height: 100;\n}\n.slider[data-v-7a3987f0] {\n  height: 652;\n}\n.pannel[data-v-7a3987f0] {\n  width: 750;\n  height: 592;\n  flex-direction: row;\n}\n.middle-col[data-v-7a3987f0] {\n  margin-left:4;\n  margin-right:4;\n  width: 240;\n  height: 588;\n}\n", ""]);

	// exports


/***/ }),
/* 258 */
/***/ (function(module, exports, __webpack_require__) {

	'use strict';

	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//

	module.exports = {
	  components: {
	    banner: __webpack_require__(240)
	  },
	  props: {
	    ds: {
	      default: function _default() {
	        return {};
	      }
	    }
	  },
	  data: function data() {
	    return {
	      NUMBER_251: 251,
	      NUMBER_240: 240,
	      NUMBER_292: 292,
	      NUMBER_588: 588
	    };
	  }
	};

/***/ }),
/* 259 */
/***/ (function(module, exports, __webpack_require__) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return (!_vm.ds.isHide) ? _c('div', [(_vm.ds.floorTitle) ? _c('image', {
	    staticClass: "title",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "src": _vm.ds.floorTitle
	    }
	  }) : _vm._e(), _vm._v(" "), _c('slider', {
	    staticClass: "slider",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "show-indicators": "true",
	      "auto-play": "true",
	      "interval": "3000"
	    }
	  }, [_vm._l((_vm.ds.bannerItems), function(item, i) {
	    return _c('div', {
	      key: i,
	      staticClass: "pannel",
	      staticStyle: _vm.$processStyle(undefined),
	      style: (_vm.$processStyle(undefined))
	    }, [_c('div', [_c('banner', {
	      staticStyle: _vm.$processStyle({
	        "margin-bottom": "4"
	      }),
	      style: (_vm.$processStyle(undefined)),
	      attrs: {
	        "width": _vm.NUMBER_251,
	        "height": _vm.NUMBER_292,
	        "src": item.img1,
	        "href": item.url1
	      }
	    }), _vm._v(" "), _c('banner', {
	      staticStyle: _vm.$processStyle(undefined),
	      style: (_vm.$processStyle(undefined)),
	      attrs: {
	        "width": _vm.NUMBER_251,
	        "height": _vm.NUMBER_292,
	        "src": item.img2,
	        "href": item.url2
	      }
	    })], 1), _vm._v(" "), _c('div', {
	      staticClass: "middle-col",
	      staticStyle: _vm.$processStyle(undefined),
	      style: (_vm.$processStyle(undefined))
	    }, [_c('banner', {
	      staticStyle: _vm.$processStyle(undefined),
	      style: (_vm.$processStyle(undefined)),
	      attrs: {
	        "width": _vm.NUMBER_240,
	        "height": _vm.NUMBER_588,
	        "src": item.img3,
	        "href": item.url3
	      }
	    })], 1), _vm._v(" "), _c('div', [_c('banner', {
	      staticStyle: _vm.$processStyle({
	        "margin-bottom": "4"
	      }),
	      style: (_vm.$processStyle(undefined)),
	      attrs: {
	        "width": _vm.NUMBER_251,
	        "height": _vm.NUMBER_292,
	        "src": item.img4,
	        "href": item.url4
	      }
	    }), _vm._v(" "), _c('banner', {
	      staticStyle: _vm.$processStyle(undefined),
	      style: (_vm.$processStyle(undefined)),
	      attrs: {
	        "width": _vm.NUMBER_251,
	        "height": _vm.NUMBER_292,
	        "src": item.img5,
	        "href": item.url5
	      }
	    })], 1)])
	  }), _vm._v(" "), _c('indicator', {
	    staticStyle: _vm.$processStyle({
	      "position": "absolute",
	      "width": "714",
	      "height": "200",
	      "left": "10",
	      "bottom": "-80",
	      "itemSize": "20",
	      "itemColor": "#999999",
	      "itemSelectedColor": "#000000"
	    }),
	    style: (_vm.$processStyle(undefined))
	  })], 2)], 1) : _vm._e()
	},staticRenderFns: []}
	module.exports.render._withStripped = true
	if (false) {
	  module.hot.accept()
	  if (module.hot.data) {
	     require("vue-hot-reload-api").rerender("data-v-7a3987f0", module.exports)
	  }
	}

/***/ }),
/* 260 */
/***/ (function(module, exports, __webpack_require__) {

	var disposed = false
	function injectStyle (ssrContext) {
	  if (disposed) return
	  __webpack_require__(261)
	}
	var Component = __webpack_require__(7)(
	  /* script */
	  __webpack_require__(263),
	  /* template */
	  __webpack_require__(264),
	  /* styles */
	  injectStyle,
	  /* scopeId */
	  "data-v-8e543768",
	  /* moduleIdentifier (server only) */
	  null
	)
	Component.options.__file = "/Users/Hanks/Codes/work/incubator-weex/examples/vue/showcase/include/resource.vue"
	if (Component.esModule && Object.keys(Component.esModule).some(function (key) {return key !== "default" && key.substr(0, 2) !== "__"})) {console.error("named exports are not supported in *.vue files.")}
	if (Component.options.functional) {console.error("[vue-loader] resource.vue: functional components are not supported with templates, they should use render functions.")}

	/* hot reload */
	if (false) {(function () {
	  var hotAPI = require("vue-hot-reload-api")
	  hotAPI.install(require("vue"), false)
	  if (!hotAPI.compatible) return
	  module.hot.accept()
	  if (!module.hot.data) {
	    hotAPI.createRecord("data-v-8e543768", Component.options)
	  } else {
	    hotAPI.reload("data-v-8e543768", Component.options)
	  }
	  module.hot.dispose(function (data) {
	    disposed = true
	  })
	})()}

	module.exports = Component.exports


/***/ }),
/* 261 */
/***/ (function(module, exports, __webpack_require__) {

	// style-loader: Adds some css to the DOM by adding a <style> tag

	// load the styles
	var content = __webpack_require__(262);
	if(typeof content === 'string') content = [[module.id, content, '']];
	if(content.locals) module.exports = content.locals;
	// add the styles to the DOM
	var update = __webpack_require__(5)("4d02c6fc", content, false);
	// Hot Module Replacement
	if(false) {
	 // When the styles change, update the <style> tags
	 if(!content.locals) {
	   module.hot.accept("!!../../../../node_modules/css-loader/index.js!../../../../node_modules/vue-loader/lib/style-compiler/index.js?{\"vue\":true,\"id\":\"data-v-8e543768\",\"scoped\":true,\"hasInlineConfig\":false}!../../../../node_modules/vue-loader/lib/selector.js?type=styles&index=0!./resource.vue", function() {
	     var newContent = require("!!../../../../node_modules/css-loader/index.js!../../../../node_modules/vue-loader/lib/style-compiler/index.js?{\"vue\":true,\"id\":\"data-v-8e543768\",\"scoped\":true,\"hasInlineConfig\":false}!../../../../node_modules/vue-loader/lib/selector.js?type=styles&index=0!./resource.vue");
	     if(typeof newContent === 'string') newContent = [[module.id, newContent, '']];
	     update(newContent);
	   });
	 }
	 // When the module is disposed, remove the <style> tags
	 module.hot.dispose(function() { update(); });
	}

/***/ }),
/* 262 */
/***/ (function(module, exports, __webpack_require__) {

	exports = module.exports = __webpack_require__(4)();
	// imports


	// module
	exports.push([module.id, "\n.title[data-v-8e543768] {\n  width: 750;\n  height: 100;\n}\n.big-banner-container[data-v-8e543768]{\n  margin-bottom:4;\n}\n", ""]);

	// exports


/***/ }),
/* 263 */
/***/ (function(module, exports, __webpack_require__) {

	'use strict';

	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//

	module.exports = {
	  components: {
	    banners: __webpack_require__(238),
	    banner: __webpack_require__(240)
	  },
	  props: {
	    ds: {
	      default: function _default() {
	        return {};
	      }
	    }
	  },
	  data: function data() {
	    return {
	      NUMBER_750: 750,
	      NUMBER_200: 200,
	      NUMBER_373: 373,
	      NUMBER_224: 224,
	      NUMBER_4: 4
	    };
	  }
	};

/***/ }),
/* 264 */
/***/ (function(module, exports, __webpack_require__) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return (!_vm.ds.isHide) ? _c('div', [_c('div', {
	    staticClass: "big-banner-container",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined))
	  }, [_c('banner', {
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "width": _vm.NUMBER_750,
	      "height": _vm.NUMBER_200,
	      "src": _vm.ds.bigBannerImg,
	      "href": _vm.ds.bigBannerUrl
	    }
	  })], 1), _vm._v(" "), _c('banners', {
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "direction": "row",
	      "ds": _vm.ds.smallBannerItems,
	      "width": _vm.NUMBER_373,
	      "height": _vm.NUMBER_224,
	      "space": _vm.NUMBER_4
	    }
	  })], 1) : _vm._e()
	},staticRenderFns: []}
	module.exports.render._withStripped = true
	if (false) {
	  module.hot.accept()
	  if (module.hot.data) {
	     require("vue-hot-reload-api").rerender("data-v-8e543768", module.exports)
	  }
	}

/***/ }),
/* 265 */
/***/ (function(module, exports, __webpack_require__) {

	var disposed = false
	function injectStyle (ssrContext) {
	  if (disposed) return
	  __webpack_require__(266)
	}
	var Component = __webpack_require__(7)(
	  /* script */
	  __webpack_require__(268),
	  /* template */
	  __webpack_require__(269),
	  /* styles */
	  injectStyle,
	  /* scopeId */
	  "data-v-52519b84",
	  /* moduleIdentifier (server only) */
	  null
	)
	Component.options.__file = "/Users/Hanks/Codes/work/incubator-weex/examples/vue/showcase/include/scene.vue"
	if (Component.esModule && Object.keys(Component.esModule).some(function (key) {return key !== "default" && key.substr(0, 2) !== "__"})) {console.error("named exports are not supported in *.vue files.")}
	if (Component.options.functional) {console.error("[vue-loader] scene.vue: functional components are not supported with templates, they should use render functions.")}

	/* hot reload */
	if (false) {(function () {
	  var hotAPI = require("vue-hot-reload-api")
	  hotAPI.install(require("vue"), false)
	  if (!hotAPI.compatible) return
	  module.hot.accept()
	  if (!module.hot.data) {
	    hotAPI.createRecord("data-v-52519b84", Component.options)
	  } else {
	    hotAPI.reload("data-v-52519b84", Component.options)
	  }
	  module.hot.dispose(function (data) {
	    disposed = true
	  })
	})()}

	module.exports = Component.exports


/***/ }),
/* 266 */
/***/ (function(module, exports, __webpack_require__) {

	// style-loader: Adds some css to the DOM by adding a <style> tag

	// load the styles
	var content = __webpack_require__(267);
	if(typeof content === 'string') content = [[module.id, content, '']];
	if(content.locals) module.exports = content.locals;
	// add the styles to the DOM
	var update = __webpack_require__(5)("93ba78ec", content, false);
	// Hot Module Replacement
	if(false) {
	 // When the styles change, update the <style> tags
	 if(!content.locals) {
	   module.hot.accept("!!../../../../node_modules/css-loader/index.js!../../../../node_modules/vue-loader/lib/style-compiler/index.js?{\"vue\":true,\"id\":\"data-v-52519b84\",\"scoped\":true,\"hasInlineConfig\":false}!../../../../node_modules/vue-loader/lib/selector.js?type=styles&index=0!./scene.vue", function() {
	     var newContent = require("!!../../../../node_modules/css-loader/index.js!../../../../node_modules/vue-loader/lib/style-compiler/index.js?{\"vue\":true,\"id\":\"data-v-52519b84\",\"scoped\":true,\"hasInlineConfig\":false}!../../../../node_modules/vue-loader/lib/selector.js?type=styles&index=0!./scene.vue");
	     if(typeof newContent === 'string') newContent = [[module.id, newContent, '']];
	     update(newContent);
	   });
	 }
	 // When the module is disposed, remove the <style> tags
	 module.hot.dispose(function() { update(); });
	}

/***/ }),
/* 267 */
/***/ (function(module, exports, __webpack_require__) {

	exports = module.exports = __webpack_require__(4)();
	// imports


	// module
	exports.push([module.id, "\n.title[data-v-52519b84] {\n  width: 750;\n  height: 100;\n}\n.item[data-v-52519b84] {\n  width: 750;\n  height: 340;\n  margin-bottom: 4;\n}\n.item-img[data-v-52519b84] {\n  position: absolute;\n  left: 110;\n  top: 20;\n}\n.right-item-img[data-v-52519b84] {\n  position: absolute;\n  right: 110;\n  top: 20;\n}\n.benifit-img[data-v-52519b84] {\n  position: absolute;\n  right: 5;\n  top: 160;\n}\n.right-benifit-img[data-v-52519b84] {\n  position: absolute;\n  left: 40;\n  top: 160;\n}\n.small-item-container[data-v-52519b84] {\n  width: 750;\n}\n.small-item[data-v-52519b84] {\n  position: absolute;\n  width: 373;\n  height: 230;\n  flex-direction: row;\n}\n.small-item-bg[data-v-52519b84] {\n  width: 373;\n  height: 230;\n}\n.small-item-img[data-v-52519b84] {\n  position: absolute;\n  left: 12;\n  top: 12;\n}\n.small-benifit-img[data-v-52519b84] {\n  position: absolute;\n  right: 0;\n  top: 65;\n}\n", ""]);

	// exports


/***/ }),
/* 268 */
/***/ (function(module, exports, __webpack_require__) {

	"use strict";

	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//

	var demoContent = [{
	  "id": "1111",
	  "sceneUrl": "https://pages.tmall.com/wow/act/15617/scene?acm=03653.1003.1.740603&aldid=VyrSqx06&wh_id=1392&scm=1003.1.03653.SUB_1111_740603&pos=1",
	  "_s_": "d",
	  "itemImg": "https://img.alicdn.com/tps/TB1c7t.MXXXXXchXXXXXXXXXXXX-182-206.jpg",
	  "sceneCat": "50012010,50012027,50012032,50007003,50008881,50008883,1623,162116,50010850,123216004",
	  "finalScore": "10000.0",
	  "_pos_": "1",
	  "entityType": "SUB",
	  "benefitImg": "//img.alicdn.com/tps/TB1Zgl2MXXXXXaYXFXXXXXXXXXX-170-100.png"
	}, {
	  "id": "1396",
	  "sceneUrl": "https://pages.tmall.com/wow/act/15617/scene?acm=03653.1003.1.740603&aldid=VyrSqx06&wh_id=1396&scm=1003.1.03653.SUB_1396_740603&pos=2",
	  "_s_": "d",
	  "itemImg": "//img.alicdn.com/tps/i2/TB1O1VfMXXXXXX0aXXXKofZZpXX-182-206.jpg",
	  "sceneCat": "50010368,50011892,50011894,50013794,50010792,50010790,50010790,121434025,121396024,50010793,50010789,50010793,121386035,121392037,121458034,121392036,121482027,121392015,121368014,121418013,121392016,50011982,122690003,302910,50012010,122654005,50012019,50012825,50012027,50012042,50011744,50012906,50012907,50008881,50008883,50008882?,50012784?,50022889,50022728,50013228,50013896,50014775,50019578,50014822,50026394,50019694,50019279,50019272,50014079,50014078,50014076,50014077,50017238,121464011,121468014,50019130,50019127,50019126,50019128,50019125,50015984,50014803,50014802,50015980,50014788,50014787,50019526,50014791,50019520,50013932,50014677,50015978,50014786,50014785,50522001,50014493,50023100,121418021,121388027,121424021,121418020,121368020,50012036,50012044,50012043,50010368, 50011892, 50019274,50012044,50012048,50012043,50013228,50013238,50011726,50019279,50015984,50014803,50014802,50019526,50014791,50019520,50013932,50014677,50019526,50013314,50023341,50016752,50016749,50016747,50016744,50016741,50016740,50016739,50016738,50016737,50016769,50016763,50016750,50007003,302910,50012010,50012032,50012033,50011745,50011746,50011894,121458034,121756003,121426033,50011982,50011979,121390006,124730001,50001748,50011130,124730001,50001748,50011130,50011129,50010160,50011161,50011159,50010159,50010158,50011127,3035,50010167,50000557,50011123,50010402,50000436,50011153,50010850,50000671,162201,50022566,50007068,50026651,50000852,162104,162116,162205,1623,123216004,162404,162401,50011412,50011411,162402,50008903,50011413,162403,50000697,1629,50011277,121412004,50008897,50008901,50008899,50013194,50008904,50008905,50008898,162103,50008900,50013196,121434004,50005065,162702,162701,162703,1636,1512,123534001,14,50008090,50012341,50132002,50012342,50012343,50012345,50012346,50013189,50010527,121394006,50013618,50010537,50012424,121452038,121484044,50010540,50010548,50024769,121450007,50013697,50010218,50005960,50005962,50012820,50012406,50023620,50023605,50008829,50024064,50023440,50008687,124392001,50023771,50006804,50012410,124482010,121398023,50023789,50011975",
	  "finalScore": "10000.0",
	  "_pos_": "2",
	  "entityType": "SUB",
	  "benefitImg": "//img.alicdn.com/tps/i1/TB1cp4sMXXXXXbLXFXX3bpXTXXX-170-100.png"
	}, {
	  "id": "1616",
	  "sceneUrl": "https://pages.tmall.com/wow/act/15617/scene?acm=03653.1003.1.740603&aldid=VyrSqx06&wh_id=1616&scm=1003.1.03653.SUB_1616_740603&pos=3",
	  "_s_": "d",
	  "itemImg": "//img.alicdn.com/tps/i4/TB1dlEALVXXXXbxXpXXKofZZpXX-182-206.jpg",
	  "sceneCat": "50009032,50007003,302910,1512,124688012,124986002,124942005,50023722,50010368,50011892,50011894",
	  "finalScore": "10000.0",
	  "_pos_": "3",
	  "entityType": "SUB",
	  "benefitImg": "//img.alicdn.com/tps/i3/TB1fxonLVXXXXXUaXXX3bpXTXXX-170-100.png"
	}, {
	  "id": "16424",
	  "sceneUrl": "https://brand.tmall.com/mobilestreet/subject.htm?id=16424&acm=03653.1003.1.740603&aldid=VyrSqx06&spm=0.0.0.0.Wv8ion&scm=1003.1.03653.SUB_16424_740603&pos=4",
	  "_s_": "d",
	  "itemImg": "//img.alicdn.com/tps/i4/TB1.i2_LVXXXXbrXpXXKofZZpXX-182-206.jpg",
	  "sceneCat": "50012043,50012036,50010850,50000671,1622,50000852,162104,50000697,162205,162116,50012027,50012032,50012028,50012033,50012042,50008882,50012772,50006846,50008881,50000436,50011123,3035,50010167,50000557,50010158,50010159,50010402,50010160,50011980,50011993,121390006,50011978,50011977,50011979,121454013,50011745",
	  "finalScore": "6.729749999999999",
	  "_pos_": "4",
	  "entityType": "SUB",
	  "benefitImg": "//img.alicdn.com/tps/i2/TB1bhH5LVXXXXapXFXX3bpXTXXX-170-100.png"
	}, {
	  "id": "16425",
	  "sceneUrl": "https://brand.tmall.com/mobilestreet/subject.htm?id=16360&acm=03653.1003.1.740603&aldid=VyrSqx06&spm=0.0.0.0.MttFEC&scm=1003.1.03653.SUB_16425_740603&pos=5",
	  "_s_": "d",
	  "itemImg": "//img.alicdn.com/tps/i3/TB15ywbLVXXXXcdXXXXKofZZpXX-182-206.jpg",
	  "sceneCat": "50012043,50012036,50010850,50000671,1622,50000852,162104,50000697,162205,162116,50012027,50012032,50012028,50012033,50012042,50008882,50012772,50006846,50008881,50000436,50011123,3035,50010167,50000557,50010158,50010159,50010402,50010160,50011980,50011993,121390006,50011978,50011977,50011979,121454013,50011745",
	  "finalScore": "6.729749999999999",
	  "_pos_": "5",
	  "entityType": "SUB",
	  "benefitImg": "//img.alicdn.com/tps/i1/TB1JBLYLVXXXXXlXVXX3bpXTXXX-170-100.png"
	}, {
	  "id": "1625",
	  "sceneUrl": "https://pages.tmall.com/wow/fushi/act/gf-chaoliu?acm=03653.1003.1.740603&aldid=VyrSqx06&scm=1003.1.03653.SUB_1625_740603&pos=6",
	  "_s_": "d",
	  "itemImg": "//img.alicdn.com/bao/uploaded/i2/TB1Gd21KVXXXXXZXVXXXXXXXXXX_!!0-item_pic.jpg",
	  "sceneCat": "50012043,50012036,50010850,50000671,1622,50000852,162104,50000697,162205,162116,50012027,50012032,50012028,50012033,50012042,50008882,50012772,50006846,50008881,50000436,50011123,3035,50010167,50000557,50010158,50010159,50010402,50010160,50011980,50011993,121390006,50011978,50011977,50011979,121454013,50011982,50011991,50012906,50011744,50011745,50011743,50012907,50011746,50010815,121460005,50013794,122438001,121398006,121386023,50010808",
	  "finalScore": "6.729749999999999",
	  "_pos_": "6",
	  "entityType": "SUB",
	  "benefitImg": "//img.alicdn.com/tps/i3/TB1qxXzMXXXXXcjXFXX3bpXTXXX-170-100.png"
	}, {
	  "id": "1392",
	  "sceneUrl": "https://pages.tmall.com/wow/act/15617/scene?acm=03653.1003.1.740603&aldid=VyrSqx06&wh_id=1392&scm=1003.1.03653.SUB_1392_740603&pos=7",
	  "_s_": "d",
	  "itemImg": "https://img.alicdn.com/tps/TB1c7t.MXXXXXchXXXXXXXXXXXX-182-206.jpg",
	  "sceneCat": "50006842,50006840,50008881,50008883,50008882,50008884,50012774,50012775,50012776,50012781,5000888,50012766,50012771,50012772,50010404,50010368, 50011892, 50011894,124688012,124730001,50001748,50011130,124730001,50001748,50011130,50011129,50010160,50011161,50011159,50010159,50010158,50011127,3035,50010167,50000557,50011123,50010402,50000436,50011153,50011740,50008882,50008886,50008885,50012777,50012778,50011977,50011978,50011979,50011980,50011982,50011990,50011999,50012000,50011993,50011996,50011997,121366011,121484009,121368010,121390006,121398029,121404030,121404031,121408040,121410035,121412033,121414041,121418013,121454013,121410013,121422013,121484013,50010789,50010790,50010792,50010793,50010794,50010796,50010797,50010798,50010801,50010803,50010805,50010807,50010808,50010815,121410007,121426007,121452004,121460005,50012341,50132002,50012342,50012343,50012345,50012346,50013189,50010527,121394006,50013618,50010537,50012424,121452038,121484044,50010540,50010548,50024769,121366037,121458036,121392038,121420035,121364028,121468026,121472031,121476036,121470042,121418032,121410036,121472032,121420032,121470040,121384032,121460028,121448028,121400030,121424024,121482025,121418031,121402028,121388035,121460029,121400028,121400029,121418030,121476034,121478031,121448029,121480030,121366035,121424025,121426030,121484030,121412031,121384033,121424023,121402027,121408039,121420033,121452028,121466040,121416029,121482028,121420034,121396046,121422052,121370037,121424031,21364043,121472040,121412045,121424032,125054002,125074007,125074008,125104003,125080003,125024006,125080004",
	  "finalScore": "4.4951428571428576",
	  "_pos_": "7",
	  "entityType": "SUB",
	  "benefitImg": "//img.alicdn.com/tps/TB1Zgl2MXXXXXaYXFXXXXXXXXXX-170-100.png"
	}, {
	  "id": "1395",
	  "sceneUrl": "https://pages.tmall.com/wow/act/15617/scene?acm=03653.1003.1.740603&aldid=VyrSqx06&wh_id=1395&scm=1003.1.03653.SUB_1395_740603&pos=8",
	  "_s_": "d",
	  "itemImg": "//img.alicdn.com/tps/i2/TB1p4NcMXXXXXbGaXXXKofZZpXX-182-206.jpg",
	  "sceneCat": "50008881,50008883,50008882,50008884,50012774,50012775,50012776,50012781,50008886,50012766,50012771,50012772,50010850,50000671,162201,50022566,50007068,50026651,50000852,162104,162116,162205,1623,123216004,162404,162401,50011412,50011411,16240,50012773,50006846,50010394,50008888,50008890,50008889,50012784,50012785,50012786,50012787,50008885,50012777,50012778,124730001,50001748,50011130,124730001,50001748,50011130,50011129,50010160,50011161,50011159,50010159,50010158,50011127,3035,50010167,50000557,50011123,50010402,50000436,50011153,50011740,50006843,50006842,124942005,50010368,50011892,50011894,50023751,121366037,121458036,121392038,121420035,121364028,121468026,121400029,121418030,121478031,121480030,121366035,121424025,121426030,121484030,121412031,125054002,125074007,125074008,125104003,125080003,125024006,125080004,121400030,121424024,121482025,121418031,121402028,121388035",
	  "finalScore": "4.465166666666667",
	  "_pos_": "8",
	  "entityType": "SUB",
	  "benefitImg": "//img.alicdn.com/tps/i1/TB1ySNtMXXXXXamXFXX3bpXTXXX-170-100.png"
	}, {
	  "id": "1394",
	  "sceneUrl": "https://pages.tmall.com/wow/act/15617/scene?acm=03653.1003.1.740603&aldid=VyrSqx06&wh_id=1394&scm=1003.1.03653.SUB_1394_740603&pos=9",
	  "_s_": "d",
	  "itemImg": "//img.alicdn.com/tps/i4/TB1Ns8eMXXXXXb1aXXXKofZZpXX-182-206.jpg",
	  "sceneCat": "50010850,50000671,162201,50022566,50007068,50026651,50000852,162104,162116,162205,1623,123216004,162404,162401,50011412,50011411,162402,50008903,50011413,162403,50000697,1629,50011277,121412004,50008897,50008901,50008899,50013194,50008904,50008905,50008898,162103,50008900,50013196,121434004,50005065,162702,162701,162703,1636,124730001,50001748,50011130,124730001,50001748,50011130,50011129,50010160,50011161,50011159,50010159,50010158,50011127,3035,50010167,50000557,50011123,50010402,50000436,50011153,50011740,50006843,50006842,124986002,50010368, 50011892,50011894,121480030,121366035,121424025,121426030,121484030,121412031",
	  "finalScore": "4.309925925925926",
	  "_pos_": "9",
	  "entityType": "SUB",
	  "benefitImg": "//img.alicdn.com/tps/i3/TB1ZEXfMXXXXXapaXXX3bpXTXXX-170-100.png"
	}, {
	  "id": "1610",
	  "sceneUrl": "https://pages.tmall.com/wow/act/15617/scene?acm=03653.1003.1.740603&aldid=VyrSqx06&wh_id=1610&scm=1003.1.03653.SUB_1610_740603&pos=10",
	  "_s_": "d",
	  "itemImg": "//img.alicdn.com/tps/i3/TB1ONckLVXXXXbJaXXXKofZZpXX-182-206.jpg",
	  "sceneCat": "121484044,121452038,50012342,50012027,50012032,50006843,50010850,50000671,162104,1623,50011277,162116,123216004,50008898,121366037,121458036,121392038,121420035,121364028,121468026,121472031,121476036,121470042,121418032,121410036,121472032,121420032,121470040,121384032,121460028,121448028,121400030,121424024,121482025,121418031,121402028,121388035,121460029,121400028,121400029,121418030,121476034,121478031,121448029,121480030,121366035,121424025,121426030,121484030,121412031,121384033,121424023,121402027,121408039,121420033,121452028,121466040,121416029,121482028,121420034,121396046,121422052,121370037,121424031,21364043,121472040,121412045,121424032,125054002,125074007,125074008,125104003,125080003,125024006,125080004",
	  "finalScore": "4.293333333333334",
	  "_pos_": "10",
	  "entityType": "SUB",
	  "benefitImg": "//img.alicdn.com/tps/i2/TB1RzwFLVXXXXXsXpXX3bpXTXXX-170-100.png"
	}, {
	  "id": "1619",
	  "sceneUrl": "https://pages.tmall.com/wow/act/15617/scene?acm=03653.1003.1.740603&aldid=VyrSqx06&wh_id=1619&scm=1003.1.03653.SUB_1619_740603&pos=11",
	  "_s_": "d",
	  "itemImg": "//img.alicdn.com/tps/i3/TB1eagyLVXXXXaiXFXXKofZZpXX-182-206.jpg",
	  "sceneCat": "124730001,50001748,50011130,124730001,50001748,50011130,50011129,50010160,50011161,50011159,50010159,50010158,50011127,3035,50010167,50000557,50011123,50010402,50000436,50011153,122654005,50011740,50009032,124688012，213202,121396029,121476023,121410029,213203,50023326,213205,50022686,50023293,50466023,50024999,50016885，50011894",
	  "finalScore": "4.23",
	  "_pos_": "11",
	  "entityType": "SUB",
	  "benefitImg": "//img.alicdn.com/tps/i4/TB1j0AxLVXXXXaRXFXX3bpXTXXX-170-100.png"
	}, {
	  "id": "1613",
	  "sceneUrl": "https://pages.tmall.com/wow/act/15617/scene?acm=03653.1003.1.740603&aldid=VyrSqx06&wh_id=1613&scm=1003.1.03653.SUB_1613_740603&pos=12",
	  "_s_": "d",
	  "itemImg": "//img.alicdn.com/tps/i2/TB1WssFLVXXXXXUXpXXKofZZpXX-182-206.jpg",
	  "sceneCat": "50023108,50022893,50022892,50022890,50013228,50019520,50017120,50019560,50017129,50017128,50017127,50017126,50017125,50017124,50017123,50017122,50017121,50020039,50020038,50017119,50019559,50017110,50017108,50017107,50017097,50017096,50017095,50017094,50017093,50017092,50012036,50012041",
	  "finalScore": "4.2186666666666675",
	  "_pos_": "12",
	  "entityType": "SUB",
	  "benefitImg": "//img.alicdn.com/tps/i1/TB1Rg.qLVXXXXbqXVXX3bpXTXXX-170-100.png"
	}, {
	  "id": "1614",
	  "sceneUrl": "https://pages.tmall.com/wow/act/15617/scene?acm=03653.1003.1.740603&aldid=VyrSqx06&wh_id=1614&scm=1003.1.03653.SUB_1614_740603&pos=13",
	  "_s_": "d",
	  "itemImg": "//img.alicdn.com/tps/i1/TB1KzUxLVXXXXajXFXXKofZZpXX-182-206.jpg",
	  "sceneCat": "50013238,50011726,50022893,50022892,50022890,50013228,50017129,50020038,50017119,50019559,50017102,50017110,50017097,50019707,50019555,50016677,50018317,50016675,50016673,50016672,50016671,50016670,50016732,50016669,50016668,50016667,50016666,50016665,50016683,50016682,50016608,50016607,50016637,50016638,50012036,50012043",
	  "finalScore": "4.216",
	  "_pos_": "13",
	  "entityType": "SUB",
	  "benefitImg": "//img.alicdn.com/tps/i2/TB1WyouLVXXXXXcXVXX3bpXTXXX-170-100.png"
	}, {
	  "id": "1397",
	  "sceneUrl": "https://pages.tmall.com/wow/act/15617/scene?acm=03653.1003.1.740603&aldid=VyrSqx06&wh_id=1397&scm=1003.1.03653.SUB_1397_740603&pos=14",
	  "_s_": "d",
	  "itemImg": "//img.alicdn.com/tps/i4/TB1dCxvMXXXXXc1XpXXKofZZpXX-182-206.jpg",
	  "sceneCat": "122846004,50020776,50024799,50024801,50024921,50002794,50020778,50002792,50002791,50002793,50003942,50005500,50005497,50001692,50005495,50005496,50005503,50005498,50005499,50005501,50005502,50003455,50003457,122904003,50023091,50020770,50008863,50005507,121414011,50020768,50022701,50008862,121420012,50020766,50020764,121398012,121380011,50020751,50020676,50024945,50020720,50002790,50020730,50001866,50020678,121476008,121458013,50002795,121364011,50005505,121422014,50024943,50000583,50000582,50000584,213002,50005033,50000563,50008607,50006988,50008608,50024601,50024602,50012051,50001871,122854005,121458010,121452006,121416008,50008779,50002777,121394007,121386009,50010825,50013810,121434010,50001865,50008565,50008246,50017192,121404045,121368009,121462014,50019372,50008248,121418011,50008249,50017193,50008252,50008251,50000512,121364047,122846004,50020776,50024799,50024801,50024921,50002794,50020778,50002792,50002791,50002793,50003942,50005500,50005497,50001692,50005495,50005496,50005503,50005498,50005499,50005501,50005502,50003455,50003457,122904003,50023091,50020770,50008863,50005507,121414011,50020768,50022701,50008862,121420012,50020766,50020764,121398012,121380011,50020751,50020676,50024945,50020720,50002790,50020730,50001866,50020678,121476008,121458013,50002795,121364011,50005505,121422014,50024943,50000583,50000582,50000584,213002,50005033,50000563,50008607,50006988,50008608,50024601,50024602,50012051,50020835,50021907,121472021,50021902,50005919,50002045,122724003,122652007,121408025,122688004,50021924,50021923,50000561,50011738,122676005,121396026,121388028,50003338,50021915,50011415,50011416,50003454,50021928,50021930,50021929,50021931,50024939,121384024,121472020,50020902,50020903,50020904,50020905,50021888,50021889,50021890,50023266,122960005,50012001,50011999,50012004,50012000,50012006,121394024,121412033,121394025,121450036,121472007,121480009,121388013,121466009,121484012,121366014,121434014,121422012,121476006,121396013,50011997,121434013,122646001,121458012,121468012,121368013,50011979,121366015,121406016,121456012,121366011,121414010,121460006,121368014,121388016,121396013,121400010,121470011,121474011,121478009,124760009,124768010,124822006,121482009,122430002,121454014,121392016，50001871,122854005,121458010,121452006,121416008,50008779,50002777,121394007,121386009,50010825,50013810,121434010,50001865,50008565,50008246,50017192,121404045,121368009,121462014,50019372,50008248,121418011,50008249,50017193,50008252,50008251,50000512,121364047,122846004,50020776,50024799,50024801,50024921,50002794,50020778,50002792,50002791,50002793,50003942,50005500,50005497,50001692,50005495,50005496,50005503,50005498,50005499,50005501,50005502,50003455,50003457,122904003,50023091,50020770,50008863,50005507,121414011,50020768,50022701,50008862,121420012,50020766,50020764,121398012,121380011,50020751,50020676,50024945,50020720,50002790,50020730,50001866,50020678,121476008,121458013,50002795,121364011,50005505,121422014,50024943,50000583,50000582,50000584,213002,50005033,50000563,50008607,50006988,50008608,50024601,50024602,50012051",
	  "finalScore": "3.4206666666666665",
	  "_pos_": "14",
	  "entityType": "SUB",
	  "benefitImg": "//img.alicdn.com/tps/i3/TB1ZARnMXXXXXaGXVXX3bpXTXXX-170-100.png"
	}, {
	  "id": "1608",
	  "sceneUrl": "https://pages.tmall.com/wow/act/15617/scene?acm=03653.1003.1.740603&aldid=VyrSqx06&wh_id=1608&scm=1003.1.03653.SUB_1608_740603&pos=15",
	  "_s_": "d",
	  "itemImg": "//img.alicdn.com/tps/i3/TB1xA3jLVXXXXXqapXXKofZZpXX-182-206.jpg",
	  "sceneCat": "50012010,50012027,50012032,50007003,50008881,50008883,1623,162116,50010850,123216004",
	  "finalScore": "0.0",
	  "_pos_": "15",
	  "entityType": "SUB",
	  "benefitImg": "//img.alicdn.com/tps/i2/TB1WvoALVXXXXcdXpXX3bpXTXXX-170-100.png"
	}];

	module.exports = {
	  components: {
	    banner: __webpack_require__(240)
	  },
	  props: {
	    ds: {
	      default: function _default() {
	        return {};
	      }
	    }
	  },
	  data: function data() {
	    return {
	      NUMBER_186: 186,
	      NUMBER_206: 206,
	      NUMBER_373: 373,
	      NUMBER_230: 230,
	      NUMBER_170: 170,
	      NUMBER_100: 100,
	      NUMBER_750: 750,
	      NUMBER_300: 300,
	      NUMBER_340: 340,
	      smallItemBg: 'https://gw.alicdn.com/tps/TB1oBIeLVXXXXatXpXXXXXXXXXX-373-230.jpg',
	      items: [],
	      smallItems: [],
	      sceneBgs: ['https://gw.alicdn.com/tps/TB1xBT8LVXXXXbYXVXXXXXXXXXX-750-340.jpg', 'https://gw.alicdn.com/tps/TB1RoEkLVXXXXaMXpXXXXXXXXXX-750-340.jpg', 'https://gw.alicdn.com/tps/TB1DiEpLVXXXXa7XXXXXXXXXXXX-750-340.jpg']
	    };
	  },
	  created: function created() {
	    this.items = demoContent.slice(0, 3);
	    this.smallItems = demoContent.slice(3);
	  },
	  methods: {
	    getItemTop: function getItemTop(i) {
	      return (i - i % 2) / 2 * (230 + 4);
	    },
	    getItemLeft: function getItemLeft(i) {
	      return i % 2 * (373 + 4);
	    }
	  }
	};

/***/ }),
/* 269 */
/***/ (function(module, exports, __webpack_require__) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', [(_vm.ds.floorTitle) ? _c('image', {
	    staticClass: "title",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "src": _vm.ds.floorTitle
	    }
	  }) : _vm._e(), _vm._v(" "), _c('div', {
	    staticClass: "small-item-container",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle({
	      height: Math.round(_vm.smallItems.length / 2) * 234
	    }))
	  }, _vm._l((_vm.smallItems), function(item, i) {
	    return _c('div', {
	      key: i,
	      staticClass: "small-item",
	      staticStyle: _vm.$processStyle(undefined),
	      style: (_vm.$processStyle({
	        top: _vm.getItemTop(i),
	        left: _vm.getItemLeft(i)
	      }))
	    }, [_c('div', [_c('image', {
	      staticClass: "small-item-bg",
	      staticStyle: _vm.$processStyle(undefined),
	      style: (_vm.$processStyle(undefined)),
	      attrs: {
	        "src": _vm.smallItemBg
	      }
	    })]), _vm._v(" "), _c('div', {
	      staticClass: "small-item-img",
	      staticStyle: _vm.$processStyle(undefined),
	      style: (_vm.$processStyle(undefined))
	    }, [_c('banner', {
	      staticStyle: _vm.$processStyle(undefined),
	      style: (_vm.$processStyle(undefined)),
	      attrs: {
	        "width": _vm.NUMBER_186,
	        "height": _vm.NUMBER_206,
	        "src": item.itemImg,
	        "href": item.sceneUrl
	      }
	    })], 1), _vm._v(" "), _c('div', {
	      staticClass: "small-benifit-img",
	      staticStyle: _vm.$processStyle(undefined),
	      style: (_vm.$processStyle(undefined))
	    }, [_c('banner', {
	      staticStyle: _vm.$processStyle(undefined),
	      style: (_vm.$processStyle(undefined)),
	      attrs: {
	        "width": _vm.NUMBER_170,
	        "height": _vm.NUMBER_100,
	        "src": item.benefitImg,
	        "href": item.sceneUrl
	      }
	    })], 1)])
	  }))])
	},staticRenderFns: []}
	module.exports.render._withStripped = true
	if (false) {
	  module.hot.accept()
	  if (module.hot.data) {
	     require("vue-hot-reload-api").rerender("data-v-52519b84", module.exports)
	  }
	}

/***/ }),
/* 270 */
/***/ (function(module, exports, __webpack_require__) {

	var disposed = false
	function injectStyle (ssrContext) {
	  if (disposed) return
	  __webpack_require__(271)
	}
	var Component = __webpack_require__(7)(
	  /* script */
	  __webpack_require__(273),
	  /* template */
	  __webpack_require__(274),
	  /* styles */
	  injectStyle,
	  /* scopeId */
	  "data-v-d746a212",
	  /* moduleIdentifier (server only) */
	  null
	)
	Component.options.__file = "/Users/Hanks/Codes/work/incubator-weex/examples/vue/showcase/include/match.vue"
	if (Component.esModule && Object.keys(Component.esModule).some(function (key) {return key !== "default" && key.substr(0, 2) !== "__"})) {console.error("named exports are not supported in *.vue files.")}
	if (Component.options.functional) {console.error("[vue-loader] match.vue: functional components are not supported with templates, they should use render functions.")}

	/* hot reload */
	if (false) {(function () {
	  var hotAPI = require("vue-hot-reload-api")
	  hotAPI.install(require("vue"), false)
	  if (!hotAPI.compatible) return
	  module.hot.accept()
	  if (!module.hot.data) {
	    hotAPI.createRecord("data-v-d746a212", Component.options)
	  } else {
	    hotAPI.reload("data-v-d746a212", Component.options)
	  }
	  module.hot.dispose(function (data) {
	    disposed = true
	  })
	})()}

	module.exports = Component.exports


/***/ }),
/* 271 */
/***/ (function(module, exports, __webpack_require__) {

	// style-loader: Adds some css to the DOM by adding a <style> tag

	// load the styles
	var content = __webpack_require__(272);
	if(typeof content === 'string') content = [[module.id, content, '']];
	if(content.locals) module.exports = content.locals;
	// add the styles to the DOM
	var update = __webpack_require__(5)("c7d918cc", content, false);
	// Hot Module Replacement
	if(false) {
	 // When the styles change, update the <style> tags
	 if(!content.locals) {
	   module.hot.accept("!!../../../../node_modules/css-loader/index.js!../../../../node_modules/vue-loader/lib/style-compiler/index.js?{\"vue\":true,\"id\":\"data-v-d746a212\",\"scoped\":true,\"hasInlineConfig\":false}!../../../../node_modules/vue-loader/lib/selector.js?type=styles&index=0!./match.vue", function() {
	     var newContent = require("!!../../../../node_modules/css-loader/index.js!../../../../node_modules/vue-loader/lib/style-compiler/index.js?{\"vue\":true,\"id\":\"data-v-d746a212\",\"scoped\":true,\"hasInlineConfig\":false}!../../../../node_modules/vue-loader/lib/selector.js?type=styles&index=0!./match.vue");
	     if(typeof newContent === 'string') newContent = [[module.id, newContent, '']];
	     update(newContent);
	   });
	 }
	 // When the module is disposed, remove the <style> tags
	 module.hot.dispose(function() { update(); });
	}

/***/ }),
/* 272 */
/***/ (function(module, exports, __webpack_require__) {

	exports = module.exports = __webpack_require__(4)();
	// imports


	// module
	exports.push([module.id, "\n.title[data-v-d746a212] {\n  width: 750;\n  height: 100;\n}\n.tm-banner[data-v-d746a212]{\n  bottom:4;\n}\n", ""]);

	// exports


/***/ }),
/* 273 */
/***/ (function(module, exports, __webpack_require__) {

	'use strict';

	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//

	module.exports = {
	  components: {
	    banner: __webpack_require__(240)
	  },
	  props: {
	    ds: {
	      default: function _default() {
	        return {};
	      }
	    }
	  },
	  data: function data() {
	    return {
	      NUMBER_750: 750,
	      NUMBER_373: 373,
	      NUMBER_240: 240,
	      NUMBER_200: 200,
	      NUMBER_4: 4
	    };
	  }
	};

/***/ }),
/* 274 */
/***/ (function(module, exports, __webpack_require__) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', [(_vm.ds.floorTitle) ? _c('image', {
	    staticClass: "title",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "src": _vm.ds.floorTitle
	    }
	  }) : _vm._e(), _vm._v(" "), (_vm.ds.isShowTmSpecBanner && _vm.ds.tmSpecBanner) ? _c('div', {
	    staticClass: "tm-banner",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined))
	  }, [_c('banner', {
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "width": _vm.NUMBER_750,
	      "height": _vm.NUMBER_200,
	      "src": _vm.ds.tmSpecBanner.img,
	      "href": _vm.ds.tmSpecBanner.url
	    }
	  })], 1) : _vm._e(), _vm._v(" "), _vm._l((_vm.ds.bannerItems), function(item, i) {
	    return _c('div', {
	      key: i,
	      staticStyle: _vm.$processStyle(undefined),
	      style: (_vm.$processStyle({
	        flexDirection: 'row',
	        marginBottom: _vm.NUMBER_4
	      }))
	    }, [_c('div', {
	      staticStyle: _vm.$processStyle(undefined),
	      style: (_vm.$processStyle({
	        marginRight: _vm.NUMBER_4
	      }))
	    }, [_c('banner', {
	      staticStyle: _vm.$processStyle(undefined),
	      style: (_vm.$processStyle(undefined)),
	      attrs: {
	        "width": _vm.NUMBER_373,
	        "height": _vm.NUMBER_240,
	        "src": item.leftImg,
	        "href": item.leftUrl
	      }
	    })], 1), _vm._v(" "), _c('div', [_c('banner', {
	      staticStyle: _vm.$processStyle(undefined),
	      style: (_vm.$processStyle(undefined)),
	      attrs: {
	        "width": _vm.NUMBER_373,
	        "height": _vm.NUMBER_240,
	        "src": item.rightImg,
	        "href": item.rightUrl
	      }
	    })], 1)])
	  })], 2)
	},staticRenderFns: []}
	module.exports.render._withStripped = true
	if (false) {
	  module.hot.accept()
	  if (module.hot.data) {
	     require("vue-hot-reload-api").rerender("data-v-d746a212", module.exports)
	  }
	}

/***/ }),
/* 275 */
/***/ (function(module, exports, __webpack_require__) {

	var disposed = false
	function injectStyle (ssrContext) {
	  if (disposed) return
	  __webpack_require__(276)
	}
	var Component = __webpack_require__(7)(
	  /* script */
	  __webpack_require__(278),
	  /* template */
	  __webpack_require__(279),
	  /* styles */
	  injectStyle,
	  /* scopeId */
	  "data-v-40d66fb9",
	  /* moduleIdentifier (server only) */
	  null
	)
	Component.options.__file = "/Users/Hanks/Codes/work/incubator-weex/examples/vue/showcase/include/brand.vue"
	if (Component.esModule && Object.keys(Component.esModule).some(function (key) {return key !== "default" && key.substr(0, 2) !== "__"})) {console.error("named exports are not supported in *.vue files.")}
	if (Component.options.functional) {console.error("[vue-loader] brand.vue: functional components are not supported with templates, they should use render functions.")}

	/* hot reload */
	if (false) {(function () {
	  var hotAPI = require("vue-hot-reload-api")
	  hotAPI.install(require("vue"), false)
	  if (!hotAPI.compatible) return
	  module.hot.accept()
	  if (!module.hot.data) {
	    hotAPI.createRecord("data-v-40d66fb9", Component.options)
	  } else {
	    hotAPI.reload("data-v-40d66fb9", Component.options)
	  }
	  module.hot.dispose(function (data) {
	    disposed = true
	  })
	})()}

	module.exports = Component.exports


/***/ }),
/* 276 */
/***/ (function(module, exports, __webpack_require__) {

	// style-loader: Adds some css to the DOM by adding a <style> tag

	// load the styles
	var content = __webpack_require__(277);
	if(typeof content === 'string') content = [[module.id, content, '']];
	if(content.locals) module.exports = content.locals;
	// add the styles to the DOM
	var update = __webpack_require__(5)("94c84bf4", content, false);
	// Hot Module Replacement
	if(false) {
	 // When the styles change, update the <style> tags
	 if(!content.locals) {
	   module.hot.accept("!!../../../../node_modules/css-loader/index.js!../../../../node_modules/vue-loader/lib/style-compiler/index.js?{\"vue\":true,\"id\":\"data-v-40d66fb9\",\"scoped\":true,\"hasInlineConfig\":false}!../../../../node_modules/vue-loader/lib/selector.js?type=styles&index=0!./brand.vue", function() {
	     var newContent = require("!!../../../../node_modules/css-loader/index.js!../../../../node_modules/vue-loader/lib/style-compiler/index.js?{\"vue\":true,\"id\":\"data-v-40d66fb9\",\"scoped\":true,\"hasInlineConfig\":false}!../../../../node_modules/vue-loader/lib/selector.js?type=styles&index=0!./brand.vue");
	     if(typeof newContent === 'string') newContent = [[module.id, newContent, '']];
	     update(newContent);
	   });
	 }
	 // When the module is disposed, remove the <style> tags
	 module.hot.dispose(function() { update(); });
	}

/***/ }),
/* 277 */
/***/ (function(module, exports, __webpack_require__) {

	exports = module.exports = __webpack_require__(4)();
	// imports


	// module
	exports.push([module.id, "\n.title[data-v-40d66fb9] {\n  width: 750;\n  height: 100;\n}\n.container[data-v-40d66fb9] {\n  margin-bottom: 4;\n  background-color:#C0BABC;\n}\n", ""]);

	// exports


/***/ }),
/* 278 */
/***/ (function(module, exports, __webpack_require__) {

	'use strict';

	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//

	module.exports = {
	  components: {
	    banners: __webpack_require__(238)
	  },
	  props: {
	    ds: {
	      default: function _default() {
	        return {
	          bannerItems: []
	        };
	      }
	    }
	  },
	  data: function data() {
	    return {
	      NUMBER_742: 742,
	      NUMBER_230: 230,
	      NUMBER_4: 4
	    };
	  },
	  computed: {
	    bannerItems: function bannerItems() {
	      return this.ds.bannerItems.slice(0, 8);
	    }
	  },
	  created: function created() {
	    var bannerItems = this.ds.bannerItems;
	    bannerItems.sort(function () {
	      return Math.random() - 0.5;
	    });
	    for (var i = 0; i < bannerItems.length; i++) {
	      var item = bannerItems[i];
	      if (i % 2 === 0) {
	        item.img = item.leftImg;
	        item.url = item.rightUrl;
	      } else {
	        item.img = item.rightImg;
	        item.url = item.rightUrl;
	      }
	    }
	  }
	};

/***/ }),
/* 279 */
/***/ (function(module, exports, __webpack_require__) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', {
	    staticClass: "container",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined))
	  }, [(_vm.ds.floorTitle) ? _c('image', {
	    staticClass: "title",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "src": _vm.ds.floorTitle
	    }
	  }) : _vm._e(), _vm._v(" "), _c('div', {
	    staticStyle: _vm.$processStyle({
	      "margin-left": "4",
	      "margin-right": "4"
	    }),
	    style: (_vm.$processStyle(undefined))
	  }, [_c('banners', {
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "direction": "column",
	      "ds": _vm.bannerItems,
	      "width": _vm.NUMBER_742,
	      "height": _vm.NUMBER_230,
	      "space": _vm.NUMBER_4
	    }
	  })], 1)])
	},staticRenderFns: []}
	module.exports.render._withStripped = true
	if (false) {
	  module.hot.accept()
	  if (module.hot.data) {
	     require("vue-hot-reload-api").rerender("data-v-40d66fb9", module.exports)
	  }
	}

/***/ }),
/* 280 */
/***/ (function(module, exports, __webpack_require__) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('scroller', {
	    staticClass: "content",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined))
	  }, [_c('refresh', {
	    staticClass: "refresh",
	    staticStyle: _vm.$processStyle({
	      "flex-direction": "row"
	    }),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "display": _vm.displayRefresh
	    },
	    on: {
	      "refresh": _vm.handleRefresh
	    }
	  }, [_c('loading-indicator'), _vm._v(" "), _c('text', {
	    staticStyle: _vm.$processStyle({
	      "margin-left": "36px",
	      "color": "#eee"
	    }),
	    style: (_vm.$processStyle(undefined))
	  }, [_vm._v("下拉刷新...")])], 1), _vm._v(" "), (_vm.show0) ? _c('headlines', {
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "ds": _vm.showData.header
	    }
	  }) : _vm._e(), _vm._v(" "), (_vm.show0) ? _c('category', {
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "ds": _vm.showData.category
	    }
	  }) : _vm._e(), _vm._v(" "), (_vm.show1) ? _c('coupon', {
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "ds": _vm.showData.coupon
	    }
	  }) : _vm._e(), _vm._v(" "), (_vm.show1) ? _c('goods', {
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "ds": _vm.showData.goods
	    }
	  }) : _vm._e(), _vm._v(" "), (_vm.show1) ? _c('resource', {
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "ds": _vm.showData.resource
	    }
	  }) : _vm._e(), _vm._v(" "), (_vm.show1) ? _c('scene', {
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "ds": _vm.showData.scene
	    }
	  }) : _vm._e(), _vm._v(" "), (_vm.show2) ? _c('match', {
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "ds": _vm.showData.match
	    }
	  }) : _vm._e(), _vm._v(" "), (_vm.show2) ? _c('brand', {
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "ds": _vm.showData.brand
	    }
	  }) : _vm._e(), _vm._v(" "), _c('loading', {
	    staticClass: "loading",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "display": _vm.displayLoading
	    },
	    on: {
	      "loading": _vm.handleLoading
	    }
	  }, [_c('text', {
	    staticStyle: _vm.$processStyle({
	      "color": "#eee"
	    }),
	    style: (_vm.$processStyle(undefined))
	  }, [_vm._v("加载更多...")])])], 1)
	},staticRenderFns: []}
	module.exports.render._withStripped = true
	if (false) {
	  module.hot.accept()
	  if (module.hot.data) {
	     require("vue-hot-reload-api").rerender("data-v-1ef3bcb8", module.exports)
	  }
	}

/***/ })
/******/ ]);