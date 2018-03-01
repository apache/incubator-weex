'use strict';

var _ = require('macaca-utils');
var assert = require('chai').assert
var wd = require('weex-wd')
var path = require('path');
var os = require('os');
var util = require("./util.js");

var goal = 'cover-generate';
var timeout = util.getGETActionWaitTimeMills();
describe('weex '+goal+' test @ignore-android', function () {
  this.timeout(util.getTimeoutMills());
  var driver = util.createDriver(wd);

  beforeEach(function () {
    return util.init(driver)
      .get(util.getPage('/modules/'+goal+'.js'))
      .waitForElementByName(goal, timeout, 2000)
  });

  afterEach(function () {
    return util.quit(driver);
  })

  it('#1 '+goal + ' event', () => {
    //TODO ：截图比对
    return driver.waitForElementByName('generate', timeout, 2000)
      .click()
      .waitForElementByName('coverage success', timeout, 2000) 
  })
});