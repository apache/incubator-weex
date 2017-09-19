'use strict';

var _ = require('macaca-utils');
var assert = require('chai').assert
var wd = require('weex-wd')
var path = require('path');
var os = require('os');
var util = require("../util.js");


var goal = 'gesture-longpress';
var timeout = util.getGETActionWaitTimeMills();
describe('weex '+goal+' test', function () {
  this.timeout(util.getTimeoutMills());
  var driver = util.createDriver(wd);

  beforeEach(function () {
    return util.init(driver)
      .get(util.getPage('/modules/'+goal+'.js'))
  });

  afterEach(function () {
    return util.quit(driver);
  })

  it('#1 '+goal + ' event', () => {
    return driver
      .waitForElementByName(goal, timeout, 2000)
      .waitForElementByName('longpress', timeout, 2000)
      .touch('press', { duration: 2 })
      .sleep(1000)
      .waitForElementById('result',timeout,1000)
      .text()
      .then((text) => {
        // assert.include(text, 'screenY')
      })
  })
});