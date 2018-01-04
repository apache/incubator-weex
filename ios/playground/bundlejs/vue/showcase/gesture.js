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
	__vue_styles__.push(__webpack_require__(148)
	)

	/* script */
	__vue_exports__ = __webpack_require__(149)

	/* template */
	var __vue_template__ = __webpack_require__(150)
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
	__vue_options__.__file = "/Users/Hanks/Codes/work/incubator-weex/examples/vue/showcase/gesture.vue"
	__vue_options__.render = __vue_template__.render
	__vue_options__.staticRenderFns = __vue_template__.staticRenderFns
	__vue_options__._scopeId = "data-v-54c2a822"
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

/***/ 148:
/***/ (function(module, exports) {

	module.exports = {
	  "ct": {
	    "position": "absolute",
	    "width": 750,
	    "height": 1000,
	    "backgroundColor": "#f7f7f7"
	  }
	}

/***/ }),

/***/ 149:
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

	module.exports = {
	  data: function data() {
	    return {
	      text: 'no press.',
	      top: '0px',
	      left: '0px',
	      flag: false
	    };
	  },
	  methods: {
	    handlePress: function handlePress(e) {
	      this.text = '->longpress(' + e.touch.pageX + ',' + e.touch.pageY + ')id:' + e.touch.identifier;
	    },
	    handleSwipe: function handleSwipe(e) {
	      this.text += '->swipe(' + e.touch.pageX + ',' + e.touch.pageY + ')id:' + e.touch.identifier;
	    },
	    handlePanStart: function handlePanStart(e) {
	      this.flag = false;
	      this.text = '->panstart(' + e.touch.pageX + ',' + e.touch.pageY + ')id:' + e.touch.identifier;
	    },
	    handlePanMove: function handlePanMove(e) {
	      if (this.flag) {
	        return;
	      }
	      this.flag = true;
	      this.text += '->panmove(' + e.touch.pageX + ',' + e.touch.pageY + ')id:' + e.touch.identifier;
	    },
	    handlePanEnd: function handlePanEnd(e) {
	      this.text += '->panend(' + e.touch.pageX + ',' + e.touch.pageY + ')id:' + e.touch.identifier;
	    }
	  }
	};

/***/ }),

/***/ 150:
/***/ (function(module, exports) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', [_c('div', {
	    staticClass: ["ct"],
	    style: {
	      top: _vm.top,
	      left: _vm.left
	    },
	    on: {
	      "longpress": _vm.handlePress,
	      "swipe": _vm.handleSwipe,
	      "panstart": _vm.handlePanStart,
	      "panmove": _vm.handlePanMove,
	      "panend": _vm.handlePanEnd
	    }
	  }, [_c('text', [_vm._v(_vm._s(_vm.text))])])])
	},staticRenderFns: []}
	module.exports.render._withStripped = true

/***/ })

/******/ });