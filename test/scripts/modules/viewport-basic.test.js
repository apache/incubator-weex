'use strict';

var _ = require('macaca-utils');
var assert = require('chai').assert
var wd = require('weex-wd')
var path = require('path');
var os = require('os');
var util = require("../util.js");


var goal = 'viewport-basic';
var timeout = util.getGETActionWaitTimeMills();
describe('weex '+goal+' test', function () {
  this.timeout(util.getTimeoutMills());
  var driver = util.createDriver(wd);

  let scaleFactor = 0
  let screenWidth = 0

  beforeEach(function () {
    return util.init(driver)
      .get(util.getPage('/modules/'+goal+'.js'))
      .sleep(2000)
      .getWindowSize()
      .then(size => {
        screenWidth = size.width
        scaleFactor = screenWidth / 750
      })
  });

  afterEach(function () {
    return util.quit(driver);
  })

  it('#1 '+goal + ' test', () => {
    return driver
      .waitForElementByName(goal, timeout, 2000)
      .waitForElementById('test-point', timeout, 2000)
      .getRect().then((rect)=>{
        var width = rect.width;
        console.log('width:'+width+', screenWidth:'+screenWidth)
        assert.closeTo(width, screenWidth/2, 2);
      });
  })
})