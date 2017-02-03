'use strict';

var _ = require('macaca-utils');
var assert = require('chai').assert
var wd = require('weex-wd')
var path = require('path');
var os = require('os');
var util = require("./util.js");

describe('weex mobile index', function () {
  this.timeout(1 * 30 * 1000);

  var driver = wd(util.getConfig()).initPromiseChain();

  var windowSize = driver.getWindowSize();

  driver.configureHttp({
    timeout: 100000
  });
  // driver.initDriver()

  before(function () {
    console.log("run before");
    return driver
      .initDriver()
      // .sleep(1000)
      // .get('wxpage://' + util.getDeviceHost() +'/index.js')
      .sleep(1000);
  });


  it('#1 Index', () => {
    return driver
    .get('wxpage://' + util.getDeviceHost() +'/index.js').sleep(1000)
    .elementByXPath('//div/text[1]')
    .text()
    .then((text)=>{
      assert.equal(text.description,'hello world.')
    })
  })

  it('#2 Click Button', () => {
    return driver
    .elementByXPath('//div/text[3]')
    .click()
    .elementByXPath('//div/text[2]')
    .text()
    .then((text)=>{
      assert.equal(text.description,'btn click.')
    })
  })

  it('#2 Input Blur', () => {
    return driver
    .elementByXPath('//div/input')
    .click()
    .elementByXPath('//div/text[4]')
    .click()
    .elementByXPath('//div/text[2]')
    .text()
    .then((text)=>{
      assert.equal(text.description,'input blur.')
    })
  })

  
});