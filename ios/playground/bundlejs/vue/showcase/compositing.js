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
	__vue_styles__.push(__webpack_require__(145)
	)

	/* script */
	__vue_exports__ = __webpack_require__(146)

	/* template */
	var __vue_template__ = __webpack_require__(147)
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
	__vue_options__.__file = "/Users/Hanks/Codes/work/incubator-weex/examples/vue/showcase/compositing.vue"
	__vue_options__.render = __vue_template__.render
	__vue_options__.staticRenderFns = __vue_template__.staticRenderFns
	__vue_options__._scopeId = "data-v-76fdcbaa"
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

/***/ 145:
/***/ (function(module, exports) {

	module.exports = {
	  "wrapper": {
	    "alignItems": "center",
	    "opacity": 0.8
	  },
	  "itemWrapper": {
	    "flex": 1,
	    "borderTopRightRadius": 80,
	    "borderBottomRightRadius": 80,
	    "borderTopWidth": 10,
	    "borderTopColor": "#FF0000",
	    "borderStyle": "dashed",
	    "alignItems": "center",
	    "backgroundColor": "#FFFFFF"
	  },
	  "nameWrapper": {
	    "backgroundColor": "rgba(255,0,0,0.5)",
	    "borderRadius": 30,
	    "borderColor": "#000000",
	    "borderWidth": 10
	  },
	  "item": {
	    "marginTop": 120,
	    "backgroundColor": "#CCCCCC",
	    "alignItems": "center"
	  },
	  "itemName": {
	    "fontSize": 28,
	    "color": "#333333",
	    "lineHeight": 42,
	    "textAlign": "left",
	    "marginTop": 24
	  },
	  "itemPhoto": {
	    "marginTop": 18,
	    "width": 220,
	    "height": 220,
	    "marginBottom": 18,
	    "borderRadius": 30,
	    "overflow": "hidden",
	    "backgroundColor": "#0000FF"
	  },
	  "itemDesc": {
	    "fontSize": 24,
	    "marginTop": 12,
	    "marginRight": 12,
	    "marginBottom": 12,
	    "marginLeft": 12,
	    "color": "#999999",
	    "lineHeight": 36,
	    "textAlign": "left"
	  }
	}

/***/ }),

/***/ 146:
/***/ (function(module, exports) {

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

	module.exports = {
	  data: {},
	  methods: {}
	};

/***/ }),

/***/ 147:
/***/ (function(module, exports) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', {
	    staticClass: ["wrapper"],
	    on: {
	      "click": _vm.update
	    }
	  }, [_vm._m(0), _vm._m(1)])
	},staticRenderFns: [function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', {
	    staticClass: ["item"],
	    attrs: {
	      "compositing": "true"
	    }
	  }, [_c('div', {
	    staticClass: ["itemWrapper"]
	  }, [_c('div', {
	    staticClass: ["nameWrapper"]
	  }, [_c('text', {
	    staticClass: ["itemName"]
	  }, [_vm._v("Thomas Carlyle")])]), _c('image', {
	    staticClass: ["itemPhoto"],
	    attrs: {
	      "src": "https://gw.alicdn.com/tps/TB1Jl1CPFXXXXcJXXXXXXXXXXXX-370-370.jpg"
	    }
	  }), _c('div', {
	    staticClass: ["descWrapper"]
	  }, [_c('text', {
	    staticClass: ["itemDesc"]
	  }, [_vm._v("Genius only means hard-working all one\\'s life")])])])])
	},function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', {
	    staticClass: ["item"],
	    attrs: {
	      "compositing": "false"
	    }
	  }, [_c('div', {
	    staticClass: ["itemWrapper"]
	  }, [_c('div', {
	    staticClass: ["nameWrapper"]
	  }, [_c('text', {
	    staticClass: ["itemName"]
	  }, [_vm._v("Thomas Carlyle")])]), _c('image', {
	    staticClass: ["itemPhoto"],
	    attrs: {
	      "src": "https://gw.alicdn.com/tps/TB1Jl1CPFXXXXcJXXXXXXXXXXXX-370-370.jpg"
	    }
	  }), _c('div', {
	    staticClass: ["descWrapper"]
	  }, [_c('text', {
	    staticClass: ["itemDesc"]
	  }, [_vm._v("Genius only means hard-working all one\\'s life")])])])])
	}]}
	module.exports.render._withStripped = true

/***/ })

/******/ });