'use strict';

var _ = require('macaca-utils');
var assert = require('chai').assert
var wd = require('weex-wd')
var path = require('path');
var os = require('os');
var util = require("../util.js");

var switchTag = 'switch-obj';

var goal = 'switch-event';
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
      .waitForElementById(switchTag, interval, 2000)
      .click()
      .waitForElementByName('change response:true', interval, 2000)

      .waitForElementById(switchTag, interval, 2000)
      .click()
      .waitForElementByName('change response:false', interval, 2000)
  })
});