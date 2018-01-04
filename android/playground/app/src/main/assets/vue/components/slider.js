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
	__vue_styles__.push(__webpack_require__(54)
	)

	/* script */
	__vue_exports__ = __webpack_require__(55)

	/* template */
	var __vue_template__ = __webpack_require__(64)
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
	__vue_options__.__file = "/Users/Hanks/Codes/work/incubator-weex/examples/vue/components/slider.vue"
	__vue_options__.render = __vue_template__.render
	__vue_options__.staticRenderFns = __vue_template__.staticRenderFns
	__vue_options__._scopeId = "data-v-e4ae44bc"
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

/***/ 54:
/***/ (function(module, exports) {

	module.exports = {
	  "body": {
	    "backgroundColor": "#ffffff"
	  },
	  "slider": {
	    "flexDirection": "row",
	    "width": 690,
	    "height": 360
	  },
	  "indicator": {
	    "position": "absolute",
	    "width": 690,
	    "height": 420,
	    "top": 140,
	    "left": 240,
	    "itemColor": "#dddddd",
	    "itemSelectedColor": "rgb(40,96,144)"
	  }
	}

/***/ }),

/***/ 55:
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

	var img0 = '//gw.alicdn.com/tps/i2/TB1DpsmMpXXXXabaXXX20ySQVXX-512-512.png_400x400.jpg';
	var img1 = '//gw.alicdn.com/tps/i1/TB1M3sQMpXXXXakXXXXApNeJVXX-360-360.png';
	module.exports = {
	  data: function data() {
	    return {
	      eventCnt: 0,
	      togglePlayMsg: 'pause',
	      sliders: [{
	        interval: 1000,
	        autoPlay: true,
	        sliderPages: [{
	          items: [{
	            image: img0,
	            link: '//h5.m.taobao.com/1'
	          }, {
	            image: img0,
	            link: '//h5.m.taobao.com/1'
	          }]
	        }, {
	          items: [{
	            image: img1,
	            link: '//h5.m.taobao.com/1'
	          }, {
	            image: img1,
	            link: '//h5.m.taobao.com/1'
	          }]
	        }, {
	          items: [{
	            image: img0,
	            link: '//h5.m.taobao.com/1'
	          }, {
	            image: img1,
	            link: '//h5.m.taobao.com/1'
	          }]
	        }]
	      }, {
	        interval: 3000,
	        autoPlay: true,
	        sliderPages: [{
	          items: [{
	            image: img0,
	            link: '//h5.m.taobao.com/1'
	          }, {
	            image: img0,
	            link: '//h5.m.taobao.com/1'
	          }]
	        }, {
	          items: [{
	            image: img1,
	            link: '//h5.m.taobao.com/1'
	          }, {
	            image: img1,
	            link: '//h5.m.taobao.com/1'
	          }]
	        }, {
	          items: [{
	            image: img0,
	            link: '//h5.m.taobao.com/1'
	          }, {
	            image: img1,
	            link: '//h5.m.taobao.com/1'
	          }]
	        }]
	      }, {
	        interval: 5000,
	        autoPlay: true,
	        sliderPages: [{
	          items: [{
	            image: img0,
	            link: '//h5.m.taobao.com/1'
	          }, {
	            image: img0,
	            link: '//h5.m.taobao.com/1'
	          }]
	        }, {
	          items: [{
	            image: img1,
	            link: '//h5.m.taobao.com/1'
	          }, {
	            image: img1,
	            link: '//h5.m.taobao.com/1'
	          }]
	        }, {
	          items: [{
	            image: img0,
	            link: '//h5.m.taobao.com/1'
	          }, {
	            image: img1,
	            link: '//h5.m.taobao.com/1'
	          }]
	        }]
	      }]
	    };
	  },
	  components: {
	    panel: __webpack_require__(3),
	    sliderPage: __webpack_require__(56)
	  },
	  methods: {
	    togglePlay: function togglePlay() {
	      var autoPlay = this.sliders[0].autoPlay;
	      autoPlay = !autoPlay;
	      this.sliders[0].autoPlay = autoPlay;
	      this.togglePlayMsg = autoPlay ? 'pause' : 'play';
	    },
	    handleSliderChange: function handleSliderChange() {
	      var nowCnt = this.eventCnt + 1;
	      this.eventCnt = nowCnt;
	    },
	    setInterval1: function setInterval1() {
	      this.sliders[0].interval = 1000;
	    },
	    setInterval3: function setInterval3() {
	      this.sliders[0].interval = 3000;
	    },
	    setInterval5: function setInterval5() {
	      this.sliders[0].interval = 5000;
	    }
	  }
	};

/***/ }),

