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

	var App = __webpack_require__(293)
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

/***/ 9:
/***/ (function(module, exports, __webpack_require__) {

	var disposed = false
	function injectStyle (ssrContext) {
	  if (disposed) return
	  __webpack_require__(10)
	}
	var Component = __webpack_require__(7)(
	  /* script */
	  __webpack_require__(12),
	  /* template */
	  __webpack_require__(13),
	  /* styles */
	  injectStyle,
	  /* scopeId */
	  "data-v-66798af2",
	  /* moduleIdentifier (server only) */
	  null
	)
	Component.options.__file = "/Users/Hanks/Codes/work/incubator-weex/examples/vue/include/panel.vue"
	if (Component.esModule && Object.keys(Component.esModule).some(function (key) {return key !== "default" && key.substr(0, 2) !== "__"})) {console.error("named exports are not supported in *.vue files.")}
	if (Component.options.functional) {console.error("[vue-loader] panel.vue: functional components are not supported with templates, they should use render functions.")}

	/* hot reload */
	if (false) {(function () {
	  var hotAPI = require("vue-hot-reload-api")
	  hotAPI.install(require("vue"), false)
	  if (!hotAPI.compatible) return
	  module.hot.accept()
	  if (!module.hot.data) {
	    hotAPI.createRecord("data-v-66798af2", Component.options)
	  } else {
	    hotAPI.reload("data-v-66798af2", Component.options)
	  }
	  module.hot.dispose(function (data) {
	    disposed = true
	  })
	})()}

	module.exports = Component.exports


/***/ }),

/***/ 10:
/***/ (function(module, exports, __webpack_require__) {

	// style-loader: Adds some css to the DOM by adding a <style> tag

	// load the styles
	var content = __webpack_require__(11);
	if(typeof content === 'string') content = [[module.id, content, '']];
	if(content.locals) module.exports = content.locals;
	// add the styles to the DOM
	var update = __webpack_require__(5)("392aaf4e", content, false);
	// Hot Module Replacement
	if(false) {
	 // When the styles change, update the <style> tags
	 if(!content.locals) {
	   module.hot.accept("!!../../../node_modules/css-loader/index.js!../../../node_modules/vue-loader/lib/style-compiler/index.js?{\"vue\":true,\"id\":\"data-v-66798af2\",\"scoped\":true,\"hasInlineConfig\":false}!../../../node_modules/vue-loader/lib/selector.js?type=styles&index=0!./panel.vue", function() {
	     var newContent = require("!!../../../node_modules/css-loader/index.js!../../../node_modules/vue-loader/lib/style-compiler/index.js?{\"vue\":true,\"id\":\"data-v-66798af2\",\"scoped\":true,\"hasInlineConfig\":false}!../../../node_modules/vue-loader/lib/selector.js?type=styles&index=0!./panel.vue");
	     if(typeof newContent === 'string') newContent = [[module.id, newContent, '']];
	     update(newContent);
	   });
	 }
	 // When the module is disposed, remove the <style> tags
	 module.hot.dispose(function() { update(); });
	}

/***/ }),

