'use strict';

var os = require("os");

/**
 * Weex Utilities.
 */
function Weex(scheme,webdriver){
    this.wd = webdriver;
    this.host = scheme+"://"+getIpAddress()+":12581";
}

function getIpAddress(){
    let ifs = os.networkInterfaces()
    let addresses = ["127.0.0.1"];
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

Weex.prototype.page = function(path){
    return this.host+"/"+path;
}

module.exports = function(scheme,webdriver){
        return new Weex(scheme,webdriver)
    }

