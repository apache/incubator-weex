'use strict';

var _ = require('macaca-utils');
var assert = require('chai').assert
var wd = require('weex-wd')
var path = require('path');
var os = require('os');
var util = require("./util.js");

describe('weex mobile index', function () {
  this.timeout(util.getTimeoutMills());
  var driver = util.createDriver(wd);

  before(function () {
    return util.init(driver)
      .get('wxpage://' + util.getDeviceHost() +'/index.js')
      .waitForElementByXPath('//div/text[1]',util.getGETActionWaitTimeMills(),1000);
  });

  after(function () {
      return util.quit(driver);
  })


  it('#1 Index', () => {
    return driver
    .elementByXPath('//div/text[1]')
    .text()
    .then((text)=>{
      assert.equal(text,'hello world.')
    })
  })

  it('#2 Click Button', () => {
    return driver
    .elementByXPath('//div/text[3]')
    .click()
    .elementByXPath('//div/text[2]')
    .text()
    .then((text)=>{
      assert.equal(text,'btn click.')
    })
  })

  it('#3 Input Blur', () => {
    return driver
    .elementByXPath('//div/input')
    .click()
    .elementByXPath('//div/text[4]')
    .click()
    .elementByXPath('//div/text[2]')
    .text()
    .then((text)=>{
      assert.equal(text,'input blur.')
    })
  })

  
});
