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
	__vue_styles__.push(__webpack_require__(203)
	)

	/* script */
	__vue_exports__ = __webpack_require__(204)

	/* template */
	var __vue_template__ = __webpack_require__(217)
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
	__vue_options__.__file = "/Users/Hanks/Codes/work/incubator-weex/examples/vue/style/index.vue"
	__vue_options__.render = __vue_template__.render
	__vue_options__.staticRenderFns = __vue_template__.staticRenderFns
	__vue_options__._scopeId = "data-v-2f3b8d9c"
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

/***/ 13:
/***/ (function(module, exports, __webpack_require__) {

	var __vue_exports__, __vue_options__
	var __vue_styles__ = []

	/* styles */
	__vue_styles__.push(__webpack_require__(14)
	)

	/* script */
	__vue_exports__ = __webpack_require__(15)

	/* template */
	var __vue_template__ = __webpack_require__(16)
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
	__vue_options__.__file = "/Users/Hanks/Codes/work/incubator-weex/examples/vue/include/tip.vue"
	__vue_options__.render = __vue_template__.render
	__vue_options__.staticRenderFns = __vue_template__.staticRenderFns
	__vue_options__._scopeId = "data-v-36fe495c"
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

/***/ 14:
/***/ (function(module, exports) {

	module.exports = {
	  "tip": {
	    "paddingLeft": 36,
	    "paddingRight": 36,
	    "paddingTop": 36,
	    "paddingBottom": 36,
	    "borderRadius": 10
	  },
	  "tip-txt": {
	    "fontSize": 28
	  },
	  "tip-success": {
	    "backgroundColor": "#dff0d8",
	    "borderColor": "#d6e9c6"
	  },
	  "tip-txt-success": {
	    "color": "#3c763d"
	  },
	  "tip-info": {
	    "backgroundColor": "#d9edf7",
	    "borderColor": "#bce8f1"
	  },
	  "tip-txt-info": {
	    "color": "#31708f"
	  },
	  "tip-warning": {
	    "backgroundColor": "#fcf8e3",
	    "borderColor": "#faebcc"
	  },
	  "tip-txt-warning": {
	    "color": "#8a6d3b"
	  },
	  "tip-danger": {
	    "backgroundColor": "#f2dede",
	    "borderColor": "#ebccd1"
	  },
	  "tip-txt-danger": {
	    "color": "#a94442"
	  }
	}

/***/ }),

/***/ 15:
/***/ (function(module, exports) {

	'use strict';

	//
	//
	//
	//
	//
	//
	//

	module.exports = {
	  props: {
	    type: { default: 'success' },
	    value: { default: '' }
	  }
	};

/***/ }),

/***/ 16:
/***/ (function(module, exports) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', {
	    class: ['tip', 'tip-' + _vm.type]
	  }, [_c('text', {
	    class: ['tip-txt', 'tip-txt-' + _vm.type]
	  }, [_vm._v(_vm._s(_vm.value))])])
	},staticRenderFns: []}
	module.exports.render._withStripped = true

/***/ }),

/***/ 203:
/***/ (function(module, exports) {

	module.exports = {
	  "bg-item": {
	    "width": 690,
	    "marginBottom": 10
	  }
	}

/***/ }),

/***/ 204:
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

	module.exports = {
	  components: {
	    panel: __webpack_require__(3),
	    styleBox: __webpack_require__(205),
	    styleFlex: __webpack_require__(213),
	    styleItem: __webpack_require__(208)
	  }
	};

/***/ }),