/***/ 11:
/***/ (function(module, exports, __webpack_require__) {

	exports = module.exports = __webpack_require__(4)();
	// imports


	// module
	exports.push([module.id, "\n.panel[data-v-66798af2] {\n  margin-bottom: 20px;\n  background-color: #fff;\n  /*border: 1px solid transparent;*/\n  /*border-radius: 10px;*/\n  /*-webkit-box-shadow: 0 1px 1px rgba(0,0,0,.05);*/\n  /*box-shadow: 0 1px 1px rgba(0,0,0,.05);*/\n  border-color: #dddddd;\n  border-width: 1px;\n}\n.panel-default[data-v-66798af2] {\n}\n.panel-primary[data-v-66798af2] {\n  border-color: rgb(40, 96, 144);\n}\n.panel-success[data-v-66798af2] {\n  border-color: rgb(76, 174, 76);\n}\n.panel-info[data-v-66798af2] {\n  border-color: rgb(70, 184, 218);\n}\n.panel-warning[data-v-66798af2] {\n  border-color: rgb(238, 162, 54);\n}\n.panel-danger[data-v-66798af2] {\n  border-color: rgb(212, 63, 58);\n}\n.panel-header[data-v-66798af2] {\n  background-color: #f5f5f5;\n  font-size: 40px;\n  /*padding-left: 12px;*/\n  /*padding-right: 12px;*/\n  /*padding-top: 20px;*/\n  /*padding-bottom: 20px;*/\n  color: #333;\n}\n.panel-header-default[data-v-66798af2] {\n}\n.panel-header-primary[data-v-66798af2] {\n  background-color: rgb(40, 96, 144);\n  color: #ffffff;\n}\n.panel-header-success[data-v-66798af2] {\n  background-color: rgb(92, 184, 92);\n  color: #ffffff;\n}\n.panel-header-info[data-v-66798af2] {\n  background-color: rgb(91, 192, 222);\n  color: #ffffff;\n}\n.panel-header-warning[data-v-66798af2] {\n  background-color: rgb(240, 173, 78);\n  color: #ffffff;\n}\n.panel-header-danger[data-v-66798af2] {\n  background-color: rgb(217, 83, 79);\n  color: #ffffff;\n}\n.panel-body[data-v-66798af2] {\n  padding-left: 12px;\n  padding-right: 12px;\n  padding-top: 20px;\n  padding-bottom: 20px;\n}\n.panel-body-default[data-v-66798af2] {\n}\n.panel-body-primary[data-v-66798af2] {\n}\n.panel-body-success[data-v-66798af2] {\n}\n.panel-body-info[data-v-66798af2] {\n}\n.panel-body-warning[data-v-66798af2] {\n}\n.panel-body-danger[data-v-66798af2] {\n}\n", ""]);

	// exports


/***/ }),

/***/ 12:
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

	module.exports = {
	  props: {
	    type: { default: 'default' },
	    title: { default: '' },
	    paddingBody: { default: 20 },
	    paddingHead: { default: 20 },
	    dataClass: { default: '' }, // FIXME transfer class
	    border: { default: 0 }
	  }
	};

/***/ }),

/***/ 13:
/***/ (function(module, exports, __webpack_require__) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', {
	    class: ['panel', 'panel-' + _vm.type],
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle({
	      borderWidth: _vm.border
	    }))
	  }, [_c('text', {
	    class: ['panel-header', 'panel-header-' + _vm.type],
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle({
	      paddingTop: _vm.paddingHead,
	      paddingBottom: _vm.paddingHead,
	      paddingLeft: _vm.paddingHead * 1.5,
	      paddingRight: _vm.paddingHead * 1.5
	    }))
	  }, [_vm._v(_vm._s(_vm.title))]), _vm._v(" "), _c('div', {
	    class: ['panel-body', 'panel-body-' + _vm.type],
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle({
	      paddingTop: _vm.paddingBody,
	      paddingBottom: _vm.paddingBody,
	      paddingLeft: _vm.paddingBody * 1.5,
	      paddingRight: _vm.paddingBody * 1.5
	    }))
	  }, [_vm._t("default")], 2)])
	},staticRenderFns: []}
	module.exports.render._withStripped = true
	if (false) {
	  module.hot.accept()
	  if (module.hot.data) {
	     require("vue-hot-reload-api").rerender("data-v-66798af2", module.exports)
	  }
	}

/***/ }),

/***/ 22:
/***/ (function(module, exports, __webpack_require__) {

	var disposed = false
	function injectStyle (ssrContext) {
	  if (disposed) return
	  __webpack_require__(23)
	}
	var Component = __webpack_require__(7)(
	  /* script */
	  __webpack_require__(25),
	  /* template */
	  __webpack_require__(26),
	  /* styles */
	  injectStyle,
	  /* scopeId */
	  "data-v-f15788ee",
	  /* moduleIdentifier (server only) */
	  null
	)
	Component.options.__file = "/Users/Hanks/Codes/work/incubator-weex/examples/vue/include/tip.vue"
	if (Component.esModule && Object.keys(Component.esModule).some(function (key) {return key !== "default" && key.substr(0, 2) !== "__"})) {console.error("named exports are not supported in *.vue files.")}
	if (Component.options.functional) {console.error("[vue-loader] tip.vue: functional components are not supported with templates, they should use render functions.")}

	/* hot reload */
	if (false) {(function () {
	  var hotAPI = require("vue-hot-reload-api")
	  hotAPI.install(require("vue"), false)
	  if (!hotAPI.compatible) return
	  module.hot.accept()
	  if (!module.hot.data) {
	    hotAPI.createRecord("data-v-f15788ee", Component.options)
	  } else {
	    hotAPI.reload("data-v-f15788ee", Component.options)
	  }
	  module.hot.dispose(function (data) {
	    disposed = true
	  })
	})()}

	module.exports = Component.exports


/***/ }),

