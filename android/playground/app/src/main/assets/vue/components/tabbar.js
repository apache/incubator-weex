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

	/* script */
	__vue_exports__ = __webpack_require__(68)

	/* template */
	var __vue_template__ = __webpack_require__(77)
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
	__vue_options__.__file = "/Users/Hanks/Codes/work/incubator-weex/examples/vue/components/tabbar.vue"
	__vue_options__.render = __vue_template__.render
	__vue_options__.staticRenderFns = __vue_template__.staticRenderFns
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

/***/ 41:
/***/ (function(module, exports) {

	/*
	 * Licensed to the Apache Software Foundation (ASF) under one
	 * or more contributor license agreements.  See the NOTICE file
	 * distributed with this work for additional information
	 * regarding copyright ownership.  The ASF licenses this file
	 * to you under the Apache License, Version 2.0 (the
	 * "License"); you may not use this file except in compliance
	 * with the License.  You may obtain a copy of the License at
	 *
	 *   http://www.apache.org/licenses/LICENSE-2.0
	 *
	 * Unless required by applicable law or agreed to in writing,
	 * software distributed under the License is distributed on an
	 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
	 * KIND, either express or implied.  See the License for the
	 * specific language governing permissions and limitations
	 * under the License.
	 */
	exports.getBaseURL = function (vm) {
	  var bundleUrl = weex.config.bundleUrl;
	  var nativeBase;
	  var isAndroidAssets = bundleUrl.indexOf('your_current_IP') >= 0 || bundleUrl.indexOf('file://assets/')>=0;
	  var isiOSAssets = bundleUrl.indexOf('file:///') >= 0 && bundleUrl.indexOf('WeexDemo.app') > 0;
	  if (isAndroidAssets) {
	    nativeBase = 'file://assets/';
	  }
	  else if (isiOSAssets) {
	    // file:///var/mobile/Containers/Bundle/Application/{id}/WeexDemo.app/
	    // file:///Users/{user}/Library/Developer/CoreSimulator/Devices/{id}/data/Containers/Bundle/Application/{id}/WeexDemo.app/
	    nativeBase = bundleUrl.substring(0, bundleUrl.lastIndexOf('/') + 1);
	  }
	  else {
	    var host = 'localhost:12580';
	    var matches = /\/\/([^\/]+?)\//.exec(weex.config.bundleUrl);
	    if (matches && matches.length >= 2) {
	      host = matches[1];
	    }
	    nativeBase = 'http://' + host + '/' + vm.dir + '/build/';
	  }
	  var h5Base = './vue.html?page=./' + vm.dir + '/build/';
	  // in Native
	  var base = nativeBase;
	  if (typeof window === 'object') {
	    // in Browser or WebView
	    base = h5Base;
	  }
	  return base
	}


/***/ }),

