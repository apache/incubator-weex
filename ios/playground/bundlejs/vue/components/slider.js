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
	__vue_styles__.push(__webpack_require__(450)
	)

	/* script */
	__vue_exports__ = __webpack_require__(451)

	/* template */
	var __vue_template__ = __webpack_require__(460)
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
	__vue_options__.__file = "/Users/bobning/work/source/apache-incubator-weex/examples/vue/components/slider.vue"
	__vue_options__.render = __vue_template__.render
	__vue_options__.staticRenderFns = __vue_template__.staticRenderFns
	__vue_options__._scopeId = "data-v-5e494ae2"
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

/***/ 399:
/***/ function(module, exports, __webpack_require__) {

	var __vue_exports__, __vue_options__
	var __vue_styles__ = []

	/* styles */
	__vue_styles__.push(__webpack_require__(400)
	)

	/* script */
	__vue_exports__ = __webpack_require__(401)

	/* template */
	var __vue_template__ = __webpack_require__(402)
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
	__vue_options__.__file = "/Users/bobning/work/source/apache-incubator-weex/examples/vue/include/panel.vue"
	__vue_options__.render = __vue_template__.render
	__vue_options__.staticRenderFns = __vue_template__.staticRenderFns
	__vue_options__._scopeId = "data-v-46407d45"
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


/***/ },

/***/ 400:
/***/ function(module, exports) {

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
	  }
	}

/***/ },

/***/ 401:
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

/***/ },

/***/ 402:
/***/ function(module, exports) {

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

/***/ },

/***/ 450:
/***/ function(module, exports) {

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
	    "itemSelectedColor": "rgb(40, 96, 144)"
	  }
	}

/***/ },

/***/ 451:
/***/ function(module, exports, __webpack_require__) {

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
	    panel: __webpack_require__(399),
	    sliderPage: __webpack_require__(452)
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

/***/ },

/***/ 452:
/***/ function(module, exports, __webpack_require__) {

	var __vue_exports__, __vue_options__
	var __vue_styles__ = []

	/* styles */
	__vue_styles__.push(__webpack_require__(453)
	)

	/* script */
	__vue_exports__ = __webpack_require__(454)

	/* template */
	var __vue_template__ = __webpack_require__(459)
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
	__vue_options__.__file = "/Users/bobning/work/source/apache-incubator-weex/examples/vue/include/slider-page.vue"
	__vue_options__.render = __vue_template__.render
	__vue_options__.staticRenderFns = __vue_template__.staticRenderFns
	__vue_options__._scopeId = "data-v-8d84a608"
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


/***/ },

/***/ 453:
/***/ function(module, exports) {

	module.exports = {
	  "slider-page": {
	    "flexDirection": "row",
	    "justifyContent": "space-between",
	    "width": 714,
	    "height": 420
	  }
	}

/***/ },

/***/ 454:
/***/ function(module, exports, __webpack_require__) {

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
	    sliderItem: __webpack_require__(455)
	  }
	};

/***/ },

/***/ 455:
/***/ function(module, exports, __webpack_require__) {

	var __vue_exports__, __vue_options__
	var __vue_styles__ = []

	/* styles */
	__vue_styles__.push(__webpack_require__(456)
	)

	/* script */
	__vue_exports__ = __webpack_require__(457)

	/* template */
	var __vue_template__ = __webpack_require__(458)
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
	__vue_options__.__file = "/Users/bobning/work/source/apache-incubator-weex/examples/vue/include/slider-item.vue"
	__vue_options__.render = __vue_template__.render
	__vue_options__.staticRenderFns = __vue_template__.staticRenderFns
	__vue_options__._scopeId = "data-v-679a3900"
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


/***/ },

/***/ 456:
/***/ function(module, exports) {

	module.exports = {
	  "slider-item": {
	    "width": 348,
	    "height": 400
	  }
	}

/***/ },

/***/ 457:
/***/ function(module, exports) {

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

/***/ },

/***/ 458:
/***/ function(module, exports) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('image', {
	    staticClass: ["slider-item"],
	    attrs: {
	      "src": _vm.image
	    }
	  })
	},staticRenderFns: []}
	module.exports.render._withStripped = true

/***/ },

/***/ 459:
/***/ function(module, exports) {

	module.exports={render:function(){},staticRenderFns:[]}

/***/ },

/***/ 460:
/***/ function(module, exports) {

	module.exports={render:function(){},staticRenderFns:[]}

/***/ }

/******/ });