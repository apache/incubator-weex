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
/***/ function(module, exports) {

	;__weex_define__("@weex-component/de562f46a417f3110ded06161e4a46db", [], function(__weex_require__, __weex_exports__, __weex_module__){

	;
	  __weex_module__.exports = {
	    methods: {
	      clicked: function() {
	        this.$call('modal', 'toast', {'message': 'clicked!'});
	      }  
	    }
	  }

	;__weex_module__.exports.template={
	  "type": "container",
	  "children": [
	    {
	      "type": "foo",
	      "events": {
	        "click": "clicked"
	      }
	    },
	    {
	      "type": "bar",
	      "attr": {
	        "click": function () {return this.clicked}
	      }
	    }
	  ]
	}
	})
	;__weex_bootstrap__("@weex-component/de562f46a417f3110ded06161e4a46db", {"transformerVersion":"0.3.1"}, undefined)

/***/ }
/******/ ]);