function installAndroidSDK {
    # brew install android-sdk
    # export ANDROID_HOME=/usr/local/opt/android-sdk
    # export PATH=$PATH:$ANDROID_HOME/tools:$ANDROID_HOME/platform-tools
    echo yes | android update sdk --all --no-ui --force -t platform-tools #platform tools
    echo yes | android update sdk --all --no-ui --force -t tools #tools
    echo yes | android update sdk --all --no-ui --force -t build-tools-23.0.2 #build-tool
    echo yes | android update sdk --all --no-ui --force -t android-19 #sdk android-19
    echo yes | android update sdk --all --no-ui --force -t android-23 #sdk android-23
    echo yes | android update sdk --all --no-ui --force -t sys-img-armeabi-v7a-android-21 #sys-img
    echo yes | android update sdk --all --no-ui --force -t extra-android-m2repository #support
}

function createAVD {
    echo no | android create avd --force -n weexavd -t android-19 --abi default/armeabi-v7a
}

function startAVD {
  emulator -avd weexavd -no-audio -no-window
}

function waitForEmulator {
  local bootanim=""
  export PATH=$(dirname $(dirname $(which android)))/platform-tools:$PATH
  until [[ "$bootanim" =~ "stopped" ]]; do
    sleep 5
    bootanim=$(adb -e shell getprop init.svc.bootanim 2>&1)
    echo "emulator status=$bootanim"
  done
  adb shell input keyevent 82 &
}

function setup_cpt {
    target_android='android'
    target_ios='ios'
    target_danger='danger'
    target_jsfm='jsfm'

    target=${1:-$target_android}
    
    setupBasic

    if [ $target = $target_android ]; then
        # setupBasic
        # installAndroidSDK
        npm install -g macaca-cli
        npm install -g macaca-android
        createAVD
        startAVD &
        npm install
        export DISPLAY=:99.0
    elif [ $target = $target_ios ]
    then
        # setupBasic
        npm install -g macaca-cli
        brew update
        brew install ios-webkit-debug-proxy
        npm install -g macaca-ios
        npm install
        gem install danger danger-xcode_summary xcpretty xcpretty-json-formatter
    elif [ $target = $target_jsfm ]
    then
        # setupBasic
        npm install
    else
        npm install
    fi
}

function setupBasic {
    curl -o- https://raw.githubusercontent.com/creationix/nvm/v0.33.1/install.sh | bash
    export NVM_DIR="$HOME/.nvm"
    [ -s "$NVM_DIR/nvm.sh" ] && \. "$NVM_DIR/nvm.sh"
    export CHROME_BIN=chromium-browser
    nvm install 7.0.0
    nvm use 7.0.0
    
}

function printEnvInfo {
    node -v
    nvm ls
    printenv
}

function test_cpt {
    echo 'cilog:start test ......'

    target_android='android'
    target_ios='ios'
    target_danger='danger'
    target_jsfm='jsfm'

    target=${1:-$target_android}
    echo "cilog: target: $target"
    
    if [ $target = $target_android ]; then
        ./test/serve.sh 2&>1 > /dev/null &
        # export ANDROID_HOME=/usr/local/opt/android-sdk
        cd android && ./run-ci.sh && cd $TRAVIS_BUILD_DIR
        waitForEmulator
        run_in_ci=true ./test/run.sh
    elif [ $target = $target_ios ]
    then
        ./test/serve.sh 2&>1 > /dev/null &
        xcodebuild -project ios/sdk/WeexSDK.xcodeproj test -scheme WeexSDKTests CODE_SIGN_IDENTITY="" CODE_SIGNING_REQUIRED=NO -destination 'platform=iOS Simulator,name=iPhone 6' | XCPRETTY_JSON_FILE_OUTPUT=ios/sdk/xcodebuild.json xcpretty -f `xcpretty-json-formatter`
        run_in_ci=true ./test/run.sh ios
        bundle exec danger --dangerfile=Dangerfile-ios
    elif [ $target = $target_jsfm ]
    then
        npm run build
        npm run test
    else
        npm run danger
    fi
}

