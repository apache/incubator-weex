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
	__vue_styles__.push(__webpack_require__(78)
	)

	/* script */
	__vue_exports__ = __webpack_require__(79)

	/* template */
	var __vue_template__ = __webpack_require__(80)
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
	__vue_options__.__file = "/Users/Hanks/Codes/work/incubator-weex/examples/vue/components/text.vue"
	__vue_options__.render = __vue_template__.render
	__vue_options__.staticRenderFns = __vue_template__.staticRenderFns
	__vue_options__._scopeId = "data-v-c5678ba4"
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

/***/ 3:
/***/ (function(module, exports, __webpack_require__) {

	var __vue_exports__, __vue_options__
	var __vue_styles__ = []

	/* styles */
	__vue_styles__.push(__webpack_require__(4)
	)

	/* script */
	__vue_exports__ = __webpack_require__(5)

	/* template */
	var __vue_template__ = __webpack_require__(6)
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
	__vue_options__.__file = "/Users/Hanks/Codes/work/incubator-weex/examples/vue/include/panel.vue"
	__vue_options__.render = __vue_template__.render
	__vue_options__.staticRenderFns = __vue_template__.staticRenderFns
	__vue_options__._scopeId = "data-v-58511ff6"
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


/***/ }),

/***/ 4:
/***/ (function(module, exports) {

	module.exports = {
	  "panel": {
	    "marginBottom": 20,
	    "backgroundColor": "#ffffff",
	    "borderColor": "#dddddd",
	    "borderWidth": 1
	  },
	  "panel-primary": {
	    "borderColor": "rgb(40,96,144)"
	  },
	  "panel-success": {
	    "borderColor": "rgb(76,174,76)"
	  },
	  "panel-info": {
	    "borderColor": "rgb(70,184,218)"
	  },
	  "panel-warning": {
	    "borderColor": "rgb(238,162,54)"
	  },
	  "panel-danger": {
	    "borderColor": "rgb(212,63,58)"
	  },
	  "panel-header": {
	    "backgroundColor": "#f5f5f5",
	    "fontSize": 40,
	    "color": "#333333"
	  },
	  "panel-header-primary": {
	    "backgroundColor": "rgb(40,96,144)",
	    "color": "#ffffff"
	  },
	  "panel-header-success": {
	    "backgroundColor": "rgb(92,184,92)",
	    "color": "#ffffff"
	  },
	  "panel-header-info": {
	    "backgroundColor": "rgb(91,192,222)",
	    "color": "#ffffff"
	  },
	  "panel-header-warning": {
	    "backgroundColor": "rgb(240,173,78)",
	    "color": "#ffffff"
	  },
	  "panel-header-danger": {
	    "backgroundColor": "rgb(217,83,79)",
	    "color": "#ffffff"
	  },
	  "panel-body": {
	    "paddingLeft": 12,
	    "paddingRight": 12,
	    "paddingTop": 20,
	    "paddingBottom": 20
	  }
	}

/***/ }),

/***/ 5:
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

	module.exports = {
	  props: {
	    type: { default: 'default' },
	    title: { default: '' },
	    paddingBody: { default: 20 },
	    paddingHead: { default: 20 },
	    dataClass: { default: '' }, // FIXME transfer class
	    border: { default: 0 }
	  }
	};

/***/ }),

/***/ 6:
/***/ (function(module, exports) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', {
	    class: ['panel', 'panel-' + _vm.type],
	    style: {
	      borderWidth: _vm.border
	    }
	  }, [_c('text', {
	    class: ['panel-header', 'panel-header-' + _vm.type],
	    style: {
	      paddingTop: _vm.paddingHead,
	      paddingBottom: _vm.paddingHead,
	      paddingLeft: _vm.paddingHead * 1.5,
	      paddingRight: _vm.paddingHead * 1.5
	    }
	  }, [_vm._v(_vm._s(_vm.title))]), _c('div', {
	    class: ['panel-body', 'panel-body-' + _vm.type],
	    style: {
	      paddingTop: _vm.paddingBody,
	      paddingBottom: _vm.paddingBody,
	      paddingLeft: _vm.paddingBody * 1.5,
	      paddingRight: _vm.paddingBody * 1.5
	    }
	  }, [_vm._t("default")], 2)])
	},staticRenderFns: []}
	module.exports.render._withStripped = true

/***/ }),

/***/ 78:
/***/ (function(module, exports) {

	module.exports = {
	  "txt": {
	    "marginBottom": 12,
	    "fontSize": 40
	  }
	}

/***/ }),

/***/ 79:
/***/ (function(module, exports, __webpack_require__) {

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

	module.exports = {
	  components: {
	    panel: __webpack_require__(3)
	  }
	};

/***/ }),

