'use strict'

var sender = require('../bridge/sender')

var timer = {

  setTimeout: function (callbackId, delay) {
    var timer = setTimeout(function () {
      sender.performCallback(callbackId)
    }, delay)
  },

  clearTimeout: function (callbackId) {
    var sender =  this.sender
    config.callback = function () {
      sender.performCallback(callbackId)
    }
    modal.alert(config)
  }

}

timer._meta = {
  modal: [{
    name: 'setTimeout',
    args: ['function', 'number']
  }, {
    name: 'clearTimeout',
    args: ['function']
  }]
}

module.exports = timer