/***/ 56:
/***/ (function(module, exports, __webpack_require__) {

	var __vue_exports__, __vue_options__
	var __vue_styles__ = []

	/* styles */
	__vue_styles__.push(__webpack_require__(57)
	)

	/* script */
	__vue_exports__ = __webpack_require__(58)

	/* template */
	var __vue_template__ = __webpack_require__(63)
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
	__vue_options__.__file = "/Users/Hanks/Codes/work/incubator-weex/examples/vue/include/slider-page.vue"
	__vue_options__.render = __vue_template__.render
	__vue_options__.staticRenderFns = __vue_template__.staticRenderFns
	__vue_options__._scopeId = "data-v-e1f8e088"
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

/***/ 57:
/***/ (function(module, exports) {

	module.exports = {
	  "slider-page": {
	    "flexDirection": "row",
	    "justifyContent": "space-between",
	    "width": 714,
	    "height": 420
	  }
	}

/***/ }),

/***/ 58:
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

	module.exports = {
	  props: {
	    items: { default: [] }
	  },
	  components: {
	    sliderItem: __webpack_require__(59)
	  }
	};

/***/ }),

/***/ 59:
/***/ (function(module, exports, __webpack_require__) {

	var __vue_exports__, __vue_options__
	var __vue_styles__ = []

	/* styles */
	__vue_styles__.push(__webpack_require__(60)
	)

	/* script */
	__vue_exports__ = __webpack_require__(61)

	/* template */
	var __vue_template__ = __webpack_require__(62)
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
	__vue_options__.__file = "/Users/Hanks/Codes/work/incubator-weex/examples/vue/include/slider-item.vue"
	__vue_options__.render = __vue_template__.render
	__vue_options__.staticRenderFns = __vue_template__.staticRenderFns
	__vue_options__._scopeId = "data-v-bc0e7380"
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

/***/ 60:
/***/ (function(module, exports) {

	module.exports = {
	  "slider-item": {
	    "width": 348,
	    "height": 400
	  }
	}

/***/ }),

/***/ 61:
/***/ (function(module, exports) {

	'use strict';

	//
	//
	//
	//

	module.exports = {
	  props: {
	    image: { default: '' },
	    link: { default: '' }
	  }
	};

/***/ }),

/***/ 62:
/***/ (function(module, exports) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('image', {
	    staticClass: ["slider-item"],
	    attrs: {
	      "src": _vm.image
	    }
	  })
	},staticRenderFns: []}
	module.exports.render._withStripped = true

/***/ }),

/***/ 63:
/***/ (function(module, exports) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', {
	    staticClass: ["slider-page"]
	  }, _vm._l((_vm.items), function(v, i) {
	    return _c('slider-item', {
	      key: i,
	      attrs: {
	        "image": v.image,
	        "link": v.link
	      }
	    })
	  }))
	},staticRenderFns: []}
	module.exports.render._withStripped = true

/***/ }),

