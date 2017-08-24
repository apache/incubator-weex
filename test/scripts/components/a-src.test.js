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
      .waitForElementByName("a-src",maxWt,2000)
  });

  afterEach(function () {
    return util.quit(driver);
  })

  click_a_tag('#1 Click A tag itself', 'a-itself')
  click_a_tag('#2 Click some content inside a', 'content-inside-a')

  function click_a_tag(title, id) {
    it(title, () => {
      return driver
        .waitForElementById(id, maxWt, 2000)
        .click()
        .waitForElementByName('a-support-href1', maxWt, 2000)
        .waitForElementByName('jump_back', maxWt, 2000)
        .click();
    })
  }

  it('change src', () => {
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
});