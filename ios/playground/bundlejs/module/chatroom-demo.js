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

	var __weex_template__ = __webpack_require__(217)
	var __weex_style__ = __webpack_require__(218)
	var __weex_script__ = __webpack_require__(219)

	__weex_define__('@weex-component/29706d742e69367afe49c8e240836540', [], function(__weex_require__, __weex_exports__, __weex_module__) {

	    __weex_script__(__weex_module__, __weex_exports__, __weex_require__)
	    if (__weex_exports__.__esModule && __weex_exports__.default) {
	      __weex_module__.exports = __weex_exports__.default
	    }

	    __weex_module__.exports.template = __weex_template__

	    __weex_module__.exports.style = __weex_style__

	})

	__weex_bootstrap__('@weex-component/29706d742e69367afe49c8e240836540',undefined,undefined)

/***/ },

/***/ 217:
/***/ function(module, exports) {

	module.exports = {
	  "type": "div",
	  "classList": [
	    "container"
	  ],
	  "children": [
	    {
	      "type": "list",
	      "classList": [
	        "list"
	      ],
	      "attr": {
	        "loadmore": "loadmore",
	        "loadmoreoffset": "500"
	      },
	      "children": [
	        {
	          "type": "cell",
	          "append": "tree",
	          "repeat": {
	            "expression": function () {return this.rows},
	            "value": "row"
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
	                  "style": {
	                    "textAlign": function () {return this.row.align},
	                    "backgroundColor": function () {return this.row.bg}
	                  },
	                  "attr": {
	                    "value": function () {return this.row.message}
	                  }
	                }
	              ]
	            }
	          ]
	        },
	        {
	          "type": "cell",
	          "append": "tree",
	          "children": [
	            {
	              "type": "text",
	              "id": "cellfoot",
	              "style": {
	                "marginBottom": 40
	              }
	            }
	          ]
	        }
	      ]
	    },
	    {
	      "type": "div",
	      "style": {
	        "flexDirection": "row",
	        "justifyContent": "center"
	      },
	      "children": [
	        {
	          "type": "input",
	          "attr": {
	            "type": "text",
	            "placeholder": "请输入聊天信息",
	            "autofocus": "false",
	            "value": ""
	          },
	          "classList": [
	            "input"
	          ],
	          "events": {
	            "change": "onchange",
	            "input": "oninput"
	          },
	          "id": "input"
	        },
	        {
	          "type": "text",
	          "classList": [
	            "button"
	          ],
	          "events": {
	            "click": "send"
	          },
	          "attr": {
	            "value": "发送"
	          }
	        }
	      ]
	    },
	    {
	      "type": "text",
	      "id": "foot",
	      "style": {
	        "color": "#000000",
	        "height": 40
	      }
	    }
	  ]
	}

/***/ },

/***/ 218:
/***/ function(module, exports) {

	module.exports = {
	  "input": {
	    "fontSize": 40,
	    "height": 80,
	    "width": 500,
	    "marginBottom": 40,
	    "borderWidth": 2,
	    "borderStyle": "solid",
	    "borderColor": "rgb(162,217,192)"
	  },
	  "button": {
	    "fontSize": 36,
	    "width": 150,
	    "height": 80,
	    "textAlign": "center",
	    "paddingTop": 15,
	    "paddingBottom": 15,
	    "borderWidth": 2,
	    "borderStyle": "solid",
	    "marginRight": 20,
	    "marginLeft": 20,
	    "borderColor": "rgb(162,217,192)",
	    "backgroundColor": "#00BFFF",
	    "color": "#FFFFFF",
	    "fontWeight": "900",
	    "boxShadow": "1 1px 8px rgba(205, 155, 29, 0.85)",
	    "backgroundColor:active": "#1E90FF",
	    "boxShadow:active": "1 4px 6px rgba(255, 99, 71, 0.85)"
	  },
	  "container": {
	    "flex": 1,
	    "justifyContent": "center",
	    "alignItems": "center",
	    "flexDirection": "column",
	    "borderTopStyle": "solid",
	    "borderTopWidth": 2,
	    "borderTopColor": "#DFDFDF"
	  },
	  "list": {
	    "flex": 1,
	    "width": 750,
	    "justifyContent": "center",
	    "alignItems": "center",
	    "flexDirection": "column",
	    "borderTopStyle": "solid",
	    "borderTopWidth": 2,
	    "borderTopColor": "#DFDFDF"
	  },
	  "item": {
	    "justifyContent": "center",
	    "height": 60,
	    "padding": 20,
	    "marginTop": 5
	  },
	  "item-title": {
	    "fontSize": 30,
	    "height": 60,
	    "paddingTop": 10,
	    "paddingRight": 20,
	    "paddingLeft": 20
	  }
	}

/***/ },

