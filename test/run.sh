#!/bin/bash -eu
set -e
port="${serport:-12581}"

function startMacacaServer {
    macaca server --verbose &
    while ! nc -z 127.0.0.1 3456; do sleep 5; done
}

function startWeexServer {
    echo "local serve at port:$port"
    while ! nc -z 127.0.0.1 $port; do sleep 5; done
}

function buildAndroid {
    dir=$(pwd)
    builddir=$dir'/android'
    current_dir=$PWD;
    cd $builddir;
    ./gradlew assembleDebug;
    cd $current_dir;
    pwd
}
function runAndroid {
    buildAndroid
    startMacacaServer
    startWeexServer
    platform=android ./node_modules/mocha/bin/mocha  $1 -f '@ignore-android' -i --recursive --bail
}

function buildiOS {
    builddir=$(pwd)'/ios/playground'
    current_dir=$PWD
    cd $builddir
    product=$(PWD)'/build/Debug-iphoneos/WeexDemo.app'

    pod update
    [ -f product ] && rm -rf product
    
    xcodebuild clean build -quiet -workspace WeexDemo.xcworkspace -sdk iphonesimulator -scheme Pods-WeexDemo SYMROOT=$(PWD)/build CODE_SIGN_IDENTITY="" CODE_SIGNING_REQUIRED=NO
    xcodebuild clean build -quiet -workspace WeexDemo.xcworkspace -sdk iphonesimulator -scheme WeexSDK SYMROOT=$(PWD)/build CODE_SIGN_IDENTITY="" CODE_SIGNING_REQUIRED=NO
    xcodebuild clean build -quiet -arch x86_64 -configuration RELEASE -workspace WeexDemo.xcworkspace -sdk iphonesimulator -scheme WeexDemo SYMROOT=$(PWD)/build CODE_SIGN_IDENTITY="" CODE_SIGNING_REQUIRED=NO

    echo $product
    cd $current_dir;

}

function runiOS {
    echo 'Run in iOS...'
    echo $1
    buildiOS
    echo 'killAll Simulator......'
    killAll Simulator || echo 'killall failed'
    # ps -ef
    startMacacaServer
    startWeexServer
    platform=ios ./node_modules/mocha/bin/mocha  $1 -f '@ignore-ios' -i --recursive --bail --verbose
}

function runWeb {
    echo 'run web'
    startMacacaServer
    startWeexServer
    browser=chrome ./node_modules/mocha/bin/mocha  $1 -f '@ignore-web' -i --recursive --bail
}

function killserver {
    ps -ef | grep 'macaca-cli-server' | grep -v grep | awk '{print $2}' | xargs kill || echo 'nothing to kill'
}

platform_android='android'
platform=${1:-$platform_android}
 
killserver
#run tests
if [ $platform = $platform_android ]; then
    runAndroid ./test/scripts/
elif [ $platform = 'web' ]; 
then
    runWeb ./test/scripts/
else
    runiOS ./test/scripts/
fi
killserver
