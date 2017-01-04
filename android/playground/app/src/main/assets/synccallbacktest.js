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

	;__weex_define__("@weex-component/c66559a2adb9a85f22776631b2443d82", [], function(__weex_require__, __weex_exports__, __weex_module__){

	;
	 var syncTest = __weex_require__('@weex-module/syncTest')
	  // console.log('syncTest:', syncTest)


	  __weex_module__.exports = {
	    data: function () {return {
	      string: '',
	      number: 0,
	      array: ['none'],
	      object: { empty: true }
	    }},
	    created: function () {
	      this.string = syncTest.getString()
	      this.number = syncTest.getNumber()
	      this.array = syncTest.getArray()
	    this.object = JSON.stringify(syncTest.getObject())
	    },
	    methods: {
	    onclickSyncTest: function(){
	       this.$el("synctest").testSyncCall();
	       this.$el("synctest").testAsyncCall();
	    }

	    }
	  }

	;__weex_module__.exports.template = __weex_module__.exports.template || {}
	;Object.assign(__weex_module__.exports.template, {
	  "type": "div",
	  "children": [
	    {
	      "type": "synccomponent",
	      "classList": [
	        "group"
	      ],
	      "id": "synctest",
	      "events": {
	        "click": "onclickSyncTest"
	      },
	      "children": [
	        {
	          "type": "text",
	          "classList": [
	            "title"
	          ],
	          "attr": {
	            "value": "synccomponent"
	          }
	        },
	        {
	          "type": "text",
	          "classList": [
	            "result"
	          ],
	          "attr": {
	            "value": function () {return this.number}
	          }
	        }
	      ]
	    },
	    {
	      "type": "div",
	      "classList": [
	        "group"
	      ],
	      "children": [
	        {
	          "type": "text",
	          "classList": [
	            "title"
	          ],
	          "attr": {
	            "value": "getString():"
	          }
	        },
	        {
	          "type": "text",
	          "classList": [
	            "result"
	          ],
	          "attr": {
	            "value": function () {return this.string}
	          }
	        }
	      ]
	    },
	    {
	      "type": "div",
	      "classList": [
	        "group"
	      ],
	      "children": [
	        {
	          "type": "text",
	          "classList": [
	            "title"
	          ],
	          "attr": {
	            "value": "getArray():"
	          }
	        },
	        {
	          "type": "text",
	          "classList": [
	            "result"
	          ],
	          "repeat": {
	            "expression": function () {return this.array},
	            "value": "str"
	          },
	          "attr": {
	            "value": function () {return (this.$index) + '. ' + (this.str)}
	          }
	        }
	      ]
	    },
	    {
	      "type": "div",
	      "classList": [
	        "group"
	      ],
	      "children": [
	        {
	          "type": "text",
	          "classList": [
	            "title"
	          ],
	          "attr": {
	            "value": "getObject():"
	          }
	        },
	        {
	          "type": "text",
	          "classList": [
	            "result"
	          ],
	          "attr": {
	            "value": function () {return this.object}
	          }
	        }
	      ]
	    }
	  ]
	})
	;__weex_module__.exports.style = __weex_module__.exports.style || {}
	;Object.assign(__weex_module__.exports.style, {
	  "group": {
	    "paddingTop": 20,
	    "paddingBottom": 20,
	    "paddingLeft": 40,
	    "paddingRight": 40,
	    "borderBottomWidth": 2,
	    "borderBottomStyle": "solid",
	    "borderBottomColor": "#DDDDDD"
	  },
	  "title": {
	    "color": "#666666",
	    "fontSize": 50
	  },
	  "result": {
	    "paddingLeft": 20,
	    "fontSize": 40
	  }
	})
	})
	;__weex_bootstrap__("@weex-component/c66559a2adb9a85f22776631b2443d82", {
	  "transformerVersion": "0.3.1"
	},undefined)

/***/ }
/******/ ]);