/***/ 23:
/***/ (function(module, exports, __webpack_require__) {

	// style-loader: Adds some css to the DOM by adding a <style> tag

	// load the styles
	var content = __webpack_require__(24);
	if(typeof content === 'string') content = [[module.id, content, '']];
	if(content.locals) module.exports = content.locals;
	// add the styles to the DOM
	var update = __webpack_require__(5)("b14ba53e", content, false);
	// Hot Module Replacement
	if(false) {
	 // When the styles change, update the <style> tags
	 if(!content.locals) {
	   module.hot.accept("!!../../../node_modules/css-loader/index.js!../../../node_modules/vue-loader/lib/style-compiler/index.js?{\"vue\":true,\"id\":\"data-v-f15788ee\",\"scoped\":true,\"hasInlineConfig\":false}!../../../node_modules/vue-loader/lib/selector.js?type=styles&index=0!./tip.vue", function() {
	     var newContent = require("!!../../../node_modules/css-loader/index.js!../../../node_modules/vue-loader/lib/style-compiler/index.js?{\"vue\":true,\"id\":\"data-v-f15788ee\",\"scoped\":true,\"hasInlineConfig\":false}!../../../node_modules/vue-loader/lib/selector.js?type=styles&index=0!./tip.vue");
	     if(typeof newContent === 'string') newContent = [[module.id, newContent, '']];
	     update(newContent);
	   });
	 }
	 // When the module is disposed, remove the <style> tags
	 module.hot.dispose(function() { update(); });
	}

/***/ }),

/***/ 24:
/***/ (function(module, exports, __webpack_require__) {

	exports = module.exports = __webpack_require__(4)();
	// imports


	// module
	exports.push([module.id, "\n.tip[data-v-f15788ee] {\n  padding-left: 36px;\n  padding-right: 36px;\n  padding-top: 36px;\n  padding-bottom: 36px;\n  border-radius: 10px;\n}\n.tip-txt[data-v-f15788ee]{\n  font-size: 28px;\n}\n.tip-success[data-v-f15788ee] {\n  background-color: #dff0d8;\n  border-color: #d6e9c6;\n}\n.tip-txt-success[data-v-f15788ee] {\n  color: #3c763d;\n}\n.tip-info[data-v-f15788ee] {\n  background-color: #d9edf7;\n  border-color: #bce8f1;\n}\n.tip-txt-info[data-v-f15788ee] {\n  color: #31708f;\n}\n.tip-warning[data-v-f15788ee] {\n  background-color: #fcf8e3;\n  border-color: #faebcc;\n}\n.tip-txt-warning[data-v-f15788ee] {\n  color: #8a6d3b;\n}\n.tip-danger[data-v-f15788ee] {\n  background-color: #f2dede;\n  border-color: #ebccd1;\n}\n.tip-txt-danger[data-v-f15788ee] {\n  color: #a94442;\n}\n", ""]);

	// exports


/***/ }),

/***/ 25:
/***/ (function(module, exports) {

	'use strict';

	//
	//
	//
	//
	//
	//
	//

	module.exports = {
	  props: {
	    type: { default: 'success' },
	    value: { default: '' }
	  }
	};

/***/ }),

/***/ 26:
/***/ (function(module, exports, __webpack_require__) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', {
	    class: ['tip', 'tip-' + _vm.type],
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined))
	  }, [_c('text', {
	    class: ['tip-txt', 'tip-txt-' + _vm.type],
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined))
	  }, [_vm._v(_vm._s(_vm.value))])])
	},staticRenderFns: []}
	module.exports.render._withStripped = true
	if (false) {
	  module.hot.accept()
	  if (module.hot.data) {
	     require("vue-hot-reload-api").rerender("data-v-f15788ee", module.exports)
	  }
	}

/***/ }),

