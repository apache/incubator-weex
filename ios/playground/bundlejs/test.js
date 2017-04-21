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
/***/ function(module, exports, __webpack_require__) {

	var __weex_template__ = __webpack_require__(395)
	var __weex_script__ = __webpack_require__(396)

	__weex_define__('@weex-component/9305542c7d16b107fad92f56b2a08dab', [], function(__weex_require__, __weex_exports__, __weex_module__) {

	    __weex_script__(__weex_module__, __weex_exports__, __weex_require__)
	    if (__weex_exports__.__esModule && __weex_exports__.default) {
	      __weex_module__.exports = __weex_exports__.default
	    }

	    __weex_module__.exports.template = __weex_template__

	})

	__weex_bootstrap__('@weex-component/9305542c7d16b107fad92f56b2a08dab',undefined,undefined)

/***/ },

/***/ 395:
/***/ function(module, exports) {

	module.exports = {
	  "type": "scroller",
	  "children": [
	    {
	      "type": "div",
	      "id": "r",
	      "children": [
	        {
	          "type": "text",
	          "attr": {
	            "value": function () {return this.x}
	          }
	        },
	        {
	          "type": "div",
	          "style": {
	            "height": 2000,
	            "backgroundColor": "#ff0000"
	          }
	        },
	        {
	          "type": "text",
	          "events": {
	            "click": "foo"
	          },
	          "attr": {
	            "value": function () {return this.x}
	          }
	        }
	      ]
	    }
	  ]
	}

/***/ },

/***/ 396:
/***/ function(module, exports) {

	module.exports = function(module, exports, __weex_require__){'use strict';

	var dom = __weex_require__('@weex-module/dom');
	module.exports = {
	  data: function data() {
	    return {
	      x: 1
	    };
	  },
	  methods: {
	    foo: function foo(e) {
	      dom.scrollToElement(this.$el('r'), { offset: 0 });
	    }
	  }
	};}
	/* generated by weex-loader */


/***/ }

/******/ });