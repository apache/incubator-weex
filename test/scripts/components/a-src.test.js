'use strict';

var _ = require('macaca-utils');
var assert = require('chai').assert
var wd = require('weex-wd')
var path = require('path');
var os = require('os');
var util = require("../util.js");

describe('a-src-test', function () {
  this.timeout(util.getTimeoutMills());
  var driver = util.createDriver(wd);
  var maxWt = util.getGETActionWaitTimeMills();
  beforeEach(function () {
    return util.init(driver)
      .get(util.getPage('/components/a-src.js'))
  });

  afterEach(function () {
    return util.quit(driver);
  })

  it('#1 Click A tag itself', () => {
    return driver
      .waitForElementByName("a-src",maxWt,2000)
      .waitForElementById('a-itself', maxWt, 2000)
      .click()
      .waitForElementByName('a-support-href1', maxWt, 2000)
      .waitForElementByName('jump_back', maxWt, 2000)
      .click();
  })

  it('#2 Click some content inside a', () => {
    return driver
      .waitForElementByName('Jump', maxWt, 2000)
      .click()
      .waitForElementByName('a-support-href1', maxWt, 2000)
      .waitForElementByName('jump_back', maxWt, 2000)
      .click();
  })

  it('#3 change src', () => {
    return driver
      .waitForElementByName('changeSrc', maxWt, 2000)
      .click()
      .sleep(500)
      .waitForElementById('a-itself', maxWt, 2000)
      .click()
      .waitForElementByName('a-support-href2', maxWt, 2000)
      .waitForElementByName('jump_back', maxWt, 2000)
      .click();
  })

  it('#4 JumpBlank', () => {
    return driver
      .waitForElementByName('changeSrc', maxWt, 2000)
      .click()
      .sleep(500)
      .waitForElementByName('JumpBlank', maxWt, 2000)
      .click()
      .waitForElementByName('a-support-href1.js?_wxpage=true', maxWt, 2000)
      .waitForElementByName('jump_back', maxWt, 2000)
      .click();
  })
});