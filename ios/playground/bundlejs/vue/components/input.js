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
	__vue_styles__.push(__webpack_require__(28)
	)

	/* script */
	__vue_exports__ = __webpack_require__(29)

	/* template */
	var __vue_template__ = __webpack_require__(30)
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
	__vue_options__.__file = "/Users/Hanks/Codes/work/incubator-weex/examples/vue/components/input.vue"
	__vue_options__.render = __vue_template__.render
	__vue_options__.staticRenderFns = __vue_template__.staticRenderFns
	__vue_options__._scopeId = "data-v-9b1cdcce"
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

/***/ 28:
/***/ (function(module, exports) {

	module.exports = {
	  "input": {
	    "fontSize": 60,
	    "height": 80,
	    "width": 750
	  },
	  "button": {
	    "fontSize": 36,
	    "width": 200,
	    "color": "#41B883",
	    "textAlign": "center",
	    "paddingTop": 10,
	    "paddingBottom": 10,
	    "borderWidth": 2,
	    "borderStyle": "solid",
	    "marginRight": 20,
	    "borderColor": "rgb(162,217,192)",
	    "backgroundColor": "rgba(162,217,192,0.2)"
	  }
	}

/***/ }),

/***/ 29:
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
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
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
	      txtInput: '',
	      txtChange: '',
	      txtReturnType: '',
	      txtSelection: '',
	      autofocus: false
	    };
	  },
	  methods: {
	    ready: function ready() {
	      var self = this;
	      setTimeout(function () {
	        self.autofocus = true;
	      }, 1000);
	    },
	    onchange: function onchange(event) {
	      this.txtChange = event.value;
	      console.log('onchange', event.value);
	    },
	    onreturn: function onreturn(event) {
	      this.txtReturnType = event.returnKeyType;
	      console.log('onreturn', event.type);
	    },
	    oninput: function oninput(event) {
	      this.txtInput = event.value;
	      console.log('oninput', event.value);
	    },
	    focus: function focus() {
	      this.$refs['input1'].focus();
	    },
	    blur: function blur() {
	      this.$refs['input1'].blur();
	    },
	    setRange: function setRange() {
	      console.log(this.$refs["inputselection"]);
	      this.$refs["inputselection"].setSelectionRange(2, 6);
	    },
	    getSelectionRange: function getSelectionRange() {
	      console.log(this.$refs["inputselection"]);
	      var self = this;
	      this.$refs["inputselection"].getSelectionRange(function (e) {
	        self.txtSelection = e.selectionStart + '-' + e.selectionEnd;
	      });
	    }
	  }
	};

/***/ }),

