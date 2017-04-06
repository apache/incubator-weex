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

	/* script */
	__vue_exports__ = __webpack_require__(508)

	/* template */
	var __vue_template__ = __webpack_require__(509)
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
	__vue_options__.__file = "/Users/bobning/work/source/apache-incubator-weex/examples/vue/modules/stream.vue"
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

/***/ 508:
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

	var stream = weex.requireModule('stream');
	module.exports = {
	  data: function data() {
	    return {
	      getJsonpResult: 'loading...',
	      getResult: 'loading...',
	      postResult: 'loading...',
	      putResult: 'loading...',
	      deleteResult: 'loading...',
	      headResult: 'loading...',
	      patchResult: 'loading...'
	    };
	  },
	  components: {
	    panel: __webpack_require__(399)
	  },
	  created: function created() {
	    var me = this;
	    var GET_URL_JSONP = 'http://jsfiddle.net/echo/jsonp/?callback=anything&result=content_in_response';
	    var GET_URL = 'http://httpbin.org/get';
	    var POST_URL = 'http://httpbin.org/post';
	    var PUT_URL = 'http://httpbin.org/put';
	    var DELETE_URL = 'http://httpbin.org/delete';
	    var HEAD_URL = 'http://httpbin.org/status/418';
	    var PATCH_URL = 'http://httpbin.org/patch';

	    stream.fetch({
	      method: 'GET',
	      url: GET_URL_JSONP,
	      type: 'jsonp'
	    }, function (ret) {
	      if (!ret.ok) {
	        me.getJsonpResult = "request failed";
	      } else {
	        console.log('get:' + ret);
	        me.getJsonpResult = JSON.stringify(ret.data);
	      }
	    }, function (response) {
	      console.log('get jsonp in progress:' + response.length);
	      me.getJsonpResult = "bytes received:" + response.length;
	    });

	    stream.fetch({
	      method: 'GET',
	      url: GET_URL,
	      type: 'json'
	    }, function (ret) {
	      if (!ret.ok) {
	        me.getResult = "request failed";
	      } else {
	        console.log('get:' + ret);
	        me.getResult = JSON.stringify(ret.data);
	      }
	    }, function (response) {
	      console.log('get in progress:' + response.length);
	      me.getResult = "bytes received:" + response.length;
	    });

	    stream.fetch({
	      method: 'POST',
	      url: POST_URL,
	      type: 'json'
	    }, function (ret) {
	      if (!ret.ok) {
	        me.postResult = "request failed";
	      } else {
	        console.log('get:' + JSON.stringify(ret));
	        me.postResult = JSON.stringify(ret.data);
	      }
	    }, function (response) {
	      console.log('get in progress:' + response.length);
	      me.postResult = "bytes received:" + response.length;
	    });

	    stream.fetch({
	      method: 'PUT',
	      url: PUT_URL,
	      type: 'json'
	    }, function (ret) {
	      if (!ret.ok) {
	        me.putResult = "request failed";
	      } else {
	        console.log('get:' + JSON.stringify(ret));
	        me.putResult = JSON.stringify(ret.data);
	      }
	    }, function (response) {
	      console.log('get in progress:' + response.length);
	      me.putResult = "bytes received:" + response.length;
	    });

	    stream.fetch({
	      method: 'DELETE',
	      url: DELETE_URL,
	      type: 'json'
	    }, function (ret) {

	      if (!ret.ok) {
	        me.deleteResult = "request failed";
	      } else {
	        console.log('get:' + JSON.stringify(ret));
	        me.deleteResult = JSON.stringify(ret.data);
	      }
	    }, function (response) {
	      console.log('get in progress:' + response.length);
	      me.deleteResult = "bytes received:" + response.length;
	    });

	    stream.fetch({
	      method: 'HEAD',
	      url: HEAD_URL,
	      type: 'json'
	    }, function (ret) {
	      if (ret.statusText !== 'I\'m a teapot') {
	        me.headResult = "request failed";
	      } else {
	        console.log('get:' + JSON.stringify(ret));
	        me.headResult = ret.statusText;
	      }
	    }, function (response) {
	      console.log('get in progress:' + response.length);
	      me.headResult = "bytes received:" + response.length;
	    });

	    stream.fetch({
	      method: 'PATCH',
	      url: PATCH_URL,
	      type: 'json'
	    }, function (ret) {
	      if (!ret.ok) {
	        me.patchResult = "request failed";
	      } else {
	        console.log('get:' + JSON.stringify(ret));
	        me.patchResult = JSON.stringify(ret.data);
	      }
	    }, function (response) {
	      console.log('get in progress:' + response.length);
	      me.patchResult = "bytes received:" + response.length;
	    });
	  }
	};

/***/ },

/***/ 509:
/***/ function(module, exports) {

	module.exports={render:function(){},staticRenderFns:[]}

/***/ }

/******/ });