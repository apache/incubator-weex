'use strict';

var _ = require('macaca-utils');
var assert = require('chai').assert
var wd = require('weex-wd')
var path = require('path');
var os = require('os');
var util = require("../util.js");

describe('image onload', function () {
  this.timeout(util.getTimeoutMills());
  var driver = util.createDriver(wd);

  before(function () {
    return util.init(driver)
      .get('wxpage://' + util.getDeviceHost() +'/image-onload.js')
      .waitForElementByXPath('//div/text[1]',util.getGETActionWaitTimeMills(),1000)
  });

  after(function () {
      return util.quit(driver);
  })


  it('#1 download image', () => {
    return driver
    .sleep(5000)
    .elementByXPath('//div/text')
    .text()
    .then((text)=>{
        if(text == '-1,-1') {
            return;
        }
        assert.equal(text, '360,388')
    })
  })
});


