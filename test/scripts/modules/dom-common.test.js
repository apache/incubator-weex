'use strict';

var _ = require('macaca-utils');
var assert = require('chai').assert
var wd = require('weex-wd')
var path = require('path');
var os = require('os');
var util = require("../util.js");

var goal = 'dom-common';
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

  it('#1 '+goal + ' common opt', () => {
    return driver
      .waitForElementByName(goal, timeout, 2000)
      .waitForElementByName('show text', timeout, 2000)
      .waitForElementByName('Zechang-Xie', timeout, 2000)
      .waitForElementByName('runMethod', timeout, 2000)
      .waitForElementByName('promise support', timeout, 2000)
      //TODO rect compare
      .waitForElementByName('styleChange', timeout, 2000)
      .waitForElementByName('classChange', timeout, 2000)
  })
});