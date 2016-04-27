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

	;__weex_define__("@weex-component/8e45800ef96f056e806dbc3423e9def7", [], function(__weex_require__, __weex_exports__, __weex_module__){

	;
	  __weex_module__.exports = {
	    data: function() {
	      var items = [];
	      var N = 5;
	      if (N > 10) N = 10;
	      for (var i = 0; i < N; i++) {
	        for (var j = 0; j < N; j++) {
	          for (var k = 0; k < N; k++) {
	            items.push({i: i, j: j, k: k});
	          }
	        }
	      }

	      return {
	        items: items,
	        size: 256
	      }
	    }
	  }
	  
	//  https://www.gravatar.com/avatar/5dab88d418972368dcd1d9032b1c3320?s=128&d=identicon&r=PG
	//  https://www.gravatar.com/avatar/5dab88d418972368dcd1d9032b1c3330?s=128&d=identicon&r=PG

	;__weex_module__.exports.template={
	  "type": "scroller",
	  "children": [
	    {
	      "type": "image",
	      "attr": {
	        "width": function () {return this.size},
	        "height": function () {return this.size},
	        "src": function () {return 'https://www.gravatar.com/avatar/5dab88d418972368dcd1d9032b1c3' + (this.i) + (this.j) + (this.k) + '?s=' + (this.size/2) + '&d=identicon&r=PG'}
	      },
	      "style": {
	        "marginBottom": 10
	      },
	      "repeat": function () {return this.items}
	    }
	  ]
	}
	})
	;__weex_bootstrap__("@weex-component/8e45800ef96f056e806dbc3423e9def7", {"transformerVersion":"0.3.1"}, undefined)

/***/ }
/******/ ]);