/***/ 205:
/***/ (function(module, exports, __webpack_require__) {

	var __vue_exports__, __vue_options__
	var __vue_styles__ = []

	/* styles */
	__vue_styles__.push(__webpack_require__(206)
	)

	/* script */
	__vue_exports__ = __webpack_require__(207)

	/* template */
	var __vue_template__ = __webpack_require__(212)
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
	__vue_options__.__file = "/Users/Hanks/Codes/work/incubator-weex/examples/vue/style/style-box.vue"
	__vue_options__.render = __vue_template__.render
	__vue_options__.staticRenderFns = __vue_template__.staticRenderFns
	__vue_options__._scopeId = "data-v-9578d60e"
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

/***/ 206:
/***/ (function(module, exports) {

	module.exports = {
	  "box": {
	    "backgroundColor": "#f5f5f5",
	    "width": 260,
	    "height": 260,
	    "paddingLeft": 40,
	    "paddingTop": 40,
	    "paddingRight": 40,
	    "paddingBottom": 40,
	    "marginLeft": 40,
	    "marginTop": 40,
	    "marginRight": 40,
	    "marginBottom": 40,
	    "borderWidth": 40,
	    "borderColor": "#333333",
	    "borderStyle": "solid"
	  }
	}

/***/ }),

/***/ 207:
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
	    panel: __webpack_require__(3),
	    tip: __webpack_require__(13),
	    styleItem: __webpack_require__(208)
	  }
	};

/***/ }),

/***/ 208:
/***/ (function(module, exports, __webpack_require__) {

	var __vue_exports__, __vue_options__
	var __vue_styles__ = []

	/* styles */
	__vue_styles__.push(__webpack_require__(209)
	)

	/* script */
	__vue_exports__ = __webpack_require__(210)

	/* template */
	var __vue_template__ = __webpack_require__(211)
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
	__vue_options__.__file = "/Users/Hanks/Codes/work/incubator-weex/examples/vue/style/style-item.vue"
	__vue_options__.render = __vue_template__.render
	__vue_options__.staticRenderFns = __vue_template__.staticRenderFns
	__vue_options__._scopeId = "data-v-947d2656"
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

/***/ 209:
/***/ (function(module, exports) {

	module.exports = {
	  "item": {
	    "marginRight": 10,
	    "width": 160,
	    "height": 75,
	    "paddingLeft": 8,
	    "paddingRight": 8,
	    "paddingTop": 8,
	    "paddingBottom": 8
	  },
	  "txt": {
	    "color": "#eeeeee"
	  }
	}

/***/ }),

/***/ 210:
/***/ (function(module, exports) {

	'use strict';

	//
	//
	//
	//
	//
	//
	//

	module.exports = {
	  props: {
	    value: { default: '' },
	    type: { default: '0' // 0, 1
	    } },
	  computed: {
	    bgColor: function bgColor() {
	      return this.type == '1' ? '#7BA3A8' : '#BEAD92';
	    }
	  }
	};

/***/ }),

/***/ 211:
/***/ (function(module, exports) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('text', {
	    staticClass: ["item", "txt"],
	    style: {
	      backgroundColor: _vm.bgColor
	    },
	    attrs: {
	      "value": _vm.value
	    }
	  })
	},staticRenderFns: []}
	module.exports.render._withStripped = true

/***/ }),

