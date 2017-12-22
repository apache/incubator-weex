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
	__vue_styles__.push(__webpack_require__(31)
	)

	/* script */
	__vue_exports__ = __webpack_require__(32)

	/* template */
	var __vue_template__ = __webpack_require__(33)
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
	__vue_options__.__file = "/Users/Hanks/Codes/work/incubator-weex/examples/vue/components/list.vue"
	__vue_options__.render = __vue_template__.render
	__vue_options__.staticRenderFns = __vue_template__.staticRenderFns
	__vue_options__._scopeId = "data-v-2953851f"
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

/***/ 31:
/***/ (function(module, exports) {

	module.exports = {
	  "list": {
	    "height": 850
	  },
	  "count": {
	    "fontSize": 48,
	    "marginTop": 10,
	    "marginRight": 10,
	    "marginBottom": 10,
	    "marginLeft": 10
	  },
	  "indicator": {
	    "height": 40,
	    "width": 40,
	    "color": "#45b5f0"
	  },
	  "row": {
	    "width": 750
	  },
	  "item": {
	    "justifyContent": "center",
	    "borderBottomWidth": 2,
	    "borderBottomColor": "#c0c0c0",
	    "height": 100,
	    "paddingTop": 20,
	    "paddingRight": 20,
	    "paddingBottom": 20,
	    "paddingLeft": 20,
	    "backgroundColor:active": "#00BDFF"
	  }
	}

/***/ }),

/***/ 32:
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
	  methods: {
	    onappear: function onappear(idx, e) {
	      var appearId = this.rows[idx].id;
	      console.log('+++++', appearId);
	      var appearIds = this.appearIds;
	      appearIds.push(appearId);
	      this.getMinAndMaxIds(appearIds);
	    },
	    ondisappear: function ondisappear(idx, e) {
	      var disAppearId = this.rows[idx].id;
	      console.log('+++++', disAppearId);
	      var appearIds = this.appearIds;
	      var index = appearIds.indexOf(disAppearId);
	      if (index > -1) {
	        appearIds.splice(index, 1);
	      }
	      this.getMinAndMaxIds(appearIds);
	    },
	    getMinAndMaxIds: function getMinAndMaxIds(appearIds) {
	      appearIds.sort(function (a, b) {
	        return a - b;
	      });
	      this.appearIds = appearIds;
	      this.appearMax = appearIds[appearIds.length - 1];
	      this.appearMin = appearIds[0];
	    }
	  },
	  data: function data() {
	    return {
	      appearMin: 1,
	      appearMax: 1,
	      appearIds: [],
	      rows: [{ id: 1 }, { id: 2 }, { id: 3 }, { id: 4 }, { id: 5 }, { id: 6 }, { id: 7 }, { id: 8 }, { id: 9 }, { id: 10 }, { id: 11 }, { id: 12 }, { id: 13 }, { id: 14 }, { id: 15 }, { id: 16 }, { id: 17 }, { id: 18 }, { id: 19 }, { id: 20 }, { id: 21 }, { id: 22 }, { id: 23 }, { id: 24 }, { id: 25 }, { id: 26 }, { id: 27 }, { id: 28 }, { id: 29 }]
	    };
	  }
	};

/***/ }),

/***/ 33:
/***/ (function(module, exports) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', [_c('list', {
	    staticClass: ["list"]
	  }, _vm._l((_vm.rows), function(v, i) {
	    return _c('cell', {
	      key: i,
	      staticClass: ["row"],
	      appendAsTree: true,
	      attrs: {
	        "append": "tree",
	        "index": i
	      },
	      on: {
	        "appear": function($event) {
	          _vm.onappear(i, $event)
	        },
	        "disappear": function($event) {
	          _vm.ondisappear(i, $event)
	        }
	      }
	    }, [_c('div', {
	      staticClass: ["item"]
	    }, [_c('text', {
	      staticClass: ["item-title"]
	    }, [_vm._v("row " + _vm._s(v.id))])])])
	  })), _c('text', {
	    staticClass: ["count"],
	    attrs: {
	      "value": 'Appear items: ' + _vm.appearMin + ' ~ ' + _vm.appearMax
	    }
	  })])
	},staticRenderFns: []}
	module.exports.render._withStripped = true

/***/ })

/******/ });