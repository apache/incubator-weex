'use strict';

var _ = require('macaca-utils');
var assert = require('chai').assert
var wd = require('weex-wd')
var path = require('path');
var os = require('os');
var util = require("../util.js");

describe('list scroll event', function () {
  this.timeout(util.getTimeoutMills());
  var driver = util.createDriver(wd);

  before(function () {
    return util.init(driver)
      .get('wxpage://' + util.getDeviceHost() +'/list-scroll.js')
      .waitForElementByXPath('//div/text[1]',util.getGETActionWaitTimeMills(),1000)
  });

  after(function () {
      return util.quit(driver);
  })


  it('#1 Drag list', () => {
    return driver
    .touch('drag', {
        fromX: 200,
        fromY: 500,
        toX: 200,
        toY: 200,
        duration: 0.5
    })
    .sleep(2000)
    .touch('drag', {
      fromX:200, 
      fromY:600, 
      toX:100, 
      toY:200,
      duration: 0.5
    })
    .sleep(2000)
    .elementByXPath('//div/text')
    .text()
    .then((text)=>{
      var y = Math.abs(parseInt(text))
      assert.equal(y > 200,true)
    })
  })
});

describe('scroller scroll event', function () {
  this.timeout(util.getTimeoutMills());
  var driver = util.createDriver(wd);

  before(function () {
    return util.init(driver)
      .get('wxpage://' + util.getDeviceHost() +'/scroller-scroll.js')
      .waitForElementByXPath('//div/text[1]',util.getGETActionWaitTimeMills(),1000)
  });

  after(function () {
      return util.quit(driver)
  })


  it('#1 Drag scroller', () => {
    return driver
    .touch('drag', {fromX:200, fromY:500, toX:200, toY: 200})
    .sleep(2000)
    .touch('drag', {fromX:200, fromY:500, toX:100, toY:200})
    .sleep(2000)
    .elementByXPath('//div/text')
    .text()
    .then((text)=>{
      var y = Math.abs(parseInt(text))
      assert.equal(y > 200,true)
    })
  })
});


