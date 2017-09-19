'use strict';

var _ = require('macaca-utils');
var assert = require('chai').assert
var wd = require('weex-wd')
var path = require('path');
var os = require('os');
var util = require("../util.js");

var goal = 'dom-update';
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

  it('#1 '+goal + ' event,add and remove dom', () => {
    return driver
      .waitForElementByName(goal, timeout, 2000)
      .waitForElementByName('concat', timeout, 2000)
      .click()
      .waitForElementByName('List2-0', timeout, 2000)
      .waitForElementByName('List2-4', timeout, 2000)
      .waitForElementByName('push', timeout, 2000)
      .click()
      .waitForElementByName('List-5', timeout, 2000)
      .waitForElementByName('unshift', timeout, 2000)
      .click()
      .waitForElementByName('List-unshift-6', timeout, 2000)
      //TODO 判断view高度
      .waitForElementByName('shift', timeout, 2000)
      .click().sleep(1000)
      .elementByNameOrNull('List-unshift-6').then((els)=>{
        assert.equal(els, null);
        return driver.waitForElementByName('pop', timeout, 2000)
          .click().sleep(1000)
          .elementByNameOrNull('List-5').then((els1)=>{
            assert.equal(els1, null);
            return driver;
          })
      })
  })
});