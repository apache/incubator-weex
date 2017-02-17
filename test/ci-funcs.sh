function setup {
    //detect platform
    platform_android='android'
    platform=${1:-$platform_android}
    
    if [ $platform = $platform_android ]; then
        setupBasic
        "JAVA_HOME=$(/usr/libexec/java_home) npm install -g macaca-android"
        npm install
        source setup-android-ci.sh
        installAndroidSDK
        createAndStartAVD &
        export DISPLAY=:99.0
    else
        setupBasic
        npm install -g macaca-ios
        npm install
    fi
}

function setupBasic {
    brew update
    brew install nvm
    export CHROME_BIN=chromium-browser
    source $(brew --prefix nvm)/nvm.sh
    nvm install 7.0.0
    brew install ios-webkit-debug-proxy
    npm install -g macaca-cli
}

function test_cpt {
    //detect platform
    ./test/serve.sh 2&>1 > /dev/null &

    platform_android='android'
    platform=${1:-$platform_android}
    
    if [ $platform = $platform_android ]; then
        cd android && ./run-ci.sh && cd $TRAVIS_BUILD_DIR
        waitForEmulator
        JAVA_HOME=$(/usr/libexec/java_home) run_in_ci=true ./test/run.sh
    else
        run_in_ci=true ./test/run.sh ios
    fi
}

