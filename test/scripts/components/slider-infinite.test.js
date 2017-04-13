'use strict';

var _ = require('macaca-utils');
var assert = require('chai').assert
var wd = require('weex-wd')
var path = require('path');
var os = require('os');
var util = require("../util.js");

describe('slider infinite scroll', function () {
  this.timeout(util.getTimeoutMills());
  var driver = util.createDriver(wd);

  before(function () {
    return util.init(driver)
      .get(util.getPage('/slider-infinite.js'))
      .waitForElementById('txt1',util.getGETActionWaitTimeMills(),1000)
  });

  after(function () {
      // return util.quit(driver);
  })


  it('#1 waiting for auto play', () => {
    return driver
    .sleep(5000)
    .elementById('txt1')
    .text()
    .then((text)=>{
        assert.equal(text, 1);
    })
    .elementById('txt2')
    .text()
    .then((text)=>{
        assert.equal(text>2, true);
    })
  })
});


