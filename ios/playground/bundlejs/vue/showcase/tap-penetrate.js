// { "framework": "Vue" }

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

	var __vue_exports__, __vue_options__
	var __vue_styles__ = []

	/* styles */
	__vue_styles__.push(__webpack_require__(200)
	)

	/* script */
	__vue_exports__ = __webpack_require__(201)

	/* template */
	var __vue_template__ = __webpack_require__(202)
	__vue_options__ = __vue_exports__ = __vue_exports__ || {}
	if (
	  typeof __vue_exports__.default === "object" ||
	  typeof __vue_exports__.default === "function"
	) {
	if (Object.keys(__vue_exports__).some(function (key) { return key !== "default" && key !== "__esModule" })) {console.error("named exports are not supported in *.vue files.")}
	__vue_options__ = __vue_exports__ = __vue_exports__.default
	}
	if (typeof __vue_options__ === "function") {
	  __vue_options__ = __vue_options__.options
	}
	__vue_options__.__file = "/Users/Hanks/Codes/work/incubator-weex/examples/vue/showcase/tap-penetrate.vue"
	__vue_options__.render = __vue_template__.render
	__vue_options__.staticRenderFns = __vue_template__.staticRenderFns
	__vue_options__._scopeId = "data-v-12117f38"
	__vue_options__.style = __vue_options__.style || {}
	__vue_styles__.forEach(function (module) {
	  for (var name in module) {
	    __vue_options__.style[name] = module[name]
	  }
	})
	if (typeof __register_static_styles__ === "function") {
	  __register_static_styles__(__vue_options__._scopeId, __vue_styles__)
	}

	module.exports = __vue_exports__
	module.exports.el = 'true'
	new Vue(module.exports)


/***/ }),

/***/ 200:
/***/ (function(module, exports) {

	module.exports = {
	  "items-box": {
	    "flexDirection": "row",
	    "justifyContent": "space-between",
	    "WebkitBoxOrient": "horizontal",
	    "WebkitBoxDirection": "normal",
	    "WebkitBoxPack": "justify"
	  },
	  "items": {
	    "flexFlow": "row nowrap",
	    "alignItems": "center",
	    "overflow": "hidden",
	    "position": "relative",
	    "top": 0,
	    "left": 0,
	    "boxSizing": "border-box",
	    "width": 374,
	    "height": 564,
	    "flexBasis": "auto",
	    "fontSize": 0,
	    "borderColor": "rgb(0,0,0)",
	    "borderStyle": "solid",
	    "backgroundColor": "rgb(245,245,245)",
	    "marginBottom": 2,
	    "textAlign": "center"
	  },
	  "flexRow": {
	    "flexDirection": "row"
	  },
	  "flexColumn": {
	    "flexDirection": "column"
	  },
	  "text": {
	    "flex": 1,
	    "textAlign": "center"
	  },
	  "fixed": {
	    "position": "fixed",
	    "bottom": 50,
	    "left": 50,
	    "width": 300,
	    "height": 50,
	    "alignItems": "center",
	    "justifyContent": "center"
	  },
	  "shopImg": {
	    "width": 220,
	    "height": 220
	  },
	  "list": {
	    "flexDirection": "column",
	    "overflow": "hidden",
	    "width": 750,
	    "height": 100,
	    "backgroundColor": "#dddddd"
	  },
	  "cell": {
	    "backgroundColor": "#dddddd",
	    "flexDirection": "column",
	    "width": 750
	  },
	  "test": {
	    "width": 60,
	    "height": 60
	  },
	  "shopDiv": {
	    "flexDirection": "column",
	    "backgroundColor": "#ffffff",
	    "marginTop": 5,
	    "marginRight": 5,
	    "marginBottom": 5,
	    "marginLeft": 5,
	    "paddingTop": 10,
	    "paddingRight": 10,
	    "paddingBottom": 10,
	    "paddingLeft": 10,
	    "borderWidth": 1,
	    "borderColor": "#cccccc"
	  },
	  "shopRowList": {
	    "flexDirection": "column",
	    "borderWidth": 1,
	    "borderColor": "#cccccc",
	    "marginTop": 5,
	    "marginRight": 5,
	    "marginBottom": 5,
	    "marginLeft": 5,
	    "paddingTop": 10,
	    "paddingRight": 10,
	    "paddingBottom": 10,
	    "paddingLeft": 10,
	    "backgroundColor": "#ffffff"
	  },
	  "shopHeader": {
	    "flexDirection": "row",
	    "width": 720
	  },
	  "shopFooter": {
	    "flexDirection": "row",
	    "width": 720
	  },
	  "smallImg": {
	    "width": 20,
	    "height": 20
	  },
	  "wrapper": {
	    "backgroundColor": "#f8f8f8",
	    "border": "1px solid #ddd",
	    "position": "fixed",
	    "bottom": 20,
	    "right": 20,
	    "width": 88,
	    "height": 88,
	    "zIndex": 10100,
	    "flexFlow": "row nowrap",
	    "alignItems": "center",
	    "cursor": "pointer"
	  },
	  "backTop": {
	    "width": 88,
	    "height": 88
	  }
	}

/***/ }),

