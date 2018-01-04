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
	__vue_styles__.push(__webpack_require__(133)
	)

	/* script */
	__vue_exports__ = __webpack_require__(134)

	/* template */
	var __vue_template__ = __webpack_require__(135)
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
	__vue_options__.__file = "/Users/Hanks/Codes/work/incubator-weex/examples/vue/modules/websocket.vue"
	__vue_options__.render = __vue_template__.render
	__vue_options__.staticRenderFns = __vue_template__.staticRenderFns
	__vue_options__._scopeId = "data-v-35ca2732"
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

/***/ 133:
/***/ (function(module, exports) {

	module.exports = {
	  "input": {
	    "fontSize": 40,
	    "height": 80,
	    "width": 600
	  },
	  "button": {
	    "fontSize": 36,
	    "width": 150,
	    "color": "#41B883",
	    "textAlign": "center",
	    "paddingTop": 25,
	    "paddingBottom": 25,
	    "borderWidth": 2,
	    "borderStyle": "solid",
	    "marginRight": 20,
	    "borderColor": "rgb(162,217,192)",
	    "backgroundColor": "rgba(162,217,192,0.2)"
	  }
	}

/***/ }),

/***/ 134:
/***/ (function(module, exports) {

	'use strict';

	Object.defineProperty(exports, "__esModule", {
	  value: true
	});

	var _typeof = typeof Symbol === "function" && typeof Symbol.iterator === "symbol" ? function (obj) { return typeof obj; } : function (obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; };

	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//

	var websocket = weex.requireModule('webSocket');
	exports.default = {
	  data: function data() {
	    return {
	      connectinfo: '',
	      sendinfo: '',
	      onopeninfo: '',
	      onmessage: '',
	      oncloseinfo: '',
	      onerrorinfo: '',
	      closeinfo: '',
	      txtInput: '',
	      navBarHeight: 88,
	      title: 'Navigator',
	      dir: 'examples',
	      baseURL: ''
	    };
	  },

	  methods: {

	    connect: function connect() {
	      websocket.WebSocket('ws://echo.websocket.org', '');
	      var self = this;
	      self.onopeninfo = 'connecting...';
	      websocket.onopen = function (e) {
	        self.onopeninfo = 'websocket open';
	      };
	      websocket.onmessage = function (e) {
	        console.log(_typeof(e.data));
	        if (typeof e.data === 'string') {
	          self.onmessage = e.data;
	        } else {
	          var str = 'receive array buffer show with string:' + String.fromCharCode.apply(null, new Float32Array(e.data));
	          self.onmessage = str;
	        }
	      };
	      websocket.onerror = function (e) {
	        self.onerrorinfo = e.data;
	      };
	      websocket.onclose = function (e) {
	        self.onopeninfo = '';
	        self.oncloseinfo = e.code;
	      };
	    },
	    send: function send(e) {
	      var input = this.$refs.input;
	      input.blur();
	      websocket.send(this.txtInput);
	      this.sendinfo = this.txtInput;
	    },
	    sendArrayBuffer: function sendArrayBuffer(e) {
	      var input = this.$refs.input;
	      input.blur();
	      var buffer = new ArrayBuffer(16);
	      var view = new Float32Array(buffer);
	      view.set([4, 89, 36.9, 0.765]);
	      console.log(buffer);
	      var str = 'send array buffer show with string:' + String.fromCharCode.apply(null, new Float32Array(buffer));
	      this.sendinfo = str;
	      websocket.send(buffer);
	    },
	    oninput: function oninput(event) {
	      this.txtInput = event.value;
	    },
	    close: function close(e) {
	      this.closeinfo = 'close connect';
	      websocket.close();
	    }
	  }
	};

/***/ }),

