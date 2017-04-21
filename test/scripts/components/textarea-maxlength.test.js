'use strict';

var _ = require('macaca-utils');
var assert = require('chai').assert
var wd = require('weex-wd')
var path = require('path');
var os = require('os');
var util = require("../util.js");

describe('textarea maxlength vue test2 ', function () {
  this.timeout(util.getTimeoutMills());
  var driver = util.createDriver(wd);

  before(function () {
    return util.init(driver)
      .get(util.getPage('/components/textarea-maxlength.js'))
      .waitForElementByXPath('//div/text[1]',util.getGETActionWaitTimeMills(),1000)
  });

  after(function () {
      return util.quit(driver)
  })

  it('#1 textarea maxlenght', () => {
    return driver
      .waitForElementByXPath('//div/textarea')
      .sendKeys('12345678')
      .sleep(2000)
      .elementByXPath('//div/text[1]')
      .text()
      .then((text)=>{
      assert.equal(text,'1234')
     })
  })

});