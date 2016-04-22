define('@weex-component/calculator', function (require, exports, module) {

;
  module.exports = {
    data: function () {return {
      result: ''
    }},
    methods: {
      getResult: function () {
        return this.result;
      },
      input: function (e) {
        var value = e.target.attr["value"];
        this.result = this.result + value;
      },
      calculate: function(e) {
        var result = eval(this.result);
        this.result = this.result + " = " + result;
      },
      clear: function(e) {
        this.result = "";
      }
    }
  }


;module.exports.style = {
  "row": {
    "flexDirection": "row"
  },
  "result": {
    "color": "#FFFFFF",
    "backgroundColor": "#666666",
    "textAlign": "right",
    "height": 100,
    "padding": 30,
    "margin": 5
  },
  "btn": {
    "flex": 1,
    "textAlign": "center",
    "backgroundColor": "#eeeeee",
    "height": 100,
    "padding": 30,
    "margin": 5
  },
  "btn-operator": {
    "color": "#FFFFFF",
    "backgroundColor": "#666699"
  }
}

;module.exports.template = {
  "type": "div",
  "style": {
    "padding": 5
  },
  "children": [
    {
      "type": "text",
      "classList": [
        "result"
      ],
      "attr": {
        "value": function () {return this.getResult()}
      }
    },
    {
      "type": "div",
      "classList": [
        "row"
      ],
      "children": [
        {
          "type": "text",
          "classList": [
            "btn"
          ],
          "events": {
            "click": "input"
          },
          "attr": {
            "value": "1"
          }
        },
        {
          "type": "text",
          "classList": [
            "btn"
          ],
          "events": {
            "click": "input"
          },
          "attr": {
            "value": "2"
          }
        },
        {
          "type": "text",
          "classList": [
            "btn"
          ],
          "events": {
            "click": "input"
          },
          "attr": {
            "value": "3"
          }
        },
        {
          "type": "text",
          "classList": [
            "btn",
            "btn-operator"
          ],
          "events": {
            "click": "input"
          },
          "attr": {
            "value": "+"
          }
        }
      ]
    },
    {
      "type": "div",
      "classList": [
        "row"
      ],
      "children": [
        {
          "type": "text",
          "classList": [
            "btn"
          ],
          "events": {
            "click": "input"
          },
          "attr": {
            "value": "4"
          }
        },
        {
          "type": "text",
          "classList": [
            "btn"
          ],
          "events": {
            "click": "input"
          },
          "attr": {
            "value": "5"
          }
        },
        {
          "type": "text",
          "classList": [
            "btn"
          ],
          "events": {
            "click": "input"
          },
          "attr": {
            "value": "6"
          }
        },
        {
          "type": "text",
          "classList": [
            "btn",
            "btn-operator"
          ],
          "events": {
            "click": "input"
          },
          "attr": {
            "value": "-"
          }
        }
      ]
    },
    {
      "type": "div",
      "classList": [
        "row"
      ],
      "children": [
        {
          "type": "text",
          "classList": [
            "btn"
          ],
          "events": {
            "click": "input"
          },
          "attr": {
            "value": "7"
          }
        },
        {
          "type": "text",
          "classList": [
            "btn"
          ],
          "events": {
            "click": "input"
          },
          "attr": {
            "value": "8"
          }
        },
        {
          "type": "text",
          "classList": [
            "btn"
          ],
          "events": {
            "click": "input"
          },
          "attr": {
            "value": "9"
          }
        },
        {
          "type": "text",
          "classList": [
            "btn",
            "btn-operator"
          ],
          "events": {
            "click": "input"
          },
          "attr": {
            "value": "*"
          }
        }
      ]
    },
    {
      "type": "div",
      "classList": [
        "row"
      ],
      "children": [
        {
          "type": "text",
          "classList": [
            "btn"
          ],
          "events": {
            "click": "input"
          },
          "attr": {
            "value": "0"
          }
        },
        {
          "type": "text",
          "classList": [
            "btn"
          ],
          "events": {
            "click": "input"
          },
          "attr": {
            "value": "."
          }
        },
        {
          "type": "text",
          "classList": [
            "btn"
          ],
          "events": {
            "click": "clear"
          },
          "attr": {
            "value": "AC"
          }
        },
        {
          "type": "text",
          "classList": [
            "btn",
            "btn-operator"
          ],
          "events": {
            "click": "calculate"
          },
          "attr": {
            "value": "="
          }
        }
      ]
    }
  ]
}

;})

// require module
bootstrap('@weex-component/calculator', {"transformerVersion":"0.1.8"})