/***/ 68:
/***/ (function(module, exports, __webpack_require__) {

	'use strict';

	//
	//
	//
	//
	//
	//

	var getBaseURL = __webpack_require__(41).getBaseURL;
	module.exports = {
	  data: function data() {
	    return {
	      dir: 'examples',
	      tabItems: [{
	        index: 0,
	        title: 'tab1',
	        titleColor: '#000000',
	        icon: '',
	        image: 'http://gtms01.alicdn.com/tps/i1/TB1qw.hMpXXXXagXXXX9t7RGVXX-46-46.png',
	        selectedImage: 'http://gtms04.alicdn.com/tps/i4/TB16jjPMpXXXXazXVXX9t7RGVXX-46-46.png',
	        src: 'component/tabbar/tabbar-item.js?itemId=tab1',
	        visibility: 'visible'
	      }, {
	        index: 1,
	        title: 'tab2',
	        titleColor: '#000000',
	        icon: '',
	        image: 'http://gtms03.alicdn.com/tps/i3/TB1LEn9MpXXXXaUXpXX9t7RGVXX-46-46.png',
	        selectedImage: 'http://gtms02.alicdn.com/tps/i2/TB1qysbMpXXXXcnXXXX9t7RGVXX-46-46.png',
	        src: 'component/tabbar/tabbar-item.js?itemId=tab2',
	        visibility: 'hidden'
	      }, {
	        index: 2,
	        title: 'tab3',
	        titleColor: '#000000',
	        icon: '',
	        image: 'http://gtms01.alicdn.com/tps/i1/TB1B0v5MpXXXXcvXpXX9t7RGVXX-46-46.png',
	        selectedImage: 'http://gtms04.alicdn.com/tps/i4/TB1NxY5MpXXXXcrXpXX9t7RGVXX-46-46.png',
	        src: 'component/tabbar/tabbar-item.js?itemId=tab3',
	        visibility: 'hidden'
	      }]
	    };
	  },
	  components: {
	    tabbar: __webpack_require__(69)
	  },
	  created: function created() {
	    var baseURL = getBaseURL(this);
	    for (var i = 0; i < this.tabItems.length; i++) {
	      var tabItem = this.tabItems[i];
	      tabItem.src = baseURL + tabItem.src;
	    }
	  },
	  methods: {
	    tabBarOnClick: function tabBarOnClick(e) {
	      console.log('tabBarOnClick', e.index);
	    }
	  }
	};

/***/ }),

/***/ 69:
/***/ (function(module, exports, __webpack_require__) {

	var __vue_exports__, __vue_options__
	var __vue_styles__ = []

	/* styles */
	__vue_styles__.push(__webpack_require__(70)
	)

	/* script */
	__vue_exports__ = __webpack_require__(71)

	/* template */
	var __vue_template__ = __webpack_require__(76)
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
	__vue_options__.__file = "/Users/Hanks/Codes/work/incubator-weex/examples/vue/include/tabbar.vue"
	__vue_options__.render = __vue_template__.render
	__vue_options__.staticRenderFns = __vue_template__.staticRenderFns
	__vue_options__._scopeId = "data-v-285878a6"
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

/***/ 70:
/***/ (function(module, exports) {

	module.exports = {
	  "wrapper": {
	    "width": 750,
	    "position": "absolute",
	    "top": 0,
	    "left": 0,
	    "right": 0,
	    "bottom": 0
	  },
	  "content": {
	    "position": "absolute",
	    "top": 0,
	    "left": 0,
	    "right": 0,
	    "bottom": 0,
	    "marginTop": 0,
	    "marginBottom": 88
	  },
	  "tabbar": {
	    "flexDirection": "row",
	    "position": "fixed",
	    "bottom": 0,
	    "left": 0,
	    "right": 0,
	    "height": 88
	  }
	}

/***/ }),

/***/ 71:
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

	module.exports = {
	  props: {
	    tabItems: { default: [] },
	    selectedColor: { default: '#ff0000' },
	    unselectedColor: { default: '#000000' }
	  },
	  data: function data() {
	    return {
	      selectedIndex: 0
	    };
	  },
	  components: {
	    tabitem: __webpack_require__(72)
	  },
	  created: function created() {
	    this.select(this.selectedIndex);
	  },
	  methods: {
	    tabItemOnClick: function tabItemOnClick(e) {
	      this.selectedIndex = e.index;
	      this.select(e.index);
	      this.$emit('tabBarOnClick', e);
	    },
	    select: function select(index) {
	      for (var i = 0; i < this.tabItems.length; i++) {
	        var tabItem = this.tabItems[i];
	        if (i == index) {
	          tabItem.icon = tabItem.selectedImage;
	          tabItem.titleColor = this.selectedColor;
	          tabItem.visibility = 'visible';
	        } else {
	          tabItem.icon = tabItem.image;
	          tabItem.titleColor = this.unselectedColor;
	          tabItem.visibility = 'hidden';
	        }
	      }
	    }
	  }
	};

/***/ }),