/***/ 64:
/***/ (function(module, exports) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('scroller', {
	    staticClass: ["body"]
	  }, [_c('panel', {
	    attrs: {
	      "title": "auto-play",
	      "type": "primary"
	    }
	  }, [_c('panel', {
	    attrs: {
	      "title": "auto-play = false",
	      "paddingBody": "0"
	    }
	  }, [_c('slider', {
	    staticClass: ["slider"],
	    appendAsTree: true,
	    attrs: {
	      "append": "tree",
	      "interval": _vm.sliders[1].interval,
	      "autoPlay": "false"
	    }
	  }, [_c('indicator', {
	    staticClass: ["indicator"]
	  }), _vm._l((_vm.sliders[1].sliderPages), function(v, i) {
	    return _c('slider-page', {
	      key: i,
	      attrs: {
	        "items": v.items
	      }
	    })
	  })], 2)]), _c('panel', {
	    attrs: {
	      "title": "auto-play = true",
	      "paddingBody": "0"
	    }
	  }, [_c('slider', {
	    staticClass: ["slider"],
	    appendAsTree: true,
	    attrs: {
	      "append": "tree",
	      "interval": _vm.sliders[0].interval,
	      "autoPlay": _vm.sliders[0].autoPlay
	    }
	  }, [_c('indicator', {
	    staticClass: ["indicator"]
	  }), _vm._l((_vm.sliders[0].sliderPages), function(v) {
	    return _c('slider-page', {
	      key: v.index,
	      attrs: {
	        "items": v.items
	      }
	    })
	  })], 2)])], 1), _c('panel', {
	    attrs: {
	      "title": 'Event, ' + _vm.eventCnt + ' change',
	      "type": "primary"
	    }
	  }, [_c('slider', {
	    staticClass: ["slider"],
	    appendAsTree: true,
	    attrs: {
	      "append": "tree",
	      "interval": _vm.sliders[0].interval,
	      "autoPlay": _vm.sliders[0].autoPlay
	    },
	    on: {
	      "change": _vm.handleSliderChange
	    }
	  }, [_c('indicator', {
	    staticClass: ["indicator"]
	  }), _vm._l((_vm.sliders[0].sliderPages), function(v) {
	    return _c('slider-page', {
	      key: v.index,
	      attrs: {
	        "items": v.items
	      }
	    })
	  })], 2)]), _c('panel', {
	    attrs: {
	      "title": "Indicator",
	      "type": "primary"
	    }
	  }, [_c('panel', {
	    attrs: {
	      "title": "default style",
	      "paddingBody": "0"
	    }
	  }, [_c('slider', {
	    staticClass: ["slider"],
	    appendAsTree: true,
	    attrs: {
	      "append": "tree",
	      "interval": _vm.sliders[0].interval,
	      "autoPlay": _vm.sliders[0].autoPlay
	    },
	    on: {
	      "change": _vm.handleSliderChange
	    }
	  }, [_c('indicator', {
	    staticClass: ["indicator"]
	  }), _vm._l((_vm.sliders[0].sliderPages), function(v) {
	    return _c('slider-page', {
	      key: v.index,
	      attrs: {
	        "items": v.items
	      }
	    })
	  })], 2)]), _c('panel', {
	    attrs: {
	      "title": "width & height",
	      "paddingBody": "0"
	    }
	  }, [_c('slider', {
	    staticClass: ["slider"],
	    appendAsTree: true,
	    attrs: {
	      "append": "tree",
	      "interval": _vm.sliders[0].interval,
	      "autoPlay": _vm.sliders[0].autoPlay
	    }
	  }, [_c('indicator', {
	    staticStyle: {
	      itemColor: "#dddddd",
	      width: "714px",
	      height: "460px"
	    }
	  }), _vm._l((_vm.sliders[0].sliderPages), function(v) {
	    return _c('slider-page', {
	      key: v.index,
	      attrs: {
	        "items": v.items
	      }
	    })
	  })], 2)]), _c('panel', {
	    attrs: {
	      "title": "left & top",
	      "paddingBody": "0"
	    }
	  }, [_c('slider', {
	    staticClass: ["slider"],
	    appendAsTree: true,
	    attrs: {
	      "append": "tree",
	      "interval": _vm.sliders[1].interval,
	      "autoPlay": _vm.sliders[1].autoPlay
	    }
	  }, [_c('indicator', {
	    staticClass: ["indicator"],
	    staticStyle: {
	      top: "-140px",
	      left: "-240px"
	    }
	  }), _vm._l((_vm.sliders[1].sliderPages), function(v, i) {
	    return _c('slider-page', {
	      key: i,
	      attrs: {
	        "items": v.items
	      }
	    })
	  })], 2)]), _c('panel', {
	    attrs: {
	      "title": "itemColor & itemSelectedColor",
	      "paddingBody": "0"
	    }
	  }, [_c('slider', {
	    staticClass: ["slider"],
	    appendAsTree: true,
	    attrs: {
	      "append": "tree",
	      "interval": _vm.sliders[2].interval,
	      "autoPlay": _vm.sliders[2].autoPlay
	    }
	  }, [_c('indicator', {
	    staticClass: ["indicator"],
	    staticStyle: {
	      itemSelectedColor: "rgb(217, 83, 79)"
	    }
	  }), _vm._l((_vm.sliders[2].sliderPages), function(v) {
	    return _c('slider-page', {
	      key: v.index,
	      attrs: {
	        "items": v.items
	      }
	    })
	  })], 2)]), _c('panel', {
	    attrs: {
	      "title": "itemSize",
	      "paddingBody": "0"
	    }
	  }, [_c('slider', {
	    staticClass: ["slider"],
	    appendAsTree: true,
	    attrs: {
	      "append": "tree",
	      "interval": _vm.sliders[1].interval,
	      "autoPlay": _vm.sliders[1].autoPlay
	    }
	  }, [_c('indicator', {
	    staticStyle: {
	      itemColor: "#dddddd",
	      itemSize: "40px",
	      top: "140px",
	      left: "180px",
	      width: "700px",
	      height: "380px"
	    }
	  }), _vm._l((_vm.sliders[1].sliderPages), function(v, i) {
	    return _c('slider-page', {
	      key: i,
	      attrs: {
	        "items": v.items
	      }
	    })
	  })], 2)])], 1)], 1)
	},staticRenderFns: []}
	module.exports.render._withStripped = true

/***/ })

/******/ });