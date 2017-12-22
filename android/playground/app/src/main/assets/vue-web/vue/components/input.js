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
/******/ ({

/***/ 0:
/***/ (function(module, exports, __webpack_require__) {

	var App = __webpack_require__(43)
	App.el = '#root'
	new Vue(App)


/***/ }),

/***/ 4:
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

/***/ 5:
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

/***/ 6:
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

/***/ 7:
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

/***/ 43:
/***/ (function(module, exports, __webpack_require__) {

	var disposed = false
	function injectStyle (ssrContext) {
	  if (disposed) return
	  __webpack_require__(44)
	}
	var Component = __webpack_require__(7)(
	  /* script */
	  __webpack_require__(46),
	  /* template */
	  __webpack_require__(47),
	  /* styles */
	  injectStyle,
	  /* scopeId */
	  "data-v-de6618e8",
	  /* moduleIdentifier (server only) */
	  null
	)
	Component.options.__file = "/Users/Hanks/Codes/work/incubator-weex/examples/vue/components/input.vue"
	if (Component.esModule && Object.keys(Component.esModule).some(function (key) {return key !== "default" && key.substr(0, 2) !== "__"})) {console.error("named exports are not supported in *.vue files.")}
	if (Component.options.functional) {console.error("[vue-loader] input.vue: functional components are not supported with templates, they should use render functions.")}

	/* hot reload */
	if (false) {(function () {
	  var hotAPI = require("vue-hot-reload-api")
	  hotAPI.install(require("vue"), false)
	  if (!hotAPI.compatible) return
	  module.hot.accept()
	  if (!module.hot.data) {
	    hotAPI.createRecord("data-v-de6618e8", Component.options)
	  } else {
	    hotAPI.reload("data-v-de6618e8", Component.options)
	  }
	  module.hot.dispose(function (data) {
	    disposed = true
	  })
	})()}

	module.exports = Component.exports


/***/ }),

/***/ 44:
/***/ (function(module, exports, __webpack_require__) {

	// style-loader: Adds some css to the DOM by adding a <style> tag

	// load the styles
	var content = __webpack_require__(45);
	if(typeof content === 'string') content = [[module.id, content, '']];
	if(content.locals) module.exports = content.locals;
	// add the styles to the DOM
	var update = __webpack_require__(5)("ab21e8f8", content, false);
	// Hot Module Replacement
	if(false) {
	 // When the styles change, update the <style> tags
	 if(!content.locals) {
	   module.hot.accept("!!../../../node_modules/css-loader/index.js!../../../node_modules/vue-loader/lib/style-compiler/index.js?{\"vue\":true,\"id\":\"data-v-de6618e8\",\"scoped\":true,\"hasInlineConfig\":false}!../../../node_modules/vue-loader/lib/selector.js?type=styles&index=0!./input.vue", function() {
	     var newContent = require("!!../../../node_modules/css-loader/index.js!../../../node_modules/vue-loader/lib/style-compiler/index.js?{\"vue\":true,\"id\":\"data-v-de6618e8\",\"scoped\":true,\"hasInlineConfig\":false}!../../../node_modules/vue-loader/lib/selector.js?type=styles&index=0!./input.vue");
	     if(typeof newContent === 'string') newContent = [[module.id, newContent, '']];
	     update(newContent);
	   });
	 }
	 // When the module is disposed, remove the <style> tags
	 module.hot.dispose(function() { update(); });
	}

/***/ }),

/***/ 45:
/***/ (function(module, exports, __webpack_require__) {

	exports = module.exports = __webpack_require__(4)();
	// imports


	// module
	exports.push([module.id, "\n.input[data-v-de6618e8] {\n  font-size: 60px;\n  height: 80px;\n  width: 750px;\n}\n.button[data-v-de6618e8] {\n  font-size: 36;\n  width: 200;\n  color: #41B883;\n  text-align: center;\n  padding-top: 10;\n  padding-bottom: 10;\n  border-width: 2;\n  border-style: solid;\n  margin-right: 20;\n  border-color: rgb(162, 217, 192);\n  background-color: rgba(162, 217, 192, 0.2);\n}\n", ""]);

	// exports


/***/ }),

/***/ 46:
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
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
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
	  data: function data() {
	    return {
	      txtInput: '',
	      txtChange: '',
	      txtReturnType: '',
	      txtSelection: '',
	      autofocus: false
	    };
	  },
	  methods: {
	    ready: function ready() {
	      var self = this;
	      setTimeout(function () {
	        self.autofocus = true;
	      }, 1000);
	    },
	    onchange: function onchange(event) {
	      this.txtChange = event.value;
	      console.log('onchange', event.value);
	    },
	    onreturn: function onreturn(event) {
	      this.txtReturnType = event.returnKeyType;
	      console.log('onreturn', event.type);
	    },
	    oninput: function oninput(event) {
	      this.txtInput = event.value;
	      console.log('oninput', event.value);
	    },
	    focus: function focus() {
	      this.$refs['input1'].focus();
	    },
	    blur: function blur() {
	      this.$refs['input1'].blur();
	    },
	    setRange: function setRange() {
	      console.log(this.$refs["inputselection"]);
	      this.$refs["inputselection"].setSelectionRange(2, 6);
	    },
	    getSelectionRange: function getSelectionRange() {
	      console.log(this.$refs["inputselection"]);
	      var self = this;
	      this.$refs["inputselection"].getSelectionRange(function (e) {
	        self.txtSelection = e.selectionStart + '-' + e.selectionEnd;
	      });
	    }
	  }
	};

/***/ }),

