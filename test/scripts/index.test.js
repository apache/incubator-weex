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
      .get(util.getPage('/index.js'))
      .waitForElementById('title',util.getGETActionWaitTimeMills(),1000);
  });

  after(function () {
      return util.quit(driver);
  })


  it('#1 Index', () => {
    return driver
    .elementById('title')
    .text()
    .then((text)=>{
      assert.equal(text,'hello world.')
    })
  })

  it('#2 Click Button', () => {
    return driver
    .elementById('button')
    .click()
    .elementById('status')
    .text()
    .then((text)=>{
      assert.equal(text,'btn click.')
    })
  })

  it('#3 Input Blur', () => {
    return driver
    .elementById('input')
    .click()
    .elementById('button2')
    .click()
    .elementById('status')
    .text()
    .then((text)=>{
      assert.equal(text,'input blur.')
    })
  })

  
});
