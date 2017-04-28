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

	var __weex_template__ = __webpack_require__(102)
	var __weex_style__ = __webpack_require__(103)
	var __weex_script__ = __webpack_require__(104)

	__weex_define__('@weex-component/e4cde146096cc51ec5ab930a518ea157', [], function(__weex_require__, __weex_exports__, __weex_module__) {

	    __weex_script__(__weex_module__, __weex_exports__, __weex_require__)
	    if (__weex_exports__.__esModule && __weex_exports__.default) {
	      __weex_module__.exports = __weex_exports__.default
	    }

	    __weex_module__.exports.template = __weex_template__

	    __weex_module__.exports.style = __weex_style__

	})

	__weex_bootstrap__('@weex-component/e4cde146096cc51ec5ab930a518ea157',undefined,undefined)

/***/ },

/***/ 34:
/***/ function(module, exports) {

	var core = module.exports = {version: '2.4.0'};
	if(typeof __e == 'number')__e = core; // eslint-disable-line no-undef

/***/ },

/***/ 100:
/***/ function(module, exports, __webpack_require__) {

	module.exports = { "default": __webpack_require__(101), __esModule: true };

/***/ },

/***/ 101:
/***/ function(module, exports, __webpack_require__) {

	var core  = __webpack_require__(34)
	  , $JSON = core.JSON || (core.JSON = {stringify: JSON.stringify});
	module.exports = function stringify(it){ // eslint-disable-line no-unused-vars
	  return $JSON.stringify.apply($JSON, arguments);
	};

/***/ },

/***/ 102:
/***/ function(module, exports) {

	module.exports = {
	  "type": "div",
	  "children": [
	    {
	      "type": "div",
	      "style": {
	        "backgroundColor": "#eeeeee"
	      },
	      "children": [
	        {
	          "type": "text",
	          "classList": [
	            "count"
	          ],
	          "attr": {
	            "value": function () {return 'Appear items:' + (this.appearMin) + ' - ' + (this.appearMax)}
	          }
	        },
	        {
	          "type": "div",
	          "style": {
	            "height": 20,
	            "width": 750,
	            "borderColor": "#000000",
	            "borderWidth": 2,
	            "justifyContent": "center"
	          },
	          "children": [
	            {
	              "type": "div",
	              "style": {
	                "height": 14,
	                "width": function () {return this.progress_width},
	                "marginLeft": function () {return this.progress},
	                "backgroundColor": "#808080"
	              }
	            }
	          ]
	        },
	        {
	          "type": "text",
	          "style": {
	            "width": 750
	          },
	          "attr": {
	            "value": function () {return this.event}
	          }
	        }
	      ]
	    },
	    {
	      "type": "list",
	      "id": "list",
	      "classList": [
	        "list"
	      ],
	      "events": {
	        "scroll": "onScroll"
	      },
	      "attr": {
	        "offsetAccuracy": "5"
	      },
	      "children": [
	        {
	          "type": "refresh",
	          "classList": [
	            "refresh-view"
	          ],
	          "attr": {
	            "display": function () {return this.refresh_display}
	          },
	          "events": {
	            "refresh": "onrefresh"
	          },
	          "children": [
	            {
	              "type": "text",
	              "shown": function () {return (this.refresh_display==='hide')},
	              "attr": {
	                "value": "↓ pull to refresh"
	              }
	            },
	            {
	              "type": "loading-indicator",
	              "classList": [
	                "indicator"
	              ]
	            }
	          ]
	        },
	        {
	          "type": "cell",
	          "append": "tree",
	          "events": {
	            "appear": "onappear",
	            "disappear": "ondisappear"
	          },
	          "classList": [
	            "row"
	          ],
	          "repeat": function () {return this.rows},
	          "attr": {
	            "index": function () {return this.$index}
	          },
	          "children": [
	            {
	              "type": "div",
	              "classList": [
	                "item"
	              ],
	              "children": [
	                {
	                  "type": "text",
	                  "classList": [
	                    "item-title"
	                  ],
	                  "attr": {
	                    "value": function () {return 'row ' + (this.id)}
	                  }
	                }
	              ]
	            }
	          ]
	        },
	        {
	          "type": "loading",
	          "classList": [
	            "loading-view"
	          ],
	          "attr": {
	            "display": function () {return this.loading_display}
	          },
	          "events": {
	            "loading": "onloading"
	          },
	          "children": [
	            {
	              "type": "text",
	              "shown": function () {return (this.loading_display==='hide')},
	              "attr": {
	                "value": "↑ Loadmore"
	              }
	            },
	            {
	              "type": "loading-indicator",
	              "classList": [
	                "indicator"
	              ]
	            }
	          ]
	        }
	      ]
	    }
	  ]
	}

/***/ },

