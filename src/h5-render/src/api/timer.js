'use strict'

var timer = {

  setTimeout: function (callbackId, delay) {
    var sender = this.sender
    var timer = setTimeout(function () {
      sender.performCallback(callbackId, timer)
    }, delay)
  },

  clearTimeout: function (timer) {
    clearTimeout(timer)
  }

}

timer._meta = {
  timer: [{
    name: 'setTimeout',
    args: ['function', 'number']
  }, {
    name: 'clearTimeout',
    args: ['number']
  }]
}

module.exports = timer
