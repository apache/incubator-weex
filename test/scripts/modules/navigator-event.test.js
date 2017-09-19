'use strict';

var _ = require('macaca-utils');
var assert = require('chai').assert
var wd = require('weex-wd')
var path = require('path');
var os = require('os');
var util = require("../util.js");


var goal = 'navigator-event';
var timeout = util.getGETActionWaitTimeMills();
describe('weex '+goal+' test', function () {
  this.timeout(util.getTimeoutMills());
  var driver = util.createDriver(wd);

  beforeEach(function () {
    return util.init(driver)
      .get(util.getPage('/modules/'+goal+'.js'))
  });

  afterEach(function () {
    return util.quitWithoutBack(driver);
  })

  it('#1 '+goal + ' event', () => {
    return driver
    .waitForElementByName(goal, timeout, 2000)
    .waitForElementByName('Navigator')
    .waitForElementByName('push')
    .click()
    .waitForElementByName('pop')
    .click()
    .waitForElementByName('push')
    .click()
    //open，close方法未实现
    // .waitForElementByName('open')
    // .click()
    // .waitForElementByName('push')
    // .click()
    // .waitForElementByName('close')
    // .click()
    // .waitForElementByName('push')
    .waitForElementByName('Left')
    .click()
    .waitForElementByName('naviBarLeftItemClick')
    .waitForElementByName('Right')
    .click()
    .waitForElementByName('naviBarRightItemClick');
  })
});
