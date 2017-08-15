#!/usr/bin/env bash
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
    platform=android ./node_modules/mocha/bin/mocha  $1 --reporter mocha-simple-html-reporter --reporter-options output=report.html -f '@ignore-android' -i --recursive --bail --verbose --retries 3
}

function buildiOS {
    builddir=$(pwd)'/ios/playground'
    current_dir=$PWD
    cd $builddir
    
    pod update
    if [ $needCoverage = "cover" ] && [ -d "./XcodeCoverage/" ]; then
        ./XcodeCoverage/podsGcovConfig
    fi
    product=$(PWD)'/build/Debug-iphonesimulator/'
    [ -f $product ] && rm -rf $product
    
    xcodebuild clean build -quiet -workspace WeexDemo.xcworkspace -sdk iphonesimulator -scheme Pods-WeexDemo SYMROOT=$(PWD)/build CODE_SIGN_IDENTITY="" CODE_SIGNING_REQUIRED=NO
    xcodebuild clean build -quiet -workspace WeexDemo.xcworkspace -sdk iphonesimulator -scheme WeexSDK SYMROOT=$(PWD)/build CODE_SIGN_IDENTITY="" CODE_SIGNING_REQUIRED=NO
    xcodebuild clean build -quiet -arch x86_64 -configuration RELEASE -workspace WeexDemo.xcworkspace -sdk iphonesimulator -scheme WeexDemo SYMROOT=$(PWD)/build CODE_SIGN_IDENTITY="" CODE_SIGNING_REQUIRED=NO

    echo $product
    cd $current_dir;

}

function runiOS {
    echo 'Run in iOS...'
    echo $1
    buildiOS $2
    echo 'killAll Simulator......'
    killAll Simulator || echo 'killall failed'
    # ps -ef
    startMacacaServer
    startWeexServer
    platform=ios ./node_modules/mocha/bin/mocha  $1 --reporter mocha-simple-html-reporter --reporter-options output=report.html -f '@ignore-ios' -i --recursive --bail --verbose --retries 3
}

function runWeb {
    echo 'run web'
    startMacacaServer
    startWeexServer
    browser=chrome ./node_modules/mocha/bin/mocha  $1 --reporter mocha-simple-html-reporter --reporter-options output=report.html -f '@ignore-web' -i --recursive --bail --verbose --retries 3
}

function killserver {
    ps -ef | grep 'macaca-cli-server' | grep -v grep | awk '{print $2}' | xargs kill || echo 'nothing to kill'
}

platform_android='android'
platform=${1:-$platform_android}
coverage_status='noCover'
needCoverage=${2:-$coverage_status}
 
killserver
#run tests
if [ $platform = $platform_android ]; then
    runAndroid ./test/scripts/
elif [ $platform = 'web' ]; 
then
    runWeb ./test/scripts/
else
    echo "$needCoverage"
    runiOS ./test/scripts/ "$needCoverage"
fi
killserver
