define('@weex-component/ui-panel', function (require, exports, module) {

;
  module.exports = {
    data: function () {return {
      type: 'default',
      title: '',
      paddingBody: 20,
      paddingHead: 20,
      dataClass: '', // FIXME transfer class
      border: 0
    }},
    ready: function() {
      console.log(this.data);
    }
  }


;module.exports.style = {
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
  },
  "panel-body": {}
}

;module.exports.template = {
  "type": "div",
  "classList": function () {return ['panel', 'panel-' + (this.type)]},
  "style": {
    "borderWidth": function () {return this.border}
  },
  "children": [
    {
      "type": "text",
      "classList": function () {return ['panel-header', 'panel-header-' + (this.type)]},
      "style": {
        "paddingTop": function () {return this.paddingHead},
        "paddingBottom": function () {return this.paddingHead},
        "paddingLeft": function () {return this.paddingHead*1.5},
        "paddingRight": function () {return this.paddingHead*1.5}
      },
      "attr": {
        "value": function () {return this.title}
      }
    },
    {
      "type": "div",
      "classList": function () {return ['panel-body', 'panel-body-' + (this.type)]},
      "style": {
        "paddingTop": function () {return this.paddingBody},
        "paddingBottom": function () {return this.paddingBody},
        "paddingLeft": function () {return this.paddingBody*1.5},
        "paddingRight": function () {return this.paddingBody*1.5}
      },
      "children": [
        {
          "type": "content"
        }
      ]
    }
  ]
}

;})

// module

define('@weex-component/textDemo', function (require, exports, module) {

;

;module.exports.style = {
  "txt": {
    "marginBottom": 12,
    "fontSize": 40
  }
}

;module.exports.template = {
  "type": "scroller",
  "children": [
    {
      "type": "ui-panel",
      "attr": {
        "title": "color"
      },
      "children": [
        {
          "type": "text",
          "classList": [
            "txt"
          ],
          "attr": {
            "value": "default is black"
          }
        },
        {
          "type": "text",
          "classList": [
            "txt"
          ],
          "style": {
            "color": "#286090"
          },
          "attr": {
            "value": "#286090"
          }
        },
        {
          "type": "text",
          "classList": [
            "txt"
          ],
          "style": {
            "color": "#00ff00"
          },
          "attr": {
            "value": "#0f0"
          }
        },
        {
          "type": "text",
          "classList": [
            "txt"
          ],
          "style": {
            "color": "#FF0000"
          },
          "attr": {
            "value": "keyword"
          }
        },
        {
          "type": "text",
          "classList": [
            "txt"
          ],
          "style": {
            "color": "rgb(238,162,54)"
          },
          "attr": {
            "value": "rgb(238, 162, 54)"
          }
        },
        {
          "type": "text",
          "classList": [
            "txt"
          ],
          "style": {},
          "attr": {
            "value": "rgb(238, 162, 54, 0.5)"
          }
        }
      ]
    },
    {
      "type": "ui-panel",
      "attr": {
        "title": "font-size"
      },
      "children": [
        {
          "type": "text",
          "classList": [
            "txt"
          ],
          "style": {
            "fontSize": 32
          },
          "attr": {
            "value": "32"
          }
        },
        {
          "type": "text",
          "classList": [
            "txt"
          ],
          "style": {
            "fontSize": 64
          },
          "attr": {
            "value": "64"
          }
        },
        {
          "type": "text",
          "classList": [
            "txt"
          ],
          "style": {
            "fontSize": 100
          },
          "attr": {
            "value": "100"
          }
        }
      ]
    },
    {
      "type": "ui-panel",
      "attr": {
        "title": "font-style"
      },
      "children": [
        {
          "type": "text",
          "classList": [
            "txt"
          ],
          "style": {
            "fontStyle": "normal"
          },
          "attr": {
            "value": "normal"
          }
        },
        {
          "type": "text",
          "classList": [
            "txt"
          ],
          "style": {
            "fontStyle": "italic"
          },
          "attr": {
            "value": "italic"
          }
        }
      ]
    },
    {
      "type": "ui-panel",
      "attr": {
        "title": "font-weight"
      },
      "children": [
        {
          "type": "text",
          "classList": [
            "txt"
          ],
          "style": {
            "fontWeight": "normal"
          },
          "attr": {
            "value": "normal"
          }
        },
        {
          "type": "text",
          "classList": [
            "txt"
          ],
          "style": {
            "fontWeight": "bold"
          },
          "attr": {
            "value": "bold"
          }
        }
      ]
    },
    {
      "type": "ui-panel",
      "attr": {
        "title": "text-decoration"
      },
      "children": [
        {
          "type": "text",
          "classList": [
            "txt"
          ],
          "style": {
            "textDecoration": "none"
          },
          "attr": {
            "value": "none"
          }
        },
        {
          "type": "text",
          "classList": [
            "txt"
          ],
          "style": {
            "textDecoration": "underline"
          },
          "attr": {
            "value": "underline"
          }
        },
        {
          "type": "text",
          "classList": [
            "txt"
          ],
          "style": {
            "textDecoration": "line-through"
          },
          "attr": {
            "value": "line-through"
          }
        }
      ]
    },
    {
      "type": "ui-panel",
      "attr": {
        "title": "text-align"
      },
      "children": [
        {
          "type": "text",
          "classList": [
            "txt"
          ],
          "style": {
            "textAlign": "left"
          },
          "attr": {
            "value": "left"
          }
        },
        {
          "type": "text",
          "classList": [
            "txt"
          ],
          "style": {
            "textAlign": "center"
          },
          "attr": {
            "value": "center"
          }
        },
        {
          "type": "text",
          "classList": [
            "txt"
          ],
          "style": {
            "textAlign": "right"
          },
          "attr": {
            "value": "right"
          }
        }
      ]
    }
  ]
}

;})

// require module
bootstrap('@weex-component/textDemo', {"transformerVersion":"0.1.8"})