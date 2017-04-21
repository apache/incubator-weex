'use strict'

var path = require('path');
var os = require('os')

var platform = process.env.platform || 'android';
platform = platform.toLowerCase();
var browser = process.env.browser || '';

const isIOS = platform === 'ios';
const isRunInCI = process.env.run_in_ci?true:false;

var iOSOpts = {
  deviceName: 'iPhone 6',
  target: 'ios',
  platformName: 'iOS',
  slowEnv: isRunInCI,
  app: path.join(__dirname, '..', '../ios/playground/build/Debug-iphonesimulator/WeexDemo.app')
};

var androidOpts = {
  platformName: 'Android',
  target: 'android',
  slowEnv: isRunInCI,
  app: path.join(__dirname, '..', `../android/playground/app/build/outputs/apk/playground.apk`)
};

var androidChromeOpts = {
  platformName: 'Android',
  target: 'web',
  browserName: 'Chrome'
};

if(isRunInCI){
    console.log("Running in CI Envirment");
}

function getIpAddress(){
    let ifs = os.networkInterfaces()
    let addresses = ['127.0.0.1'];
    for( var i in ifs){
        let interfaces = ifs[i];
        interfaces.forEach((face)=>{
            if(!face.internal && face.family == 'IPv4'){
                addresses.unshift(face.address);
            }
        })
    }
    return addresses[0];
}


module.exports = {
    getConfig:function(){
        if(browser){
            return androidChromeOpts;
        }
        return isIOS? iOSOpts : androidOpts;
    },
    getDeviceHost:function(){
        return getIpAddress()+":12581";
    },
    getPage:function(name){
        let url
        if(browser){
             url = 'http://'+ getIpAddress()+':12581/vue.html?page=/test/build-web'+name
        }else{
            url = 'wxpage://' + getIpAddress()+":12581/test/build"+name;
        }
        console.log(url)
        return url
    },
    getTimeoutMills:function(){
        return ( isRunInCI ? 60 : 10 ) * 60 * 1000;
    },
    getGETActionWaitTimeMills:function(){
        return (isRunInCI ? 120 : 5 ) * 1000;
    },
    createDriver:function(wd){
        var driver = global._wxDriver;
        if(!driver){
            console.log('Create new driver');
            driver = wd(this.getConfig()).initPromiseChain();
            driver.configureHttp({
                timeout: 100000
            });
            global._wxDriver = driver;
        }
        
        return driver;
    },
    init:function(driver){
        if(driver._isInit)
            return driver.status()
        else{
            driver._isInit = true;
            return driver.initDriver()
        }
    },
    quit:function(driver){
        if(browser)
            return driver.quit()
        return driver.sleep(1000).back().sleep(1000);
    }
}
