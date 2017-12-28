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
	__vue_styles__.push(__webpack_require__(136)
	)

	/* script */
	__vue_exports__ = __webpack_require__(137)

	/* template */
	var __vue_template__ = __webpack_require__(138)
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
	__vue_options__.__file = "/Users/Hanks/Codes/work/incubator-weex/examples/vue/showcase/a-node-click.vue"
	__vue_options__.render = __vue_template__.render
	__vue_options__.staticRenderFns = __vue_template__.staticRenderFns
	__vue_options__._scopeId = "data-v-07076933"
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

/***/ 136:
/***/ (function(module, exports) {

	module.exports = {
	  "list": {
	    "height": 2000
	  },
	  "cell": {
	    "height": 400,
	    "backgroundColor": "#0000FF"
	  },
	  "txt": {
	    "fontSize": 64,
	    "height": 84,
	    "lineHeight": 84,
	    "paddingTop": 10,
	    "paddingRight": 10,
	    "paddingBottom": 10,
	    "paddingLeft": 10
	  },
	  "ct": {
	    "width": 200,
	    "height": 200,
	    "backgroundColor": "#ffffff"
	  }
	}

/***/ }),

/***/ 137:
/***/ (function(module, exports) {

	'use strict';

	Object.defineProperty(exports, "__esModule", {
	  value: true
	});
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//

	exports.default = {
	  data: function data() {
	    return {
	      // url: '//m.taobao.com'
	      url: '#'
	    };
	  },

	  components: {
	    xlink: {
	      props: {
	        href: String
	      },
	      render: function render(createElement) {
	        return createElement('a', {
	          ref: 'link',
	          attrs: {
	            'component-name': 'link',
	            'href': this.href
	          }
	        }, this.$slots.default);
	      }
	    }
	  },
	  methods: {
	    click: function click(evt) {
	      evt.preventDefault();
	      // var d = Date.now()
	      // while (true) {
	      //   var now = Date.now()
	      //   if (now - d > 2000) { break }
	      // }
	      // throw new Error('test')
	      console.log('click on inner');
	    },
	    clickCell: function clickCell(evt) {
	      console.log('click on cell');
	    }
	  }
	};

/***/ }),

/***/ 138:
/***/ (function(module, exports) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('list', {
	    staticClass: ["list"]
	  }, [_c('cell', {
	    staticClass: ["cell"],
	    appendAsTree: true,
	    attrs: {
	      "append": "tree"
	    },
	    on: {
	      "click": _vm.clickCell
	    }
	  }, [_c('xlink', {
	    staticStyle: {
	      width: "400px",
	      height: "400px",
	      backgroundColor: "yellow"
	    },
	    attrs: {
	      "target": "_blank",
	      "href": _vm.url
	    }
	  }, [_c('div', {
	    staticClass: ["ct"],
	    on: {
	      "click": _vm.click
	    }
	  }, [_c('text', [_vm._v("CLICK ME")])])])], 1)])
	},staticRenderFns: []}
	module.exports.render._withStripped = true

/***/ })

/******/ });