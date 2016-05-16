'use strict'

var timer = {

  setTimeout: function (timeoutCallbackId, delay, callbackId) {
    var sender = this.sender
    var timer = setTimeout(function () {
      sender.performCallback(timeoutCallbackId)
    }, delay)
    sender.performCallback(callbackId, timer)
  },

  clearTimeout: function (timer) {
    clearTimeout(timer)
  }

}

timer._meta = {
  timer: [{
    name: 'setTimeout',
    args: ['function', 'number', 'function']
  }, {
    name: 'clearTimeout',
    args: ['number']
  }]
}

module.exports = timer