/***/ 212:
/***/ (function(module, exports) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', [_c('panel', {
	    attrs: {
	      "title": "Box Model",
	      "paddingBody": "0",
	      "type": "primary"
	    }
	  }, [_c('text', {
	    staticClass: ["box"]
	  }, [_vm._v("Box")])]), _c('panel', {
	    attrs: {
	      "title": "border",
	      "type": "primary"
	    }
	  }, [_c('panel', {
	    attrs: {
	      "title": "border-width",
	      "type": ""
	    }
	  }, [_c('div', {
	    staticStyle: {
	      flexDirection: "row"
	    }
	  }, [_c('style-item', {
	    staticStyle: {
	      borderStyle: "solid",
	      borderWidth: "2px",
	      borderColor: "#333"
	    },
	    attrs: {
	      "value": ""
	    }
	  }), _c('style-item', {
	    staticStyle: {
	      borderStyle: "solid",
	      borderWidth: "10px",
	      borderColor: "#333"
	    },
	    attrs: {
	      "value": ""
	    }
	  }), _c('style-item', {
	    staticStyle: {
	      borderStyle: "solid",
	      borderLeftWidth: "4px",
	      borderColor: "#333"
	    },
	    attrs: {
	      "value": ""
	    }
	  }), _c('style-item', {
	    staticStyle: {
	      borderStyle: "solid",
	      borderBottomWidth: "4px",
	      borderColor: "#333"
	    },
	    attrs: {
	      "value": ""
	    }
	  })], 1)]), _c('panel', {
	    attrs: {
	      "title": "border-color",
	      "type": ""
	    }
	  }, [_c('div', {
	    staticStyle: {
	      flexDirection: "row"
	    }
	  }, [_c('style-item', {
	    staticStyle: {
	      borderStyle: "solid",
	      borderWidth: "4px",
	      borderColor: "#333"
	    },
	    attrs: {
	      "value": ""
	    }
	  }), _c('style-item', {
	    staticStyle: {
	      borderStyle: "solid",
	      borderWidth: "4px",
	      borderColor: "#ddd"
	    },
	    attrs: {
	      "value": ""
	    }
	  }), _c('style-item', {
	    staticStyle: {
	      borderStyle: "solid",
	      borderWidth: "4px",
	      borderColor: "red"
	    },
	    attrs: {
	      "value": ""
	    }
	  })], 1)]), _c('panel', {
	    attrs: {
	      "title": "border-style",
	      "type": ""
	    }
	  }, [_c('tip', {
	    staticStyle: {
	      marginBottom: "10px"
	    },
	    attrs: {
	      "type": "warning",
	      "value": "just support four edges"
	    }
	  }), _c('div', {
	    staticStyle: {
	      flexDirection: "row"
	    }
	  }, [_c('style-item', {
	    staticStyle: {
	      borderStyle: "solid",
	      borderWidth: "4px",
	      borderColor: "#333"
	    },
	    attrs: {
	      "value": ""
	    }
	  }), _c('style-item', {
	    staticStyle: {
	      borderStyle: "dashed",
	      borderWidth: "4px",
	      borderColor: "#333"
	    },
	    attrs: {
	      "value": "",
	      "type": ""
	    }
	  }), _c('style-item', {
	    staticStyle: {
	      borderStyle: "dotted",
	      borderWidth: "4px",
	      borderColor: "#333"
	    },
	    attrs: {
	      "value": ""
	    }
	  })], 1)], 1), _c('panel', {
	    attrs: {
	      "title": "border-radius",
	      "type": ""
	    }
	  }, [_c('div', {
	    staticStyle: {
	      flexDirection: "row"
	    }
	  }, [_c('style-item', {
	    staticStyle: {
	      width: "75px",
	      borderRadius: "38px"
	    },
	    attrs: {
	      "value": ""
	    }
	  }), _c('style-item', {
	    staticStyle: {
	      borderRadius: "20px"
	    },
	    attrs: {
	      "value": ""
	    }
	  }), _c('style-item', {
	    staticStyle: {
	      borderTopLeftRadius: "20px",
	      borderTopRightRadius: "20px"
	    },
	    attrs: {
	      "value": ""
	    }
	  }), _c('style-item', {
	    staticStyle: {
	      borderBottomLeftRadius: "20px",
	      borderBottomRightRadius: "20px"
	    },
	    attrs: {
	      "value": ""
	    }
	  })], 1)])], 1)], 1)
	},staticRenderFns: []}
	module.exports.render._withStripped = true

/***/ }),

/***/ 213:
/***/ (function(module, exports, __webpack_require__) {

	var __vue_exports__, __vue_options__
	var __vue_styles__ = []

	/* styles */
	__vue_styles__.push(__webpack_require__(214)
	)

	/* script */
	__vue_exports__ = __webpack_require__(215)

	/* template */
	var __vue_template__ = __webpack_require__(216)
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
	__vue_options__.__file = "/Users/Hanks/Codes/work/incubator-weex/examples/vue/style/style-flex.vue"
	__vue_options__.render = __vue_template__.render
	__vue_options__.staticRenderFns = __vue_template__.staticRenderFns
	__vue_options__._scopeId = "data-v-50f1e28a"
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

/***/ 214:
/***/ (function(module, exports) {

	module.exports = {
	  "row": {
	    "flexDirection": "row",
	    "marginBottom": 10
	  },
	  "row-align": {
	    "height": 160,
	    "borderWidth": 3,
	    "borderColor": "#dddddd",
	    "borderStyle": "solid"
	  }
	}

/***/ }),

/***/ 215:
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
	  components: {
	    panel: __webpack_require__(3),
	    styleItem: __webpack_require__(208)
	  }
	};

/***/ }),

