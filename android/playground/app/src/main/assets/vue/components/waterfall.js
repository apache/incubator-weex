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
	__vue_styles__.push(__webpack_require__(87)
	)

	/* script */
	__vue_exports__ = __webpack_require__(88)

	/* template */
	var __vue_template__ = __webpack_require__(89)
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
	__vue_options__.__file = "/Users/Hanks/Codes/work/incubator-weex/examples/vue/components/waterfall.vue"
	__vue_options__.render = __vue_template__.render
	__vue_options__.staticRenderFns = __vue_template__.staticRenderFns
	__vue_options__._scopeId = "data-v-127cc93e"
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

/***/ 87:
/***/ (function(module, exports) {

	module.exports = {
	  "page": {
	    "backgroundColor": "#EFEFEF"
	  },
	  "refresh": {
	    "height": 128,
	    "width": 750,
	    "flexDirection": "row",
	    "alignItems": "center",
	    "justifyContent": "center"
	  },
	  "refreshText": {
	    "color": "#888888",
	    "fontWeight": "bold"
	  },
	  "indicator": {
	    "color": "#888888",
	    "height": 40,
	    "width": 40,
	    "marginRight": 30
	  },
	  "header": {
	    "backgroundColor": "#ffffff"
	  },
	  "banner": {
	    "height": 377,
	    "flexDirection": "row"
	  },
	  "bannerInfo": {
	    "width": 270,
	    "alignItems": "center",
	    "justifyContent": "center"
	  },
	  "avatar": {
	    "width": 148,
	    "height": 108,
	    "borderRadius": 54,
	    "borderWidth": 4,
	    "borderColor": "#FFFFFF",
	    "marginBottom": 14
	  },
	  "name": {
	    "fontWeight": "bold",
	    "fontSize": 32,
	    "color": "#ffffff",
	    "lineHeight": 32,
	    "textAlign": "center",
	    "marginBottom": 16
	  },
	  "titleWrap": {
	    "width": 100,
	    "height": 24,
	    "marginBottom": 10,
	    "backgroundColor": "rgba(255,255,255,0.8)",
	    "borderRadius": 12,
	    "justifyContent": "center",
	    "alignItems": "center"
	  },
	  "title": {
	    "fontSize": 20,
	    "color": "#000000"
	  },
	  "bannerPhotoWrap": {
	    "width": 449,
	    "height": 305,
	    "backgroundColor": "#FFFFFF",
	    "borderRadius": 12,
	    "marginTop": 35,
	    "paddingTop": 12,
	    "paddingRight": 12,
	    "paddingBottom": 12,
	    "paddingLeft": 12,
	    "flexDirection": "row",
	    "justifyContent": "space-between",
	    "flexWrap": "wrap"
	  },
	  "bannerPhoto": {
	    "width": 137,
	    "height": 137,
	    "marginBottom": 6
	  },
	  "stickyHeader": {
	    "position": "sticky",
	    "height": 94,
	    "flexDirection": "row",
	    "paddingBottom": 6
	  },
	  "stickyWrapper": {
	    "flexDirection": "row",
	    "backgroundColor": "#00cc99",
	    "justifyContent": "center",
	    "alignItems": "center",
	    "flex": 1
	  },
	  "stickyTextImageWrapper": {
	    "flex": 1,
	    "justifyContent": "center",
	    "alignItems": "center",
	    "flexDirection": "row"
	  },
	  "stickyText": {
	    "color": "#FFFFFF",
	    "fontWeight": "bold",
	    "fontSize": 32,
	    "marginRight": 12
	  },
	  "stickyImage": {
	    "width": 64,
	    "height": 64,
	    "borderRadius": 32
	  },
	  "cell": {
	    "paddingTop": 6,
	    "paddingBottom": 6
	  },
	  "item": {
	    "backgroundColor": "#FFFFFF",
	    "alignItems": "center"
	  },
	  "itemName": {
	    "fontSize": 28,
	    "color": "#333333",
	    "lineHeight": 42,
	    "textAlign": "left",
	    "marginTop": 24
	  },
	  "itemPhoto": {
	    "marginTop": 18,
	    "width": 220,
	    "height": 220,
	    "marginBottom": 18
	  },
	  "itemDesc": {
	    "fontSize": 24,
	    "marginTop": 12,
	    "marginRight": 12,
	    "marginBottom": 12,
	    "marginLeft": 12,
	    "color": "#999999",
	    "lineHeight": 36,
	    "textAlign": "left"
	  },
	  "itemClickBehaviour": {
	    "fontSize": 36,
	    "color": "#00cc99",
	    "lineHeight": 36,
	    "textAlign": "center",
	    "marginTop": 6,
	    "marginLeft": 24,
	    "marginRight": 24,
	    "marginBottom": 30
	  },
	  "fixedItem": {
	    "position": "fixed",
	    "width": 78,
	    "height": 78,
	    "backgroundColor": "#00cc99",
	    "right": 32,
	    "bottom": 32,
	    "borderRadius": 39,
	    "alignItems": "center",
	    "justifyContent": "center"
	  },
	  "fixedText": {
	    "fontSize": 36,
	    "color": "#FFFFFF",
	    "lineHeight": 36
	  }
	}

/***/ }),

