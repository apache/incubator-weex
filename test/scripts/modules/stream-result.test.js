'use strict';

var _ = require('macaca-utils');
var assert = require('chai').assert
var wd = require('weex-wd')
var path = require('path');
var os = require('os');
var util = require("../util.js");

var goal = 'stream-result';

describe('stream-result-test', function () {
  this.timeout(util.getTimeoutMills());
  var driver = util.createDriver(wd);
  var maxWt = util.getGETActionWaitTimeMills();
  beforeEach(function () {
    return util.init(driver)
      .get(util.getPage('/modules/stream-result.js'))
  });

  afterEach(function () {
    return util.quit(driver);
  })

  it('#1 ' + 'stream-result' + ' event', () => {
    return driver.waitForElementById('streamGet', maxWt, 2000)
    .click()
    .waitForElementByName('Completed', maxWt, 10000);
  })
});