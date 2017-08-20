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

	var __weex_template__ = __webpack_require__(244)
	var __weex_style__ = __webpack_require__(245)
	var __weex_script__ = __webpack_require__(246)

	__weex_define__('@weex-component/2b448ff7e01d43a38e2ee57f8aa1d6de', [], function(__weex_require__, __weex_exports__, __weex_module__) {

	    __weex_script__(__weex_module__, __weex_exports__, __weex_require__)
	    if (__weex_exports__.__esModule && __weex_exports__.default) {
	      __weex_module__.exports = __weex_exports__.default
	    }

	    __weex_module__.exports.template = __weex_template__

	    __weex_module__.exports.style = __weex_style__

	})

	__weex_bootstrap__('@weex-component/2b448ff7e01d43a38e2ee57f8aa1d6de',undefined,undefined)

/***/ },

/***/ 244:
/***/ function(module, exports) {

	module.exports = {
	  "type": "div",
	  "classList": [
	    "wrapper"
	  ],
	  "events": {
	    "click": "update"
	  },
	  "children": [
	    {
	      "type": "div",
	      "style": {
	        "height": 60,
	        "backgroundColor": "#FFE4C4",
	        "boxShadow": function () {return this.bs},
	        "width": function () {return this.w}
	      },
	      "children": [
	        {
	          "type": "text",
	          "classList": [
	            "title"
	          ],
	          "style": {
	            "textAlign": "center"
	          },
	          "attr": {
	            "value": function () {return 'Hello ' + (this.target)}
	          }
	        }
	      ]
	    },
	    {
	      "type": "div",
	      "style": {
	        "marginTop": 80,
	        "width": 400,
	        "height": 60,
	        "backgroundColor": "#FFE4C4",
	        "boxShadow": "20px  10px 5px rgba(255, 69, 0, 0.8)"
	      },
	      "children": [
	        {
	          "type": "text",
	          "classList": [
	            "title"
	          ],
	          "style": {
	            "textAlign": "center"
	          },
	          "attr": {
	            "value": function () {return 'Hello ' + (this.target)}
	          }
	        }
	      ]
	    },
	    {
	      "type": "div",
	      "style": {
	        "marginTop": 80,
	        "width": 400,
	        "height": 60,
	        "backgroundColor": "#FFE4C4",
	        "boxShadow": "inset 20px  10px 5px rgba(255, 69, 0, 0.8)"
	      },
	      "children": [
	        {
	          "type": "text",
	          "classList": [
	            "title"
	          ],
	          "style": {
	            "textAlign": "center"
	          },
	          "attr": {
	            "value": function () {return 'Hello ' + (this.target)}
	          }
	        }
	      ]
	    },
	    {
	      "type": "div",
	      "style": {
	        "marginTop": 80,
	        "width": 400,
	        "height": 60,
	        "backgroundColor": "#FFE4C4",
	        "boxShadow": "inset 20px  10px 5px rgb(255, 69, 0)"
	      },
	      "children": [
	        {
	          "type": "text",
	          "classList": [
	            "title"
	          ],
	          "style": {
	            "textAlign": "center"
	          },
	          "attr": {
	            "value": function () {return 'Hello ' + (this.target)}
	          }
	        }
	      ]
	    },
	    {
	      "type": "div",
	      "style": {
	        "marginTop": 80,
	        "width": 400,
	        "height": 60,
	        "backgroundColor": "#FFE4C4",
	        "boxShadow": "20px  10px 5px black"
	      },
	      "children": [
	        {
	          "type": "text",
	          "classList": [
	            "title"
	          ],
	          "style": {
	            "textAlign": "center"
	          },
	          "attr": {
	            "value": function () {return 'Hello ' + (this.target)}
	          }
	        }
	      ]
	    },
	    {
	      "type": "div",
	      "style": {
	        "marginTop": 80,
	        "width": 400,
	        "height": 60,
	        "backgroundColor": "#FFE4C4",
	        "boxShadow": "20px  10px 5px #008B00"
	      },
	      "children": [
	        {
	          "type": "text",
	          "classList": [
	            "title"
	          ],
	          "style": {
	            "textAlign": "center"
	          },
	          "attr": {
	            "value": function () {return 'Hello ' + (this.target)}
	          }
	        }
	      ]
	    }
	  ]
	}

/***/ },

/***/ 245:
/***/ function(module, exports) {

	module.exports = {
	  "wrapper": {
	    "alignItems": "center",
	    "marginTop": 120
	  },
	  "title": {
	    "fontSize": 48
	  }
	}

/***/ },

/***/ 246:
/***/ function(module, exports) {

	module.exports = function(module, exports, __weex_require__){'use strict';

	module.exports = {
	    data: function () {return {
	        logoUrl: 'https://alibaba.github.io/weex/img/weex_logo_blue@3x.png',
	        target: 'World',
	        w: '400',
	        bs: '20px  10px  5px rgb(255, 69, 0)'
	    }},
	    methods: {
	        update: function update(e) {
	            this.target = 'Weex';
	            this.w = 600;
	            this.bs = 'inset 20px  10px 5px rgba(0, 0, 0, 0.5)';
	        }
	    }
	};}
	/* generated by weex-loader */


/***/ }

/******/ });