/***/ 293:
/***/ (function(module, exports, __webpack_require__) {

	var disposed = false
	function injectStyle (ssrContext) {
	  if (disposed) return
	  __webpack_require__(294)
	}
	var Component = __webpack_require__(7)(
	  /* script */
	  __webpack_require__(296),
	  /* template */
	  __webpack_require__(302),
	  /* styles */
	  injectStyle,
	  /* scopeId */
	  "data-v-27caa8a6",
	  /* moduleIdentifier (server only) */
	  null
	)
	Component.options.__file = "/Users/Hanks/Codes/work/incubator-weex/examples/vue/style/style-box.vue"
	if (Component.esModule && Object.keys(Component.esModule).some(function (key) {return key !== "default" && key.substr(0, 2) !== "__"})) {console.error("named exports are not supported in *.vue files.")}
	if (Component.options.functional) {console.error("[vue-loader] style-box.vue: functional components are not supported with templates, they should use render functions.")}

	/* hot reload */
	if (false) {(function () {
	  var hotAPI = require("vue-hot-reload-api")
	  hotAPI.install(require("vue"), false)
	  if (!hotAPI.compatible) return
	  module.hot.accept()
	  if (!module.hot.data) {
	    hotAPI.createRecord("data-v-27caa8a6", Component.options)
	  } else {
	    hotAPI.reload("data-v-27caa8a6", Component.options)
	  }
	  module.hot.dispose(function (data) {
	    disposed = true
	  })
	})()}

	module.exports = Component.exports


/***/ }),

/***/ 294:
/***/ (function(module, exports, __webpack_require__) {

	// style-loader: Adds some css to the DOM by adding a <style> tag

	// load the styles
	var content = __webpack_require__(295);
	if(typeof content === 'string') content = [[module.id, content, '']];
	if(content.locals) module.exports = content.locals;
	// add the styles to the DOM
	var update = __webpack_require__(5)("33b9bd8b", content, false);
	// Hot Module Replacement
	if(false) {
	 // When the styles change, update the <style> tags
	 if(!content.locals) {
	   module.hot.accept("!!../../../node_modules/css-loader/index.js!../../../node_modules/vue-loader/lib/style-compiler/index.js?{\"vue\":true,\"id\":\"data-v-27caa8a6\",\"scoped\":true,\"hasInlineConfig\":false}!../../../node_modules/vue-loader/lib/selector.js?type=styles&index=0!./style-box.vue", function() {
	     var newContent = require("!!../../../node_modules/css-loader/index.js!../../../node_modules/vue-loader/lib/style-compiler/index.js?{\"vue\":true,\"id\":\"data-v-27caa8a6\",\"scoped\":true,\"hasInlineConfig\":false}!../../../node_modules/vue-loader/lib/selector.js?type=styles&index=0!./style-box.vue");
	     if(typeof newContent === 'string') newContent = [[module.id, newContent, '']];
	     update(newContent);
	   });
	 }
	 // When the module is disposed, remove the <style> tags
	 module.hot.dispose(function() { update(); });
	}

/***/ }),

/***/ 295:
/***/ (function(module, exports, __webpack_require__) {

	exports = module.exports = __webpack_require__(4)();
	// imports


	// module
	exports.push([module.id, "\n.box[data-v-27caa8a6] {\n  background-color: #f5f5f5;\n  width: 260px;\n  height: 260px;\n  padding-left: 40px;\n  padding-top: 40px;\n  padding-right: 40px;\n  padding-bottom: 40px;\n  margin-left: 40px;\n  margin-top: 40px;\n  margin-right: 40px;\n  margin-bottom: 40px;\n  border-width: 40px;\n  border-color: #333;\n  border-style: solid;\n}\n", ""]);

	// exports


/***/ }),

/***/ 296:
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

	module.exports = {
	  components: {
	    panel: __webpack_require__(9),
	    tip: __webpack_require__(22),
	    styleItem: __webpack_require__(297)
	  }
	};

/***/ }),

/***/ 297:
/***/ (function(module, exports, __webpack_require__) {

	var disposed = false
	function injectStyle (ssrContext) {
	  if (disposed) return
	  __webpack_require__(298)
	}
	var Component = __webpack_require__(7)(
	  /* script */
	  __webpack_require__(300),
	  /* template */
	  __webpack_require__(301),
	  /* styles */
	  injectStyle,
	  /* scopeId */
	  "data-v-d7c66270",
	  /* moduleIdentifier (server only) */
	  null
	)
	Component.options.__file = "/Users/Hanks/Codes/work/incubator-weex/examples/vue/style/style-item.vue"
	if (Component.esModule && Object.keys(Component.esModule).some(function (key) {return key !== "default" && key.substr(0, 2) !== "__"})) {console.error("named exports are not supported in *.vue files.")}
	if (Component.options.functional) {console.error("[vue-loader] style-item.vue: functional components are not supported with templates, they should use render functions.")}

	/* hot reload */
	if (false) {(function () {
	  var hotAPI = require("vue-hot-reload-api")
	  hotAPI.install(require("vue"), false)
	  if (!hotAPI.compatible) return
	  module.hot.accept()
	  if (!module.hot.data) {
	    hotAPI.createRecord("data-v-d7c66270", Component.options)
	  } else {
	    hotAPI.reload("data-v-d7c66270", Component.options)
	  }
	  module.hot.dispose(function (data) {
	    disposed = true
	  })
	})()}

	module.exports = Component.exports


/***/ }),

