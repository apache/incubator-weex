Weex.define('@weex-component/foo', function (require, exports, module) {
  module.exports = {
    data: function () {
      return {
        x: Weex.config.env.appName
      }
    },
    template: {
      "type": "div",
      "attr": {
        "a": function () {
          return this.x
        }
      }
    }
  }
})

Weex.bootstrap('@weex-component/foo')