/***/ 103:
/***/ function(module, exports) {

	module.exports = {
	  "list": {
	    "height": 850
	  },
	  "count": {
	    "fontSize": 48,
	    "margin": 10
	  },
	  "indicator": {
	    "height": 60,
	    "width": 60,
	    "color": "#889967"
	  },
	  "refresh-arrow": {
	    "fontSize": 30,
	    "color": "#45b5f0"
	  },
	  "row": {
	    "width": 750
	  },
	  "item": {
	    "justifyContent": "center",
	    "borderBottomWidth": 2,
	    "borderBottomColor": "#c0c0c0",
	    "height": 100,
	    "padding": 20,
	    "backgroundColor:active": "#00BFFF"
	  },
	  "refresh-view": {
	    "width": 750,
	    "height": 100,
	    "display": "flex",
	    "MsFlexAlign": "center",
	    "WebkitAlignItems": "center",
	    "WebkitBoxAlign": "center",
	    "alignItems": "center"
	  },
	  "loading-view": {
	    "width": 750,
	    "height": 100,
	    "display": "flex",
	    "MsFlexAlign": "center",
	    "WebkitAlignItems": "center",
	    "WebkitBoxAlign": "center",
	    "alignItems": "center"
	  }
	}

/***/ },

/***/ 104:
/***/ function(module, exports, __webpack_require__) {

	module.exports = function(module, exports, __weex_require__){'use strict';

	var _stringify = __webpack_require__(100);

	var _stringify2 = _interopRequireDefault(_stringify);

	function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

	var dom = weex.requireModule('dom');
	var isFirst = true;
	module.exports = {
	  methods: {
	    onappear: function onappear(e) {
	      var appearId = this.rows[e.target.attr.index].id;
	      nativeLog('+++++', appearId);
	      var appearIds = this.appearIds;
	      appearIds.push(appearId);
	      this.getMinAndMaxIds(appearIds);
	    },
	    ondisappear: function ondisappear(e) {
	      var disAppearId = this.rows[e.target.attr.index].id;
	      nativeLog('+++++', disAppearId);
	      var appearIds = this.appearIds;
	      var index = appearIds.indexOf(disAppearId);
	      if (index > -1) {
	        appearIds.splice(index, 1);
	      }
	      this.getMinAndMaxIds(appearIds);
	    },
	    getMinAndMaxIds: function getMinAndMaxIds(appearIds) {
	      appearIds.sort(function (a, b) {
	        return a - b;
	      });
	      this.appearIds = appearIds;
	      this.appearMax = appearIds[appearIds.length - 1];
	      this.appearMin = appearIds[0];
	    },
	    onrefresh: function onrefresh(e) {
	      var self = this;
	      self.refresh_display = 'show';
	      self.$call('modal', 'toast', {
	        'message': 'onrefresh'
	      });

	      this.$call('timer', 'setTimeout', function () {
	        self.refresh_display = 'hide';
	      }, 3000);
	    },

	    onloading: function onloading() {
	      var self = this;
	      self.loading_display = 'show';
	      self.$call('modal', 'toast', {
	        'message': 'onloading'
	      });

	      this.$call('timer', 'setTimeout', function () {
	        if (self.rows.length <= 33) {
	          self.rows.push(self.moreRows[self.rows.length - 29]);
	        }
	        self.loading_display = 'hide';
	      }, 3000);
	    },

	    onScroll: function onScroll(e) {
	      var self = this;
	      this.event = 'contentOffset: ' + (0, _stringify2.default)(e.contentOffset) + '\ncontentSize: ' + (0, _stringify2.default)(e.contentSize);
	      dom.getComponentRect(this.$el('list'), function (ret) {
	        if (e.contentOffset.x == 0 && e.contentOffset.y == 0 && isFirst) {
	          isFirst = false;
	          return;
	        }
	        var listHeight = ret.size.height;
	        self.progress_width = listHeight / e.contentSize.height * 750;
	        var offsetY = e.contentOffset.y > 0 ? 0 : Math.abs(e.contentOffset.y);
	        self.progress = offsetY / (e.contentSize.height - listHeight) * (750 - self.progress_width);
	      });
	    }
	  },
	  data: function () {return {
	    progress_width: 0,
	    progress: 0,
	    event: '-',
	    refresh_display: 'hide',
	    loading_display: 'hide',
	    appearMin: 1,
	    appearMax: 1,
	    appearIds: [],
	    rows: [{ id: 1 }, { id: 2 }, { id: 3 }, { id: 4 }, { id: 5 }, { id: 6 }, { id: 7 }, { id: 8 }, { id: 9 }, { id: 10 }, { id: 11 }, { id: 12 }, { id: 13 }, { id: 14 }, { id: 15 }, { id: 16 }, { id: 17 }, { id: 18 }, { id: 19 }, { id: 20 }, { id: 21 }, { id: 22 }, { id: 23 }, { id: 24 }, { id: 25 }, { id: 26 }, { id: 27 }, { id: 28 }, { id: 29 }],
	    moreRows: [{ id: 30 }, { id: 31 }, { id: 32 }, { id: 33 }]
	  }}
	};}
	/* generated by weex-loader */


/***/ }

/******/ });