/***/ 88:
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
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
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
	    var items = [{
	      src: 'https://gw.alicdn.com/tps/TB1Jl1CPFXXXXcJXXXXXXXXXXXX-370-370.jpg',
	      name: 'Thomas Carlyle',
	      desc: 'Genius only means hard-working all one\'s life',
	      behaviourName: 'Change count',
	      behaviour: 'changeColumnCount'
	    }, {
	      src: 'https://gw.alicdn.com/tps/TB1Hv1JPFXXXXa3XXXXXXXXXXXX-370-370.jpg',
	      desc: 'The man who has made up his mind to win will never say "impossible "',
	      behaviourName: 'Change gap',
	      behaviour: 'changeColumnGap'
	    }, {
	      src: 'https://gw.alicdn.com/tps/TB1eNKuPFXXXXc_XpXXXXXXXXXX-370-370.jpg',
	      desc: 'There is no such thing as a great talent without great will - power',
	      behaviourName: 'Show scrollbar',
	      behaviour: 'showScrollbar'
	    }, {
	      src: 'https://gw.alicdn.com/tps/TB1DCh8PFXXXXX7aXXXXXXXXXXX-370-370.jpg',
	      name: 'Addison',
	      desc: 'Cease to struggle and you cease to live',
	      behaviourName: 'Change width',
	      behaviour: 'changeColumnWidth'
	    }, {
	      src: 'https://gw.alicdn.com/tps/TB1ACygPFXXXXXwXVXXXXXXXXXX-370-370.jpg',
	      desc: 'A strong man will struggle with the storms of fate',
	      behaviourName: 'Listen appear',
	      behaviour: 'listenAppear'
	    }, {
	      src: 'https://gw.alicdn.com/tps/TB1IGShPFXXXXaqXVXXXXXXXXXX-370-370.jpg',
	      name: 'Ruskin',
	      desc: 'Living without an aim is like sailing without a compass',
	      behaviourName: 'Set scrollable',
	      behaviour: 'setScrollable'
	    }, {
	      src: 'https://gw.alicdn.com/tps/TB1xU93PFXXXXXHaXXXXXXXXXXX-240-240.jpg',
	      behaviourName: 'waterfall padding',
	      behaviour: 'setPadding'
	    }, {
	      src: 'https://gw.alicdn.com/tps/TB19hu0PFXXXXaXaXXXXXXXXXXX-240-240.jpg',
	      name: 'Balzac',
	      desc: 'There is no such thing as a great talent without great will - power',
	      behaviourName: 'listen scroll',
	      behaviour: 'listenScroll'
	    }, {
	      src: 'https://gw.alicdn.com/tps/TB1ux2vPFXXXXbkXXXXXXXXXXXX-240-240.jpg',
	      behaviourName: 'Remove cell',
	      behaviour: 'removeCell'
	    }, {
	      src: 'https://gw.alicdn.com/tps/TB1tCCWPFXXXXa7aXXXXXXXXXXX-240-240.jpg',
	      behaviourName: 'Move cell',
	      behaviour: 'moveCell'
	    }];

	    var repeatItems = [];
	    for (var i = 0; i < 3; i++) {
	      repeatItems.push.apply(repeatItems, items);
	    }

	    return {
	      padding: 0,
	      refreshing: false,
	      refreshText: '↓   pull to refresh...',
	      columnCount: 2,
	      columnGap: 12,
	      columnWidth: 'auto',
	      contentOffset: '0',
	      showHeader: true,
	      showScrollbar: false,
	      scrollable: true,
	      showStickyHeader: false,
	      appearImage: null,
	      disappearImage: null,
	      stickyHeaderType: 'none',
	      banner: {
	        photos: [{ src: 'https://gw.alicdn.com/tps/TB1JyaCPFXXXXc9XXXXXXXXXXXX-140-140.jpg' }, { src: 'https://gw.alicdn.com/tps/TB1MwSFPFXXXXbdXXXXXXXXXXXX-140-140.jpg' }, { src: 'https://gw.alicdn.com/tps/TB1U8avPFXXXXaDXpXXXXXXXXXX-140-140.jpg' }, { src: 'https://gw.alicdn.com/tps/TB17Xh8PFXXXXbkaXXXXXXXXXXX-140-140.jpg' }, { src: 'https://gw.alicdn.com/tps/TB1cTmLPFXXXXXRXXXXXXXXXXXX-140-140.jpg' }, { src: 'https://gw.alicdn.com/tps/TB1oCefPFXXXXbVXVXXXXXXXXXX-140-140.jpg' }]
	      },
	      items: repeatItems
	    };
	  },

	  methods: {
	    recylerScroll: function recylerScroll(e) {
	      this.contentOffset = e.contentOffset.y;
	    },
	    showOrRemoveHeader: function showOrRemoveHeader() {
	      this.showHeader = !this.showHeader;
	    },
	    onItemclick: function onItemclick(behaviour, index) {
	      console.log('click...' + behaviour + ' at index ' + index);
	      switch (behaviour) {
	        case 'changeColumnCount':
	          this.changeColumnCount();
	          break;
	        case 'changeColumnGap':
	          this.changeColumnGap();
	          break;
	        case 'changeColumnWidth':
	          this.changeColumnWidth();
	          break;
	        case 'showScrollbar':
	          this.showOrHideScrollbar();
	          break;
	        case 'listenAppear':
	          this.listenAppearAndDisappear();
	          break;
	        case 'setScrollable':
	          this.setScrollable();
	          break;
	        case 'setPadding':
	          this.setRecyclerPadding();
	          break;
	        case 'listenScroll':
	          this.listenScrollEvent();
	          break;
	        case 'removeCell':
	          this.removeCell(index);
	          break;
	        case 'moveCell':
	          this.moveCell(index);
	          break;
	      }
	    },

	    itemAppear: function itemAppear(src) {
	      this.appearImage = src;
	    },

	    itemDisappear: function itemDisappear(src) {
	      this.disappearImage = src;
	    },

	    changeColumnCount: function changeColumnCount() {
	      if (this.columnCount === 2) {
	        this.columnCount = 3;
	      } else {
	        this.columnCount = 2;
	      }
	    },

	    changeColumnGap: function changeColumnGap() {
	      if (this.columnGap === 12) {
	        this.columnGap = 'normal';
	      } else {
	        this.columnGap = 12;
	      }
	    },

	    changeColumnWidth: function changeColumnWidth() {
	      if (this.columnWidth === 'auto') {
	        this.columnWidth = 600;
	      } else {
	        this.columnWidth = 'auto';
	      }
	    },

	    showOrHideScrollbar: function showOrHideScrollbar() {
	      this.showScrollbar = !this.showScrollbar;
	    },

	    setScrollable: function setScrollable() {
	      this.scrollable = !this.scrollable;
	    },

	    listenAppearAndDisappear: function listenAppearAndDisappear() {
	      this.stickyHeaderType = this.stickyHeaderType === 'appear' ? 'none' : 'appear';
	    },

	    listenScrollEvent: function listenScrollEvent() {
	      this.stickyHeaderType = this.stickyHeaderType === 'scroll' ? 'none' : 'scroll';
	    },

	    scrollToTop: function scrollToTop() {
	      // weex.requireModule('dom').scrollToElement(this.$refs.cell0)
	      weex.requireModule('dom').scrollToElement(this.$refs.header);
	    },

	    setRecyclerPadding: function setRecyclerPadding() {
	      debugger;
	      this.padding = this.padding == 0 ? 12 : 0;
	    },

	    removeCell: function removeCell(index) {
	      this.items.splice(index, 1);
	    },

	    moveCell: function moveCell(index) {

	      if (index == 0) {
	        this.items.splice(this.items.length - 1, 0, this.items.splice(index, 1)[0]);
	      } else {
	        this.items.splice(0, 0, this.items.splice(index, 1)[0]);
	      }
	    },

	    onrefresh: function onrefresh(event) {
	      var _this = this;

	      this.refreshing = true;
	      this.refreshText = "loading...";
	      setTimeout(function () {
	        _this.refreshing = false;
	        _this.refreshText = '↓   pull to refresh...';
	      }, 2000);
	    },
	    onpullingdown: function onpullingdown(event) {
	      // console.log(`${event.pullingDistance}`)
	      if (event.pullingDistance < -64) {
	        this.refreshText = '↑   release to refresh...';
	      } else {
	        this.refreshText = '↓   pull to refresh...';
	      }
	    }
	  }
	};

