'use strict';

var _ = require('macaca-utils');
var assert = require('chai').assert
var wd = require('weex-wd')
var path = require('path');
var os = require('os');
var util = require("../util.js");

describe('weex text @ignore-ios', function () {
  this.timeout(util.getTimeoutMills());
  var driver = util.createDriver(wd);

  let scaleFactor = 0
  let screenWidth = 0

  beforeEach(function () {
    return util.init(driver)
      .get(util.getPage('/components/text-layout.js'))
      .sleep(2000)
      .getWindowSize()
      .then(size => {
        screenWidth = size.width
        scaleFactor = screenWidth / 750
      })
  });

  afterEach(function () {
    return util.quit(driver);
  })

  it('#2 Text Content', () => {
    return driver
      .waitForElementById("lines", util.getGETActionWaitTimeMills(), 1000)
      .elementById('text-only')
      .text()
      .then((text) => {
        assert.equal(text, 'Hello World')
      })
  })

  it('#3 Line Height', () => {
    return driver
      .waitForElementById("lines", util.getGETActionWaitTimeMills(), 1000)
      .elementById('lines')
      .getRect()
      .then(rect => {
        assert.equal(rect.width, screenWidth)
        assert.closeTo(rect.height, 32 * scaleFactor * 2, 2)
        return driver.dragUp(rect.height)
      })
  })

  it('#4 Font Size', () => {
    return driver
      .waitForElementById("lines", util.getGETActionWaitTimeMills(), 1000)
      .elementById('font')
      .getRect()
      .then(rect => {
        assert.equal(rect.width, screenWidth)
        assert.isAtLeast(rect.height, Math.floor(2 * 48 * scaleFactor))
        return driver.dragUp(rect.height)
      })
  })

  it('#5 Fixed-Size', () => {
    return driver
      .waitForElementById("lines", util.getGETActionWaitTimeMills(), 1000)
      .elementById('fixed-size')
      .getRect()
      .then(rect => {
        assert.equal(rect.width, Math.floor(300 * scaleFactor))
        assert.equal(rect.height, Math.floor(100 * scaleFactor))
        return driver.dragUp(rect.height)
      })
  })

  it('#6 flex:1; align-Items: stretch; flex-direction:row', () => {
    return driver
      .waitForElementById("lines", util.getGETActionWaitTimeMills(), 1000)
      .elementById('flexgrow-alignitems')
      .getRect()
      .then(rect => {
        assert.equal(rect.width, Math.floor(500 * scaleFactor))
        assert.equal(rect.height, Math.floor(300 * scaleFactor))
        return driver.dragUp(rect.height)
      })
  })

  it('#7 flex:1; align-Items: center; flex-direction:row', () => {
    return driver
      .waitForElementById("lines", util.getGETActionWaitTimeMills(), 1000)
      .elementById('flexgrow')
      .getRect()
      .then(rect => {
        assert.equal(rect.width, Math.floor(500 * scaleFactor))
        assert.closeTo(rect.height, 40 * scaleFactor, 1)
        return driver.dragUp(rect.height)
      })
  })

  // it('#8 flex:1; align-Items: stretch; flex-direction:column', () => {
  //   return driver
  //     .sleep(2000)
  //     .elementById('flexgrow-alignitems-coloumn')
  //     .getRect()
  //     .then(rect => {
  //       assert.equal(rect.width, Math.floor(500 * scaleFactor))
  //       assert.closeTo(rect.height, 300 * scaleFactor, 1)
  //       return driver.dragUp(rect.height)
  //     })
  // })

  // it('#9 flex:1; align-Items: auto; flex-direction:column', () => {
  //   return driver
  //     .sleep(2000)
  //     .elementById('flexgrow-column')
  //     .getRect()
  //     .then(rect => {
  //       assert.isBelow(rect.width, 500 * scaleFactor / 2)
  //       assert.closeTo(rect.height, 300 * scaleFactor, 1)
  //       return driver.dragUp(rect.height)
  //     })
  // })

});