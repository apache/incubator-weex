'use strict';

var _ = require('macaca-utils');
var wd = require('./wdconfig.js')
var WEEX = require('./utility.js')


wd.addPromiseChainMethod('customback', function () {
  if (platform === 'ios') {
    return this.elementByName('back').sleep(1000).click().sleep(1000)
  }

  return this
    .back();
});

describe('weex mobile index', function () {
  this.timeout(5 * 60 * 1000);

  var driver = wd.initPromiseChain();

  var weex = WEEX("wxpage", driver);

  var windowSize = driver.getWindowSize()
  driver.configureHttp({
    timeout: 100000
  });

  before(function () {
    let url = weex.page("index.js");//same name as the weex script
    return driver
      .initDriver()
      .get(url)
      .sleep(1000);
  });


  it('#1 Index', () => {
    return driver.sleep(2000).elementByName('hello world.')
  })

  
});