/***/ 298:
/***/ (function(module, exports, __webpack_require__) {

	// style-loader: Adds some css to the DOM by adding a <style> tag

	// load the styles
	var content = __webpack_require__(299);
	if(typeof content === 'string') content = [[module.id, content, '']];
	if(content.locals) module.exports = content.locals;
	// add the styles to the DOM
	var update = __webpack_require__(5)("189a7392", content, false);
	// Hot Module Replacement
	if(false) {
	 // When the styles change, update the <style> tags
	 if(!content.locals) {
	   module.hot.accept("!!../../../node_modules/css-loader/index.js!../../../node_modules/vue-loader/lib/style-compiler/index.js?{\"vue\":true,\"id\":\"data-v-d7c66270\",\"scoped\":true,\"hasInlineConfig\":false}!../../../node_modules/vue-loader/lib/selector.js?type=styles&index=0!./style-item.vue", function() {
	     var newContent = require("!!../../../node_modules/css-loader/index.js!../../../node_modules/vue-loader/lib/style-compiler/index.js?{\"vue\":true,\"id\":\"data-v-d7c66270\",\"scoped\":true,\"hasInlineConfig\":false}!../../../node_modules/vue-loader/lib/selector.js?type=styles&index=0!./style-item.vue");
	     if(typeof newContent === 'string') newContent = [[module.id, newContent, '']];
	     update(newContent);
	   });
	 }
	 // When the module is disposed, remove the <style> tags
	 module.hot.dispose(function() { update(); });
	}

/***/ }),

/***/ 299:
/***/ (function(module, exports, __webpack_require__) {

	exports = module.exports = __webpack_require__(4)();
	// imports


	// module
	exports.push([module.id, "\n.item[data-v-d7c66270] {\n  margin-right: 10px;\n  /*margin-bottom: 10px;*/\n  width: 160px;\n  height: 75px;\n  padding-left: 8px;\n  padding-right: 8px;\n  padding-top: 8px;\n  padding-bottom: 8px;\n}\n.txt[data-v-d7c66270] {\n  color: #eee;\n}\n", ""]);

	// exports


/***/ }),

/***/ 300:
/***/ (function(module, exports) {

	'use strict';

	//
	//
	//
	//
	//
	//
	//

	module.exports = {
	  props: {
	    value: { default: '' },
	    type: { default: '0' // 0, 1
	    } },
	  computed: {
	    bgColor: function bgColor() {
	      return this.type == '1' ? '#7BA3A8' : '#BEAD92';
	    }
	  }
	};

/***/ }),

/***/ 301:
/***/ (function(module, exports, __webpack_require__) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('text', {
	    staticClass: "item txt",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle({
	      backgroundColor: _vm.bgColor
	    })),
	    attrs: {
	      "value": _vm.value
	    }
	  })
	},staticRenderFns: []}
	module.exports.render._withStripped = true
	if (false) {
	  module.hot.accept()
	  if (module.hot.data) {
	     require("vue-hot-reload-api").rerender("data-v-d7c66270", module.exports)
	  }
	}

/***/ }),

