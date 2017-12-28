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
	__vue_styles__.push(__webpack_require__(139)
	)

	/* script */
	__vue_exports__ = __webpack_require__(140)

	/* template */
	var __vue_template__ = __webpack_require__(141)
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
	__vue_options__.__file = "/Users/Hanks/Codes/work/incubator-weex/examples/vue/showcase/boxshadow.vue"
	__vue_options__.render = __vue_template__.render
	__vue_options__.staticRenderFns = __vue_template__.staticRenderFns
	__vue_options__._scopeId = "data-v-4db814d1"
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

/***/ 139:
/***/ (function(module, exports) {

	module.exports = {
	  "wrapper": {
	    "alignItems": "center",
	    "marginTop": 120
	  },
	  "title": {
	    "fontSize": 48
	  }
	}

/***/ }),

/***/ 140:
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

	module.exports = {
	    data: function data() {
	        return {
	            logoUrl: 'https://alibaba.github.io/weex/img/weex_logo_blue@3x.png',
	            target: 'World'
	        };
	    }
	};

/***/ }),

/***/ 141:
/***/ (function(module, exports) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', {
	    staticClass: ["wrapper"]
	  }, [_c('div', {
	    staticStyle: {
	      width: "400px",
	      height: "60px",
	      backgroundColor: "#FFE4C4",
	      boxShadow: "20px  10px rgb(255, 69, 0)"
	    }
	  }, [_c('text', {
	    staticClass: ["title"],
	    staticStyle: {
	      textAlign: "center"
	    }
	  }, [_vm._v("Hello " + _vm._s(_vm.target))])]), _c('div', {
	    staticStyle: {
	      marginTop: "80px",
	      width: "400px",
	      height: "60px",
	      backgroundColor: "#FFE4C4",
	      boxShadow: "20px  10px 5px rgba(255, 69, 0, 0.8)"
	    }
	  }, [_c('text', {
	    staticClass: ["title"],
	    staticStyle: {
	      textAlign: "center"
	    }
	  }, [_vm._v("Hello " + _vm._s(_vm.target))])]), _c('div', {
	    staticStyle: {
	      marginTop: "80px",
	      width: "400px",
	      height: "60px",
	      backgroundColor: "#FFE4C4",
	      boxShadow: "inset 20px  10px 5px rgba(255, 69, 0, 0.8)"
	    }
	  }, [_c('text', {
	    staticClass: ["title"],
	    staticStyle: {
	      textAlign: "center"
	    }
	  }, [_vm._v("Hello " + _vm._s(_vm.target))])]), _c('div', {
	    staticStyle: {
	      marginTop: "80px",
	      width: "400px",
	      height: "60px",
	      backgroundColor: "#FFE4C4",
	      boxShadow: "inset 20px  10px 5px rgb(255, 69, 0)"
	    }
	  }, [_c('text', {
	    staticClass: ["title"],
	    staticStyle: {
	      textAlign: "center"
	    }
	  }, [_vm._v("Hello " + _vm._s(_vm.target))])]), _c('div', {
	    staticStyle: {
	      marginTop: "80px",
	      width: "400px",
	      height: "60px",
	      backgroundColor: "#FFE4C4",
	      boxShadow: "20px  10px 5px black"
	    }
	  }, [_c('text', {
	    staticClass: ["title"],
	    staticStyle: {
	      textAlign: "center"
	    }
	  }, [_vm._v("Hello " + _vm._s(_vm.target))])]), _c('div', {
	    staticStyle: {
	      marginTop: "80px",
	      width: "400px",
	      height: "60px",
	      backgroundColor: "#FFE4C4",
	      boxShadow: "20px  10px 5px #008B00"
	    }
	  }, [_c('text', {
	    staticClass: ["title"],
	    staticStyle: {
	      textAlign: "center"
	    }
	  }, [_vm._v("Hello " + _vm._s(_vm.target))])])])
	},staticRenderFns: []}
	module.exports.render._withStripped = true

/***/ })

/******/ });