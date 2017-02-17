'use strict';

var _ = require('macaca-utils');
var assert = require('chai').assert
var wd = require('weex-wd')
var path = require('path');
var os = require('os');
var util = require("../util.js");

describe('weex mobile index', function () {
  this.timeout(1 * 60 * 1000);
  var driver = wd(util.getConfig()).initPromiseChain();
  driver.configureHttp({
    timeout: 100000
  });

  before(function () {
    return driver
      .initDriver()
      .get('wxpage://' + util.getDeviceHost() +'/list-scroll.js')
      .sleep(1000);
  });


  it('#1 Drag list', () => {
    return driver
    .touch('drag', {
        fromX: 200,
        fromY: 500,
        toX: 200,
        toY: 200,
        duration: 0.5
    })
    .sleep(2000)
    .touch('drag', {
      fromX:200, 
      fromY:600, 
      toX:100, 
      toY:200,
      duration: 0.5
    })
    .elementByXPath('//div/text')
    .text()
    .then((text)=>{
      var y = Math.abs(parseInt(text))
      assert.equal(y > 400,true)
    })
  })
});