function setup {
    target_android='android'
    target_ios='ios'
    target_danger='danger'

    target=${1:-$target_android}
    
    if [ $target = $target_android ]; then
        setupBasic
        "JAVA_HOME=$(/usr/libexec/java_home) npm install -g macaca-android"
        npm install
        source setup-android-ci.sh
        installAndroidSDK
        createAndStartAVD &
        export DISPLAY=:99.0
    elif [ $target = $target_ios]
        setupBasic
        npm install -g macaca-ios
        npm install
    else
        gem install danger danger-xcode_summary xcpretty xcpretty-json-formatter
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
    ./test/serve.sh 2&>1 > /dev/null &
    export ANDROID_HOME=/usr/local/opt/android-sdk
    target_android='android'
    target_ios='ios'
    target_danger='danger'

    target=${1:-$target_android}
    
    if [ $target = $target_android ]; then
        cd android && ./run-ci.sh && cd $TRAVIS_BUILD_DIR
        waitForEmulator
        JAVA_HOME=$(/usr/libexec/java_home) run_in_ci=true ./test/run.sh
    elif [ $target = $target_ios]
        xcodebuild -project ios/sdk/WeexSDK.xcodeproj test -scheme WeexSDKTests CODE_SIGN_IDENTITY="" CODE_SIGNING_REQUIRED=NO -destination 'platform=iOS Simulator,name=iPhone 6' | XCPRETTY_JSON_FILE_OUTPUT=ios/sdk/xcodebuild.json xcpretty -f `xcpretty-json-formatter`
        run_in_ci=true ./test/run.sh ios
    else
        bundle exec danger
    fi
}

