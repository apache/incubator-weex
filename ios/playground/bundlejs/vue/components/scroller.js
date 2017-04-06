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
/***/ function(module, exports, __webpack_require__) {

	var __vue_exports__, __vue_options__
	var __vue_styles__ = []

	/* styles */
	__vue_styles__.push(__webpack_require__(447)
	)

	/* script */
	__vue_exports__ = __webpack_require__(448)

	/* template */
	var __vue_template__ = __webpack_require__(449)
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
	__vue_options__.__file = "/Users/bobning/work/source/apache-incubator-weex/examples/vue/components/scroller.vue"
	__vue_options__.render = __vue_template__.render
	__vue_options__.staticRenderFns = __vue_template__.staticRenderFns
	__vue_options__._scopeId = "data-v-bc3ac90a"
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


/***/ },

/***/ 447:
/***/ function(module, exports) {

	module.exports = {
	  "refresh-view": {
	    "height": 120,
	    "width": 750,
	    "display": "flex",
	    "MsFlexAlign": "center",
	    "WebkitAlignItems": "center",
	    "WebkitBoxAlign": "center",
	    "alignItems": "center"
	  },
	  "refresh-arrow": {
	    "fontSize": 30,
	    "color": "#45b5f0"
	  },
	  "loading-view": {
	    "height": 80,
	    "width": 750,
	    "justifyContent": "center",
	    "alignItems": "center"
	  },
	  "indicator": {
	    "height": 40,
	    "width": 40,
	    "color": "#45b5f0"
	  },
	  "header": {
	    "backgroundColor": "#45b5f0",
	    "padding": 20,
	    "height": 88,
	    "justifyContent": "center"
	  },
	  "header-title": {
	    "color": "#FFFFFF",
	    "fontWeight": "bold"
	  },
	  "item": {
	    "justifyContent": "center",
	    "borderBottomWidth": 2,
	    "borderBottomColor": "#c0c0c0",
	    "height": 100,
	    "padding": 20
	  }
	}

/***/ },

/***/ 448:
/***/ function(module, exports) {

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
	    onrefresh: function onrefresh(e) {
	      var self = this;
	      self.refresh_display = 'show';
	      setTimeout(function () {
	        self.refresh_display = 'hide';
	      }, 1000);
	    },
	    onloading: function onloading(e) {
	      var self = this;
	      self.loading_display = 'show';
	      setTimeout(function () {
	        self.loading_display = 'hide';
	      }, 1000);
	    }
	  },
	  data: function data() {
	    return {
	      refresh_display: 'hide',
	      loading_display: 'hide',
	      sections: [{
	        title: 'Header 1',
	        items: [{ id: 1 }, { id: 2 }, { id: 3 }, { id: 4 }, { id: 5 }]
	      }, {
	        title: 'Header 2',
	        items: [{ id: 6 }, { id: 7 }, { id: 8 }, { id: 9 }, { id: 10 }, { id: 11 }]
	      }],
	      moreSections: [{
	        title: 'Header 3',
	        items: [{ id: 12 }, { id: 13 }, { id: 14 }, { id: 15 }, { id: 16 }, { id: 17 }, { id: 18 }]
	      }, {
	        title: 'Header 4',
	        items: [{ id: 19 }, { id: 20 }, { id: 21 }, { id: 22 }]
	      }, {
	        title: 'Header 5',
	        items: [{ id: 23 }, { id: 24 }, { id: 25 }, { id: 26 }, { id: 27 }]
	      }, {
	        title: 'Header 6',
	        items: [{ id: 28 }, { id: 29 }, { id: 30 }, { id: 31 }, { id: 32 }]
	      }]
	    };
	  }
	};

/***/ },

/***/ 449:
/***/ function(module, exports) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('scroller', {
	    staticClass: ["list"],
	    appendAsTree: true,
	    attrs: {
	      "append": "tree"
	    }
	  }, [_vm._l((_vm.sections), function(sec) {
	    return _c('div', {
	      staticClass: ["section"]
	    }, [_c('div', {
	      staticClass: ["header"]
	    }, [_c('text', {
	      staticClass: ["header-title"]
	    }, [_vm._v(_vm._s(sec.title))])]), _vm._l((sec.items), function(item) {
	      return _c('div', {
	        staticClass: ["item"]
	      }, [_c('text', {
	        staticClass: ["item-title"]
	      }, [_vm._v("row " + _vm._s(item.id))])])
	    })], 2)
	  }), _c('loading', {
	    staticClass: ["loading-view"],
	    attrs: {
	      "display": _vm.loading_display,
	      "onloading": "onloading"
	    }
	  }, [_c('loading-indicator', {
	    staticStyle: {
	      height: "60",
	      width: "60"
	    }
	  })], 1)], 2)
	},staticRenderFns: []}
	module.exports.render._withStripped = true

/***/ }

/******/ });