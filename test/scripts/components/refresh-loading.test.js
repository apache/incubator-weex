'use strict';

var _ = require('macaca-utils');
var assert = require('chai').assert
var wd = require('weex-wd')
var path = require('path');
var os = require('os');
var util = require("../util.js");

var goal = 'refresh-loading';
describe('weex '+goal+' test', function () {
  this.timeout(util.getTimeoutMills());
  var driver = util.createDriver(wd);

  beforeEach(function () {
    return util.init(driver)
      .get(util.getPage('/components/'+goal+'.js'))
      .waitForElementByName("refresh:hide", util.getGETActionWaitTimeMills(), 2000)
  });

  afterEach(function () {
    return util.quit(driver);
  })

  it('#1 '+goal + ' event', () => {
    return driver
    .drag('toUp',0.4)
    .waitForElementByName("refresh:show", util.getGETActionWaitTimeMills(), 2000)
    .waitForElementByName("hideRefresh").click()
    .waitForElementByName("refresh:hide", util.getGETActionWaitTimeMills(), 2000)

    .drag('toDown',0.4)
    .drag('toDown',0.4)
    .waitForElementByName("loading:show", util.getGETActionWaitTimeMills(), 2000)
    .waitForElementByName("hideLoading").click()
    .waitForElementByName("loading:hide", util.getGETActionWaitTimeMills(), 2000)
  })
});