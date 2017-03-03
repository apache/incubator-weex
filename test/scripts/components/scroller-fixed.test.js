'use strict';

var _ = require('macaca-utils');
var assert = require('chai').assert
var wd = require('weex-wd')
var path = require('path');
var os = require('os');
var util = require("../util.js");

describe('scroller fixed position item ', function () {
  this.timeout(util.getTimeoutMills());
  var driver = util.createDriver(wd);

  before(function () {
    return util.init(driver)
      .get('wxpage://' + util.getDeviceHost() +'/components/scroller-fixed.js')
      .waitForElementByXPath('//scroller[1]/div[1]',util.getGETActionWaitTimeMills(),1000)
  });

  after(function () {
      return util.quit(driver)
  })


  it('#1 position:fixed items', () => {
    return driver
    .touch('drag', {fromX:200, fromY:500, toX:200, toY: 400})
    .sleep(2000)
    .touch('drag', {fromX:200, fromY:400, toX:200, toY: 500})
    .sleep(2000)
    .elementByXPath('//scroller[1]/div[1]/text[1]')
    .text()
    .then((text)=>{
        var parts = text.split("|");
        assert.equal(parts[0],parts[1]);
        assert.equal(parts[2],0);
    })
  })
});