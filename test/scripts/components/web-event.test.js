'use strict';

var _ = require('macaca-utils');
var assert = require('chai').assert
var wd = require('weex-wd')
var path = require('path');
var os = require('os');
var util = require("../util.js");

var goal = 'web-event';
var maxW = util.getGETActionWaitTimeMills();
// android has some bug
describe('weex '+goal+' test @ignore-android', function () {
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
      .waitForElementByName(goal, maxW, 2000)
      .waitForElementByName('txtStart:https://www.baidu.com/', maxW, 2000)
      .waitForElementByName('txtFinish:https://www.baidu.com/', maxW, 2000)
      .waitForElementByName('txtError:http://www.error.com/', maxW, 2000)
      .waitForElementByName('reload', maxW, 2000)
      .click().sleep(500)
      .waitForElementByName('txtFinish:https://www.baidu.com/', maxW, 2000)
      .waitForElementByName('goBack', maxW, 2000)
      .click()
      .waitForElementByName('txtStart:https://h5.m.taobao.com/#index', maxW, 2000)
      .waitForElementByName('txtFinish:https://h5.m.taobao.com/#index', maxW, 2000)
      .waitForElementByName('goForward', maxW, 2000)
      .click().sleep(500)
      .waitForElementByName('txtStart:https://www.baidu.com/', maxW, 2000)
      .waitForElementByName('txtFinish:https://www.baidu.com/', maxW, 2000)
  })
});