'use strict';

var _ = require('macaca-utils');
var assert = require('chai').assert
var wd = require('weex-wd')
var path = require('path');
var os = require('os');
var util = require("../util.js");


var goal = 'input-event';
var interval = util.getGETActionWaitTimeMills();
describe('weex '+goal+' test', function () {
  this.timeout(util.getTimeoutMills());
  var driver = util.createDriver(wd);

  beforeEach(function () {
    return util.init(driver)
      .get(util.getPage('/components/'+goal+'.js'))
  });

  afterEach(function () {
    return util.quit(driver);
  })

  it('#1 '+goal + ' event', () => {
    return driver
      .waitForElementByName(goal, interval, 2000)
      .waitForElementById("input-obj", interval, 2000)
      .sendKeys('testInput')
      .sleep(500)
      .waitForElementByName('input:testInput', interval, 2000)


  })
});