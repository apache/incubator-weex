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
	__vue_styles__.push(__webpack_require__(94)
	)

	/* script */
	__vue_exports__ = __webpack_require__(95)

	/* template */
	var __vue_template__ = __webpack_require__(96)
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
	__vue_options__.__file = "/Users/Hanks/Codes/work/incubator-weex/examples/vue/iconfont.vue"
	__vue_options__.render = __vue_template__.render
	__vue_options__.staticRenderFns = __vue_template__.staticRenderFns
	__vue_options__._scopeId = "data-v-f9298a60"
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

/***/ 94:
/***/ (function(module, exports) {

	module.exports = {
	  "title1": {
	    "color": "#FF0000",
	    "fontSize": 36,
	    "fontFamily": "iconfont1"
	  },
	  "title2": {
	    "color": "#808080",
	    "fontSize": 36,
	    "fontFamily": "iconfont2"
	  },
	  "title3": {
	    "color": "#FFC0CB",
	    "fontSize": 36,
	    "fontFamily": "iconfont3"
	  },
	  "title4": {
	    "color": "#FF0000",
	    "fontSize": 36,
	    "fontFamily": "iconfont4"
	  }
	}

/***/ }),

/***/ 95:
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
	//
	//
	//
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
	  created: function created() {

	    var domModule = weex.requireModule('dom');
	    //目前支持ttf、woff文件，不支持svg、eot类型,moreItem at http://www.iconfont.cn/

	    domModule.addRule('fontFace', {
	      'fontFamily': "iconfont2",
	      'src': "url('http://at.alicdn.com/t/font_1469606063_76593.ttf')"
	    });
	    domModule.addRule('fontFace', {
	      'fontFamily': "iconfont3",
	      'src': "url('http://at.alicdn.com/t/font_1469606522_9417143.woff')"
	    });

	    domModule.addRule('fontFace', {
	      'fontFamily': "iconfont4",
	      'src': "url('http://at.alicdn.com/t/font_zn5b3jswpofuhaor.ttf')"
	    });

	    // you can use the absolute path or the local:// scheme path
	    //  domModule.addRule('fontFace', {
	    //   'fontFamily': "iconfont4",
	    //   'src': "url('file:///storage/emulated/0/Android/data/com.alibaba.weex/cache/http:__at.alicdn.com_t_font_1469606063_76593.ttf')"
	    // });
	  }
	};

/***/ }),

/***/ 96:
/***/ (function(module, exports) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _vm._m(0)
	},staticRenderFns: [function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('scroller', {
	    staticStyle: {
	      alignItems: "center"
	    }
	  }, [_c('div', {
	    staticStyle: {
	      marginTop: "20px"
	    }
	  }, [_c('text', {
	    staticStyle: {
	      color: "red",
	      fontSize: "50px"
	    }
	  }, [_vm._v("only support font for ttf and woff")])]), _c('text', {
	    staticClass: ["title2"],
	    staticStyle: {
	      marginTop: "50px",
	      width: "500px"
	    }
	  }, [_vm._v("http ttf: ")]), _c('text', {
	    staticClass: ["title3"],
	    staticStyle: {
	      marginTop: "50px",
	      width: "500px"
	    }
	  }, [_vm._v("http woff: ")]), _c('div', {
	    staticStyle: {
	      flexDirection: "row",
	      marginTop: "50px"
	    }
	  }, [_c('text', {
	    staticStyle: {
	      fontFamily: "iconfont4",
	      fontSize: "50",
	      color: "green"
	    }
	  }, [_vm._v("")]), _c('text', {
	    staticStyle: {
	      fontFamily: "iconfont4",
	      fontSize: "50"
	    }
	  }, [_vm._v("")]), _c('text', {
	    staticStyle: {
	      fontFamily: "iconfont4",
	      fontSize: "60",
	      color: "blue"
	    }
	  }, [_vm._v("")]), _c('text', {
	    staticStyle: {
	      fontFamily: "iconfont4",
	      fontSize: "60",
	      color: "green"
	    }
	  }, [_vm._v("")])]), _c('div', {
	    staticStyle: {
	      flexDirection: "row",
	      marginTop: "50px"
	    }
	  }, [_c('text', {
	    staticStyle: {
	      fontFamily: "iconfont4",
	      fontSize: "50",
	      color: "green"
	    }
	  }, [_vm._v("")]), _c('text', {
	    staticStyle: {
	      fontFamily: "iconfont4",
	      fontSize: "50"
	    }
	  }, [_vm._v("")]), _c('text', {
	    staticStyle: {
	      fontFamily: "iconfont4",
	      fontSize: "60",
	      color: "blue"
	    }
	  }, [_vm._v("")]), _c('text', {
	    staticStyle: {
	      fontFamily: "iconfont4",
	      fontSize: "60",
	      color: "green"
	    }
	  }, [_vm._v("")])]), _c('div', [_c('text', {
	    staticStyle: {
	      fontFamily: "iconfont4",
	      fontSize: "100",
	      marginTop: "50px"
	    }
	  }, [_vm._v("")])]), _c('div', [_c('text', {
	    staticStyle: {
	      fontFamily: "iconfont4",
	      fontSize: "100",
	      color: "green",
	      marginTop: "50px"
	    }
	  }, [_vm._v("")]), _c('text', {
	    staticStyle: {
	      fontFamily: "iconfont4",
	      fontSize: "100",
	      marginTop: "50px"
	    }
	  })]), _c('div', {
	    staticStyle: {
	      flexDirection: "row"
	    }
	  }, [_c('text', {
	    staticStyle: {
	      fontFamily: "iconfont4",
	      fontSize: "70",
	      width: "750px"
	    }
	  }, [_vm._v("")])])])
	}]}
	module.exports.render._withStripped = true

/***/ })

/******/ });