/***/ 72:
/***/ (function(module, exports, __webpack_require__) {

	var __vue_exports__, __vue_options__
	var __vue_styles__ = []

	/* styles */
	__vue_styles__.push(__webpack_require__(73)
	)

	/* script */
	__vue_exports__ = __webpack_require__(74)

	/* template */
	var __vue_template__ = __webpack_require__(75)
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
	__vue_options__.__file = "/Users/Hanks/Codes/work/incubator-weex/examples/vue/include/tabitem.vue"
	__vue_options__.render = __vue_template__.render
	__vue_options__.staticRenderFns = __vue_template__.staticRenderFns
	__vue_options__._scopeId = "data-v-400b65e9"
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

/***/ 73:
/***/ (function(module, exports) {

	module.exports = {
	  "container": {
	    "flex": 1,
	    "flexDirection": "column",
	    "alignItems": "center",
	    "justifyContent": "center",
	    "height": 88
	  },
	  "top-line": {
	    "position": "absolute",
	    "top": 0,
	    "left": 0,
	    "right": 0,
	    "height": 2
	  },
	  "tab-icon": {
	    "marginTop": 5,
	    "width": 40,
	    "height": 40
	  },
	  "tab-text": {
	    "marginTop": 5,
	    "textAlign": "center",
	    "fontSize": 20
	  }
	}

/***/ }),

/***/ 74:
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

	module.exports = {
	  props: {
	    index: { default: 0 },
	    title: { default: '' },
	    titleColor: { default: '#000000' },
	    icon: { default: '' },
	    backgroundColor: { default: '#ffffff' }
	  },
	  methods: {
	    onclickitem: function onclickitem(e) {
	      var params = {
	        index: this.index
	      };
	      this.$emit('tabItemOnClick', params);
	    }
	  }
	};

/***/ }),

/***/ 75:
/***/ (function(module, exports) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', {
	    staticClass: ["container"],
	    style: {
	      backgroundColor: _vm.backgroundColor
	    },
	    on: {
	      "click": _vm.onclickitem
	    }
	  }, [_c('image', {
	    staticClass: ["top-line"],
	    attrs: {
	      "src": "http://gtms03.alicdn.com/tps/i3/TB1mdsiMpXXXXXpXXXXNw4JIXXX-640-4.png"
	    }
	  }), _c('image', {
	    staticClass: ["tab-icon"],
	    attrs: {
	      "src": _vm.icon
	    }
	  }), _c('text', {
	    staticClass: ["tab-text"],
	    style: {
	      color: _vm.titleColor
	    }
	  }, [_vm._v(_vm._s(_vm.title))])])
	},staticRenderFns: []}
	module.exports.render._withStripped = true

/***/ }),

/***/ 76:
/***/ (function(module, exports) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', {
	    staticClass: ["wrapper"]
	  }, [_vm._l((_vm.tabItems), function(item, i) {
	    return _c('embed', {
	      key: i,
	      staticClass: ["content"],
	      style: {
	        visibility: item.visibility
	      },
	      attrs: {
	        "src": item.src,
	        "type": "weex"
	      }
	    })
	  }), _c('div', {
	    staticClass: ["tabbar"],
	    appendAsTree: true,
	    attrs: {
	      "append": "tree"
	    }
	  }, _vm._l((_vm.tabItems), function(item) {
	    return _c('tabitem', {
	      key: item.index,
	      attrs: {
	        "index": item.index,
	        "icon": item.icon,
	        "title": item.title,
	        "titleColor": item.titleColor
	      },
	      on: {
	        "tabItemOnClick": _vm.tabItemOnClick
	      }
	    })
	  }))], 2)
	},staticRenderFns: []}
	module.exports.render._withStripped = true

/***/ }),

/***/ 77:
/***/ (function(module, exports) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', {
	    staticStyle: {
	      flexDirection: "column"
	    }
	  }, [_c('tabbar', {
	    attrs: {
	      "tabItems": _vm.tabItems
	    },
	    on: {
	      "tabBarOnClick": _vm.tabBarOnClick
	    }
	  })])
	},staticRenderFns: []}
	module.exports.render._withStripped = true

/***/ })

/******/ });