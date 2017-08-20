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

	var __weex_template__ = __webpack_require__(94)
	var __weex_style__ = __webpack_require__(95)
	var __weex_script__ = __webpack_require__(96)

	__weex_define__('@weex-component/9b05297660bffdeefca2cb266249ee4d', [], function(__weex_require__, __weex_exports__, __weex_module__) {

	    __weex_script__(__weex_module__, __weex_exports__, __weex_require__)
	    if (__weex_exports__.__esModule && __weex_exports__.default) {
	      __weex_module__.exports = __weex_exports__.default
	    }

	    __weex_module__.exports.template = __weex_template__

	    __weex_module__.exports.style = __weex_style__

	})

	__weex_bootstrap__('@weex-component/9b05297660bffdeefca2cb266249ee4d',undefined,undefined)

/***/ },

/***/ 94:
/***/ function(module, exports) {

	module.exports = {
	  "type": "div",
	  "children": [
	    {
	      "type": "div",
	      "children": [
	        {
	          "type": "text",
	          "style": {
	            "fontSize": 40
	          },
	          "attr": {
	            "value": function () {return 'oninput: ' + (this.txtInput)}
	          }
	        },
	        {
	          "type": "text",
	          "style": {
	            "fontSize": 40
	          },
	          "attr": {
	            "value": function () {return 'onchange: ' + (this.txtChange)}
	          }
	        },
	        {
	          "type": "text",
	          "style": {
	            "fontSize": 40
	          },
	          "attr": {
	            "value": function () {return 'onreturntype: ' + (this.txtReturnType)}
	          }
	        },
	        {
	          "type": "text",
	          "style": {
	            "fontSize": 40
	          },
	          "attr": {
	            "value": function () {return 'onrange: ' + (this.txtRange)}
	          }
	        }
	      ]
	    },
	    {
	      "type": "scroller",
	      "children": [
	        {
	          "type": "div",
	          "children": [
	            {
	              "type": "div",
	              "style": {
	                "backgroundColor": "#286090"
	              },
	              "children": [
	                {
	                  "type": "text",
	                  "classList": [
	                    "title"
	                  ],
	                  "style": {
	                    "height": 80,
	                    "padding": 20,
	                    "color": "#FFFFFF"
	                  },
	                  "attr": {
	                    "value": "input type = text"
	                  }
	                }
	              ]
	            },
	            {
	              "type": "input",
	              "attr": {
	                "type": "text",
	                "placeholder": "Input Text",
	                ":autofocus": "autofocus",
	                "value": ""
	              },
	              "classList": [
	                "input"
	              ],
	              "events": {
	                "change": "onchange",
	                "input": "oninput"
	              }
	            }
	          ]
	        },
	        {
	          "type": "div",
	          "children": [
	            {
	              "type": "div",
	              "style": {
	                "backgroundColor": "#286090"
	              },
	              "children": [
	                {
	                  "type": "text",
	                  "classList": [
	                    "title"
	                  ],
	                  "style": {
	                    "height": 80,
	                    "padding": 20,
	                    "color": "#FFFFFF"
	                  },
	                  "attr": {
	                    "value": "input type = password"
	                  }
	                }
	              ]
	            },
	            {
	              "type": "input",
	              "attr": {
	                "type": "password",
	                "placeholder": "Input Password"
	              },
	              "classList": [
	                "input"
	              ],
	              "events": {
	                "change": "onchange",
	                "input": "oninput"
	              }
	            }
	          ]
	        },
	        {
	          "type": "div",
	          "children": [
	            {
	              "type": "div",
	              "style": {
	                "backgroundColor": "#286090"
	              },
	              "children": [
	                {
	                  "type": "text",
	                  "classList": [
	                    "title"
	                  ],
	                  "style": {
	                    "height": 80,
	                    "padding": 20,
	                    "color": "#FFFFFF"
	                  },
	                  "attr": {
	                    "value": "input type = url"
	                  }
	                }
	              ]
	            },
	            {
	              "type": "input",
	              "attr": {
	                "type": "url",
	                "placeholder": "Input URL"
	              },
	              "classList": [
	                "input"
	              ],
	              "events": {
	                "change": "onchange",
	                "input": "oninput"
	              }
	            }
	          ]
	        },
	        {
	          "type": "div",
	          "children": [
	            {
	              "type": "div",
	              "style": {
	                "backgroundColor": "#286090"
	              },
	              "children": [
	                {
	                  "type": "text",
	                  "classList": [
	                    "title"
	                  ],
	                  "style": {
	                    "height": 80,
	                    "padding": 20,
	                    "color": "#FFFFFF"
	                  },
	                  "attr": {
	                    "value": "input type = email"
	                  }
	                }
	              ]
	            },
	            {
	              "type": "input",
	              "attr": {
	                "type": "email",
	                "placeholder": "Input Email"
	              },
	              "classList": [
	                "input"
	              ],
	              "events": {
	                "change": "onchange",
	                "input": "oninput"
	              }
	            }
	          ]
	        },
	        {
	          "type": "div",
	          "children": [
	            {
	              "type": "div",
	              "style": {
	                "backgroundColor": "#286090"
	              },
	              "children": [
	                {
	                  "type": "text",
	                  "classList": [
	                    "title"
	                  ],
	                  "style": {
	                    "height": 80,
	                    "padding": 20,
	                    "color": "#FFFFFF"
	                  },
	                  "attr": {
	                    "value": "input type = tel"
	                  }
	                }
	              ]
	            },
	            {
	              "type": "input",
	              "attr": {
	                "type": "tel",
	                "placeholder": "Input Tel"
	              },
	              "classList": [
	                "input"
	              ],
	              "events": {
	                "change": "onchange",
	                "input": "oninput"
	              }
	            }
	          ]
	        },
	        {
	          "type": "div",
	          "children": [
	            {
	              "type": "div",
	              "style": {
	                "backgroundColor": "#286090"
	              },
	              "children": [
	                {
	                  "type": "text",
	                  "classList": [
	                    "title"
	                  ],
	                  "style": {
	                    "height": 80,
	                    "padding": 20,
	                    "color": "#FFFFFF"
	                  },
	                  "attr": {
	                    "value": "input type = time"
	                  }
	                }
	              ]
	            },
	            {
	              "type": "input",
	              "attr": {
	                "type": "time",
	                "placeholder": "Input Time"
	              },
	              "classList": [
	                "input"
	              ],
	              "events": {
	                "change": "onchange",
	                "input": "oninput"
	              }
	            }
	          ]
	        },
	        {
	          "type": "div",
	          "children": [
	            {
	              "type": "div",
	              "style": {
	                "backgroundColor": "#286090"
	              },
	              "children": [
	                {
	                  "type": "text",
	                  "classList": [
	                    "title"
	                  ],
	                  "style": {
	                    "height": 80,
	                    "padding": 20,
	                    "color": "#FFFFFF"
	                  },
	                  "attr": {
	                    "value": "input type = date"
	                  }
	                }
	              ]
	            },
	            {
	              "type": "input",
	              "attr": {
	                "type": "date",
	                "placeholder": "Input Date",
	                "max": "2017-12-12",
	                "min": "2015-01-01"
	              },
	              "classList": [
	                "input"
	              ],
	              "events": {
	                "change": "onchange",
	                "input": "oninput"
	              }
	            }
	          ]
	        },
	        {
	          "type": "div",
	          "children": [
	            {
	              "type": "div",
	              "style": {
	                "backgroundColor": "#286090"
	              },
	              "children": [
	                {
	                  "type": "text",
	                  "classList": [
	                    "title"
	                  ],
	                  "style": {
	                    "height": 80,
	                    "padding": 20,
	                    "color": "#FFFFFF"
	                  },
	                  "attr": {
	                    "value": "input keyboard = default"
	                  }
	                }
	              ]
	            },
	            {
	              "type": "input",
	              "attr": {
	                "type": "text",
	                "placeholder": "please input",
	                "returnKeyType": "default"
	              },
	              "classList": [
	                "input"
	              ],
	              "events": {
	                "change": "onchange",
	                "return": "onreturn",
	                "input": "oninput"
	              }
	            }
	          ]
	        },
	        {
	          "type": "div",
	          "children": [
	            {
	              "type": "div",
	              "style": {
	                "backgroundColor": "#286090"
	              },
	              "children": [
	                {
	                  "type": "text",
	                  "classList": [
	                    "title"
	                  ],
	                  "style": {
	                    "height": 80,
	                    "padding": 20,
	                    "color": "#FFFFFF"
	                  },
	                  "attr": {
	                    "value": "input keyboard = go"
	                  }
	                }
	              ]
	            },
	            {
	              "type": "input",
	              "attr": {
	                "type": "text",
	                "placeholder": "please input",
	                "returnKeyType": "go"
	              },
	              "classList": [
	                "input"
	              ],
	              "events": {
	                "change": "onchange",
	                "return": "onreturn",
	                "input": "oninput"
	              }
	            }
	          ]
	        },
	        {
	          "type": "div",
	          "children": [
	            {
	              "type": "div",
	              "style": {
	                "backgroundColor": "#286090"
	              },
	              "children": [
	                {
	                  "type": "text",
	                  "classList": [
	                    "title"
	                  ],
	                  "style": {
	                    "height": 80,
	                    "padding": 20,
	                    "color": "#FFFFFF"
	                  },
	                  "attr": {
	                    "value": "input keyboard = next"
	                  }
	                }
	              ]
	            },
	            {
	              "type": "input",
	              "attr": {
	                "type": "text",
	                "placeholder": "please input",
	                "returnKeyType": "next"
	              },
	              "classList": [
	                "input"
	              ],
	              "events": {
	                "change": "onchange",
	                "return": "onreturn",
	                "input": "oninput"
	              }
	            }
	          ]
	        },
	        {
	          "type": "div",
	          "children": [
	            {
	              "type": "div",
	              "style": {
	                "backgroundColor": "#286090"
	              },
	              "children": [
	                {
	                  "type": "text",
	                  "classList": [
	                    "title"
	                  ],
	                  "style": {
	                    "height": 80,
	                    "padding": 20,
	                    "color": "#FFFFFF"
	                  },
	                  "attr": {
	                    "value": "input keyboard = search"
	                  }
	                }
	              ]
	            },
	            {
	              "type": "input",
	              "attr": {
	                "type": "text",
	                "placeholder": "please input",
	                "returnKeyType": "search"
	              },
	              "classList": [
	                "input"
	              ],
	              "events": {
	                "change": "onchange",
	                "return": "onreturn",
	                "input": "oninput"
	              }
	            }
	          ]
	        },
	        {
	          "type": "div",
	          "children": [
	            {
	              "type": "div",
	              "style": {
	                "backgroundColor": "#286090"
	              },
	              "children": [
	                {
	                  "type": "text",
	                  "classList": [
	                    "title"
	                  ],
	                  "style": {
	                    "height": 80,
	                    "padding": 20,
	                    "color": "#FFFFFF"
	                  },
	                  "attr": {
	                    "value": "input keyboard = send"
	                  }
	                }
	              ]
	            },
	            {
	              "type": "input",
	              "attr": {
	                "type": "text",
	                "placeholder": "please input",
	                "returnKeyType": "send"
	              },
	              "classList": [
	                "input"
	              ],
	              "events": {
	                "change": "onchange",
	                "return": "onreturn",
	                "input": "oninput"
	              }
	            }
	          ]
	        },
	        {
	          "type": "div",
	          "children": [
	            {
	              "type": "div",
	              "style": {
	                "backgroundColor": "#286090"
	              },
	              "children": [
	                {
	                  "type": "text",
	                  "classList": [
	                    "title"
	                  ],
	                  "style": {
	                    "height": 80,
	                    "padding": 20,
	                    "color": "#FFFFFF"
	                  },
	                  "attr": {
	                    "value": "input keyboard = done"
	                  }
	                }
	              ]
	            },
	            {
	              "type": "input",
	              "attr": {
	                "type": "text",
	                "placeholder": "please input",
	                "returnKeyType": "done"
	              },
	              "classList": [
	                "input"
	              ],
	              "events": {
	                "change": "onchange",
	                "return": "onreturn",
	                "input": "oninput"
	              }
	            }
	          ]
	        },
	        {
	          "type": "div",
	          "children": [
	            {
	              "type": "div",
	              "style": {
	                "backgroundColor": "#286090"
	              },
	              "children": [
	                {
	                  "type": "text",
	                  "classList": [
	                    "title"
	                  ],
	                  "style": {
	                    "height": 80,
	                    "padding": 20,
	                    "color": "#FFFFFF"
	                  },
	                  "attr": {
	                    "value": "function focus() & blur()"
	                  }
	                }
	              ]
	            },
	            {
	              "type": "div",
	              "style": {
	                "flexDirection": "row",
	                "marginBottom": 16,
	                "justifyContent": "space-between"
	              },
	              "children": [
	                {
	                  "type": "text",
	                  "classList": [
	                    "button"
	                  ],
	                  "attr": {
	                    "value": "Focus",
	                    "type": "primary"
	                  },
	                  "events": {
	                    "click": "focus"
	                  }
	                },
	                {
	                  "type": "text",
	                  "classList": [
	                    "button"
	                  ],
	                  "attr": {
	                    "value": "Blur",
	                    "type": "primary"
	                  },
	                  "events": {
	                    "click": "blur"
	                  }
	                }
	              ]
	            },
	            {
	              "type": "input",
	              "attr": {
	                "type": "text",
	                "placeholder": "Input1",
	                "value": ""
	              },
	              "classList": [
	                "input"
	              ],
	              "id": "input1"
	            }
	          ]
	        },
	        {
	          "type": "div",
	          "children": [
	            {
	              "type": "div",
	              "style": {
	                "backgroundColor": "#286090"
	              },
	              "children": [
	                {
	                  "type": "text",
	                  "classList": [
	                    "title"
	                  ],
	                  "style": {
	                    "height": 80,
	                    "padding": 20,
	                    "color": "#FFFFFF"
	                  },
	                  "attr": {
	                    "value": "input selection"
	                  }
	                }
	              ]
	            },
	            {
	              "type": "div",
	              "style": {
	                "flexDirection": "row",
	                "marginBottom": 16,
	                "justifyContent": "space-between"
	              },
	              "children": [
	                {
	                  "type": "text",
	                  "classList": [
	                    "button"
	                  ],
	                  "attr": {
	                    "value": "setRange",
	                    "type": "primary"
	                  },
	                  "events": {
	                    "click": "setRange"
	                  }
	                },
	                {
	                  "type": "text",
	                  "classList": [
	                    "button"
	                  ],
	                  "attr": {
	                    "value": "getRange",
	                    "type": "primary"
	                  },
	                  "events": {
	                    "click": "getRange"
	                  }
	                }
	              ]
	            },
	            {
	              "type": "input",
	              "attr": {
	                "type": "text",
	                "placeholder": "please input",
	                "value": "123456789"
	              },
	              "id": "inputselection",
	              "classList": [
	                "input"
	              ],
	              "events": {
	                "change": "onchange",
	                "return": "onreturn",
	                "input": "oninput"
	              }
	            }
	          ]
	        }
	      ]
	    }
	  ]
	}

/***/ },

/***/ 95:
/***/ function(module, exports) {

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

/***/ },

/***/ 96:
/***/ function(module, exports) {

	module.exports = function(module, exports, __weex_require__){'use strict';

	module.exports = {
	    data: function data() {
	        return {
	            txtInput: '',
	            txtChange: '',
	            txtReturnType: '',
	            txtSelection: '',
	            autofocus: false,
	            txtRange: ''
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
	            this.$el('input1').focus();
	        },
	        blur: function blur() {
	            this.$el('input1').blur();
	        },
	        setRange: function setRange() {
	            console.log(this.$el["inputselection"]);
	            this.$el("inputselection").setSelectionRange(2, 6);
	        },
	        getRange: function getRange() {
	            var self = this;
	            this.$el("inputselection").getSelectionRange(function (e) {
	                self.txtRange = e.selectionStart + '-' + e.selectionEnd;
	            });
	        }
	    }
	};}
	/* generated by weex-loader */


/***/ }

/******/ });