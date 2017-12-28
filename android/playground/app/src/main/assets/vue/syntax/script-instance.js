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
	__vue_styles__.push(__webpack_require__(243)
	)

	/* script */
	__vue_exports__ = __webpack_require__(244)

	/* template */
	var __vue_template__ = __webpack_require__(245)
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
	__vue_options__.__file = "/Users/Hanks/Codes/work/incubator-weex/examples/vue/syntax/script-instance.vue"
	__vue_options__.render = __vue_template__.render
	__vue_options__.staticRenderFns = __vue_template__.staticRenderFns
	__vue_options__._scopeId = "data-v-6937c633"
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

/***/ 243:
/***/ (function(module, exports) {

	module.exports = {
	  "title": {
	    "fontSize": 48
	  }
	}

/***/ }),

/***/ 244:
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

	module.exports = {
	  data: function data() {
	    return {
	      x: 1,
	      y: 2
	    };
	  },
	  methods: {
	    foo: function foo() {
	      console.log('foo');
	    },
	    test: function test() {

	      // access data/props
	      console.log(this.x);

	      // access computed
	      console.log(this.z);

	      // call methods
	      this.foo();

	      // get config
	      console.log(this.$getConfig());

	      // listen and fire component events
	      this.$emit('custom');
	      this.$on('custom', this.foo);
	      this.$off('custom', this.foo);

	      // // parent component and sub components
	      // this.$parent.$emit('custom')
	      // this.$refs['sub'].$emit('custom')

	      // // find element
	      // var dom = weex.requireModule('dom')
	      // dom.scrollToElement(this.$el('title'), {offset: 0})
	    }
	  },
	  computed: {
	    z: function z() {
	      return this.x + this.y;
	    }
	  },
	  mounted: function mounted() {
	    this.test();
	  }
	};

/***/ }),

/***/ 245:
/***/ (function(module, exports) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _vm._m(0)
	},staticRenderFns: [function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', [_c('text', {
	    staticClass: ["title"],
	    attrs: {
	      "id": "title"
	    }
	  }, [_vm._v("Please check out the source code.")])])
	}]}
	module.exports.render._withStripped = true

/***/ })

/******/ });