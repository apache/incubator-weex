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

	var App = __webpack_require__(1)
	App.el = '#root'
	new Vue(App)


/***/ }),
/* 1 */
/***/ (function(module, exports, __webpack_require__) {

	var disposed = false
	function injectStyle (ssrContext) {
	  if (disposed) return
	  __webpack_require__(2)
	}
	var Component = __webpack_require__(7)(
	  /* script */
	  __webpack_require__(8),
	  /* template */
	  __webpack_require__(19),
	  /* styles */
	  injectStyle,
	  /* scopeId */
	  "data-v-48e47b99",
	  /* moduleIdentifier (server only) */
	  null
	)
	Component.options.__file = "/Users/Hanks/Codes/work/incubator-weex/examples/vue/animation.vue"
	if (Component.esModule && Object.keys(Component.esModule).some(function (key) {return key !== "default" && key.substr(0, 2) !== "__"})) {console.error("named exports are not supported in *.vue files.")}
	if (Component.options.functional) {console.error("[vue-loader] animation.vue: functional components are not supported with templates, they should use render functions.")}

	/* hot reload */
	if (false) {(function () {
	  var hotAPI = require("vue-hot-reload-api")
	  hotAPI.install(require("vue"), false)
	  if (!hotAPI.compatible) return
	  module.hot.accept()
	  if (!module.hot.data) {
	    hotAPI.createRecord("data-v-48e47b99", Component.options)
	  } else {
	    hotAPI.reload("data-v-48e47b99", Component.options)
	  }
	  module.hot.dispose(function (data) {
	    disposed = true
	  })
	})()}

	module.exports = Component.exports


/***/ }),
/* 2 */
/***/ (function(module, exports, __webpack_require__) {

	// style-loader: Adds some css to the DOM by adding a <style> tag

	// load the styles
	var content = __webpack_require__(3);
	if(typeof content === 'string') content = [[module.id, content, '']];
	if(content.locals) module.exports = content.locals;
	// add the styles to the DOM
	var update = __webpack_require__(5)("471a3152", content, false);
	// Hot Module Replacement
	if(false) {
	 // When the styles change, update the <style> tags
	 if(!content.locals) {
	   module.hot.accept("!!../../node_modules/css-loader/index.js!../../node_modules/vue-loader/lib/style-compiler/index.js?{\"vue\":true,\"id\":\"data-v-48e47b99\",\"scoped\":true,\"hasInlineConfig\":false}!../../node_modules/vue-loader/lib/selector.js?type=styles&index=0!./animation.vue", function() {
	     var newContent = require("!!../../node_modules/css-loader/index.js!../../node_modules/vue-loader/lib/style-compiler/index.js?{\"vue\":true,\"id\":\"data-v-48e47b99\",\"scoped\":true,\"hasInlineConfig\":false}!../../node_modules/vue-loader/lib/selector.js?type=styles&index=0!./animation.vue");
	     if(typeof newContent === 'string') newContent = [[module.id, newContent, '']];
	     update(newContent);
	   });
	 }
	 // When the module is disposed, remove the <style> tags
	 module.hot.dispose(function() { update(); });
	}

/***/ }),
/* 3 */
/***/ (function(module, exports, __webpack_require__) {

	exports = module.exports = __webpack_require__(4)();
	// imports


	// module
	exports.push([module.id, "\n.block[data-v-48e47b99] {\n  position: absolute;\n  width: 250px;\n  height: 250px;\n  top: 300px;\n  left: 400px;\n  background-color: #F0AD4E;\n  align-items: center;\n  justify-content: center;\n}\n.block-txt[data-v-48e47b99] {\n  color: #FFFFFF;\n  font-size: 70px;\n}\n", ""]);

	// exports


/***/ }),
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
/* 8 */
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

	var animation = weex.requireModule('animation');
	module.exports = {
	  data: function data() {
	    return {
	      transformOrigin: 'center center',
	      current_rotate: 0,
	      current_scale: 1,
	      current_color: '#FF0000',
	      current_opacity: 1,
	      current_translate: '',
	      current_transform: '',
	      isStop: true
	    };
	  },
	  components: {
	    panel: __webpack_require__(9),
	    button: __webpack_require__(14)
	  },
	  methods: {
	    anim: function anim(styles, timingFunction, duration, callback) {
	      animation.transition(this.$refs.block, {
	        styles: styles,
	        timingFunction: timingFunction,
	        duration: duration
	      }, callback);
	    },
	    rotate: function rotate() {
	      var self = this;
	      self.current_rotate += 90;
	      self.anim({
	        transform: 'rotate(' + self.current_rotate + 'deg)'
	      }, 'ease-in-out', 500, function () {
	        if (self.current_rotate === 360) {
	          self.current_rotate = 0;
	        } else {
	          self.rotate();
	        }
	      });
	    },
	    translate: function translate() {
	      this.current_translate = this.current_translate ? '' : 'translate(50%, 50%)';
	      this.anim({
	        transform: this.current_translate
	      }, 'ease-in', 500, function () {});
	    },
	    scale: function scale() {
	      var self = this;
	      self.current_scale = self.current_scale === 2 ? .5 : 2;
	      self.anim({
	        transform: 'scale(' + self.current_scale + ')'
	      }, 'linear', 500, function () {});
	    },
	    transform: function transform() {
	      var self = this;
	      this.current_transform = this.current_transform ? '' : 'rotate(45deg) scale(1.5)';
	      this.anim({
	        transform: this.current_transform,
	        transformOrigin: 'left top'
	      }, 'ease-out', 500, function () {
	        if (self.current_transform !== '') {
	          self.anim({
	            transform: 'rotate(-90deg) scale(1.2)',
	            transformOrigin: 'left top'
	          }, 'ease-out', 500, function () {});
	        } else {}
	      });
	    },
	    composite: function composite() {
	      var self = this;
	      self.current_transform = self.current_transform ? '' : 'rotate(45deg) scale(1.5) translate(50%, 50%)';
	      self.current_color = self.current_color === '#F0AD4E' ? '#D9534F' : '#F0AD4E';
	      self.current_opacity = self.current_opacity === 1 ? 0.1 : 1;
	      this.anim({
	        transform: this.current_transform,
	        transformOrigin: 'left top',
	        backgroundColor: self.current_color,
	        opacity: self.current_opacity
	      }, 'ease-out', 1000, function () {});
	    },
	    color: function color() {
	      var self = this;
	      self.current_color = self.current_color === '#F0AD4E' ? '#D9534F' : '#F0AD4E';
	      self.anim({
	        backgroundColor: self.current_color
	      }, 'linear', 500, function () {});
	    },
	    opacity: function opacity() {
	      var self = this;
	      self.current_opacity = self.current_opacity === 1 ? 0.1 : 1;
	      self.anim({
	        opacity: self.current_opacity
	      }, 'linear', 500, function () {});
	    }
	  }
	};