/***/ 135:
/***/ (function(module, exports) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('scroller', [_c('div', [_vm._m(0), _c('input', {
	    ref: "input",
	    staticClass: ["input"],
	    attrs: {
	      "type": "text",
	      "placeholder": "please input message to send",
	      "autofocus": "false",
	      "value": ""
	    },
	    on: {
	      "input": _vm.oninput
	    }
	  }), _c('div', {
	    staticStyle: {
	      flexDirection: "row",
	      justifyContent: "center"
	    }
	  }, [_c('text', {
	    staticClass: ["button"],
	    on: {
	      "click": _vm.connect
	    }
	  }, [_vm._v("connect")]), _c('text', {
	    staticClass: ["button"],
	    on: {
	      "click": _vm.send
	    }
	  }, [_vm._v("send")]), _c('text', {
	    staticClass: ["button"],
	    on: {
	      "click": _vm.sendArrayBuffer
	    }
	  }, [_vm._v("sendArrayBuffer")]), _c('text', {
	    staticClass: ["button"],
	    on: {
	      "click": _vm.close
	    }
	  }, [_vm._v("close")])]), _vm._m(1), _c('text', {
	    staticStyle: {
	      color: "black",
	      height: "80px"
	    }
	  }, [_vm._v(_vm._s(_vm.closeinfo))]), _vm._m(2), _c('text', {
	    staticStyle: {
	      color: "black",
	      height: "80px"
	    }
	  }, [_vm._v(_vm._s(_vm.sendinfo))]), _vm._m(3), _c('text', {
	    staticStyle: {
	      color: "black",
	      height: "80px"
	    }
	  }, [_vm._v(_vm._s(_vm.onopeninfo))]), _vm._m(4), _c('text', {
	    staticStyle: {
	      color: "black",
	      height: "400px"
	    }
	  }, [_vm._v(_vm._s(_vm.onmessage))]), _vm._m(5), _c('text', {
	    staticStyle: {
	      color: "black",
	      height: "80px"
	    }
	  }, [_vm._v(_vm._s(_vm.oncloseinfo))]), _vm._m(6), _c('text', {
	    staticStyle: {
	      color: "black",
	      height: "80px"
	    }
	  }, [_vm._v(_vm._s(_vm.onerrorinfo))])])])
	},staticRenderFns: [function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', {
	    staticStyle: {
	      backgroundColor: "#286090"
	    }
	  }, [_c('text', {
	    staticClass: ["title"],
	    staticStyle: {
	      height: "80px",
	      padding: "20px",
	      color: "white"
	    }
	  }, [_vm._v("websocket")])])
	},function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', {
	    staticStyle: {
	      backgroundColor: "lightgray"
	    }
	  }, [_c('text', {
	    staticClass: ["title"],
	    staticStyle: {
	      height: "80px",
	      padding: "20px",
	      color: "black"
	    }
	  }, [_vm._v("method = close")])])
	},function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', {
	    staticStyle: {
	      backgroundColor: "lightgray"
	    }
	  }, [_c('text', {
	    staticClass: ["title"],
	    staticStyle: {
	      height: "80px",
	      padding: "20px",
	      color: "black"
	    }
	  }, [_vm._v("method = send")])])
	},function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', {
	    staticStyle: {
	      backgroundColor: "lightgray"
	    }
	  }, [_c('text', {
	    staticClass: ["title"],
	    staticStyle: {
	      height: "80px",
	      padding: "20px",
	      color: "black"
	    }
	  }, [_vm._v("method = onopen")])])
	},function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', {
	    staticStyle: {
	      backgroundColor: "lightgray"
	    }
	  }, [_c('text', {
	    staticClass: ["title"],
	    staticStyle: {
	      height: "80px",
	      padding: "20px",
	      color: "black"
	    }
	  }, [_vm._v("method = onmessage")])])
	},function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', {
	    staticStyle: {
	      backgroundColor: "lightgray"
	    }
	  }, [_c('text', {
	    staticClass: ["title"],
	    staticStyle: {
	      height: "80px",
	      padding: "20px",
	      color: "black"
	    }
	  }, [_vm._v("method = onclose")])])
	},function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', {
	    staticStyle: {
	      backgroundColor: "lightgray"
	    }
	  }, [_c('text', {
	    staticClass: ["title"],
	    staticStyle: {
	      height: "80px",
	      padding: "20px",
	      color: "black"
	    }
	  }, [_vm._v("method = onerror")])])
	}]}
	module.exports.render._withStripped = true

/***/ })

/******/ });