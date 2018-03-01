'use strict';

var _ = require('macaca-utils');
var assert = require('chai').assert
var wd = require('weex-wd')
var path = require('path');
var os = require('os');
var util = require("../util.js");

do_test('Time Module in .vue', '/modules/vue_timer.js');

function do_test(test_title, url) {
  describe(test_title, function () {
    this.timeout(util.getTimeoutMills());
    var driver = util.createDriver(wd);

    beforeEach(function () {
      return util.init(driver)
        .get(util.getPage(url))
    });

    afterEach(function () {
      return util.quit(driver);
    })

    it('#1 SetTimeOut', () => {
      return driver
        .waitForElementById("interval", util.getGETActionWaitTimeMills(), 1000)
        .elementById('setTimeout')
        .click()
        .sleep(4000)
        .click()
        .sleep(4000)
        .elementById('timeout')
        .text()
        .then(text => {
          assert.equal(text, 2)
        })
    })

    it('#2 SetTimeOut Double click', () => {
      return driver
        .waitForElementById("interval", util.getGETActionWaitTimeMills(), 1000)
        .elementById('setTimeout')
        .click()
        .click()
        .sleep(4000)
        .elementById('timeout')
        .text()
        .then(text => {
          assert.equal(text, 2)
        })
    })

    it('#3 ClearTimeOut', () => {
      return driver
        .waitForElementById("interval", util.getGETActionWaitTimeMills(), 1000)
        .elementById('setTimeout')
        .click()
        .elementById('clearTimeout')
        .click()
        .sleep(4000)
        .elementById('timeout')
        .text()
        .then(text => {
          assert.equal(text, 0)
        })
    })

    let previous = 0
    it('#4 SetInterval ClearInterval', () => {
      return driver
        .waitForElementById("interval", util.getGETActionWaitTimeMills(), 1000)
        .elementById('setInterval')
        .click()
        .sleep(7000)
        .elementById('interval')
        .text()
        .then(text => {
          assert.equal(text, 2)
          return driver
            .elementById('clearInterval')
            .click()
            .elementById('interval')
            .text()
            .then(text => {
              previous = text
            })
            .sleep(4000)
            .elementById('interval')
            .text()
            .then(text => {
              assert.equal(previous, text)
            })
        })
    })
  })
}