/***/ }),
/* 9 */
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
/* 10 */
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
/* 11 */
/***/ (function(module, exports, __webpack_require__) {

	exports = module.exports = __webpack_require__(4)();
	// imports


	// module
	exports.push([module.id, "\n.panel[data-v-66798af2] {\n  margin-bottom: 20px;\n  background-color: #fff;\n  /*border: 1px solid transparent;*/\n  /*border-radius: 10px;*/\n  /*-webkit-box-shadow: 0 1px 1px rgba(0,0,0,.05);*/\n  /*box-shadow: 0 1px 1px rgba(0,0,0,.05);*/\n  border-color: #dddddd;\n  border-width: 1px;\n}\n.panel-default[data-v-66798af2] {\n}\n.panel-primary[data-v-66798af2] {\n  border-color: rgb(40, 96, 144);\n}\n.panel-success[data-v-66798af2] {\n  border-color: rgb(76, 174, 76);\n}\n.panel-info[data-v-66798af2] {\n  border-color: rgb(70, 184, 218);\n}\n.panel-warning[data-v-66798af2] {\n  border-color: rgb(238, 162, 54);\n}\n.panel-danger[data-v-66798af2] {\n  border-color: rgb(212, 63, 58);\n}\n.panel-header[data-v-66798af2] {\n  background-color: #f5f5f5;\n  font-size: 40px;\n  /*padding-left: 12px;*/\n  /*padding-right: 12px;*/\n  /*padding-top: 20px;*/\n  /*padding-bottom: 20px;*/\n  color: #333;\n}\n.panel-header-default[data-v-66798af2] {\n}\n.panel-header-primary[data-v-66798af2] {\n  background-color: rgb(40, 96, 144);\n  color: #ffffff;\n}\n.panel-header-success[data-v-66798af2] {\n  background-color: rgb(92, 184, 92);\n  color: #ffffff;\n}\n.panel-header-info[data-v-66798af2] {\n  background-color: rgb(91, 192, 222);\n  color: #ffffff;\n}\n.panel-header-warning[data-v-66798af2] {\n  background-color: rgb(240, 173, 78);\n  color: #ffffff;\n}\n.panel-header-danger[data-v-66798af2] {\n  background-color: rgb(217, 83, 79);\n  color: #ffffff;\n}\n.panel-body[data-v-66798af2] {\n  padding-left: 12px;\n  padding-right: 12px;\n  padding-top: 20px;\n  padding-bottom: 20px;\n}\n.panel-body-default[data-v-66798af2] {\n}\n.panel-body-primary[data-v-66798af2] {\n}\n.panel-body-success[data-v-66798af2] {\n}\n.panel-body-info[data-v-66798af2] {\n}\n.panel-body-warning[data-v-66798af2] {\n}\n.panel-body-danger[data-v-66798af2] {\n}\n", ""]);

	// exports


/***/ }),
/* 12 */
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
/* 13 */
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
/* 14 */
/***/ (function(module, exports, __webpack_require__) {

	var disposed = false
	function injectStyle (ssrContext) {
	  if (disposed) return
	  __webpack_require__(15)
	}
	var Component = __webpack_require__(7)(
	  /* script */
	  __webpack_require__(17),
	  /* template */
	  __webpack_require__(18),
	  /* styles */
	  injectStyle,
	  /* scopeId */
	  "data-v-33548b34",
	  /* moduleIdentifier (server only) */
	  null
	)
	Component.options.__file = "/Users/Hanks/Codes/work/incubator-weex/examples/vue/include/button.vue"
	if (Component.esModule && Object.keys(Component.esModule).some(function (key) {return key !== "default" && key.substr(0, 2) !== "__"})) {console.error("named exports are not supported in *.vue files.")}
	if (Component.options.functional) {console.error("[vue-loader] button.vue: functional components are not supported with templates, they should use render functions.")}

	/* hot reload */
	if (false) {(function () {
	  var hotAPI = require("vue-hot-reload-api")
	  hotAPI.install(require("vue"), false)
	  if (!hotAPI.compatible) return
	  module.hot.accept()
	  if (!module.hot.data) {
	    hotAPI.createRecord("data-v-33548b34", Component.options)
	  } else {
	    hotAPI.reload("data-v-33548b34", Component.options)
	  }
	  module.hot.dispose(function (data) {
	    disposed = true
	  })
	})()}

	module.exports = Component.exports


/***/ }),
/* 15 */
/***/ (function(module, exports, __webpack_require__) {

	// style-loader: Adds some css to the DOM by adding a <style> tag

	// load the styles
	var content = __webpack_require__(16);
	if(typeof content === 'string') content = [[module.id, content, '']];
	if(content.locals) module.exports = content.locals;
	// add the styles to the DOM
	var update = __webpack_require__(5)("3204e9c3", content, false);
	// Hot Module Replacement
	if(false) {
	 // When the styles change, update the <style> tags
	 if(!content.locals) {
	   module.hot.accept("!!../../../node_modules/css-loader/index.js!../../../node_modules/vue-loader/lib/style-compiler/index.js?{\"vue\":true,\"id\":\"data-v-33548b34\",\"scoped\":true,\"hasInlineConfig\":false}!../../../node_modules/vue-loader/lib/selector.js?type=styles&index=0!./button.vue", function() {
	     var newContent = require("!!../../../node_modules/css-loader/index.js!../../../node_modules/vue-loader/lib/style-compiler/index.js?{\"vue\":true,\"id\":\"data-v-33548b34\",\"scoped\":true,\"hasInlineConfig\":false}!../../../node_modules/vue-loader/lib/selector.js?type=styles&index=0!./button.vue");
	     if(typeof newContent === 'string') newContent = [[module.id, newContent, '']];
	     update(newContent);
	   });
	 }
	 // When the module is disposed, remove the <style> tags
	 module.hot.dispose(function() { update(); });
	}

/***/ }),
/* 16 */
/***/ (function(module, exports, __webpack_require__) {

	exports = module.exports = __webpack_require__(4)();
	// imports


	// module
	exports.push([module.id, "\n.btn[data-v-33548b34] {\n  margin-bottom: 0;\n  align-items: center;\n  justify-content: center;\n  border-width: 1px;\n  border-style: solid;\n  border-color: #333;\n\n  /*white-space: nowrap;*/\n  /*vertical-align: middle;*/\n  /*touch-action: manipulation;*/\n  /*cursor: pointer;*/\n  /*-webkit-user-select: none;*/\n  /*background-image: none;*/\n  /*border-image-source: initial;*/\n  /*border-image-slice: initial;*/\n  /*border-image-width: initial;*/\n  /*border-image-outset: initial;*/\n  /*border-image-repeat: initial;*/\n}\n.btn-txt[data-v-33548b34] {\n}\n\n/**TYPE**/\n.btn-default[data-v-33548b34] {\n  color: rgb(51, 51, 51);\n}\n.btn-primary[data-v-33548b34] {\n  background-color: rgb(40, 96, 144);\n  border-color: rgb(40, 96, 144);\n}\n.btn-success[data-v-33548b34] {\n  background-color: rgb(92, 184, 92);\n  border-color: rgb(76, 174, 76);\n}\n.btn-info[data-v-33548b34] {\n  background-color: rgb(91, 192, 222);\n  border-color: rgb(70, 184, 218);\n}\n.btn-warning[data-v-33548b34] {\n  background-color: rgb(240, 173, 78);\n  border-color: rgb(238, 162, 54);\n}\n.btn-danger[data-v-33548b34] {\n  background-color: rgb(217, 83, 79);\n  border-color: rgb(212, 63, 58);\n}\n.btn-link[data-v-33548b34] {\n  border-color: transparent;\n  border-radius: 0;\n}\n.btn-txt-default[data-v-33548b34] {\n  color: rgb(51, 51, 51);\n}\n.btn-txt-primary[data-v-33548b34] {\n  color: rgb(255, 255, 255);\n}\n.btn-txt-success[data-v-33548b34] {\n  color: rgb(255, 255, 255);\n}\n.btn-txt-info[data-v-33548b34] {\n  color: rgb(255, 255, 255);\n}\n.btn-txt-warning[data-v-33548b34] {\n  color: rgb(255, 255, 255);\n}\n.btn-txt-danger[data-v-33548b34] {\n  color: rgb(255, 255, 255);\n}\n.btn-txt-link[data-v-33548b34] {\n  color: rgb(51, 122, 183);\n  /*font-weight: 400;*/\n}\n\n/**SIZE**/\n.btn-sz-large[data-v-33548b34] {\n  width: 300px;\n  height: 100px;\n  padding-top: 25px;\n  padding-bottom: 25px;\n  padding-left: 40px;\n  padding-right: 40px;\n  /*line-height: 1.33333;*/\n  border-radius: 15px;\n}\n.btn-sz-middle[data-v-33548b34] {\n  width: 240px;\n  height: 80px;\n  padding-top: 15px;\n  padding-bottom: 15px;\n  padding-left: 30px;\n  padding-right: 30px;\n  /*line-height: 1.42857;*/\n  border-radius: 10px;\n}\n.btn-sz-small[data-v-33548b34] {\n  width: 170px;\n  height: 60px;\n  padding-top: 12px;\n  padding-bottom: 12px;\n  padding-left: 25px;\n  padding-right: 25px;\n  /*line-height: 1.5;*/\n  border-radius: 7px;\n}\n.btn-txt-sz-large[data-v-33548b34] {\n  font-size: 45px;\n}\n.btn-txt-sz-middle[data-v-33548b34] {\n  font-size: 35px;\n}\n.btn-txt-sz-small[data-v-33548b34] {\n  font-size: 30px;\n}\n\n/*DISABLED*/\n.disabled[data-v-33548b34] {\n}\n\n", ""]);

	// exports


/***/ }),
/* 17 */
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
	    type: { default: 'default' },
	    size: { default: 'large' },
	    value: { default: '' }
	  }
	};

