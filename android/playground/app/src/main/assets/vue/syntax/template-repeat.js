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
	__vue_styles__.push(__webpack_require__(269)
	)

	/* script */
	__vue_exports__ = __webpack_require__(270)

	/* template */
	var __vue_template__ = __webpack_require__(271)
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
	__vue_options__.__file = "/Users/Hanks/Codes/work/incubator-weex/examples/vue/syntax/template-repeat.vue"
	__vue_options__.render = __vue_template__.render
	__vue_options__.staticRenderFns = __vue_template__.staticRenderFns
	__vue_options__._scopeId = "data-v-0cf7b4ac"
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

/***/ 269:
/***/ (function(module, exports) {

	module.exports = {
	  "title": {
	    "fontSize": 48
	  },
	  "subtitle": {
	    "fontSize": 36
	  }
	}

/***/ }),

/***/ 270:
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

	module.exports = {
	  data: function data() {
	    return {
	      list: ['a', 'b', 'c', 'd', 'e'],
	      list2: [{ text: 'a' }, { text: 'b' }, { text: 'c' }, { text: 'd' }, { text: 'e' }]
	    };
	  }
	};

/***/ }),

/***/ 271:
/***/ (function(module, exports) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', [_c('text', {
	    staticClass: ["title"]
	  }, [_vm._v("Custom item")]), _vm._l((_vm.list), function(item) {
	    return _c('text', {
	      key: item.index,
	      staticClass: ["subtitle"]
	    }, [_vm._v(_vm._s(item))])
	  }), _c('text', {
	    staticClass: ["title"]
	  }, [_vm._v("Custom key and item")]), _vm._l((_vm.list), function(v, i) {
	    return _c('text', {
	      key: i,
	      staticClass: ["subtitle"]
	    }, [_vm._v(_vm._s(i) + "-" + _vm._s(v))])
	  }), _c('text', {
	    staticClass: ["title"]
	  }, [_vm._v("Array of Object")]), _vm._l((_vm.list2), function(item, k, index) {
	    return _c('text', {
	      key: index,
	      staticClass: ["subtitle"]
	    }, [_vm._v("> " + _vm._s(index) + "-" + _vm._s(item.text))])
	  })], 2)
	},staticRenderFns: []}
	module.exports.render._withStripped = true

/***/ })

/******/ });