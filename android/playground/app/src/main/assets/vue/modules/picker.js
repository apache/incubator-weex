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
	__vue_styles__.push(__webpack_require__(126)
	)

	/* script */
	__vue_exports__ = __webpack_require__(127)

	/* template */
	var __vue_template__ = __webpack_require__(128)
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
	__vue_options__.__file = "/Users/Hanks/Codes/work/incubator-weex/examples/vue/modules/picker.vue"
	__vue_options__.render = __vue_template__.render
	__vue_options__.staticRenderFns = __vue_template__.staticRenderFns
	__vue_options__._scopeId = "data-v-4373d0c4"
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

/***/ 126:
/***/ (function(module, exports) {

	module.exports = {
	  "container": {
	    "flex": 1
	  }
	}

/***/ }),

/***/ 127:
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

	var picker = weex.requireModule('picker');
	module.exports = {
	  data: {
	    normalpickData: '',
	    pickDateData: '',
	    pickTimeData: ''
	  },
	  methods: {
	    normalpick: function normalpick() {
	      var self = this;
	      picker.pick({
	        height: '300px',
	        items: [1, 2, 3, 4]
	      }, function (ret) {
	        var result = ret.result;
	        if (result == 'success') {
	          self.normalpickData = ret.data;
	        }
	      });
	    },
	    pickDate: function pickDate(e) {
	      var self = this;
	      picker.pickDate({
	        'value': '2016-12-20',
	        'max': '2016-12-30',
	        'min': '2014-12-30'
	      }, function (ret) {
	        var result = ret.result;
	        if (result == 'success') {
	          self.pickDateData = ret.data;
	        }
	      });
	    },
	    pickTime: function pickTime() {
	      var self = this;
	      picker.pickTime({}, function (ret) {
	        var result = ret.result;
	        if (result == 'success') {
	          self.pickTimeData = ret.data;
	        }
	      });
	    }
	  }
	};

/***/ }),

/***/ 128:
/***/ (function(module, exports) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', {
	    staticClass: ["conatiner"]
	  }, [_vm._m(0), _c('div', {
	    staticStyle: {
	      margin: "20px",
	      padding: "20px",
	      backgroundColor: "#1ba1e2",
	      color: "#fff"
	    },
	    on: {
	      "click": _vm.normalpick
	    }
	  }, [_c('text', {
	    staticStyle: {
	      color: "#fff"
	    }
	  }, [_vm._v("normalpick" + _vm._s(_vm.normalpickData))])]), _c('div', {
	    staticStyle: {
	      margin: "20px",
	      padding: "20px",
	      backgroundColor: "#1ba1e2",
	      color: "#fff"
	    },
	    on: {
	      "click": _vm.pickDate
	    }
	  }, [_c('text', {
	    staticStyle: {
	      color: "#fff"
	    }
	  }, [_vm._v("pickDate" + _vm._s(_vm.pickDateData))])]), _c('div', {
	    staticStyle: {
	      margin: "20px",
	      padding: "20px",
	      backgroundColor: "#1ba1e2",
	      color: "#fff"
	    },
	    on: {
	      "click": _vm.pickTime
	    }
	  }, [_c('text', {
	    staticStyle: {
	      color: "#fff"
	    }
	  }, [_vm._v("pickTime" + _vm._s(_vm.pickTimeData))])])])
	},staticRenderFns: [function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', {
	    staticStyle: {
	      backgroundColor: "#286090"
	    }
	  }, [_c('text', {
	    staticClass: ["title"],
	    staticStyle: {
	      height: "80px",
	      padding: "20px",
	      color: "white"
	    }
	  }, [_vm._v("picker")])])
	}]}
	module.exports.render._withStripped = true

/***/ })

/******/ });