/***/ 80:
/***/ (function(module, exports) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('scroller', [_c('panel', {
	    attrs: {
	      "title": "color",
	      "type": "primary"
	    }
	  }, [_c('text', {
	    staticClass: ["txt"]
	  }, [_vm._v("default is black")]), _c('text', {
	    staticClass: ["txt"],
	    staticStyle: {
	      color: "#286090"
	    }
	  }, [_vm._v("#286090")]), _c('text', {
	    staticClass: ["txt"],
	    staticStyle: {
	      color: "#0f0"
	    }
	  }, [_vm._v("#0f0")]), _c('text', {
	    staticClass: ["txt"],
	    staticStyle: {
	      color: "red"
	    }
	  }, [_vm._v("keyword")]), _c('text', {
	    staticClass: ["txt"],
	    staticStyle: {
	      color: "rgb(238, 162, 54)"
	    }
	  }, [_vm._v("rgb(238, 162, 54)")]), _c('text', {
	    staticClass: ["txt"],
	    staticStyle: {
	      color: "rgba(238, 162, 54, 0.5)"
	    }
	  }, [_vm._v("rgba(238, 162, 54, 0.5)")])]), _c('panel', {
	    attrs: {
	      "title": "font-size",
	      "type": "primary"
	    }
	  }, [_c('text', {
	    staticClass: ["txt"],
	    staticStyle: {
	      fontSize: "32px"
	    }
	  }, [_vm._v("32")]), _c('text', {
	    staticClass: ["txt"],
	    staticStyle: {
	      fontSize: "64px"
	    }
	  }, [_vm._v("64")]), _c('text', {
	    staticClass: ["txt"],
	    staticStyle: {
	      fontSize: "100px"
	    }
	  }, [_vm._v("100")])]), _c('panel', {
	    attrs: {
	      "title": "font-style",
	      "type": "primary"
	    }
	  }, [_c('text', {
	    staticClass: ["txt"],
	    staticStyle: {
	      fontStyle: "normal"
	    }
	  }, [_vm._v("normal")]), _c('text', {
	    staticClass: ["txt"],
	    staticStyle: {
	      fontStyle: "italic"
	    }
	  }, [_vm._v("italic")])]), _c('panel', {
	    attrs: {
	      "title": "font-weight",
	      "type": "primary"
	    }
	  }, [_c('text', {
	    staticClass: ["txt"],
	    staticStyle: {
	      fontWeight: "normal"
	    }
	  }, [_vm._v("normal")]), _c('text', {
	    staticClass: ["txt"],
	    staticStyle: {
	      fontWeight: "bold"
	    }
	  }, [_vm._v("bold")])]), _c('panel', {
	    attrs: {
	      "title": "text-decoration",
	      "type": "primary"
	    }
	  }, [_c('text', {
	    staticClass: ["txt"],
	    staticStyle: {
	      textDecoration: "none"
	    }
	  }, [_vm._v("none")]), _c('text', {
	    staticClass: ["txt"],
	    staticStyle: {
	      textDecoration: "underline"
	    }
	  }, [_vm._v("underline")]), _c('text', {
	    staticClass: ["txt"],
	    staticStyle: {
	      textDecoration: "line-through"
	    }
	  }, [_vm._v("line-through")])]), _c('panel', {
	    attrs: {
	      "title": "text-align",
	      "type": "primary"
	    }
	  }, [_c('text', {
	    staticClass: ["txt"],
	    staticStyle: {
	      textAlign: "left"
	    }
	  }, [_vm._v("left")]), _c('text', {
	    staticClass: ["txt"],
	    staticStyle: {
	      textAlign: "center"
	    }
	  }, [_vm._v("center")]), _c('text', {
	    staticClass: ["txt"],
	    staticStyle: {
	      textAlign: "right"
	    }
	  }, [_vm._v("right")])]), _c('panel', {
	    attrs: {
	      "title": "text-overflow",
	      "type": "primary"
	    }
	  }, [_c('text', {
	    staticClass: ["txt"],
	    staticStyle: {
	      lines: "1"
	    }
	  }, [_vm._v("no text-overflow, no text-overflow")]), _c('text', {
	    staticClass: ["txt"],
	    staticStyle: {
	      textOverflow: "clip",
	      width: "450px",
	      lines: "1"
	    }
	  }, [_vm._v("text-overflow: clip, text-overflow: clip")]), _c('text', {
	    staticClass: ["txt"],
	    staticStyle: {
	      textOverflow: "ellipsis",
	      width: "450px",
	      lines: "1"
	    }
	  }, [_vm._v("text-overflow: ellipsis, text-overflow: ellipsis")])]), _c('panel', {
	    attrs: {
	      "title": "line-height",
	      "type": "primary"
	    }
	  }, [_c('text', {
	    staticClass: ["txt"]
	  }, [_vm._v("no lineheight setting")]), _c('text', {
	    staticClass: ["txt"],
	    staticStyle: {
	      lineHeight: "50px"
	    }
	  }, [_vm._v("lineheight 50")]), _c('text', {
	    staticClass: ["txt"],
	    staticStyle: {
	      lineHeight: "80px"
	    }
	  }, [_vm._v(" lineheight 80")])])], 1)
	},staticRenderFns: []}
	module.exports.render._withStripped = true

/***/ })

/******/ });