/***/ 201:
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
	//
	//
	//
	//
	//
	//
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
	var dom = weex.requireModule('dom');
	module.exports = {
	  data: function data() {
	    return {
	      show: true,
	      href: 'http://weex.apache.org/cn/references/'
	    };
	  },
	  methods: {
	    clickitem: function clickitem(e) {
	      modal.toast({
	        message: 'click' + e
	      });
	    },
	    preventDefault: function preventDefault(e) {
	      this.href = 'javascript:void(0)';
	      modal.toast({
	        message: 'click'
	      });
	    },
	    backTop: function backTop() {
	      var self = this;
	      window.scrollTo(0, 0);
	      this.show = false;
	    }
	  }

	};

/***/ }),

/***/ 202:
/***/ (function(module, exports) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', [_c('div', {
	    staticClass: ["items-box"]
	  }, [_c('a', {
	    staticClass: ["items"],
	    attrs: {
	      "weexType": "a"
	    },
	    on: {
	      "click": function($event) {
	        _vm.clickitem('Tap1')
	      }
	    }
	  }, [_c('text', {
	    staticClass: ["text"]
	  }, [_vm._v("Tap1")])]), _c('a', {
	    staticClass: ["items"],
	    attrs: {
	      "weexType": "a"
	    },
	    on: {
	      "click": function($event) {
	        _vm.clickitem('Tap2')
	      }
	    }
	  }, [_c('text', {
	    staticClass: ["text"]
	  }, [_vm._v("Tap2")])])]), _c('div', {
	    staticClass: ["items-box"]
	  }, [_c('a', {
	    staticClass: ["items"],
	    attrs: {
	      "weexType": "a",
	      "href": "http://weex.apache.org/cn/references/",
	      "target": "_blank"
	    },
	    on: {
	      "click": function($event) {
	        _vm.clickitem('Tap in the a tag')
	      }
	    }
	  }, [_c('text', {
	    staticClass: ["text"]
	  }, [_vm._v("Tap in the A tag")])]), _c('a', {
	    staticClass: ["items"],
	    attrs: {
	      "weexType": "a",
	      "href": _vm.href,
	      "target": "_blank"
	    },
	    on: {
	      "click": _vm.preventDefault
	    }
	  }, [_c('text', {
	    staticClass: ["text"]
	  }, [_vm._v("Prevent A tag from href jumping")])])]), _vm._m(0), _vm._m(1), _vm._m(2), (_vm.show) ? _c('div', {
	    staticClass: ["wrapper"],
	    on: {
	      "click": _vm.backTop
	    }
	  }, [_c('text', {
	    staticClass: ["text"]
	  }, [_vm._v("Tap")])]) : _vm._e()])
	},staticRenderFns: [function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', {
	    staticClass: ["items-box"]
	  }, [_c('a', {
	    staticClass: ["items"],
	    attrs: {
	      "weexType": "a",
	      "href": "http://weex.apache.org/cn/references/",
	      "target": "_blank"
	    }
	  }, [_c('text', {
	    staticClass: ["text"]
	  }, [_vm._v("Url")])]), _c('a', {
	    staticClass: ["items"],
	    attrs: {
	      "weexType": "a",
	      "href": "http://weex.apache.org/cn/references/",
	      "target": "_blank"
	    }
	  }, [_c('text', {
	    staticClass: ["text"]
	  }, [_vm._v("Url")])])])
	},function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', {
	    staticClass: ["items-box"]
	  }, [_c('a', {
	    staticClass: ["items"],
	    attrs: {
	      "weexType": "a",
	      "href": "http://weex.apache.org/cn/references/",
	      "target": "_blank"
	    }
	  }, [_c('text', {
	    staticClass: ["text"]
	  }, [_vm._v("Url")])]), _c('a', {
	    staticClass: ["items"],
	    attrs: {
	      "weexType": "a",
	      "href": "http://weex.apache.org/cn/references/",
	      "target": "_blank"
	    }
	  }, [_c('text', {
	    staticClass: ["text"]
	  }, [_vm._v("Url")])])])
	},function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', {
	    staticClass: ["items-box"]
	  }, [_c('a', {
	    staticClass: ["items"],
	    attrs: {
	      "weexType": "a",
	      "href": "http://weex.apache.org/cn/references/",
	      "target": "_blank"
	    }
	  }, [_c('text', {
	    staticClass: ["text"]
	  }, [_vm._v("Url")])]), _c('a', {
	    staticClass: ["items"],
	    attrs: {
	      "weexType": "a",
	      "href": "http://weex.apache.org/cn/references/",
	      "target": "_blank"
	    }
	  }, [_c('text', {
	    staticClass: ["text"]
	  }, [_vm._v("Url")])])])
	}]}
	module.exports.render._withStripped = true

/***/ })

/******/ });