/***/ 216:
/***/ (function(module, exports) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', [_c('panel', {
	    attrs: {
	      "title": "flex-direction",
	      "type": "primary"
	    }
	  }, [_c('div', {
	    staticClass: ["row"],
	    staticStyle: {
	      flexDirection: "row"
	    }
	  }, [_c('style-item', {
	    staticStyle: {
	      marginRight: "0"
	    },
	    attrs: {
	      "value": "row"
	    }
	  }), _c('style-item', {
	    attrs: {
	      "type": "1"
	    }
	  })], 1), _c('div', {
	    staticClass: ["row"],
	    staticStyle: {
	      flexDirection: "column"
	    }
	  }, [_c('style-item', {
	    attrs: {
	      "value": "column"
	    }
	  }), _c('style-item', {
	    attrs: {
	      "type": "1"
	    }
	  })], 1)]), _c('panel', {
	    attrs: {
	      "title": "justify-content",
	      "type": "primary"
	    }
	  }, [_c('div', {
	    staticClass: ["row"],
	    staticStyle: {
	      justifyContent: "flex-start"
	    }
	  }, [_c('style-item', {
	    staticStyle: {
	      marginRight: "0"
	    }
	  }), _c('style-item', {
	    staticStyle: {
	      marginRight: "0"
	    },
	    attrs: {
	      "type": "1"
	    }
	  }), _c('style-item', {
	    staticStyle: {
	      marginRight: "0"
	    }
	  })], 1), _c('div', {
	    staticClass: ["row"],
	    staticStyle: {
	      justifyContent: "flex-end"
	    }
	  }, [_c('style-item', {
	    staticStyle: {
	      marginRight: "0"
	    }
	  }), _c('style-item', {
	    staticStyle: {
	      marginRight: "0"
	    },
	    attrs: {
	      "type": "1"
	    }
	  }), _c('style-item', {
	    staticStyle: {
	      marginRight: "0"
	    }
	  })], 1), _c('div', {
	    staticClass: ["row"],
	    staticStyle: {
	      justifyContent: "center"
	    }
	  }, [_c('style-item', {
	    staticStyle: {
	      marginRight: "0"
	    }
	  }), _c('style-item', {
	    staticStyle: {
	      marginRight: "0"
	    },
	    attrs: {
	      "type": "1"
	    }
	  }), _c('style-item', {
	    staticStyle: {
	      marginRight: "0"
	    }
	  })], 1), _c('div', {
	    staticClass: ["row"],
	    staticStyle: {
	      justifyContent: "space-between"
	    }
	  }, [_c('style-item', {
	    staticStyle: {
	      marginRight: "0"
	    }
	  }), _c('style-item', {
	    staticStyle: {
	      marginRight: "0"
	    },
	    attrs: {
	      "type": "1"
	    }
	  }), _c('style-item', {
	    staticStyle: {
	      marginRight: "0"
	    }
	  })], 1)]), _c('panel', {
	    attrs: {
	      "title": "align-items",
	      "type": "primary"
	    }
	  }, [_c('div', {
	    staticClass: ["row", "row-align"],
	    staticStyle: {
	      alignItems: "flex-start"
	    }
	  }, [_c('style-item', {
	    staticStyle: {
	      marginRight: "0"
	    }
	  }), _c('style-item', {
	    staticStyle: {
	      marginRight: "0"
	    },
	    attrs: {
	      "type": "1"
	    }
	  }), _c('style-item', {
	    staticStyle: {
	      marginRight: "0"
	    }
	  })], 1), _c('div', {
	    staticClass: ["row", "row-align"],
	    staticStyle: {
	      alignItems: "flex-end"
	    }
	  }, [_c('style-item', {
	    staticStyle: {
	      marginRight: "0"
	    }
	  }), _c('style-item', {
	    staticStyle: {
	      marginRight: "0"
	    },
	    attrs: {
	      "type": "1"
	    }
	  }), _c('style-item', {
	    staticStyle: {
	      marginRight: "0"
	    }
	  })], 1), _c('div', {
	    staticClass: ["row", "row-align"],
	    staticStyle: {
	      alignItems: "center"
	    }
	  }, [_c('style-item', {
	    staticStyle: {
	      marginRight: "0"
	    }
	  }), _c('style-item', {
	    staticStyle: {
	      marginRight: "0"
	    },
	    attrs: {
	      "type": "1"
	    }
	  }), _c('style-item', {
	    staticStyle: {
	      marginRight: "0"
	    }
	  })], 1), _c('div', {
	    staticClass: ["row", "row-align"],
	    staticStyle: {
	      alignItems: "stretch"
	    }
	  }, [_c('style-item', {
	    staticStyle: {
	      marginRight: "0"
	    }
	  }), _c('style-item', {
	    staticStyle: {
	      marginRight: "0"
	    },
	    attrs: {
	      "type": "1"
	    }
	  }), _c('style-item', {
	    staticStyle: {
	      marginRight: "0"
	    }
	  })], 1)]), _c('panel', {
	    attrs: {
	      "title": "flex",
	      "type": "primary"
	    }
	  }, [_c('div', {
	    staticClass: ["row"],
	    staticStyle: {
	      alignItems: "flex-start"
	    }
	  }, [_c('style-item', {
	    staticStyle: {
	      marginRight: "0"
	    },
	    attrs: {
	      "value": "no flex"
	    }
	  }), _c('style-item', {
	    staticStyle: {
	      marginRight: "0"
	    },
	    attrs: {
	      "value": "no flex",
	      "type": "1"
	    }
	  }), _c('style-item', {
	    staticStyle: {
	      marginRight: "0"
	    },
	    attrs: {
	      "value": "no flex"
	    }
	  })], 1), _c('div', {
	    staticClass: ["row"],
	    staticStyle: {
	      alignItems: "flex-start"
	    }
	  }, [_c('style-item', {
	    staticStyle: {
	      marginRight: "0"
	    },
	    attrs: {
	      "value": "no flex"
	    }
	  }), _c('style-item', {
	    staticStyle: {
	      marginRight: "0",
	      flex: "1"
	    },
	    attrs: {
	      "type": "1",
	      "value": "flex:1"
	    }
	  }), _c('style-item', {
	    staticStyle: {
	      marginRight: "0",
	      flex: "2"
	    },
	    attrs: {
	      "value": "flex:2"
	    }
	  })], 1), _c('div', {
	    staticClass: ["row"],
	    staticStyle: {
	      alignItems: "flex-start"
	    }
	  }, [_c('style-item', {
	    staticStyle: {
	      marginRight: "0",
	      flex: "1"
	    },
	    attrs: {
	      "value": "flex:1"
	    }
	  }), _c('style-item', {
	    staticStyle: {
	      marginRight: "0",
	      flex: "1"
	    },
	    attrs: {
	      "type": "1",
	      "value": "flex:1"
	    }
	  }), _c('style-item', {
	    staticStyle: {
	      marginRight: "0",
	      flex: "1"
	    },
	    attrs: {
	      "value": "flex:1"
	    }
	  })], 1), _c('div', {
	    staticClass: ["row"],
	    staticStyle: {
	      alignItems: "flex-start"
	    }
	  }, [_c('style-item', {
	    staticStyle: {
	      marginRight: "0",
	      flex: "1"
	    },
	    attrs: {
	      "value": "flex:1"
	    }
	  }), _c('style-item', {
	    staticStyle: {
	      marginRight: "0",
	      flex: "2"
	    },
	    attrs: {
	      "type": "1",
	      "value": "flex:2"
	    }
	  }), _c('style-item', {
	    staticStyle: {
	      marginRight: "0",
	      flex: "3"
	    },
	    attrs: {
	      "value": "flex:3"
	    }
	  })], 1)]), _c('panel', {
	    attrs: {
	      "title": "flex-wrap",
	      "type": "primary"
	    }
	  }, [_c('div', {
	    staticClass: ["row"],
	    staticStyle: {
	      flexWrap: "wrap",
	      backgroundColor: "grey",
	      padding: "10"
	    }
	  }, [_c('style-item', {
	    attrs: {
	      "value": "1:wrap"
	    }
	  }), _c('style-item', {
	    attrs: {
	      "value": "2:wrap"
	    }
	  }), _c('style-item', {
	    attrs: {
	      "value": "3:wrap"
	    }
	  }), _c('style-item', {
	    attrs: {
	      "value": "4:wrap"
	    }
	  }), _c('style-item', {
	    attrs: {
	      "value": "5:wrap"
	    }
	  }), _c('style-item', {
	    attrs: {
	      "value": "6:wrap"
	    }
	  })], 1), _c('div', {
	    staticClass: ["row"],
	    staticStyle: {
	      flexWrap: "nowrap",
	      backgroundColor: "grey",
	      padding: "10"
	    }
	  }, [_c('style-item', {
	    attrs: {
	      "value": "1:nowrap"
	    }
	  }), _c('style-item', {
	    attrs: {
	      "value": "2:nowrap"
	    }
	  }), _c('style-item', {
	    attrs: {
	      "value": "3:nowrap"
	    }
	  }), _c('style-item', {
	    attrs: {
	      "value": "4:nowrap"
	    }
	  }), _c('style-item', {
	    attrs: {
	      "value": "5:nowrap"
	    }
	  }), _c('style-item', {
	    attrs: {
	      "value": "6:nowrap"
	    }
	  })], 1)])], 1)
	},staticRenderFns: []}
	module.exports.render._withStripped = true