/***/ }),
/* 18 */
/***/ (function(module, exports, __webpack_require__) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', {
	    class: ['btn', 'btn-' + _vm.type, 'btn-sz-' + _vm.size],
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined))
	  }, [_c('text', {
	    class: ['btn-txt', 'btn-txt-' + _vm.type, 'btn-txt-sz-' + _vm.size],
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined))
	  }, [_vm._v(_vm._s(_vm.value))])])
	},staticRenderFns: []}
	module.exports.render._withStripped = true
	if (false) {
	  module.hot.accept()
	  if (module.hot.data) {
	     require("vue-hot-reload-api").rerender("data-v-33548b34", module.exports)
	  }
	}

/***/ }),
/* 19 */
/***/ (function(module, exports, __webpack_require__) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', [_c('panel', {
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "title": "Transform",
	      "type": "primary"
	    }
	  }, [_c('button', {
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "value": "Rotate",
	      "type": "primary",
	      "size": "middle"
	    },
	    nativeOn: {
	      "click": function($event) {
	        _vm.rotate($event)
	      }
	    }
	  }), _vm._v(" "), _c('button', {
	    staticStyle: _vm.$processStyle({
	      "margin-top": "12px"
	    }),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "value": "Scale",
	      "type": "primary",
	      "size": "middle"
	    },
	    nativeOn: {
	      "click": function($event) {
	        _vm.scale($event)
	      }
	    }
	  }), _vm._v(" "), _c('button', {
	    staticStyle: _vm.$processStyle({
	      "margin-top": "12px"
	    }),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "value": "Translate",
	      "type": "primary",
	      "size": "middle"
	    },
	    nativeOn: {
	      "click": function($event) {
	        _vm.translate($event)
	      }
	    }
	  }), _vm._v(" "), _c('button', {
	    staticStyle: _vm.$processStyle({
	      "margin-top": "12px"
	    }),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "value": "Transform",
	      "type": "success",
	      "size": "middle"
	    },
	    nativeOn: {
	      "click": function($event) {
	        _vm.transform($event)
	      }
	    }
	  })]), _vm._v(" "), _c('panel', {
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "title": "Others",
	      "type": "primary"
	    }
	  }, [_c('button', {
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "value": "BgColor",
	      "type": "primary",
	      "size": "middle"
	    },
	    nativeOn: {
	      "click": function($event) {
	        _vm.color($event)
	      }
	    }
	  }), _vm._v(" "), _c('button', {
	    staticStyle: _vm.$processStyle({
	      "margin-top": "12px"
	    }),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "value": "Opacity",
	      "type": "primary",
	      "size": "middle"
	    },
	    nativeOn: {
	      "click": function($event) {
	        _vm.opacity($event)
	      }
	    }
	  }), _vm._v(" "), _c('button', {
	    staticStyle: _vm.$processStyle({
	      "margin-top": "12px"
	    }),
	    style: (_vm.$processStyle(undefined)),
	    attrs: {
	      "value": "All",
	      "type": "success",
	      "size": "middle"
	    },
	    nativeOn: {
	      "click": function($event) {
	        _vm.composite($event)
	      }
	    }
	  })]), _vm._v(" "), _c('div', {
	    ref: "block",
	    staticClass: "block",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle({
	      transformOrigin: _vm.transformOrigin
	    }))
	  }, [_c('text', {
	    staticClass: "block-txt",
	    staticStyle: _vm.$processStyle(undefined),
	    style: (_vm.$processStyle(undefined))
	  }, [_vm._v("Anim")])])], 1)
	},staticRenderFns: []}
	module.exports.render._withStripped = true
	if (false) {
	  module.hot.accept()
	  if (module.hot.data) {
	     require("vue-hot-reload-api").rerender("data-v-48e47b99", module.exports)
	  }
	}

/***/ })
/******/ ]);