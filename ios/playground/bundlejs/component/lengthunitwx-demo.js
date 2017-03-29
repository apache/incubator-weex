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

	var __weex_template__ = __webpack_require__(97)
	var __weex_style__ = __webpack_require__(98)
	var __weex_script__ = __webpack_require__(99)

	__weex_define__('@weex-component/1cedab9e17186432a3ad57749557d3a0', [], function(__weex_require__, __weex_exports__, __weex_module__) {

	    __weex_script__(__weex_module__, __weex_exports__, __weex_require__)
	    if (__weex_exports__.__esModule && __weex_exports__.default) {
	      __weex_module__.exports = __weex_exports__.default
	    }

	    __weex_module__.exports.template = __weex_template__

	    __weex_module__.exports.style = __weex_style__

	})

	__weex_bootstrap__('@weex-component/1cedab9e17186432a3ad57749557d3a0',undefined,undefined)

/***/ },

/***/ 34:
/***/ function(module, exports) {

	var core = module.exports = {version: '2.4.0'};
	if(typeof __e == 'number')__e = core; // eslint-disable-line no-undef

/***/ },

/***/ 97:
/***/ function(module, exports) {

	module.exports = {
	  "type": "div",
	  "children": [
	    {
	      "type": "text",
	      "style": {
	        "fontSize": 20,
	        "backgroundColor": "#ff0000",
	        "marginTop": 10,
	        "width": 100
	      },
	      "attr": {
	        "value": "width:100"
	      }
	    },
	    {
	      "type": "text",
	      "style": {
	        "fontSize": 20,
	        "backgroundColor": "#ff0000",
	        "marginTop": 10,
	        "width": 200
	      },
	      "attr": {
	        "value": "width:200"
	      }
	    },
	    {
	      "type": "text",
	      "style": {
	        "fontSize": 20,
	        "backgroundColor": "#ff0000",
	        "marginTop": 10,
	        "width": 300
	      },
	      "attr": {
	        "value": "width:300"
	      }
	    },
	    {
	      "type": "text",
	      "style": {
	        "fontSize": 20,
	        "backgroundColor": "#ff0000",
	        "marginTop": 10,
	        "width": 400
	      },
	      "attr": {
	        "value": "width:400"
	      }
	    },
	    {
	      "type": "text",
	      "style": {
	        "fontSize": 20,
	        "backgroundColor": "#ff0000",
	        "marginTop": 10,
	        "width": 700
	      },
	      "attr": {
	        "value": "width:700"
	      }
	    },
	    {
	      "type": "text",
	      "style": {
	        "fontSize": 20,
	        "backgroundColor": "#ff0000",
	        "marginTop": 10,
	        "width": 750
	      },
	      "attr": {
	        "value": "width:750"
	      }
	    },
	    {
	      "type": "text",
	      "style": {
	        "fontSize": 20,
	        "backgroundColor": "#ff0000",
	        "marginTop": 10,
	        "width": "50wx"
	      },
	      "attr": {
	        "value": "width:50wx"
	      }
	    },
	    {
	      "type": "text",
	      "style": {
	        "fontSize": 20,
	        "backgroundColor": "#ff0000",
	        "marginTop": 10,
	        "width": "100wx"
	      },
	      "attr": {
	        "value": "width:100wx"
	      }
	    },
	    {
	      "type": "text",
	      "style": {
	        "fontSize": 20,
	        "backgroundColor": "#ff0000",
	        "marginTop": 10,
	        "width": "200wx"
	      },
	      "attr": {
	        "value": "width:200wx"
	      }
	    },
	    {
	      "type": "text",
	      "style": {
	        "fontSize": 20,
	        "backgroundColor": "#ff0000",
	        "marginTop": 10,
	        "width": "300wx"
	      },
	      "attr": {
	        "value": "width:300wx"
	      }
	    },
	    {
	      "type": "text",
	      "style": {
	        "fontSize": 20,
	        "backgroundColor": "#ff0000",
	        "marginTop": 10,
	        "width": "320wx"
	      },
	      "attr": {
	        "value": "width:320wx"
	      }
	    },
	    {
	      "type": "text",
	      "style": {
	        "fontSize": 20,
	        "backgroundColor": "#ff0000",
	        "marginTop": 10,
	        "width": "360wx"
	      },
	      "attr": {
	        "value": "width:360wx"
	      }
	    },
	    {
	      "type": "text",
	      "style": {
	        "fontSize": 20,
	        "backgroundColor": "#ff0000",
	        "marginTop": 10,
	        "width": "375wx"
	      },
	      "attr": {
	        "value": "width:375wx"
	      }
	    },
	    {
	      "type": "text",
	      "id": "theText",
	      "classList": [
	        "unitwx"
	      ],
	      "attr": {
	        "value": "hello world"
	      }
	    }
	  ]
	}

/***/ },

/***/ 98:
/***/ function(module, exports) {

	module.exports = {
	  "unitwx": {
	    "width": "200wx",
	    "height": "130wx",
	    "marginTop": 80,
	    "backgroundColor": "#00ff00",
	    "color": "#FFFFFF"
	  }
	}

/***/ },

/***/ 99:
/***/ function(module, exports, __webpack_require__) {

	module.exports = function(module, exports, __weex_require__){'use strict';

	var _stringify = __webpack_require__(100);

	var _stringify2 = _interopRequireDefault(_stringify);

	function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

	module.exports = {
	  data: function () {return {
	    theWidth: 100
	  }},
	  methods: {
	    ready: function ready() {
	      console.log('in ready ' + (0, _stringify2.default)(WXEnvironment));

	      var me = this;
	      setTimeout(function () {
	        me.theWidth = 200;
	      }, 2000);

	      setTimeout(function () {
	        me.theWidth = '200wx';
	      }, 4000);
	    },

	    setTo200: function setTo200() {
	      this.theWidth = '200';
	    },
	    setTo200wx: function setTo200wx() {
	      this.theWidth = '200wx';
	    }
	  }
	};}
	/* generated by weex-loader */


/***/ },

/***/ 100:
/***/ function(module, exports, __webpack_require__) {

	module.exports = { "default": __webpack_require__(101), __esModule: true };

/***/ },

/***/ 101:
/***/ function(module, exports, __webpack_require__) {

	var core  = __webpack_require__(34)
	  , $JSON = core.JSON || (core.JSON = {stringify: JSON.stringify});
	module.exports = function stringify(it){ // eslint-disable-line no-unused-vars
	  return $JSON.stringify.apply($JSON, arguments);
	};

/***/ }

/******/ });