/***/ 47:
/***/ (function(module, exports, __webpack_require__) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', [_c('div', [_c('text', {
	    staticStyle: _vm.$processStyle({
	      "font-size": "40px"
	    }),
	    style: (_vm.$processStyle(undefined))
	  }, [_vm._v("oninput: " + _vm._s(_vm.txtInput))]), _vm._v(" "), _c('text', {
	    staticStyle: _vm.$processStyle({
	      "font-size": "40px"
	    }),
	    style: (_vm.$processStyle(undefined))
	  }, [_vm._v("onchange: " + _vm._s(_vm.txtChange))]), _vm._v(" "), _c('text', {
	    staticStyle: _vm.$processStyle({
	      "font-size": "40px"
	    }),
	    style: (_vm.$processStyle(undefined))
	  }, [_vm._v("onreturntype: " + _vm._s(_vm.txtReturnType))]), _vm._v(" "), _c('text', {
	    staticStyle: _vm.$processStyle({
	      "font-size": "40px"
	    }),
	    style: (_vm.$processStyle(undefined))
	  }, [_vm._v("selection: " + _vm._s(_vm.txtSelection))])]), _vm._v(" "), _c('scroller', [_c('div', [_c('div', {
	    staticStyle: _vm.$processStyle({
	      "background-color": "#286090"
	    }),
	    style: (_vm.$processStyle(undefined))
	  }, [_c('text', {
	    staticClass: "title",
	    staticStyle: _vm.$processStyle({
	      "height": "80",
	      "padding": "20",
	      "color": "#FFFFFF"
	    }),
	    style: (_vm.$processStyle(undefined))
	  }, [_vm._v("input type = text")])]), _vm._v(" "), _c('input', {
	    staticClass: "input",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "type": "text",
	      "placeholder": "Input Text",
	      "autofocus": true,
	      "value": ""
	    },
	    on: {
	      "change": _vm.onchange,
	      "input": _vm.oninput
	    }
	  })]), _vm._v(" "), _c('div', [_c('div', {
	    staticStyle: _vm.$processStyle({
	      "background-color": "#286090"
	    }),
	    style: (_vm.$processStyle(undefined))
	  }, [_c('text', {
	    staticClass: "title",
	    staticStyle: _vm.$processStyle({
	      "height": "80",
	      "padding": "20",
	      "color": "#FFFFFF"
	    }),
	    style: (_vm.$processStyle(undefined))
	  }, [_vm._v("input type = password")])]), _vm._v(" "), _c('input', {
	    staticClass: "input",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "type": "password",
	      "placeholder": "Input Password"
	    },
	    on: {
	      "change": _vm.onchange,
	      "input": _vm.oninput
	    }
	  })]), _vm._v(" "), _c('div', [_c('div', {
	    staticStyle: _vm.$processStyle({
	      "background-color": "#286090"
	    }),
	    style: (_vm.$processStyle(undefined))
	  }, [_c('text', {
	    staticClass: "title",
	    staticStyle: _vm.$processStyle({
	      "height": "80",
	      "padding": "20",
	      "color": "#FFFFFF"
	    }),
	    style: (_vm.$processStyle(undefined))
	  }, [_vm._v("input type = url")])]), _vm._v(" "), _c('input', {
	    staticClass: "input",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "type": "url",
	      "placeholder": "Input URL"
	    },
	    on: {
	      "change": _vm.onchange,
	      "input": _vm.oninput
	    }
	  })]), _vm._v(" "), _c('div', [_c('div', {
	    staticStyle: _vm.$processStyle({
	      "background-color": "#286090"
	    }),
	    style: (_vm.$processStyle(undefined))
	  }, [_c('text', {
	    staticClass: "title",
	    staticStyle: _vm.$processStyle({
	      "height": "80",
	      "padding": "20",
	      "color": "#FFFFFF"
	    }),
	    style: (_vm.$processStyle(undefined))
	  }, [_vm._v("input type = email")])]), _vm._v(" "), _c('input', {
	    staticClass: "input",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "type": "email",
	      "placeholder": "Input Email"
	    },
	    on: {
	      "change": _vm.onchange,
	      "input": _vm.oninput
	    }
	  })]), _vm._v(" "), _c('div', [_c('div', {
	    staticStyle: _vm.$processStyle({
	      "background-color": "#286090"
	    }),
	    style: (_vm.$processStyle(undefined))
	  }, [_c('text', {
	    staticClass: "title",
	    staticStyle: _vm.$processStyle({
	      "height": "80",
	      "padding": "20",
	      "color": "#FFFFFF"
	    }),
	    style: (_vm.$processStyle(undefined))
	  }, [_vm._v("input type = tel")])]), _vm._v(" "), _c('input', {
	    staticClass: "input",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "type": "tel",
	      "placeholder": "Input Tel"
	    },
	    on: {
	      "change": _vm.onchange,
	      "input": _vm.oninput
	    }
	  })]), _vm._v(" "), _c('div', [_c('div', {
	    staticStyle: _vm.$processStyle({
	      "background-color": "#286090"
	    }),
	    style: (_vm.$processStyle(undefined))
	  }, [_c('text', {
	    staticClass: "title",
	    staticStyle: _vm.$processStyle({
	      "height": "80",
	      "padding": "20",
	      "color": "#FFFFFF"
	    }),
	    style: (_vm.$processStyle(undefined))
	  }, [_vm._v("input type = time")])]), _vm._v(" "), _c('input', {
	    staticClass: "input",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "type": "time",
	      "placeholder": "Input Time"
	    },
	    on: {
	      "change": _vm.onchange,
	      "input": _vm.oninput
	    }
	  })]), _vm._v(" "), _c('div', [_c('div', {
	    staticStyle: _vm.$processStyle({
	      "background-color": "#286090"
	    }),
	    style: (_vm.$processStyle(undefined))
	  }, [_c('text', {
	    staticClass: "title",
	    staticStyle: _vm.$processStyle({
	      "height": "80",
	      "padding": "20",
	      "color": "#FFFFFF"
	    }),
	    style: (_vm.$processStyle(undefined))
	  }, [_vm._v("input type = number")])]), _vm._v(" "), _c('input', {
	    staticClass: "input",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "type": "number",
	      "placeholder": "Input number"
	    },
	    on: {
	      "change": _vm.onchange,
	      "input": _vm.oninput
	    }
	  })]), _vm._v(" "), _c('div', [_c('div', {
	    staticStyle: _vm.$processStyle({
	      "background-color": "#286090"
	    }),
	    style: (_vm.$processStyle(undefined))
	  }, [_c('text', {
	    staticClass: "title",
	    staticStyle: _vm.$processStyle({
	      "height": "80",
	      "padding": "20",
	      "color": "#FFFFFF"
	    }),
	    style: (_vm.$processStyle(undefined))
	  }, [_vm._v("input type = date")])]), _vm._v(" "), _c('input', {
	    staticClass: "input",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "type": "date",
	      "placeholder": "Input Date",
	      "max": "2017-12-12",
	      "min": "2015-01-01"
	    },
	    on: {
	      "change": _vm.onchange,
	      "input": _vm.oninput
	    }
	  })]), _vm._v(" "), _c('div', [_c('div', {
	    staticStyle: _vm.$processStyle({
	      "background-color": "#286090"
	    }),
	    style: (_vm.$processStyle(undefined))
	  }, [_c('text', {
	    staticClass: "title",
	    staticStyle: _vm.$processStyle({
	      "height": "80",
	      "padding": "20",
	      "color": "#FFFFFF"
	    }),
	    style: (_vm.$processStyle(undefined))
	  }, [_vm._v("input return-key-type = default")])]), _vm._v(" "), _c('input', {
	    staticClass: "input",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "type": "text",
	      "placeholder": "please input",
	      "return-key-type": "default"
	    },
	    on: {
	      "change": _vm.onchange,
	      "return": _vm.onreturn,
	      "input": _vm.oninput
	    }
	  })]), _vm._v(" "), _c('div', [_c('div', {
	    staticStyle: _vm.$processStyle({
	      "background-color": "#286090"
	    }),
	    style: (_vm.$processStyle(undefined))
	  }, [_c('text', {
	    staticClass: "title",
	    staticStyle: _vm.$processStyle({
	      "height": "80",
	      "padding": "20",
	      "color": "#FFFFFF"
	    }),
	    style: (_vm.$processStyle(undefined))
	  }, [_vm._v("input return-key-type = go")])]), _vm._v(" "), _c('input', {
	    staticClass: "input",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "type": "text",
	      "placeholder": "please input",
	      "return-key-type": "go"
	    },
	    on: {
	      "change": _vm.onchange,
	      "return": _vm.onreturn,
	      "input": _vm.oninput
	    }
	  })]), _vm._v(" "), _c('div', [_c('div', {
	    staticStyle: _vm.$processStyle({
	      "background-color": "#286090"
	    }),
	    style: (_vm.$processStyle(undefined))
	  }, [_c('text', {
	    staticClass: "title",
	    staticStyle: _vm.$processStyle({
	      "height": "80",
	      "padding": "20",
	      "color": "#FFFFFF"
	    }),
	    style: (_vm.$processStyle(undefined))
	  }, [_vm._v("input return-key-type = next")])]), _vm._v(" "), _c('input', {
	    staticClass: "input",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "type": "text",
	      "placeholder": "please input",
	      "return-key-type": "next"
	    },
	    on: {
	      "change": _vm.onchange,
	      "return": _vm.onreturn,
	      "input": _vm.oninput
	    }
	  })]), _vm._v(" "), _c('div', [_c('div', {
	    staticStyle: _vm.$processStyle({
	      "background-color": "#286090"
	    }),
	    style: (_vm.$processStyle(undefined))
	  }, [_c('text', {
	    staticClass: "title",
	    staticStyle: _vm.$processStyle({
	      "height": "80",
	      "padding": "20",
	      "color": "#FFFFFF"
	    }),
	    style: (_vm.$processStyle(undefined))
	  }, [_vm._v("input return-key-type = search")])]), _vm._v(" "), _c('input', {
	    staticClass: "input",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "type": "text",
	      "placeholder": "please input",
	      "return-key-type": "search"
	    },
	    on: {
	      "change": _vm.onchange,
	      "return": _vm.onreturn,
	      "input": _vm.oninput
	    }
	  })]), _vm._v(" "), _c('div', [_c('div', {
	    staticStyle: _vm.$processStyle({
	      "background-color": "#286090"
	    }),
	    style: (_vm.$processStyle(undefined))
	  }, [_c('text', {
	    staticClass: "title",
	    staticStyle: _vm.$processStyle({
	      "height": "80",
	      "padding": "20",
	      "color": "#FFFFFF"
	    }),
	    style: (_vm.$processStyle(undefined))
	  }, [_vm._v("input return-key-type = send")])]), _vm._v(" "), _c('input', {
	    staticClass: "input",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "type": "text",
	      "placeholder": "please input",
	      "return-key-type": "send"
	    },
	    on: {
	      "change": _vm.onchange,
	      "return": _vm.onreturn,
	      "input": _vm.oninput
	    }
	  })]), _vm._v(" "), _c('div', [_c('div', {
	    staticStyle: _vm.$processStyle({
	      "background-color": "#286090"
	    }),
	    style: (_vm.$processStyle(undefined))
	  }, [_c('text', {
	    staticClass: "title",
	    staticStyle: _vm.$processStyle({
	      "height": "80",
	      "padding": "20",
	      "color": "#FFFFFF"
	    }),
	    style: (_vm.$processStyle(undefined))
	  }, [_vm._v("input return-key-type = done")])]), _vm._v(" "), _c('input', {
	    staticClass: "input",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "type": "text",
	      "placeholder": "please input",
	      "return-key-type": "done"
	    },
	    on: {
	      "change": _vm.onchange,
	      "return": _vm.onreturn,
	      "input": _vm.oninput
	    }
	  })]), _vm._v(" "), _c('div', [_c('div', {
	    staticStyle: _vm.$processStyle({
	      "background-color": "#286090"
	    }),
	    style: (_vm.$processStyle(undefined))
	  }, [_c('text', {
	    staticClass: "title",
	    staticStyle: _vm.$processStyle({
	      "height": "80",
	      "padding": "20",
	      "color": "#FFFFFF"
	    }),
	    style: (_vm.$processStyle(undefined))
	  }, [_vm._v("function focus() & blur()")])]), _vm._v(" "), _c('div', {
	    staticStyle: _vm.$processStyle({
	      "flex-direction": "row",
	      "margin-bottom": "16px",
	      "justify-content": "space-between"
	    }),
	    style: (_vm.$processStyle(undefined))
	  }, [_c('text', {
	    staticClass: "button",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "value": "Focus",
	      "type": "primary"
	    },
	    on: {
	      "click": _vm.focus
	    }
	  }), _vm._v(" "), _c('text', {
	    staticClass: "button",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "value": "Blur",
	      "type": "primary"
	    },
	    on: {
	      "click": _vm.blur
	    }
	  })]), _vm._v(" "), _c('input', {
	    ref: "input1",
	    staticClass: "input",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "type": "text",
	      "placeholder": "Input1",
	      "value": ""
	    }
	  })]), _vm._v(" "), _c('div', [_c('div', {
	    staticStyle: _vm.$processStyle({
	      "background-color": "#286090"
	    }),
	    style: (_vm.$processStyle(undefined))
	  }, [_c('text', {
	    staticClass: "title",
	    staticStyle: _vm.$processStyle({
	      "height": "80",
	      "padding": "20",
	      "color": "#FFFFFF"
	    }),
	    style: (_vm.$processStyle(undefined))
	  }, [_vm._v("input selection")])]), _vm._v(" "), _c('div', {
	    staticStyle: _vm.$processStyle({
	      "flex-direction": "row",
	      "margin-bottom": "16px",
	      "justify-content": "space-between"
	    }),
	    style: (_vm.$processStyle(undefined))
	  }, [_c('text', {
	    staticClass: "button",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "value": "setRange",
	      "type": "primary"
	    },
	    on: {
	      "click": _vm.setRange
	    }
	  }), _vm._v(" "), _c('text', {
	    staticClass: "button",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "value": "getSelectionRange",
	      "type": "primary"
	    },
	    on: {
	      "click": _vm.getSelectionRange
	    }
	  })]), _vm._v(" "), _c('input', {
	    ref: "inputselection",
	    staticClass: "input",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "type": "text",
	      "placeholder": "please input",
	      "value": "123456789"
	    },
	    on: {
	      "change": _vm.onchange,
	      "return": _vm.onreturn,
	      "input": _vm.oninput
	    }
	  })])])], 1)
	},staticRenderFns: []}
	module.exports.render._withStripped = true
	if (false) {
	  module.hot.accept()
	  if (module.hot.data) {
	     require("vue-hot-reload-api").rerender("data-v-de6618e8", module.exports)
	  }
	}

/***/ })

/******/ });