/***/ 302:
/***/ (function(module, exports, __webpack_require__) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', [_c('panel', {
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "title": "Box Model",
	      "padding-body": "0",
	      "type": "primary"
	    }
	  }, [_c('text', {
	    staticClass: "box",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined))
	  }, [_vm._v("Box")])]), _vm._v(" "), _c('panel', {
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "title": "border",
	      "type": "primary"
	    }
	  }, [_c('panel', {
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "title": "border-width",
	      "type": ""
	    }
	  }, [_c('div', {
	    staticStyle: _vm.$processStyle({
	      "flex-direction": "row"
	    }),
	    style: (_vm.$processStyle(undefined))
	  }, [_c('style-item', {
	    staticStyle: _vm.$processStyle({
	      "border-style": "solid",
	      "border-width": "2px",
	      "border-color": "#333"
	    }),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "value": ""
	    }
	  }), _vm._v(" "), _c('style-item', {
	    staticStyle: _vm.$processStyle({
	      "border-style": "solid",
	      "border-width": "10px",
	      "border-color": "#333"
	    }),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "value": ""
	    }
	  }), _vm._v(" "), _c('style-item', {
	    staticStyle: _vm.$processStyle({
	      "border-style": "solid",
	      "border-left-width": "4px",
	      "border-color": "#333"
	    }),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "value": ""
	    }
	  }), _vm._v(" "), _c('style-item', {
	    staticStyle: _vm.$processStyle({
	      "border-style": "solid",
	      "border-bottom-width": "4px",
	      "border-color": "#333"
	    }),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "value": ""
	    }
	  })], 1)]), _vm._v(" "), _c('panel', {
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "title": "border-color",
	      "type": ""
	    }
	  }, [_c('div', {
	    staticStyle: _vm.$processStyle({
	      "flex-direction": "row"
	    }),
	    style: (_vm.$processStyle(undefined))
	  }, [_c('style-item', {
	    staticStyle: _vm.$processStyle({
	      "border-style": "solid",
	      "border-width": "4px",
	      "border-color": "#333"
	    }),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "value": ""
	    }
	  }), _vm._v(" "), _c('style-item', {
	    staticStyle: _vm.$processStyle({
	      "border-style": "solid",
	      "border-width": "4px",
	      "border-color": "#ddd"
	    }),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "value": ""
	    }
	  }), _vm._v(" "), _c('style-item', {
	    staticStyle: _vm.$processStyle({
	      "border-style": "solid",
	      "border-width": "4px",
	      "border-color": "red"
	    }),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "value": ""
	    }
	  })], 1)]), _vm._v(" "), _c('panel', {
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "title": "border-style",
	      "type": ""
	    }
	  }, [_c('tip', {
	    staticStyle: _vm.$processStyle({
	      "margin-bottom": "10px"
	    }),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "type": "warning",
	      "value": "just support four edges"
	    }
	  }), _vm._v(" "), _c('div', {
	    staticStyle: _vm.$processStyle({
	      "flex-direction": "row"
	    }),
	    style: (_vm.$processStyle(undefined))
	  }, [_c('style-item', {
	    staticStyle: _vm.$processStyle({
	      "border-style": "solid",
	      "border-width": "4px",
	      "border-color": "#333"
	    }),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "value": ""
	    }
	  }), _vm._v(" "), _c('style-item', {
	    staticStyle: _vm.$processStyle({
	      "border-style": "dashed",
	      "border-width": "4px",
	      "border-color": "#333"
	    }),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "value": "",
	      "type": ""
	    }
	  }), _vm._v(" "), _c('style-item', {
	    staticStyle: _vm.$processStyle({
	      "border-style": "dotted",
	      "border-width": "4px",
	      "border-color": "#333"
	    }),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "value": ""
	    }
	  })], 1)], 1), _vm._v(" "), _c('panel', {
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "title": "border-radius",
	      "type": ""
	    }
	  }, [_c('div', {
	    staticStyle: _vm.$processStyle({
	      "flex-direction": "row"
	    }),
	    style: (_vm.$processStyle(undefined))
	  }, [_c('style-item', {
	    staticStyle: _vm.$processStyle({
	      "width": "75px",
	      "border-radius": "38px"
	    }),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "value": ""
	    }
	  }), _vm._v(" "), _c('style-item', {
	    staticStyle: _vm.$processStyle({
	      "border-radius": "20px"
	    }),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "value": ""
	    }
	  }), _vm._v(" "), _c('style-item', {
	    staticStyle: _vm.$processStyle({
	      "border-top-left-radius": "20px",
	      "border-top-right-radius": "20px"
	    }),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "value": ""
	    }
	  }), _vm._v(" "), _c('style-item', {
	    staticStyle: _vm.$processStyle({
	      "border-bottom-left-radius": "20px",
	      "border-bottom-right-radius": "20px"
	    }),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "value": ""
	    }
	  })], 1)])], 1)], 1)
	},staticRenderFns: []}
	module.exports.render._withStripped = true
	if (false) {
	  module.hot.accept()
	  if (module.hot.data) {
	     require("vue-hot-reload-api").rerender("data-v-27caa8a6", module.exports)
	  }
	}

/***/ })

/******/ });