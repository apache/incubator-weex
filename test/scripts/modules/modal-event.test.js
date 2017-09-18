'use strict';

var _ = require('macaca-utils');
var assert = require('chai').assert
var wd = require('weex-wd')
var path = require('path');
var os = require('os');
var util = require("../util.js");

var goal = 'modal-event';
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

  it('#1 '+goal + ' event', () => {
    return driver
      .waitForElementByName('toast', timeout, 2000)
      .click()
      .sleep(2000)
      .waitForElementByName('alert', timeout, 2000)
      .click()
      .sleep(1000)
      .dismissAlert()
      .waitForElementByName('confirm', timeout, 2000)
      .click()
      .sleep(1000)
      .dismissAlert()
      .waitForElementByName('prompt', timeout, 2000)
      .click()
      .sleep(1000)
      .dismissAlert()
      .waitForElementByName('Prompt success', timeout, 2000)
  })
});