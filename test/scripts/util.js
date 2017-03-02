'use strict'

var path = require('path');
var os = require('os')

var platform = process.env.platform || 'android';
platform = platform.toLowerCase();

const isIOS = platform === 'ios';
const isRunInCI = process.env.run_in_ci?true:false;

var iOSOpts = {
  deviceName: 'iPhone 6',
  platformName: 'iOS',
  slowEnv: isRunInCI,
  app: path.join(__dirname, '..', '../ios/playground/build/Debug-iphonesimulator/WeexDemo.app')
};

var androidOpts = {
  platformName: 'Android',
  slowEnv: isRunInCI,
  app: path.join(__dirname, '..', `../android/playground/app/build/outputs/apk/playground.apk`)
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
        return isIOS? iOSOpts : androidOpts;
    },
    getDeviceHost:function(){
        return getIpAddress()+":12581";
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
        return driver.sleep(1000).back().sleep(1000);
    }
}