/***/ }),

/***/ 217:
/***/ (function(module, exports) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('scroller', {
	    staticStyle: {
	      overflowX: "hidden"
	    }
	  }, [_c('style-box'), _c('style-flex'), _c('panel', {
	    attrs: {
	      "title": "opacity",
	      "type": "primary"
	    }
	  }, [_c('div', {
	    staticStyle: {
	      flexDirection: "row"
	    }
	  }, [_c('style-item', {
	    staticStyle: {
	      opacity: "1"
	    },
	    attrs: {
	      "value": "1"
	    }
	  }), _c('style-item', {
	    staticStyle: {
	      opacity: "0.9"
	    },
	    attrs: {
	      "value": "0.9"
	    }
	  }), _c('style-item', {
	    staticStyle: {
	      opacity: "0.5"
	    },
	    attrs: {
	      "value": "0.5"
	    }
	  }), _c('style-item', {
	    staticStyle: {
	      opacity: "0.2"
	    },
	    attrs: {
	      "value": "0.2"
	    }
	  })], 1)]), _c('panel', {
	    attrs: {
	      "title": "background-color",
	      "type": "primary"
	    }
	  }, [_c('div', [_c('style-item', {
	    staticClass: ["bg-item"],
	    staticStyle: {
	      backgroundColor: "#333"
	    },
	    attrs: {
	      "value": "#333"
	    }
	  }), _c('style-item', {
	    staticClass: ["bg-item"],
	    staticStyle: {
	      backgroundColor: "#333333"
	    },
	    attrs: {
	      "value": "#333333"
	    }
	  }), _c('style-item', {
	    staticClass: ["bg-item"],
	    staticStyle: {
	      backgroundColor: "rgb(238, 162, 54)"
	    },
	    attrs: {
	      "value": "rgb()"
	    }
	  }), _c('style-item', {
	    staticClass: ["bg-item"],
	    staticStyle: {
	      backgroundColor: "rgba(238, 162, 54, 0.5)"
	    },
	    attrs: {
	      "value": "rgba()"
	    }
	  }), _c('style-item', {
	    staticClass: ["bg-item"],
	    staticStyle: {
	      backgroundColor: "red"
	    },
	    attrs: {
	      "value": "red"
	    }
	  })], 1)])], 1)
	},staticRenderFns: []}
	module.exports.render._withStripped = true

/***/ })

/******/ });