/***/ 219:
/***/ function(module, exports) {

	module.exports = function(module, exports, __weex_require__){'use strict';

	var dom = __weex_require__('@weex-module/dom');
	var websocket = __weex_require__('@weex-module/webSocket');
	module.exports = {
	  data: function () {return {
	    rows: [],
	    names: ['A', 'B', 'C', 'D', 'E', 'F', 'H', 'G'],
	    connectinfo: '',
	    sendinfo: '',
	    onopeninfo: '',
	    onmessage: '',
	    oncloseinfo: '',
	    onerrorinfo: '',
	    closeinfo: '',
	    txtInput: '',
	    info: '',
	    message: '',
	    align: 'left',
	    bg: 'white',
	    from_client_id: '',
	    name: '',
	    number: 1
	  }},
	  methods: {
	    ready: function ready() {
	      var self = this;
	      self.connect();
	    },
	    connect: function connect() {
	      websocket.WebSocket('ws://chat.workerman.net:7272/', '');
	      var self = this;
	      self.info = 'connecting...';
	      websocket.onopen = function (e) {
	        self.info = 'websocket open';
	        var count = self.names.length;
	        var id = Math.ceil(Math.random() * count);
	        self.name = self.names[id];
	        var loginData = '{"type":"login","client_name":"' + self.name + '","room_id":"1"}';
	        self.login(loginData);
	      };

	      websocket.onmessage = function (e) {
	        self.onmessage = e.data;
	        var message = JSON.parse(e.data);
	        self.align = 'left';
	        self.bg = 'white';
	        var loginMessage = '';
	        if (message.type == 'login') {
	          if (self.from_client_id.length == 0) {
	            self.from_client_id = message.client_id;

	            self.align = 'right';
	            self.bg = '#00CD00';
	            loginMessage = self.name + ' 欢迎您加入了聊天室';
	          } else {
	            self.info = 'type is login';
	            self.align = 'left';
	            self.bg = 'white';
	            loginMessage = message.client_name + '加入了聊天室';
	          }
	        }
	        if (self.from_client_id == message.from_client_id) {
	          self.align = 'right';
	          self.bg = '#00CD00';
	        }

	        if (message.type == 'ping') {}

	        if (message.type == 'login') {
	          self.rows.push({ message: loginMessage, align: self.align, bg: self.bg });
	        } else if (message.type == 'logout') {
	          self.rows.push({ message: message.from_client_name + '离开了聊天室', align: self.align, bg: self.bg });
	        }

	        if (message.content) {
	          self.rows.push({ message: message.from_client_name + ':' + message.content, align: self.align, bg: self.bg });
	          if (self.rows.length > 16) {
	            dom.scrollToElement(self.$el('cellfoot'), { offset: 0 });
	          }
	        }
	      };
	      websocket.onerror = function (e) {
	        self.onerrorinfo = e.data;
	      };
	      websocket.onclose = function (e) {
	        self.onopeninfo = '';
	        self.onerrorinfo = e.code;
	      };
	    },
	    login: function login(loginInfo) {
	      websocket.send(loginInfo);
	    },
	    send: function send(e) {

	      var input = this.$el('input');
	      input.blur();
	      var self = this;
	      var sendinfo = '{"type":"say","from_client_id":"' + self.from_client_id + '","from_client_name":"' + '游客' + '","to_client_id":"all","content":"' + this.txtInput + '","time":"2017-03-15 01:04:00"}';
	      websocket.send(sendinfo);
	    },
	    oninput: function oninput(event) {
	      this.txtInput = event.value;
	    },
	    close: function close(e) {
	      websocket.close();
	    }
	  }
	};}
	/* generated by weex-loader */


/***/ }

/******/ });