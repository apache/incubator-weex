'use strict';

var _ = require('macaca-utils');
var assert = require('chai').assert
var wd = require('weex-wd')
var path = require('path');
var os = require('os');
var util = require("../util.js");


var goal = 'video-property';
var interval = util.getGETActionWaitTimeMills();
describe('weex '+goal+' test', function () {
  this.timeout(util.getTimeoutMills());
  var driver = util.createDriver(wd);

  beforeEach(function () {
    return util.init(driver)
      .get(util.getPage('/components/'+goal+'.js'))
      .waitForElementByName(goal, interval, 2000)
  });

  afterEach(function () {
    return util.quit(driver);
  })

  it('#1 '+goal + ' event', () => {
    return driver
      // .waitForElementByName("video-fail", interval, 2000) //android未触发
      .waitForElementByName("setSrc", interval, 2000)
      .click()
      // .waitForElementByName("video-pause", interval, 2000) //android未触发
      .sleep(1000)
      .waitForElementByName('setPlay', interval, 2000)
      .click()
      .waitForElementByName("video-start", interval, 2000)
      .waitForElementByName('setPause', interval, 2000)
      .click()
      .waitForElementByName("video-pause", interval, 2000)
      .waitForElementByName('setPlay', interval, 2000)
      .click()
      .sleep(5000)
      .waitForElementByName('video-finish', interval, 2000)
  })
});