/***/ 30:
/***/ (function(module, exports) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', [_c('div', [_c('text', {
	    staticStyle: {
	      fontSize: "40px"
	    }
	  }, [_vm._v("oninput: " + _vm._s(_vm.txtInput))]), _c('text', {
	    staticStyle: {
	      fontSize: "40px"
	    }
	  }, [_vm._v("onchange: " + _vm._s(_vm.txtChange))]), _c('text', {
	    staticStyle: {
	      fontSize: "40px"
	    }
	  }, [_vm._v("onreturntype: " + _vm._s(_vm.txtReturnType))]), _c('text', {
	    staticStyle: {
	      fontSize: "40px"
	    }
	  }, [_vm._v("selection: " + _vm._s(_vm.txtSelection))])]), _c('scroller', [_c('div', [_vm._m(0), _c('input', {
	    staticClass: ["input"],
	    attrs: {
	      "type": "text",
	      "placeholder": "Input Text",
	      "autofocus": true,
	      "value": ""
	    },
	    on: {
	      "change": _vm.onchange,
	      "input": _vm.oninput
	    }
	  })]), _c('div', [_vm._m(1), _c('input', {
	    staticClass: ["input"],
	    attrs: {
	      "type": "password",
	      "placeholder": "Input Password"
	    },
	    on: {
	      "change": _vm.onchange,
	      "input": _vm.oninput
	    }
	  })]), _c('div', [_vm._m(2), _c('input', {
	    staticClass: ["input"],
	    attrs: {
	      "type": "url",
	      "placeholder": "Input URL"
	    },
	    on: {
	      "change": _vm.onchange,
	      "input": _vm.oninput
	    }
	  })]), _c('div', [_vm._m(3), _c('input', {
	    staticClass: ["input"],
	    attrs: {
	      "type": "email",
	      "placeholder": "Input Email"
	    },
	    on: {
	      "change": _vm.onchange,
	      "input": _vm.oninput
	    }
	  })]), _c('div', [_vm._m(4), _c('input', {
	    staticClass: ["input"],
	    attrs: {
	      "type": "tel",
	      "placeholder": "Input Tel"
	    },
	    on: {
	      "change": _vm.onchange,
	      "input": _vm.oninput
	    }
	  })]), _c('div', [_vm._m(5), _c('input', {
	    staticClass: ["input"],
	    attrs: {
	      "type": "time",
	      "placeholder": "Input Time"
	    },
	    on: {
	      "change": _vm.onchange,
	      "input": _vm.oninput
	    }
	  })]), _c('div', [_vm._m(6), _c('input', {
	    staticClass: ["input"],
	    attrs: {
	      "type": "number",
	      "placeholder": "Input number"
	    },
	    on: {
	      "change": _vm.onchange,
	      "input": _vm.oninput
	    }
	  })]), _c('div', [_vm._m(7), _c('input', {
	    staticClass: ["input"],
	    attrs: {
	      "type": "date",
	      "placeholder": "Input Date",
	      "max": "2017-12-12",
	      "min": "2015-01-01"
	    },
	    on: {
	      "change": _vm.onchange,
	      "input": _vm.oninput
	    }
	  })]), _c('div', [_vm._m(8), _c('input', {
	    staticClass: ["input"],
	    attrs: {
	      "type": "text",
	      "placeholder": "please input",
	      "returnKeyType": "default"
	    },
	    on: {
	      "change": _vm.onchange,
	      "return": _vm.onreturn,
	      "input": _vm.oninput
	    }
	  })]), _c('div', [_vm._m(9), _c('input', {
	    staticClass: ["input"],
	    attrs: {
	      "type": "text",
	      "placeholder": "please input",
	      "returnKeyType": "go"
	    },
	    on: {
	      "change": _vm.onchange,
	      "return": _vm.onreturn,
	      "input": _vm.oninput
	    }
	  })]), _c('div', [_vm._m(10), _c('input', {
	    staticClass: ["input"],
	    attrs: {
	      "type": "text",
	      "placeholder": "please input",
	      "returnKeyType": "next"
	    },
	    on: {
	      "change": _vm.onchange,
	      "return": _vm.onreturn,
	      "input": _vm.oninput
	    }
	  })]), _c('div', [_vm._m(11), _c('input', {
	    staticClass: ["input"],
	    attrs: {
	      "type": "text",
	      "placeholder": "please input",
	      "returnKeyType": "search"
	    },
	    on: {
	      "change": _vm.onchange,
	      "return": _vm.onreturn,
	      "input": _vm.oninput
	    }
	  })]), _c('div', [_vm._m(12), _c('input', {
	    staticClass: ["input"],
	    attrs: {
	      "type": "text",
	      "placeholder": "please input",
	      "returnKeyType": "send"
	    },
	    on: {
	      "change": _vm.onchange,
	      "return": _vm.onreturn,
	      "input": _vm.oninput
	    }
	  })]), _c('div', [_vm._m(13), _c('input', {
	    staticClass: ["input"],
	    attrs: {
	      "type": "text",
	      "placeholder": "please input",
	      "returnKeyType": "done"
	    },
	    on: {
	      "change": _vm.onchange,
	      "return": _vm.onreturn,
	      "input": _vm.oninput
	    }
	  })]), _c('div', [_vm._m(14), _c('div', {
	    staticStyle: {
	      flexDirection: "row",
	      marginBottom: "16px",
	      justifyContent: "space-between"
	    }
	  }, [_c('text', {
	    staticClass: ["button"],
	    attrs: {
	      "value": "Focus",
	      "type": "primary"
	    },
	    on: {
	      "click": _vm.focus
	    }
	  }), _c('text', {
	    staticClass: ["button"],
	    attrs: {
	      "value": "Blur",
	      "type": "primary"
	    },
	    on: {
	      "click": _vm.blur
	    }
	  })]), _c('input', {
	    ref: "input1",
	    staticClass: ["input"],
	    attrs: {
	      "type": "text",
	      "placeholder": "Input1",
	      "value": ""
	    }
	  })]), _c('div', [_vm._m(15), _c('div', {
	    staticStyle: {
	      flexDirection: "row",
	      marginBottom: "16px",
	      justifyContent: "space-between"
	    }
	  }, [_c('text', {
	    staticClass: ["button"],
	    attrs: {
	      "value": "setRange",
	      "type": "primary"
	    },
	    on: {
	      "click": _vm.setRange
	    }
	  }), _c('text', {
	    staticClass: ["button"],
	    attrs: {
	      "value": "getSelectionRange",
	      "type": "primary"
	    },
	    on: {
	      "click": _vm.getSelectionRange
	    }
	  })]), _c('input', {
	    ref: "inputselection",
	    staticClass: ["input"],
	    attrs: {
	      "type": "text",
	      "placeholder": "please input",
	      "value": "123456789"
	    },
	    on: {
	      "change": _vm.onchange,
	      "return": _vm.onreturn,
	      "input": _vm.oninput
	    }
	  })])])])
	},staticRenderFns: [function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', {
	    staticStyle: {
	      backgroundColor: "#286090"
	    }
	  }, [_c('text', {
	    staticClass: ["title"],
	    staticStyle: {
	      height: "80",
	      padding: "20",
	      color: "#FFFFFF"
	    }
	  }, [_vm._v("input type = text")])])
	},function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', {
	    staticStyle: {
	      backgroundColor: "#286090"
	    }
	  }, [_c('text', {
	    staticClass: ["title"],
	    staticStyle: {
	      height: "80",
	      padding: "20",
	      color: "#FFFFFF"
	    }
	  }, [_vm._v("input type = password")])])
	},function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', {
	    staticStyle: {
	      backgroundColor: "#286090"
	    }
	  }, [_c('text', {
	    staticClass: ["title"],
	    staticStyle: {
	      height: "80",
	      padding: "20",
	      color: "#FFFFFF"
	    }
	  }, [_vm._v("input type = url")])])
	},function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', {
	    staticStyle: {
	      backgroundColor: "#286090"
	    }
	  }, [_c('text', {
	    staticClass: ["title"],
	    staticStyle: {
	      height: "80",
	      padding: "20",
	      color: "#FFFFFF"
	    }
	  }, [_vm._v("input type = email")])])
	},function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', {
	    staticStyle: {
	      backgroundColor: "#286090"
	    }
	  }, [_c('text', {
	    staticClass: ["title"],
	    staticStyle: {
	      height: "80",
	      padding: "20",
	      color: "#FFFFFF"
	    }
	  }, [_vm._v("input type = tel")])])
	},function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', {
	    staticStyle: {
	      backgroundColor: "#286090"
	    }
	  }, [_c('text', {
	    staticClass: ["title"],
	    staticStyle: {
	      height: "80",
	      padding: "20",
	      color: "#FFFFFF"
	    }
	  }, [_vm._v("input type = time")])])
	},function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', {
	    staticStyle: {
	      backgroundColor: "#286090"
	    }
	  }, [_c('text', {
	    staticClass: ["title"],
	    staticStyle: {
	      height: "80",
	      padding: "20",
	      color: "#FFFFFF"
	    }
	  }, [_vm._v("input type = number")])])
	},function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', {
	    staticStyle: {
	      backgroundColor: "#286090"
	    }
	  }, [_c('text', {
	    staticClass: ["title"],
	    staticStyle: {
	      height: "80",
	      padding: "20",
	      color: "#FFFFFF"
	    }
	  }, [_vm._v("input type = date")])])
	},function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', {
	    staticStyle: {
	      backgroundColor: "#286090"
	    }
	  }, [_c('text', {
	    staticClass: ["title"],
	    staticStyle: {
	      height: "80",
	      padding: "20",
	      color: "#FFFFFF"
	    }
	  }, [_vm._v("input return-key-type = default")])])
	},function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', {
	    staticStyle: {
	      backgroundColor: "#286090"
	    }
	  }, [_c('text', {
	    staticClass: ["title"],
	    staticStyle: {
	      height: "80",
	      padding: "20",
	      color: "#FFFFFF"
	    }
	  }, [_vm._v("input return-key-type = go")])])
	},function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', {
	    staticStyle: {
	      backgroundColor: "#286090"
	    }
	  }, [_c('text', {
	    staticClass: ["title"],
	    staticStyle: {
	      height: "80",
	      padding: "20",
	      color: "#FFFFFF"
	    }
	  }, [_vm._v("input return-key-type = next")])])
	},function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', {
	    staticStyle: {
	      backgroundColor: "#286090"
	    }
	  }, [_c('text', {
	    staticClass: ["title"],
	    staticStyle: {
	      height: "80",
	      padding: "20",
	      color: "#FFFFFF"
	    }
	  }, [_vm._v("input return-key-type = search")])])
	},function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', {
	    staticStyle: {
	      backgroundColor: "#286090"
	    }
	  }, [_c('text', {
	    staticClass: ["title"],
	    staticStyle: {
	      height: "80",
	      padding: "20",
	      color: "#FFFFFF"
	    }
	  }, [_vm._v("input return-key-type = send")])])
	},function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', {
	    staticStyle: {
	      backgroundColor: "#286090"
	    }
	  }, [_c('text', {
	    staticClass: ["title"],
	    staticStyle: {
	      height: "80",
	      padding: "20",
	      color: "#FFFFFF"
	    }
	  }, [_vm._v("input return-key-type = done")])])
	},function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', {
	    staticStyle: {
	      backgroundColor: "#286090"
	    }
	  }, [_c('text', {
	    staticClass: ["title"],
	    staticStyle: {
	      height: "80",
	      padding: "20",
	      color: "#FFFFFF"
	    }
	  }, [_vm._v("function focus() & blur()")])])
	},function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', {
	    staticStyle: {
	      backgroundColor: "#286090"
	    }
	  }, [_c('text', {
	    staticClass: ["title"],
	    staticStyle: {
	      height: "80",
	      padding: "20",
	      color: "#FFFFFF"
	    }
	  }, [_vm._v("input selection")])])
	}]}
	module.exports.render._withStripped = true

/***/ })

/******/ });