/***/ }),

/***/ 89:
/***/ (function(module, exports) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('waterfall', {
	    staticClass: ["page"],
	    style: {
	      padding: _vm.padding
	    },
	    attrs: {
	      "columnWidth": _vm.columnWidth,
	      "columnCount": _vm.columnCount,
	      "columnGap": _vm.columnGap,
	      "showScrollbar": _vm.showScrollbar,
	      "scrollable": _vm.scrollable
	    },
	    on: {
	      "scroll": _vm.recylerScroll
	    }
	  }, [_c('refresh', {
	    staticClass: ["refresh"],
	    attrs: {
	      "display": _vm.refreshing ? 'show' : 'hide'
	    },
	    on: {
	      "refresh": _vm.onrefresh,
	      "pullingdown": _vm.onpullingdown
	    }
	  }, [_c('loading-indicator', {
	    staticClass: ["indicator"]
	  }), _c('text', {
	    staticClass: ["refreshText"]
	  }, [_vm._v(_vm._s(_vm.refreshText))])]), (_vm.showHeader) ? _c('header', {
	    ref: "header",
	    staticClass: ["header"]
	  }, [_c('div', {
	    staticClass: ["banner"],
	    attrs: {
	      "src": "https://gw.alicdn.com/tps/TB1ESN1PFXXXXX1apXXXXXXXXXX-1000-600.jpg",
	      "resize": "cover"
	    }
	  }, [_c('div', {
	    staticClass: ["bannerInfo"]
	  }, [_c('image', {
	    staticClass: ["avatar"],
	    attrs: {
	      "src": "https://gw.alicdn.com/tps/TB1EP9bPFXXXXbpXVXXXXXXXXXX-150-110.jpg",
	      "resize": "cover"
	    }
	  }), _c('text', {
	    staticClass: ["name"]
	  }, [_vm._v("Adam Cat")]), _c('div', {
	    staticClass: ["titleWrap"]
	  }, [_c('text', {
	    staticClass: ["title"]
	  }, [_vm._v("Genius")])])]), _c('div', {
	    staticClass: ["bannerPhotoWrap"]
	  }, _vm._l((_vm.banner.photos), function(photo) {
	    return _c('image', {
	      staticClass: ["bannerPhoto"],
	      attrs: {
	        "src": photo.src
	      }
	    })
	  }))])]) : _vm._e(), _c('header', {
	    staticClass: ["stickyHeader"]
	  }, [(_vm.stickyHeaderType === 'none') ? _c('div', {
	    staticClass: ["stickyWrapper"]
	  }, [_c('text', {
	    staticClass: ["stickyText"]
	  }, [_vm._v("Sticky Header")])]) : _vm._e(), (_vm.stickyHeaderType === 'appear') ? _c('div', {
	    staticClass: ["stickyWrapper"]
	  }, [_c('div', {
	    staticClass: ["stickyTextImageWrapper"]
	  }, [_c('text', {
	    staticClass: ["stickyText"]
	  }, [_vm._v("Last Appear:")]), _c('image', {
	    staticClass: ["stickyImage"],
	    attrs: {
	      "src": _vm.appearImage
	    }
	  })]), _c('div', {
	    staticClass: ["stickyTextImageWrapper"]
	  }, [_c('text', {
	    staticClass: ["stickyText"]
	  }, [_vm._v("Last Disappear:")]), _c('image', {
	    staticClass: ["stickyImage"],
	    attrs: {
	      "src": _vm.disappearImage
	    }
	  })])]) : _vm._e(), (_vm.stickyHeaderType === 'scroll') ? _c('div', {
	    staticClass: ["stickyWrapper"]
	  }, [_c('text', {
	    staticClass: ["stickyText"]
	  }, [_vm._v("Content Offset:" + _vm._s(_vm.contentOffset))])]) : _vm._e()]), _vm._l((_vm.items), function(item, index) {
	    return _c('cell', {
	      key: index,
	      ref: ("cell" + index),
	      refInFor: true,
	      staticClass: ["cell"],
	      appendAsTree: true,
	      attrs: {
	        "append": "tree"
	      }
	    }, [_c('div', {
	      staticClass: ["item"],
	      on: {
	        "click": function($event) {
	          _vm.onItemclick(item.behaviour, index)
	        },
	        "appear": function($event) {
	          _vm.itemAppear(item.src)
	        },
	        "disappear": function($event) {
	          _vm.itemDisappear(item.src)
	        }
	      }
	    }, [(item.name) ? _c('text', {
	      staticClass: ["itemName"]
	    }, [_vm._v(_vm._s(item.name))]) : _vm._e(), _c('image', {
	      staticClass: ["itemPhoto"],
	      attrs: {
	        "src": item.src
	      }
	    }), (item.desc) ? _c('text', {
	      staticClass: ["itemDesc"]
	    }, [_vm._v(_vm._s(item.desc))]) : _vm._e(), (item.behaviourName) ? _c('text', {
	      staticClass: ["itemClickBehaviour"]
	    }, [_vm._v(" " + _vm._s(item.behaviourName))]) : _vm._e()])])
	  }), _c('div', {
	    staticClass: ["fixedItem"],
	    on: {
	      "click": _vm.scrollToTop
	    }
	  }, [_c('text', {
	    staticClass: ["fixedText"]
	  }, [_vm._v("Top")])])], 2)
	},staticRenderFns: []}
	module.exports.render._withStripped = true

/***/ })

/******/ });